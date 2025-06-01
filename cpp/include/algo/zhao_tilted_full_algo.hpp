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
  std::array<segment_lengths_t, num_sites> segment_lengths{};

  std::vector<dtype> storage;
  DoNotOptimize(storage);

  xorshift_generator gen{};
  for (uint32_t i = 0; i < num_items; ++i) {
    const auto data = downcast_value<dtype>(gen(i));

    if (i < num_sites) {
      storage.push_back(data);
      segment_lengths[0] += 1;
      continue;
    }

    uint32_t pos = 0;
    uint32_t j;
    for (j = 0;
         segment_lengths[j] <= segment_lengths[j + 1] && segment_lengths[j + 1];
         ++j) {
      pos += segment_lengths[j];
    }
    pos += segment_lengths[j];
    segment_lengths[j] -= 2;
    segment_lengths[j + 1] += 1;

    const auto erase_idx = num_sites - pos;
    storage.erase(std::next(std::begin(storage), erase_idx));
    storage.push_back(data);
    segment_lengths[0] += 1;
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
