/**
 * @file int128.h
 * @brief Polyfills for 128-bit integers (`__int128`).
 *
 * Provides standard ABI-compliant definitions and operations for 128-bit
 * integers.
 */

#ifndef C_CDD_INT128_H
#define C_CDD_INT128_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#if defined(_MSC_VER) && _MSC_VER < 1800
/* clang-format off */
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "c_cdd/msvc/stdint.h"
#else
#include <stdint.h>
#endif
#endif
/* clang-format on */
#endif
#endif

/**
 * @brief 128-bit unsigned integer structure.
 */
typedef struct __cdd_uint128 {
  uint64_t low;  /**< Lower 64 bits */
  uint64_t high; /**< Upper 64 bits */
} cdd_uint128_t;

/**
 * @brief 128-bit signed integer structure.
 */
typedef struct __cdd_int128 {
  uint64_t low; /**< Lower 64 bits */
  int64_t high; /**< Upper 64 bits (signed) */
} cdd_int128_t;

/**
 * @brief Creates a 128-bit unsigned integer from two 64-bit halves.
 *
 * @param high Upper 64 bits.
 * @param low Lower 64 bits.
 * @return The constructed 128-bit unsigned integer.
 */
static cdd_uint128_t cdd_make_uint128(uint64_t high, uint64_t low) {
  cdd_uint128_t res;
  res.high = high;
  res.low = low;
  return res;
}

/**
 * @brief Creates a 128-bit signed integer from two 64-bit halves.
 *
 * @param high Upper 64 bits.
 * @param low Lower 64 bits.
 * @return The constructed 128-bit signed integer.
 */
static cdd_int128_t cdd_make_int128(int64_t high, uint64_t low) {
  cdd_int128_t res;
  res.high = high;
  res.low = low;
  return res;
}

#if defined(__SIZEOF_INT128__)
__extension__ typedef __int128 cdd_native_int128_t;
__extension__ typedef unsigned __int128 cdd_native_uint128_t;

#define cdd_make_int128_from_native(val)                                       \
  cdd_make_int128((int64_t)((val) >> 64), (uint64_t)(val))

#define cdd_make_int128_from_native_u(val)                                     \
  cdd_make_int128((int64_t)((val) >> 64), (uint64_t)(val))

/**
 * @brief Polyfill for extracting 128-bit ints from va_list.
 * Since cdd_int128_t is a struct, standard C ABI rules for structs apply,
 * which differ from native __int128 rules. This macro bridges the gap.
 */
#define CDD_VA_ARG_INT128(ap, is_signed)                                       \
  ((is_signed)                                                                 \
       ? cdd_make_int128_from_native(va_arg(ap, cdd_native_int128_t))          \
       : cdd_make_int128_from_native_u(va_arg(ap, cdd_native_uint128_t)))
#else
/**
 * @brief Polyfill for extracting 128-bit ints from va_list.
 * Since cdd_int128_t is a struct, standard C ABI rules for structs apply,
 * which differ from native __int128 rules. This macro bridges the gap.
 */
#define CDD_VA_ARG_INT128(ap, is_signed) va_arg(ap, cdd_int128_t)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_INT128_H */
