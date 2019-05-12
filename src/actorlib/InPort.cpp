//
// Created by Bruno Macedo Miguel on 2019-05-11.
//

#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#include "../../backups/ActorRegistration.hpp"
#include "AbstractInPort.hpp"
#include "AbstractOutPort.hpp"
#include "Channel.hpp"
#include "InPort.hpp"
#include "OutPort.hpp"
#include "mpi.h"
#include "utils/mpi_helper.hpp"

template <typename T, int capacity>
InPort<T, capacity>::InPort(const std::string &name)
    : AbstractInPort(name), otherPort(nullptr) {}

template <typename T, int capacity>
size_t InPort<T, capacity>::available() const {
  return myChannel.size();
}

template <typename T, int capacity> T InPort<T, capacity>::peek() const {
  return myChannel.peek();
}

template <typename T, int capacity>
T InPort<T, capacity>::read(int elementCount) {
  if (!otherPort->isConnected())
    throw std::runtime_error(
        std::string("Unable to read from channel, channel not connected."));

  if (otherPort->isExternal()) {
    // Todo: ideally use buffer direct in channel. It is work. It will not
    // conflict, because I either have internal or external, there is no
    // conflict with capacity
    for (auto request : requests) {
      if (request.first) {
        // Receive
        if (MPIHelper::hasCompleted(request.first)) {
          myChannel.enqueue(request.second);
          request.first = nullptr;
        }
      } else {
        // Listen
        request.first =
            MPIHelper::IRecv(&request.second, elementCount, otherPort->rankId,
                             myIdentification.tagIdentification);
      }
    }
  }

  return myChannel.dequeue();
}

template <typename T, int capacity>
void *InPort<T, capacity>::getChannel() const {
  return (void *)(&this->myChannel);
}

template <typename T, int capacity>
std::string InPort<T, capacity>::toString() const {
  std::stringstream ss;
  ss << "[IP-" << capacity << " ID: " << myIdentification.portName << "]";
  return ss.str();
}

template <typename T, int capacity>
void InPort<T, capacity>::receiveMessagesFrom(
    std::unique_ptr<PortIdentification<AbstractOutPort>> portIdentification) {
  otherPort = std::move(portIdentification);
}