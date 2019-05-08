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

#include "actorlib/ActorGraph.hpp"
#include "actorlib/Actor.hpp"


using namespace std;
using namespace std::string_literals;

std::string getNext(int myActorIdx, int actorsPerPlace) {
    if (myActorIdx == actorsPerPlace - 1) {
        return "A-"s + to_string((upcxx::rank_me() + 1) % upcxx::rank_n()) + "-0"s;
    } else {
        return "A-"s + to_string(upcxx::rank_me()) + "-"s + to_string(myActorIdx + 1);
    }
}

std::string getName(int actorIdx) {
    return "A-"s + to_string(upcxx::rank_me()) + "-"s + to_string(actorIdx);
}

int main(int argc, const char **argv) {
    MPI_Init(NULL, NULL);
    upcxx::init();

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    ActorGraph ag;
    Actor * meActor = new PingPongActor("A-"s + to_string(world_rank));
    ag.addActor(meActor);
    ag.synchronizeActors();

    MPI_Barrier(MPI_COMM_WORLD);

    int otherRank = world_rank == 1 ? 0 : 1;

    ag.connectPorts(meActor, PingPongActor::OUT_PORT_NAME, otherActor, PingPongActor::IN_PORT_NAME);

    //    ag.run();

    upcxx::finalize();
    MPI_Finalize();
}

