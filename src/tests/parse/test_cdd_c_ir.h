/**
 * @file test_cdd_c_ir.h
 * @brief Tests for CDD-C IR parsers.
 */

#ifndef C_CDD_TEST_CDD_C_IR_H
#define C_CDD_TEST_CDD_C_IR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/parse/cdd_c_ir.h"
#include <greatest.h>
/* clang-format on */

TEST test_parse_sql_select(void) {
  const char *sql = "SELECT id, name AS user_name, COUNT(age) FROM users;";
  cdd_c_ir_t ir;

  ASSERT_EQ(0, cdd_c_ir_init(&ir));
  ASSERT_EQ(0, parse_sql_into_ir(sql, &ir));

  ASSERT_EQ(0, ir.n_tables);
  ASSERT_EQ(1, ir.n_projections);

  ASSERT_EQ(3, ir.projections[0].n_fields);

  /* Field 1: id */
  ASSERT_STR_EQ("id", ir.projections[0].fields[0].name);
  ASSERT_EQ(NULL, ir.projections[0].fields[0].original_name);
  ASSERT_EQ(0, ir.projections[0].fields[0].is_aggregate);
  ASSERT_EQ(0, ir.projections[0].fields[0].length);
  ASSERT_EQ(0, ir.projections[0].fields[0].is_array);

  /* Field 2: name AS user_name */
  ASSERT_STR_EQ("user_name", ir.projections[0].fields[1].name);
  ASSERT_STR_EQ("name", ir.projections[0].fields[1].original_name);
  ASSERT_EQ(0, ir.projections[0].fields[1].is_aggregate);
  ASSERT_EQ(0, ir.projections[0].fields[1].length);
  ASSERT_EQ(0, ir.projections[0].fields[1].is_array);

  /* Field 3: COUNT(age) */
  ASSERT_STR_EQ("COUNT", ir.projections[0].fields[2].name);
  ASSERT_EQ(1, ir.projections[0].fields[2].is_aggregate);
  ASSERT_EQ(SQL_TYPE_BIGINT,
            ir.projections[0].fields[2].type); /* Type inference */
  ASSERT_EQ(0, ir.projections[0].fields[2].length);
  ASSERT_EQ(0, ir.projections[0].fields[2].is_array);

  ASSERT_STR_EQ("users", ir.projections[0].source_table);

  ASSERT_EQ(0, cdd_c_ir_free(&ir));
  PASS();
}

TEST test_parse_sql_returning(void) {
  const char *sql =
      "INSERT INTO users (name) VALUES ('test') RETURNING id, name;";
  cdd_c_ir_t ir;

  ASSERT_EQ(0, cdd_c_ir_init(&ir));
  ASSERT_EQ(0, parse_sql_into_ir(sql, &ir));

  ASSERT_EQ(0, ir.n_tables);
  ASSERT_EQ(1, ir.n_projections); /* Should pick up the returning clause */

  ASSERT_EQ(2, ir.projections[0].n_fields);
  ASSERT_STR_EQ("id", ir.projections[0].fields[0].name);
  ASSERT_STR_EQ("name", ir.projections[0].fields[1].name);

  ASSERT_EQ(0, cdd_c_ir_free(&ir));
  PASS();
}

SUITE(cdd_c_ir_suite) {
  RUN_TEST(test_parse_sql_select);
  RUN_TEST(test_parse_sql_returning);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_CDD_C_IR_H */
