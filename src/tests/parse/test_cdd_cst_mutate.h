#ifndef TEST_CDD_CST_MUTATE_H
#define TEST_CDD_CST_MUTATE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_query.h"
#include "classes/emit/cdd_cst_emit.h"
/* clang-format on */

TEST test_cdd_cst_mutate_replace(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  return 0;\n}";
  char *out = NULL;
  cdd_cst_query_result_t res;
  int rc;
  cdd_cst_node_t *target;
  cdd_cst_node_t *clone = NULL;

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_STATEMENT, &res);
  /* Wait, our parser puts it in CDD_CST_UNKNOWN since we didn't specify
   * STATEMENT */
  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_UNKNOWN, &res);
  ASSERT_EQ(0, rc);
  ASSERT(res.size > 0);

  target = res.nodes[0];
  rc = cdd_cst_clone_tree(tree, target, &clone);
  ASSERT_EQ(0, rc);

  /* Just test replace works without crashing and unparses */
  rc = cdd_cst_replace_node(tree, target, clone);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ(code, out); /* Same code after replace with clone */

  free(res.nodes);
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(cdd_cst_mutate_suite) { RUN_TEST(test_cdd_cst_mutate_replace); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_MUTATE_H */
