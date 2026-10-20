/**
 * @file test_db_loader.h
 * @brief Unit tests and mocks in test_db_loader.h.
 */
#ifndef TEST_DB_LOADER_H
#define TEST_DB_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "functions/parse/db_loader.h"
#include <greatest.h>
#if !defined(_WIN32)
#include <dlfcn.h>
#endif
/* clang-format on */

/* Moved extern declarations for C89 compliance */
/** @brief Test definition g_cdd_mock_dlopen_success. */
extern C_CDD_EXPORT int g_cdd_mock_dlopen_success;

/**
 * @brief Unit test test_db_loader_basic.
 * @return Greatest test result.
 */
TEST test_db_loader_basic(void) {
  int avail;
  ASSERT_EQ(CDD_C_SUCCESS, check_libpq_available(&avail));
  ASSERT_EQ(CDD_C_SUCCESS, check_sqlite3_available(&avail));
  ASSERT_EQ(CDD_C_SUCCESS, check_mysql_available(&avail));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, check_libpq_available(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, check_sqlite3_available(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, check_mysql_available(NULL));
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_db_loader_success.
 * @return Greatest test result.
 */
TEST test_db_loader_success(void) {
  int avail;
#ifdef CDD_BUILD_TESTS
  /* extern C_CDD_EXPORT int g_cdd_mock_dlopen_success; (moved to global) */
  g_cdd_mock_dlopen_success = 1;
  ASSERT_EQ(0, check_libpq_available(&avail));
  ASSERT_EQ(1, avail);
  ASSERT_EQ(0, check_sqlite3_available(&avail));
  ASSERT_EQ(1, avail);
  ASSERT_EQ(0, check_mysql_available(&avail));
  ASSERT_EQ(1, avail);
  g_cdd_mock_dlopen_success = 0;
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the db_loader_suite test suite.
 */
SUITE(db_loader_suite) {
  RUN_TEST(test_db_loader_basic);
  RUN_TEST(test_db_loader_success);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DB_LOADER_H */
