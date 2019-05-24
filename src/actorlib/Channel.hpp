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

#include <array>
#include <iostream>
#include <memory>
#include <queue>

#pragma once

class AbstractInPort;

class AbstractOutPort;

template <typename T, int capacity> class Channel {

private:
  int lastElement;
  int firstElement;
  bool isFull;

  std::array<T, capacity> buffer;

  std::queue<T *> elements;
  std::queue<T *> freeSpace;

public:
  Channel();

  T *reserve();

  void returnElement(T *);

  T getNext();

  T peek() const;

  size_t available() const;
};

template <typename T, int capacity>
Channel<T, capacity>::Channel()
    : lastElement(0), firstElement(0), isFull(false) {
  for (auto &el : buffer) {
    freeSpace.push(&el);
  }
}

template <typename T, int capacity>
void Channel<T, capacity>::returnElement(T *element) {
  if (elements.size() >= capacity)
    throw std::runtime_error("Cannot emplace a previously reserved element.");

  elements.push(element);
}

template <typename T, int capacity> T *Channel<T, capacity>::reserve() {
  if (freeSpace.empty())
    throw std::runtime_error("Channel is full");

  T *firstFree = freeSpace.front();
  freeSpace.pop();
  return firstFree;
}

template <typename T, int capacity> T Channel<T, capacity>::getNext() {
  if (elements.empty())
    throw std::runtime_error("Channel is empty");

  T *elementAddress = elements.front();
  T element = *elements.front();

  elements.pop();
  freeSpace.push(elementAddress);

  return element;
}

template <typename T, int capacity> T Channel<T, capacity>::peek() const {
  if (elements.empty())
    throw std::runtime_error("Channel is empty");

  return *elements.front();
}

template <typename T, int capacity>
size_t Channel<T, capacity>::available() const {
  return elements.size();
}
