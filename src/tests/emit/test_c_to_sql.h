/**
 * @file test_c_to_sql.h
 * @brief Tests for C struct to SQL generation.
 */

#ifndef TEST_C_TO_SQL_H
#define TEST_C_TO_SQL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/emit/c_to_sql.h"
#include <string.h>
/* clang-format on */

TEST test_write_struct_to_sql_create_table(void) {
  struct StructField field;
  struct StructFields sf;
  char buf[1024];
  FILE *fp;
  int rc;

  memset(buf, 0, sizeof(buf));
  memset(&field, 0, sizeof(field));
  strcpy(field.name, "id");
  strcpy(field.type, "integer");
  field.required = 1;

  sf.size = 1;
  sf.fields = &field;

  fp = fopen("test_c_to_sql.txt", "w+");
  ASSERT(fp != NULL);

  rc = write_struct_to_sql_create_table(fp, "users", &sf,
                                        C_TO_SQL_DIALECT_SQLITE);
  ASSERT_EQ(0, rc);

  rewind(fp);
  fread(buf, 1, sizeof(buf) - 1, fp);
  fclose(fp);

  ASSERT(strstr(buf, "CREATE TABLE users") != NULL);
  ASSERT(strstr(buf, "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL") != NULL);

  PASS();
}

SUITE(test_c_to_sql_suite) { RUN_TEST(test_write_struct_to_sql_create_table); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C_TO_SQL_H */