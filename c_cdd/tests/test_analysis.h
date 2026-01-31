#ifndef TEST_ANALYSIS_H
#define TEST_ANALYSIS_H

#include <greatest.h>
#include <string.h>

#include "analysis.h"
#include "tokenizer.h"

TEST test_allocation_list_lifecycle(void) {
  struct AllocationSiteList list = {0};
  ASSERT_EQ(0, allocation_site_list_init(&list));
  ASSERT_EQ(0, list.size);
  ASSERT_NEQ(NULL, list.sites);
  allocation_site_list_free(&list);
  ASSERT_EQ(NULL, list.sites);
  PASS();
}

TEST test_find_simple_unchecked_malloc(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "void f() { char *p = malloc(10); *p = 5; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  if (find_allocations(tl, &sites) != 0) {
    free_token_list(tl);
    FAIL();
  }

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("p", sites.sites[0].var_name);
  ASSERT_EQ(0, sites.sites[0].is_checked);
  /* Verify spec mapping */
  ASSERT_STR_EQ("malloc", sites.sites[0].spec->name);
  ASSERT_EQ(ALLOC_STYLE_RETURN_PTR, sites.sites[0].spec->style);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_simple_checked_malloc(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code =
      "void f() { char *p = malloc(10); if (!p) return; *p = 5; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  if (find_allocations(tl, &sites) != 0) {
    free_token_list(tl);
    FAIL();
  }

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("p", sites.sites[0].var_name);
  ASSERT_EQ(1, sites.sites[0].is_checked); /* Checked is 1 */

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_malloc_in_if_condition(void) {
  /* `if ((p = malloc(10)) == NULL) ...`
     Current logic scans for `malloc`. is_checked detects it is inside a
     condition.
  */
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "void f() { if ((p = malloc(10)) == NULL) return; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);
  ASSERT_GTE(sites.size, 1);

  ASSERT_STR_EQ("p", sites.sites[0].var_name);
  ASSERT_EQ(1, sites.sites[0].is_checked); /* Should be checked now */

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_unchecked_usage_before_check(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code =
      "void f() { char *p = malloc(1); *p = 'a'; if (p) free(p); }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("p", sites.sites[0].var_name);
  ASSERT_EQ(0, sites.sites[0].is_checked); /* Used (`*p`) before check */

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_realloc_calloc_strdup(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "void f() { "
                     "a = realloc(a, 2); "
                     "b = calloc(1, 1); "
                     "c = strdup(s); "
                     "}";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);

  ASSERT_EQ(3, sites.size);
  ASSERT_STR_EQ("a", sites.sites[0].var_name);
  ASSERT_STR_EQ("b", sites.sites[1].var_name);
  ASSERT_STR_EQ("c", sites.sites[2].var_name);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_arg_ptr_asprintf_unchecked(void) {
  /* Test asprintf where pointer is passed as argument */
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "void f() { char *s; asprintf(&s, \"fmt\"); *s = 0; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("s", sites.sites[0].var_name);
  ASSERT_STR_EQ("asprintf", sites.sites[0].spec->name);
  ASSERT_EQ(ALLOC_STYLE_ARG_PTR, sites.sites[0].spec->style);
  ASSERT_EQ(0, sites.sites[0].is_checked); /* Unchecked */

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_arg_ptr_asprintf_checked(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  /* Check usage: if (!s) */
  const char *code =
      "void f() { char *s; asprintf(&s, \"fmt\"); if (!s) return; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("s", sites.sites[0].var_name);
  ASSERT_EQ(1, sites.sites[0].is_checked);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_arg_ptr_asprintf_checked_in_condition(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  /* Check usage: if (asprintf(...) < 0) */
  const char *code =
      "void f() { char *s; if (asprintf(&s, \"fmt\") < 0) return; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("s", sites.sites[0].var_name);
  ASSERT_EQ(
      1,
      sites.sites[0].is_checked); /* Should be checked as it is in condition */

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_arg_ptr_getline(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  /* getline arg index 0 */
  const char *code = "void f() { getline(&line, &n, f); }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("line", sites.sites[0].var_name);
  ASSERT_STR_EQ("getline", sites.sites[0].spec->name);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_allocation_no_match(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "void f() { int x = 5; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);
  ASSERT_EQ(0, sites.size);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

SUITE(analysis_suite) {
  RUN_TEST(test_allocation_list_lifecycle);
  RUN_TEST(test_find_simple_unchecked_malloc);
  RUN_TEST(test_find_simple_checked_malloc);
  RUN_TEST(test_find_malloc_in_if_condition);
  RUN_TEST(test_find_unchecked_usage_before_check);
  RUN_TEST(test_realloc_calloc_strdup);
  RUN_TEST(test_arg_ptr_asprintf_unchecked);
  RUN_TEST(test_arg_ptr_asprintf_checked);
  RUN_TEST(test_arg_ptr_asprintf_checked_in_condition);
  RUN_TEST(test_arg_ptr_getline);
  RUN_TEST(test_find_allocation_no_match);
}

#endif /* TEST_ANALYSIS_H */
