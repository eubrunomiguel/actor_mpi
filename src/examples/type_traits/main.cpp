#include "mpi.h"
#include "mpi_helper.hpp"
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace std;
using namespace mpi;

int main() {
  MPI_Init(nullptr, nullptr);

  auto other = (me() + 1) % 2;
  auto tag = 666;

  if (me() == 0) {
    auto value = 260894;
    auto req = Isend(value, other, tag);
    req.wait();

  } else {
    auto req = Irecv<int>(other, tag);
    req.get();
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  return 0;
}