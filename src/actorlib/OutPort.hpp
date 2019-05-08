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

#include <memory>
#include <mutex>

#include <upcxx/upcxx.hpp>

#include "Channel.hpp"
#include "AbstractOutPort.hpp"
#include "AbstractInPort.hpp"
#include "ActorRegistration.hpp"

#pragma once

class ActorGraph;
class Actor;

template <typename T, int capacity>
class OutPort : public AbstractOutPort {

    friend class Actor;
    friend class ActorGraph;

    private: 
        upcxx::global_ptr<Channel<T, capacity>> remoteChannel;
        int unusedCapacity;
        std::mutex lock;
        
    public:
        void write(T element);
        size_t freeCapacity();
        void updateCapacity(size_t newVal);
        std::string toString();
        upcxx::future<> registerWithChannel();
        void setChannel(GlobalChannelRef newChannel);
    
    private:
        OutPort<T, capacity>(std::string name);
};

template <typename T, int capacity>
OutPort<T, capacity>::OutPort(std::string name)
    : AbstractOutPort(name),
      remoteChannel(nullptr),
      unusedCapacity(capacity) {
}

template <typename T, int capacity>
size_t OutPort<T, capacity>::freeCapacity() {
    std::lock_guard<std::mutex> writeLock(lock);
    return this->unusedCapacity;
}

template <typename T, int capacity>
void OutPort<T, capacity>::updateCapacity(size_t newVal) {
    std::lock_guard<std::mutex> writeLock(lock);
    //std::cout << "updating capacity to " << newVal << std::endl;
    unusedCapacity = newVal;
}

template <typename T, int capacity>
void OutPort<T, capacity>::setChannel(GlobalChannelRef newChannel) {
    this->remoteChannel = upcxx::reinterpret_pointer_cast<Channel<T, capacity>>(newChannel);
}

template <typename T, int capacity>
void OutPort<T, capacity>::write(T element) {
    // we need the rank to where write here, and memory information to deposit a one-sided communication
    std::lock_guard<std::mutex> writeLock(lock);
    if (remoteChannel == nullptr) {
        throw std::runtime_error(std::string("Unable to write to channel, channel not connected. Channel: ") 
                + std::to_string(remoteChannel.where()) + " " + std::to_string((size_t) remoteChannel.local()));
    }
    if (unusedCapacity == 0) {
        throw std::runtime_error("No free space in channel!");
    }
    this->unusedCapacity--;
    registerRpc(connectedActor);
    upcxx::rpc(
        remoteChannel.where(), 
        [](upcxx::global_ptr<Channel<T, capacity>> remoteChannel, T data) {
            remoteChannel.local()->enqueue(data);
            AbstractInPort *cip = remoteChannel.local()->connectedInPort.second;
            upcxx::persona *actorPersona = cip->actorPersona; 
            registerLpc(cip->connectedActor);
            actorPersona->lpc(
                [cip] () { 
                    cip->notify(); 
                }
            ).then([cip]() {
                deregisterLpc(cip->connectedActor);
            });
        }, 
        remoteChannel, 
        element
    ).then([this]() { 
        deregisterRpc(this->connectedActor); 
    });
}

template <typename T, int capacity>
std::string OutPort<T, capacity>::toString() {
    std::stringstream ss;
    ss << "[OP-" << capacity << " ID: " << name <<  " C: " << this->remoteChannel << "]"; 
    return ss.str();
}

template <typename T, int capacity>
upcxx::future<> OutPort<T, capacity>::registerWithChannel() {
    upcxx::intrank_t opRank = upcxx::rank_me();
    AbstractOutPort *cop = this;
    return upcxx::rpc(
        this->remoteChannel.where(),
        [](upcxx::global_ptr<Channel<T, capacity>> c, upcxx::intrank_t opRank, AbstractOutPort *cop) {
            c.local()->connectedOutPort = std::make_pair(opRank, cop);                
        },
        this->remoteChannel,
        opRank,
        cop
    );
}
