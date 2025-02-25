#pragma once
#ifndef AUX_DONOTOPTIMIZE_HPP_INCLUDE
#define AUX_DONOTOPTIMIZE_HPP_INCLUDE

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
#endif // #ifndef AUX_DONOTOPTIMIZE_HPP_INCLUDE
