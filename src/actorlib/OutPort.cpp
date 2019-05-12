//
// Created by Bruno Macedo Miguel on 2019-05-11.
//

#include <memory>
#include <mutex>
#include "OutPort.hpp"
#include "AbstractInPort.hpp"
#include "AbstractOutPort.hpp"
#include "ActorRegistration.hpp"
#include "Channel.hpp"
#include "mpi.h"
#include "utils/mpi_helper.hpp"


template<typename T, int capacity>
OutPort<T, capacity>::OutPort(const std::string& name)
        : AbstractOutPort(name) {}

template<typename T, int capacity>
size_t OutPort<T, capacity>::freeCapacity() const {
    std::lock_guard<std::mutex> writeLock(lock);
    return std::count_if(requests.begin(), requests.end(), [](const auto request){return request.first == nullptr;});
}

template<typename T, int capacity>
void OutPort<T, capacity>::write(const T &element) {
    std::lock_guard<std::mutex> writeLock(lock);

    if (freeCapacity() == 0)
        throw std::runtime_error("No free space in channel!");

    if (!otherPort->isConnected())
        throw std::runtime_error(std::string("Unable to write to channel, channel not connected.");

    if (otherPort->isLocal()) {
        writeToLocal(element);
    } else {
        writeToExternal(element);
    }
}

template<typename T, int capacity>
void OutPort<T, capacity>::writeToLocal(const T &element) {
    auto channel = dynamic_cast<Channel<T, capacity> *>(sendToInPort->getChannel());
    channel->enqueue(element);
    // notify the actor from the inPort that has value
}

template<typename T, int capacity>
void OutPort<T, capacity>::writeToExternal(const T &element) {
    auto firstFreeRequestIt = find_if(requests.begin(), requests.end(),
                                      [](const auto request) { return request.first == nullptr; });
    if (firstFreeRequestIt != requests.end()) {
        firstFreeRequestIt->second = element;
        firstFreeRequestIt->first = MPIHelper::ISend<T>(
                firstFreeRequestIt->second,
                otherPort->rankId,
                otherPort->tagIdentification);
    }
}

template<typename T, int capacity>
std::string OutPort<T, capacity>::toString() const {
    std::stringstream ss;
    ss << "[OP-" << capacity << " ID: " << name << " C: " << this->remoteChannel << "]";
    return ss.str();
}

template<typename T, int capacity>
void OutPort<T, capacity>::sendMessagesTo(std::unique_ptr<PortIdentification<AbstractInPort>> portIdentification) {
    otherPort = std::move(portIdentification);
}
