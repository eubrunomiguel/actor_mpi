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

#include "actorlib/Actor.hpp"

#include <random>

#pragma once

class SourceActor : public Actor {
    public:
        const double minNumber;
        const double maxNumber;
        OutPort<double, 5> * op;

    private:
        std::random_device rd;
        std::mt19937_64 generator;
        std::uniform_real_distribution<double> dist;

    public:
        SourceActor(double minNumber, double maxNumber);
        void act();

    private:
        double getNext();
};
