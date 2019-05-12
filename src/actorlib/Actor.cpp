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

#include "Actor.hpp"

#include "InPort.hpp"
#include "OutPort.hpp"
#include "utils/config.hpp"

#include "ActorGraph.hpp"

#include <string>
#include <sstream>

//#define DEBUG_ACTOR_TERMINATION
#define ACTOR_PARALLEL_TERMINATION_HACK

using namespace std;
using namespace std::string_literals;

Actor::Actor(const string &name)
    : actorPersona(nullptr),
      isRunning(false),
      triggerCount(0),
      name(name) {
}

Actor::~Actor() {
    for (auto aip : this->inPorts) {
        delete aip.second;
    }
    for (auto aop : this->outPorts) {
        delete aop.second;
    }
}

std::string Actor::toString() {
    stringstream ss;
    ss << "Actor( " << name << " ) { ";
    for (auto &aip : this->inPorts) {
        ss << aip.second->toString() << " ";
    }
    for (auto &aop : this->outPorts) {
        ss << aop.second->toString() << " ";
    }
    ss << "}";
    return ss.str();
}

AbstractInPort * Actor::getInPort(const string &name) {
    auto res = inPorts.find(name);
    if (res != inPorts.end()) {
        return res->second;
    } else {
        throw std::runtime_error("Actor "s + this->toString() + "has no InPort with id "s + name);
    }
}

AbstractOutPort * Actor::getOutPort(const string &name) {
    auto res = outPorts.find(name);
    if (res != outPorts.end()) {
        return res->second;
    } else {
        throw std::runtime_error("Actor "s + this->toString() + "has no OutPort with name "s + name);
    }
}

void Actor::start() {
    finishInitialization();
}

void Actor::stop() {
#ifdef DEBUG_ACTOR_TERMINATION
    std::cout << " N: " << this->name << "\treceived Stop signal. " << std::endl;
#endif
    this->isRunning = false;
}

void Actor::runLoop() {
    while(isRunning || triggerCount.load() > 0) {
        upcxx::progress();
        if (triggerCount.load() > 0) {
            triggerCount--;
            act();
        }
    }
    
    do {
#ifdef DEBUG_ACTOR_TERMINATION
        //std::cout << "RPCs: " << parentActorGraph->rpcsInFlight.load() << " LPCs: " << parentActorGraph->lpcsInFlight.load() << " N: " << this->name << std::endl;
#endif
        upcxx::progress();
    } while (parentActorGraph->rpcsInFlight.load() > 0);
    this->parentActorGraph->activeActors--;
#ifdef DEBUG_ACTOR_TERMINATION
    std::cout << " N: " << this->name << "\tawaiting Termination signal. " << std::endl;
#endif
    awaitActorGraphTermination(parentActorGraph);
    do {
#ifdef DEBUG_ACTOR_TERMINATION
        std::cout << "Post Signal: RPCs: " << parentActorGraph->rpcsInFlight.load() << " LPCs: " << parentActorGraph->lpcsInFlight.load() << " N: " << this->name << std::endl;
#endif
        upcxx::progress();
#ifdef ACTOR_PARALLEL_TERMINATION_HACK
        std::this_thread::sleep_for(1s);
#endif
    } while (parentActorGraph->rpcsInFlight.load() > 0 || parentActorGraph->lpcsInFlight.load() > 0);
#ifdef DEBUG_ACTOR_TERMINATION
    std::cout << this->name << " done spinning" << std::endl;
#endif
}

void Actor::trigger() {
    parentActorGraph->markAsDirty(this);
}

void Actor::finishInitialization() {
    this->actorPersona = &upcxx::current_persona();
    this->isRunning = true;

    for (auto ip : this->inPorts) {
        ip.second->setActorPersona(this->actorPersona);
    }
    for (auto op : this->outPorts) {
        op.second->setActorPersona(this->actorPersona);
    }
}
