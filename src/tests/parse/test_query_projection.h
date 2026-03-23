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
#include "classes/parse/query_projection.h"
#include <greatest.h>
/* clang-format on */

TEST test_query_projection_init(void) {
  cdd_c_query_projection_t proj;
  ASSERT_EQ(0, cdd_c_query_projection_init(&proj));
  ASSERT_EQ(0, proj.n_fields);
  ASSERT_EQ(NULL, proj.fields);
  PASS();
}

TEST test_query_projection_add_field(void) {
  cdd_c_query_projection_t proj;
  cdd_c_query_projection_field_t field;

  ASSERT_EQ(0, cdd_c_query_projection_init(&proj));

  field.name = (char *)"id";
  field.original_name = (char *)"user_id";
  field.type = SQL_TYPE_INT;
  field.is_aggregate = 0;

  ASSERT_EQ(0, cdd_c_query_projection_add_field(&proj, &field));
  ASSERT_EQ(1, proj.n_fields);
  ASSERT_STR_EQ("id", proj.fields[0].name);
  ASSERT_STR_EQ("user_id", proj.fields[0].original_name);
  ASSERT_EQ(SQL_TYPE_INT, proj.fields[0].type);

  ASSERT_EQ(0, cdd_c_query_projection_free(&proj));
  PASS();
}

SUITE(query_projection_suite) {
  RUN_TEST(test_query_projection_init);
  RUN_TEST(test_query_projection_add_field);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_QUERY_PROJECTION_H */
