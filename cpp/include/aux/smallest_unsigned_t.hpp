#pragma once
#ifndef AUX_SMALLEST_UNSIGNED_T_HPP_INCLUDE
#define AUX_SMALLEST_UNSIGNED_T_HPP_INCLUDE

#include <stdint.h>

// adapted from https://stackoverflow.com/a/7039269/17332200
template <unsigned long long Max> struct RequiredBits {
  enum {
    value = Max <= 0xff         ? 8
            : Max <= 0xffff     ? 16
            : Max <= 0xffffffff ? 32
                                : 64
  };
};

template <int bits> struct SelectInteger_;
template <> struct SelectInteger_<8> {
  typedef uint8_t type;
};
template <> struct SelectInteger_<16> {
  typedef uint16_t type;
};
template <> struct SelectInteger_<32> {
  typedef uint32_t type;
};
template <> struct SelectInteger_<64> {
  typedef uint64_t type;
};

template <unsigned long long Max>
struct smallest_unsigned_t : SelectInteger_<RequiredBits<Max>::value> {};
#endif // #ifndef AUX_SMALLEST_UNSIGNED_T_HPP_INCLUDE
