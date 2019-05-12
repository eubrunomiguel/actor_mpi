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

#include <string>
#include <thread>
#include <vector>

#include <upcxx/upcxx.hpp>

#pragma once

class ActorGraph;
class AbstractInPort;
class AbstractOutPort;
template <typename T, int capacity> class InPort;
template <typename T, int capacity> class OutPort;


class Actor {

    friend class ActorGraph;

    private:
        std::unordered_map<std::string, AbstractInPort *> inPorts;
        std::unordered_map<std::string, AbstractOutPort *> outPorts;
        std::thread actorThread;
        upcxx::persona *actorPersona;
        bool isRunning;
        std::atomic<int> triggerCount;

    public: 
        const std::string name;

    protected:
        template <typename T, int capacity> InPort<T, capacity> * makeInPort(std::string name);
        template <typename T, int capacity> OutPort<T, capacity> * makeOutPort(std::string name);
        void stop();

    public:
        Actor(const std::string &name);
        virtual ~Actor();
        Actor(Actor &other) = delete;
        Actor & operator=(Actor &other) = delete;
        std::string toString();
        AbstractOutPort * getOutPort(const std::string &name);
        AbstractInPort * getInPort(const std::string &name);
        void trigger();
        virtual void act() = 0;

    private:
        void start();
        void finishInitialization();
        void runLoop();

        friend void registerRpc(Actor *a);
        friend void deregisterRpc(Actor *a);
        friend void registerLpc(Actor *a);
        friend void deregisterLpc(Actor *a);
        friend void awaitGraphReadiness(Actor *a);
        friend void signalActorReadiness(Actor *a);
};

typedef upcxx::global_ptr<Actor *> GlobalActorRef;


// BEGIN IMPLEMENTATION

template <typename T, int capacity> 
InPort<T, capacity> * Actor::makeInPort(const std::string& name) {
    InPort<T, capacity> *ip = new InPort<T, capacity>(name);
    ip->connectedActor = this;
    inPorts[name] = ip;
    return ip;
}

template <typename T, int capacity> 
OutPort<T, capacity> * Actor::makeOutPort(const std::string& name) {
    OutPort<T, capacity> *op = new OutPort<T, capacity>(name);
    op->connectedActor = this;
    outPorts[name] = op;
    return op;
}
