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

#include "utils/mpi_helper.hpp"
#include <memory>
#include <mutex>
#include <unordered_map>

#include "Actor.hpp"

#pragma once

class ActorGraph {

  friend class Actor;

private:
  std::unordered_map<std::string, MPIHelper::RankId> actors;
  std::unordered_map<std::string, Actor *> localActors;

  std::mutex actorLock; // TODO: these locks may be removed
  std::unordered_map<Actor *, size_t> actorTriggerCount;

public:
  ActorGraph() = default;
  ~ActorGraph();
  ActorGraph(ActorGraph &other) = delete;
  ActorGraph &operator=(ActorGraph &other) = delete;

  void addLocalActor(Actor *a);
  void synchronizeActors();
  void connectPorts(const std::string &sourceActorName,
                    const std::string &sourcePortName,
                    const std::string &destinationActorName,
                    const std::string &destinationPortName);
  int getNumActors() const;
  int getNumActorsLocal() const;
  MPIHelper::RankId getActorByName(const std::string &name) const;
  std::string prettyPrint() const;
  double run();

private:
  void checkInsert(const std::string &actorName, int actorRank);
};
