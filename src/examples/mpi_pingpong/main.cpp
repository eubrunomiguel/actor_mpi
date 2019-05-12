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

#include <string>

#include <upcxx/upcxx.hpp>
#include <mpi.h>

#include "PingPongActor.hpp"

#include "actorlib/Actor.hpp"
#include "actorlib/ActorGraph.hpp"
#include "actorlib/utils/mpi_helper.hpp"

using namespace std;
using namespace std::string_literals;

std::string getActorName(MPIHelper::RankId actorIdx) {
    return "A-"s + to_string(actorIdx);
}

int main(int argc, const char **argv) {
    MPI_Init(NULL, NULL);
    upcxx::init();

    ActorGraph ag;
    const string myActorName = getActorName(MPIHelper::myRank());
    Actor * myActor = new PingPongActor(myActorName);
    ag.addLocalActor(myActor);
    ag.synchronizeActors();

    MPI_Barrier(MPI_COMM_WORLD);

    MPIHelper::RankId otherActorRank = MPIHelper::myRank() == 1 ? 0 : 1;

    ag.connectPorts(myActorName, PingPongActor::OUT_PORT_NAME, getActorName(otherActorRank), PingPongActor::IN_PORT_NAME);

    ag.run();

    upcxx::finalize();
    MPI_Finalize();
}

