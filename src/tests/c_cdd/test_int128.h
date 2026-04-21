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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C_CDD_INT128_H */
