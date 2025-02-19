#pragma once

#include <cstdint>
#include <type_traits>

template <typename dtype> dtype downcast_value(const uint32_t value) {
  if constexpr (std::is_same_v<dtype, bool>) {
    return static_cast<bool>(value & 1);
  } else
    return static_cast<dtype>(value);
}
