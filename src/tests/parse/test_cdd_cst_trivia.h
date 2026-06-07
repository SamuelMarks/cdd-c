extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
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
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_trivia.h"
/* clang-format on */
/* LCOV_EXCL_START */

/**
 * @brief Tests basic whitespace trivia detection.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_trivia_detect(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n    return 0;\n}";
  cdd_cst_format_config_t config;
  cdd_cst_tree_t tree2;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  memset(&tree2, 0, sizeof(tree2));

  ASSERT_EQ(EINVAL, cdd_cst_detect_format_config(NULL, &config));
  ASSERT_EQ(EINVAL, cdd_cst_detect_format_config(tree, NULL));

  rc = cdd_cst_detect_format_config(tree, &config);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, config.use_tabs);
  ASSERT_EQ(4, config.indent_width); /* "    " is 4 spaces */

  cdd_cst_tree_free(tree);

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

  /* Test fallback to 4 spaces */
  tree = NULL;
  code = "int main() {\n     return 0;\n}"; /* 5 spaces avg >= 3 -> 4 spaces */
  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_detect_format_config(tree, &config);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, config.use_tabs);
  ASSERT_EQ(4, config.indent_width);
  cdd_cst_tree_free(tree);

  /* Test spaces > 8 ignored from sum */
  tree = NULL;
  code = "int main() {\n         return 0;\n}"; /* 9 spaces ignored */
  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_detect_format_config(tree, &config);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, config.use_tabs);
  ASSERT_EQ(2, config.indent_width); /* default if no space counts added */
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;
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
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief CST trivia test suite.
 */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;

TEST test_cdd_cst_trivia_oom(void) {
  cdd_cst_format_config_t config = {0};
  cdd_trivia_t *out = NULL;
  int rc_tmp, rc_tmp3;

  g_cdd_cst_alloc_token_fail = 1;
  rc_tmp = cdd_cst_generate_indent_trivia(NULL, &config, 1, &out);
  if (rc_tmp != ENOMEM) {
    printf("cdd_cst_generate_indent_trivia = %d, expected ENOMEM\n", rc_tmp);
  }
  ASSERT(rc_tmp != 0);

  g_cdd_cst_alloc_token_fail = 2;
  rc_tmp = cdd_cst_generate_indent_trivia(NULL, &config, 1, &out);
  if (rc_tmp != ENOMEM) {
    printf("cdd_cst_generate_indent_trivia = %d, expected ENOMEM\n", rc_tmp);
  }
  ASSERT(rc_tmp != 0);

  g_cdd_cst_alloc_token_fail = 3;
  rc_tmp3 = cdd_cst_generate_indent_trivia(NULL, &config, 1, &out);
  ASSERT(rc_tmp3 != 0);
  g_cdd_cst_alloc_token_fail = 0;
  g_fail_io_after = -1;

  PASS();
}
#endif

TEST test_trivia_branches(void) {
  cdd_cst_tree_t tree = {0};
  cdd_token_list_t lst = {0};
  cdd_trivia_t t1 = {0};
  cdd_trivia_t t2 = {0};
  cdd_trivia_t t3 = {0};
  cdd_cst_format_config_t config = {0};
  tree.base_tokens = &lst;
  lst.size = 1;
  lst.capacity = 1;
  lst.tokens = calloc(1, sizeof(cdd_token_t));

  t1.kind = TRIVIA_WHITESPACE; /* Not newline */

  t2.kind = TRIVIA_NEWLINE;
  t2.start = (const uint8_t *)"abc";
  t2.length = 3; /* No newline char */

  t3.kind = TRIVIA_NEWLINE;
  t3.start = (const uint8_t *)"\n\r";
  t3.length = 2; /* After newline is not space/tab */

  t1.next = &t2;
  t2.next = &t3;
  lst.tokens[0].leading_trivia = &t1;

  cdd_cst_detect_format_config(&tree, &config);

  free(lst.tokens);
  g_fail_io_after = -1;
  PASS();
}

SUITE(cdd_cst_trivia_suite) {
  RUN_TEST(test_trivia_branches);
  RUN_TEST(test_cdd_cst_trivia_detect);
  RUN_TEST(test_cdd_cst_trivia_detect_tabs);
  RUN_TEST(test_cdd_cst_trivia_generate);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_cdd_cst_trivia_oom);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_TRIVIA_H */

/* LCOV_EXCL_STOP */
