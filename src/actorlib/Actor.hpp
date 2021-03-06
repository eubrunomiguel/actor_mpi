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

#include "InPort.hpp"
#include "OutPort.hpp"
#include <memory>
#include <string>
#include <unordered_map>

#pragma once

class ActorGraph;
class AbstractInPort;
class AbstractOutPort;

class Actor {

  friend class ActorGraph;

protected:
  template <typename T, int capacity>
  InPort<T, capacity> *makeInPort(std::string);

  template <typename T, int capacity>
  OutPort<T, capacity> *makeOutPort(std::string);

public:
  template <class T> Actor(T &&name) : name(std::forward<T>(name)) {}

  virtual ~Actor() = default;

  Actor(Actor &other) = delete;

  Actor &operator=(Actor &other) = delete;

  std::string toString() const;

  AbstractInPort *getInPort(const std::string &portName) const;

  AbstractOutPort *getOutPort(const std::string &portName) const;

  virtual bool act() = 0;

protected:
  std::string name;

private:
  std::unordered_map<std::string, AbstractInPort *> inPorts;
  std::unordered_map<std::string, AbstractOutPort *> outPorts;
};

template <typename T, int capacity>
InPort<T, capacity> *Actor::makeInPort(std::string portName) {
  auto ip = new InPort<T, capacity>(portName);
  inPorts.emplace(portName, ip);
  return ip;
}

template <typename T, int capacity>
OutPort<T, capacity> *Actor::makeOutPort(std::string portName) {
  auto op = new OutPort<T, capacity>(portName);
  outPorts.emplace(portName, op);
  return op;
}
