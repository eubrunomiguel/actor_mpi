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

#include "AbstractInPort.hpp"
#include "AbstractOutPort.hpp"
#include "Channel.hpp"
#include <algorithm>
#include <memory>

#pragma once

class ActorGraph;
class Actor;
class AbstractInPort;

template <typename T, int capacity> class OutPort : public AbstractOutPort {

  friend class Actor;
  friend class ActorGraph;

private:
  PortIdentification<AbstractInPort> otherPortIdentification;
  std::array<mpi::SendRequest<T>, capacity> requests;

public:
  template <class str>
  explicit OutPort<T, capacity>(str &&name)
      : AbstractOutPort(std::forward<str>(name)),
        otherPortIdentification(nullptr) {}

  OutPort<T, capacity>(OutPort<T, capacity> const &) = delete;
  OutPort<T, capacity>(OutPort<T, capacity> &&) = delete;
  OutPort<T, capacity> &operator=(OutPort<T, capacity> const &) = delete;
  OutPort<T, capacity> &operator=(OutPort<T, capacity> &&) = delete;

  void write(const T &);
  void write(T &&);

  size_t freeCapacity() const;

  std::string toString() const final;

  void
  sendMessagesTo(PortIdentification<AbstractInPort> portIdentification) final {
    otherPortIdentification = portIdentification;
  }

private:
  void recycleRequests();

  void preWrite() const;

  void writeToLocal(const T &element);
  void writeToLocal(T &&element);

  void writeToExternal(const T &element);
  void writeToExternal(T &&element);
};

template <typename T, int capacity>
size_t OutPort<T, capacity>::freeCapacity() const {
  return std::count_if(requests.begin(), requests.end(),
                       [](const auto &request) { return !request.isDirty(); });
}

template <typename T, int capacity>
void OutPort<T, capacity>::recycleRequests() {
  for (auto &request : requests) {
    if (request.isDirty()) {
      if (request.hasFinished())
        request = mpi::SendRequest<T>();
    }
  }
}

template <typename T, int capacity>
void OutPort<T, capacity>::preWrite() const {
  if (freeCapacity() == 0)
    throw std::runtime_error("No free space in channel!");

  if (!otherPortIdentification.isConnected())
    throw std::runtime_error(
        "Unable to write to channel, channel not connected.");
}

template <typename T, int capacity>
void OutPort<T, capacity>::write(const T &element) {
  preWrite();

  if (otherPortIdentification.isLocal()) {
    writeToLocal(element);
  } else {
    writeToExternal(element);
  }
}
template <typename T, int capacity>
void OutPort<T, capacity>::write(T &&element) {
  preWrite();

  if (otherPortIdentification.isLocal()) {
    writeToLocal(std::forward(element));
  } else {
    writeToExternal(std::forward(element));
  }
}

template <typename T, int capacity>
void OutPort<T, capacity>::writeToLocal(const T &element) {
  auto channel = static_cast<Channel<T, capacity> *>(
      otherPortIdentification.getPort()->getChannel());
  T *freeSpace = channel->reserve();
  *freeSpace = element;
  channel->returnElement(freeSpace);
}

template <typename T, int capacity>
void OutPort<T, capacity>::writeToLocal(T &&element) {
  auto channel = static_cast<Channel<T, capacity> *>(
      otherPortIdentification.getPort()->getChannel());
  T *freeSpace = channel->reserve();
  *freeSpace = std::move(element);
  channel->returnElement(freeSpace);
}

template <typename T, int capacity>
void OutPort<T, capacity>::writeToExternal(const T &element) {
  recycleRequests();

  for (auto &request : requests) {
    if (!request.isDirty()) {
      request = mpi::SendRequest<T>(otherPortIdentification.getRank(),
                                    otherPortIdentification.getTag(), element);
      break;
    }
  }
}

template <typename T, int capacity>
void OutPort<T, capacity>::writeToExternal(T &&element) {
  recycleRequests();

  auto &freeRequest =
      find_if(requests.begin(), requests.end(),
              [](const auto &request) { return !request.isDirty(); });

  freeRequest = mpi::SendRequest<T>(otherPortIdentification.getRank(),
                                    otherPortIdentification.getTag(),
                                    std::forward(element));
}

template <typename T, int capacity>
std::string OutPort<T, capacity>::toString() const {
  std::stringstream ss;
  ss << "[OP-" << capacity << " ID: " << myIdentification.getName() << "]";
  return ss.str();
}