#pragma once
#ifndef ALGO_DSTREAM_HELPERS_HPP_INCLUDE
#define ALGO_DSTREAM_HELPERS_HPP_INCLUDE

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>

#include "../aux/NOFLASH.hpp"
#include "../aux/smallbitops.hpp"
#include "../aux/smallest_unsigned_t.hpp"

template <uint32_t S> struct bs_table {
  constexpr bs_table() : data() {
    constexpr uint32_t s = std::bit_width(S) - 1;
    for (uint32_t blT = 0; blT < 32; ++blT) {
      const uint32_t t = blT - std::min(s, blT); // Current epoch
      const uint32_t blt = std::bit_width(t);    // Bit length of t

      bool epsilon_tau = std::bit_floor(t << 1) > t + blt; // Correction factor
      const uint32_t tau = blt - epsilon_tau;              // Current meta-epoch
      data[blT] = std::max<uint32_t>(S >> (tau + 1), 1);
      // ^^^ Num bunches available to h.v.
    }
  }
  smallest_unsigned_t<S>::type data[32];
};

template <uint32_t S> inline uint32_t lookup_bs(uint32_t x) {
  const static bs_table<S> NOFLASH lookup_bs_table{};
  return lookup_bs_table.data[x];
}

template <uint32_t S>
uint32_t inline constexpr calc_B(const uint32_t blT, const uint32_t h) {
  constexpr uint32_t s = std::bit_width(S) - 1;
  const uint32_t t = blT - std::min(s, blT); // Current epoch

  const uint32_t blt = bitwidth_uint8(t); // Bit length of t
  const bool epsilon_tau =
      std::bit_floor<uint32_t>(t << 1) > t + blt; // Correction factor
  // for some reason calculating epsilon_tau as
  //   !((t + blt) >> blt)
  // substantially pessimizes performance on raspberry pi pico
  const uint32_t tau = blt - epsilon_tau;         // Current meta-epoch
  const uint32_t t_0 = (1 << tau) - tau;          // Opening epoch of meta-epoch
  const uint32_t t_1 =
      (1 << (tau + 1)) - (tau + 1); // Opening epoch of next meta-epoch
  const bool epsilon_b =
      t < h + t_0 && h + t_0 < t_1; // Uninvaded correction factor
  const uint32_t B = std::max<uint32_t>(S >> (tau + 1 - epsilon_b), 1);
  // ^^^ Num bunches available to h.v.
  return B;
}

template <uint32_t S, uint32_t max_h> struct B_table {

  constexpr B_table() : data() {
    for (uint32_t h = 0; h < max_h; ++h) {
      for (uint32_t blT = 0; blT < 32; ++blT) {
        data[h * 32 + blT] = calc_B<S>(blT, h);
      }
    }
  }
  smallest_unsigned_t<S>::type data[32 * max_h];
};

template <uint32_t S, uint32_t max_h>
inline uint32_t lookup_B(const uint32_t blT, const uint32_t h) {
  assert(h < max_h);
  const static B_table<S, max_h> NOFLASH lookup_B_table{};
  return lookup_B_table.data[h * 32 + blT];
}

template <uint32_t S> uint32_t constexpr inline calc_kb(const uint32_t b_l) {
  if (b_l == 0)
    return 0;

  constexpr uint32_t twoS = S << 1;
  // Compute nestedness depth level based on S.
  const uint32_t v = (S <= 256) ? bitwidth_uint8(b_l) : bitwidth_uint16(b_l);
  const uint32_t w =
      S >> v; // Number of bunches spaced between bunches at this nest level.
  const uint32_t o = w >> 1;               // Offset in physical bunch order.
  const uint32_t p = b_l - (1 << (v - 1)); // Position within the nest level.
  const uint32_t b_p = o + w * p;          // Physical bunch index.

  // Use appropriate popcount function depending on S.
  return (b_p << 1) +
         ((S <= 128) ? popcount_uint8(twoS - b_p)
                     : popcount_uint16(twoS - b_p)) -
         2;
}

template <uint32_t S> struct kb_table {
  constexpr kb_table() : data() {
    for (uint32_t b_l = 0; b_l < S / 2; ++b_l) {
      data[b_l] = calc_kb<S>(b_l);
    }
  }
  smallest_unsigned_t<S>::type data[S / 2];
};

template <uint32_t S> inline uint32_t lookup_kb(const uint32_t b_l) {
  const static kb_table<S> NOFLASH lookup_kb_table{};
  return lookup_kb_table.data[b_l];
}
#endif // #ifndef ALGO_DSTREAM_HELPERS_HPP_INCLUDE
