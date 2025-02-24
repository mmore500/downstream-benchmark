#pragma once
#ifndef AUX_GET_COMPILER_NAME_HPP_INCLUDE
#define AUX_GET_COMPILER_NAME_HPP_INCLUDE

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
#endif // #ifndef AUX_GET_COMPILER_NAME_HPP_INCLUDE
