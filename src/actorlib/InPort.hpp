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
#include <string>
#include <sstream>
#include <mutex>

#include <upcxx/upcxx.hpp>

#include "Channel.hpp"
#include "AbstractInPort.hpp"
#include "AbstractOutPort.hpp"
#include "OutPort.hpp"
#include "ActorRegistration.hpp"

#pragma once

class Actor;

template <typename T, int capacity>
class InPort : public AbstractInPort {

    friend class Actor;

    private: 
        Channel<T, capacity> *connectedChannel;
        GlobalChannelRef opaqueChannelRef;
        std::mutex lock;

    public:
        static const int channelSize = capacity;
        T read();
        T peek();
        size_t available();
        std::string toString();
        void registerWithChannel();
        
    private:
        InPort<T, capacity>(std::string name);
        GlobalChannelRef getChannel();
};

template <typename T, int capacity>
InPort<T, capacity>::InPort(std::string name)
    : AbstractInPort(name),
      connectedChannel(nullptr),
      opaqueChannelRef(nullptr) {
}

template <typename T, int capacity>
size_t InPort<T, capacity>::available() {
    std::lock_guard<std::mutex> readLock(lock);
    if (connectedChannel != nullptr) {
        return connectedChannel->size();
    } else {
        throw std::runtime_error("Unable to get size, channel not connected.");
    }
}

template <typename T, int capacity>
T InPort<T, capacity>::peek() {
    std::lock_guard<std::mutex> readLock(lock);
    if (connectedChannel != nullptr) {
        return this->connectedChannel->peek();
    } else {
        throw std::runtime_error("Unable to get , channel not connected.");
    }
}

template <typename T, int capacity>
T InPort<T, capacity>::read() {
    std::lock_guard<std::mutex> readLock(lock);
    if (connectedChannel != nullptr) {
        auto cop = this->connectedChannel->connectedOutPort;
        registerRpc(this->connectedActor);
        upcxx::rpc(
            cop.first,
            [](AbstractOutPort *op, size_t newCapacity) {
                dynamic_cast<OutPort<T, capacity> *>(op)->updateCapacity(newCapacity);
                registerLpc(op->connectedActor);
                op->actorPersona->lpc(
                    [op]() {
                        op->notify();
                    }
                ).then([op]() {
                    deregisterLpc(op->connectedActor);
                });
            },
            cop.second,
            (capacity - (connectedChannel->size() + 1))
        ).then([this]() {
            deregisterRpc(connectedActor);
        });
        return this->connectedChannel->dequeue();
    } else {
        throw std::runtime_error("Unable to get size, channel not connected.");
    }
}

template <typename T, int capacity>
GlobalChannelRef InPort<T, capacity>::getChannel() {
    if (!connectedChannel) {
        auto channelRef = Channel<T, capacity>::createShared();
        this->opaqueChannelRef = upcxx::reinterpret_pointer_cast<void>(channelRef);
        this->connectedChannel = channelRef.local();

    }
    return this->opaqueChannelRef;
}

template <typename T, int capacity>
std::string InPort<T, capacity>::toString() {
    std::stringstream ss;
    ss << "[IP-" << capacity << " ID: " << name <<  " C: " << opaqueChannelRef << "]"; 
    return ss.str();
}

template <typename T, int capacity>
void InPort<T, capacity>::registerWithChannel() {
    this->connectedChannel->connectedInPort = std::make_pair(upcxx::rank_me(), this);
}

