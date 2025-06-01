#pragma once
#ifndef ALGO_ZHAO_STEADY_ALGO_HPP_INCLUDE
#define ALGO_ZHAO_STEADY_ALGO_HPP_INCLUDE

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <vector>

#include "../aux/DoNotOptimize.hpp"
#include "../aux/downcast_value.hpp"
#include "../aux/sizeof_vector.hpp"
#include "../aux/xorshift_generator.hpp"

struct zhao_steady_algo {
  static std::string_view get_algo_name() { return "zhao_steady_algo"; }
};

template <typename dtype, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_zhao_steady_assign_storage_site(const uint32_t num_items) {
  std::vector<uint8_t> segment_lengths;
  std::vector<dtype> storage;
  segment_lengths.reserve(num_sites);
  storage.reserve(num_sites);
  DoNotOptimize(storage);

  xorshift_generator gen{};
  for (uint32_t i = 0; i < num_items; ++i) {
    const auto data = downcast_value<dtype>(gen());

    if (storage.size() < num_sites) {
      storage.push_back(data);
      segment_lengths.push_back(0);
      continue;
    }

    if (segment_lengths[num_sites - 1] < segment_lengths[num_sites - 2]) {
      segment_lengths[num_sites - 1] += 1;
      continue;
    }

    storage.push_back(data);
    segment_lengths.push_back(0);

    uint32_t j;
    for (j = num_sites - 3; j > 0; --j) {
      if (segment_lengths[j] > segment_lengths[num_sites - 2])
        break;
    }

    const auto collapse_idx = j + 1;
    assert(segment_lengths[collapse_idx] == segment_lengths[collapse_idx + 1]);
    segment_lengths[collapse_idx] += 1;
    storage.erase(std::next(std::begin(storage), collapse_idx + 1));
    segment_lengths.erase(
        std::next(std::begin(segment_lengths), collapse_idx + 1));
  }

  DoNotOptimize(storage);
  DoNotOptimize(gen.state);
  // use vector-based implementation for efficiency
  // std::bitset has bad performance for shift-down on erase
  using storage_t =
      std::conditional_t<std::is_same_v<dtype, bool>, std::bitset<num_sites>,
                         std::array<dtype, num_sites>>;
  using segment_lengths_t = std::array<uint8_t, num_sites>;
  return sizeof(storage_t) + sizeof(segment_lengths_t);
}
#endif // #ifndef ALGO_ZHAO_STEADY_ALGO_HPP_INCLUDE
