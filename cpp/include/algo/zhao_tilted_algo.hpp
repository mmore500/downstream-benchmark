#pragma once
#ifndef ALGO_ZHAO_TILTED_ALGO_HPP_INCLUDE
#define ALGO_ZHAO_TILTED_ALGO_HPP_INCLUDE

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <vector>

#include "../aux/DoNotOptimize.hpp"
#include "../aux/downcast_value.hpp"
#include "../aux/sizeof_vector.hpp"
#include "../aux/xorshift_generator.hpp"

struct zhao_tilted_algo {
  static std::string_view get_algo_name() { return "zhao_tilted_algo"; }
};

template <typename dtype, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_zhao_tilted_assign_storage_site(const uint32_t num_items) {
  std::vector<uint32_t> segment_lengths;
  std::vector<dtype> storage;
  segment_lengths.reserve(num_sites);
  storage.reserve(num_sites);
  DoNotOptimize(storage);

  xorshift_generator gen{};
  for (uint32_t i = 0; i < num_items; ++i) {
    const auto data = downcast_value<dtype>(gen());

    storage.push_back(data);
    segment_lengths.push_back(1);

    const auto indexRange = std::ranges::reverse_view(
        std::views::iota(size_t{}, static_cast<size_t>(storage.size() - 1)));
    const auto collapse_iter =
        std::ranges::find_if(indexRange, [&](std::size_t i) {
          return segment_lengths[i] == segment_lengths[i + 1];
        });

    if (collapse_iter == std::end(indexRange))
      continue;

    const auto collapse_idx = *collapse_iter;

    segment_lengths[collapse_idx] += segment_lengths[collapse_idx + 1];
    storage.erase(std::next(std::begin(storage), collapse_idx + 1));
    segment_lengths.erase(
        std::next(std::begin(segment_lengths), collapse_idx + 1));
  }

  DoNotOptimize(storage);
  DoNotOptimize(gen.state);
  return sizeof_vector(storage) + sizeof_vector(segment_lengths);
}
#endif // #ifndef ALGO_ZHAO_TILTED_ALGO_HPP_INCLUDE
