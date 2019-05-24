#include "mpi.h"
#include "mpi_helper.hpp"
#include "mpi_type_traits.h"
#include <array>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <type_traits>
#include <vector>

using namespace std;
using namespace mpi;

auto sendUnique() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  auto value = 260894;
  auto req = Isend(other, tag, value);
  req.wait();
}

auto receiveUnique() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  int value = 0;
  auto req = Irecv(other, tag, value);
  req.wait();
  std::cout << value;
}

auto sendStdArray() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  array<int, 3> values{1, 2, 3};
  auto req = Isend(other, tag, values);
  req.wait();
}

auto receiveStdArray() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  array<int, 3> values{0, 0, 0};

  auto req = Irecv(other, tag, values);
  req.wait();
  for (auto &val : values)
    cout << val;
}

auto sendVec() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  vector<int> values{1, 2, 3};
  auto req = Isend(other, tag, values);
  req.wait();
}

auto receiveVec() {
  auto other = (me() + 1) % 2;
  constexpr int tag = 666;

  vector<int> values{0, 0, 0};

  auto req = Irecv(other, tag, values);
  req.wait();
  for (auto &val : values)
    cout << val;
}

void sendRecv() {
  if (me() == 0) {
    sendVec();
  } else {
    receiveVec();
  }
}

int main() {
  MPI_Init(nullptr, nullptr);

  sendRecv();

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  return 0;
}