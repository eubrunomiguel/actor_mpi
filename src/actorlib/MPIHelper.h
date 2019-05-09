//
// Created by Bruno Macedo Miguel on 2019-05-09.
//

#ifndef ACTORUPCXX_MPIHELPER_H
#define ACTORUPCXX_MPIHELPER_H

#include <mpi.h>

namespace MPIHelper{
    typedef int RankId;

    int myRank(){
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        return rank;
    }

    int worldSize(){
        int worldSize;
        MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
        return worldSize;
    }
}


#endif //ACTORUPCXX_MPIHELPER_H
