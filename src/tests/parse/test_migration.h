#ifndef TEST_MIGRATION_H
#define TEST_MIGRATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <greatest.h>
#include "classes/parse/migration.h"
#include "functions/parse/migration_runner.h"
#include "functions/parse/fs.h"
/* clang-format on */

TEST test_parse_migration_file_valid(void) {
  const char *filename = "test_valid_migration.sql";
  const char *content = "-- UP\n"
                        "CREATE TABLE test (id INT);\n"
                        "-- DOWN\n"
                        "DROP TABLE test;\n";
  struct MigrationStatements stmts;
  int rc;

  rc = fs_write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  rc = parse_migration_file(filename, &stmts);
  ASSERT_EQ(0, rc);
  ASSERT(stmts.up_statement != NULL);
  ASSERT(stmts.down_statement != NULL);
  ASSERT(strstr(stmts.up_statement, "CREATE TABLE test") != NULL);
  ASSERT(strstr(stmts.down_statement, "DROP TABLE test") != NULL);

  migration_statements_free(&stmts);
  remove(filename);
  PASS();
}

TEST test_parse_migration_file_no_down(void) {
  const char *filename = "test_up_only.sql";
  const char *content = "-- UP\n"
                        "CREATE TABLE test2 (id INT);\n";
  struct MigrationStatements stmts;
  int rc;

  rc = fs_write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  rc = parse_migration_file(filename, &stmts);
  ASSERT_EQ(0, rc);
  ASSERT(stmts.up_statement != NULL);
  ASSERT_EQ(NULL, stmts.down_statement);
  ASSERT(strstr(stmts.up_statement, "CREATE TABLE test2") != NULL);

  migration_statements_free(&stmts);
  remove(filename);
  PASS();
}

TEST test_parse_migration_file_no_markers(void) {
  const char *filename = "test_no_markers.sql";
  const char *content = "CREATE TABLE test3 (id INT);\n";
  struct MigrationStatements stmts;
  int rc;

  rc = fs_write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  rc = parse_migration_file(filename, &stmts);
  ASSERT_EQ(0, rc);
  ASSERT(stmts.up_statement != NULL);
  ASSERT_EQ(NULL, stmts.down_statement);
  ASSERT(strstr(stmts.up_statement, "CREATE TABLE test3") != NULL);

  migration_statements_free(&stmts);
  remove(filename);
  PASS();
}

SUITE(migration_suite) {
  RUN_TEST(test_parse_migration_file_valid);
  RUN_TEST(test_parse_migration_file_no_down);
  RUN_TEST(test_parse_migration_file_no_markers);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_MIGRATION_H */
