//
// Created by Bruno Macedo Miguel on 2019-05-09.
//

#ifndef ACTORUPCXX_MPIHELPER_CPP
#define ACTORUPCXX_MPIHELPER_CPP

#include "mpi.h"
#include "mpi_type_traits.h"

using namespace mpi;

namespace MPIHelper {
    using RankId = int;
    using TagId = int;

    constexpr RankId InvalidRankId = -1;

    int myRank() {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        return rank;
    }

    int worldSize() {
        int worldSize;
        MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
        return worldSize;
    }

    template<class T>
    std::unique_ptr<MPI_Request> ISend(T &&value, RankId rank, TagId tag) {
        auto request = std::make_unique<MPI_Request >();
        MPI_ISend(mpi_type_traits<T>::get_addr(value),
                  mpi_type_traits<T>::get_size(value),
                  mpi_type_traits<T>::get_type(value),
                  rank,
                  tag,
                  MPI_COMM_WORLD,
                  request);
        return std::move(request);
    }
} // namespace MPIHelper

#endif // ACTORUPCXX_MPIHELPER_CPP
