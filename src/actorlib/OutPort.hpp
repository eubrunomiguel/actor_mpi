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

#include "AbstractOutPort.hpp"
#include "Port.h"
#include "utils/mpi_helper.hpp"
#include <memory>
#include <mutex>

#pragma once

class ActorGraph;
class Actor;
class AbstractInPort;

template<typename T, int capacity>
class OutPort : public AbstractOutPort {

    friend class Actor;
    friend class ActorGraph;

private:
    std::mutex lock;

    std::unique_ptr<PortIdentification<AbstractInPort>> otherPort;
    std::array<std::pair<std::unique_ptr<MPI_Request>, T>, capacity> requests;

public:
    void write(const T &);
    size_t freeCapacity() const;
    std::string toString() const final;
    void sendMessagesTo(std::unique_ptr<PortIdentification<AbstractInPort>>) final;

private:
    OutPort<T, capacity>(const std::string& name);
    void writeToLocal(const T &element);
    void writeToExternal(const T &element);
};