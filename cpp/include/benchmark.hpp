#pragma once
#ifndef BENCHMARK_HPP_INCLUDE
#define BENCHMARK_HPP_INCLUDE

#include <array>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string_view>
#include <type_traits>

#include "../downstream/include/downstream/dstream/dstream.hpp"

#include "./algo/control_throwaway_algo.hpp"
#include "./algo/doubling_steady_algo.hpp"
#include "./algo/doubling_tilted_algo.hpp"
#include "./algo/dstream_stretched_algo.hpp"
#include "./algo/dstream_tilted_algo.hpp"
#include "./algo/zhao_steady_algo.hpp"
#include "./algo/zhao_tilted_algo.hpp"
#include "./algo/zhao_tilted_full_algo.hpp"
#include "./aux/DoNotOptimize.hpp"
#include "./aux/downcast_value.hpp"
#include "./aux/get_compiler_name.hpp"
#include "./aux/name_value.hpp"
#include "./aux/xorshift_generator.hpp"

struct benchmark_result {
  std::string_view algo_name;
  std::string_view data_type;
  uint32_t memory_bytes;
  uint32_t num_items;
  uint32_t num_sites;
  uint32_t replicate;
  double duration_s;

  static std::string_view make_csv_header() {
    return ("algo_name,data_type,compiler,memory_bytes,num_items,"
            "num_sites,replicate,duration_s\n");
  }

  std::string make_csv_row() const {
    constexpr std::string_view compiler_name = get_compiler_name();
    return std::format("{},{},{},{},{},{},{},{}\n", algo_name, data_type,
                       compiler_name, memory_bytes, num_items, num_sites,
                       replicate, duration_s);
  }
};

namespace std {
std::ostream &operator<<(std::ostream &os, const benchmark_result &result) {
  os << result.make_csv_row();
  return os;
}
} // namespace std

template <typename dstream_algo, typename dtype, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_dstream_assign_storage_site(const uint32_t num_items) {

  using storage_t =
      std::conditional_t<std::is_same_v<dtype, bool>, std::bitset<num_sites>,
                         std::array<dtype, num_sites>>;
  std::optional<storage_t> storage; // bypass zero-initialization
  DoNotOptimize(*storage);
  xorshift_generator gen{};
  for (uint32_t i = 0; i < num_items; ++i) {
    const auto k = dstream_algo::_assign_storage_site(num_sites, i);
    const auto data = downcast_value<dtype>(gen());
    if (k != num_sites)
      (*storage)[k] = data;
  }

  DoNotOptimize(*storage);
  DoNotOptimize(gen.state);
  return sizeof(storage_t) + sizeof(uint32_t /* i */);
}

template <typename dtype, uint32_t num_sites, typename algo>
struct execute_assign_storage_site {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_dstream_assign_storage_site<algo, dtype, num_sites>(
        num_items);
  }
};

template <typename dtype, uint32_t num_sites>
struct execute_assign_storage_site<dtype, num_sites, doubling_steady_algo> {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_doubling_steady_assign_storage_site<dtype, num_sites>(
        num_items);
  }
};

template <typename dtype, uint32_t num_sites>
struct execute_assign_storage_site<dtype, num_sites, doubling_tilted_algo> {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_doubling_tilted_assign_storage_site<dtype, num_sites>(
        num_items);
  }
};

template <typename dtype, uint32_t num_sites>
struct execute_assign_storage_site<dtype, num_sites, zhao_steady_algo> {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_zhao_steady_assign_storage_site<dtype, num_sites>(num_items);
  }
};

template <typename dtype, uint32_t num_sites>
struct execute_assign_storage_site<dtype, num_sites, zhao_tilted_algo> {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_zhao_tilted_assign_storage_site<dtype, num_sites>(num_items);
  }
};

template <typename dtype, uint32_t num_sites>
struct execute_assign_storage_site<dtype, num_sites, zhao_tilted_full_algo> {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_zhao_tilted_full_assign_storage_site<dtype, num_sites>(
        num_items);
  }
};

template <typename algo, typename dtype, uint32_t num_sites>
benchmark_result time_assign_storage_site(const uint32_t replicate,
                                          const uint32_t num_items) {
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;

  const auto t1 = high_resolution_clock::now();
  using executor = execute_assign_storage_site<dtype, num_sites, algo>;
  const auto memory_bytes = executor::operator()(num_items);
  const auto t2 = high_resolution_clock::now();

  return {.algo_name = algo::get_algo_name(),
          .data_type = name_value<dtype>(),
          .memory_bytes = memory_bytes,
          .num_items = num_items,
          .num_sites = num_sites,
          .replicate = replicate,
          .duration_s =
              duration_cast<std::chrono::duration<double>>(t2 - t1).count()};
}

template <typename algo, typename dtype, uint32_t num_sites, typename OutputIt>
void benchmark_assign_storage_site_(OutputIt out) {
  const uint32_t num_replicates = 10;
  for (const uint32_t num_items : {10'000, 100'000, 1'000'000}) {
    uint32_t replicate{};
    std::generate_n(out, num_replicates, [num_items, &replicate]() {
      const auto env_var = std::getenv("DSTREAM_OBFUSCATE_UNSET_ENV_VAR") ?: "";
      // prevent compiler from knowing num_items in advance
      const uint32_t obfuscated_num_items = num_items + std::strlen(env_var);
      return time_assign_storage_site<algo, dtype, num_sites>(
          replicate++, obfuscated_num_items);
    });
  }
}

template <typename algo, typename OutputIt>
void benchmark_assign_storage_site(OutputIt out) {
  benchmark_assign_storage_site_<algo, uint32_t, 4096>(out);
  benchmark_assign_storage_site_<algo, uint32_t, 1024>(out);
  benchmark_assign_storage_site_<algo, uint32_t, 256>(out);
  benchmark_assign_storage_site_<algo, uint32_t, 64>(out);

  benchmark_assign_storage_site_<algo, uint16_t, 4096>(out);
  benchmark_assign_storage_site_<algo, uint16_t, 1024>(out);
  benchmark_assign_storage_site_<algo, uint16_t, 256>(out);
  benchmark_assign_storage_site_<algo, uint16_t, 64>(out);

  benchmark_assign_storage_site_<algo, uint8_t, 4096>(out);
  benchmark_assign_storage_site_<algo, uint8_t, 1024>(out);
  benchmark_assign_storage_site_<algo, uint8_t, 256>(out);
  benchmark_assign_storage_site_<algo, uint8_t, 64>(out);

  benchmark_assign_storage_site_<algo, bool, 4096>(out);
  benchmark_assign_storage_site_<algo, bool, 1024>(out);
  benchmark_assign_storage_site_<algo, bool, 256>(out);
  benchmark_assign_storage_site_<algo, bool, 64>(out);
}

int run_benchmark() {
  using u32 = std::uint32_t;
  using dstream_circular_algo_ = downstream::dstream::circular_algo_<u32>;
  using dstream_compressing_algo_ = downstream::dstream::compressing_algo_<u32>;
  using dstream_steady_algo_ = downstream::dstream::steady_algo_<u32>;
  using dstream_stretched_algo_ = downstream::dstream::stretched_algo_<u32>;
  using dstream_tilted_algo_ = downstream::dstream::tilted_algo_<u32>;

  std::cout << benchmark_result::make_csv_header();
  auto out = std::ostream_iterator<benchmark_result>(std::cout);
  benchmark_assign_storage_site<control_throwaway_algo>(out);
  benchmark_assign_storage_site<dstream_stretched_algo>(out);
  benchmark_assign_storage_site<dstream_tilted_algo>(out);
  benchmark_assign_storage_site<dstream_circular_algo_>(out);
  benchmark_assign_storage_site<dstream_compressing_algo_>(out);
  benchmark_assign_storage_site<dstream_steady_algo_>(out);
  benchmark_assign_storage_site<dstream_stretched_algo_>(out);
  benchmark_assign_storage_site<dstream_tilted_algo_>(out);
  benchmark_assign_storage_site<doubling_steady_algo>(out);
  benchmark_assign_storage_site<doubling_tilted_algo>(out);
  benchmark_assign_storage_site<zhao_steady_algo>(out);
  benchmark_assign_storage_site<zhao_tilted_algo>(out);
  benchmark_assign_storage_site<zhao_tilted_full_algo>(out);
  return 0;
}
#endif // #ifndef BENCHMARK_HPP_INCLUDE
