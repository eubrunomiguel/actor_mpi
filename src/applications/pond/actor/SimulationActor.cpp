#include "actor/SimulationActor.hpp"

#include "util/Configuration.hpp"
#include "block/SWE_Block.hh"
#include "block/SWE_WaveAccumulationBlock.hh"
#include "block/BlockCommunicator.hpp"
#include "scenario/SimulationArea.hpp"
#include "scenario/SWE_Scenario.hh"
#include "writer/Writer.hh"
#ifdef WRITENETCDF
#include "writer/NetCdfWriter.hh"
#else 
#include "writer/VtkWriter.hh"
#endif

#include "actorlib/Actor.hpp"
#include "actorlib/InPort.hpp"
#include "actorlib/OutPort.hpp"
#include "util/Logger.hh"

static tools::Logger &l = tools::Logger::logger;

SimulationActor::SimulationActor(Configuration &config, size_t xPos, size_t yPos)
    : Actor(makePatchArea(config, xPos, yPos).toString()),
      config(config),
      position{xPos, yPos},
      block(config.patchSize, config.patchSize, config.dx, config.dy),
      currentState(SimulationActorState::INITIAL),
      currentTime(0.0f),
      timestepBaseline(0.0f),
      endTime(config.scenario->endSimulation()),
      patchUpdates(0),
      patchArea(makePatchArea(config, xPos, yPos)) {
    auto totalX = config.xSize / config.patchSize;
    auto totalY = config.ySize / config.patchSize;
    dataIn[BND_LEFT] = (xPos != 0) ? this->makeInPort<std::vector<float>, 32>("BND_LEFT") : nullptr;
    dataOut[BND_LEFT] = (xPos != 0) ? this->makeOutPort<std::vector<float>, 32>("BND_LEFT") : nullptr;
    dataIn[BND_RIGHT] = (xPos != totalX - 1) ? this->makeInPort<std::vector<float>, 32>("BND_RIGHT") : nullptr;
    dataOut[BND_RIGHT] = (xPos != totalX - 1) ? this->makeOutPort<std::vector<float>, 32>("BND_RIGHT") : nullptr;
    dataIn[BND_BOTTOM] = (yPos != 0) ? this->makeInPort<std::vector<float>, 32>("BND_BOTTOM") : nullptr;
    dataOut[BND_BOTTOM] = (yPos != 0) ? this->makeOutPort<std::vector<float>, 32>("BND_BOTTOM") : nullptr;
    dataIn[BND_TOP] = (yPos != totalY - 1) ? this->makeInPort<std::vector<float>, 32>("BND_TOP") : nullptr;
    dataOut[BND_TOP] = (yPos != totalY - 1) ? this->makeOutPort<std::vector<float>, 32>("BND_TOP") : nullptr;
#if defined(WRITENETCDF)
    writer = new io::NetCdfWriter(
            config.fileNameBase,
            block.getBathymetry(),
            {1,1,1,1},
            patchArea,
            config.scenario->endSimulation() / config.numberOfCheckpoints,
            config.patchSize,
            config.patchSize,
            config.dx,
            config.dy,
            patchArea.minX,
            patchArea.minY,
            1);
#elif defined(WRITEVTK)
    writer = new io::VtkWriter(config.fileNameBase, 
            block.getBathymetry(), 
            {1,1,1,1}, 
            config.patchSize, 
            config.patchSize,
            config.dx,
            config.dy,
            position[0] * config.patchSize, 
            position[1] * config.patchSize);
#else
    #error "Undefined!"
    writer = nullptr;
#endif
}

void SimulationActor::initializeBlock() {
    size_t xPos = position[0];
    size_t yPos = position[1];
    auto totalX = config.xSize / config.patchSize;
    auto totalY = config.ySize / config.patchSize;
    block.initScenario(patchArea.minX, patchArea.minY, *(config.scenario), true);
    initializeBoundary(BND_LEFT, [xPos]() { return xPos == 0; });
    initializeBoundary(BND_RIGHT, [xPos, totalX]() { return xPos == totalX - 1; });
    initializeBoundary(BND_BOTTOM, [yPos]() { return yPos == 0; });
    initializeBoundary(BND_TOP, [yPos, totalY]() { return yPos == totalY - 1; });
    this->computeWriteDelta();
#ifndef NDEBUG
    std::cout << this->toString() << std::endl;
#endif
}

void SimulationActor::initializeBoundary(BoundaryEdge edge, std::function<bool()> isBoundary) {
    const Scenario *sc = config.scenario;
    if (isBoundary()) {
        block.setBoundaryType(edge, sc->getBoundaryType(edge));
    } else {
        block.setBoundaryType(edge, PASSIVE);
        communicators[edge] = BlockCommunicator(config.patchSize, block.registerCopyLayer(edge), block.grabGhostLayer(edge));
    }
}

float SimulationActor::getMaxBlockTimestepSize() {
    block.computeMaxTimestep();
    return block.getMaxTimestep();
}

void SimulationActor::setTimestepBaseline(float timestepBaseline) {
    this->timestepBaseline = timestepBaseline;
}

void SimulationActor::act() {
    if (currentState == SimulationActorState::INITIAL && mayWrite()) {
#ifndef NDEBUG
        l.cout() << name << " sending initial data to neighbors." << std::endl;
#endif
#ifndef NOWRITE
        writeTimeStep(0.0f);
#endif
        sendData();
        currentState = SimulationActorState::RUNNING;
    } else if (currentState == SimulationActorState::RUNNING && currentTime < endTime
                && !hasReceivedTerminationSignal() && mayRead() && mayWrite() ) {
#ifndef NDEBUG
        l.cout() << name << " iteration at " << currentTime << std::endl;
#endif
        receiveData();
        block.setGhostLayer();
        block.computeNumericalFluxes();
        block.updateUnknowns(timestepBaseline);
        sendData();
        currentTime += timestepBaseline;
#ifndef NOWRITE
        if (currentTime >= nextWriteTime) {
            writeTimeStep(currentTime);
            nextWriteTime += outputDelta;
        }
#endif
        patchUpdates++;
        if (currentTime > endTime) {
#ifndef NDEBUG
            l.cout() << name << "\treached endTime." << std::endl;
#endif
            currentState = SimulationActorState::FINISHED;
            this->trigger();
        }
    } else if ((currentState == SimulationActorState::FINISHED || hasReceivedTerminationSignal())) {
#ifndef NDEBUG
        l.cout() << name << " terminating at " << currentTime << std::endl;
#endif
//        sendTerminationSignal();
        currentState = SimulationActorState::TERMINATED;
        stop();
    }
}

SimulationActor::~SimulationActor() {
    delete writer;   
} 

void SimulationActor::writeTimeStep(float currentTime) {
    writer->writeTimeStep(block.getWaterHeight(), 
            block.getDischarge_hu(),
            block.getDischarge_hv(),
            currentTime);
}

bool SimulationActor::mayRead() {
    bool res = true;
    for (int i = 0; i < 4; i++) {
        res &= (!this->dataIn[i] || this->dataIn[i]->available() > 0);
    }
    return res;
}

bool SimulationActor::mayWrite() {
    bool res = true;
    for (int i = 0; i < 4; i++) {
        res &= (!this->dataOut[i] || this->dataOut[i]->freeCapacity() > 0);
    }
    return res;
}

bool SimulationActor::hasReceivedTerminationSignal() {
    bool res = false;
    for (int i = 0; i < 4; i++) {
        if (this->dataIn[i] && this->dataIn[i]->available() > 0 && dataIn[i]->peek().empty()) {
            res |= true;
            dataIn[i]->read();
        }
    }
    return res;
}

void SimulationActor::sendData() {
    for (int i = 0; i < 4; i++) {
        if (this->dataOut[i]) {
            auto packedData = communicators[i].packCopyLayer();
            dataOut[i]->write(packedData);
        }
    }
}

void SimulationActor::sendTerminationSignal() {
    for (int i = 0; i < 4; i++) {
        if (this->dataOut[i] && this->dataOut[i]->freeCapacity() > 0) {
            dataOut[i]->write(std::vector<float>());
        }
    }
}

void SimulationActor::receiveData() {
    for (int i = 0; i < 4; i++) {
        if (this->dataIn[i]) {
            auto packedData = dataIn[i]->read();
            communicators[i].receiveGhostLayer(packedData);
        }
    }
}

void SimulationActor::computeWriteDelta() {
    auto numCheckpoints = config.numberOfCheckpoints;
    auto endTime = config.scenario->endSimulation();
    this->outputDelta = endTime / numCheckpoints;
    this->nextWriteTime = this->outputDelta;
}

uint64_t SimulationActor::getNumberOfPatchUpdates() {
    return patchUpdates;
}
