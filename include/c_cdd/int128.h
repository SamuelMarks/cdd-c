/**
 * @file int128.h
 * @brief Polyfills for 128-bit integers (`__int128`).
 *
 * Provides standard ABI-compliant definitions and operations for 128-bit integers.
 */

#ifndef C_CDD_INT128_H
#define C_CDD_INT128_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdint.h>
/* clang-format on */

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
  uint64_t low;  /**< Lower 64 bits */
  int64_t high;  /**< Upper 64 bits (signed) */
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_INT128_H */
