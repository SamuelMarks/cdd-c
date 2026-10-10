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

SUITE(c_cdd_int128_suite) { RUN_TEST(test_int128_constructors); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_INT128_H */
