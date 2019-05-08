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
 */

#include "AbstractInPort.hpp"
#include "Actor.hpp"

using namespace std;

AbstractInPort::AbstractInPort(std::string name)
    : name(name),
      connectedActor(nullptr),
      actorPersona(nullptr) {
}

AbstractInPort::~AbstractInPort() = default;

void AbstractInPort::setActorPersona(upcxx::persona *actorPersona) {
    this->actorPersona = actorPersona;
}

void AbstractInPort::notify() {
    if (!connectedActor) {
        throw std::runtime_error("Connected actor of port "s + name + " is NULL!");
    } else {
        this->connectedActor->trigger();
    }
}
