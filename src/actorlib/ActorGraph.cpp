/**
 * @file
 * This file is part of actorlib.
 *
 * @author Alexander Pöppl (poeppl AT in.tum.de, https://www5.in.tum.de/wiki/index.php/Alexander_P%C3%B6ppl,_M.Sc.)
 *
 * @section LICENSE
 *
 * actorlib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * actorlib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with actorlib.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 */

#include "ActorGraph.hpp"


#include "Actor.hpp"
#include "AbstractInPort.hpp"
#include "AbstractOutPort.hpp"
#include "Channel.hpp"
#include "config.hpp"
#include <mpi.h>

#include <chrono>

//#define DEBUG_ACTOR_TERMINATION
#define ACTOR_PARALLEL_TERMINATION_HACK


using namespace std;

GlobalChannelRef connectDestination(GlobalActorRef destinationActor, std::string destinationPortName);
upcxx::future<> connectSource(GlobalActorRef sourceActor, std::string sourcePortName, GlobalChannelRef channelRef);

// ActorGraph instance methods

ActorGraph::ActorGraph() 
    : localActors(0),
      remoteGraphComponents(this),
      masterPersona(upcxx::master_persona()),
      activeActors(0),
      rpcsInFlight(0),
      lpcsInFlight(0) {
    if (!upcxx::rank_me()) {
        std::cout << config::configToString();
    }
}

void ActorGraph::addActor(Actor *a) {
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    checkInsert(a->name, world_rank);

    a->parentActorGraph = this;
    localActors++;
}

void ActorGraph::synchronizeActors() {
    // Load world information
    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Exchange local number of actors per rank
    int *numActorsPerRank = (int *)malloc(sizeof(int) * worldSize);
    MPI_Allgather(&localActors, 1, MPI_INT, numActorsPerRank, 1, MPI_INT, MPI_COMM_WORLD);

    int totalActors = 0;
    for (int i = 0; i < worldSize; i++){
        totalActors += numActorsPerRank[i];
    }

    // Exchange Actors
    // TODO: only int at the moment, need the name as well

    // Current map to flat array
    int *myActors = (int *)malloc(sizeof(int) * actors.size());
    int myActorsIndex = 0;
    for (std::pair<std::string, int> element : actors){
        myActors[myActorsIndex] = element.second;
        myActorsIndex++;
    }

    int *displacement = (int *)malloc(sizeof(int) * worldSize);
    displacement[0] = 0;
    int currentIndex = 0;
    for (int i = 1; i < worldSize; i++){
        displacement[i] = numActorsPerRank[i-1] + currentIndex;
        currentIndex = displacement[i];
    }

    int *globalActors = (int *)malloc(sizeof(int) * totalActors);
    MPI_Allgatherv(myActors,
            localActors,
            MPI_INT,
            globalActors,
            numActorsPerRank,
            displacement,
            MPI_INT,
            MPI_COMM_WORLD);

    // Flat global actors or map
    for (int i = 0; i < totalActors; i++){
        // TODO: do not add our locals
        this->checkInsert("Name not defined by mpi structure", globalActors[i]);
    }

    delete numActorsPerRank;
    delete myActors;
    delete displacement;
    delete globalActors;
}

void ActorGraph::checkInsert(string actorName, int actorRank) {
    if (this->actors.find(actorName) != this->actors.end()) {
        throw std::runtime_error("May not add actor that is already existing.");
    }    
    this->actorLock.lock();
    this->actors.emplace(actorName, actorRank);
    this->actorLock.unlock();
}

void ActorGraph::connectPorts(int sourceActor, std::string sourcePortName, int destinationActor, std::string destinationPortName) {
    // upcxx::future<R> upcxx::rpc(intrank_t r, F &&func, Args &&...args);
    //    This executes function func on rank r and returns the result as a future of type R

    if (destinationActor.where() == upcxx::rank_me()) {
        connectFromDestination(sourceActor, sourcePortName, destinationActor, destinationPortName).wait();
    } else if (sourceActor.where() == upcxx::rank_me()) {
        connectFromSource(sourceActor, sourcePortName, destinationActor, destinationPortName).wait();
    } else {
        connectFromThird(sourceActor, sourcePortName, destinationActor, destinationPortName).wait();
    }
}

upcxx::future<> ActorGraph::connectFromDestination(GlobalActorRef sourceActor, std::string sourcePortName, GlobalActorRef destinationActor, std::string destinationPortName) {
    GlobalChannelRef c = connectDestination(destinationActor, destinationPortName);
    auto srcFut = upcxx::rpc(sourceActor.where(), connectSource, sourceActor, sourcePortName, c);
    return srcFut;
}

upcxx::future<> ActorGraph::connectFromSource(GlobalActorRef sourceActor, std::string sourcePortName, GlobalActorRef destinationActor, std::string destinationPortName) {
    upcxx::future<GlobalChannelRef> channelFut = upcxx::rpc(destinationActor.where(), connectDestination, destinationActor, destinationPortName);
    upcxx::future<> sourceDone = channelFut.then([=](GlobalChannelRef c) {
        upcxx::future<> sourceRpcDone = upcxx::rpc(sourceActor.where(), connectSource, sourceActor, sourcePortName, c);
        return sourceRpcDone;
    });
    return sourceDone;
}

upcxx::future<> ActorGraph::connectFromThird(GlobalActorRef sourceActor, std::string sourcePortName, GlobalActorRef destinationActor, std::string destinationPortName) {
    upcxx::future<> allDone = upcxx::rpc(
        sourceActor.where(), 
        [] (upcxx::dist_object<ActorGraph *> &rag,  GlobalActorRef sourceActor, std::string sourcePortName, GlobalActorRef destinationActor, std::string destinationPortName) {
            return (*rag)->connectFromSource(sourceActor, sourcePortName, destinationActor, destinationPortName);
        }, 
        this->remoteGraphComponents, 
        sourceActor, 
        sourcePortName, 
        destinationActor, 
        destinationPortName
    );
    return allDone;
}

int ActorGraph::getNumActors() {
    return actors.size();
}

int ActorGraph::getActor(string name) {
    auto entry = actors.find(name);
    if (entry != actors.end()) {
        return entry->second;
    } else {
        throw std::runtime_error("unable to find actor named "s + name);
    }
}

string ActorGraph::prettyPrint() {
    stringstream ss;
    ss << "ActorGraph {" << endl;
    ss << "  Actors {" << endl;

    for (auto &kv : this->actors) {
        ss << "    " << kv.first << "\t" << kv.second << endl;
    }
    ss << "  }" << endl;
    ss << "}";
    return ss.str();

}

double ActorGraph::run() {
    double runTime = 0.0;
    upcxx::barrier();
    this->finishInitialization();
    upcxx::barrier();

    for (auto &actorPairs : actors) {
        if (actorPairs.second.where() == upcxx::rank_me()) {
            this->activeActors++;
            auto aRef = *(actorPairs.second.local());
            aRef->start();
        }
    }
    upcxx::barrier();
    auto start = std::chrono::steady_clock::now();
    while (activeActors.load() > 0) {
        upcxx::progress();
        for (auto &at : actorTriggerCount) {
            if (at.second > 0) {
                at.first->act();
                at.second--;
            }
        }
    }
    auto end = std::chrono::steady_clock::now();
    runTime = std::chrono::duration<double, std::ratio<1>>(end - start).count();

    // Drain the queues, we want no more messages in flight.
    while (rpcsInFlight.load() > 0 || lpcsInFlight.load() > 0) {
        // std::cout << "Rank " << upcxx::rank_me() << "\tWind-Down: RPCs: " << rpcsInFlight.load() << " LPCs: " << lpcsInFlight.load() << std::endl;
        upcxx::progress();
    }

    upcxx::barrier();
    return runTime;
}

void ActorGraph::finishInitialization() {
    upcxx::progress();
    for (auto &actorPairs : actors) {
        if (actorPairs.second.where() == upcxx::rank_me()) {
            auto aRef = *(actorPairs.second.local());
            this->actorTriggerCount[aRef] = 1;
        }
    }
}

// Non-instance methods

GlobalChannelRef connectDestination(GlobalActorRef destinationActor, std::string destinationPortName) {
    auto dstActorPtr = *(destinationActor.local());
    auto dstIp = dstActorPtr->getInPort(destinationPortName);
    auto channelRef = dstIp->getChannel();
    dstIp->registerWithChannel();
    return channelRef;
}

upcxx::future<> connectSource(GlobalActorRef sourceActor, std::string sourcePortName, GlobalChannelRef channelRef) {
    auto srcActorPtr = *(sourceActor.local());
    auto srcOp = srcActorPtr->getOutPort(sourcePortName);
    srcOp->setChannel(channelRef);
    auto regFut = srcOp->registerWithChannel();
    return regFut;
}

ActorGraph::~ActorGraph() {
    actors.clear();
    actorTriggerCount.clear();
}

void ActorGraph::markAsDirty(Actor *a) {
    this->actorTriggerCount[a] = this->actorTriggerCount[a] + 1;
}
