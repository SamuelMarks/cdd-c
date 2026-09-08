/**
 * @file test_query_projection.h
 * @brief Tests for query projection AST representations.
 */

#ifndef C_CDD_TEST_QUERY_PROJECTION_H
#define C_CDD_TEST_QUERY_PROJECTION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "classes/parse/query_projection.h"
#include <greatest.h>
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_strdup_fail;

/**
 * @brief Test initialization of query projection.
 */
TEST test_query_projection_init(void) {
  cdd_c_query_projection_t proj;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_c_query_projection_init(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_init(&proj));
  ASSERT_EQ(0, proj.n_fields);
  ASSERT_EQ(NULL, proj.fields);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Test free function of query projection.
 */
TEST test_query_projection_free(void) {
  cdd_c_query_projection_t proj;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_c_query_projection_free(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_init(&proj));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_free(&proj));
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Test adding fields to query projection.
 */
TEST test_query_projection_add_field(void) {
  cdd_c_query_projection_t proj;
  cdd_c_query_projection_field_t field;
  cdd_c_query_projection_field_t null_field;
  size_t i;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_init(&proj));

  field.name = (char *)(size_t)(size_t) "id";
  field.original_name = (char *)(size_t)(size_t) "user_id";
  field.type = SQL_TYPE_INT;
  field.is_aggregate = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_c_query_projection_add_field(NULL, &field));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_c_query_projection_add_field(&proj, NULL));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_add_field(&proj, &field));
  ASSERT_EQ(1, proj.n_fields);
  ASSERT_STR_EQ("id", proj.fields[0].name);
  ASSERT_STR_EQ("user_id", proj.fields[0].original_name);
  ASSERT_EQ(SQL_TYPE_INT, proj.fields[0].type);

  null_field.name = NULL;
  null_field.original_name = NULL;
  null_field.type = SQL_TYPE_VARCHAR;
  null_field.is_aggregate = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_c_query_projection_add_field(&proj, &null_field));
  ASSERT_EQ(2, proj.n_fields);
  ASSERT_EQ(NULL, proj.fields[1].name);
  ASSERT_EQ(NULL, proj.fields[1].original_name);
  ASSERT_EQ(SQL_TYPE_VARCHAR, proj.fields[1].type);
  ASSERT_EQ(1, proj.fields[1].is_aggregate);

  /* Add more fields to trigger capacity doubling (capacity 4 -> 8) */
  for (i = 0; i < 3; i++) {
    ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_add_field(&proj, &field));
  }
  ASSERT_EQ(5, proj.n_fields);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_free(&proj));
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Test OOM error paths in query projection.
 */
TEST test_query_projection_oom(void) {
  cdd_c_query_projection_t proj;
  cdd_c_query_projection_field_t field;

  field.name = (char *)(size_t)(size_t) "col";
  field.original_name = (char *)(size_t)(size_t) "orig";
  field.type = SQL_TYPE_INT;
  field.is_aggregate = 0;

  /* Test C_CDD_REALLOC failure when g_cdd_alloc_fail == 1 */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_init(&proj));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_c_query_projection_add_field(&proj, &field));
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_free(&proj));

  /* Test C_CDD_REALLOC branch when g_cdd_alloc_fail == 2 (first passes, second
   * fails) */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_init(&proj));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_add_field(&proj, &field));
  proj.capacity = proj.n_fields;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_c_query_projection_add_field(&proj, &field));
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_free(&proj));

  /* Test strdup failure on field->name */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_init(&proj));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_c_query_projection_add_field(&proj, &field));
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_free(&proj));

  /* Test strdup failure on field->original_name */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_init(&proj));
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_c_query_projection_add_field(&proj, &field));
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_query_projection_free(&proj));

  g_fail_io_after = -1;
  PASS();
}

SUITE(query_projection_suite) {
  RUN_TEST(test_query_projection_init);
  RUN_TEST(test_query_projection_free);
  RUN_TEST(test_query_projection_add_field);
  RUN_TEST(test_query_projection_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_QUERY_PROJECTION_H */
