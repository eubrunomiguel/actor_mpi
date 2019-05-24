/**
 * @file
 * This file is part of actorlib.
 *
 * @author Alexander Pöppl (poeppl AT in.tum.de,
 * https://www5.in.tum.de/wiki/index.php/Alexander_P%C3%B6ppl,_M.Sc.)
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

#include "mpi.h"
#include <chrono>

#include "ActorGraph.hpp"

#include "AbstractInPort.hpp"
#include "AbstractOutPort.hpp"
#include "Actor.hpp"
#include "Channel.hpp"
#include "PortIdentification.h"
#include "utils/mpi_helper.hpp"

using namespace std;

void ActorGraph::addLocalActor(Actor *a) { localActors.emplace(a->name, a); }

void ActorGraph::synchronizeActors() {
  int worldSize = mpi::world();

  // Exchange local number of actors
  int *numActorsPerRank = (int *)malloc(sizeof(int) * worldSize);
  int totalLocalActors = getNumActorsLocal();
  MPI_Allgather(&totalLocalActors, 1, MPI_INT, numActorsPerRank, 1, MPI_INT,
                MPI_COMM_WORLD);

  int totalActors = 0;
  for (int i = 0; i < worldSize; i++) {
    totalActors += numActorsPerRank[i];
  }

  // Exchange Actors
  // TODO: only int at the moment, need a user-defined data type

  // Current map to flat array
  int *myActors = (int *)malloc(sizeof(int) * getNumActorsLocal());
  int myActorsIndex = 0;
  for (std::pair<std::string, int> element : actors) {
    myActors[myActorsIndex] = element.second;
    myActorsIndex++;
  }

  int *displacement = (int *)malloc(sizeof(int) * worldSize);
  displacement[0] = 0;
  int currentIndex = 0;
  for (int i = 1; i < worldSize; i++) {
    displacement[i] = numActorsPerRank[i - 1] + currentIndex;
    currentIndex = displacement[i];
  }

  int *globalActors = (int *)malloc(sizeof(int) * totalActors);
  MPI_Allgatherv(myActors, getNumActorsLocal(), MPI_INT, globalActors,
                 numActorsPerRank, displacement, MPI_INT, MPI_COMM_WORLD);

  for (int i = 0; i < totalActors; i++) {
    this->checkInsert("Name not defined by mpi structure", globalActors[i]);
  }

  free(numActorsPerRank);
  free(myActors);
  free(displacement);
  free(globalActors);
}

void ActorGraph::checkInsert(const string &actorName, int actorRank) {
  if (this->actors.find(actorName) != this->actors.end())
    throw std::runtime_error("May not add actor that is already existing.");
  this->actors.emplace(actorName, actorRank);
}

void ActorGraph::connectPorts(const string &sourceActorName,
                              const string &sourcePortName,
                              const string &destinationActorName,
                              const string &destinationPortName) {
  auto srcLocalActorIt = localActors.find(sourceActorName);
  auto destLocalActorIt = localActors.find(destinationActorName);

  if (destLocalActorIt == localActors.end() &&
      srcLocalActorIt == localActors.end())
    throw std::runtime_error("Cannot connect two external actors.");

  if (destLocalActorIt != localActors.end()) {
    // Destination is local
    // inPort || destinated Actor holds the channel
    // It is responsible to warn writter when it has been read
    Actor *actor = destLocalActorIt->second;
    AbstractInPort *destInPort = actor->getInPort(destinationPortName);

    if (srcLocalActorIt != localActors.end()) {
      // localToLocal
      AbstractOutPort *srcOutPort =
          srcLocalActorIt->second->getOutPort(sourcePortName);
      destInPort->receiveMessagesFrom(
          PortIdentification<AbstractOutPort>(srcOutPort));
    } else {
      // remoteToLocal
      auto actorIt = actors.find(sourceActorName);
      if (actorIt == actors.end())
        throw std::runtime_error("Cannot find source actor.");
      destInPort->receiveMessagesFrom(
          PortIdentification<AbstractOutPort>(sourcePortName, actorIt->second));
    }
  }

  if (srcLocalActorIt != localActors.end()) {
    // Source is Local
    auto actorPtr = srcLocalActorIt->second;
    auto srcOutPort = actorPtr->getOutPort(sourcePortName);

    if (destLocalActorIt != localActors.end()) {
      // localToLocal
      auto destInPort =
          destLocalActorIt->second->getInPort(destinationPortName);
      srcOutPort->sendMessagesTo(
          PortIdentification<AbstractInPort>(destInPort));
    } else {
      // localToRemote
      auto actorIt = actors.find(destinationActorName);
      if (actorIt == actors.end())
        throw std::runtime_error("Cannot find destined actor.");
      srcOutPort->sendMessagesTo(PortIdentification<AbstractInPort>(
          destinationPortName, actorIt->second));
    }
  }
}

int ActorGraph::getNumActors() const { return actors.size(); }

int ActorGraph::getNumActorsLocal() const { return actors.size(); }

mpi::rank ActorGraph::getActorByName(const std::string &name) const {
  auto entry = actors.find(name);
  if (entry != actors.end()) {
    return entry->second;
  } else {
    throw std::runtime_error("unable to find actor named "s + name);
  }
}

string ActorGraph::prettyPrint() const {
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
  MPI_Barrier(MPI_COMM_WORLD);
  auto start = std::chrono::steady_clock::now();

  while (true) {
    auto finished = 0;
    for (auto actorPairs : localActors) {
      if (actorPairs.second->act())
        finished++;
    }
    if (finished == localActors.size())
      break;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  auto end = std::chrono::steady_clock::now();

  return std::chrono::duration<double, std::ratio<1>>(end - start).count();
}

ActorGraph::~ActorGraph() {
  actors.clear();
  localActors.clear();
}
