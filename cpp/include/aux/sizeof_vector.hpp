#pragma once

#include <cstdint>
#include <vector>

template <typename T> size_t sizeof_vector(const std::vector<T> &vec) {
  return sizeof(vec) + vec.size() * sizeof(T);
}

template <> size_t sizeof_vector(const std::vector<bool> &vec) {
  return sizeof(vec) + (vec.size() + 7) / 8;
}
