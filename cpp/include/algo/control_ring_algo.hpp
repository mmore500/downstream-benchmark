#pragma once

#include <cstdint>
#include <string_view>

#include "../../downstream/include/downstream/_auxlib/modpow2.hpp"

struct control_ring_algo {
  static std::string_view get_algo_name() { return "control_ring_algo"; }
  static uint32_t _assign_storage_site(const uint32_t S, const uint32_t T) {
    return downstream::_auxlib::modpow2(T, S);
  }
};
