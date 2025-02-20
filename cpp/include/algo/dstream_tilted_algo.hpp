#pragma once

#include <cassert>
#include <string_view>

#include "../../downstream/include/downstream/_auxlib/modpow2.hpp"
#include "../../downstream/include/downstream/dstream/dstream.hpp"

#include "../aux/ctz_naive.hpp"
#include "../aux/log2_naive.hpp"
#include "./dstream_helpers.hpp"

template <uint32_t S>
uint32_t _dstream_tilted_assign_storage_site_impl(const uint32_t T) {

  constexpr uint32_t _1{1};
  namespace aux = downstream::_auxlib;

  const uint32_t blT = log2_naive(T) + bool(T);
  const uint32_t h = ctz_naive(T + _1); // Current hanoi value

  const uint32_t i = T >> (h + _1);
  // ^^^ Hanoi value incidence (i.e., num seen)

  uint32_t B;
  if (h < 8) [[likely]]
    B = lookup_B<S, 8>(blT, h);
  else
    B = calc_B<S>(blT, h);

  const uint32_t b_l = aux::modpow2(i, B);
  ; // Logical bunch index...
  constexpr kb_table<S> kb{};
  const auto k_b = kb.data[b_l];

  return k_b + h; // Calculate placement site...
                  // ... where h.v. h is offset within bunch
}

uint32_t _dstream_tilted_assign_storage_site(const uint32_t S,
                                             const uint32_t T) {
  if (S == 64)
    return _dstream_tilted_assign_storage_site_impl<64>(T);
  else if (S == 256)
    return _dstream_tilted_assign_storage_site_impl<256>(T);
  else if (S == 1024)
    return _dstream_tilted_assign_storage_site_impl<1024>(T);
  else if (S == 4096)
    return _dstream_tilted_assign_storage_site_impl<4096>(T);
  else
    __builtin_unreachable();
}

struct dstream_tilted_algo {
  static std::string_view get_algo_name() { return "dstream_tilted_algo"; }
  static uint32_t _assign_storage_site(uint32_t S, uint32_t T) {
    const auto result = _dstream_tilted_assign_storage_site(S, T);

    using u32 = uint32_t;
    using dstream_tilted_algo = downstream::dstream::tilted_algo_<u32>;
    [[maybe_unused]] const auto expected =
        dstream_tilted_algo::_assign_storage_site(S, T);
    assert(result == expected);

    return result;
  }
};
