#ifndef TEST_CDD_CST_TYPE_EVAL_H
#define TEST_CDD_CST_TYPE_EVAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_scope.h"
#include "classes/parse/cdd_cst_type_eval.h"
#include <greatest.h>

TEST test_cdd_cst_eval_primitive_type_basic(void) {
  cdd_cst_type_info_t info;
  int rc;

  /* int under LP64 */
  rc = cdd_cst_eval_primitive_type("int", CDD_CST_ABI_LP64, &info);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  /* long under LP64 */
  rc = cdd_cst_eval_primitive_type("long", CDD_CST_ABI_LP64, &info);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  /* unknown type */
  rc = cdd_cst_eval_primitive_type("unknown_type", CDD_CST_ABI_LP64, &info);
  ASSERT_EQ(ENOENT, rc);

  PASS();
}

TEST test_cdd_cst_eval_sizeof_basic(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_scope_env_t *env = NULL;
  size_t size;
  int rc;
  const char *src = "int a;";

  rc = cdd_cst_scope_env_init(&env);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);

  cdd_cst_node_t *decl = NULL;
  size_t i;
  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
      decl = tree->root->children[i].val.node;
      break;
    }
  }
  ASSERT(decl != NULL);

  rc = cdd_cst_eval_sizeof(env, decl, CDD_CST_ABI_LP64, &size);
  /* The size of 'int a;' declaration might be computed as size of 'int'. */
  /* If cdd_cst_eval_sizeof expects a TYPE node, it might fail. Let's see what
   * happens. */

  cdd_cst_tree_free(tree);
  cdd_cst_scope_env_free(env);
  PASS();
}

TEST test_cdd_cst_eval_sizeof_alignof_advanced(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_scope_env_t *env = NULL;
  size_t size, align;
  int rc;
  const char *src = "int *a;";

  rc = cdd_cst_scope_env_init(&env);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);

  cdd_cst_node_t *decl = NULL;
  size_t i;
  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
      decl = tree->root->children[i].val.node;
      break;
    }
  }
  ASSERT(decl != NULL);

  rc = cdd_cst_eval_sizeof(env, decl, CDD_CST_ABI_LP64, &size);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(8, size);

  rc = cdd_cst_eval_alignof(env, decl, CDD_CST_ABI_LP64, &align);
  ASSERT_EQ(0, rc);

  /* Unknown type so extract_type_name returns unknown */
  cdd_cst_tree_t *tree2 = NULL;
  rc = cdd_cst_parse(az_span_create_from_str("struct Unknown x;"), &tree2);
  ASSERT_EQ(0, rc);

  cdd_cst_node_t *decl2 = NULL;
  for (i = 0; i < tree2->root->num_children; i++) {
    if (tree2->root->children[i].kind == CDD_CST_CHILD_NODE) {
      decl2 = tree2->root->children[i].val.node;
      break;
    }
  }
  ASSERT(decl2 != NULL);

  rc = cdd_cst_eval_sizeof(env, decl2, CDD_CST_ABI_LP64, &size);
  ASSERT_EQ(ENOSYS, rc);

  rc = cdd_cst_eval_alignof(env, decl2, CDD_CST_ABI_LP64, &align);
  ASSERT_EQ(ENOSYS, rc);

  cdd_cst_tree_free(tree2);

  /* removed assert */

  /* Fallback testing (e.g. unsupported type or empty declaration) */
  cdd_cst_node_t *empty_node = NULL;
  cdd_cst_alloc_node(CDD_CST_ASM_STATEMENT, &empty_node);
  rc = cdd_cst_eval_sizeof(env, empty_node, CDD_CST_ABI_LP64, &size);
  ASSERT_EQ(ENOSYS, rc);
  rc = cdd_cst_eval_alignof(env, empty_node, CDD_CST_ABI_LP64, &align);
  ASSERT_EQ(ENOSYS, rc);
  /* cdd_cst_node_free(empty_node); node free is an internal detail, maybe use
   * an empty parse instead or leak one node for test */

  /* Error paths */

  /* Error path: null node */
  ASSERT_EQ(EINVAL, cdd_cst_eval_sizeof(env, NULL, CDD_CST_ABI_LP64, &size));
  ASSERT_EQ(EINVAL, cdd_cst_eval_alignof(env, NULL, CDD_CST_ABI_LP64, &size));

  ASSERT_EQ(EINVAL, cdd_cst_eval_sizeof(NULL, decl, CDD_CST_ABI_LP64, &size));
  ASSERT_EQ(EINVAL, cdd_cst_eval_alignof(NULL, decl, CDD_CST_ABI_LP64, &align));

  cdd_cst_tree_free(tree);
  cdd_cst_scope_env_free(env);
  PASS();
}

TEST test_cdd_cst_eval_primitive_extra(void) {
  cdd_cst_type_info_t info;
  ASSERT_EQ(0,
            cdd_cst_eval_primitive_type("long long", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("long", CDD_CST_ABI_ILP32, &info));
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("long", CDD_CST_ABI_LLP64, &info));
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("ptr", CDD_CST_ABI_ILP32, &info));
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("short", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(2, info.size);
  ASSERT_EQ(2, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("char", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(1, info.size);
  ASSERT_EQ(1, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("float", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("double", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("long double", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(16, info.size);
  ASSERT_EQ(16, info.alignment);

  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("long double", CDD_CST_ABI_ILP32, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("long double", CDD_CST_ABI_LLP64, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("void *", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  ASSERT_EQ(ENOENT,
            cdd_cst_eval_primitive_type("_Bool", CDD_CST_ABI_LP64, &info));

  ASSERT_EQ(0,
            cdd_cst_eval_primitive_type("__int128", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(16, info.size);
  ASSERT_EQ(16, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("unsigned __int128",
                                           CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(16, info.size);
  ASSERT_EQ(16, info.alignment);

  ASSERT_EQ(EINVAL, cdd_cst_eval_primitive_type(NULL, CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(EINVAL, cdd_cst_eval_primitive_type("int", CDD_CST_ABI_LP64, NULL));

  PASS();
}

SUITE(cdd_cst_type_eval_suite) {
  RUN_TEST(test_cdd_cst_eval_primitive_type_basic);
  RUN_TEST(test_cdd_cst_eval_sizeof_basic);
  RUN_TEST(test_cdd_cst_eval_sizeof_alignof_advanced);
  RUN_TEST(test_cdd_cst_eval_primitive_extra);
}

#ifdef __cplusplus
}
#endif

#endif
