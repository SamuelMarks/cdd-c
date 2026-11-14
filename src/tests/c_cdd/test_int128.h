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
#include "../../include/c_cdd/int128_math.h"
#include "../../include/cdd_c_error.h"
#include "../cdd_test_helpers/cdd_helpers.h"
#include <stdarg.h>
/* clang-format on */

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
  if (count <= 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  va_start(ap, is_signed);
  rc = CDD_VA_ARG_INT128_INTO(ap, is_signed, out);
  va_end(ap);
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
 * @brief Exercises all arithmetic, bitwise, shift, negation, comparison, and
 * float/int64 conversion helpers in `int128_math.h`.
 * @return Greatest test result.
 */
TEST test_int128_math_all_operations(void) {
  cdd_uint128_t ua, ub, uq, ur, uout, uz;
  cdd_int128_t sa, sb, sout, sz;
  uint64_t u64 = 0;
  int64_t s64 = 0;
  float fv = 0.0f;
  double dv = 0.0;
  int cmp = 0;
  union {
    uint32_t u;
    float f;
  } fbits;
  union {
    uint64_t u;
    double d;
  } dbits;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(0, 0, &uz));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_make_int128(0, 0, &sz));

  /* NULL output pointer checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_add(uz, uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_add(sz, sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_sub(uz, uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_sub(sz, sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_mul(uz, uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_mul(sz, sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_shl(uz, 1, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_shr(uz, 1, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_shl(sz, 1, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_shr(sz, 1, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_div(uz, uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_mod(uz, uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_div(sz, sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_mod(sz, sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_and(uz, uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_and(sz, sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_or(uz, uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_or(sz, sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_xor(uz, uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_xor(sz, sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_not(uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_not(sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_neg(uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_neg(sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_cmp(uz, uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_cmp(sz, sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint64_to_uint128(0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int64_to_int128(0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_to_uint64(uz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_to_int64(sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_float_to_int128(1.0f, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_double_to_int128(1.0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_to_float(sz, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_to_double(sz, NULL));

  /* Carry & borrow in add/sub/mul */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(0, ~(uint64_t)0, &ua));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(0, 2, &ub));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_add(ua, ub, &uout));
  ASSERT_EQ(1U, (unsigned int)uout.high);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_sub(ub, ua, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_mul(ua, ua, &uout));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_make_int128(0, ~(uint64_t)0, &sa));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_make_int128(0, 2, &sb));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_add(sa, sb, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_sub(sb, sa, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_sub(sa, sb, &sout));

  /* Shifts: 0, <64, >=64 (signed positive & negative, unsigned) */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint64_to_uint128(100, &ua));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint64_to_uint128(7, &ub));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_add(ua, ub, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_to_uint64(uout, &u64));
  ASSERT_EQ(107U, (unsigned int)u64);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_sub(ua, ub, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_mul(ua, ub, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_shl(ua, 0, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_shl(ua, 4, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_shl(ua, 70, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_shr(uout, 0, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_shr(uout, 4, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_shr(uout, 66, &uout));

  /* Unsigned divmod: div-by-zero, high quotient bits, den.high > 0 */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_uint128_divmod(ua, uz, &uq, &ur));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_uint128_divmod(ua, uz, NULL, NULL));
  {
    cdd_uint128_t d_hi, n_hi;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(2, 10, &d_hi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(5, 5, &n_hi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_divmod(n_hi, d_hi, &uq, &ur));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(8, 0, &n_hi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_div(n_hi, ub, &uout));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_mod(ua, ub, &uout));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(0, ~(uint64_t)0, &d_hi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(3, 0, &n_hi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_divmod(n_hi, d_hi, &uq, &ur));
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_and(ua, ub, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_or(ua, ub, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_xor(ua, ub, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_not(ua, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_neg(uz, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_neg(ua, &uout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_neg(uout, &uout));

  /* Unsigned 3-way comparisons */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_cmp(ua, uout, &cmp));
  ASSERT_EQ(0, cmp);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_cmp(ub, ua, &cmp));
  ASSERT_EQ(-1, cmp);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_cmp(ua, ub, &cmp));
  ASSERT_EQ(1, cmp);
  {
    cdd_uint128_t h1, h2;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(1, 0, &h1));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_make_uint128(2, 0, &h2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_cmp(h1, h2, &cmp));
    ASSERT_EQ(-1, cmp);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_uint128_cmp(h2, h1, &cmp));
    ASSERT_EQ(1, cmp);
  }

  /* Signed arithmetic, shifts, div/mod sign combinations, neg, cmp */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int64_to_int128(-50, &sa));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int64_to_int128(8, &sb));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_add(sa, sb, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_to_int64(sout, &s64));
  ASSERT_EQ(-42, (int)s64);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_mul(sa, sb, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_shl(sa, 3, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_shr(sout, 0, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_shr(sout, 3, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_shr(sa, 70, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_shr(sb, 70, &sout));

  /* Signed div & mod: (-,+), (+,-), (-,-), (+,+), and div-by-zero */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_div(sa, sz, &sout));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_int128_mod(sa, sz, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_div(sa, sb, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_div(sb, sa, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_div(sa, sa, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_div(sb, sb, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_mod(sa, sb, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_mod(sb, sa, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_mod(sa, sa, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_mod(sb, sb, &sout));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_and(sa, sb, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_or(sa, sb, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_xor(sa, sb, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_not(sa, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_neg(sz, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_neg(sa, &sout));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_cmp(sa, sb, &cmp));
  ASSERT_EQ(-1, cmp);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_cmp(sb, sa, &cmp));
  ASSERT_EQ(1, cmp);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_cmp(sb, sb, &cmp));
  ASSERT_EQ(0, cmp);
  {
    cdd_int128_t s1, s2;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_int64_to_int128(3, &s1));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_cmp(s1, sb, &cmp));
    ASSERT_EQ(-1, cmp);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_cmp(sb, s1, &cmp));
    ASSERT_EQ(1, cmp);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_make_int128(-1, 0, &s2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_to_float(s2, &fv));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_to_double(s2, &dv));
  }

  /* Float <-> int128 edge cases */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_float_to_int128(0.0f, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_float_to_int128(0.25f, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_float_to_int128(8388608.0f, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_float_to_int128(16777216.0f, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_float_to_int128(-12.0f, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_to_float(sout, &fv));
  ASSERT(fv < -11.5f && fv > -12.5f);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_to_float(sb, &fv));
  fbits.u = 0x00000001u; /* subnormal float */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_float_to_int128(fbits.f, &sout));
  fbits.u = 0x7F800000u; /* +Inf */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_float_to_int128(fbits.f, &sout));
  fbits.u = 0x7F000000u; /* +2^127 overflow */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_float_to_int128(fbits.f, &sout));
  fbits.u = 0xFF000000u; /* -2^127 overflow */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_float_to_int128(fbits.f, &sout));

  /* Double <-> int128 edge cases */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_double_to_int128(0.0, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_double_to_int128(0.25, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_double_to_int128(4503599627370496.0, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_double_to_int128(-9007199254740992.0, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_double_to_int128(1024.0, &sout));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_to_double(sout, &dv));
  ASSERT(dv > 1023.5 && dv < 1024.5);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_int128_to_double(sa, &dv));
  dbits.u = (uint64_t)1; /* subnormal double */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_double_to_int128(dbits.d, &sout));
  dbits.u = ((uint64_t)0x7FFu) << 52; /* +Inf */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_double_to_int128(dbits.d, &sout));
  dbits.u = ((uint64_t)0x47Fu) << 52; /* +2^128 overflow */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_double_to_int128(dbits.d, &sout));
  dbits.u = ((uint64_t)0xC7Fu) << 52; /* -2^128 overflow */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_double_to_int128(dbits.d, &sout));

  PASS();
}

/**
 * @brief Executes the c_cdd_int128_suite test suite.
 */
SUITE(c_cdd_int128_suite) {
  RUN_TEST(test_int128_constructors);
  RUN_TEST(test_int128_va_arg);
  RUN_TEST(test_int128_math_all_operations);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_INT128_H */
