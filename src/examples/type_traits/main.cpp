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

  if (me() == 0) {
    auto value = 260894;
    auto req = ISend<int>(value, 1, 0);
    wait(req.get());

    vector<int> values{1, 2, 3};
    req = ISend<vector<int>>(values, 1, 0);
    wait(req.get());

    cout << "sending " << value << std::endl;
  } else {
    int *recv = (int *)malloc(sizeof(int) * 1);
    auto req = IRecv(*recv, 1, 0, 0);
    wait(req.get());

    cout << "receiving " << *recv << endl;

    recv = (int *)malloc(sizeof(int) * 3);
    req = IRecv(*recv, 3, 0, 0);
    wait(req.get());

    cout << "receiving " << recv[0] << recv[1] << recv[2] << endl;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  return 0;
}