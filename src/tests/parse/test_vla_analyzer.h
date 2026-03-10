/**
 * @file test_vla_analyzer.h
 * @brief Unit tests for VLA analyzer.
 */

#ifndef TEST_VLA_ANALYZER_H
#define TEST_VLA_ANALYZER_H

#include <greatest.h>
#include <string.h>

#include "functions/parse/tokenizer.h"
#include "functions/parse/vla_analyzer.h"

TEST test_scan_for_vlas_basic(void) {
  struct TokenList *tokens = NULL;
  struct VLASiteList list;
  const char *src = ""
                    "void func(int n) {
      int normal[10];
  char vla[n];
  struct S s_vla[n * 2];
}
";

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

SUITE(vla_analyzer_suite) { RUN_TEST(test_scan_for_vlas_basic); }

#endif /* TEST_VLA_ANALYZER_H */
