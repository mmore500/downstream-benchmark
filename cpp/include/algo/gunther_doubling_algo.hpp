#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <vector>

#include "../../downstream/include/downstream/_auxlib/modpow2.hpp"

#include "../aux/DoNotOptimize.hpp"
#include "../aux/downcast_value.hpp"
#include "../aux/sizeof_vector.hpp"
#include "../aux/xorshift_generator.hpp"

struct gunther_doubling_algo {
  static std::string_view get_algo_name() { return "gunther_doubling_algo"; }
};

template <uint32_t S> uint32_t _calc_stride(const uint32_t T) {
  const uint32_t _1{1};
  const uint32_t s = std::bit_width(S) - 1;
  const uint32_t hv_thresh = std::bit_width(static_cast<uint32_t>(
    T >> s
  ));
  return _1 << hv_thresh;
}

template <typename Storage>
void _apply_thinning(Storage &storage) {
  const auto N = storage.size();
  for (size_t i = 0; i < (N >> 1); ++i) storage[i] = storage[i << 1];
}

uint32_t _div_pow2(const uint32_t dividend, const uint32_t divisor) {
  assert(divisor != 0);
  assert(std::has_single_bit(divisor));
  return dividend >> (std::bit_width(divisor) - 1);
}

template <typename dtype, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_gunther_doubling_assign_storage_site(const uint32_t num_items) {
  using storage_t =
      std::conditional_t<std::is_same_v<dtype, bool>, std::bitset<num_sites>,
                         std::array<dtype, num_sites>>;
  std::optional<storage_t> storage; // bypass zero-initialization
  DoNotOptimize(*storage);

  xorshift_generator gen{};

  // initial fill of storage
  for (uint32_t T = 0; T < num_sites; ++T)
    (*storage)[T] = downcast_value<dtype>(gen());

  // subsequent elements
  for (uint32_t T = num_sites; T < num_items; ++T) {
    const auto data = downcast_value<dtype>(gen());
    const uint32_t stride = _calc_stride<num_sites>(T);

    const bool should_keep = downstream::_auxlib::modpow2(T, stride) == 0;
    if (!should_keep) [[likely]]
      continue;

    const uint32_t k = _div_pow2(T, stride);
    if (k == num_sites >> 1) [[unlikely]]
      _apply_thinning(*storage);

    assert(num_sites >> 1 <= k && k < num_sites);
    (*storage)[k] = data;
  }

  DoNotOptimize(storage);
  DoNotOptimize(gen.state);
  return sizeof(storage_t) + sizeof(uint32_t /* i */);
}
