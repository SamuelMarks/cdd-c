/**
 * @file test_cdd_cst_trivia.h
 * @brief Unit tests for CST trivia operations.
 */

#ifndef TEST_CDD_CST_TRIVIA_H
#define TEST_CDD_CST_TRIVIA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_trivia.h"
/* clang-format on */

/**
 * @brief Tests basic whitespace trivia detection.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_trivia_detect(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n    return 0;\n}";
  cdd_cst_format_config_t config;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(EINVAL, cdd_cst_detect_format_config(NULL, &config));
  ASSERT_EQ(EINVAL, cdd_cst_detect_format_config(tree, NULL));

  rc = cdd_cst_detect_format_config(tree, &config);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, config.use_tabs);
  ASSERT_EQ(4, config.indent_width); /* "    " is 4 spaces */

  cdd_cst_tree_free(tree);

  cdd_cst_tree_t tree2 = {0};
  ASSERT_EQ(0, cdd_cst_detect_format_config(&tree2, &config));

  tree = NULL;
  code = "int main() {\n  return 0;\n}";
  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_detect_format_config(tree, &config);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, config.use_tabs);
  ASSERT_EQ(2, config.indent_width); /* "  " is 2 spaces */
  cdd_cst_tree_free(tree);

  PASS();
}

/**
 * @brief Tests tab-based trivia detection.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_trivia_detect_tabs(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n\t\treturn 0;\n}";
  cdd_cst_format_config_t config;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_detect_format_config(tree, &config);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, config.use_tabs);

  cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Tests generation of indent trivia.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_trivia_generate(void) {
  cdd_trivia_t *t = NULL;
  cdd_cst_format_config_t config = {0, 4};
  cdd_cst_format_config_t config_tabs = {1, 1};
  int rc;

  ASSERT_EQ(EINVAL, cdd_cst_generate_indent_trivia(NULL, NULL, 2, &t));
  ASSERT_EQ(EINVAL, cdd_cst_generate_indent_trivia(NULL, &config, 2, NULL));

  ASSERT_EQ(0, cdd_cst_generate_indent_trivia(NULL, &config, 0, &t));
  ASSERT(t != NULL);
  ASSERT_EQ(TRIVIA_NEWLINE, t->kind);
  ASSERT_EQ(NULL, t->next);
  free(t);
  t = NULL;

  rc = cdd_cst_generate_indent_trivia(NULL, &config, 2, &t);
  ASSERT_EQ(0, rc);
  ASSERT(t != NULL);
  ASSERT_EQ(TRIVIA_NEWLINE, t->kind);
  ASSERT(t->next != NULL);
  ASSERT_EQ(TRIVIA_WHITESPACE, t->next->kind);
  ASSERT_EQ(8, t->next->length);

  free((void *)t->next->start);
  free(t->next);
  free(t);

  rc = cdd_cst_generate_indent_trivia(NULL, &config_tabs, 2, &t);
  ASSERT_EQ(0, rc);
  ASSERT(t != NULL);
  ASSERT_EQ(TRIVIA_NEWLINE, t->kind);
  ASSERT(t->next != NULL);
  ASSERT_EQ(TRIVIA_WHITESPACE, t->next->kind);
  ASSERT_EQ(2, t->next->length);
  ASSERT_EQ('\t', t->next->start[0]);

  free((void *)t->next->start);
  free(t->next);
  free(t);

  PASS();
}

/**
 * @brief CST trivia test suite.
 */
SUITE(cdd_cst_trivia_suite) {
  RUN_TEST(test_cdd_cst_trivia_detect);
  RUN_TEST(test_cdd_cst_trivia_detect_tabs);
  RUN_TEST(test_cdd_cst_trivia_generate);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_TRIVIA_H */
