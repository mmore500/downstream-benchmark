#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <optional>
#include <string_view>

#include "../../downstream/include/downstream/_auxlib/DOWNSTREAM_UINT.hpp"
#include "../../downstream/include/downstream/_auxlib/overflow_shr.hpp"
#include "../../downstream/include/downstream/_auxlib/std_bit_casted.hpp"
#include "../../downstream/include/downstream/dstream/dstream.hpp"

#include "../aux/ctz_naive.hpp"
#include "../aux/log2_naive.hpp"

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
  uint32_t data[32];
};

template <uint32_t S> inline uint32_t lookup_bs(uint32_t x) {
  constexpr bs_table<S> bs{};
  switch (x) {
  case 0:
    return bs.data[0];
  case 1:
    return bs.data[1];
  case 2:
    return bs.data[2];
  case 3:
    return bs.data[3];
  case 4:
    return bs.data[4];
  case 5:
    return bs.data[5];
  case 6:
    return bs.data[6];
  case 7:
    return bs.data[7];
  case 8:
    return bs.data[8];
  case 9:
    return bs.data[9];
  case 10:
    return bs.data[10];
  case 11:
    return bs.data[11];
  case 12:
    return bs.data[12];
  case 13:
    return bs.data[13];
  case 14:
    return bs.data[14];
  case 15:
    return bs.data[15];
  case 16:
    return bs.data[16];
  case 17:
    return bs.data[17];
  case 18:
    return bs.data[18];
  case 19:
    return bs.data[19];
  case 20:
    return bs.data[20];
  case 21:
    return bs.data[21];
  case 22:
    return bs.data[22];
  case 23:
    return bs.data[23];
  case 24:
    return bs.data[24];
  case 25:
    return bs.data[25];
  case 26:
    return bs.data[26];
  case 27:
    return bs.data[27];
  case 28:
    return bs.data[28];
  case 29:
    return bs.data[29];
  case 30:
    return bs.data[30];
  case 31:
    return bs.data[31];
  default:
    __builtin_unreachable();
  }
}

template <uint32_t S> struct kb_table {
  constexpr kb_table() : data() {
    for (uint32_t b_l = 0; b_l < S / 2; ++b_l) {
      // Need to calculate physical bunch index...
      // ... i.e., position among bunches left-to-right in buffer space
      const uint32_t v =
          std::bit_width(b_l); // Nestedness depth level of physical bunch
      const uint32_t w =
          (S >> v) *
          (v != 0); // Num bunches spaced between bunches in nest level
      const uint32_t o =
          w >> 1; // Offset of nestedness level in physical bunch order
      const uint32_t p =
          b_l - std::bit_floor(b_l);  // Bunch position within nestedness level
      const uint32_t b_p = o + w * p; // Physical bunch index...
      // ... i.e., in left-to-right sequential bunch order

      // Need to calculate buffer position of b_p'th bunch
      const bool epsilon_k_b =
          (b_l != 0); // Correction factor for zeroth bunch...
      // ... i.e., bunch r=s at site k=0
      data[b_l] = (b_p << 1) + std::popcount((S << 1) - b_p) - 1 -
                  epsilon_k_b; // Site index of bunch
    }
  }
  uint16_t data[S / 2];
};


template <uint32_t S>
uint32_t _dstream_stretched_assign_storage_site_impl(const uint32_t T) {

  constexpr uint32_t _1{1};
  namespace aux = downstream::_auxlib;

  const uint32_t blT = log2_naive(T) + bool(T);
  const uint32_t h = ctz_naive(T + _1); // Current hanoi value

  // DEPENDS ON t, h
  const uint32_t i = T >> (h + _1);
  // ^^^ Hanoi value incidence (i.e., num seen)

  const uint32_t b = lookup_bs<S>(blT); // Num bunches available to hanoi value

  // DEPENDS ON t, h
  if (i >= b) [[likely]] { // If seen more than sites reserved to hanoi value...
    return S;              // ... discard without storing
  }

  const uint32_t b_l = i; // Logical bunch index...
  constexpr kb_table<S> kb{};
  const auto k_b = kb.data[b_l];

  return k_b + h; // Calculate placement site...
                  // ... where h.v. h is offset within bunch
}

uint32_t _dstream_stretched_assign_storage_site(const uint32_t S,
                                                const uint32_t T) {
  if (S == 64)
    return _dstream_stretched_assign_storage_site_impl<64>(T);
  else if (S == 256)
    return _dstream_stretched_assign_storage_site_impl<256>(T);
  else if (S == 1024)
    return _dstream_stretched_assign_storage_site_impl<1024>(T);
  else if (S == 4096)
    return _dstream_stretched_assign_storage_site_impl<4096>(T);
  else
    __builtin_unreachable();
}

struct dstream_stretched_algo {
  static std::string_view get_algo_name() { return "dstream_stretched_algo"; }
  static uint32_t _assign_storage_site(uint32_t S, uint32_t T) {
    const auto result = _dstream_stretched_assign_storage_site(S, T);

    using u32 = uint32_t;
    using dstream_stretched_algo = downstream::dstream::stretched_algo_<u32>;
    [[maybe_unused]] const auto expected =
        dstream_stretched_algo::_assign_storage_site(S, T);
    assert(result == expected);

    return result;
  }
};
