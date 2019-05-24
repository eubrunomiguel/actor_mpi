//
// Created by Bruno Macedo Miguel on 2019-05-11.
//
#pragma once

#include <iostream>
namespace helper {
static int compute_hash(std::string const &s) {
  const int p = 53;
  const int m = 1e9 + 9;
  long long hash_value = 0;
  long long p_pow = 1;
  for (char c : s) {
    hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
    std::cout << hash_value << std::endl;
    p_pow = (p_pow * p) % m;
  }
  return hash_value;
}
} // namespace helper
