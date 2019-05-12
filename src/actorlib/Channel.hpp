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

#include <array>
#include <iostream>
#include <memory>
#include <mutex>

#pragma once

class AbstractInPort;
class AbstractOutPort;

template <typename T, int capacity>
class Channel {

private:
    int lastElement;
    int firstElement;
    bool isFull;
    std::array<T, capacity> queue;
    std::mutex lock;

public:
    Channel();
    int enqueue(T element);
    T dequeue();
    T peek();
    size_t size();

private:
    size_t sizeInt();
};

template <typename type, int capacity>
Channel<type, capacity>::Channel()
        : lastElement(0),
          firstElement(0),
          isFull(false) {
}

template <typename type, int capacity>
int Channel<type, capacity>::enqueue(type element) {
    std::lock_guard<std::mutex> writeLock(lock);
    if ((lastElement - firstElement) % capacity == 0 && isFull) {
        throw std::runtime_error("Channel is full");
    } else {
        lastElement = (lastElement + 1) % capacity;
        isFull = (lastElement == firstElement);
        queue[lastElement] = element;
        return this->sizeInt();
    }
}

template <typename type, int capacity>
type Channel<type, capacity>::peek() {
    std::lock_guard<std::mutex> writeLock(lock);
    if (lastElement == firstElement && !isFull) {
        throw std::runtime_error("Channel is empty");
    } else {
        size_t elemPos = (firstElement + 1) % capacity;
        type element = queue[elemPos];
        return element;
    }
}

template <typename type, int capacity>
type Channel<type, capacity>::dequeue() {
    std::lock_guard<std::mutex> writeLock(lock);
    if (lastElement == firstElement && !isFull) {
        throw std::runtime_error("Channel is empty");
    } else {
        firstElement = (firstElement + 1) % capacity;
        type element = queue[firstElement];
        isFull = false;
        return element;
    }
}

template <typename type, int capacity>
size_t Channel<type, capacity>::sizeInt() {
    int tmpLast = lastElement;
    if (lastElement < firstElement || isFull) {
        tmpLast += capacity;
    }
    return tmpLast - firstElement;
}

template <typename type, int capacity>
size_t Channel<type, capacity>::size() {
    std::lock_guard<std::mutex> readLock(lock);
    int tmpLast = lastElement;
    if (lastElement < firstElement || isFull) {
        tmpLast += capacity;
    }
    return tmpLast - firstElement;
}

