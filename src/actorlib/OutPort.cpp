//
// Created by Bruno Macedo Miguel on 2019-05-11.
//

#include "OutPort.hpp"
#include "AbstractInPort.hpp"
#include "AbstractOutPort.hpp"
#include "Channel.hpp"
#include "mpi.h"
#include "utils/mpi_helper.hpp"
#include <memory>
#include <mutex>

template <typename T, int capacity>
OutPort<T, capacity>::OutPort(const std::string &name)
    : AbstractOutPort(name), otherPort(nullptr) {}

template <typename T, int capacity>
size_t OutPort<T, capacity>::freeCapacity() const {
  std::lock_guard<std::mutex> writeLock(lock);
  return std::count_if(
      requests.begin(), requests.end(),
      [](const auto request) { return request.first == nullptr; });
}

template <typename T, int capacity>
void OutPort<T, capacity>::write(const T &element, int elementCount) {
  std::lock_guard<std::mutex> writeLock(lock);

  if (freeCapacity() == 0)
    throw std::runtime_error("No free space in channel!");

  if (!otherPort->isConnected())
        throw std::runtime_error(std::string("Unable to write to channel, channel not connected.");

    if (otherPort->isLocal()) {
      writeToLocal(element);
    } else {
      writeToExternal(element, elementCount);
    }
}

template <typename T, int capacity>
void OutPort<T, capacity>::writeToLocal(const T &element) {
  auto channel =
      dynamic_cast<Channel<T, capacity> *>(otherPort->port->getChannel());
  channel->enqueue(element);
  // notify the actor from the inPort that has value
}

template <typename T, int capacity>
void OutPort<T, capacity>::writeToExternal(const T &element, int elementCount) {
  for (auto request : requests) {
    if (request.first) {
      if (MPIHelper::hasCompleted(request.first))
        request.first = nullptr;
    }
  }

  auto firstFreeRequestIt =
      find_if(requests.begin(), requests.end(),
              [](const auto request) { return request.first == nullptr; });
  if (firstFreeRequestIt != requests.end()) {
    firstFreeRequestIt->second = element;
    firstFreeRequestIt->first =
        MPIHelper::ISend<T>(firstFreeRequestIt->second, elementCount,
                            otherPort->rankId, otherPort->tagIdentification);
  }
}

template <typename T, int capacity>
std::string OutPort<T, capacity>::toString() const {
  std::stringstream ss;
  ss << "[OP-" << capacity << " ID: " << myIdentification.portName << "]";
  return ss.str();
}

template <typename T, int capacity>
void OutPort<T, capacity>::sendMessagesTo(
    std::unique_ptr<PortIdentification<AbstractInPort>> portIdentification) {
  otherPort = std::move(portIdentification);
}
