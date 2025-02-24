#pragma once
#ifndef AUX_DOWNCAST_VALUE_HPP_INCLUDE
#define AUX_DOWNCAST_VALUE_HPP_INCLUDE

#include <cstdint>
#include <type_traits>

template <typename dtype> dtype downcast_value(const uint32_t value) {
  if constexpr (std::is_same_v<dtype, bool>) {
    return static_cast<bool>(value & 1);
  } else
    return static_cast<dtype>(value);
}
#endif // #ifndef AUX_DOWNCAST_VALUE_HPP_INCLUDE
