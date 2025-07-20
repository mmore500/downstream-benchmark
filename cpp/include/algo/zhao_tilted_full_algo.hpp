#pragma once
#ifndef ALGO_ZHAO_TILTED_FULL_ALGO_HPP_INCLUDE
#define ALGO_ZHAO_TILTED_FULL_ALGO_HPP_INCLUDE

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <type_traits>

#include "../aux/DoNotOptimize.hpp"
#include "../aux/downcast_value.hpp"
#include "../aux/sizeof_vector.hpp"
#include "../aux/smallest_unsigned_t.hpp"
#include "../aux/xorshift_generator.hpp"

struct zhao_tilted_full_algo {
  static std::string_view get_algo_name() { return "zhao_tilted_full_algo"; }
};

template <typename dtype, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_zhao_tilted_full_assign_storage_site(const uint32_t num_items) {
  using segment_lengths_t = smallest_unsigned_t<num_sites>::type;
  constexpr auto max_segments = std::min(num_sites, 64u);
  std::array<segment_lengths_t, max_segments> segment_lengths{};

  // use vector to allow for vector<bool>
  std::vector<dtype> storage(num_sites);
  DoNotOptimize(storage);

  xorshift_generator gen{};
  for (uint32_t T = 0; T < num_items; ++T) {
    const auto data = downcast_value<dtype>(gen());

    if (T < num_sites) {
      storage[T] = data;
      segment_lengths[0] += 1;
      continue;
    }

    constexpr auto S = num_sites;
    auto& w = segment_lengths;
    auto& b = storage;

    w[0] += 1;
    uint32_t i = S;
    uint32_t j = 0;
    while (w[j] <= w[j + 1]) {
      i -= w[j];
      j += 1;
      assert(j < max_segments);
    }

    w[j] -= 2;
    w[j + 1] += 1;

    const auto erase_idx = i - w[j];
    b.erase(std::next(std::begin(storage), erase_idx));
    b.push_back(data);

  }

  DoNotOptimize(storage);
  DoNotOptimize(gen.state);

  // use vector-based implementation for efficiency
  // std::bitset has bad performance for shift-down on erase
  using storage_t =
      std::conditional_t<std::is_same_v<dtype, bool>, std::bitset<num_sites>,
                         std::array<dtype, num_sites>>;
  return sizeof(segment_lengths) + sizeof(storage_t);
}
#endif // #ifndef ALGO_ZHAO_TILTED_FULL_ALGO_HPP_INCLUDE
