/**
 * @file test_analysis.h
 * @brief Unit tests for the Analysis Engine.
 *
 * Verifies identifying unchecked mallocs, checked mallocs, return statements,
 * and unsafe usage like dereferencing before check.
 *
 * @author Samuel Marks
 */

#ifndef TEST_ANALYSIS_H
#define TEST_ANALYSIS_H

#include <greatest.h>
#include <string.h>

#include "functions/parse/analysis.h"
#include "functions/parse/tokenizer.h"

#include <sys/errno.h>

static struct TokenList *setup_analysis_tokens(const char *code) {
  struct TokenList *tl = NULL;
  (void)tokenize(az_span_create_from_str((char *)code), &tl);
  return tl;
}

TEST test_find_simple_unchecked_malloc(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "void f() { char *p = malloc(10); *p = 5; }";

  tl = setup_analysis_tokens(code);
  ASSERT(tl);

  if (find_allocations(tl, &sites) != 0) {
    free_token_list(tl);
    FAILm("find_allocations failed");
  }

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("p", sites.sites[0].var_name);
  ASSERT_EQ(0, sites.sites[0].is_checked);
  ASSERT_EQ(1,
            sites.sites[0].used_before_check); /* Detected '*p' immediate use */
  ASSERT_STR_EQ("malloc", sites.sites[0].spec->name);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_simple_checked_malloc(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "void f() { char *p = malloc(10); if (!p) return; }";

  tl = setup_analysis_tokens(code);
  ASSERT(tl);

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_STR_EQ("p", sites.sites[0].var_name);
  ASSERT_EQ(1, sites.sites[0].is_checked);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_alloc_inside_condition(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  /* 'if (p = malloc(10))' is considered checked logic */
  const char *code = "void f() { char *p; if (p = malloc(10)) { } }";

  tl = setup_analysis_tokens(code);
  ASSERT(tl);

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_EQ(1, sites.sites[0].is_checked);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_find_return_alloc(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "char* f() { return strdup(\"foo\"); }";

  tl = setup_analysis_tokens(code);
  ASSERT(tl);

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_EQ(NULL, sites.sites[0].var_name);
  ASSERT_EQ(1, sites.sites[0].is_return_stmt);
  ASSERT_EQ(0, sites.sites[0].is_checked);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_asprintf_unchecked(void) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const char *code = "void f() { char *s; asprintf(&s, \"fmt\"); }";

  tl = setup_analysis_tokens(code);
  ASSERT(tl);

  find_allocations(tl, &sites);

  ASSERT_EQ(1, sites.size);
  ASSERT_EQ(0, sites.sites[0].is_checked);
  /* asprintf check style is negative int, var name assignment search usually
     fails without 'rc = ', so var_name might be NULL unless we look at
     arguments logic. Current `find_allocations` looks for LHS assignment. Here
     there is none. */
  ASSERT_EQ(NULL, sites.sites[0].var_name);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  PASS();
}

TEST test_init_free_safety(void) {
  struct AllocationSiteList sites;
  ASSERT_EQ(0, allocation_site_list_init(&sites));
  ASSERT_EQ(0, sites.size);
  allocation_site_list_free(&sites);
  /* Double free safety */
  allocation_site_list_free(&sites);
  allocation_site_list_free(NULL);

  /* NULL init safety */
  ASSERT_EQ(EINVAL, allocation_site_list_init(NULL));

  PASS();
}

SUITE(analysis_suite) {
  RUN_TEST(test_find_simple_unchecked_malloc);
  RUN_TEST(test_find_simple_checked_malloc);
  RUN_TEST(test_alloc_inside_condition);
  RUN_TEST(test_find_return_alloc);
  RUN_TEST(test_asprintf_unchecked);
  RUN_TEST(test_init_free_safety);
}

#endif /* TEST_ANALYSIS_H */
