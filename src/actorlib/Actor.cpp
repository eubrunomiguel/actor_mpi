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

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "Actor.hpp"
#include "ActorGraph.hpp"
#include "InPort.hpp"
#include "OutPort.hpp"

using namespace std;
using namespace std::string_literals;

Actor::Actor(const string &name) : name(name) {}

template <typename T, int capacity>
const std::shared_ptr<InPort<T, capacity>>
Actor::makeInPort(const std::string &portName) {
  auto ip = std::make_shared<InPort<T, capacity>>(portName);
  inPorts.emplace(portName, ip);
  return ip;
}

template <typename T, int capacity>
const std::shared_ptr<OutPort<T, capacity>>
Actor::makeOutPort(const std::string &portName) {
  auto op = std::make_shared<OutPort<T, capacity>>(portName);
  outPorts.emplace(portName, op);
  return op;
}

std::string Actor::toString() const {
  stringstream ss;
  ss << "Actor( " << name << " ) { ";
  for (auto &aip : this->inPorts) {
    ss << aip.second->toString() << " ";
  }
  for (auto &aop : this->outPorts) {
    ss << aop.second->toString() << " ";
  }
  ss << "}";
  return ss.str();
}

const std::shared_ptr<AbstractInPort>
Actor::getInPort(const string &portName) const {
  auto res = inPorts.find(portName);
  if (res != inPorts.end()) {
    return res->second;
  } else {
    throw std::runtime_error("Actor "s + this->toString() +
                             "has no InPort with id "s + portName);
  }
}

const std::shared_ptr<AbstractOutPort>
Actor::getOutPort(const string &portName) const {
  auto res = outPorts.find(name);
  if (res != outPorts.end()) {
    return res->second;
  } else {
    throw std::runtime_error("Actor "s + this->toString() +
                             "has no OutPort with name "s + name);
  }
}