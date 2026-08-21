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
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/analysis.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

static cdd_c_error_t find_allocs(const char *code,
                                 struct AllocationSiteList *sites) {
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

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, find_allocations(NULL, &sites));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, find_allocations(&tl, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, allocation_site_list_init(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            allocation_site_list_add(NULL, 0, NULL, 0, 0, 0, NULL));

  allocation_site_list_free(NULL); /* Should not crash */
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief analysis_suite
 */

#ifdef CDD_BUILD_TESTS
extern int g_cdd_analysis_fail_alloc_init;
extern int g_cdd_analysis_fail_alloc_add;
extern int g_cdd_analysis_fail_alloc_varname;
#endif

TEST test_analysis_oom(void) {
#ifdef CDD_BUILD_TESTS
  struct AllocationSiteList sites = {0};
  int rc;

  /* Test init failure */
  g_cdd_analysis_fail_alloc_init = 1;
  rc = find_allocs("void *p = malloc(10);", &sites);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  allocation_site_list_free(&sites);
  g_cdd_analysis_fail_alloc_init = 0;

  /* Cover > 1 init branch */
  g_cdd_analysis_fail_alloc_init = 2;
  rc = find_allocs("void *p = malloc(10);", &sites);
  ASSERT_EQ(0, rc);
  allocation_site_list_free(&sites);
  g_cdd_analysis_fail_alloc_init = 0;

  /* Test varname alloc failure */
  g_cdd_analysis_fail_alloc_varname = 1;
  rc = find_allocs("void *p = malloc(10);", &sites);
  printf("test_analysis_oom rc = %d\n", rc);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  allocation_site_list_free(&sites);
  g_cdd_analysis_fail_alloc_varname = 0;

  /* Cover > 1 varname branch */
  g_cdd_analysis_fail_alloc_varname = 2;
  rc = find_allocs("void *p = malloc(10); void *q = malloc(10);", &sites);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  allocation_site_list_free(&sites);
  g_cdd_analysis_fail_alloc_varname = 0;

  /* Test add failure when varname is NULL */
  {
    struct AllocationSiteList s = {0};
    find_allocs("malloc(1);malloc(2);malloc(3);malloc(4);malloc(5);malloc(6);"
                "malloc(7);malloc(8);",
                &s);
    g_cdd_analysis_fail_alloc_add = 1;
    rc = find_allocs("malloc(10);", &s);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    allocation_site_list_free(&s);
    g_cdd_analysis_fail_alloc_add = 0;
  }

  /* Test add failure when is_return is true */
  {
    struct AllocationSiteList s = {0};
    find_allocs("malloc(1);malloc(2);malloc(3);malloc(4);malloc(5);malloc(6);"
                "malloc(7);malloc(8);",
                &s);
    g_cdd_analysis_fail_alloc_add = 1;
    rc = find_allocs("return malloc(10);", &s);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    allocation_site_list_free(&s);
    g_cdd_analysis_fail_alloc_add = 0;
  }
#endif

  PASS();
}

TEST test_analysis_capacity(void) {
#ifdef CDD_BUILD_TESTS
  struct AllocationSiteList sites = {0};
  int rc;
  int i;

  /* Create an initial list with capacity 0 to trigger new_cap = 8 branch */
  sites.capacity = 0;
  sites.size = 0;
  sites.sites = NULL;

  rc = allocation_site_list_add(&sites, 0, "test", 0, 0, 0, NULL);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(8, sites.capacity);
  ASSERT_EQ(1, sites.size);

  /* Trigger realloc failure (need to fill the list first to trigger capacity
   * resize) */
  for (i = 0; i < 7; i++) {
    rc = allocation_site_list_add(&sites, 0, "test", 0, 0, 0, NULL);
    ASSERT_EQ(0, rc);
  }
  /* Now size == 8, capacity == 8 */
  g_cdd_analysis_fail_alloc_add = 1;
  rc = allocation_site_list_add(&sites, 0, "test", 0, 0, 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_analysis_fail_alloc_add = 0;

  /* Cover > 1 add failure branch */
  g_cdd_analysis_fail_alloc_add = 2;
  rc = allocation_site_list_add(&sites, 0, "test", 0, 0, 0, NULL);
  ASSERT_EQ(0, rc); /* First succeeds and capacity resizes to 16 */
  /* Fill up to 16 */
  for (i = 0; i < 7; i++) {
    rc = allocation_site_list_add(&sites, 0, "test", 0, 0, 0, NULL);
    ASSERT_EQ(0, rc);
  }
  rc = allocation_site_list_add(&sites, 0, "test", 0, 0, 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc); /* Second capacity resize fails */
  g_cdd_analysis_fail_alloc_add = 0;

  allocation_site_list_free(&sites);
#endif

  PASS();
}

TEST test_analysis_edge_cases(void) {
  struct AllocationSiteList sites = {0};

  /* Assign at index 0 */
  ASSERT_EQ(0, find_allocs("= malloc(10);", &sites));
  /* It might find an empty var name instead of failing */
  if (sites.size > 0) {
    ASSERT(sites.sites[0].var_name == NULL ||
           sites.sites[0].var_name[0] == '\0');
  }
  allocation_site_list_free(&sites);

  /* Bounds tests for is_checked */
  {
    struct TokenList tl = {0};
    struct AllocatorSpec spec = {0};
    int used = 0;
    int checked = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              is_checked(&tl, 0, "p", &spec, &used, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, is_checked(NULL, 0, "p", &spec, &used, &checked));
    ASSERT_EQ(CDD_C_SUCCESS, is_checked(&tl, 0, NULL, &spec, &used, &checked));
    ASSERT_EQ(CDD_C_SUCCESS, is_checked(&tl, 0, "p", &spec, NULL, &checked));
  }

  /* Not an identifier before assign */
  ASSERT_EQ(0, find_allocs("1 = malloc(10);", &sites));
  if (sites.size > 0) {
    ASSERT(sites.sites[0].var_name == NULL ||
           sites.sites[0].var_name[0] == '\0');
  }
  allocation_site_list_free(&sites);

  /* Dereference check -> */
  ASSERT_EQ(0, find_allocs("void *p = malloc(10); p->x = 1;", &sites));
  ASSERT_EQ(1, sites.size);
  ASSERT_EQ(1, sites.sites[0].used_before_check);
  allocation_site_list_free(&sites);

  /* Dereference check [ */
  ASSERT_EQ(0, find_allocs("void *p = malloc(10); p[0] = 1;", &sites));
  ASSERT_EQ(1, sites.size);
  ASSERT_EQ(1, sites.sites[0].used_before_check);
  allocation_site_list_free(&sites);

  /* Inline check inside condition */
  ASSERT_EQ(0, find_allocs("if ((p = malloc(10)) != NULL) { }", &sites));
  ASSERT_EQ(1, sites.size);
  ASSERT_EQ(1, sites.sites[0].is_checked);
  allocation_site_list_free(&sites);

#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_fail_alloc;
    g_cdd_fail_alloc = 1;
    ASSERT_EQ(0, find_allocs("void *p = malloc(10);", &sites));
    if (sites.size > 0) {
      ASSERT(sites.sites[0].var_name == NULL);
    }
    allocation_site_list_free(&sites);

    g_cdd_fail_alloc = 2;
    ASSERT_EQ(
        0, find_allocs("void *p = malloc(10); void *q = malloc(10);", &sites));
    if (sites.size > 1) {
      ASSERT(sites.sites[1].var_name == NULL);
    }
    allocation_site_list_free(&sites);
    g_cdd_fail_alloc = 0;
  }
#endif

  g_fail_io_after = -1;

  /* Test non-ptr check styles (asprintf, _mkdir) */
  ASSERT_EQ(0,
            find_allocs("int rc = asprintf(&p, \"\"); if (rc < 0) {}", &sites));
  allocation_site_list_free(&sites);
  ASSERT_EQ(0,
            find_allocs("int rc = _mkdir(\"dir\"); if (rc != 0) {}", &sites));
  allocation_site_list_free(&sites);

  /* Usage outside condition */
  ASSERT_EQ(0, find_allocs("int rc = asprintf(&p, \"\"); int x = rc;", &sites));
  allocation_site_list_free(&sites);

  /* find_allocs with already initialized sites */
  ASSERT_EQ(0, find_allocs("malloc(1);", &sites));
  ASSERT_EQ(0, find_allocs("malloc(2);", &sites));
  allocation_site_list_free(&sites);

  /* Start file with parentheses to hit prev > 0 in is_inside_condition */
  ASSERT_EQ(0, find_allocs("(p = malloc(10))", &sites));
  allocation_site_list_free(&sites);
  ASSERT_EQ(0, find_allocs(" (p = malloc(10))", &sites));
  allocation_site_list_free(&sites);

  /* Test dereference use (arrow and bracket) */
  ASSERT_EQ(0, find_allocs("p = malloc(10); p->a = 1;", &sites));
  allocation_site_list_free(&sites);
  ASSERT_EQ(0, find_allocs("p = malloc(10); p[0] = 1;", &sites));
  allocation_site_list_free(&sites);

  /* Test used before check */
  ASSERT_EQ(0, find_allocs("p = malloc(10); p[0] = 1; if (p) {}", &sites));
  if (sites.size > 0) {
    ASSERT_EQ(1, sites.sites[0].used_before_check);
  }
  allocation_site_list_free(&sites);

  /* Test usage at EOF */
  ASSERT_EQ(0, find_allocs("p = malloc(10); p", &sites));
  allocation_site_list_free(&sites);

  /* Test condition tracking keywords (if, while) inside parens */
  ASSERT_EQ(0, find_allocs("if ( (p = malloc(10)) ) {}", &sites));
  allocation_site_list_free(&sites);
  ASSERT_EQ(0, find_allocs("while ( (p = malloc(10)) ) {}", &sites));
  allocation_site_list_free(&sites);

  /* Test cast before alloc */
  ASSERT_EQ(0, find_allocs("p = (void*)malloc(10);", &sites));
  allocation_site_list_free(&sites);

  /* Test semicolon, brace bounds in find_allocations */
  ASSERT_EQ(0, find_allocs("{ p = malloc(10); }", &sites));
  allocation_site_list_free(&sites);
  ASSERT_EQ(0, find_allocs("} p = malloc(10);", &sites));
  allocation_site_list_free(&sites);

  /* Semicolon and RBRACE parsing in is_checked bounds */
  ASSERT_EQ(0, find_allocs("p = malloc(10); struct p {", &sites));
  allocation_site_list_free(&sites);

  /* Identifier used but not dereferenced */
  ASSERT_EQ(0, find_allocs("p = malloc(10); p = 2;", &sites));
  allocation_site_list_free(&sites);

  /* Call is_checked directly to hit used_before_check = NULL branch */
  {
    struct TokenList *tl = NULL;
    struct AllocatorSpec spec = {"malloc", ALLOC_STYLE_RETURN_PTR,
                                 CHECK_PTR_NULL, 0};
    int checked = 0;
    tokenize(az_span_create_from_str("p = malloc(10); p[0] = 1;"), &tl);
    is_checked(tl, 2, "p", &spec, NULL, &checked);
    free_token_list(tl);
  }

  /* Semicolon assignment backward search limit */
  ASSERT_EQ(0, find_allocs("; p = malloc(10);", &sites));
  allocation_site_list_free(&sites);
  ASSERT_EQ(0, find_allocs("} p = malloc(10);", &sites));
  allocation_site_list_free(&sites);
  ASSERT_EQ(0, find_allocs("{ p = malloc(10);", &sites));
  allocation_site_list_free(&sites);

  PASS();
}

SUITE(analysis_suite) {
  RUN_TEST(test_analysis_find_malloc);
  RUN_TEST(test_analysis_find_calloc);
  RUN_TEST(test_analysis_find_realloc);
  RUN_TEST(test_analysis_find_none);
  RUN_TEST(test_analysis_bounds);
  RUN_TEST(test_analysis_oom);
  RUN_TEST(test_analysis_capacity);
  RUN_TEST(test_analysis_edge_cases);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_ANALYSIS_H */
