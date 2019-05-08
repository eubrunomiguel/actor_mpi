#include <mpi.h>
#include <stdlib.h>
#include <unordered_map>
#include <iostream>

std::unordered_map<std::string, int> actors;
int totalactors=0;
using namespace std;

int main(){
    MPI_Init(NULL, NULL);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int random = (rand() % 6 * world_rank + 1);
    for (int j = 0; j < random; ++j) {
        if (world_rank == 0)
            continue;
        actors.emplace("Actor " + j, world_rank * 5 + j);
        cout << world_rank << "-" << (world_rank * 5 + j) << endl;
    }

    totalactors=actors.size();

    // Load world information
    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Exchange local number of actors per rank
    int *numActorsPerRank = (int *)malloc(sizeof(int) * worldSize);
    MPI_Allgather(&totalactors, 1, MPI_INT, numActorsPerRank, 1, MPI_INT, MPI_COMM_WORLD);

    int totalActors = 0;
    for (int i = 0; i < worldSize; i++){
        totalActors += numActorsPerRank[i];
    }

    // Exchange Actors
    // TODO: only int at the moment, need the name as well

    // Current map to flat array
    int *myActors = (int *)malloc(sizeof(int) * totalactors);
    int myActorsIndex = 0;
    for (std::pair<std::string, int> element : actors){
        myActors[myActorsIndex] = element.second;
        myActorsIndex++;
    }

    int *displacement = (int *)malloc(sizeof(int) * worldSize);
    displacement[0] = 0;
    int currentIndex = 0;
    for (int i = 1; i < worldSize; i++){
        displacement[i] = numActorsPerRank[i-1] + currentIndex;
        currentIndex = displacement[i];
    }

    int *globalActors = (int *)malloc(sizeof(int) * totalActors);
    MPI_Allgatherv(myActors,
                   totalactors,
                   MPI_INT,
                   globalActors,
                   numActorsPerRank,
                   displacement,
                   MPI_INT,
                   MPI_COMM_WORLD);

    // Flat global actors or map
    for (int i = 0; i < totalActors; i++){
        // TODO: do not add our locals
        std::cout << globalActors[i] << std::endl;
    }

    delete numActorsPerRank;
    delete myActors;
    delete displacement;
    delete globalActors;



    MPI_Finalize();
    return 0;
}