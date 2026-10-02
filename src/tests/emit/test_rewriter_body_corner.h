/**
 * @file test_rewriter_body_corner.h
 * @brief Unit tests for rewriter body corner cases and error percolation.
 */

#ifndef TEST_REWRITER_BODY_CORNER_H
#define TEST_REWRITER_BODY_CORNER_H

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

TEST test_rewrite_body_corner_cases(void) {
  {
    int i;
    for (i = 1; i < 200; i++) {
      const char code[] = "{"
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "}";
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
      const char code[] = "{"
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "int y = my_func(x); int y = my_func(x); int y = "
                          "my_func(x); int y = my_func(x); "
                          "}";
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
    int i;
    for (i = 1; i < 50; i++) {
      const char code[] = "{"
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "}";
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
      const char code[] = "{"
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "}";
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
    int i;
    for (i = 1; i < 200; i++) {
      const char code[] =
          "{"
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "}";
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
      const char code[] =
          "{"
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; z=my_func(x)+1; "
          "}";
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
    int i;
    for (i = 1; i < 50; i++) {
      const char code[] = "{"
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct AllocationSiteList sites = {0};
      struct SignatureTransform t = {TRANSFORM_RET_PTR_TO_ARG, "a", "b", "c",
                                     "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      find_allocations(tl2, &sites);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, &sites, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      allocation_site_list_free(&sites);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char code[] = "{"
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "return malloc(1); return malloc(1); return "
                          "malloc(1); return malloc(1); "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct AllocationSiteList sites = {0};
      struct SignatureTransform t = {TRANSFORM_RET_PTR_TO_ARG, "a", "b", "c",
                                     "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      find_allocations(tl2, &sites);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, &sites, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      allocation_site_list_free(&sites);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char code[] = "{"
                          "return return return return return "
                          "return return return return return "
                          "return return return return return "
                          "return return return return return "
                          "}";
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
      const char code[] = "{"
                          "return return return return return "
                          "return return return return return "
                          "return return return return return "
                          "return return return return return "
                          "}";
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
  {
    int i;
    for (i = 1; i < 200; i++) {
      const char code[] = "{"
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {{"do_work", REF_VOID_TO_INT, NULL}};
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
      const char code[] = "{"
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct RefactoredFunction funcs2[] = {{"do_work", REF_VOID_TO_INT, NULL}};
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
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x)";
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
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x)";
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
    int i;
    for (i = 1; i < 50; i++) {
      const char *code =
          (char *)(size_t)(size_t)(size_t) "void *p = malloc(1); return p;";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct AllocationSiteList sites = {0};
      struct SignatureTransform t = {TRANSFORM_RET_PTR_TO_ARG, "a", "b", "c",
                                     "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      find_allocations(tl2, &sites);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, &sites, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      allocation_site_list_free(&sites);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
    for (i = 1; i < 50; i++) {
      const char *code =
          (char *)(size_t)(size_t)(size_t) "void *p = malloc(1); return p;";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2 = 0;
      struct AllocationSiteList sites = {0};
      struct SignatureTransform t = {TRANSFORM_RET_PTR_TO_ARG, "a", "b", "c",
                                     "d"};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      find_allocations(tl2, &sites);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, &sites, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = 0;
      free_token_list(tl2);
      allocation_site_list_free(&sites);
      C_CDD_FREE(out_code);
      if (rc2 == CDD_C_SUCCESS)
        rc2 += 0;
    }
  }
  {
    int i;
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
    int i;
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

TEST test_rewrite_body_corner_oom_2(void) {
  const char *cases[] = {
      "void f() { char *s = my_strdup(\"a\"); }",
      "void f() { my_func(\"a\"); }", "void f() { return my_func(\"a\"); }",
      "void f() { my_func(\"a\") }", "void f() { if (1) { return; } }"};
  struct RefactoredFunction funcs2[] = {
      {"my_strdup", REF_PTR_TO_INT_OUT, "char *"},
      {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
  struct SignatureTransform t1 = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
  struct SignatureTransform t2 = {TRANSFORM_RET_PTR_TO_ARG, "a", "b", "c", "d"};

  int c;
  for (c = 0; c < 5; ++c) {
    int i;
    for (i = 1; i < 50; ++i) {
      struct TokenList *tl = NULL;
      char *out_code = NULL;
      /*  (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)cases[c]), &tl);
      g_cdd_alloc_fail = i;
      {
        int rc2 = rewrite_body(tl, NULL, funcs2, 2,
                               c == 4 ? &t1 : (c == 2 ? &t2 : NULL), &out_code);
        g_cdd_alloc_fail = 0;
        free_token_list(tl);
        if (out_code)
          C_CDD_FREE(out_code);
        if (rc2 == CDD_C_SUCCESS) {
          break;
        }
      }
    }
  }

  for (c = 0; c < 5; ++c) {
    int i;
    for (i = 1; i < 50; ++i) {
      struct TokenList *tl = NULL;
      char *out_code = NULL;
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)cases[c]), &tl);
      g_cdd_strdup_fail = i;
      {
        int rc2 = rewrite_body(tl, NULL, funcs2, 2,
                               c == 4 ? &t1 : (c == 2 ? &t2 : NULL), &out_code);
        g_cdd_strdup_fail = 0;
        free_token_list(tl);
        if (out_code)
          C_CDD_FREE(out_code);
        if (rc2 == CDD_C_SUCCESS) {
          break;
        }
      }
    }
  }

  PASS();
}

/**
 * @brief Test error percolation across rewrite_body subroutines.
 */
TEST test_rewrite_body_error_percolation(void) {
#ifdef CDD_BUILD_TESTS
  const char *cases[8];
  struct RefactoredFunction funcs2[2];
  struct SignatureTransform t_void;
  struct SignatureTransform t_ret;
  int c;
  int i;
  struct TokenList *tl = NULL;
  char *out_code = NULL;
  cdd_c_error_t rc = 0;
  extern C_CDD_EXPORT int g_cdd_fail_find_semicolon;
  extern C_CDD_EXPORT int g_cdd_fail_find_stmt_start;
  extern C_CDD_EXPORT int g_cdd_fail_find_refactored_func;
  extern C_CDD_EXPORT int g_cdd_fail_patch_list_add;

  cases[0] = "void f() { char *s = my_strdup(\"a\"); }";
  cases[1] = "void f() { char *s; s = my_strdup(\"a\"); }";
  cases[2] = "void f() { my_func(\"a\"); }";
  cases[3] = "void f() { outer(my_func(\"a\")); }";
  cases[4] = "void f() { return my_func(\"a\"); }";
  cases[5] = "void f() { return s; }";
  cases[6] = "void f() { return; }";
  cases[7] = "void f() { int x = 1; }";

  funcs2[0].name = "my_strdup";
  funcs2[0].type = REF_PTR_TO_INT_OUT;
  funcs2[0].original_return_type = "char *";

  funcs2[1].name = "my_func";
  funcs2[1].type = REF_PTR_TO_INT_OUT;
  funcs2[1].original_return_type = "char *";

  t_void.type = TRANSFORM_VOID_TO_INT;
  t_void.return_type = "int";
  t_void.arg_name = "f";
  t_void.error_code = NULL;
  t_void.success_code = "CDD_C_SUCCESS";

  t_ret.type = TRANSFORM_RET_PTR_TO_ARG;
  t_ret.return_type = "char *";
  t_ret.arg_name = "f";
  t_ret.error_code = "out";
  t_ret.success_code = "CDD_C_SUCCESS";

  /* 1. patch_list_init OOM */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "void f() {}"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_alloc_fail = 1;
  rc = rewrite_body(tl, NULL, NULL, 0, NULL, &out_code);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  free_token_list(tl);

  /* 2. find_refactored_func failure */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { my_strdup(\"a\"); }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_find_refactored_func = 2;
  rc = rewrite_body(tl, NULL, funcs2, 2, NULL, &out_code);
  g_cdd_fail_find_refactored_func = 0;
  ASSERT(rc != CDD_C_SUCCESS);
  free_token_list(tl);

  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { my_strdup(\"a\"); }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_find_refactored_func = 1;
  rc = rewrite_body(tl, NULL, funcs2, 2, NULL, &out_code);
  g_cdd_fail_find_refactored_func = 0;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  free_token_list(tl);

  /* 3a. find_semicolon failure in statement (case 2) */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { my_strdup(\"a\"); }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_find_semicolon = 2;
  rc = rewrite_body(tl, NULL, funcs2, 2, NULL, &out_code);
  g_cdd_fail_find_semicolon = 0;
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { my_strdup(\"a\"); }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_find_semicolon = 1;
  rc = rewrite_body(tl, NULL, funcs2, 2, NULL, &out_code);
  g_cdd_fail_find_semicolon = 0;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  free_token_list(tl);

  /* 3b. find_semicolon failure in assignment (case 1) */
  rc = tokenize(
      az_span_create_from_str(
          (char *)(size_t) "void f() { struct S *s = my_strdup(\"a\"); }"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_find_semicolon = 1;
  rc = rewrite_body(tl, NULL, funcs2, 2, NULL, &out_code);
  g_cdd_fail_find_semicolon = 0;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  free_token_list(tl);

  /* 3c. find_semicolon failure in return ptr transform */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "void f() { return s; }"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_find_semicolon = 1;
  rc = rewrite_body(tl, NULL, NULL, 0, &t_ret, &out_code);
  g_cdd_fail_find_semicolon = 0;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  free_token_list(tl);

  /* 3d. patch_list_add failure in is_decl assignment (patches 1, 2, 3) */
  for (i = 1; i <= 3; ++i) {
    rc = tokenize(
        az_span_create_from_str(
            (char *)(size_t) "void f() { struct S *s = my_strdup(\"a\"); }"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_fail_patch_list_add = i;
    rc = rewrite_body(tl, NULL, funcs2, 2, NULL, &out_code);
    g_cdd_fail_patch_list_add = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    free_token_list(tl);
  }

  /* 3d2. patch_list_add failure in non-decl assignment (patches 1, 2, 3) */
  for (i = 1; i <= 3; ++i) {
    rc = tokenize(az_span_create_from_str(
                      (char *)(size_t) "void f() { s=my_strdup(\"a\"); }"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_fail_patch_list_add = i;
    rc = rewrite_body(tl, NULL, funcs2, 2, NULL, &out_code);
    g_cdd_fail_patch_list_add = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    free_token_list(tl);
  }

  /* 3e. join_tokens_range empty strdup failure */
  {
    extern C_CDD_EXPORT int g_cdd_strdup_fail;
    rc = tokenize(az_span_create_from_str(
                      (char *)(size_t) "void f() { outer(my_func()); }"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_strdup_fail = 1;
    rc = rewrite_body(tl, NULL, funcs2, 2, NULL, &out_code);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    free_token_list(tl);
  }

  /* 4. find_stmt_start failure */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { outer(my_func(\"a\")); }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_find_stmt_start = 1;
  rc = rewrite_body(tl, NULL, funcs2, 2, NULL, &out_code);
  g_cdd_fail_find_stmt_start = 0;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  free_token_list(tl);

  /* 5. Loop patch_list_add failures across all cases */
  for (c = 0; c < 8; ++c) {
    const struct SignatureTransform *tr = NULL;
    if (c == 4 || c == 5)
      tr = &t_ret;
    else if (c == 6 || c == 7)
      tr = &t_void;

    for (i = 1; i <= 10; ++i) {
      tl = NULL;
      out_code = NULL;
      tokenize(az_span_create_from_str((char *)(size_t)cases[c]), &tl);
      g_cdd_fail_patch_list_add = i;
      rc = rewrite_body(tl, NULL, funcs2, 2, tr, &out_code);
      g_cdd_fail_patch_list_add = 0;
      free_token_list(tl);
      if (out_code)
        C_CDD_FREE(out_code);
    }
  }
#endif
  PASS();
}

/**
 * @brief Test all remaining edge case branches in rewriter_body.c.
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REWRITER_BODY_CORNER_H */
