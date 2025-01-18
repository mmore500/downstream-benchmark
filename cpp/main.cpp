#include <array>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <string_view>

#include "downstream/include/downstream/dstream/dstream.hpp"

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
  uint32_t memory_bytes;
  uint32_t num_items;
  uint32_t num_sites;
  uint32_t replicate;
  double duration_s;

  static std::string_view make_csv_header() {
    return ("algo_name, compiler, memory_bytes, num_items, "
            "num_sites, replicate, duration_s\n");
  }

  std::string make_csv_row() const {
    constexpr std::string_view compiler_name = get_compiler_name();
    return std::format("{}, {}, {}, {}, {}, {}, {}\n", algo_name, compiler_name,
                       memory_bytes, num_items, num_sites, replicate,
                       duration_s);
  }
};

struct naive_steady_algo {
  static std::string_view get_algo_name() { return "naive_steady_algo"; }
};

template <typename T> size_t sizeof_vector(const std::vector<T> &vec) {
  return sizeof(vec) + vec.size() * sizeof(T);
}

template <> size_t sizeof_vector(const std::vector<bool> &vec) {
  return sizeof(vec) + (vec.size() + 7) / 8;
}

template <uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_naive_assign_storage_site(const uint32_t num_items) {
  std::vector<uint32_t> segment_lengths;
  std::vector<bool> storage;
  segment_lengths.reserve(num_sites);
  storage.reserve(num_sites);

  for (uint32_t i = 0; i < num_items; ++i) {
    const bool data = i & 1;
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
  return sizeof_vector(storage) + sizeof_vector(segment_lengths);
}

template <typename dstream_algo, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_dstream_assign_storage_site(const uint32_t num_items) {
  std::bitset<num_sites> storage;
  for (uint32_t i = 0; i < num_items; ++i) {
    const auto k = dstream_algo::_assign_storage_site(num_sites, i);
    storage[k] = i & 1;
  }

  DoNotOptimize(storage);
  return sizeof(storage) + sizeof(uint32_t /* i */);
}

template <uint32_t num_sites, typename algo>
struct execute_assign_storage_site {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_dstream_assign_storage_site<algo, num_sites>(num_items);
  }
};

template <uint32_t num_sites>
struct execute_assign_storage_site<num_sites, naive_steady_algo> {
  static uint32_t operator()(const uint32_t num_items) {
    return execute_naive_assign_storage_site<num_sites>(num_items);
  }
};

template <typename algo, uint32_t num_sites>
benchmark_result time_assign_storage_site(const uint32_t replicate,
                                          const uint32_t num_items) {
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;

  const auto t1 = high_resolution_clock::now();
  using executor = execute_assign_storage_site<num_sites, algo>;
  const auto memory_bytes = executor::operator()(num_items);
  const auto t2 = high_resolution_clock::now();

  return {.algo_name = algo::get_algo_name(),
          .memory_bytes = memory_bytes,
          .num_items = num_items,
          .num_sites = num_sites,
          .replicate = replicate,
          .duration_s =
              duration_cast<std::chrono::duration<double>>(t2 - t1).count()};
}

template <typename algo, uint32_t num_sites, typename OutputIt>
void benchmark_assign_storage_site_(OutputIt out) {
  const uint32_t num_replicates = 20;
  for (const uint32_t num_items : {1'000, 1'000'000}) {
    uint32_t replicate{};
    std::generate_n(out, num_replicates, [num_items, &replicate]() {
      return time_assign_storage_site<algo, num_sites>(replicate++, num_items);
    });
  }
}

template <typename algo, typename OutputIt>
void benchmark_assign_storage_site(OutputIt out) {
  benchmark_assign_storage_site_<algo, 64>(out);
  benchmark_assign_storage_site_<algo, 256>(out);
  benchmark_assign_storage_site_<algo, 1024>(out);
  benchmark_assign_storage_site_<algo, 4096>(out);
}

int main() {
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;

  using dstream_steady_algo = downstream::dstream::steady_algo_<uint32_t>;
  std::vector<benchmark_result> results;
  auto inserter = std::back_inserter(results);
  benchmark_assign_storage_site<dstream_steady_algo>(inserter);
  benchmark_assign_storage_site<naive_steady_algo>(inserter);

  std::cout << benchmark_result::make_csv_header();
  for (const auto &result : results)
    std::cout << result.make_csv_row();
  return 0;
}
