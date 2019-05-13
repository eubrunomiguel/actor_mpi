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
 */

#include "PortIdentification.h"
#include "utils/mpi_helper.hpp"

#pragma once

class AbstractOutPort;

class AbstractInPort {
public:
  const PortIdentification<AbstractInPort> myIdentification;

public:
  virtual void *getChannel() const = 0;

  virtual std::string toString() const = 0;

  virtual void receiveMessagesFrom(PortIdentification<AbstractOutPort>) = 0;

  explicit AbstractInPort(const std::string &name)
      : myIdentification(name, MPIHelper::myRank()) {}

  virtual ~AbstractInPort() = default;
};
