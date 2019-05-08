/**
 * @file
 * This file is part of the actorlib sample collection.
 *
 * @author Alexander Pöppl (poeppl AT in.tum.de, https://www5.in.tum.de/wiki/index.php/Alexander_P%C3%B6ppl,_M.Sc.)
 *
 * @section LICENSE
 *
 * The actorlib sample collection is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The actorlib sample collection is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The actorlib sample collection.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 *
 */

#include "SourceActor.hpp"

#include "actorlib/OutPort.hpp"

#include <iostream>

using namespace std;



SourceActor::SourceActor(double minNumber, double maxNumber)
    : Actor("Source"),
      minNumber(minNumber),
      maxNumber(maxNumber),
      rd(),
      generator(rd()),
      dist(minNumber, maxNumber) {
    op = makeOutPort<double, 5>("out");
}

double SourceActor::getNext() {
    return dist(generator);
}

void SourceActor::act() {
    if (op->freeCapacity() > 0) {
        auto randomNumber = getNext();
        cout << "Generated number: <<" << randomNumber << ">>" << endl;
        op->write(randomNumber);
    }
}
