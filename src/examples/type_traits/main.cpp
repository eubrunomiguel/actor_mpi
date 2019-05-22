#include "mpi.h"
#include "mpi_helper.hpp"
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace std;
using namespace mpi;

auto sendUnique() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  auto value = 260894;
  auto req = Isend(value, other, tag);
  req.wait();
}

auto receiveUnique() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  auto req = Irecv<int>(other, tag);
  req.get();
}

auto sendVec(int size) {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  vector<int> values(size, 13);
  auto req = Isend(values, other, tag);
  req.wait();
}

auto receiveVec() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  auto req = Irecv<vector<int>>(other, tag);
  req.get();
}

void sendRecv() {
  if (me() == 0) {
    sendVec(10);
    sendVec(15);
    sendVec(12);
    sendVec(13);
    sendVec(0);
  } else {
    receiveVec();
    receiveVec();
    receiveVec();
    receiveVec();
    receiveVec();
  }
}

void sendRecv_incorrent() {
  if (me() == 0) {
    sendUnique();
    sendVec(10);
  } else {
    receiveVec();
    receiveUnique();
  }
}

int main() {
  MPI_Init(nullptr, nullptr);

  sendRecv();

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  return 0;
}