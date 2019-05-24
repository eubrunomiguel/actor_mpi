/**
 * @file
 * This file is part of actorlib.
 *
 * @author Alexander Pöppl (poeppl AT in.tum.de,
 * https://www5.in.tum.de/wiki/index.php/Alexander_P%C3%B6ppl,_M.Sc.)
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
#include <sstream>
#include <string>

#include "AbstractInPort.hpp"
#include "AbstractOutPort.hpp"
#include "Channel.hpp"
#include "utils/mpi_helper.hpp"

#pragma once

class Actor;
class AbstractOutPort;

template <typename T, int capacity> class InPort : public AbstractInPort {

  friend class Actor;

public:
  template <class str>
  explicit InPort<T, capacity>(str &&name)
      : AbstractInPort(std::forward<str>(name)),
        otherPortIdentification(nullptr) {}

  InPort<T, capacity>(InPort<T, capacity> const &) = delete;
  InPort<T, capacity>(InPort<T, capacity> &&) = delete;
  InPort<T, capacity> &operator=(InPort<T, capacity> const &) = delete;
  InPort<T, capacity> &operator=(InPort<T, capacity> &&) = delete;

  T read();

  T peek() const;

  size_t available() const;

  std::string toString() const final;

  void receiveMessagesFrom(
      PortIdentification<AbstractOutPort> portIdentification) final {
    otherPortIdentification = portIdentification;
  }

  void *getChannel() const final;

private:
  void recycleRequests();
  void openRequests();

  Channel<T, capacity> myChannel;
  PortIdentification<AbstractOutPort> otherPortIdentification;
  std::array<mpi::ReceiveRequest<T>, capacity> requests;
};

template <typename T, int capacity>
size_t InPort<T, capacity>::available() const {
  return myChannel.available();
}

template <typename T, int capacity> T InPort<T, capacity>::peek() const {
  return myChannel.peek();
}

template <typename T, int capacity>
void InPort<T, capacity>::recycleRequests() {
  for (auto &request : requests) {
    if (request.isDirty()) {
      if (request.hasFinished()) {
        myChannel.returnElement(request.getBuffer());
        request = mpi::ReceiveRequest<T>();
      }
    }
  }
}

template <typename T, int capacity> void InPort<T, capacity>::openRequests() {
  for (auto &request : requests) {
    if (!request.isDirty())
      request = mpi::Irecv<T>(otherPortIdentification.getRank(),
                              myIdentification.getTag(), myChannel.reserve());
  }
}

template <typename T, int capacity> T InPort<T, capacity>::read() {
  if (!otherPortIdentification.isConnected())
    throw std::runtime_error(
        std::string("Unable to read from channel, channel not connected."));

  if (otherPortIdentification.isExternal()) {
    recycleRequests();
    openRequests();
  }

  return myChannel.getNext();
}

template <typename T, int capacity>
void *InPort<T, capacity>::getChannel() const {
  return (void *)(&this->myChannel);
}

template <typename T, int capacity>
std::string InPort<T, capacity>::toString() const {
  std::stringstream ss;
  ss << "[IP-" << capacity << " ID: " << myIdentification.getName() << "]";
  return ss.str();
}