#pragma once

#include <array>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <vector>

#include "./downstream/include/downstream/_auxlib/modpow2.hpp"
#include "./downstream/include/downstream/dstream/dstream.hpp"

template <class Tp> inline void DoNotOptimize(Tp const &value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

template <class Tp> inline void DoNotOptimize(Tp &value) {
#if defined(__clang__)
  asm volatile("" : "+r,m"(value) : : "memory");
#else
  asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

constexpr std::string_view get_compiler_name() {
  return
#ifdef __clang__
      "clang++";
#elif defined(__GNUC__)
      "g++";
#else
      "unknown";
#endif
}

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

struct naive_steady_algo {
  static std::string_view get_algo_name() { return "naive_steady_algo"; }
};

struct zhao_steady_algo {
  static std::string_view get_algo_name() { return "zhao_steady_algo"; }
};

struct zhao_tilted_algo {
  static std::string_view get_algo_name() { return "zhao_tilted_algo"; }
};

struct control_ring_algo {
  static std::string_view get_algo_name() { return "control_ring_algo"; }
  static uint32_t _assign_storage_site(const uint32_t S, const uint32_t T) {
    return downstream::_auxlib::modpow2(T, S);
  }
};

struct control_throwaway_algo {
  static std::string_view get_algo_name() { return "control_throwaway_algo"; }
  static uint32_t _assign_storage_site(const uint32_t S, const uint32_t T) {
    return S;
  }
};

template <typename T> size_t sizeof_vector(const std::vector<T> &vec) {
  return sizeof(vec) + vec.size() * sizeof(T);
}

template <> size_t sizeof_vector(const std::vector<bool> &vec) {
  return sizeof(vec) + (vec.size() + 7) / 8;
}

template <typename dtype> dtype downcast_value(const uint32_t value) {
  if constexpr (std::is_same_v<dtype, bool>) {
    return static_cast<bool>(value & 1);
  } else
    return static_cast<dtype>(value);
}

template <typename dtype> std::string_view name_value() {
  if constexpr (std::is_same_v<dtype, bool>) {
    return "bit";
  } else if constexpr (std::is_same_v<dtype, uint8_t>) {
    return "byte";
  } else if constexpr (std::is_same_v<dtype, uint16_t>) {
    return "word";
  } else if constexpr (std::is_same_v<dtype, uint32_t>) {
    return "double word";
  } else if constexpr (std::is_same_v<dtype, uint64_t>) {
    return "quad word";
  } else
    static_assert(false);
}

// adapted from https://en.wikipedia.org/wiki/Xorshift
struct xorshift_generator {

  uint32_t state;

  xorshift_generator() : state(0xdeadbeef) {}

  uint32_t operator()() {
    uint32_t x = this->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    this->state = x;
    return x;
  }
};

template <typename dtype, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_naive_assign_storage_site(const uint32_t num_items) {
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

    if (storage.size() <= num_sites)
      continue;

    const auto indexRange = std::views::iota(size_t{}, storage.size() - 1);
    const auto collapse_idx = *std::ranges::min_element(
        indexRange, [&](std::size_t i1, std::size_t i2) {
          const auto sum1 = segment_lengths[i1] + segment_lengths[i1 + 1];
          const auto sum2 = segment_lengths[i2] + segment_lengths[i2 + 1];
          return sum1 < sum2;
        });

    segment_lengths[collapse_idx] += segment_lengths[collapse_idx + 1];
    storage.erase(std::next(std::begin(storage), collapse_idx + 1));
    segment_lengths.erase(
        std::next(std::begin(segment_lengths), collapse_idx + 1));
  }

  DoNotOptimize(storage);
  DoNotOptimize(gen.state);
  return sizeof_vector(storage) + sizeof_vector(segment_lengths);
}

template <typename dtype, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_zhao_assign_storage_site(const uint32_t num_items) {
  std::vector<uint32_t> segment_lengths;
  std::vector<dtype> storage;
  segment_lengths.reserve(num_sites);
  storage.reserve(num_sites);
  DoNotOptimize(storage);

  xorshift_generator gen{};
  for (uint32_t i = 0; i < num_items; ++i) {
    const auto data = downcast_value<dtype>(gen());

    if (storage.size() < num_sites) {
      storage.push_back(data);
      segment_lengths.push_back(1);
      continue;
    }

    if (segment_lengths[num_sites - 1] < segment_lengths[num_sites - 2]) {
      segment_lengths[num_sites - 1] += 1;
      continue;
    }

    storage.push_back(data);
    segment_lengths.push_back(1);

    const auto indexRange = std::views::iota(size_t{}, storage.size() - 1);
    const auto collapse_idx = *std::ranges::min_element(
        indexRange, [&](std::size_t i1, std::size_t i2) {
          const auto sum1 = segment_lengths[i1] + segment_lengths[i1 + 1];
          const auto sum2 = segment_lengths[i2] + segment_lengths[i2 + 1];
          return sum1 < sum2;
        });

    segment_lengths[collapse_idx] += segment_lengths[collapse_idx + 1];
    storage.erase(std::next(std::begin(storage), collapse_idx + 1));
    segment_lengths.erase(
        std::next(std::begin(segment_lengths), collapse_idx + 1));
  }

  DoNotOptimize(storage);
  DoNotOptimize(gen.state);
  return sizeof_vector(storage) + sizeof_vector(segment_lengths);
}

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
struct execute_assign_storage_site<dtype, num_sites, naive_steady_algo> {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_naive_assign_storage_site<dtype, num_sites>(num_items);
  }
};

template <typename dtype, uint32_t num_sites>
struct execute_assign_storage_site<dtype, num_sites, zhao_steady_algo> {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_zhao_assign_storage_site<dtype, num_sites>(num_items);
  }
};

template <typename dtype, uint32_t num_sites>
struct execute_assign_storage_site<dtype, num_sites, zhao_tilted_algo> {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_zhao_tilted_assign_storage_site<dtype, num_sites>(num_items);
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
  const uint32_t num_replicates = 20;
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
  #ifndef __arm__  // skip on pico, to avoid out of memory error
  benchmark_assign_storage_site_<algo, uint64_t, 4096>(out);
  benchmark_assign_storage_site_<algo, uint64_t, 1024>(out);
  benchmark_assign_storage_site_<algo, uint64_t, 256>(out);
  benchmark_assign_storage_site_<algo, uint64_t, 64>(out);
  #endif

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

int dispatch() {
  using dstream_steady_algo = downstream::dstream::steady_algo_<uint32_t>;
  using dstream_stretched_algo = downstream::dstream::stretched_algo_<uint32_t>;
  using dstream_tilted_algo = downstream::dstream::tilted_algo_<uint32_t>;

  std::vector<benchmark_result> results;
  auto inserter = std::back_inserter(results);
  benchmark_assign_storage_site<control_ring_algo>(inserter);
  benchmark_assign_storage_site<control_throwaway_algo>(inserter);
  benchmark_assign_storage_site<dstream_steady_algo>(inserter);
  benchmark_assign_storage_site<dstream_stretched_algo>(inserter);
  benchmark_assign_storage_site<dstream_tilted_algo>(inserter);
  // benchmark_assign_storage_site<naive_steady_algo>(inserter);
  benchmark_assign_storage_site<zhao_steady_algo>(inserter);
  benchmark_assign_storage_site<zhao_tilted_algo>(inserter);

  std::cout << benchmark_result::make_csv_header();
  for (const auto &result : results)
    std::cout << result.make_csv_row();
  return 0;
}
