/**
 * @file test_sql.h
 * @brief Tests for the SQL DDL Lexer.
 */

#ifndef C_CDD_TEST_SQL_H
#define C_CDD_TEST_SQL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/parse/sql.h"
#include <greatest.h>
/* clang-format on */

TEST test_sql_lexer_basic(void) {
  const char *sql =
      "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(255));";
  az_span span = az_span_create_from_str((char *)sql);
  struct sql_token_list_t *list = NULL;
  int err;

  err = sql_lex(span, &list);
  ASSERT_EQ(0, err);
  ASSERT(list != NULL);
  ASSERT(list->size > 0);

  /* CREATE */
  ASSERT_EQ(SQL_TOKEN_KEYWORD, list->tokens[0].kind);
  ASSERT_EQ(6, list->tokens[0].length);
  /* <space> */
  ASSERT_EQ(SQL_TOKEN_WHITESPACE, list->tokens[1].kind);
  /* TABLE */
  ASSERT_EQ(SQL_TOKEN_KEYWORD, list->tokens[2].kind);
  /* <space> */
  ASSERT_EQ(SQL_TOKEN_WHITESPACE, list->tokens[3].kind);
  /* users */
  ASSERT_EQ(SQL_TOKEN_IDENTIFIER, list->tokens[4].kind);

  sql_token_list_free(list);
  PASS();
}

TEST test_sql_lexer_types(void) {
  const char *sql = "id BIGINT, is_active BOOLEAN DEFAULT true";
  az_span span = az_span_create_from_str((char *)sql);
  struct sql_token_list_t *list = NULL;
  int err;

  err = sql_lex(span, &list);
  ASSERT_EQ(0, err);
  ASSERT(list != NULL);

  sql_token_list_free(list);
  PASS();
}

TEST test_sql_parser_basic(void) {
  const char *sql =
      "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(255) NOT NULL, "
      "role_id BIGINT REFERENCES roles(id), is_active BOOLEAN DEFAULT true);";
  az_span span = az_span_create_from_str((char *)sql);
  struct sql_token_list_t *list = NULL;
  struct sql_table_t *table = NULL;
  struct sql_parse_error_t err_info;
  int err;

  err = sql_lex(span, &list);
  ASSERT_EQ(0, err);

  err = sql_parse_table(list, &table, &err_info);
  if (err != 0) {
    printf("SQL Parse Error: %s\n", err_info.message);
    if (err_info.token) {
      printf("At token: %.*s\n", (int)err_info.token->length,
             err_info.token->start);
    }
  }
  ASSERT_EQ(0, err);
  ASSERT(table != NULL);
  ASSERT_STR_EQ("users", table->name);
  ASSERT_EQ(4, table->n_columns);

  /* id INT PRIMARY KEY */
  ASSERT_STR_EQ("id", table->columns[0].name);
  ASSERT_EQ(SQL_TYPE_INT, table->columns[0].type);
  ASSERT_EQ(1, table->columns[0].n_constraints);
  ASSERT_EQ(SQL_CONSTRAINT_PRIMARY_KEY, table->columns[0].constraints[0].type);

  /* name VARCHAR(255) NOT NULL */
  ASSERT_STR_EQ("name", table->columns[1].name);
  ASSERT_EQ(SQL_TYPE_VARCHAR, table->columns[1].type);
  ASSERT_EQ(255, table->columns[1].length);
  ASSERT_EQ(1, table->columns[1].n_constraints);
  ASSERT_EQ(SQL_CONSTRAINT_NOT_NULL, table->columns[1].constraints[0].type);

  /* role_id BIGINT REFERENCES roles(id) */
  ASSERT_STR_EQ("role_id", table->columns[2].name);
  ASSERT_EQ(SQL_TYPE_BIGINT, table->columns[2].type);
  ASSERT_EQ(1, table->columns[2].n_constraints);
  ASSERT_EQ(SQL_CONSTRAINT_FOREIGN_KEY, table->columns[2].constraints[0].type);
  ASSERT_STR_EQ("roles", table->columns[2].constraints[0].reference_table);
  ASSERT_STR_EQ("id", table->columns[2].constraints[0].reference_column);

  /* is_active BOOLEAN DEFAULT true */
  ASSERT_STR_EQ("is_active", table->columns[3].name);
  ASSERT_EQ(SQL_TYPE_BOOLEAN, table->columns[3].type);
  ASSERT_EQ(1, table->columns[3].n_constraints);
  ASSERT_EQ(SQL_CONSTRAINT_DEFAULT, table->columns[3].constraints[0].type);
  ASSERT_STR_EQ("true", table->columns[3].constraints[0].default_value);

  sql_table_free(table);
  sql_token_list_free(list);
  PASS();
}

SUITE(sql_suite) {
  RUN_TEST(test_sql_lexer_basic);
  RUN_TEST(test_sql_lexer_types);
  RUN_TEST(test_sql_parser_basic);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_SQL_H */
