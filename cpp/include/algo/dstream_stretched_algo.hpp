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

uint32_t _dstream_stretched_assign_storage_site(const uint32_t S,
                                                const uint32_t T) {

  constexpr uint32_t _1{1};
  namespace aux = downstream::_auxlib;

  const uint32_t s = std::bit_width(S) - _1;
  const uint32_t blT = std::bit_width(T);
  const uint32_t t = blT - std::min(s, blT); // Current epoch
  const uint32_t h =
      aux::countr_zero_casted<uint32_t>(T + _1); // Current hanoi value
  const uint32_t i = aux::overflow_shr<uint32_t>(T, h + _1);
  // ^^^ Hanoi value incidence (i.e., num seen)

  const uint32_t blt = std::bit_width(t); // Bit length of t
  bool epsilon_tau =
      aux::bit_floor_casted<uint32_t>(t << _1) > t + blt; // Correction factor
  const uint32_t tau = blt - epsilon_tau;                 // Current meta-epoch
  const uint32_t b = std::max<uint32_t>(S >> (tau + _1), _1);
  // ^^^ Num bunches available to h.v.
  if (i >= b) { // If seen more than sites reserved to hanoi value...
    return S;   // ... discard without storing
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

struct dstream_stretched_algo {
  static std::string_view get_algo_name() { return "dstream_stretched_algo"; }
  static uint32_t _assign_storage_site(uint32_t S, uint32_t T) {
    const auto result = _dstream_stretched_assign_storage_site(S, T);

    using u32 = uint32_t;
    using dstream_stretched_algo = downstream::dstream::stretched_algo_<u32>;
    assert(result == dstream_stretched_algo::_assign_storage_site(S, T));

    return result;
  }
};
