/**
 * @file test_int128.h
 * @brief Unit tests for 128-bit integer math.
 */

#ifndef TEST_C_CDD_INT128_H
#define TEST_C_CDD_INT128_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include "../../../include/c_cdd/int128.h"
#include "../../../include/c_cdd/int128_math.h"
/* clang-format on */

TEST test_cdd_uint128_add(void) {
  cdd_uint128_t a = cdd_make_uint128(0, 0xFFFFFFFFFFFFFFFFULL);
  cdd_uint128_t b = cdd_make_uint128(0, 2);
  cdd_uint128_t res;
  ASSERT_EQ(0, cdd_uint128_add(a, b, &res));
  ASSERT_EQ(1, res.high);
  ASSERT_EQ(1, res.low);
  PASS();
}

TEST test_cdd_int128_add(void) {
  cdd_int128_t a = cdd_make_int128(0, 0xFFFFFFFFFFFFFFFFULL);
  cdd_int128_t b = cdd_make_int128(0, 2);
  cdd_int128_t res;
  ASSERT_EQ(0, cdd_int128_add(a, b, &res));
  ASSERT_EQ(1, res.high);
  ASSERT_EQ(1, res.low);
  PASS();
}

TEST test_cdd_uint128_sub(void) {
  cdd_uint128_t a = cdd_make_uint128(1, 1);
  cdd_uint128_t b = cdd_make_uint128(0, 2);
  cdd_uint128_t res;
  ASSERT_EQ(0, cdd_uint128_sub(a, b, &res));
  ASSERT_EQ(0, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.low);
  PASS();
}

TEST test_cdd_int128_sub(void) {
  cdd_int128_t a = cdd_make_int128(1, 1);
  cdd_int128_t b = cdd_make_int128(0, 2);
  cdd_int128_t res;
  ASSERT_EQ(0, cdd_int128_sub(a, b, &res));
  ASSERT_EQ(0, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.low);
  PASS();
}

TEST test_cdd_uint128_mul(void) {
  cdd_uint128_t a = cdd_make_uint128(0, 2);
  cdd_uint128_t b = cdd_make_uint128(0, 3);
  cdd_uint128_t res;
  ASSERT_EQ(0, cdd_uint128_mul(a, b, &res));
  ASSERT_EQ(0, res.high);
  ASSERT_EQ(6, res.low);

  a = cdd_make_uint128(0, 0xFFFFFFFFFFFFFFFFULL);
  b = cdd_make_uint128(0, 2);
  ASSERT_EQ(0, cdd_uint128_mul(a, b, &res));
  ASSERT_EQ(1, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFEULL, res.low);

  PASS();
}

TEST test_cdd_int128_mul(void) {
  cdd_int128_t a = cdd_make_int128(-1LL, 0xFFFFFFFFFFFFFFFFULL);
  cdd_int128_t b = cdd_make_int128(0, 2);
  cdd_int128_t res;
  ASSERT_EQ(0, cdd_int128_mul(a, b, &res));
  ASSERT_EQ(-1LL, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFEULL, res.low);
  PASS();
}

TEST test_cdd_uint128_shl(void) {
  cdd_uint128_t a = cdd_make_uint128(0, 1);
  cdd_uint128_t res;
  ASSERT_EQ(0, cdd_uint128_shl(a, 64, &res));
  ASSERT_EQ(1, res.high);
  ASSERT_EQ(0, res.low);
  PASS();
}

TEST test_cdd_uint128_shr(void) {
  cdd_uint128_t a = cdd_make_uint128(1, 0);
  cdd_uint128_t res;
  ASSERT_EQ(0, cdd_uint128_shr(a, 64, &res));
  ASSERT_EQ(0, res.high);
  ASSERT_EQ(1, res.low);
  PASS();
}

TEST test_cdd_int128_shl(void) {
  cdd_int128_t a = cdd_make_int128(0, 1);
  cdd_int128_t res;
  ASSERT_EQ(0, cdd_int128_shl(a, 64, &res));
  ASSERT_EQ(1, res.high);
  ASSERT_EQ(0, res.low);
  PASS();
}

TEST test_cdd_int128_shr(void) {
  cdd_int128_t a = cdd_make_int128(-1LL, 0);
  cdd_int128_t res;
  ASSERT_EQ(0, cdd_int128_shr(a, 64, &res));
  ASSERT_EQ(-1LL, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.low);
  PASS();
}

TEST test_cdd_uint128_div(void) {
  cdd_uint128_t a = cdd_make_uint128(1, 0);
  cdd_uint128_t b = cdd_make_uint128(0, 2);
  cdd_uint128_t res;
  ASSERT_EQ(0, cdd_uint128_div(a, b, &res));
  ASSERT_EQ(0, res.high);
  ASSERT_EQ(0x8000000000000000ULL, res.low);

  ASSERT_EQ(1, cdd_uint128_div(a, cdd_make_uint128(0, 0), &res));
  PASS();
}

TEST test_cdd_uint128_mod(void) {
  cdd_uint128_t a = cdd_make_uint128(0, 10);
  cdd_uint128_t b = cdd_make_uint128(0, 3);
  cdd_uint128_t res;
  ASSERT_EQ(0, cdd_uint128_mod(a, b, &res));
  ASSERT_EQ(0, res.high);
  ASSERT_EQ(1, res.low);
  PASS();
}

TEST test_cdd_int128_div(void) {
  cdd_int128_t a = cdd_make_int128(-1LL, 0xFFFFFFFFFFFFFFF6ULL); /* -10 */
  cdd_int128_t b = cdd_make_int128(0, 3);
  cdd_int128_t res;
  ASSERT_EQ(0, cdd_int128_div(a, b, &res));
  ASSERT_EQ(-1LL, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFDULL, res.low); /* -3 */
  PASS();
}

TEST test_cdd_int128_mod(void) {
  cdd_int128_t a = cdd_make_int128(-1LL, 0xFFFFFFFFFFFFFFF6ULL); /* -10 */
  cdd_int128_t b = cdd_make_int128(0, 3);
  cdd_int128_t res;
  ASSERT_EQ(0, cdd_int128_mod(a, b, &res));
  ASSERT_EQ(-1LL, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.low); /* -1 */
  PASS();
}

TEST test_cdd_uint128_bitwise(void) {
  cdd_uint128_t a =
      cdd_make_uint128(0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL);
  cdd_uint128_t b =
      cdd_make_uint128(0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL);
  cdd_uint128_t res;
  ASSERT_EQ(0, cdd_uint128_and(a, b, &res));
  ASSERT_EQ(0, res.high);
  ASSERT_EQ(0, res.low);
  ASSERT_EQ(0, cdd_uint128_or(a, b, &res));
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.low);
  ASSERT_EQ(0, cdd_uint128_xor(a, b, &res));
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.low);
  ASSERT_EQ(0, cdd_uint128_not(a, &res));
  ASSERT_EQ(0x5555555555555555ULL, res.high);
  ASSERT_EQ(0xAAAAAAAAAAAAAAAAULL, res.low);
  PASS();
}

TEST test_cdd_int128_bitwise(void) {
  cdd_int128_t a =
      cdd_make_int128(0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL);
  cdd_int128_t b =
      cdd_make_int128(0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL);
  cdd_int128_t res;
  ASSERT_EQ(0, cdd_int128_and(a, b, &res));
  ASSERT_EQ(0, res.high);
  ASSERT_EQ(0, res.low);
  ASSERT_EQ(0, cdd_int128_or(a, b, &res));
  ASSERT_EQ(-1LL, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.low);
  ASSERT_EQ(0, cdd_int128_xor(a, b, &res));
  ASSERT_EQ(-1LL, res.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, res.low);
  ASSERT_EQ(0, cdd_int128_not(a, &res));
  ASSERT_EQ(0x5555555555555555ULL, res.high);
  ASSERT_EQ(0xAAAAAAAAAAAAAAAAULL, res.low);
  PASS();
}

TEST test_cdd_128_casts(void) {
  cdd_uint128_t ures;
  cdd_int128_t ires;
  uint64_t u64;
  int64_t i64;
  float f;
  double d;
  ASSERT_EQ(0, cdd_uint64_to_uint128(0xFFFFFFFFFFFFFFFFULL, &ures));
  ASSERT_EQ(0, ures.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, ures.low);
  ASSERT_EQ(0, cdd_int64_to_int128(-1LL, &ires));
  ASSERT_EQ(-1LL, ires.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, ires.low);
  ASSERT_EQ(0, cdd_uint128_to_uint64(ures, &u64));
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, u64);
  ASSERT_EQ(0, cdd_int128_to_int64(ires, &i64));
  ASSERT_EQ(-1LL, i64);
  ASSERT_EQ(0, cdd_float_to_int128(-123.45f, &ires));
  ASSERT_EQ(-1LL, ires.high);
  printf("ires.low=%llx\n", (unsigned long long)ires.low);

  ASSERT_EQ_FMT(0xFFFFFFFFFFFFFF85ULL, ires.low, "%llx");
  ASSERT_EQ(0, cdd_double_to_int128(456.78, &ires));
  ASSERT_EQ(0, ires.high);
  ASSERT_EQ(456, ires.low);
  ASSERT_EQ(0, cdd_int128_to_float(cdd_make_int128(0, 100), &f));
  ASSERT_EQ(100.0f, f);
  ASSERT_EQ(0, cdd_int128_to_double(
                   cdd_make_int128(-1LL, 0xFFFFFFFFFFFFFF9CULL), &d));
  ASSERT_EQ(-100.0, d);
  PASS();
}

SUITE(c_cdd_int128_suite) {
  RUN_TEST(test_cdd_uint128_add);
  RUN_TEST(test_cdd_int128_add);
  RUN_TEST(test_cdd_uint128_sub);
  RUN_TEST(test_cdd_int128_sub);
  RUN_TEST(test_cdd_uint128_mul);
  RUN_TEST(test_cdd_int128_mul);
  RUN_TEST(test_cdd_uint128_shl);
  RUN_TEST(test_cdd_uint128_shr);
  RUN_TEST(test_cdd_int128_shl);
  RUN_TEST(test_cdd_int128_shr);
  RUN_TEST(test_cdd_uint128_div);
  RUN_TEST(test_cdd_uint128_mod);
  RUN_TEST(test_cdd_int128_div);
  RUN_TEST(test_cdd_int128_mod);
  RUN_TEST(test_cdd_uint128_bitwise);
  RUN_TEST(test_cdd_int128_bitwise);
  RUN_TEST(test_cdd_128_casts);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C_CDD_INT128_H */
