#pragma once
#ifndef AUX_LOG2_NAIVE_HPP_INCLUDE
#define AUX_LOG2_NAIVE_HPP_INCLUDE

#include <cstdint>

#include "./NOFLASH.hpp"

// https://graphics.stanford.edu/~seander/bithacks.html
__attribute__((hot)) inline uint8_t log2_naive(uint32_t v) {
  v |= v >> 1; // first round down to one less than a power of 2
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;

  static const uint8_t NOFLASH MultiplyDeBruijnBitPosition[32] = {
      0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
      8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31};
  return MultiplyDeBruijnBitPosition[(v * 0x07C4ACDDU) >> 27];
}
#endif // #ifndef AUX_LOG2_NAIVE_HPP_INCLUDE
