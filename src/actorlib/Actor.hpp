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
#include <string>
#include <unordered_map>

#pragma once

class ActorGraph;
class AbstractInPort;
class AbstractOutPort;
template <typename T, int capacity> class InPort;
template <typename T, int capacity> class OutPort;

class Actor {

  friend class ActorGraph;

private:
  std::unordered_map<std::string, std::shared_ptr<AbstractInPort>> inPorts;
  std::unordered_map<std::string, std::shared_ptr<AbstractOutPort>> outPorts;

public:
  const std::string name;

protected:
  template <typename T, int capacity>
  const std::shared_ptr<InPort<T, capacity>> makeInPort(const std::string &);
  template <typename T, int capacity>
  const std::shared_ptr<OutPort<T, capacity>> makeOutPort(const std::string &);

public:
  Actor(const std::string &name);
  virtual ~Actor() = default;
  Actor(Actor &other) = delete;
  Actor &operator=(Actor &other) = delete;
  std::string toString() const;
  std::shared_ptr<AbstractInPort> getInPort(const std::string &portName) const;
  std::shared_ptr<AbstractOutPort>
  getOutPort(const std::string &portName) const;
  virtual bool act() = 0;
};