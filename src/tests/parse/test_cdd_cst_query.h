#ifndef TEST_CDD_CST_QUERY_H
#define TEST_CDD_CST_QUERY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
/* clang-format on */

TEST test_cdd_cst_query_types(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  return 0;\n}";
  cdd_cst_query_result_t res;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_FUNCTION_DEFINITION, &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, res.size);
  free(res.nodes);

  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_UNKNOWN,
                                  &res); /* the return stmt */
  ASSERT_EQ(0, rc);
  ASSERT(res.size > 0);
  free(res.nodes);

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_cst_query_calls(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  printf(\"hello\");\n  return 0;\n}";
  cdd_cst_query_result_t res;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_find_function_calls_named(tree->root, "printf", &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, res.size);
  free(res.nodes);

  rc = cdd_cst_find_function_calls_named(tree->root, "malloc", &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, res.size);
  free(res.nodes);

  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(cdd_cst_query_suite) {
  RUN_TEST(test_cdd_cst_query_types);
  RUN_TEST(test_cdd_cst_query_calls);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_QUERY_H */
