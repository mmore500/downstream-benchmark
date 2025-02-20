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
  uint8_t data[32];
};

inline uint32_t ctz_naive(uint32_t x) {
  if (x & 1)
    return 0;
  else if (x & 2)
    return 1;
  else if (x & 4)
    return 2;
  else if (x & 8)
    return 3;
  else if (x & 16)
    return 4;

  uint32_t n = 5;
  x >>= 5;
  while ((x & 1) == 0) {
    x >>= 1;
    ++n;
  }
  return n;
}

// https://graphics.stanford.edu/~seander/bithacks.html
inline uint32_t log2_naive(uint32_t v) {

  static const int MultiplyDeBruijnBitPosition[32] =
  {
    0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
    8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
  };

  v |= v >> 1; // first round down to one less than a power of 2
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;

  return MultiplyDeBruijnBitPosition[(uint32_t)(v * 0x07C4ACDDU) >> 27];
}

template <uint32_t S>
uint32_t _dstream_stretched_assign_storage_site_impl(const uint32_t T) {

  constexpr uint32_t _1{1};
  namespace aux = downstream::_auxlib;

  const uint32_t blT = log2_naive(T) + bool(T);
  const uint32_t h = ctz_naive(T + _1); // Current hanoi value

  // DEPENDS ON t, h
  const uint32_t i = T >> (h + _1);
  // ^^^ Hanoi value incidence (i.e., num seen)

  constexpr bs_table<S> bs{};
  const uint32_t b = bs.data[blT]; // Num bunches available to hanoi value

  // DEPENDS ON t, h
  if (i >= b) [[likely]] { // If seen more than sites reserved to hanoi value...
    return S;              // ... discard without storing
  }

  const uint32_t b_l = i; // Logical bunch index...
  // ... i.e., in order filled (increasing nestedness/decreasing init size r)

  // Need to calculate physical bunch index...
  // ... i.e., position among bunches left-to-right in buffer space
  const uint32_t v =
      std::bit_width(b_l); // Nestedness depth level of physical bunch
  const uint32_t w =
      (S >> v) * (v != 0); // Num bunches spaced between bunches in nest level
  const uint32_t o =
      w >> _1; // Offset of nestedness level in physical bunch order
  const uint32_t p =
      b_l - std::bit_floor(b_l);  // Bunch position within nestedness level
  const uint32_t b_p = o + w * p; // Physical bunch index...
  // ... i.e., in left-to-right sequential bunch order

  // Need to calculate buffer position of b_p'th bunch
  const bool epsilon_k_b = (b_l != 0); // Correction factor for zeroth bunch...
  // ... i.e., bunch r=s at site k=0
  const uint32_t k_b = (b_p << 1) +
                       aux::popcount_casted<uint32_t>((S << 1) - b_p) - 1 -
                       epsilon_k_b; // Site index of bunch

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
