/**
 * @file test_sql_to_c.h
 * @brief Tests for SQL to C struct emission.
 */

#ifndef C_CDD_TEST_SQL_TO_C_H
#define C_CDD_TEST_SQL_TO_C_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

TEST test_sql_to_c_errors(void) {
  ASSERT_EQ(EINVAL, sql_to_c_header_emit(NULL, NULL));
  ASSERT_EQ(EINVAL, sql_to_c_source_emit(NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, sql_to_c_projection_struct_emit(NULL, NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, sql_to_c_projection_free_emit(NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, sql_to_c_projection_meta_emit(NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, sql_to_c_projection_hydrate_emit(NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, sql_to_c_projection_dehydrate_emit(NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, sql_to_c_projection_nested_struct_emit(NULL, NULL, NULL));
  ASSERT_EQ(EINVAL,
            sql_to_c_projection_nested_array_emit(NULL, NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, sql_to_c_projection_dirty_bitmask_emit(NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, sql_to_c_projection_union_struct_emit(NULL, NULL, 0, NULL));
  ASSERT_EQ(EINVAL,
            sql_to_c_projection_polymorphic_struct_emit(NULL, NULL, NULL));
  PASS();
}

TEST test_sql_to_c_projections(void) {
  FILE *fp = tmpfile();
  cdd_c_query_projection_t proj;
  unsigned long long out_hash;
  memset(&proj, 0, sizeof(proj));

  proj.n_fields = 2;
  proj.fields = calloc(2, sizeof(*proj.fields));
  proj.fields[0].name = "id";
  proj.fields[0].type = SQL_TYPE_INT;
  proj.fields[0].is_array = 0;

  proj.fields[1].name = "tags";
  proj.fields[1].type = SQL_TYPE_VARCHAR;
  proj.fields[1].is_array = 1;

  ASSERT_EQ(0,
            sql_to_c_projection_struct_emit(fp, &proj, "ProjTest", &out_hash));
  ASSERT_EQ(0, sql_to_c_projection_free_emit(fp, &proj, "ProjTest"));
  ASSERT_EQ(0, sql_to_c_projection_meta_emit(fp, &proj, "ProjTest"));
  ASSERT_EQ(0, sql_to_c_projection_hydrate_emit(fp, &proj, "ProjTest"));
  ASSERT_EQ(0, sql_to_c_projection_dehydrate_emit(fp, &proj, "ProjTest"));
  ASSERT_EQ(0, sql_to_c_projection_nested_struct_emit(fp, &proj, "ProjTest"));
  ASSERT_EQ(0, sql_to_c_projection_nested_array_emit(fp, &proj, "ProjTest",
                                                     "ProjTestArray"));
  ASSERT_EQ(0, sql_to_c_projection_dirty_bitmask_emit(fp, &proj, "ProjTest"));
  ASSERT_EQ(0, sql_to_c_projection_union_struct_emit(fp, &proj, 1, "ProjTest"));
  ASSERT_EQ(0,
            sql_to_c_projection_polymorphic_struct_emit(fp, &proj, "ProjTest"));

  free(proj.fields);
  fclose(fp);
  PASS();
}

SUITE(sql_to_c_suite) {
  RUN_TEST(test_sql_to_c_header_emit);
  RUN_TEST(test_sql_to_c_source_emit);
  RUN_TEST(test_sql_to_c_errors);
  RUN_TEST(test_sql_to_c_projections);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_SQL_TO_C_H */
