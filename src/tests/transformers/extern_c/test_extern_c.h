/**
 * @file test_extern_c.h
 * @brief Unit tests for extern C transformer.
 */

#ifndef TEST_CDD_TRANSFORM_EXTERN_C_H
#define TEST_CDD_TRANSFORM_EXTERN_C_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/emit/cdd_cst_emit.h"
#include "c_str_span.h"
/* clang-format on */

/**
 * @brief Tests basic functionality of the extern C transformer.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_extern_c(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "/* license */\n#include <stdio.h>\n\nint main() {\n  return 0;\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "extern \"C\" {") != NULL);
  ASSERT(strstr(out, "}") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Tests extern C transformer when guards already exist.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_extern_c_already_exists(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#ifdef __cplusplus\nextern \"C\" {\n#endif\nint "
                     "main(){}\n#ifdef __cplusplus\n}\n#endif\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Extern C transformer test suite.
 */
SUITE(transformer_extern_c_suite) {
  RUN_TEST(test_cdd_transform_extern_c);
  RUN_TEST(test_cdd_transform_extern_c_already_exists);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_EXTERN_C_H */
