#ifndef TEST_STRATEGY_H
#define TEST_STRATEGY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "functions/parse/strategy.h"
#include <greatest.h>

TEST test_strategy_errors(void) {
  struct PatchList patches;
  struct AllocationSiteList allocs;

  ASSERT_EQ(EINVAL, strategy_inject_safety_checks(NULL, &allocs, &patches));
  ASSERT_EQ(EINVAL, strategy_inject_safety_checks(NULL, NULL, &patches));

  PASS();
}

SUITE(strategy_suite) { RUN_TEST(test_strategy_errors); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_STRATEGY_H */
