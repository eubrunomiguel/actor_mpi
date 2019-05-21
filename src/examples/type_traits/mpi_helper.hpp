//
// Created by Bruno Macedo Miguel on 2019-05-09.
//

#ifndef ACTORUPCXX_MPI_CPP
#define ACTORUPCXX_MPI_CPP

#include "mpi.h"
#include "mpi_type_traits.h"
#include <iostream>

namespace mpi {

using rank = int;
using tag = int;

template <class T> class Request {
public:
  Request() : tag(0), rank(0), buffer(), request() {
    rawBuffer = (int *)malloc(sizeof(int) * 10);
  }

  Request(tag tag, rank rank) : tag(tag), rank(rank), buffer(), request() {
    rawBuffer = (int *)malloc(sizeof(int) * 10);
  }

  Request(tag tag, rank rank, T &&data)
      : tag(tag), rank(rank), buffer(data), request() {
    rawBuffer = (int *)malloc(sizeof(int) * 10);
  }

  auto &getBuffer() { return buffer; }

  auto *getRequest() { return &request; }

  auto test() {
    int completed = 0;
    MPI_Test(request, &completed, MPI_STATUS_IGNORE);
    return static_cast<bool>(completed);
  }

  auto wait() { MPI_Wait(&request, MPI_STATUS_IGNORE); }

  auto &getRawBuffer() { return *rawBuffer; }

private:
  tag tag;
  rank rank;
  T buffer;
  MPI_Request request;
  int *rawBuffer;
};

constexpr rank InvalidRankId = -1;

int me() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  return rank;
}

int world() {
  int worldSize;
  MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
  return worldSize;
}

template <typename T>
std::unique_ptr<MPI_Request> ISend(T &data, rank rank, tag tag) {
  auto request = std::make_unique<MPI_Request>();
  MPI_Isend(mpi_type_traits<T>::get_addr(data),
            mpi_type_traits<T>::get_size(data),
            mpi_type_traits<T>::get_type(std::move(data)), rank, tag,
            MPI_COMM_WORLD, request.get());
  return request;
}

template <typename T>
std::unique_ptr<MPI_Request> IRecv(T &buffer, int elementCount, rank rank,
                                   tag tag) {
  auto request = std::make_unique<MPI_Request>();
  MPI_Irecv(mpi_type_traits<T>::get_addr(buffer), elementCount,
            mpi_type_traits<T>::get_type(std::move(buffer)), rank, tag,
            MPI_COMM_WORLD, request.get());
  return request;
}

bool hasCompleted(MPI_Request *request) {
  int completed = 0;
  MPI_Test(request, &completed, MPI_STATUS_IGNORE);
  return static_cast<bool>(completed);
}

void wait(MPI_Request *request) { MPI_Wait(request, MPI_STATUS_IGNORE); }

} // namespace mpi

#endif
