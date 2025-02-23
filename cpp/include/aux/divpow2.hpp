#pragma once

#include <bit>
#include <cassert>
#include <cstdint>

uint32_t divpow2(const uint32_t dividend, const uint32_t divisor) {
  assert(divisor != 0);
  assert(std::has_single_bit(divisor));
  return dividend >> (std::bit_width(divisor) - 1);
}
