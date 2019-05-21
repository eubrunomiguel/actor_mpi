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
  Request()
      : rank(0), tag(0), buffer(), request(), ready(false), status(),
        receiveValue(false) {}

  explicit Request(rank rank, tag tag, bool checkCount)
      : rank(rank), tag(tag), buffer(), request(), ready(false), status(),
        receiveValue(checkCount) {}

  explicit Request(rank rank, tag tag, T &&data, bool checkCount)
      : rank(rank), tag(tag), buffer(data), request(), ready(false), status(),
        receiveValue(checkCount) {}

  auto &getBuffer() { return buffer; }

  auto *getRequest() { return &request; }

  void wait() {
    while (!ready)
      test();
  }

  bool test() {
    if (receiveValue)
      return testProbe();
    else
      return testTest();
  }

  T get() {
    wait();
    int count = 0;
    MPI_Get_count(&status, MPI_INT, &count);

    MPI_Recv(mpi_type_traits<T>::get_addr(buffer), count,
             mpi_type_traits<T>::get_type(std::move(buffer)), rank, tag,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Dynamically received %d numbers from %d. Message: %d", count, rank,
           buffer);
    return buffer;
  }

private:
  bool testTest() {
    if (ready)
      return ready;

    int flag = 0;
    MPI_Test(&request, &flag, &status);

    if (flag == 1)
      ready = true;
    return ready;
  }

  bool testProbe() {
    if (ready)
      return ready;

    int flag = 0;
    MPI_Iprobe(rank, tag, MPI_COMM_WORLD, &flag, &status);

    if (flag == 1)
      ready = true;
    return ready;
  }

  rank rank;
  tag tag;
  T buffer;
  MPI_Request request;

  bool ready;
  MPI_Status status;
  bool receiveValue;
};

constexpr rank INVALID_RANK_ID = -1;

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

template <typename T> Request<T> Isend(T &data, rank rank, tag tag) {
  auto request = Request<T>(rank, tag, std::move(data), false);
  auto &buffer = request.getBuffer();
  MPI_Isend(mpi_type_traits<T>::get_addr(buffer),
            mpi_type_traits<T>::get_size(buffer),
            mpi_type_traits<T>::get_type(std::move(buffer)), rank, tag,
            MPI_COMM_WORLD, request.getRequest());
  return request;
}

template <typename T> Request<T> Irecv(rank rank, tag tag) {
  auto request = Request<T>(rank, tag, true);
  return request;
}

} // namespace mpi

#endif
