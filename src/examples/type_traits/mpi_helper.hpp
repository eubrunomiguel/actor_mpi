//
// Created by Bruno Macedo Miguel on 2019-05-09.
//

#ifndef ACTORUPCXX_MPI_CPP
#define ACTORUPCXX_MPI_CPP

#include "mpi.h"
#include "mpi_type_traits.h"
#include <iostream>
#include <type_traits>

namespace mpi {

template <typename T> struct is_vector : public std::false_type {};

template <typename T, typename A>
struct is_vector<std::vector<T, A>> : public std::true_type {};

using rank = int;
using tag = int;

template <class T> class Request {
public:
  Request()
      : rank(0), tag(0), buffer(), request(), requestCompleted(false),
        requestStatus(), receiveValue(false) {}

  explicit Request(rank rank, tag tag, bool checkCount)
      : rank(rank), tag(tag), buffer(), request(), requestCompleted(false),
        requestStatus(), receiveValue(checkCount) {}

  explicit Request(rank rank, tag tag, T &&data, bool checkCount)
      : rank(rank), tag(tag), buffer(data), request(), requestCompleted(false),
        requestStatus(), receiveValue(checkCount) {}

  auto &getBuffer() { return buffer; }

  auto *getRequest() { return &request; }

  void wait() {
    while (!requestCompleted)
      test();
  }

  bool test() {
    if (receiveValue)
      return testProbe();
    else
      return testTest();
  }

  T get() {
    if (!receiveValue)
      throw std::runtime_error(
          "You cannot receive a value from a send operation, "
          "use the wait() method.");

    wait();

    prepareBuffer();

    MPI_Recv(mpi_type_traits<T>::get_addr(buffer), requestMessageSize,
             mpi_type_traits<T>::get_type(std::move(buffer)), rank, tag,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printBuffer();

    return buffer;
  }

private:
  void printBuffer() { printBuffer(is_vector<T>{}); }

  void printBuffer(std::true_type) {
    printf("Dynamically received %d numbers from rank %d, tag %d. Message: ",
           requestMessageSize, rank, tag);

    if (!buffer.empty()) {
      auto first = buffer.cbegin();
      std::cout << *first;

      while (buffer.cend() != ++first) {
        std::cout << ", " << *first;
      }
    }

    std::cout << ".\n";
  }

  void printBuffer(std::false_type) {
    printf("Dynamically received %d from rank %d, tag %d.", buffer, rank, tag);
  }

  bool testTest() {
    if (requestCompleted)
      return requestCompleted;

    int flag = 0;
    MPI_Test(&request, &flag, &requestStatus);

    if (flag == 1)
      requestCompleted = true;
    return requestCompleted;
  }

  bool testProbe() {
    if (requestCompleted)
      return requestCompleted;

    int flag = 0;
    MPI_Iprobe(rank, tag, MPI_COMM_WORLD, &flag, &requestStatus);

    if (flag == 1)
      requestCompleted = true;
    return requestCompleted;
  }

  void prepareBuffer() { prepareBuffer(is_vector<T>{}); }

  void prepareBuffer(std::true_type) {
    MPI_Get_count(&requestStatus, MPI_INT, &requestMessageSize);

    if (static_cast<bool>(is_vector<T>::value))
      buffer.resize(requestMessageSize);
  }

  void prepareBuffer(std::false_type) { requestMessageSize = 1; }

  bool receiveValue;

  rank rank;
  tag tag;
  T buffer;

  MPI_Request request;
  bool requestCompleted;
  MPI_Status requestStatus;
  int requestMessageSize;
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
