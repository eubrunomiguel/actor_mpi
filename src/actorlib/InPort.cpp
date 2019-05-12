//
// Created by Bruno Macedo Miguel on 2019-05-11.
//

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
#include "utils/mpi_helper.hpp"
#include "mpi.h"
#include "InPort.hpp"

template<typename T, int capacity>
InPort<T, capacity>::InPort(const std::string &name)
        : AbstractInPort(name),
          receiveFromRank(-1),
          receiveFromOutPort(nullptr) {
}

template<typename T, int capacity>
size_t InPort<T, capacity>::available() {
    std::lock_guard<std::mutex> readLock(lock);
    if (connectedChannel != nullptr) {
        return connectedChannel->size();
    } else {
        throw std::runtime_error("Unable to get size, channel not connected.");
    }
}

template<typename T, int capacity>
T InPort<T, capacity>::peek() {
    std::lock_guard<std::mutex> readLock(lock);
    if (connectedChannel != nullptr) {
        return this->connectedChannel->peek();
    } else {
        throw std::runtime_error("Unable to get , channel not connected.");
    }
}

template<typename T, int capacity>
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

template<typename T, int capacity>
void *InPort<T, capacity>::getChannel() {
    return (void *) (&this->myChannel);
}

template<typename T, int capacity>
std::string InPort<T, capacity>::toString() {
    std::stringstream ss;
    ss << "[IP-" << capacity << " ID: " << name << "]";
    return ss.str();
}

template<typename T, int capacity>
void InPort<T, capacity>::registerWithChannel() {
    this->connectedChannel->connectedInPort = std::make_pair(upcxx::rank_me(), this);
}

template<typename T, int capacity>
void InPort<T, capacity>::receiveMessagesFrom(std::unique_ptr<PortIdentification<AbstractOutPort>> portIdentification){
    otherPort = std::move(portIdentification);
}