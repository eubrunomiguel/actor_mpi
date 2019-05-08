/**
 * @file
 * This file is part of Pond.
 *
 * @author Alexander Pöppl (poeppl AT in.tum.de, https://www5.in.tum.de/wiki/index.php/Alexander_P%C3%B6ppl,_M.Sc.)
 *
 * @section LICENSE
 *
 * Pond is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pond is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pond.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 *
 */

#include "orchestration/ActorOrchestrator.hpp"

#include "actor/SimulationActor.hpp"
#include "orchestration/ActorDistributor.hpp"
#include "util/Configuration.hpp"
#include "util/Logger.hh"

#include <chrono>
#include <vector>
#include <algorithm>
#include <limits>

static tools::Logger &l = tools::Logger::logger;

ActorOrchestrator::ActorOrchestrator(Configuration config)
    : config(config) {
}

void ActorOrchestrator::initActorGraph() {
    createActors();
    upcxx::barrier();
#ifndef NDEBUG
    l.printString(ag.prettyPrint());
#endif
    upcxx::barrier();
    connectActors();
    initializeActors();
}

void ActorOrchestrator::createActors() {
    size_t xActors = config.xSize / config.patchSize;
    size_t yActors = config.ySize / config.patchSize;
    auto sd = createActorDistributor(xActors, yActors);
    localActorCoords = sd->getLocalActorCoordinates();
    for (std::pair<size_t, size_t> &coordPair : localActorCoords) {
        SimulationActor *a = new SimulationActor(config, coordPair.first, coordPair.second);
        ag.addActor(a);
        localActors.push_back(a);
    }
}

void ActorOrchestrator::connectActors() {
    size_t xActors = config.xSize / config.patchSize;
    size_t yActors = config.ySize / config.patchSize;
    for (auto &coordPair : localActorCoords) {
        this->connectToNeighbors(coordPair, xActors, yActors);
    }
}

void ActorOrchestrator::initializeActors() {
    float safeTimestep{std::numeric_limits<float>::infinity()};
    for (SimulationActor *a : localActors) {
        a->initializeBlock();
        auto blockSafeTs = a->getMaxBlockTimestepSize();
        safeTimestep = std::min(safeTimestep, blockSafeTs);
#ifndef NDEBUG
        std::cout << "Safe timestep on actor " << a->name << " is " << blockSafeTs << " current min: " << safeTimestep << std::endl;
#endif
    }
    float globalSafeTs = upcxx::reduce_all(safeTimestep, upcxx::op_min).wait();
#ifndef NDEBUG
    l.cout() << "Received safe timestep: " << globalSafeTs << " local was " << safeTimestep << std::endl;
#endif
    for (SimulationActor *a : localActors) {
        a->setTimestepBaseline(globalSafeTs);
    }
}

void ActorOrchestrator::simulate() {
    l.printString("********************* Start Simulation **********************", false);
    auto runTime = ag.run();
    l.printString("********************** End Simulation ***********************", false);
    uint64_t localPatchUpdates = 0;
    uint64_t totalPatchUpdates = 0;
    for (SimulationActor *a : localActors) {
        localPatchUpdates += a->getNumberOfPatchUpdates();
    }
    totalPatchUpdates = upcxx::reduce_all(localPatchUpdates, upcxx::op_add).wait();
    if (!upcxx::rank_me()) {
        l.cout() << "Performed " << totalPatchUpdates << " patch updates in " << runTime << " seconds." << std::endl;
        l.cout() << "Performed " << (totalPatchUpdates * config.patchSize * config.patchSize) << " cell updates in " << runTime << " seconds." << std::endl;
        l.cout() << "=> " <<  (static_cast<double>(totalPatchUpdates * config.patchSize * config.patchSize) / runTime)<< " CellUpdates/s" << std::endl;
        l.cout() << "=> " << (static_cast<double>(135 * 2 * totalPatchUpdates * config.patchSize * config.patchSize) / runTime) << " FlOps/s" << std::endl;
    }
}

void ActorOrchestrator::connectToNeighbors(std::pair<size_t, size_t> &coords, size_t xActors, size_t yActors) {
    auto simArea = makePatchArea(config, coords.first, coords.second);
    auto curActor = ag.getActor(simArea.toString());
#ifndef NDEBUG
    l.cout() << simArea << " is local " << curActor.is_local() << std::endl;
#endif
    if (coords.first > 0) {
        auto leftSimArea = makePatchArea(config, coords.first - 1, coords.second);
        auto leftActor = ag.getActor(leftSimArea.toString());
#ifndef NDEBUG
        l.cout() << "Connecting " << simArea << " -> " << leftSimArea << std::endl;
#endif
        ag.connectPorts(curActor, "BND_LEFT", leftActor, "BND_RIGHT");
    }
    if (coords.first < xActors - 1) {
        auto rightSimArea = makePatchArea(config, coords.first + 1, coords.second);
        auto rightActor = ag.getActor(rightSimArea.toString());
#ifndef NDEBUG
        l.cout() << "Connecting " << simArea << " -> " << rightSimArea << std::endl;
#endif
        ag.connectPorts(curActor, "BND_RIGHT", rightActor, "BND_LEFT");
    }
    if (coords.second > 0) {
        auto bottomSimArea = makePatchArea(config, coords.first, coords.second - 1);
        auto bottomActor = ag.getActor(bottomSimArea.toString());
#ifndef NDEBUG
        l.cout() << "Connecting " << simArea << " -> " << bottomSimArea << std::endl;
#endif
        ag.connectPorts(curActor, "BND_BOTTOM", bottomActor, "BND_TOP");
    }
    if (coords.second < yActors - 1) {
        auto topSimArea = makePatchArea(config, coords.first, coords.second + 1);
        auto topActor = ag.getActor(topSimArea.toString());
#ifndef NDEBUG
        l.cout() << "Connecting " << simArea << " -> " << topSimArea << std::endl;
#endif
        ag.connectPorts(curActor, "BND_TOP", topActor, "BND_BOTTOM");
    }
}
