/**
 * @file test_int128.h
 * @brief Unit tests and mocks in test_int128.h.
 */
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

/**
 * @brief Unit test test_int128_constructors.
 * @return Greatest test result.
 */
TEST test_int128_constructors(void) {
  cdd_uint128_t u128;
  cdd_int128_t i128;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_make_uint128((uint64_t)1, (uint64_t)2, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_make_int128((int64_t)-1, (uint64_t)2, NULL));

  rc = cdd_make_uint128((uint64_t)1, (uint64_t)2, &u128);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ_FMT(1, (int)u128.high, "%d");
  ASSERT_EQ_FMT(2, (int)u128.low, "%d");

  rc = cdd_make_int128((int64_t)-1, (uint64_t)2, &i128);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ_FMT(-1, (int)i128.high, "%d");
  ASSERT_EQ_FMT(2, (int)i128.low, "%d");

  PASS();
}

/**
 * @brief Helper function test_va_arg_helper.
 * @param[out] out Parameter out.
 * @param[in] count Parameter count.
 * @param[in] is_signed Parameter is_signed.
 * @return Result status or value.
 */
static cdd_c_error_t test_va_arg_helper(cdd_int128_t *out, int count,
                                        int is_signed, ...) {
  va_list ap;
  cdd_c_error_t rc;
  va_start(ap, is_signed);
  rc = CDD_VA_ARG_INT128_INTO(ap, is_signed, out);
  va_end(ap);
  (void)count;
  return rc;
}

/**
 * @brief Unit test test_int128_va_arg.
 * @return Greatest test result.
 */
TEST test_int128_va_arg(void) {
  cdd_int128_t res;
  cdd_c_error_t rc;

#if defined(__SIZEOF_INT128__)
  cdd_native_int128_t n_i128 =
      (cdd_native_int128_t)(((cdd_native_uint128_t)-1 << 64) | 2);
  cdd_native_uint128_t n_u128 = ((cdd_native_uint128_t)1 << 64) | 2;

  rc = test_va_arg_helper(&res, 1, 1, n_i128);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ_FMT(-1, (int)res.high, "%d");
  ASSERT_EQ_FMT(2, (int)res.low, "%d");

  rc = test_va_arg_helper(&res, 1, 0, n_u128);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ_FMT(1, (int)res.high, "%d");
  ASSERT_EQ_FMT(2, (int)res.low, "%d");
#else
  cdd_int128_t i128;
  rc = cdd_make_int128(-1, 2, &i128);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = test_va_arg_helper(&res, 1, 1, i128);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ_FMT(-1, (int)res.high, "%d");
  ASSERT_EQ_FMT(2, (int)res.low, "%d");
#endif
  PASS();
}

/**
 * @brief Executes the c_cdd_int128_suite test suite.
 */
SUITE(c_cdd_int128_suite) {
  RUN_TEST(test_int128_constructors);
  RUN_TEST(test_int128_va_arg);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_INT128_H */
