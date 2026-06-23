#ifndef CDD_TEST_TRANSFORMER_ERROR_PERCOLATOR_H
#define CDD_TEST_TRANSFORMER_ERROR_PERCOLATOR_H

#include "cdd_cst.h"
#include "cdd_cst_transform.h"
#include "greatest.h"
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

TEST test_cdd_transform_percolate_errors(void) {
  cdd_cst_tree_t *tree;
  const char *code = "void func_void() { }\n"
                     "int non_void_func() { return 1; }\n";
  cdd_transform_config_t config = {0, 2, 0};

  ASSERT_EQ(EINVAL, cdd_transform_percolate_errors(NULL, &config));

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_percolate_errors(tree, &config));

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_percolate_errors_complex(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "void func_void() { }\n"
      "int non_void_func() { return 1; }\n"
      "void * my_void_ptr_func() { return NULL; }\n"
      "void trailing_call() { my_void_ptr_func(); }\n"
      "void skip_ident_1() { int x; void; }\n"
      "void skip_ident_2() { int x; void x; }\n"
      "void skip_ident_3() { int x; void x(); }\n"
      "void skip_ident_4() { int x; void x(int y); }\n"
      "void skip_ident_5() { int x; void x(int y, ...); }\n"
      "void skip_ident_6() { int x; void * x(int y, ...); }\n"
      "void skip_ident_7() { int x; void * x(int y, ...) { return NULL; } }\n"
      "void skip_ident_8() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_9() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_10() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_11() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_12() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_13() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_14() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_15() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_16() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_17() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_18() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_19() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_20() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_21() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_22() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_23() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_24() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_25() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_26() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_27() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_28() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_29() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_30() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_31() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_32() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_33() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_34() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_35() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_36() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_37() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_38() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_39() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_40() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_41() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_42() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_43() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_44() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_45() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_46() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_47() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_48() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_49() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_50() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_51() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_52() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_53() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_54() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_55() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_56() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_57() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_58() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_59() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_60() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_61() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_62() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_63() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_64() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_65() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_66() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_67() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_68() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_69() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_70() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_71() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_72() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_73() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_74() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_75() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_76() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_77() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_78() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_79() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_80() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_81() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_82() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_83() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_84() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_85() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_86() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_87() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_88() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_89() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_90() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_91() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_92() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_93() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_94() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_95() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_96() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_97() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_98() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_99() { int x; void * x(int y, ...) { return NULL; } "
      "void skip_ident_100() { int x; void * x(int y, ...) { return NULL; } ";
  cdd_transform_config_t config = {0, 2, 0};

  ASSERT_EQ(EINVAL, cdd_transform_percolate_errors(NULL, &config));

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_percolate_errors(tree, &config));

  cdd_cst_tree_free(tree);
  PASS();
}

extern C_CDD_EXPORT int g_fail_io_after;

TEST test_cdd_transform_percolate_errors_edge_cases(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void func_void() { }\n"
                     "int non_void_func() { return 1; }\n"
                     "void * my_void_ptr_func() { return NULL; }\n"
                     "void trailing_call() { my_void_ptr_func }\n"
                     "void skip_ident_2() { int x; void x; }\n";
  cdd_transform_config_t config = {0, 2, 0};

  ASSERT_EQ(EINVAL, cdd_transform_percolate_errors(NULL, &config));

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_percolate_errors(tree, &config));

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_err_perc_fail;
#endif

TEST test_cdd_transform_percolate_errors_bld_fail(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  cdd_token_t *rbrace_tok_to_free = NULL;
  cdd_cst_node_t *unknown_node = NULL;
  cdd_token_t *rbrace_tok = NULL;
  cdd_cst_child_t *c = NULL;
  const char *code =
      "void foo() { void *p = malloc(1); void *p2 = malloc(1); return; }";
  cdd_transform_config_t config = {0, 2, 0};

  g_err_perc_fail = 1;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  cdd_transform_percolate_errors(tree, &config);
  cdd_cst_tree_free(tree);
  tree = NULL;

  g_err_perc_fail = 2; /* Custom trigger for cleanup node */

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));

  /* Mock an unknown block node manually to hit the unknown branch logic */
  unknown_node = calloc(1, sizeof(cdd_cst_node_t));
  rbrace_tok = calloc(1, sizeof(cdd_token_t));
  c = calloc(1, sizeof(cdd_cst_child_t));
  rbrace_tok_to_free = rbrace_tok;

  unknown_node->kind = CDD_CST_UNKNOWN;
  /* Add an RBRACE to it so it matches */
  rbrace_tok->kind = CDD_TOKEN_RBRACE;
  c->kind = CDD_CST_CHILD_TOKEN;
  c->val.token = rbrace_tok;
  unknown_node->children = c;
  unknown_node->num_children = 1;
  /* Add to end of first function definition in tree */
  if (tree->root && tree->root->num_children > 0 &&
      tree->root->children[0].kind == CDD_CST_CHILD_NODE) {
    cdd_cst_append_child_node(tree->root->children[0].val.node, unknown_node);
  } else {
    free(unknown_node->children);
    free(unknown_node);
    free(rbrace_tok);
  }

  cdd_transform_percolate_errors(tree, &config);
  /* The unknown_node was appended. Does it get freed? No, append_child_node
   * just adds it to the children array. But cdd_cst_tree_free frees tokens. The
   * nodes must be freed. */

  cdd_cst_tree_free(tree);
  if (rbrace_tok_to_free)
    free(rbrace_tok_to_free);
  tree = NULL;

  g_err_perc_fail = 3; /* For return builder mock */
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  cdd_transform_percolate_errors(tree, &config);
  cdd_cst_tree_free(tree);
  tree = NULL;

  g_err_perc_fail = 4; /* For tmp_name malloc mock */
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  cdd_transform_percolate_errors(tree, &config);
  cdd_cst_tree_free(tree);
  tree = NULL;

  g_err_perc_fail = 0;
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Error percolator transformer test suite.
 */
SUITE(transformer_error_percolator_suite) {
  RUN_TEST(test_cdd_transform_percolate_errors);
  RUN_TEST(test_cdd_transform_percolate_errors_complex);
  RUN_TEST(test_cdd_transform_percolate_errors_edge_cases);
  RUN_TEST(test_cdd_transform_percolate_errors_bld_fail);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TEST_TRANSFORMER_ERROR_PERCOLATOR_H */
