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

#include <iostream>
#include <string>

#include "actorlib/ActorGraph.hpp"
#include "actorlib/Channel.hpp"
#include "actorlib/Actor.hpp"

using namespace std;
using namespace std::string_literals;

namespace test {

bool channelTest(bool verbose) {
    Channel<int, 5> c;
    for (int i = 0; i < 5; i++) {
        c.enqueue((i + 1) * 10);
        if (verbose) {
            cout << "[E: " << ((i + 1) * 10) << ", " << c.size() << "] ";
        }
    }
    
    if (verbose) {
        cout << endl;
    }
    
    for (int i = 0; i < 5; i++) {
        auto peekedElem = c.peek();
        auto elem = c.dequeue();
        if (peekedElem != (i+1) * 10) {
            cout << "Peeked element mismatch, expected " << ((i+1) * 10) << " but got " << peekedElem << endl;
            return false;
        }

        if (peekedElem != elem) {
            cout << "Peeked element + Popped element mismatch, expected " << peekedElem << " != " << elem << endl;
            return false;
        }
        
        if (elem != (i + 1) * 10) {
            cout << "Popped element mismatch, expected " << ((i+1) * 10) << " but got " << peekedElem << endl;
            return false;
        }
        if (verbose) {
            cout << "[D: " << elem << ", " << c.size() << "] ";
        }
    }

    try {
        c.dequeue();
        if (verbose) {
            cout << "Dequeue empty test: This should have thrown!" << endl;
        }
        return false;
    } catch (std::runtime_error &e) {
        if (verbose) {
            cout << "Threw error " << e.what() << endl;
        }
    }

    try {
        for (int i = 0; i < 6; i++) {
            c.enqueue(i);   
        }
        if (verbose) {
            cout << "Enqueue full test: This should have thrown!" << endl;
        }
        return false;
    } catch (std::runtime_error &e) {
        if (verbose) {
            cout << "Threw error " << e.what() << endl;
        }
    }
    
    return true;
}

class TestActor : public Actor {
    public:
        TestActor(int i) : Actor("Testactor"s + std::to_string(i)) {}
        void act() {}
};

bool performActorGraphTest(bool verbose) {
    ActorGraph ag;

    for (int i = 0; i < 4; i++) {
        ag.addActor(new TestActor(i + 4 * upcxx::rank_me()));
    }

    upcxx::barrier();

    if (!upcxx::rank_me()) {
        cout << ag.getNumActors() << endl;
    }

    return true;
}

bool performTest(bool (*f)(bool)) {
    if (f(true)) {
        cout << "Test succeeeded." << endl;
        return true;
    } else {
        cout << "!!! TEST FAILED !!!\nPerforming again with verbose output." << endl;
        f(true);
        return false;
    }
}

bool performTests() {
    cout << "Test class Channel: ";
    bool channelTestResult = performTest(channelTest);
    return channelTestResult;
}

}
