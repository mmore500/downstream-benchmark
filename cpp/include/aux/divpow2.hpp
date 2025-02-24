#pragma once
#ifndef AUX_DIVPOW2_HPP_INCLUDE
#define AUX_DIVPOW2_HPP_INCLUDE

#include <bit>
#include <cassert>
#include <cstdint>

uint32_t divpow2(const uint32_t dividend, const uint32_t divisor) {
  assert(divisor != 0);
  assert(std::has_single_bit(divisor));
  return dividend >> (std::bit_width(divisor) - 1);
}
#endif // #ifndef AUX_DIVPOW2_HPP_INCLUDE
