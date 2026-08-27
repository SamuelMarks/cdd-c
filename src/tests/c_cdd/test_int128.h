#ifndef TEST_INT128_H
#define TEST_INT128_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd_export.h"
#include "c_cdd/int128.h"
#include "c_cdd/int128_math.h"
#include <greatest.h>
/* clang-format on */

TEST test_cdd_make_uint128(void) {
  cdd_uint128_t u =
      cdd_make_uint128((((uint64_t)0x12345678 << 32) | (uint64_t)0x90ABCDEF),
                       (((uint64_t)0xFEDCBA09 << 32) | (uint64_t)0x87654321));
  ASSERT_EQ((((uint64_t)0x12345678 << 32) | (uint64_t)0x90ABCDEF), u.high);
  ASSERT_EQ((((uint64_t)0xFEDCBA09 << 32) | (uint64_t)0x87654321), u.low);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_make_int128(void) {
  cdd_int128_t i = cdd_make_int128(
      ((int64_t)(((uint64_t)0xFFFFFFFF << 32) | (uint64_t)0xB669FD2E)),
      (((uint64_t)0xFEDCBA09 << 32) | (uint64_t)0x87654321));
  ASSERT_EQ(((int64_t)(((uint64_t)0xFFFFFFFF << 32) | (uint64_t)0xB669FD2E)),
            i.high);
  ASSERT_EQ((((uint64_t)0xFEDCBA09 << 32) | (uint64_t)0x87654321), i.low);
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

TEST test_cdd_math_add_sub_carry(void) {
  cdd_uint128_t u1 = cdd_make_uint128(0, (((uint64_t)-1)));
  cdd_uint128_t u2 = cdd_make_uint128(0, 1);
  cdd_uint128_t uout;
  cdd_int128_t i1 = cdd_make_int128(0, (((uint64_t)-1)));
  cdd_int128_t i2 = cdd_make_int128(0, 1);
  cdd_int128_t iout;

  cdd_uint128_add(u1, u2, &uout);
  ASSERT_EQ(1, uout.high);
  ASSERT_EQ(0, uout.low);

  cdd_uint128_sub(uout, u2, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ((((uint64_t)-1)), uout.low);

  cdd_int128_add(i1, i2, &iout);
  ASSERT_EQ(1, iout.high);
  ASSERT_EQ(0, iout.low);

  cdd_int128_sub(iout, i2, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ((((uint64_t)-1)), iout.low);
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

TEST test_cdd_math_mul_carry(void) {
  cdd_uint128_t u1 = cdd_make_uint128(0, (((uint64_t)-1)));
  cdd_uint128_t u2 = cdd_make_uint128(0, (((uint64_t)-1)));
  cdd_uint128_t uout;

  cdd_uint128_mul(u1, u2, &uout);
  ASSERT_EQ((((uint64_t)-2)), uout.high);
  ASSERT_EQ(1, uout.low);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_div_mod_neg(void) {
  cdd_int128_t i1 = cdd_make_int128(((int64_t)-1), (((uint64_t)-10))); /* -10 */
  cdd_int128_t i2 = cdd_make_int128(0, 2);
  cdd_int128_t iout;

  cdd_int128_div(i1, i2, &iout);
  ASSERT_EQ(((int64_t)-1), iout.high);
  ASSERT_EQ((((uint64_t)-5)), iout.low);

  cdd_int128_mod(i1, i2, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(0, iout.low);

  cdd_int128_div(i2, i1, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(0, iout.low);

  cdd_int128_mod(i2, i1, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(2, iout.low);

  i2 = cdd_make_int128(((int64_t)-1), (((uint64_t)-2))); /* -2 */
  cdd_int128_div(i1, i2, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(5, iout.low);

  cdd_int128_mod(i1, i2, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ(0, iout.low);
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

TEST test_cdd_math_shl_shr_edge(void) {
  cdd_uint128_t u1 =
      cdd_make_uint128((((uint64_t)0x12345678 << 32) | (uint64_t)0x90ABCDEF),
                       (((uint64_t)0xFEDCBA09 << 32) | (uint64_t)0x87654321));
  cdd_uint128_t uout;
  cdd_int128_t i1 = cdd_make_int128(
      ((int64_t)(((uint64_t)0x12345678 << 32) | (uint64_t)0x90ABCDEF)),
      (((uint64_t)0xFEDCBA09 << 32) | (uint64_t)0x87654321));
  cdd_int128_t ineg = cdd_make_int128(
      ((int64_t)-1), (((uint64_t)0xFEDCBA09 << 32) | (uint64_t)0x87654321));
  cdd_int128_t iout;

  cdd_uint128_shl(u1, 0, &uout);
  ASSERT_EQ(u1.high, uout.high);
  ASSERT_EQ(u1.low, uout.low);
  cdd_uint128_shr(u1, 0, &uout);
  ASSERT_EQ(u1.high, uout.high);
  ASSERT_EQ(u1.low, uout.low);

  cdd_uint128_shl(u1, 68, &uout);
  ASSERT_EQ((((uint64_t)0xFEDCBA09 << 32) | (uint64_t)0x87654321) << 4,
            uout.high);
  ASSERT_EQ(0, uout.low);

  cdd_uint128_shr(u1, 68, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ((((uint64_t)0x12345678 << 32) | (uint64_t)0x90ABCDEF) >> 4,
            uout.low);

  cdd_int128_shl(i1, 0, &iout);
  ASSERT_EQ(i1.high, iout.high);
  ASSERT_EQ(i1.low, iout.low);
  cdd_int128_shr(i1, 0, &iout);
  ASSERT_EQ(i1.high, iout.high);
  ASSERT_EQ(i1.low, iout.low);

  cdd_int128_shl(i1, 68, &iout);
  ASSERT_EQ((((uint64_t)0xFEDCBA09 << 32) | (uint64_t)0x87654321) << 4,
            (uint64_t)iout.high);
  ASSERT_EQ(0, iout.low);

  cdd_int128_shr(i1, 68, &iout);
  ASSERT_EQ(0, iout.high);
  ASSERT_EQ((((uint64_t)0x12345678 << 32) | (uint64_t)0x90ABCDEF) >> 4,
            iout.low);

  cdd_int128_shr(ineg, 68, &iout);
  ASSERT_EQ(((int64_t)-1), iout.high);
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
  ASSERT_EQ(~(((uint64_t)0x0 << 32) | (uint64_t)0x1), uout.low);

  cdd_int128_and(i1, i2, &iout);
  ASSERT_EQ(1, iout.low);
  cdd_int128_or(i1, i2, &iout);
  ASSERT_EQ(3, iout.low);
  cdd_int128_xor(i1, i2, &iout);
  ASSERT_EQ(2, iout.low);
  cdd_int128_not(i2, &iout);
  ASSERT_EQ(~(((uint64_t)0x0 << 32) | (uint64_t)0x1), iout.low);
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

TEST test_cdd_math_casts_neg(void) {
  cdd_int128_t i;
  cdd_float_to_int128(-10.5f, &i);
  ASSERT_EQ(((int64_t)-1), i.high);
  cdd_double_to_int128(-10.5, &i);
  ASSERT_EQ(((int64_t)-1), i.high);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_div_large_quotient(void) {
  cdd_uint128_t u1 = cdd_make_uint128((((uint64_t)-1)), 0);
  cdd_uint128_t u2 = cdd_make_uint128(0, 2);
  cdd_uint128_t uout;
  cdd_uint128_div(u1, u2, &uout);
  ASSERT_EQ((((uint64_t)0x7FFFFFFF << 32) | (uint64_t)0xFFFFFFFF), uout.high);
  ASSERT_EQ((((uint64_t)0x80000000 << 32) | (uint64_t)0x0), uout.low);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_div_by_zero(void) {
  cdd_uint128_t u1 = cdd_make_uint128(0, 10);
  cdd_uint128_t u2 = cdd_make_uint128(0, 0);
  cdd_uint128_t uout;
  cdd_int128_t i1 = cdd_make_int128(0, 10);
  cdd_int128_t i2 = cdd_make_int128(0, 0);
  cdd_int128_t iout;
  ASSERT_EQ(1, cdd_uint128_div(u1, u2, &uout));
  ASSERT_EQ(1, cdd_int128_div(i1, i2, &iout));
  ASSERT_EQ(1, cdd_int128_mod(i1, i2, &iout));
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_div_large_den(void) {
  cdd_uint128_t u1 = cdd_make_uint128(5, 0);
  cdd_uint128_t u2 = cdd_make_uint128(2, 0);
  cdd_uint128_t uout;
  cdd_uint128_t urem;

  cdd_uint128_div(u1, u2, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(2, uout.low);

  cdd_uint128_mod(u1, u2, &urem);
  ASSERT_EQ(1, urem.high);
  ASSERT_EQ(0, urem.low);

  cdd_uint128_divmod(u1, u2, &uout, &urem);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(2, uout.low);
  ASSERT_EQ(1, urem.high);
  ASSERT_EQ(0, urem.low);

  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_math_div_branch_coverage(void) {
  /* Cover r.high > den.high */
  cdd_uint128_t u1 =
      cdd_make_uint128((((uint64_t)0xFFFFFFF << 32) | (uint64_t)0xFFFFFFFF), 0);
  cdd_uint128_t u2 =
      cdd_make_uint128((((uint64_t)0x0 << 32) | (uint64_t)0x2), 0);
  cdd_uint128_t uout;
  cdd_uint128_div(u1, u2, &uout);

  /* Cover r.high == den.high && r.low < den.low */
  u1 = cdd_make_uint128(0, 5);
  u2 = cdd_make_uint128(0, 10);
  cdd_uint128_div(u1, u2, &uout);

  /* Another edge case for divmod */
  u1 = cdd_make_uint128(10, 5);
  u2 = cdd_make_uint128(10, 10);
  cdd_uint128_div(u1, u2, &uout);

  g_fail_io_after = -1;
  PASS();
}

SUITE(c_cdd_int128_suite) {
  RUN_TEST(test_cdd_math_div_branch_coverage);
  RUN_TEST(test_cdd_math_div_large_den);
  RUN_TEST(test_cdd_make_uint128);
  RUN_TEST(test_cdd_make_int128);
  RUN_TEST(test_cdd_math_add_sub);
  RUN_TEST(test_cdd_math_add_sub_carry);
  RUN_TEST(test_cdd_math_mul_div);
  RUN_TEST(test_cdd_math_mul_carry);
  RUN_TEST(test_cdd_math_div_mod_neg);
  RUN_TEST(test_cdd_math_mod_shl_shr);
  RUN_TEST(test_cdd_math_shl_shr_edge);
  RUN_TEST(test_cdd_math_bitwise);
  RUN_TEST(test_cdd_math_casts);
  RUN_TEST(test_cdd_math_casts_neg);
  RUN_TEST(test_cdd_math_div_large_quotient);
  RUN_TEST(test_cdd_math_div_by_zero);
}

#ifdef __cplusplus
}
#endif

#endif
