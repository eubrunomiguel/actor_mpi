/**
 * @file
 * This file is part of Pond.
 *
 * @author Alexander Pöppl (poeppl AT in.tum.de, https://www5.in.tum.de/wiki/index.php/Alexander_P%C3%B6ppl,_M.Sc.)
 *
 * @section LICENSE
 *
 * Pond is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pond is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pond.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 *
 */
#include <string>

#include "scenario/SWE_Scenario.hh"

#pragma once
struct Configuration {
    const size_t xSize;
    const size_t ySize;
    const size_t patchSize;
    const size_t numberOfCheckpoints;
    const std::string fileNameBase;
    const Scenario *scenario;
    const float dx;
    const float dy;

    Configuration(size_t xSize, size_t ySize, size_t patchSize, size_t numberOfCheckpoints, std::string fileNameBase, Scenario *scenario);
    std::string toString();

    static Configuration build(int argc, char **argv, size_t rank);
};
