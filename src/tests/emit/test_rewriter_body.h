#ifndef TEST_REWRITER_BODY_H
#define TEST_REWRITER_BODY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
  /* extern C_CDD_EXPORT int g_patcher_test_cap_1; (moved to global) */
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/rewriter_body.h"
#include "functions/parse/analysis.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */

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

/* --- Call-Site Propagation Tests --- */

/**
 * @brief test_propagate_void_stmt
 * @return TEST
 */
TEST test_propagate_void_stmt(void) {
  const char *input = ""
                      "void f() { do_work(); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"do_work", REF_VOID_TO_INT, NULL}};
  int rc = 0;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  printf("OUTPUT: %s\n", output);
  ASSERT(strstr(output, "cdd_c_error_t rc = CDD_C_SUCCESS;") != NULL);
  ASSERT(strstr(output,
                "rc = do_work(); if (rc != CDD_C_SUCCESS) return rc;") != NULL);

  free(output);
  g_fail_io_after = -1;
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

/**
 * @brief test_propagate_ptr_assignment
 * @return TEST
 */
TEST test_propagate_ptr_assignment2(void) {
  const char *input = ""
                      "void f() { char *s; s=my_strdup(\"a\"); }";
  char *output = NULL;
  struct RefactoredFunction funcs2[] = {
      {"my_strdup", REF_PTR_TO_INT_OUT, "char *"}};
  int rc2 = 0;

  rc2 = run_body_rewrite(input, funcs2, 1, NULL, &output);
  ASSERT_EQ(0, rc2);

  printf("OUTPUT4: \"%s\"\n", output);
  fflush(stdout);

  ASSERT(strstr(output, "rc =my_strdup(\"a\", &s);") != NULL ||
         strstr(output, "rc = my_strdup(\"a\", &s);") != NULL);
  ASSERT(strstr(output, "if (rc != CDD_C_SUCCESS) return rc;") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
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
      /* int rc2; */
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
TEST test_propagate_ptr_assignment(void) {
  const char *input = ""
                      "void f() { char *s; s = my_strdup(\"a\"); }";
  char *output = NULL;
  struct RefactoredFunction funcs2[] = {
      {"my_strdup", REF_PTR_TO_INT_OUT, "char *"}};
  int rc2 = 0;

  rc2 = run_body_rewrite(input, funcs2, 1, NULL, &output);
  ASSERT_EQ(0, rc2);

  /* s = my_strdup(\"a\") -> rc = my_strdup("a", &s); if(rc) ... */
  ASSERT(strstr(output, "rc = my_strdup(\"a\", &s);") != NULL);
  ASSERT(strstr(output, "if (rc != CDD_C_SUCCESS) return rc;") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
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
      /* int rc2; */
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

/**
 * @brief test_propagate_ptr_declaration
 * @return TEST
 */
TEST test_propagate_ptr_declaration(void) {
  const char *input = ""
                      "void f() { char *s = my_strdup(\"a\"); }";
  char *output = NULL;
  struct RefactoredFunction funcs2[] = {
      {"my_strdup", REF_PTR_TO_INT_OUT, "char *"}};
  int rc2 = 0;

  rc2 = run_body_rewrite(input, funcs2, 1, NULL, &output);
  ASSERT_EQ(0, rc2);

  /* char *s = ... -> char *s ; = my_strdup("a", &s); ... */
  /* Logic: split decl `char *s` and call */
  ASSERT(strstr(output, "char *s") != NULL);
  ASSERT(strstr(output, "; rc = my_strdup(\"a\", &s);") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
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
      /* int rc2; */
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

/**
 * @brief test_propagate_nested_hoisting
 * @return TEST
 */
TEST test_propagate_nested_hoisting(void) {
  const char *input = ""
                      "void f() { outer(inner(\"x\")); }";
  char *output = NULL;
  struct RefactoredFunction funcs2[] = {
      {"inner", REF_PTR_TO_INT_OUT, "char *"}};
  int rc2 = 0;

  rc2 = run_body_rewrite(input, funcs2, 1, NULL, &output);
  ASSERT_EQ(0, rc2);

  /* Should hoist: char *tmp; rc = inner("x", &tmp); if(rc)... outer(tmp); */
  ASSERT(strstr(output, "char * _tmp_cdd_0;") != NULL);
  ASSERT(strstr(output, "rc = inner(\"x\", &_tmp_cdd_0);") != NULL);
  ASSERT(strstr(output, "outer(_tmp_cdd_0);") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
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
      /* int rc2; */
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

/* --- Safety Tests (Repeat from Deliv 2 for Integration Check) --- */

/**
 * @brief test_integration_safety_and_prop
 * @return TEST
 */
TEST test_integration_safety_and_prop(void) {
  const char *input =
      ""
      "void f() { char * p = (char *)(size_t)(size_t)malloc(10); "
      "if(!p) return; do_work(); }";
  char *output = NULL;
  struct RefactoredFunction funcs2[] = {{"do_work", REF_VOID_TO_INT, NULL}};
  int rc2 = 0;

  rc2 = run_body_rewrite(input, funcs2, 1, NULL, &output);
  ASSERT_EQ(0, rc2);

  printf("OUTPUT: %s\n", output);
  ASSERT(strstr(output, "cdd_c_error_t rc = CDD_C_SUCCESS;") != NULL);
  /* Malloc analysis finding check so no injection */
  /* do_work rewritten */
  ASSERT(strstr(output, "rc = do_work();") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = i;
      g_cdd_strdup_fail = 0;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
      struct RefactoredFunction funcs_inner[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
      tokenize(az_span_create_from_str((char *)(size_t)(size_t)(size_t)code),
               &tl2);
      g_cdd_alloc_fail = 0;
      g_cdd_strdup_fail = i;
      rc2 = rewrite_body(tl2, NULL, funcs_inner, 1, NULL, &out_code);
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
      /* int rc2; */
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
      /* int rc2; */
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

/**
 * @brief test_realloc_safety_injection
 * @return TEST
 */
TEST test_realloc_safety_injection(void) {
  const char *input = ""
                      "void f() { char *p; p = realloc(p, 100); }";
  char *output = NULL;
  int rc2 = 0;

  rc2 = run_body_rewrite(input, NULL, 0, NULL, &output);
  ASSERT_EQ(0, rc2);

  /* Should rewrite: p = realloc(p, 100); ->
     { void *_safe_tmp = realloc(p, 100); if (!_safe_tmp) return
     CDD_C_ERROR_MEMORY; p = _safe_tmp; } */
  ASSERT(strstr(output, "void *_safe_tmp = realloc(p, 100);") != NULL);
  printf("OUTPUT: %s\n", output);
  ASSERT(strstr(output, "if (!_safe_tmp) return CDD_C_ERROR_MEMORY;") != NULL);
  ASSERT(strstr(output, "p = _safe_tmp;") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      /* int rc2; */
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
      /* int rc2; */
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
      /* int rc2; */
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
      /* int rc2; */
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

/**
 * @brief rewriter_body_suite
 */

#include "emit/test_rewriter_body_bounds.h"
#include "emit/test_rewriter_body_branches.h"
#include "emit/test_rewriter_body_corner.h"
#include "emit/test_rewriter_body_oom.h"

SUITE(rewriter_body_suite) {
  RUN_TEST(test_run_body_rewrite_coverage);
  RUN_TEST(test_rewrite_body_all_branches);
  RUN_TEST(test_rewrite_body_funcs_oom);
  RUN_TEST(test_rewrite_body_funcs_oom_strdup);
  RUN_TEST(test_rewrite_body_funcs_oom_assignment);
  RUN_TEST(test_rewrite_body_funcs_oom_debug);
  RUN_TEST(test_rewrite_body_oom);
  RUN_TEST(test_rewriter_body_bounds);
  RUN_TEST(test_propagate_void_stmt);
  RUN_TEST(test_propagate_ptr_assignment);
  RUN_TEST(test_propagate_ptr_assignment2);
  RUN_TEST(test_propagate_ptr_declaration);
  RUN_TEST(test_propagate_nested_hoisting);
  RUN_TEST(test_integration_safety_and_prop);
  RUN_TEST(test_realloc_safety_injection);
  RUN_TEST(test_rewriter_body_bounds2);
  RUN_TEST(test_rewrite_body_corner_cases);
  RUN_TEST(test_rewriter_body_oom);
  RUN_TEST(test_rewrite_body_corner_oom_2);
  RUN_TEST(test_rewrite_body_error_percolation);

  RUN_TEST(test_propagate_void_stmt_return);
  RUN_TEST(test_propagate_void_stmt_transform);
  RUN_TEST(test_propagate_nested_parens);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REWRITER_BODY_H */
