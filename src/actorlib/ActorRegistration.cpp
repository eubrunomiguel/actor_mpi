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

#include "ActorRegistration.hpp"

#include "Actor.hpp"
#include "ActorGraph.hpp"

#include <upcxx/upcxx.hpp>

#include <mutex>
#include <condition_variable>

ReadinessIndicator::ReadinessIndicator() 
    : readyActors(0),
      startSignal(false),
      stopSignal(false){
}

void registerRpc(Actor *a) {
    a->parentActorGraph->rpcsInFlight++;
}

void deregisterRpc(Actor *a) {
    a->parentActorGraph->rpcsInFlight--;
}

void registerLpc(Actor *a) {
    a->parentActorGraph->lpcsInFlight++;
}

void deregisterLpc(Actor *a) {
    a->parentActorGraph->lpcsInFlight--;
}

void awaitGraphReadiness(Actor *a) {
    std::unique_lock<std::mutex> lck(a->parentActorGraph->actorLock);
    while (!a->parentActorGraph->readinessIndicator.startSignal) {
        a->parentActorGraph->readinessIndicator.actorSignal.wait(lck);
    }
}

void awaitActorReadiness(ActorGraph *ag) {
    std::unique_lock<std::mutex> lck(ag->actorLock);
    while (ag->localActors != ag->readinessIndicator.readyActors.load()) {
        ag->readinessIndicator.actorGraphSignal.wait(lck);
    }
}

void signalActorReadiness(Actor *a) {
    std::unique_lock<std::mutex> lck(a->parentActorGraph->actorLock);
    a->parentActorGraph->readinessIndicator.readyActors++;
    a->parentActorGraph->readinessIndicator.actorGraphSignal.notify_one();
}

void signalActorGraphReadiness(ActorGraph *ag) {
    std::unique_lock<std::mutex> lck(ag->actorLock);
    ag->readinessIndicator.startSignal = true;
    ag->readinessIndicator.actorSignal.notify_all();
}

void awaitActorGraphTermination(ActorGraph *ag) {
    std::unique_lock<std::mutex> lck(ag->actorLock);
    while (!ag->readinessIndicator.stopSignal) {
        ag->readinessIndicator.actorSignal.wait(lck);
    }
}

void signalActorGraphTermination(ActorGraph *ag) {
    std::unique_lock<std::mutex> lc(ag->actorLock);
    upcxx::barrier();
    ag->readinessIndicator.stopSignal = true;
    ag->readinessIndicator.actorSignal.notify_all();
}
