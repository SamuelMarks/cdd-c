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
TEST test_cdd_transform_macros(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "#define FOO(a) a + 1\nint main() {\n  return FOO(42);\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_macros(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  printf("OUT WAS:\n[%s]\n", out);

  ASSERT(strstr(out, "FOO(42)") == NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_macros_operators(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#define STRINGIFY(x) #x\n"
                     "#define CONCAT(a, b) a ## b\n"
                     "int main() {\n"
                     "  STRINGIFY(hello);\n"
                     "  CONCAT(4, 2);\n"
                     "}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_macros(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  printf("OUT WAS:\n[%s]\n", out);

  ASSERT(strstr(out, "STRINGIFY(hello)") == NULL);
  ASSERT(strstr(out, "\"hello\"") != NULL);
  ASSERT(strstr(out, "CONCAT(4, 2)") == NULL);
  ASSERT(strstr(out, "42") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_macros_alloc_fails(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#define STRINGIFY(x) #x\n"
                     "#define CONCAT(a, b) a ## b\n"
                     "#define FOO(a) a + 1\n"
                     "int main() {\n"
                     "  STRINGIFY(hello);\n"
                     "  CONCAT(4, 2);\n"
                     "  FOO(42);\n"
                     "}\n";
  int rc;
  int k;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  /* Null arg */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_macros(NULL, &config));

  for (k = 1; k < 500; k++) {
    tree = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
#ifdef CDD_BUILD_TESTS
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
    g_cdd_cst_alloc_node_fail = k;
    g_cdd_cst_realloc_fail = 0;
#endif
    rc = cdd_transform_macros(tree, &config);
    (void)rc;
#ifdef CDD_BUILD_TESTS
    g_cdd_cst_alloc_node_fail = 0;
    g_cdd_cst_realloc_fail = 0;
#endif
    cdd_cst_tree_free(tree);
  }

  for (k = 1; k < 500; k++) {
    tree = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
#ifdef CDD_BUILD_TESTS
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
    g_cdd_cst_alloc_node_fail = 0;
    g_cdd_cst_realloc_fail = k;
#endif
    rc = cdd_transform_macros(tree, &config);
    (void)rc;
#ifdef CDD_BUILD_TESTS
    g_cdd_cst_alloc_node_fail = 0;
    g_cdd_cst_realloc_fail = 0;
#endif
    cdd_cst_tree_free(tree);
  }

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_query_err_fail;
    tree = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
    g_cdd_query_err_fail = 1;
    rc = cdd_transform_macros(tree, &config);
    g_cdd_query_err_fail = 0;
    cdd_cst_tree_free(tree);
  }
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Test suite for Macro AST transformation.
 */
SUITE(transformer_macros_suite) {
  RUN_TEST(test_cdd_transform_macros);
  RUN_TEST(test_cdd_transform_macros_operators);
  RUN_TEST(test_cdd_transform_macros_alloc_fails);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_MACROS_H */
