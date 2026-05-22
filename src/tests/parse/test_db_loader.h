/**
 * @file test_db_loader.h
 * @brief Unit tests for db_loader
 */

#ifndef TEST_DB_LOADER_H
#define TEST_DB_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "functions/parse/db_loader.h"
#include <greatest.h>
/* clang-format on */

/**
 * @brief Basic test for DB loader check
 * @return TEST
 */
TEST test_db_loader_basic(void) {
  int avail;
  ASSERT_EQ(0, check_libpq_available(&avail));
  ASSERT_EQ(0, check_sqlite3_available(&avail));
  ASSERT_EQ(0, check_mysql_available(&avail));

  /* Error bounds */
  ASSERT_EQ(22, check_libpq_available(NULL));
  ASSERT_EQ(22, check_sqlite3_available(NULL));
  ASSERT_EQ(22, check_mysql_available(NULL));

  PASS();
}

/**
 * @brief Suite for DB loader checks
 */
SUITE(db_loader_suite) { RUN_TEST(test_db_loader_basic); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DB_LOADER_H */
