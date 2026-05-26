extern int g_fail_io_after;
extern int g_io_calls;
/**
 * @file test_analysis.h
 * @brief Unit tests for code analysis features like allocation discovery.
 */

#ifndef TEST_ANALYSIS_H
#define TEST_ANALYSIS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/analysis.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

static int find_allocs(const char *code, struct AllocationSiteList *sites) {
  struct TokenList *tl = NULL;
  int rc;
  const az_span source = az_span_create_from_str((char *)code);

  if (tokenize(source, &tl) != 0)
    return -1;

  rc = find_allocations(tl, sites);
  free_token_list(tl);
  return rc;
}

/**
 * @brief test_analysis_find_malloc
 * @return TEST
 */
TEST test_analysis_find_malloc(void) {
  const char *code = "void *p = malloc(10);";
  struct AllocationSiteList sites = {0};
  int rc;

  rc = find_allocs(code, &sites);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, sites.size);
  ASSERT(strcmp(sites.sites[0].spec->name, "malloc") == 0);

  allocation_site_list_free(&sites);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_analysis_find_calloc
 * @return TEST
 */
TEST test_analysis_find_calloc(void) {
  const char *code = "void *p = calloc(1, 10);";
  struct AllocationSiteList sites = {0};
  int rc;

  rc = find_allocs(code, &sites);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, sites.size);
  ASSERT(strcmp(sites.sites[0].spec->name, "calloc") == 0);

  allocation_site_list_free(&sites);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_analysis_find_realloc
 * @return TEST
 */
TEST test_analysis_find_realloc(void) {
  const char *code = "void *p = realloc(old_p, 20);";
  struct AllocationSiteList sites = {0};
  int rc;

  rc = find_allocs(code, &sites);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, sites.size);
  ASSERT(strcmp(sites.sites[0].spec->name, "realloc") == 0);

  allocation_site_list_free(&sites);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_analysis_find_none
 * @return TEST
 */
TEST test_analysis_find_none(void) {
  const char *code = "int a = 1;";
  struct AllocationSiteList sites = {0};
  int rc;

  rc = find_allocs(code, &sites);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, sites.size);

  allocation_site_list_free(&sites);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_analysis_bounds
 * @return TEST
 */
TEST test_analysis_bounds(void) {
  struct AllocationSiteList sites = {0};
  struct TokenList tl = {0};

  ASSERT_EQ(EINVAL, find_allocations(NULL, &sites));
  ASSERT_EQ(EINVAL, find_allocations(&tl, NULL));

  allocation_site_list_free(NULL); /* Should not crash */
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief analysis_suite
 */
SUITE(analysis_suite) {
  RUN_TEST(test_analysis_find_malloc);
  RUN_TEST(test_analysis_find_calloc);
  RUN_TEST(test_analysis_find_realloc);
  RUN_TEST(test_analysis_find_none);
  RUN_TEST(test_analysis_bounds);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_ANALYSIS_H */
