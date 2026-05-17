/**
 * @file test_vla_analyzer.h
 * @brief Unit tests for VLA analyzer.
 */

#ifndef TEST_VLA_ANALYZER_H
#define TEST_VLA_ANALYZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>

#include "functions/parse/tokenizer.h"
#include "functions/parse/vla_analyzer.h"
/* clang-format on */

TEST test_scan_for_vlas_basic(void) {
  struct TokenList *tokens = NULL;
  struct VLASiteList list;
  const char *src = "void func(int n) {\n"
                    "  int normal[10];\n"
                    "  char vla[n];\n"
                    "  struct S s_vla[n * 2];\n"
                    "}\n";

  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src), &tokens));
  vla_site_list_init(&list);

  ASSERT_EQ(0, scan_for_vlas(tokens, &list));

  ASSERT_EQ(2, list.count);

  ASSERT_STR_EQ("char", list.sites[0].type_str);
  ASSERT_STR_EQ("vla", list.sites[0].var_name);
  ASSERT_STR_EQ("n", list.sites[0].size_expr);

  ASSERT_STR_EQ("struct S", list.sites[1].type_str);
  ASSERT_STR_EQ("s_vla", list.sites[1].var_name);
  ASSERT_STR_EQ("n * 2", list.sites[1].size_expr);

  vla_site_list_free(&list);
  free_token_list(tokens);
  PASS();
}

TEST test_scan_for_vlas_errors(void) {
  struct TokenList *tl = NULL;
  struct VLASiteList list;
  vla_site_list_init(&list);
  ASSERT_EQ(0, tokenize(az_span_create_from_str("int x[n];"), &tl));

  ASSERT_EQ(EINVAL, scan_for_vlas(NULL, &list));
  ASSERT_EQ(EINVAL, scan_for_vlas(tl, NULL));

  free_token_list(tl);
  PASS();
}

SUITE(vla_analyzer_suite) {
  RUN_TEST(test_scan_for_vlas_basic);
  RUN_TEST(test_scan_for_vlas_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_VLA_ANALYZER_H */
