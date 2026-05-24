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
TEST test_cdd_transform_extern_c_null_args(void) {
  cdd_transform_config_t config = {0, 2, 0};
  cdd_cst_tree_t tree = {0};
  ASSERT_EQ(EINVAL, cdd_transform_extern_c(NULL, &config));

  ASSERT_EQ(EINVAL, cdd_transform_extern_c(&tree, &config));
  PASS();
}

TEST test_cdd_transform_extern_c_empty_tree(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0};

  root->kind = CDD_CST_TRANSLATION_UNIT;
  tree->root = root;

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

TEST test_cdd_transform_extern_c_empty_c_file(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "   \n";
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

TEST test_cdd_transform_extern_c_malformed_ifdef(void) {
  cdd_cst_tree_t *tree = NULL;
  /* Missing the identifier after #ifdef */
  const char *code = "#ifdef \nint main() { return 0; }\n";
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

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_extern_c_already_exists(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
  cdd_token_t *tok_ifdef = calloc(1, sizeof(cdd_token_t));
  cdd_token_t *tok_cpp = calloc(1, sizeof(cdd_token_t));
  int rc;
  cdd_transform_config_t config = {0, 2, 0};

  tok_ifdef->kind = CDD_TOKEN_PREPROC_IFDEF;
  tok_ifdef->start = (const uint8_t *)"#ifdef";
  tok_ifdef->length = 6;

  tok_cpp->kind = CDD_TOKEN_IDENTIFIER;
  tok_cpp->start = (const uint8_t *)"__cplusplus";
  tok_cpp->length = 11;

  root->kind = CDD_CST_TRANSLATION_UNIT;
  dir->kind = CDD_CST_PREPROC_DIRECTIVE;
  cdd_cst_append_child_token(dir, tok_ifdef);
  cdd_cst_append_child_token(dir, tok_cpp);
  cdd_cst_append_child_node(root, dir);
  tree->root = root;

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc); /* should return 0 directly without changes */
  ASSERT_EQ(1, root->num_children); /* nothing added */

  /* Mutate token to test branch logic where it doesn't match __cplusplus */
  tok_cpp->length = 10;
  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(3, root->num_children); /* added top and bottom nodes */

  /* Cleanup */
  free(tok_ifdef);
  free(tok_cpp);
  cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Extern C transformer test suite.
 */
SUITE(transformer_extern_c_suite) {
  RUN_TEST(test_cdd_transform_extern_c);
  RUN_TEST(test_cdd_transform_extern_c_null_args);
  RUN_TEST(test_cdd_transform_extern_c_empty_tree);
  RUN_TEST(test_cdd_transform_extern_c_empty_c_file);
  RUN_TEST(test_cdd_transform_extern_c_malformed_ifdef);
  RUN_TEST(test_cdd_transform_extern_c_already_exists);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_EXTERN_C_H */
