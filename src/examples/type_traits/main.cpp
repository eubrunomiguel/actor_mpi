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

auto sendVec() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  vector<int> values{1, 2, 3};
  auto req = Isend(values, other, tag);
  req.wait();
}

auto receiveVec() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  auto req = Irecv<vector<int>>(other, tag);
  req.get();
}

int main() {
  MPI_Init(nullptr, nullptr);

  if (me() == 0) {
    sendVec();
    sendUnique();
  } else {
    receiveVec();
    receiveUnique();
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  return 0;
}