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
      cdd_make_uint128(0x1234567890ABCDEFULL, 0xFEDCBA0987654321ULL);
  ASSERT_EQ(0x1234567890ABCDEFULL, u.high);
  ASSERT_EQ(0xFEDCBA0987654321ULL, u.low);

  PASS();
}

TEST test_cdd_make_int128(void) {
  cdd_int128_t i = cdd_make_int128(-1234567890LL, 0xFEDCBA0987654321ULL);
  ASSERT_EQ(-1234567890LL, i.high);
  ASSERT_EQ(0xFEDCBA0987654321ULL, i.low);

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

  PASS();
}

TEST test_cdd_math_div_by_zero(void) {
  cdd_uint128_t u1 = cdd_make_uint128(0, 10);
  cdd_uint128_t u2 = cdd_make_uint128(0, 0);
  cdd_uint128_t uout;
  ASSERT_EQ(1, cdd_uint128_div(u1, u2, &uout));

  PASS();
}

TEST test_uint128_add(void) {
  cdd_uint128_t a = cdd_make_uint128(1, 0xFFFFFFFFFFFFFFFFULL);
  cdd_uint128_t b = cdd_make_uint128(0, 1);
  cdd_uint128_t out;
  cdd_uint128_add(a, b, &out);
  ASSERT_EQ(2, out.high);
  ASSERT_EQ(0, out.low);
  PASS();
}

TEST test_int128_add(void) {
  cdd_int128_t a = cdd_make_int128(1, 0xFFFFFFFFFFFFFFFFULL);
  cdd_int128_t b = cdd_make_int128(0, 1);
  cdd_int128_t out;
  cdd_int128_add(a, b, &out);
  ASSERT_EQ(2, out.high);
  ASSERT_EQ(0, out.low);
  PASS();
}

TEST test_uint128_sub(void) {
  cdd_uint128_t a = cdd_make_uint128(2, 0);
  cdd_uint128_t b = cdd_make_uint128(0, 1);
  cdd_uint128_t out;
  cdd_uint128_sub(a, b, &out);
  ASSERT_EQ(1, out.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, out.low);
  PASS();
}

TEST test_int128_sub(void) {
  cdd_int128_t a = cdd_make_int128(2, 0);
  cdd_int128_t b = cdd_make_int128(0, 1);
  cdd_int128_t out;
  cdd_int128_sub(a, b, &out);
  ASSERT_EQ(1, out.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, out.low);
  PASS();
}

TEST test_uint128_mul(void) {
  cdd_uint128_t a = cdd_make_uint128(0, 0x100000000ULL);
  cdd_uint128_t b = cdd_make_uint128(0, 0x100000000ULL);
  cdd_uint128_t out;
  cdd_uint128_mul(a, b, &out);
  ASSERT_EQ(1, out.high);
  ASSERT_EQ(0, out.low);
  PASS();
}

TEST test_int128_mul(void) {
  cdd_int128_t a = cdd_make_int128(0, 0x100000000ULL);
  cdd_int128_t b = cdd_make_int128(0, 0x100000000ULL);
  cdd_int128_t out;
  cdd_int128_mul(a, b, &out);
  ASSERT_EQ(1, out.high);
  ASSERT_EQ(0, out.low);
  PASS();
}

TEST test_uint128_shl(void) {
  cdd_uint128_t a = cdd_make_uint128(0, 1);
  cdd_uint128_t out;

  cdd_uint128_shl(a, 0, &out);
  ASSERT_EQ(0, out.high);
  ASSERT_EQ(1, out.low);

  cdd_uint128_shl(a, 63, &out);
  ASSERT_EQ(0, out.high);
  ASSERT_EQ(0x8000000000000000ULL, out.low);

  cdd_uint128_shl(a, 64, &out);
  ASSERT_EQ(1, out.high);
  ASSERT_EQ(0, out.low);
  PASS();
}

TEST test_uint128_shr(void) {
  cdd_uint128_t a = cdd_make_uint128(1, 0);
  cdd_uint128_t out;

  cdd_uint128_shr(a, 0, &out);
  ASSERT_EQ(1, out.high);
  ASSERT_EQ(0, out.low);

  cdd_uint128_shr(a, 1, &out);
  ASSERT_EQ(0, out.high);
  ASSERT_EQ(0x8000000000000000ULL, out.low);

  cdd_uint128_shr(a, 64, &out);
  ASSERT_EQ(0, out.high);
  ASSERT_EQ(1, out.low);
  PASS();
}

TEST test_int128_shl(void) {
  cdd_int128_t a = cdd_make_int128(0, 1);
  cdd_int128_t out;
  cdd_int128_shl(a, 64, &out);
  ASSERT_EQ(1, out.high);
  ASSERT_EQ(0, out.low);
  PASS();
}

TEST test_int128_shr(void) {
  cdd_int128_t a = cdd_make_int128(-1LL, 0);
  cdd_int128_t out;

  cdd_int128_shr(a, 0, &out);
  ASSERT_EQ(-1LL, out.high);
  ASSERT_EQ(0, out.low);

  cdd_int128_shr(a, 64, &out);
  ASSERT_EQ(-1LL, out.high);
  ASSERT_EQ((uint64_t)-1LL, out.low);

  cdd_int128_t b = cdd_make_int128(1, 0);
  cdd_int128_shr(b, 64, &out);
  ASSERT_EQ(0, out.high);
  ASSERT_EQ(1, out.low);

  cdd_int128_shr(b, 1, &out);
  ASSERT_EQ(0, out.high);
  ASSERT_EQ(0x8000000000000000ULL, out.low);

  PASS();
}

TEST test_uint128_divmod(void) {
  cdd_uint128_t num = cdd_make_uint128(0, 100);
  cdd_uint128_t den = cdd_make_uint128(0, 3);
  cdd_uint128_t q, r;

  ASSERT_EQ(0, cdd_uint128_divmod(num, den, &q, &r));
  ASSERT_EQ(0, q.high);
  ASSERT_EQ(33, q.low);
  ASSERT_EQ(0, r.high);
  ASSERT_EQ(1, r.low);

  cdd_uint128_t zero = cdd_make_uint128(0, 0);
  ASSERT_EQ(1, cdd_uint128_divmod(num, zero, &q, &r));

  ASSERT_EQ(0, cdd_uint128_div(num, den, &q));
  ASSERT_EQ(33, q.low);

  ASSERT_EQ(0, cdd_uint128_mod(num, den, &r));
  ASSERT_EQ(1, r.low);

  cdd_uint128_t big_num = cdd_make_uint128(3, 3);
  ASSERT_EQ(0, cdd_uint128_divmod(big_num, den, &q, &r));
  cdd_uint128_t den2 = cdd_make_uint128(0, 2);
  ASSERT_EQ(0, cdd_uint128_divmod(big_num, den2, &q, &r));
  ASSERT_EQ(0, cdd_uint128_divmod(big_num, big_num, &q, &r));

  PASS();
}

TEST test_int128_divmod(void) {
  cdd_int128_t num = cdd_make_int128(0, 100);
  cdd_int128_t den = cdd_make_int128(0, 3);
  cdd_int128_t q, r;

  ASSERT_EQ(0, cdd_int128_div(num, den, &q));
  ASSERT_EQ(33, q.low);
  ASSERT_EQ(0, cdd_int128_mod(num, den, &r));
  ASSERT_EQ(1, r.low);

  cdd_int128_t neg_num =
      cdd_make_int128(-1LL, 0xFFFFFFFFFFFFFF9CULL); /* -100 */
  ASSERT_EQ(0, cdd_int128_div(neg_num, den, &q));
  ASSERT_EQ(0xFFFFFFFFFFFFFFDFULL, q.low); /* -33 */

  cdd_int128_t neg_den = cdd_make_int128(-1LL, 0xFFFFFFFFFFFFFFFDULL); /* -3 */
  ASSERT_EQ(0, cdd_int128_div(num, neg_den, &q));
  ASSERT_EQ(0xFFFFFFFFFFFFFFDFULL, q.low); /* -33 */

  ASSERT_EQ(0, cdd_int128_div(neg_num, neg_den, &q));
  ASSERT_EQ(33, q.low);

  ASSERT_EQ(0, cdd_int128_mod(neg_num, den, &r));
  ASSERT_EQ(0, cdd_int128_mod(num, neg_den, &r));

  cdd_int128_t zero = cdd_make_int128(0, 0);
  ASSERT_EQ(1, cdd_int128_div(num, zero, &q));
  ASSERT_EQ(1, cdd_int128_mod(num, zero, &q));

  PASS();
}

TEST test_int128_bitwise(void) {
  cdd_uint128_t ua = cdd_make_uint128(1, 1);
  cdd_uint128_t ub = cdd_make_uint128(3, 2);
  cdd_uint128_t ur;

  cdd_uint128_and(ua, ub, &ur);
  ASSERT_EQ(1, ur.high);
  ASSERT_EQ(0, ur.low);

  cdd_uint128_or(ua, ub, &ur);
  ASSERT_EQ(3, ur.high);
  ASSERT_EQ(3, ur.low);

  cdd_uint128_xor(ua, ub, &ur);
  ASSERT_EQ(2, ur.high);
  ASSERT_EQ(3, ur.low);

  cdd_uint128_not(ua, &ur);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFEULL, ur.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFFFEULL, ur.low);

  cdd_int128_t ia = cdd_make_int128(1, 1);
  cdd_int128_t ib = cdd_make_int128(3, 2);
  cdd_int128_t ir;

  cdd_int128_and(ia, ib, &ir);
  cdd_int128_or(ia, ib, &ir);
  cdd_int128_xor(ia, ib, &ir);
  cdd_int128_not(ia, &ir);

  PASS();
}

TEST test_int128_casts(void) {
  cdd_uint128_t uout;
  cdd_int128_t iout;
  uint64_t u64;
  int64_t i64;
  float f;
  double d;

  cdd_uint64_to_uint128(100ULL, &uout);
  ASSERT_EQ(0, uout.high);
  ASSERT_EQ(100ULL, uout.low);

  cdd_uint128_to_uint64(uout, &u64);
  ASSERT_EQ(100ULL, u64);

  cdd_int64_to_int128(-100LL, &iout);
  ASSERT_EQ(-1LL, iout.high);
  ASSERT_EQ(0xFFFFFFFFFFFFFF9CULL, iout.low);

  cdd_int128_to_int64(iout, &i64);
  ASSERT_EQ(-100LL, i64);

  cdd_int64_to_int128(100LL, &iout);

  cdd_float_to_int128(-1.5f, &iout);
  cdd_float_to_int128(1.5f, &iout);

  cdd_double_to_int128(-1.5, &iout);
  cdd_double_to_int128(1.5, &iout);

  cdd_int128_to_float(iout, &f);
  cdd_int128_to_double(iout, &d);

  PASS();
}

SUITE(c_cdd_int128_suite) {
  RUN_TEST(test_uint128_add);
  RUN_TEST(test_int128_add);
  RUN_TEST(test_uint128_sub);
  RUN_TEST(test_int128_sub);
  RUN_TEST(test_uint128_mul);
  RUN_TEST(test_int128_mul);
  RUN_TEST(test_uint128_shl);
  RUN_TEST(test_uint128_shr);
  RUN_TEST(test_int128_shl);
  RUN_TEST(test_int128_shr);
  RUN_TEST(test_uint128_divmod);
  RUN_TEST(test_int128_divmod);
  RUN_TEST(test_int128_bitwise);
  RUN_TEST(test_int128_casts);
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
