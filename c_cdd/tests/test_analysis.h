#ifndef TEST_ANALYSIS_H
#define TEST_ANALYSIS_H

#include <greatest.h>
#include <string.h>

#include "analysis.h"
#include "tokenizer.h"

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
  ASSERT_EQ(1, sites.sites[0].used_before_check); /* Used immediately */
  ASSERT_STR_EQ("malloc", sites.sites[0].spec->name);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_asprintf_checked_inline(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  /* asprintf returns -1 on error. Checking < 0 is valid. */
  const char *code =
      "void f() { char *p; if (asprintf(&p, \"f\") < 0) return; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  if (find_allocations(tl, &sites) != 0) {
    free_token_list(tl);
    FAIL();
  }

  ASSERT_EQ(1, sites.size);
  /* Inline implicit check */
  ASSERT_EQ(1, sites.sites[0].is_checked);
  ASSERT_STR_EQ("asprintf", sites.sites[0].spec->name);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_asprintf_unchecked_inline(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  /* boolean check for asprintf is INVALID (returns char count on success) */
  const char *code = "void f() { char *p; if (asprintf(&p, \"f\")) return; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_EQ(
      0,
      sites.sites[0].is_checked); /* Should be identified as Unchecked logic */

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_asprintf_checked_var(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  /* Explicit variable check */
  const char *code = "void f() { char *p; int rc = asprintf(&p, \"f\"); if (rc "
                     "== -1) return; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("rc", sites.sites[0].var_name);
  ASSERT_EQ(1, sites.sites[0].is_checked);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_glob_nonzero_checked(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  /* glob returns non-zero on error. if(rc) is valid. */
  const char *code =
      "void f() { int r = glob(\"p\", 0, 0, &g); if (r) return; }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("r", sites.sites[0].var_name);
  ASSERT_EQ(1, sites.sites[0].is_checked);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_return_alloc(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "char* f() { return strdup(\"foo\"); }";

  if (tokenize(az_span_create_from_str((char *)code), &tl) != 0)
    FAIL();
  if (find_allocations(tl, &sites) != 0) {
    free_token_list(tl);
    FAIL();
  }

  ASSERT_EQ(1, sites.size);
  ASSERT_EQ(NULL, sites.sites[0].var_name);
  ASSERT_EQ(1, sites.sites[0].is_return_stmt);
  ASSERT_EQ(0, sites.sites[0].is_checked);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

SUITE(analysis_suite) {
  RUN_TEST(test_find_simple_unchecked_malloc);
  RUN_TEST(test_find_asprintf_checked_inline);
  RUN_TEST(test_find_asprintf_unchecked_inline);
  RUN_TEST(test_find_asprintf_checked_var);
  RUN_TEST(test_glob_nonzero_checked);
  RUN_TEST(test_find_return_alloc);
}

#endif /* TEST_ANALYSIS_H */
