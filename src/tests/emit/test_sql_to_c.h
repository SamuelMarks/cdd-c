/**
 * @file test_sql_to_c.h
 * @brief Tests for SQL to C struct emission.
 */

#ifndef C_CDD_TEST_SQL_TO_C_H
#define C_CDD_TEST_SQL_TO_C_H

/* clang-format off */
#include "classes/emit/sql_to_c.h"
#include <greatest.h>
#include <string.h>
/* clang-format on */

TEST test_sql_to_c_header_emit(void) {
  const char *sql =
      "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(255) NOT NULL, "
      "role_id BIGINT REFERENCES roles(id), is_active BOOLEAN DEFAULT true);";
  az_span span = az_span_create_from_str((char *)sql);
  struct sql_token_list_t *list = NULL;
  struct sql_table_t *table = NULL;
  struct sql_parse_error_t err_info;
  int err;

  char buf[4096];
  FILE *fp;

  err = sql_lex(span, &list);
  ASSERT_EQ(0, err);

  err = sql_parse_table(list, &table, &err_info);
  ASSERT_EQ(0, err);

  fp = tmpfile();
  ASSERT(fp != NULL);

  err = sql_to_c_header_emit(fp, table);
  ASSERT_EQ(0, err);

  rewind(fp);
  memset(buf, 0, sizeof(buf));
  fread(buf, 1, sizeof(buf) - 1, fp);
  fclose(fp);

  ASSERT(strstr(buf, "#ifndef C_ORM_MODEL_USERS_H") != NULL);
  ASSERT(strstr(buf, "struct Users {") != NULL);
  ASSERT(strstr(buf, "int32_t id;") != NULL);
  ASSERT(strstr(buf, "char * name;") != NULL);
  ASSERT(strstr(buf, "int64_t *role_id; /**< Nullable */") != NULL);
  ASSERT(strstr(buf, "bool *is_active; /**< Nullable */") != NULL);
  ASSERT(strstr(buf, "struct Users_Array {") != NULL);

  sql_table_free(table);
  sql_token_list_free(list);
  PASS();
}

TEST test_sql_to_c_source_emit(void) {
  const char *sql =
      "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(255) NOT NULL, "
      "role_id BIGINT REFERENCES roles(id), is_active BOOLEAN DEFAULT true);";
  az_span span = az_span_create_from_str((char *)sql);
  struct sql_token_list_t *list = NULL;
  struct sql_table_t *table = NULL;
  struct sql_parse_error_t err_info;
  int err;

  char buf[4096];
  FILE *fp;

  err = sql_lex(span, &list);
  ASSERT_EQ(0, err);

  err = sql_parse_table(list, &table, &err_info);
  ASSERT_EQ(0, err);

  fp = tmpfile();
  ASSERT(fp != NULL);

  err = sql_to_c_source_emit(fp, table, "users.h");
  ASSERT_EQ(0, err);

  rewind(fp);
  memset(buf, 0, sizeof(buf));
  fread(buf, 1, sizeof(buf) - 1, fp);
  fclose(fp);

  ASSERT(strstr(buf, "#include \"users.h\"") != NULL);
  ASSERT(strstr(buf, "int Users_Array_init(") != NULL);
  ASSERT(strstr(buf, "void Users_free(") != NULL);
  ASSERT(strstr(buf, "void Users_Array_free(") != NULL);
  ASSERT(strstr(buf, "int Users_deepcopy(") != NULL);
  ASSERT(strstr(buf, "int Users_Array_deepcopy(") != NULL);

  sql_table_free(table);
  sql_token_list_free(list);
  PASS();
}

SUITE(sql_to_c_suite) {
  RUN_TEST(test_sql_to_c_header_emit);
  RUN_TEST(test_sql_to_c_source_emit);
}

#endif /* C_CDD_TEST_SQL_TO_C_H */