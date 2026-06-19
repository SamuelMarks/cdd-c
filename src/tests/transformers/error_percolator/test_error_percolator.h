extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_error_percolator.h
 * @brief Unit tests for error percolator transformer.
 */

#ifndef TEST_CDD_TRANSFORM_ERROR_PERCOLATOR_H
#define TEST_CDD_TRANSFORM_ERROR_PERCOLATOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/emit/cdd_cst_emit.h"
#include "c_str_span.h"
/* clang-format on */

/**
 * @brief Tests basic functionality of error percolation.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_percolate_errors(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "void foo() {\n  int *ptr = malloc(sizeof(int));\n  return;\n}\nvoid "
      "bar(int a) {\n  void *b = calloc(1, 1);\n  foo();\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_percolate_errors(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  fprintf(stderr, "OUT WAS:\n%s\n", out);
  ASSERT(strstr(out, "int foo(void **out_result)") != NULL);
  ASSERT(strstr(out, "if (!ptr) { return ENOMEM; }") != NULL);
  ASSERT(strstr(out, "rc = 0; goto cleanup;") != NULL);
  printf("OUT WAS:\n%s\n", out);
  fprintf(stderr, "OUT FOR FOO:\n%s\n", out);
  ASSERT(strstr(out, "cleanup:") != NULL);
  ASSERT(strstr(out, "return rc;") != NULL);
  ASSERT(strstr(out, "int bar(int a, void **out_result)") != NULL);
  ASSERT(strstr(out, "if (!b) { return ENOMEM; }") != NULL);
  ASSERT(strstr(out, "if (foo(&out_foo) != 0)") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests error percolation with complex allocations.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_percolate_errors_complex(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void complex_allocs() {\n"
                     "  int *p1 = malloc(1);\n"
                     "  int *p2 = malloc(2);\n"
                     "  return;\n"
                     "}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_percolate_errors(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "if (!p1) { return ENOMEM; }") != NULL);
  ASSERT(strstr(out, "if (!p2) { rc = ENOMEM; goto cleanup; }") != NULL);
  ASSERT(strstr(out, "rc = 0; goto cleanup;") != NULL);
  printf("OUT WAS:\n%s\n", out);
  fprintf(stderr, "OUT FOR FOO:\n%s\n", out);
  ASSERT(strstr(out, "cleanup:") != NULL);
  ASSERT(strstr(out, "return rc;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests edge cases for error percolation.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_percolate_errors_edge_cases(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "int x = 5;\n"
      "void nothing() { int y = 6; }\n"
      "void foo(int x) { int *p = malloc(1); }\n"
      "void calls_foo() { foo(1); foo(2); }\n"
      "void nested() { if (1) { int *p = calloc(1, 1); } }\n"
      "void myrealloc() { void *p = realloc(NULL, 10); }\n"
      "void ret() { void *p = malloc(1); if(p) return; return; }\n"
      "void trailing_ident() { int trailing }\n"
      "void non_call() { int x; x = 5; }\n"
      "void skip_macro() { MY_MACRO }\n"
      "void skip_ident() { int x; x; }\n"
      "void func_void() { }\n"
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
  {
    cdd_cst_node_t *unknown_node = calloc(1, sizeof(cdd_cst_node_t));
    cdd_token_t *rbrace_tok = calloc(1, sizeof(cdd_token_t));
    cdd_cst_child_t *c = calloc(1, sizeof(cdd_cst_child_t));
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
  }

  cdd_transform_percolate_errors(tree, &config);
  cdd_cst_tree_free(tree);
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

#endif /* TEST_CDD_TRANSFORM_ERROR_PERCOLATOR_H */
