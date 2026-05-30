extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
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

TEST test_db_loader_basic(void) {
  int avail;
  ASSERT_EQ(0, check_libpq_available(&avail));
  ASSERT_EQ(0, check_sqlite3_available(&avail));
  ASSERT_EQ(0, check_mysql_available(&avail));

  ASSERT_EQ(22, check_libpq_available(NULL));
  ASSERT_EQ(22, check_sqlite3_available(NULL));
  ASSERT_EQ(22, check_mysql_available(NULL));
  g_fail_io_after = -1;

  PASS();
}

TEST test_db_loader_success(void) {
  int avail;
#ifdef CDD_BUILD_TESTS
  extern int g_cdd_mock_dlopen_success;
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

SUITE(db_loader_suite) {
  RUN_TEST(test_db_loader_basic);
  RUN_TEST(test_db_loader_success);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DB_LOADER_H */
