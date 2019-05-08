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

#include <iostream>
#include <string>

#include <upcxx/upcxx.hpp>

#include "orchestration/ActorOrchestrator.hpp"

#include "util/Configuration.hpp"
#include "util/Logger.hh"

#include "block/SWE_Block.hh"

using namespace std;

void initLogger();

static tools::Logger &l = tools::Logger::logger;

int main(int argc, char **argv) {
    upcxx::init();
    {
        initLogger();
        auto config = Configuration::build(argc, argv, upcxx::rank_me());
        if (!upcxx::rank_me()) cout << config.toString(); 
        ActorOrchestrator orch(config);
        orch.initActorGraph();
        upcxx::barrier();
        orch.simulate();
    }
    upcxx::finalize();
}

void initLogger() {
    l.setProcessRank(upcxx::rank_me());
    l.printWelcomeMessage();
}
