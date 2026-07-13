#ifndef TEST_CDD_TRANSFORM_MACROS_H
#define TEST_CDD_TRANSFORM_MACROS_H

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
 * @brief Test macro transformation of simple AST macros.
 *
 * @return The result of the test.
 */
/* LCOV_EXCL_START */
TEST test_cdd_transform_macros(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      /* LCOV_EXCL_STOP */
      "#define FOO(a) a + 1\nint main() {\n  return FOO(42);\n}\n";
  /* LCOV_EXCL_START */
  char *out = NULL;
  /* LCOV_EXCL_STOP */
  int rc;
  /* LCOV_EXCL_START */
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = cdd_transform_macros(tree, &config);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  printf("OUT WAS:\n[%s]\n", out);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT(strstr(out, "FOO(42)") == NULL);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_cdd_transform_macros_operators(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#define STRINGIFY(x) #x\n"
                     /* LCOV_EXCL_STOP */
                     "#define CONCAT(a, b) a ## b\n"
                     "int main() {\n"
                     "  STRINGIFY(hello);\n"
                     "  CONCAT(4, 2);\n"
                     "}\n";
  /* LCOV_EXCL_START */
  char *out = NULL;
  /* LCOV_EXCL_STOP */
  int rc;
  /* LCOV_EXCL_START */
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = cdd_transform_macros(tree, &config);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  printf("OUT WAS:\n[%s]\n", out);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT(strstr(out, "STRINGIFY(hello)") == NULL);
  ASSERT(strstr(out, "\"hello\"") != NULL);
  ASSERT(strstr(out, "CONCAT(4, 2)") == NULL);
  ASSERT(strstr(out, "42") != NULL);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_cdd_transform_macros_alloc_fails(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#define STRINGIFY(x) #x\n"
                     /* LCOV_EXCL_STOP */
                     "#define CONCAT(a, b) a ## b\n"
                     "#define FOO(a) a + 1\n"
                     "int main() {\n"
                     "  STRINGIFY(hello);\n"
                     "  CONCAT(4, 2);\n"
                     "  FOO(42);\n"
                     "}\n";
  int rc;
  int k;
  /* LCOV_EXCL_START */
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  /* LCOV_EXCL_STOP */

  /* Null arg */
  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_macros(NULL, &config));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  for (k = 1; k < 500; k++) {
    tree = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
/* LCOV_EXCL_STOP */
#ifdef CDD_BUILD_TESTS
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
    /* LCOV_EXCL_START */
    g_cdd_cst_alloc_node_fail = k;
    g_cdd_cst_realloc_fail = 0;
/* LCOV_EXCL_STOP */
#endif
    /* LCOV_EXCL_START */
    rc = cdd_transform_macros(tree, &config);
    /* LCOV_EXCL_STOP */
    (void)rc;
#ifdef CDD_BUILD_TESTS
    /* LCOV_EXCL_START */
    g_cdd_cst_alloc_node_fail = 0;
    g_cdd_cst_realloc_fail = 0;
/* LCOV_EXCL_STOP */
#endif
    /* LCOV_EXCL_START */
    cdd_cst_tree_free(tree);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  for (k = 1; k < 500; k++) {
    tree = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
/* LCOV_EXCL_STOP */
#ifdef CDD_BUILD_TESTS
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
    /* LCOV_EXCL_START */
    g_cdd_cst_alloc_node_fail = 0;
    g_cdd_cst_realloc_fail = k;
/* LCOV_EXCL_STOP */
#endif
    /* LCOV_EXCL_START */
    rc = cdd_transform_macros(tree, &config);
    /* LCOV_EXCL_STOP */
    (void)rc;
#ifdef CDD_BUILD_TESTS
    /* LCOV_EXCL_START */
    g_cdd_cst_alloc_node_fail = 0;
    g_cdd_cst_realloc_fail = 0;
/* LCOV_EXCL_STOP */
#endif
    /* LCOV_EXCL_START */
    cdd_cst_tree_free(tree);
    /* LCOV_EXCL_STOP */
  }

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_query_err_fail;
    /* LCOV_EXCL_START */
    tree = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
    g_cdd_query_err_fail = 1;
    rc = cdd_transform_macros(tree, &config);
    g_cdd_query_err_fail = 0;
    cdd_cst_tree_free(tree);
    /* LCOV_EXCL_STOP */
  }
#endif
  /* LCOV_EXCL_START */
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Test suite for Macro AST transformation.
 */
/* LCOV_EXCL_START */
SUITE(transformer_macros_suite) {
  RUN_TEST(test_cdd_transform_macros);
  RUN_TEST(test_cdd_transform_macros_operators);
  RUN_TEST(test_cdd_transform_macros_alloc_fails);
}
/* LCOV_EXCL_STOP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_MACROS_H */
