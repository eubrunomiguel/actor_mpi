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
    int actorsPerPlace = 1;
    if (argc == 2) {
        actorsPerPlace = std::stoi(argv[1]);
        if (actorsPerPlace < 1) {
            actorsPerPlace = 1;
        }
    }
    upcxx::init();
    ActorGraph ag;
    for (int i = 0; i < actorsPerPlace; i++) {
        Actor * a = new PingPongActor("A-"s + to_string(upcxx::rank_me()) + "-"s + to_string(i));
        ag.addActor(a);
    }
    upcxx::barrier();

    for (int i = 0; i < actorsPerPlace; i++) {
        auto actorName = getName(i);
        auto nextActorName = getNext(i, actorsPerPlace);
        cout << "Connect " << actorName << " -> " << nextActorName << endl;
        auto actor = ag.getActor(actorName);
        auto nextActor = ag.getActor(nextActorName);
        ag.connectPorts(actor, PingPongActor::OUT_PORT_NAME, nextActor, PingPongActor::IN_PORT_NAME);
    }
    
    ag.run();

    upcxx::finalize();
}

