/**
 * @file test_strategy.h
 * @brief Unit tests for parsing strategy algorithms.
 */

#ifndef TEST_STRATEGY_H
#define TEST_STRATEGY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "functions/parse/str.h"
#include "functions/parse/strategy.h"
#include "functions/parse/tokenizer.h"
#include "functions/emit/patcher.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_fail_asprintf;
#endif

static size_t find_token_index(struct TokenList *tl, const char *str);

static const struct AllocatorSpec REALLOC_SPEC = {
    "realloc", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0};
static const struct AllocatorSpec MALLOC_SPEC = {
    "malloc", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0};
static const struct AllocatorSpec ASPRINTF_SPEC = {
    "asprintf", ALLOC_STYLE_ARG_PTR, CHECK_INT_NEGATIVE, 0};
static const struct AllocatorSpec MKDIR_SPEC = {
    "_mkdir", ALLOC_STYLE_RETURN_PTR, CHECK_INT_NONZERO, 0};
static const struct AllocatorSpec UNKNOWN_SPEC = {
    "unknown", ALLOC_STYLE_RETURN_PTR, 999, 0};

/**
 * @brief Tests error handling of the strategy injection function.
 *
 * @return The result of the test.
 */
TEST test_strategy_errors(void) {
  struct PatchList patches;
  struct AllocationSiteList allocs;
  struct TokenList *tl = NULL;

  memset(&patches, 0, sizeof(patches));
  memset(&allocs, 0, sizeof(allocs));
  allocs.size = 1;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            strategy_inject_safety_checks(NULL, &allocs, &patches));

  {
    struct TokenList *tl_dummy = NULL;
    tokenize(az_span_create_from_str("p = realloc(p, 10);"), &tl_dummy);

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              strategy_inject_safety_checks(tl_dummy, NULL, &patches));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              strategy_inject_safety_checks(tl_dummy, &allocs, NULL));

    free_token_list(tl_dummy);
  }

  /* Test strategy_rewrite_realloc errors */
  {
    struct AllocationSite site = {0};
    struct TokenList *tl_dummy = NULL;
    tokenize(az_span_create_from_str("p = realloc(p, 10);"), &tl_dummy);

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              strategy_rewrite_realloc(NULL, &site, 0, &patches));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              strategy_rewrite_realloc(tl_dummy, NULL, 0, &patches));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              strategy_rewrite_realloc(tl_dummy, &site, 0, NULL));

    /* missing var_name */
    site.var_name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(tl_dummy, &site, 0, &patches));

    /* out of bounds token index */
    site.var_name = "p";
    site.token_index = 9999;
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(tl_dummy, &site, 0, &patches));

    /* missing semicolon */
    free_token_list(tl_dummy);
    tokenize(az_span_create_from_str("p = realloc(p, 10)"), &tl_dummy);
    site.token_index = find_token_index(tl_dummy, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(tl_dummy, &site, 0, &patches));

    /* missing lparen */
    free_token_list(tl_dummy);
    tokenize(az_span_create_from_str("p = realloc"), &tl_dummy);
    site.token_index = find_token_index(tl_dummy, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(tl_dummy, &site, 0, &patches));

    /* missing args */
    free_token_list(tl_dummy);
    tokenize(az_span_create_from_str("p = realloc( "), &tl_dummy);
    site.token_index = find_token_index(tl_dummy, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(tl_dummy, &site, 0, &patches));

    /* non-identifier arg */
    free_token_list(tl_dummy);
    tokenize(az_span_create_from_str("p = realloc(10);"), &tl_dummy);
    site.token_index = find_token_index(tl_dummy, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(tl_dummy, &site, 0, &patches));

    /* missing equal sign */
    free_token_list(tl_dummy);
    tokenize(az_span_create_from_str("realloc(p, 10);"), &tl_dummy);
    site.token_index = find_token_index(tl_dummy, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(tl_dummy, &site, 0, &patches));

    /* non-matching var_name */
    free_token_list(tl_dummy);
    tokenize(az_span_create_from_str("p = realloc(q, 10);"), &tl_dummy);
    site.token_index = find_token_index(tl_dummy, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(tl_dummy, &site, 0, &patches));

    /* only whitespace before assignment */
    free_token_list(tl_dummy);
    tokenize(az_span_create_from_str(" = realloc(p, 10);"), &tl_dummy);
    site.token_index = find_token_index(tl_dummy, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(tl_dummy, &site, 0, &patches));

    /* missing semicolon in inject_safety_checks */
    free_token_list(tl_dummy);
    tokenize(az_span_create_from_str("p = malloc(10)"), &tl_dummy);
    site.token_index = find_token_index(tl_dummy, "malloc");
    site.spec = &MALLOC_SPEC;
    site.var_name = "p";
    {
      struct AllocationSiteList single_alloc;
      memset(&single_alloc, 0, sizeof(single_alloc));
      single_alloc.size = 1;
      single_alloc.sites = &site;
      ASSERT_EQ(CDD_C_SUCCESS, strategy_inject_safety_checks(
                                   tl_dummy, &single_alloc, &patches));
    }

    free_token_list(tl_dummy);
  }

  g_fail_io_after = -1;
  free_token_list(tl);
  PASS();
}

static size_t find_token_index(struct TokenList *tl, const char *str);

static size_t find_token_index(struct TokenList *tl, const char *str) {
  size_t i;
  for (i = 0; i < tl->size; i++) {
    if (tl->tokens[i].length == strlen(str) &&
        strncmp((const char *)tl->tokens[i].start, str, strlen(str)) == 0) {
      return i;
    }
  }
  return 0;
}

TEST test_strategy_injection(void) {
  struct PatchList patches;
  struct AllocationSiteList allocs;
  struct TokenList *tl = NULL;

  memset(&patches, 0, sizeof(patches));
  memset(&allocs, 0, sizeof(allocs));

  patch_list_init(&patches);
  tokenize(az_span_create_from_str(
               "p = malloc(10); p = realloc(p, 20); asprintf(&p, \"\"); "
               "_mkdir(\"dir\"); p = unknown(1); p = realloc(q, 10);"),
           &tl);

  allocs.sites = calloc(6, sizeof(struct AllocationSite));
  allocs.capacity = 6;
  allocs.size = 6;

  /* malloc site */
  allocs.sites[0].token_index = find_token_index(tl, "malloc");
  allocs.sites[0].var_name = "p";
  allocs.sites[0].spec = &MALLOC_SPEC;

  /* realloc site */
  allocs.sites[1].token_index = find_token_index(tl, "realloc");
  allocs.sites[1].var_name = "p";
  allocs.sites[1].spec = &REALLOC_SPEC;

  /* asprintf site */
  allocs.sites[2].token_index = find_token_index(tl, "asprintf");
  allocs.sites[2].var_name = "rc";
  allocs.sites[2].spec = &ASPRINTF_SPEC;

  /* _mkdir site */
  allocs.sites[3].token_index = find_token_index(tl, "_mkdir");
  allocs.sites[3].var_name = "rc";
  allocs.sites[3].spec = &MKDIR_SPEC;

  /* unknown site */
  allocs.sites[4].token_index = find_token_index(tl, "unknown");
  allocs.sites[4].var_name = "p";
  allocs.sites[4].spec = &UNKNOWN_SPEC;

  /* realloc site (not self assignment) */
  {
    size_t i;
    for (i = tl->size - 1; i > 0; i--) {
      if (tl->tokens[i].length == 7 &&
          strncmp((const char *)tl->tokens[i].start, "realloc", 7) == 0) {
        allocs.sites[5].token_index = i;
        break;
      }
    }
  }
  allocs.sites[5].var_name = "p";
  allocs.sites[5].spec = &REALLOC_SPEC;

  ASSERT_EQ(CDD_C_SUCCESS,
            strategy_inject_safety_checks(tl, &allocs, &patches));
  ASSERT_EQ(5, patches.size);

  patch_list_free(&patches);
  free(allocs.sites);
  free_token_list(tl);
  PASS();
}

TEST test_strategy_injection_ooms(void) {
#ifdef CDD_BUILD_TESTS
  struct PatchList patches;
  struct AllocationSiteList allocs;
  struct TokenList *tl = NULL;
  int i;

  memset(&patches, 0, sizeof(patches));
  memset(&allocs, 0, sizeof(allocs));

  patch_list_init(&patches);
  tokenize(az_span_create_from_str("p = malloc(10); p = realloc(p, 20); "
                                   "asprintf(&p, \"\"); _mkdir(\"dir\");"),
           &tl);

  allocs.sites = calloc(4, sizeof(struct AllocationSite));
  allocs.capacity = 4;
  allocs.size = 4;

  /* malloc site */
  allocs.sites[0].token_index = find_token_index(tl, "malloc");
  allocs.sites[0].var_name = "p";
  allocs.sites[0].spec = &MALLOC_SPEC;

  /* realloc site */
  allocs.sites[1].token_index = find_token_index(tl, "realloc");
  allocs.sites[1].var_name = "p";
  allocs.sites[1].spec = &REALLOC_SPEC;

  /* asprintf site */
  allocs.sites[2].token_index = find_token_index(tl, "asprintf");
  allocs.sites[2].var_name = "rc";
  allocs.sites[2].spec = &ASPRINTF_SPEC;

  /* _mkdir site */
  allocs.sites[3].token_index = find_token_index(tl, "_mkdir");
  allocs.sites[3].var_name = "rc";
  allocs.sites[3].spec = &MKDIR_SPEC;

  /* Test malloc OOMs */
  for (i = 1; i <= 3; i++) {
    g_cdd_alloc_fail = i;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              strategy_inject_safety_checks(tl, &allocs, &patches));
    g_cdd_alloc_fail = 0;
  }

  /* Test range_to_string OOM (allocs inside strategy_rewrite_realloc) */
  {
    struct AllocationSite site = allocs.sites[1];
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(
        CDD_C_ERROR_MEMORY,
        strategy_rewrite_realloc(tl, &site, site.token_index + 7, &patches));
    g_cdd_alloc_fail = 0;
  }

  /* Test CHECK_INT_NONZERO and CHECK_INT_NEGATIVE OOMs */
  {
    struct AllocationSiteList single_alloc;
    memset(&single_alloc, 0, sizeof(single_alloc));
    single_alloc.size = 1;
    single_alloc.sites = &allocs.sites[2]; /* asprintf */

    for (i = 1; i <= 2; i++) {
      patch_list_free(&patches);
      memset(&patches, 0, sizeof(patches));
      g_cdd_alloc_fail = i;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                strategy_inject_safety_checks(tl, &single_alloc, &patches));
      g_cdd_alloc_fail = 0;
    }

    single_alloc.sites = &allocs.sites[3]; /* _mkdir */
    for (i = 1; i <= 2; i++) {
      patch_list_free(&patches);
      memset(&patches, 0, sizeof(patches));
      g_cdd_alloc_fail = i;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                strategy_inject_safety_checks(tl, &single_alloc, &patches));
      g_cdd_alloc_fail = 0;
    }
  }

  /* Test asprintf failure (assuming HAVE_ASPRINTF uses standard allocs or
   * specialized mocks) */
  g_cdd_fail_asprintf = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, strategy_rewrite_realloc(
                                    tl, &allocs.sites[1],
                                    allocs.sites[1].token_index + 7, &patches));
  g_cdd_fail_asprintf = 0;

  patch_list_free(&patches);
  free(allocs.sites);
  free_token_list(tl);
#endif
  PASS();
}

TEST test_strategy_edge_cases(void) {
  struct PatchList patches;
  struct AllocationSiteList allocs;
  struct TokenList *tl = NULL;

  memset(&patches, 0, sizeof(patches));
  memset(&allocs, 0, sizeof(allocs));

  patch_list_init(&patches);
  tokenize(az_span_create_from_str(
               "p = malloc(10) \n p = realloc(p, 20) \n = realloc(p, 20);"),
           &tl);

  allocs.sites = calloc(3, sizeof(struct AllocationSite));
  allocs.capacity = 3;
  allocs.size = 3;

  /* Missing semi colon malloc */
  allocs.sites[0].token_index = 2;
  allocs.sites[0].var_name = "p";
  allocs.sites[0].spec = &MALLOC_SPEC;

  /* Missing semi colon realloc */
  allocs.sites[1].token_index = 8;
  allocs.sites[1].var_name = "p";
  allocs.sites[1].spec = &REALLOC_SPEC;

  /* Realloc without assignment */
  allocs.sites[2].token_index = 16;
  allocs.sites[2].var_name = "p";
  allocs.sites[2].spec = &REALLOC_SPEC;

  ASSERT_EQ(CDD_C_SUCCESS,
            strategy_inject_safety_checks(tl, &allocs, &patches));

  /* Test range_to_string with start >= end */
  {
    char *out = NULL;
    struct AllocationSite site = allocs.sites[1];
    site.var_name = "p";
    site.spec = &REALLOC_SPEC;
    site.token_index = 16;
    ASSERT_EQ(CDD_C_SUCCESS,
              strategy_rewrite_realloc(
                  tl, &site, 14, &patches)); /* Not self-assign, early return */
  }

  /* Various boundary conditions for strategy_rewrite_realloc backward search */
  {
    struct AllocationSite site = {0};
    site.var_name = "p";
    site.spec = &REALLOC_SPEC;

    /* At index 0 */
    site.token_index = 0;
    ASSERT_EQ(CDD_C_SUCCESS, strategy_rewrite_realloc(tl, &site, 0, &patches));

    /* Hit SEMICOLON */
    tokenize(az_span_create_from_str("; realloc(p, 10);"), &tl);
    site.token_index = find_token_index(tl, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS, strategy_rewrite_realloc(
                                 tl, &site, site.token_index + 5, &patches));

    /* Hit LBRACE */
    tokenize(az_span_create_from_str("{ realloc(p, 10);"), &tl);
    site.token_index = find_token_index(tl, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS, strategy_rewrite_realloc(
                                 tl, &site, site.token_index + 5, &patches));

    /* Hit RBRACE */
    tokenize(az_span_create_from_str("} realloc(p, 10);"), &tl);
    site.token_index = find_token_index(tl, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS, strategy_rewrite_realloc(
                                 tl, &site, site.token_index + 5, &patches));

    /* LBRACE backward scan 2 */
    tokenize(az_span_create_from_str("{ p = realloc(p, 10);"), &tl);
    site.token_index = find_token_index(tl, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS, strategy_rewrite_realloc(
                                 tl, &site, site.token_index + 7, &patches));

    /* RBRACE backward scan 2 */
    tokenize(az_span_create_from_str("} p = realloc(p, 10);"), &tl);
    site.token_index = find_token_index(tl, "realloc");
    ASSERT_EQ(CDD_C_SUCCESS, strategy_rewrite_realloc(
                                 tl, &site, site.token_index + 7, &patches));
  }

  patch_list_free(&patches);
  free(allocs.sites);
  free_token_list(tl);
  PASS();
}

/**
 * @brief Strategy test suite.
 */
SUITE(strategy_suite) {
  RUN_TEST(test_strategy_errors);
  RUN_TEST(test_strategy_injection);
  RUN_TEST(test_strategy_edge_cases);
  RUN_TEST(test_strategy_injection_ooms);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_STRATEGY_H */
