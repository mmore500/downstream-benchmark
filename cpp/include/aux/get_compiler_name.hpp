#pragma once

#include <string_view>

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
