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

constexpr rank INVALID_RANK_ID = -1;
constexpr tag DEFAULT_TAG_ID = 0;

template <class T> class Request {
public:
  void wait() {
    MPI_Wait(&request, &requestStatus);
    requestCompleted = true;
  }

  bool hasFinished() {
    if (requestCompleted)
      return requestCompleted;

    int flag = 0;
    MPI_Test(&request, &flag, &requestStatus);

    requestCompleted = static_cast<bool>(flag);

    return requestCompleted;
  }

  bool isDirty() const { return rank != INVALID_RANK_ID; }

protected:
  Request()
      : rank(INVALID_RANK_ID), tag(DEFAULT_TAG_ID), request(),
        requestCompleted(false), requestStatus() {}

  Request(rank rank, tag tag)
      : rank(rank), tag(tag), request(), requestCompleted(false),
        requestStatus() {}

  rank rank;
  tag tag;

  MPI_Request request;
  bool requestCompleted;
  MPI_Status requestStatus;
};

template <class T> class SendRequest : public Request<T> {
public:
  SendRequest() : Request<T>(), localBuffer() {}

  SendRequest(rank rank, tag tag, const T &data)
      : Request<T>(rank, tag), localBuffer(data) {
    send();
  }

  SendRequest(rank rank, tag tag, T &&data)
      : Request<T>(rank, tag), localBuffer(std::forward<T>(data)) {
    send();
  }

private:
  void send() {
    MPI_Issend(mpi_type_traits<T>::get_addr(localBuffer),
               mpi_type_traits<T>::get_size(localBuffer),
               mpi_type_traits<T>::get_type(std::move(localBuffer)), this->rank,
               this->tag, MPI_COMM_WORLD, &this->request);
  }
  T localBuffer;
};

template <class T> class ReceiveRequest : public Request<T> {
public:
  ReceiveRequest() : Request<T>(), channelBuffer(nullptr) {}

  ReceiveRequest(rank rank, tag tag, T *buffer)
      : Request<T>(rank, tag), channelBuffer(buffer) {
    receive();
  }

  T *getBuffer() { return channelBuffer; }

private:
  void receive() {
    MPI_Irecv(mpi_type_traits<T>::get_addr(*channelBuffer),
              mpi_type_traits<T>::get_size(*channelBuffer),
              mpi_type_traits<T>::get_type(std::move(*channelBuffer)),
              this->rank, this->tag, MPI_COMM_WORLD, &this->request);
  }
  T *channelBuffer;
};

static int me() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  return rank;
}

static int world() {
  int worldSize;
  MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
  return worldSize;
}

template <typename T>
SendRequest<T> static Isend(rank rank, tag tag, T &&data) {
  return SendRequest<T>(rank, tag, std::forward<T>(data));
}

template <typename T>
ReceiveRequest<T> static Irecv(rank rank, tag tag, T *buffer) {
  return ReceiveRequest<T>(rank, tag, buffer);
}

} // namespace mpi

#endif
