#ifndef TEST_INT128_H
#define TEST_INT128_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/c_cdd/int128.h"
#include "../../include/cdd_c_error.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

#include <stdarg.h>

TEST test_int128_constructors(void) {
  cdd_uint128_t u128;
  cdd_int128_t i128;

  u128 = cdd_make_uint128((uint64_t)1, (uint64_t)2);
  ASSERT_EQ_FMT(1, (int)u128.high, "%d");
  ASSERT_EQ_FMT(2, (int)u128.low, "%d");

  i128 = cdd_make_int128((int64_t)-1, (uint64_t)2);
  ASSERT_EQ_FMT(-1, (int)i128.high, "%d");
  ASSERT_EQ_FMT(2, (int)i128.low, "%d");

  PASS();
}

static cdd_int128_t test_va_arg_helper(int count, int is_signed, ...) {
  va_list ap;
  cdd_int128_t res;
  va_start(ap, is_signed);
  res = CDD_VA_ARG_INT128(ap, is_signed);
  va_end(ap);
  (void)count;
  return res;
}

TEST test_int128_va_arg(void) {
  cdd_int128_t res;

#if defined(__SIZEOF_INT128__)
  cdd_native_int128_t n_i128 =
      (cdd_native_int128_t)(((cdd_native_uint128_t)-1 << 64) | 2);
  cdd_native_uint128_t n_u128 = ((cdd_native_uint128_t)1 << 64) | 2;

  res = test_va_arg_helper(1, 1, n_i128);
  ASSERT_EQ_FMT(-1, (int)res.high, "%d");
  ASSERT_EQ_FMT(2, (int)res.low, "%d");

  res = test_va_arg_helper(1, 0, n_u128);
  ASSERT_EQ_FMT(1, (int)res.high, "%d");
  ASSERT_EQ_FMT(2, (int)res.low, "%d");
#else
  cdd_int128_t i128 = cdd_make_int128(-1, 2);
  res = test_va_arg_helper(1, 1, i128);
  ASSERT_EQ_FMT(-1, (int)res.high, "%d");
  ASSERT_EQ_FMT(2, (int)res.low, "%d");
#endif
  PASS();
}

SUITE(c_cdd_int128_suite) {
  RUN_TEST(test_int128_constructors);
  RUN_TEST(test_int128_va_arg);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_INT128_H */
