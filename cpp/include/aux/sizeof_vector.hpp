#pragma once
#ifndef AUX_SIZEOF_VECTOR_HPP_INCLUDE
#define AUX_SIZEOF_VECTOR_HPP_INCLUDE

#include <cstdint>
#include <vector>

template <typename T> size_t sizeof_vector(const std::vector<T> &vec) {
  return sizeof(vec) + vec.size() * sizeof(T);
}

template <> size_t sizeof_vector(const std::vector<bool> &vec) {
  return sizeof(vec) + (vec.size() + 7) / 8;
}
#endif // #ifndef AUX_SIZEOF_VECTOR_HPP_INCLUDE
