/**
 * @file test_sql_to_c_proj.h
 * @brief Tests for emitting specific C structs from query projections.
 */

#ifndef C_CDD_TEST_SQL_TO_C_PROJ_H
#define C_CDD_TEST_SQL_TO_C_PROJ_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/emit/sql_to_c.h"
#include <greatest.h>
/* clang-format on */

TEST test_sql_to_c_projection_struct_emit(void) {
  cdd_c_query_projection_t proj;
  cdd_c_query_projection_field_t field1, field2;
  FILE *fp;
  char buffer[1024];
  size_t read_bytes;

  ASSERT_EQ(0, cdd_c_query_projection_init(&proj));
  proj.source_table = (char *)malloc(6);
  strcpy(proj.source_table, "users");

  field1.name = (char *)"id";
  field1.original_name = NULL;
  field1.type = SQL_TYPE_INT;
  field1.is_aggregate = 0;
  field1.length = 0;
  field1.is_array = 0;

  field2.name = (char *)"username";
  field2.original_name = NULL;
  field2.type = SQL_TYPE_VARCHAR;
  field2.is_aggregate = 0;
  field2.length = 255;
  field2.is_array = 0;

  ASSERT_EQ(0, cdd_c_query_projection_add_field(&proj, &field1));
  ASSERT_EQ(0, cdd_c_query_projection_add_field(&proj, &field2));

#if defined(_WIN32) || defined(_MSC_VER)
  fp = tmpfile();
#else
  fp = tmpfile();
#endif
  ASSERT_NEQ(NULL, fp);

  ASSERT_EQ(0, sql_to_c_projection_struct_emit(fp, &proj, "UserQueryProjection",
                                               NULL));

  rewind(fp);
  read_bytes = fread(buffer, 1, sizeof(buffer) - 1, fp);
  buffer[read_bytes] = '\0';
  fclose(fp);

  /* Verify struct output */
  ASSERT_NEQ(NULL, strstr(buffer, "typedef struct UserQueryProjection {"));
  ASSERT_NEQ(NULL, strstr(buffer, "int32_t id;"));
  ASSERT_NEQ(NULL, strstr(buffer, "char username[255];"));
  ASSERT_NEQ(NULL,
             strstr(buffer,
                    "extern C_CDD_EXPORT void "
                    "UserQueryProjection_free(UserQueryProjection *obj);"));

  cdd_c_query_projection_free(&proj);
  PASS();
}

TEST test_sql_to_c_projection_meta_emit(void) {
  cdd_c_query_projection_t proj;
  cdd_c_query_projection_field_t field1;
  FILE *fp;
  char buffer[1024];
  size_t read_bytes;

  ASSERT_EQ(0, cdd_c_query_projection_init(&proj));
  proj.source_table = (char *)malloc(6);
  strcpy(proj.source_table, "users");

  field1.name = (char *)"id";
  field1.original_name = NULL;
  field1.type = SQL_TYPE_INT;
  field1.is_aggregate = 0;
  field1.length = 0;
  field1.is_array = 0;

  ASSERT_EQ(0, cdd_c_query_projection_add_field(&proj, &field1));

  fp = tmpfile();
  ASSERT_NEQ(NULL, fp);

  ASSERT_EQ(0, sql_to_c_projection_meta_emit(fp, &proj, "UserQueryProjection"));

  rewind(fp);
  read_bytes = fread(buffer, 1, sizeof(buffer) - 1, fp);
  buffer[read_bytes] = '\0';
  fclose(fp);

  /* Verify metadata output */
  ASSERT_NEQ(
      NULL,
      strstr(buffer,
             "static const c_orm_prop_meta_t UserQueryProjection_props[] = {"));
  ASSERT_NEQ(NULL, strstr(buffer, "C_ORM_TYPE_INT32"));
  ASSERT_NEQ(NULL, strstr(buffer, "offsetof(UserQueryProjection, id)"));
  ASSERT_NEQ(NULL,
             strstr(buffer, "const c_orm_meta_t UserQueryProjection_meta = {"));

  cdd_c_query_projection_free(&proj);
  PASS();
}

TEST test_sql_to_c_projection_hydrate_dehydrate_emit(void) {
  cdd_c_query_projection_t proj;
  cdd_c_query_projection_field_t field1, field2;
  FILE *fp;
  char buffer[2048];
  size_t read_bytes;

  ASSERT_EQ(0, cdd_c_query_projection_init(&proj));
  proj.source_table = (char *)malloc(6);
  strcpy(proj.source_table, "users");

  field1.name = (char *)"id";
  field1.original_name = NULL;
  field1.type = SQL_TYPE_INT;
  field1.is_aggregate = 0;
  field1.length = 0;
  field1.is_array = 0;

  field2.name = (char *)"username";
  field2.original_name = NULL;
  field2.type = SQL_TYPE_VARCHAR;
  field2.is_aggregate = 0;
  field2.length = 255;
  field2.is_array = 0;

  ASSERT_EQ(0, cdd_c_query_projection_add_field(&proj, &field1));
  ASSERT_EQ(0, cdd_c_query_projection_add_field(&proj, &field2));

  fp = tmpfile();
  ASSERT_NEQ(NULL, fp);

  ASSERT_EQ(0,
            sql_to_c_projection_hydrate_emit(fp, &proj, "UserQueryProjection"));
  ASSERT_EQ(
      0, sql_to_c_projection_dehydrate_emit(fp, &proj, "UserQueryProjection"));

  rewind(fp);
  read_bytes = fread(buffer, 1, sizeof(buffer) - 1, fp);
  buffer[read_bytes] = '\0';
  fclose(fp);

  /* Verify hydrate/dehydrate output */
  ASSERT_NEQ(NULL,
             strstr(buffer,
                    "int UserQueryProjection_hydrate(UserQueryProjection "
                    "*out_struct, const cdd_c_abstract_struct_t *row) {"));
  ASSERT_NEQ(NULL, strstr(buffer, "out_struct->id = val->value.i_val;"));
  ASSERT_NEQ(
      NULL,
      strstr(buffer, "strncpy(out_struct->username, val->value.s_val, 255);"));

  ASSERT_NEQ(
      NULL,
      strstr(buffer,
             "int UserQueryProjection_dehydrate(const UserQueryProjection "
             "*in_struct, cdd_c_abstract_struct_t *out_row) {"));
  ASSERT_NEQ(NULL, strstr(buffer, "val.type = CDD_C_VARIANT_TYPE_INT;"));
  ASSERT_NEQ(NULL, strstr(buffer, "val.value.i_val = in_struct->id;"));
  ASSERT_NEQ(NULL,
             strstr(buffer, "val.value.s_val = (char*)in_struct->username;"));

  cdd_c_query_projection_free(&proj);
  PASS();
}

TEST test_sql_to_c_projection_nested_emit(void) {
  cdd_c_query_projection_t proj;
  cdd_c_query_projection_field_t field1;
  FILE *fp;
  char buffer[2048];
  size_t read_bytes;

  ASSERT_EQ(0, cdd_c_query_projection_init(&proj));
  proj.source_table = (char *)malloc(6);
  strcpy(proj.source_table, "roles");

  field1.name = (char *)"role_name";
  field1.original_name = NULL;
  field1.type = SQL_TYPE_VARCHAR;
  field1.is_aggregate = 0;
  field1.length = 50;
  field1.is_array = 0;

  ASSERT_EQ(0, cdd_c_query_projection_add_field(&proj, &field1));

  fp = tmpfile();
  ASSERT_NEQ(NULL, fp);

  ASSERT_EQ(0, sql_to_c_projection_nested_array_emit(
                   fp, &proj, "RoleProjection", "RoleProjectionArray"));

  rewind(fp);
  read_bytes = fread(buffer, 1, sizeof(buffer) - 1, fp);
  buffer[read_bytes] = '\0';
  fclose(fp);

  /* Verify nested array wrapper logic */
  ASSERT_NEQ(NULL, strstr(buffer, "typedef struct RoleProjectionArray {"));
  ASSERT_NEQ(NULL, strstr(buffer, "RoleProjection *items;"));
  ASSERT_NEQ(NULL, strstr(buffer, "size_t count;"));
  ASSERT_NEQ(
      NULL,
      strstr(buffer,
             "void RoleProjectionArray_free(RoleProjectionArray *arr) {"));
  ASSERT_NEQ(NULL, strstr(buffer, "RoleProjection_free(&arr->items[i]);"));

  cdd_c_query_projection_free(&proj);
  PASS();
}

TEST test_sql_to_c_projection_dirty_bitmask_emit(void) {
  cdd_c_query_projection_t proj;
  cdd_c_query_projection_field_t field1;
  FILE *fp;
  char buffer[1024];
  size_t read_bytes;

  ASSERT_EQ(0, cdd_c_query_projection_init(&proj));
  proj.source_table = (char *)malloc(6);
  strcpy(proj.source_table, "users");

  field1.name = (char *)"id";
  field1.original_name = NULL;
  field1.type = SQL_TYPE_INT;
  field1.is_aggregate = 0;
  field1.length = 0;
  field1.is_array = 0;

  ASSERT_EQ(0, cdd_c_query_projection_add_field(&proj, &field1));

  fp = tmpfile();
  ASSERT_NEQ(NULL, fp);

  ASSERT_EQ(0, sql_to_c_projection_dirty_bitmask_emit(fp, &proj,
                                                      "UserQueryProjection"));

  rewind(fp);
  read_bytes = fread(buffer, 1, sizeof(buffer) - 1, fp);
  buffer[read_bytes] = '\0';
  fclose(fp);

  ASSERT_NEQ(NULL, strstr(buffer, "typedef struct UserQueryProjection_mask {"));
  ASSERT_NEQ(NULL, strstr(buffer, "unsigned char mask;"));

  cdd_c_query_projection_free(&proj);
  PASS();
}

SUITE(sql_to_c_proj_suite) {
  RUN_TEST(test_sql_to_c_projection_struct_emit);
  RUN_TEST(test_sql_to_c_projection_meta_emit);
  RUN_TEST(test_sql_to_c_projection_hydrate_dehydrate_emit);
  RUN_TEST(test_sql_to_c_projection_nested_emit);
  RUN_TEST(test_sql_to_c_projection_dirty_bitmask_emit);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_SQL_TO_C_PROJ_H */
