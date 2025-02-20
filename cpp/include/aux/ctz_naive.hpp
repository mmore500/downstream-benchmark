#pragma once

#include <cstdint>

inline uint32_t ctz_naive(uint32_t x) {
  for (uint8_t i = 0; i < 32; i += 4) {
    switch (x & 0xF) {
    case 0:
      [[unlikely]] x >>= 4;
      continue;
    case 1:
      return 0 + i;
    case 2:
      return 1 + i;
    case 3:
      return 0 + i;
    case 4:
      return 2 + i;
    case 5:
      return 0 + i;
    case 6:
      return 1 + i;
    case 7:
      return 0 + i;
    case 8:
      return 3 + i;
    case 9:
      return 0 + i;
    case 10:
      return 1 + i;
    case 11:
      return 0 + i;
    case 12:
      return 2 + i;
    case 13:
      return 0 + i;
    case 14:
      return 1 + i;
    case 15:
      return 0 + i;
    default:
      __builtin_unreachable();
    }
  }
  return 32;
}
