#pragma once
#ifndef AUX_NAME_VALUE_HPP_INCLUDE
#define AUX_NAME_VALUE_HPP_INCLUDE

#include <cstdint>
#include <string_view>
#include <type_traits>

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
#endif // #ifndef AUX_NAME_VALUE_HPP_INCLUDE
