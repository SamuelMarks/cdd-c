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
#include <stdlib.h>
/* clang-format on */

TEST test_write_struct_to_sql_create_table(void) {
  struct StructField fields[3];
  struct StructFields sf;
  char buf[1024];
  FILE *fp;
  int rc;

  memset(buf, 0, sizeof(buf));
  memset(fields, 0, sizeof(fields));
  strcpy(fields[0].name, "id");
  strcpy(fields[0].type, "integer");
  fields[0].required = 1;

  strcpy(fields[1].name, "username");
  strcpy(fields[1].type, "string");
  strcpy(fields[1].description, "@unique @notnull");

  strcpy(fields[2].name, "company_id");
  strcpy(fields[2].type, "integer");

  sf.size = 3;
  sf.fields = fields;

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
  ASSERT(strstr(buf, "username TEXT UNIQUE NOT NULL") != NULL);
  ASSERT(strstr(buf, "company_id INTEGER REFERENCES company(id)") != NULL);

  PASS();
}

TEST test_cdd_c_meta_to_sql_create_table(void) {
  cdd_c_prop_meta_t props[2];
  cdd_c_meta_t meta;
  char *out_sql = NULL;
  int rc;

  memset(&meta, 0, sizeof(meta));
  memset(props, 0, sizeof(props));

  props[0].name = "id";
  props[0].type = "int";

  props[1].name = "name";
  props[1].type = "char*";

  meta.name = "company";
  meta.props = props;
  meta.num_props = 2;

  rc = cdd_c_meta_to_sql_create_table(&meta, C_TO_SQL_DIALECT_MYSQL, &out_sql);
  ASSERT_EQ(0, rc);
  ASSERT(out_sql != NULL);
  ASSERT(strstr(out_sql, "CREATE TABLE company") != NULL);
  ASSERT(strstr(out_sql, "id INT PRIMARY KEY") != NULL);
  ASSERT(strstr(out_sql, "name VARCHAR(255)") != NULL);

  free(out_sql);
  PASS();
}

TEST test_cdd_c_meta_diff_and_sql(void) {
  cdd_c_prop_meta_t props_old[1];
  cdd_c_prop_meta_t props_new[2];
  cdd_c_meta_t old_meta, new_meta;
  cdd_c_meta_diff_t diff;
  char *up_sql = NULL, *down_sql = NULL;
  int rc;

  memset(&old_meta, 0, sizeof(old_meta));
  memset(&new_meta, 0, sizeof(new_meta));
  memset(props_old, 0, sizeof(props_old));
  memset(props_new, 0, sizeof(props_new));

  props_old[0].name = "id";
  props_old[0].type = "int";

  props_new[0].name = "id";
  props_new[0].type = "int";
  props_new[1].name = "description";
  props_new[1].type = "char*";

  old_meta.name = "test_table";
  old_meta.props = props_old;
  old_meta.num_props = 1;

  new_meta.name = "test_table";
  new_meta.props = props_new;
  new_meta.num_props = 2;

  rc = cdd_c_meta_diff(&old_meta, &new_meta, &diff);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, diff.num_added);
  ASSERT_EQ(0, diff.num_dropped);
  ASSERT_EQ(0, diff.num_altered);

  rc = cdd_c_meta_diff_to_sql("test_table", &diff, C_TO_SQL_DIALECT_POSTGRESQL,
                              &up_sql, &down_sql);
  ASSERT_EQ(0, rc);
  ASSERT(up_sql != NULL);
  ASSERT(down_sql != NULL);
  ASSERT(strstr(up_sql, "ADD COLUMN description TEXT") != NULL);
  ASSERT(strstr(down_sql, "DROP COLUMN description") != NULL);

  free(up_sql);
  free(down_sql);
  cdd_c_meta_diff_free(&diff);
  PASS();
}

TEST test_cdd_c_get_schema_inspection_query(void) {
  char *query = NULL;
  int rc;

  rc = cdd_c_get_schema_inspection_query(C_TO_SQL_DIALECT_POSTGRESQL, "users",
                                         &query);
  ASSERT_EQ(0, rc);
  ASSERT(query != NULL);
  ASSERT(strstr(query, "information_schema") != NULL);
  free(query);

  PASS();
}

TEST test_cdd_c_emit_index(void) {
  char *query = NULL;
  int rc;

  rc = cdd_c_emit_create_index("users", "idx_users_email", "email", 1, &query);
  ASSERT_EQ(0, rc);
  ASSERT(query != NULL);
  ASSERT(strstr(query, "UNIQUE INDEX idx_users_email ON users (email)") !=
         NULL);
  free(query);

  rc = cdd_c_emit_drop_index("idx_users_email", &query);
  ASSERT_EQ(0, rc);
  ASSERT(query != NULL);
  ASSERT(strstr(query, "DROP INDEX idx_users_email") != NULL);
  free(query);

  PASS();
}

TEST test_cdd_c_meta_topological_sort(void) {
  cdd_c_prop_meta_t p_user[1], p_post[2];
  cdd_c_meta_t m_user, m_post;
  const cdd_c_meta_t *schemas[2];
  const cdd_c_meta_t *out_schemas[2];
  int rc;

  memset(&m_user, 0, sizeof(m_user));
  memset(&m_post, 0, sizeof(m_post));
  memset(p_user, 0, sizeof(p_user));
  memset(p_post, 0, sizeof(p_post));

  p_user[0].name = "id";
  p_user[0].type = "int";
  m_user.name = "user";
  m_user.props = p_user;
  m_user.num_props = 1;

  p_post[0].name = "id";
  p_post[0].type = "int";
  p_post[1].name = "user_id";
  p_post[1].type = "int";
  m_post.name = "post";
  m_post.props = p_post;
  m_post.num_props = 2;

  /* Put dependent post first to see if sort works */
  schemas[0] = &m_post;
  schemas[1] = &m_user;

  rc = cdd_c_meta_topological_sort(schemas, 2, out_schemas);
  ASSERT_EQ(0, rc);
  ASSERT(out_schemas[0] == &m_user);
  ASSERT(out_schemas[1] == &m_post);

  PASS();
}

SUITE(test_c_to_sql_suite) {
  RUN_TEST(test_write_struct_to_sql_create_table);
  RUN_TEST(test_cdd_c_meta_to_sql_create_table);
  RUN_TEST(test_cdd_c_meta_diff_and_sql);
  RUN_TEST(test_cdd_c_get_schema_inspection_query);
  RUN_TEST(test_cdd_c_emit_index);
  RUN_TEST(test_cdd_c_meta_topological_sort);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C_TO_SQL_H */
