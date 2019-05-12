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

#include <chrono>
#include "mpi.h"

#include "ActorGraph.hpp"

#include "Actor.hpp"
#include "AbstractInPort.hpp"
#include "AbstractOutPort.hpp"
#include "Channel.hpp"
#include "utils/config.hpp"
#include "utils/mpi_helper.hpp"
#include "Port.h"

//#define DEBUG_ACTOR_TERMINATION
#define ACTOR_PARALLEL_TERMINATION_HACK

using namespace std;

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

void ActorGraph::addLocalActor(Actor *a) {
    localActors.emplace(a->name, a);
}

void ActorGraph::synchronizeActors() {
    int worldSize = MPIHelper::worldSize();

    // Exchange local number of actors
    int *numActorsPerRank = (int *) malloc(sizeof(int) * worldSize);
    int totalLocalActors = getNumActorsLocal();
    MPI_Allgather(&totalLocalActors, 1, MPI_INT, numActorsPerRank, 1, MPI_INT, MPI_COMM_WORLD);

    int totalActors = 0;
    for (int i = 0; i < worldSize; i++) {
        totalActors += numActorsPerRank[i];
    }

    // Exchange Actors
    // TODO: only int at the moment, need a user-defined data type

    // Current map to flat array
    int *myActors = (int *) malloc(sizeof(int) * getNumActorsLocal());
    int myActorsIndex = 0;
    for (std::pair<std::string, int> element : actors) {
        myActors[myActorsIndex] = element.second;
        myActorsIndex++;
    }

    int *displacement = (int *) malloc(sizeof(int) * worldSize);
    displacement[0] = 0;
    int currentIndex = 0;
    for (int i = 1; i < worldSize; i++) {
        displacement[i] = numActorsPerRank[i - 1] + currentIndex;
        currentIndex = displacement[i];
    }

    int *globalActors = (int *) malloc(sizeof(int) * totalActors);
    MPI_Allgatherv(myActors,
                   localActors,
                   MPI_INT,
                   globalActors,
                   numActorsPerRank,
                   displacement,
                   MPI_INT,
                   MPI_COMM_WORLD);

    for (int i = 0; i < totalActors; i++) {
        this->checkInsert("Name not defined by mpi structure", globalActors[i]);
    }

    free(numActorsPerRank);
    free(myActors);
    free(displacement);
    free(globalActors);
}

void ActorGraph::checkInsert(const string &actorName, int actorRank) {
    // TODO: Need lock here?
    if (this->actors.find(actorName) != this->actors.end())
        throw std::runtime_error("May not add actor that is already existing.");
    this->actors.emplace(actorName, actorRank);
}

void ActorGraph::connectPorts(const string &sourceActorName, const string &sourcePortName,
                              const string &destinationActorName, const string &destinationPortName) {
    auto srcLocalActorIt = localActors.find(sourceActorName);
    auto destLocalActorIt = localActors.find(destinationActorName);

    // TODO: handle the case for me!? there was not this case onm wold implementation

    if (destLocalActorIt != localActors.end()) {
        // Destination is local
        // inPort || destinated Actor holds the channel
        // It is responsible to warn writter when it has been read
        auto actorPtr = destLocalActorIt->second;
        auto destInPort = actorPtr->getInPort(destinationPortName);

        if (srcLocalActorIt != localActors.end()) {
            // localToLocal
            auto srcOutPort = srcLocalActorIt->second->getOutPort(sourcePortName);
            destInPort->receiveMessagesFrom(std::make_unique<PortIdentification<AbstractOutPort>>(srcOutPort));
        } else {
            // remoteToLocal
            auto actorIt = actors.find(sourceActorName);
            if (actorIt == actors.end())
                throw std::runtime_error("Cannot find source actor.");
            destInPort->receiveMessagesFrom(
                    std::make_unique<PortIdentification<AbstractOutPort>>(sourcePortName, actorIt->second));
        }

        return;
    }

    if (srcLocalActorIt != localActors.end()) {
        // Source is Local
        auto actorPtr = srcLocalActorIt->second;
        auto srcOutPort = actorPtr->getOutPort(sourcePortName);

        if (destLocalActorIt != localActors.end()) {
            // localToLocal
            auto destInPort = destLocalActorIt->second->getInPort(destinationPortName);
            srcOutPort->sendMessagesTo(std::make_unique<PortIdentification<AbstractInPort>>(destInPort));
        } else {
            // localToRemote
            auto actorIt = actors.find(destinationActorName);
            if (actorIt == actors.end())
                throw std::runtime_error("Cannot find destinated actor.");
            srcOutPort->sendMessagesTo(
                    std::make_unique<PortIdentification<AbstractInPort>>(destinationPortName, actorIt->second));
        }

        return;
    }

    throw std::runtime_error("Cannot connect two external actors.");
}

int ActorGraph::getNumActors() const {
    return actors.size();
}

int ActorGraph::getNumActorsLocal() const {
    return actors.size();
}

int ActorGraph::getActorByName(std::string name) {
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
    auto runTime = 0.0;
    MPI_Barrier(MPI_COMM_WORLD);

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

ActorGraph::~ActorGraph() {
    actors.clear();
    actorTriggerCount.clear();
}

void ActorGraph::markAsDirty(Actor *a) {
    this->actorTriggerCount[a] = this->actorTriggerCount[a] + 1;
}
