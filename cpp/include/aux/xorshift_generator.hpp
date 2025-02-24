#pragma once
#ifndef AUX_XORSHIFT_GENERATOR_HPP_INCLUDE
#define AUX_XORSHIFT_GENERATOR_HPP_INCLUDE

#include <cstdint>

// adapted from https://en.wikipedia.org/wiki/Xorshift
struct xorshift_generator {

  uint32_t state;

  xorshift_generator() : state(0xdeadbeef) {}

  uint32_t operator()() {
    uint32_t x = this->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    this->state = x;
    return x;
  }
};
#endif // #ifndef AUX_XORSHIFT_GENERATOR_HPP_INCLUDE
