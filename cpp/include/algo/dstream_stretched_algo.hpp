#pragma once
#ifndef ALGO_DSTREAM_STRETCHED_ALGO_HPP_INCLUDE
#define ALGO_DSTREAM_STRETCHED_ALGO_HPP_INCLUDE

#include <cassert>
#include <string_view>

#include "../../downstream/include/downstream/dstream/dstream.hpp"

#include "../aux/ctz_naive.hpp"
#include "../aux/log2_naive.hpp"
#include "./dstream_helpers.hpp"

template <uint32_t S>
uint32_t _dstream_stretched_assign_storage_site_impl(const uint32_t T) {

  constexpr uint32_t _1{1};

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
  uint32_t k_b;           // ... bunch offset
  if constexpr (S <= 256)
    k_b = lookup_kb<S>(b_l);
  else
    k_b = calc_kb<S>(b_l);

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
#endif // #ifndef ALGO_DSTREAM_STRETCHED_ALGO_HPP_INCLUDE
