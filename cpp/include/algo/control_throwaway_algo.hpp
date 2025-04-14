#pragma once
#ifndef ALGO_CONTROL_THROWAWAY_ALGO_HPP_INCLUDE
#define ALGO_CONTROL_THROWAWAY_ALGO_HPP_INCLUDE

#include <cstdint>
#include <string_view>

struct control_throwaway_algo {
  static std::string_view get_algo_name() { return "control_throwaway_algo"; }
  static uint32_t _assign_storage_site(const uint32_t S, const uint32_t T) {
    return S;
  }
};
#endif // #ifndef ALGO_CONTROL_THROWAWAY_ALGO_HPP_INCLUDE
