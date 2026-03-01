#ifndef TEST_NUMERIC_PARSER_H
#define TEST_NUMERIC_PARSER_H

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "classes/parse/numeric.h"

/* Helper to check float equality with epsilon */
static int dbl_eq(double a, double b) { return fabs(a - b) < 1e-9; }

TEST test_parse_dec_int(void) {
  struct NumericValue nv;
  ASSERT_EQ(0, parse_numeric_literal("123", &nv));
  ASSERT_EQ(NUMERIC_INTEGER, nv.kind);
  ASSERT_EQ(123, nv.data.integer.value);
  ASSERT_EQ(10, nv.data.integer.base);
  ASSERT_EQ(0, nv.data.integer.is_unsigned);
  PASS();
}

TEST test_parse_hex_int(void) {
  struct NumericValue nv;
  ASSERT_EQ(0, parse_numeric_literal("0xFF", &nv));
  ASSERT_EQ(NUMERIC_INTEGER, nv.kind);
  ASSERT_EQ(255, nv.data.integer.value);
  ASSERT_EQ(16, nv.data.integer.base);
  PASS();
}

TEST test_parse_bin_int(void) {
  struct NumericValue nv;
  ASSERT_EQ(0, parse_numeric_literal("0b101", &nv));
  ASSERT_EQ(NUMERIC_INTEGER, nv.kind);
  ASSERT_EQ(5, nv.data.integer.value);
  ASSERT_EQ(2, nv.data.integer.base);
  PASS();
}

TEST test_parse_oct_int(void) {
  struct NumericValue nv;
  ASSERT_EQ(0, parse_numeric_literal("010", &nv)); /* Octal 10 -> Dec 8 */
  ASSERT_EQ(NUMERIC_INTEGER, nv.kind);
  ASSERT_EQ(8, nv.data.integer.value);
  ASSERT_EQ(8, nv.data.integer.base);
  PASS();
}

TEST test_parse_int_suffixes(void) {
  struct NumericValue nv;

  parse_numeric_literal("1u", &nv);
  ASSERT(nv.data.integer.is_unsigned);
  ASSERT(!nv.data.integer.is_long);

  parse_numeric_literal("1ul", &nv);
  ASSERT(nv.data.integer.is_unsigned);
  ASSERT(nv.data.integer.is_long);

  parse_numeric_literal("1LLU", &nv);
  ASSERT(nv.data.integer.is_unsigned);
  ASSERT(nv.data.integer.is_long_long);

  PASS();
}

TEST test_parse_float_simple(void) {
  struct NumericValue nv;
  ASSERT_EQ(0, parse_numeric_literal("3.14", &nv));
  ASSERT_EQ(NUMERIC_FLOAT, nv.kind);
  ASSERT(dbl_eq(3.14, nv.data.floating.value));
  ASSERT_EQ(0, nv.data.floating.is_float);
  PASS();
}

TEST test_parse_float_exponent(void) {
  struct NumericValue nv;
  ASSERT_EQ(0, parse_numeric_literal("1.5e2", &nv));
  ASSERT_EQ(NUMERIC_FLOAT, nv.kind);
  ASSERT(dbl_eq(150.0, nv.data.floating.value));
  PASS();
}

TEST test_parse_float_suffix(void) {
  struct NumericValue nv;
  ASSERT_EQ(0, parse_numeric_literal("1.0f", &nv));
  ASSERT(nv.data.floating.is_float);

  ASSERT_EQ(0, parse_numeric_literal("1.0L", &nv));
  ASSERT(nv.data.floating.is_long_double);
  PASS();
}

TEST test_parse_decimal_float_suffixes(void) {
  struct NumericValue nv;

  /* _Decimal32 */
  ASSERT_EQ(0, parse_numeric_literal("1.2df", &nv));
  ASSERT_EQ(NUMERIC_FLOAT, nv.kind);
  ASSERT_EQ(DFP_32, nv.data.floating.is_decimal);

  /* _Decimal64 */
  ASSERT_EQ(0, parse_numeric_literal("3.14dd", &nv));
  ASSERT_EQ(NUMERIC_FLOAT, nv.kind);
  ASSERT_EQ(DFP_64, nv.data.floating.is_decimal);

  /* _Decimal128 */
  ASSERT_EQ(0, parse_numeric_literal("0.1DL",
                                     &nv)); /* Case insensitive suffix check */
  ASSERT_EQ(NUMERIC_FLOAT, nv.kind);
  ASSERT_EQ(DFP_128, nv.data.floating.is_decimal);

  PASS();
}

TEST test_parse_hex_float(void) {
  struct NumericValue nv;
  /* 0x1.8p1 = 1.5 * 2^1 = 3.0 */
  ASSERT_EQ(0, parse_numeric_literal("0x1.8p1", &nv));
  ASSERT_EQ(NUMERIC_FLOAT, nv.kind);
  ASSERT(dbl_eq(3.0, nv.data.floating.value));
  PASS();
}

TEST test_parse_errors(void) {
  struct NumericValue nv;
  /* Bad hex */
  ASSERT_EQ(EINVAL, parse_numeric_literal("0xZZ", &nv));
  /* Bad float suffix */
  ASSERT_EQ(EINVAL, parse_numeric_literal("1.0z", &nv));
  /* Mixed decimal float suffix (e.g. dL vs dl/DL, spec implies case consistency
     but parser might be strict or loose. Implementation checks [0] and [1]. 'd'
     'L' -> DFP_128. Let's check invalid combo 'dx'. */
  ASSERT_EQ(EINVAL, parse_numeric_literal("1.0dx", &nv));
  /* Nothing */
  ASSERT_EQ(EINVAL, parse_numeric_literal("", &nv));
  /* NULL */
  ASSERT_EQ(EINVAL, parse_numeric_literal(NULL, &nv));
  PASS();
}

SUITE(numeric_parser_suite) {
  RUN_TEST(test_parse_dec_int);
  RUN_TEST(test_parse_hex_int);
  RUN_TEST(test_parse_bin_int);
  RUN_TEST(test_parse_oct_int);
  RUN_TEST(test_parse_int_suffixes);
  RUN_TEST(test_parse_float_simple);
  RUN_TEST(test_parse_float_exponent);
  RUN_TEST(test_parse_float_suffix);
  RUN_TEST(test_parse_decimal_float_suffixes);
  RUN_TEST(test_parse_hex_float);
  RUN_TEST(test_parse_errors);
}

#endif /* TEST_NUMERIC_PARSER_H */
