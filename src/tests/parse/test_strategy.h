extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_strategy.h
 * @brief Unit tests for parsing strategy algorithms.
 */

#ifndef TEST_STRATEGY_H
#define TEST_STRATEGY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "functions/parse/strategy.h"
#include <greatest.h>
/* clang-format on */

/**
 * @brief Tests error handling of the strategy injection function.
 *
 * @return The result of the test.
 */
TEST test_strategy_errors(void) {
  struct PatchList patches;
  struct AllocationSiteList allocs;

  memset(&patches, 0, sizeof(patches));
  memset(&allocs, 0, sizeof(allocs));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            strategy_inject_safety_checks(NULL, &allocs, &patches));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            strategy_inject_safety_checks(NULL, NULL, &patches));
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Strategy test suite.
 */
SUITE(strategy_suite) { RUN_TEST(test_strategy_errors); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_STRATEGY_H */
