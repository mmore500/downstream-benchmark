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
#include "../aux/divpow2.hpp"
#include "../aux/downcast_value.hpp"
#include "../aux/sizeof_vector.hpp"
#include "../aux/xorshift_generator.hpp"

struct doubling_tilted_algo {
  static std::string_view get_algo_name() { return "doubling_tilted_algo"; }
};

template <typename Storage> void _apply_thinning_tilted(Storage &storage) {
  const auto N = storage.size();
  for (size_t i = 0; i < (N >> 1); ++i)
    storage[i] = storage[i << 1];
}

template <typename dtype, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_doubling_tilted_assign_storage_site(const uint32_t num_items) {
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
  const uint32_t stride = num_sites >> 1;
  for (uint32_t T = num_sites; T < num_items; ++T) {
    const auto data = downcast_value<dtype>(gen());

    const uint32_t k = divpow2(T, stride) + stride;
    if (k == stride) [[unlikely]]
      _apply_thinning_tilted(*storage);

    assert(num_sites >> 1 <= k && k < num_sites);
    (*storage)[k] = data;
  }

  DoNotOptimize(storage);
  DoNotOptimize(gen.state);
  return sizeof(storage_t) + sizeof(uint32_t /* i */);
}
