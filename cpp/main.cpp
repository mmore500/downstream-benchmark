#include <array>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>
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

struct benchmark_result {
  std::string_view algo_name;
  uint32_t memory_bytes;
  uint32_t num_items;
  uint32_t num_sites;
  uint32_t replicate;
  double duration_ms;

  static std::string_view make_csv_header() {
    return ("algo_name, memory_bytes, num_items, "
            "num_sites, replicate, duration_ms\n");
  }

  std::string make_csv_row() const {
    return std::format("{}, {}, {}, {}, {}, {}\n", algo_name, memory_bytes,
                       num_items, num_sites, replicate, duration_ms);
  }
};

template <typename dstream_algo, uint32_t num_sites>
__attribute__((hot)) uint32_t
execute_dstream_assign_storage_site(const uint32_t num_items) {
  std::bitset<num_sites> storage_sites;
  for (uint32_t i = 0; i < num_items; ++i) {
    const auto k = dstream_algo::_assign_storage_site(num_sites, i);
    storage_sites[k] = i & 1;
  }

  DoNotOptimize(storage_sites);
  return sizeof(storage_sites) + sizeof(uint32_t /* i */);
}

template <typename dstream_algo, uint32_t num_sites>
benchmark_result time_dstream_assign_storage_site(const uint32_t replicate,
                                                  const uint32_t num_items) {
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;

  const auto t1 = high_resolution_clock::now();
  const auto memory_bytes =
      execute_dstream_assign_storage_site<dstream_algo, num_sites>(num_items);
  const auto t2 = high_resolution_clock::now();

  return {.algo_name = dstream_algo::get_algo_name(),
          .memory_bytes = memory_bytes,
          .num_items = num_items,
          .num_sites = num_sites,
          .replicate = replicate,
          .duration_ms =
              duration_cast<std::chrono::duration<double>>(t2 - t1).count()};
}

template <typename dstream_algo, uint32_t num_sites, typename OutputIt>
void benchmark_dstream_assign_storage_site_(OutputIt out) {
  const uint32_t num_replicates = 20;
  for (const uint32_t num_items : {1'000, 1'000'000}) {
    uint32_t replicate{};
    std::generate_n(out, num_replicates, [num_items, &replicate]() {
      return time_dstream_assign_storage_site<dstream_algo, num_sites>(
          replicate++, num_items);
    });
  }
}

template <typename dstream_algo, typename OutputIt>
void benchmark_dstream_assign_storage_site(OutputIt out) {
  benchmark_dstream_assign_storage_site_<dstream_algo, 64>(out);
  benchmark_dstream_assign_storage_site_<dstream_algo, 256>(out);
  benchmark_dstream_assign_storage_site_<dstream_algo, 1024>(out);
}

int main() {
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;

  using dstream_algo = downstream::dstream::steady_algo_<uint32_t>;
  std::vector<benchmark_result> results;
  benchmark_dstream_assign_storage_site<dstream_algo>(
      std::back_inserter(results));

  std::cout << benchmark_result::make_csv_header();
  for (const auto &result : results)
    std::cout << result.make_csv_row();
  return 0;
}
