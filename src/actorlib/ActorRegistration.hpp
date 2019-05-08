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
 *
 */

#include <condition_variable>
#include <atomic>

#pragma once

class Actor;
class ActorGraph;

struct ReadinessIndicator {
    std::atomic<size_t> readyActors;
    std::condition_variable actorGraphSignal;
    std::condition_variable actorSignal;
    bool startSignal;
    bool stopSignal;

    ReadinessIndicator();
};


void registerRpc(Actor *a);
void deregisterRpc(Actor *a);
void registerLpc(Actor *a);
void deregisterLpc(Actor *a);

void awaitGraphReadiness(Actor *a);
void awaitActorReadiness(ActorGraph *a);

void signalActorReadiness(Actor *a);
void signalActorGraphReadiness(ActorGraph *ag);

void awaitActorGraphTermination(ActorGraph *ag);
void signalActorGraphTermination(ActorGraph *ag);
