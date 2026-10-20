
/**
 * @file test_log.h
 * @brief Unit tests for c_cdd_log_debug.
 */
#ifndef TEST_LOG_H
#define TEST_LOG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/log.h"
#include "c_cdd_export.h"
#include <greatest.h>
/* clang-format on */

/**
 * @brief Tests basic and error branches of c_cdd_log_debug.
 * @return greatest test result.
 */
TEST test_log_basic(void) {
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_log_debug("test %d", 42));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_log_debug(NULL));
  PASS();
}

/**
 * @brief Executes the log_suite test suite.
 */
SUITE(log_suite) { RUN_TEST(test_log_basic); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_LOG_H */
