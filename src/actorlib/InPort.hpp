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
#include <string>

#include "AbstractInPort.hpp"
#include "Port.h"

#pragma once

class Actor;
class AbstractOutPort;

template <typename T, int capacity> class InPort : public AbstractInPort {

  friend class Actor;

private:
  Channel<T, capacity> myChannel;

  std::unique_ptr<PortIdentification<AbstractOutPort>> otherPort;
  std::array<std::pair<std::unique_ptr<MPI_Request>, T>, capacity> requests;

public:
  T read(int elementCount);
  T peek() const;
  size_t available() const;
  std::string toString() const final;
  void receiveMessagesFrom(
      std::unique_ptr<PortIdentification<AbstractOutPort>>) final;

private:
  explicit InPort<T, capacity>(const std::string &name);
  void *getChannel() const final;
};
