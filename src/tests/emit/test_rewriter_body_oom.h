/**
 * @file test_rewriter_body_oom.h
 * @brief Unit tests for rewriter body out-of-memory handling.
 */

#ifndef TEST_REWRITER_BODY_OOM_H
#define TEST_REWRITER_BODY_OOM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/rewriter_body.h"
#include "functions/parse/analysis.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT int g_patcher_test_cap_1;

#ifndef CDD_RUN_BODY_REWRITE_DEFINED
#define CDD_RUN_BODY_REWRITE_DEFINED
static cdd_c_error_t
run_body_rewrite(const char *code, const struct RefactoredFunction *funcs,
                 size_t n_funcs, const struct SignatureTransform *transform,
                 char **out) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  int rc = 0;
  const az_span source =
      az_span_create_from_str((char *)(size_t)(size_t)(size_t)code);

  if (!code || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = tokenize(source, &tl);
  rc = find_allocations(tl, &sites);

  rc = rewrite_body(tl, &sites, funcs, n_funcs, transform, out);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  return rc;
}
#endif /* CDD_RUN_BODY_REWRITE_DEFINED */

TEST test_rewrite_body_funcs_oom(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 50; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source = az_span_create_from_str(
        (char *)(size_t)(size_t) "void f() { char *s; s = my_strdup(\"a\"); "
                                 "char *s2 = "
                                 "my_strdup(\"b\"); outer(inner(\"x\")); }");
    struct RefactoredFunction funcs2[] = {
        {"my_strdup", REF_PTR_TO_INT_OUT, "char *"},
        {"inner", REF_PTR_TO_INT_OUT, "char *"}};

    tokenize(source, &tl);
    find_allocations(tl, &sites);

    /*  (moved to global) */
    g_cdd_alloc_fail = i;
    {
      int rc2 = rewrite_body(tl, &sites, funcs2, 2, NULL, &out_code);
      g_cdd_alloc_fail = 0;

      if (rc2 == CDD_C_SUCCESS) {
        C_CDD_FREE(out_code);
        free_token_list(tl);
        allocation_site_list_free(&sites);
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc2);
      C_CDD_FREE(out_code);
      free_token_list(tl);
      allocation_site_list_free(&sites);
    }
  }
#endif
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, funcs2, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, funcs2, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }

  PASS();
}

TEST test_rewrite_body_funcs_oom_strdup(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 50; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source = az_span_create_from_str(
        (char *)(size_t)(size_t) "void f() { char *s; s = my_strdup(\"a\"); "
                                 "char *s2 = "
                                 "my_strdup(\"b\"); outer(inner(\"x\")); }");
    struct RefactoredFunction funcs2[] = {
        {"my_strdup", REF_PTR_TO_INT_OUT, "char *"},
        {"inner", REF_PTR_TO_INT_OUT, "char *"}};

    tokenize(source, &tl);
    find_allocations(tl, &sites);

    /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
    g_cdd_strdup_fail = i;
    {
      int rc2 = rewrite_body(tl, &sites, funcs2, 2, NULL, &out_code);
      g_cdd_strdup_fail = 0;

      if (rc2 == CDD_C_SUCCESS) {
        C_CDD_FREE(out_code);
        free_token_list(tl);
        allocation_site_list_free(&sites);
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc2);
      C_CDD_FREE(out_code);
      free_token_list(tl);
      allocation_site_list_free(&sites);
    }
  }
#endif
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, funcs2, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, funcs2, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }

  PASS();
}
TEST test_rewrite_body_funcs_oom_assignment(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 50; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source = az_span_create_from_str(
        (char *)(size_t)(size_t) "void f() { char *s; s=my_strdup(\"c\"); }");
    struct RefactoredFunction funcs2[] = {
        {"my_strdup", REF_PTR_TO_INT_OUT, "char *"}};

    tokenize(source, &tl);
    find_allocations(tl, &sites);

    /*  (moved to global) */
    g_cdd_alloc_fail = i;
    {
      int rc2 = rewrite_body(tl, &sites, funcs2, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;

      if (rc2 == CDD_C_SUCCESS) {
        C_CDD_FREE(out_code);
        free_token_list(tl);
        allocation_site_list_free(&sites);
        break;
      }
      C_CDD_FREE(out_code);
      free_token_list(tl);
      allocation_site_list_free(&sites);
    }
  }
#endif
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, funcs2, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, funcs2, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }

  PASS();
}
TEST test_rewrite_body_funcs_oom_debug(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 50; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source = az_span_create_from_str(
        (char *)(size_t)(size_t) "void f() { char *s; s = my_strdup(\"a\"); "
                                 "char *s2 = "
                                 "my_strdup(\"b\"); outer(inner(\"x\")); }");
    struct RefactoredFunction funcs2[] = {
        {"my_strdup", REF_PTR_TO_INT_OUT, "char *"},
        {"inner", REF_PTR_TO_INT_OUT, "char *"}};

    tokenize(source, &tl);
    find_allocations(tl, &sites);

    /*  (moved to global) */
    g_cdd_alloc_fail = i;
    {
      int rc2 = rewrite_body(tl, &sites, funcs2, 2, NULL, &out_code);
      g_cdd_alloc_fail = 0;

      if (rc2 == CDD_C_ERROR_MEMORY) {
        /* Good, failed as expected */
      } else {
        /* printf("i=%d rc=%d\n", i, rc); */
      }
      C_CDD_FREE(out_code);
      free_token_list(tl);
      allocation_site_list_free(&sites);
    }
  }
#endif
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, funcs2, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, funcs2, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REWRITER_BODY_OOM_H */
