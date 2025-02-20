#pragma once

#include <cstdint>

// https://graphics.stanford.edu/~seander/bithacks.html
inline uint32_t log2_naive(uint32_t v) {
  v |= v >> 1; // first round down to one less than a power of 2
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;

  switch ((v * 0x07C4ACDDU) >> 27) {
  case 0:
    return 0;
  case 1:
    return 9;
  case 2:
    return 1;
  case 3:
    return 10;
  case 4:
    return 13;
  case 5:
    return 21;
  case 6:
    return 2;
  case 7:
    return 29;
  case 8:
    return 11;
  case 9:
    return 14;
  case 10:
    return 16;
  case 11:
    return 18;
  case 12:
    return 22;
  case 13:
    return 25;
  case 14:
    return 3;
  case 15:
    return 30;
  case 16:
    return 8;
  case 17:
    return 12;
  case 18:
    return 20;
  case 19:
    return 28;
  case 20:
    return 15;
  case 21:
    return 17;
  case 22:
    return 24;
  case 23:
    return 7;
  case 24:
    return 19;
  case 25:
    return 27;
  case 26:
    return 23;
  case 27:
    return 6;
  case 28:
    return 26;
  case 29:
    return 5;
  case 30:
    return 4;
  case 31:
    return 31;
  default:
    __builtin_unreachable();
  }
}
