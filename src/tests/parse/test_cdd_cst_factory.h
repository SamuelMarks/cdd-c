/* clang-format off */
#ifndef TEST_CDD_CST_FACTORY_H
#define TEST_CDD_CST_FACTORY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <greatest.h>
#include "../../classes/parse/cdd_cst_factory.h"
#include "../../classes/parse/cdd_cst_parser.h"
/* clang-format on */

TEST test_cdd_cst_factory_format(void) {
  cdd_cst_tree_t *dest_tree = NULL;
  cdd_cst_node_t *out_node = NULL;
  int rc;

  dest_tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));

  /* Test basic formatting */
  rc = cdd_cst_parse_format(dest_tree, &out_node, "int %s = %d;", "my_var", 42);
  ASSERT_EQ(0, rc);
  ASSERT_NEQ(NULL, out_node);

  if (out_node) {
    cdd_cst_free_node_only(out_node);
  }

  /* Test error formatting invalid syntax to trigger empty parse */
  rc = cdd_cst_parse_format(dest_tree, &out_node, "/* just a comment */");
  ASSERT_EQ(0, rc);
  ASSERT_NEQ(NULL, out_node);
  if (out_node) {
    cdd_cst_free_node_only(out_node);
  }

  /* Test appending multiple statements */
  rc = cdd_cst_parse_format(dest_tree, &out_node, "int a; int b; int c;");
  ASSERT_EQ(0, rc);
  ASSERT_NEQ(NULL, out_node);
  if (out_node) {
    cdd_cst_free_node_only(out_node);
  }

  /* Missing parameters / allocation failure mock not simple without framework,
   * check out out NULL handling */
  rc = cdd_cst_parse_format(dest_tree, &out_node, "");
  if (out_node) {
    cdd_cst_free_node_only(out_node);
  }

  cdd_cst_tree_free(dest_tree);

  PASS();
}

SUITE(cdd_cst_factory_suite) { RUN_TEST(test_cdd_cst_factory_format); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_FACTORY_H */
