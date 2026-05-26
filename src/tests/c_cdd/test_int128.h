extern int g_fail_io_after;
extern int g_io_calls;
#ifndef TEST_INT128_H
#define TEST_INT128_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd/int128.h"
#include "c_cdd/int128_math.h"
#include <greatest.h>
/* clang-format on */

TEST test_cdd_make_uint128(void) {
  cdd_uint128_t u =
      cdd_make_uint128(0x1234567890ABCDEFULL, 0xFEDCBA0987654321ULL);
  ASSERT_EQ(0x1234567890ABCDEFULL, u.high);
  ASSERT_EQ(0xFEDCBA0987654321ULL, u.low);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_make_int128(void) {
  cdd_int128_t i = cdd_make_int128(-1234567890LL, 0xFEDCBA0987654321ULL);
  ASSERT_EQ(-1234567890LL, i.high);
  ASSERT_EQ(0xFEDCBA0987654321ULL, i.low);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_add_sub(void) {
  cdd_uint128_t u1 = cdd_make_uint128(0, 10);
  cdd_uint128_t u2 = cdd_make_uint128(0, 20);
  cdd_uint128_t uout;
  cdd_int128_t i1 = cdd_make_int128(0, 10);
  cdd_int128_t i2 = cdd_make_int128(0, 20);
  cdd_int128_t iout;

  cdd_uint128_add(u1, u2, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(30, uout.low);

  cdd_uint128_sub(uout, u1, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(20, uout.low);

  cdd_int128_add(i1, i2, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(30, iout.low);

  cdd_int128_sub(iout, i1, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(20, iout.low);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_mul_div(void) {
  cdd_uint128_t u1 = cdd_make_uint128(0, 10);
  cdd_uint128_t u2 = cdd_make_uint128(0, 20);
  cdd_uint128_t uout;
  cdd_int128_t i1 = cdd_make_int128(0, 10);
  cdd_int128_t i2 = cdd_make_int128(0, 20);
  cdd_int128_t iout;

  cdd_uint128_mul(u1, u2, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(200, uout.low);

  cdd_uint128_div(uout, u1, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(20, uout.low);

  cdd_int128_mul(i1, i2, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(200, iout.low);

  cdd_int128_div(iout, i1, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(20, iout.low);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_mod_shl_shr(void) {
  cdd_uint128_t u1 = cdd_make_uint128(0, 25);
  cdd_uint128_t u2 = cdd_make_uint128(0, 10);
  cdd_uint128_t uout;
  cdd_int128_t i1 = cdd_make_int128(0, 25);
  cdd_int128_t i2 = cdd_make_int128(0, 10);
  cdd_int128_t iout;

  cdd_uint128_mod(u1, u2, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(5, uout.low);

  cdd_uint128_shl(u1, 1, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(50, uout.low);

  cdd_uint128_shr(uout, 1, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(25, uout.low);

  cdd_int128_mod(i1, i2, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(5, iout.low);

  cdd_int128_shl(i1, 1, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(50, iout.low);

  cdd_int128_shr(iout, 1, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(25, iout.low);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_bitwise(void) {
  cdd_uint128_t u1 = cdd_make_uint128(0, 3);
  cdd_uint128_t u2 = cdd_make_uint128(0, 1);
  cdd_uint128_t uout;
  cdd_int128_t i1 = cdd_make_int128(0, 3);
  cdd_int128_t i2 = cdd_make_int128(0, 1);
  cdd_int128_t iout;

  cdd_uint128_and(u1, u2, &uout);
  ASSERT_EQ(1, uout.low);
  cdd_uint128_or(u1, u2, &uout);
  ASSERT_EQ(3, uout.low);
  cdd_uint128_xor(u1, u2, &uout);
  ASSERT_EQ(2, uout.low);
  cdd_uint128_not(u2, &uout);
  ASSERT_EQ(~1ULL, uout.low);

  cdd_int128_and(i1, i2, &iout);
  ASSERT_EQ(1, iout.low);
  cdd_int128_or(i1, i2, &iout);
  ASSERT_EQ(3, iout.low);
  cdd_int128_xor(i1, i2, &iout);
  ASSERT_EQ(2, iout.low);
  cdd_int128_not(i2, &iout);
  ASSERT_EQ(~1ULL, iout.low);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_casts(void) {
  cdd_uint128_t u;
  cdd_int128_t i;
  uint64_t u64;
  int64_t i64;
  float f;
  double d;

  cdd_uint64_to_uint128(10, &u);
  cdd_int64_to_int128(-10, &i);
  cdd_uint128_to_uint64(u, &u64);
  cdd_int128_to_int64(i, &i64);
  cdd_float_to_int128(10.5f, &i);
  cdd_double_to_int128(10.5, &i);
  cdd_int128_to_float(i, &f);
  cdd_int128_to_double(i, &d);

  ASSERT_EQ(10, u64);
  ASSERT_EQ(-10, i64);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_math_div_by_zero(void) {
  cdd_uint128_t u1 = cdd_make_uint128(0, 10);
  cdd_uint128_t u2 = cdd_make_uint128(0, 0);
  cdd_uint128_t uout;
  ASSERT_EQ(1, cdd_uint128_div(u1, u2, &uout));
  g_fail_io_after = -1;
  PASS();
}

SUITE(c_cdd_int128_suite) {
  RUN_TEST(test_cdd_make_uint128);
  RUN_TEST(test_cdd_make_int128);
  RUN_TEST(test_cdd_math_add_sub);
  RUN_TEST(test_cdd_math_mul_div);
  RUN_TEST(test_cdd_math_mod_shl_shr);
  RUN_TEST(test_cdd_math_bitwise);
  RUN_TEST(test_cdd_math_casts);
  RUN_TEST(test_cdd_math_div_by_zero);
}

#ifdef __cplusplus
}
#endif

#endif
