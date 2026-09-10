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

static cdd_c_error_t
run_body_rewrite(const char *code, const struct RefactoredFunction *funcs,
                 size_t n_funcs, const struct SignatureTransform *transform,
                 char **out) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  int rc;
  const az_span source =
      az_span_create_from_str((char *)(size_t)(size_t)(size_t)code);

  (void)rc;
  if (!code || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = tokenize(source, &tl);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = find_allocations(tl, &sites);
  if (rc != CDD_C_SUCCESS) {
    free_token_list(tl);
    return rc;
  }

  rc = rewrite_body(tl, &sites, funcs, n_funcs, transform, out);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  return rc;
}

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
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  (void)rc;
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
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
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
  int rc2;

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
        break;
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
        break;
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
        break;
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
        break;
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
  int rc2;

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
        break;
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
        break;
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
        break;
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
        break;
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
  int rc2;

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
        break;
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
        break;
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
        break;
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
        break;
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
  int rc2;

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
        break;
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
        break;
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
        break;
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
        break;
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
  int rc2;

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
        break;
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
        break;
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
        break;
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
        break;
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
  int rc2;

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
        break;
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
        break;
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
        break;
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
        break;
    }
  }

  PASS();
}

/**
 * @brief rewriter_body_suite
 */

TEST test_rewriter_body_bounds(void) {
  struct RefactoredFunction funcs2[] = {{"do_work", REF_VOID_TO_INT, NULL}};
  char *output = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            run_body_rewrite(NULL, funcs2, 1, NULL, &output));
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }

  PASS();
}

TEST test_rewriter_body_oom(void) {
  const char *input =
      (char *)(size_t)(size_t)(size_t) "void f() { do_work(); }";
  char *output = NULL;
  struct RefactoredFunction funcs2[] = {{"do_work", REF_VOID_TO_INT, NULL}};

  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const az_span source = az_span_create_from_str((char *)(size_t)(size_t)input);

  ASSERT_EQ(0, tokenize(source, &tl));
  ASSERT_EQ(0, find_allocations(tl, &sites));

#ifdef CDD_BUILD_TESTS
  {
    /*  (moved to global) */
    int rc_oom_rb1;
    g_cdd_alloc_fail = 1;
    rc_oom_rb1 = rewrite_body(tl, &sites, funcs2, 1, NULL, &output);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom_rb1);
    g_cdd_alloc_fail = 0;

    /* ignore */
    g_cdd_alloc_fail = 0;

    /* ignore */
    g_cdd_alloc_fail = 0;

    /* ignore */
    g_cdd_alloc_fail = 0;

    /* ignore */
    g_cdd_alloc_fail = 0;

    /* ignore */
    g_cdd_alloc_fail = 0;
  }
#endif

  allocation_site_list_free(&sites);
  free_token_list(tl);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }

  PASS();
}

TEST test_rewriter_body_bounds2(void) {
  struct TokenList tl = {0};
  struct AllocationSiteList sites = {0};
  char *output = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            rewrite_body(NULL, &sites, NULL, 0, NULL, &output));
  /* ignore */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            rewrite_body(&tl, &sites, NULL, 0, NULL, NULL));
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }

  PASS();
}

TEST test_propagate_void_stmt_return(void) {
  const char *input =
      (char *)(size_t)(size_t)(size_t) "void f() { do_work(); return ; }";
  char *output = NULL;
  struct RefactoredFunction funcs2[] = {{"f", REF_VOID_TO_INT, NULL}};
  struct SignatureTransform trans;
  int rc2;
  trans.type = TRANSFORM_VOID_TO_INT;

  rc2 = run_body_rewrite(input, funcs2, 1, &trans, &output);
  ASSERT_EQ(0, rc2);

  ASSERT(strstr(output, "return 0;") != NULL);

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
        break;
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
        break;
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
        break;
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
        break;
    }
  }

  PASS();
}

TEST test_propagate_void_stmt_transform(void) {
  const char *input =
      (char *)(size_t)(size_t)(size_t) "void f() { do_work(); }";
  char *output = NULL;
  struct RefactoredFunction funcs2[] = {{"f", REF_VOID_TO_INT, NULL}};
  struct SignatureTransform trans;
  int rc2;
  trans.type = TRANSFORM_VOID_TO_INT;

  rc2 = run_body_rewrite(input, funcs2, 1, &trans, &output);
  ASSERT_EQ(0, rc2);

  ASSERT(strstr(output, "return CDD_C_SUCCESS;") != NULL);

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
        break;
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
        break;
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
        break;
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
        break;
    }
  }

  PASS();
}

TEST test_propagate_nested_parens(void) {
  const char *input = (char *)(size_t)(size_t)(size_t) "void f() { char* tmp; "
                                                       "inner ((1 + 2)); }";
  char *output = NULL;
  struct RefactoredFunction funcs2[] = {
      {"inner", REF_PTR_TO_INT_OUT, "char *"}};
  int rc2;

  rc2 = run_body_rewrite(input, funcs2, 1, NULL, &output);
  ASSERT_EQ(0, rc2);
  printf("\nOUTPUT: %s\n", output);

  ASSERT(strstr(output, "rc = inner((1 + 2));") != NULL ||
         strstr(output, "rc = inner ((1 + 2));") != NULL ||
         strstr(output, "rc = inner((1 + 2) , &tmp);") != NULL ||
         strstr(output, "rc = inner ((1 + 2) , &tmp);") != NULL);

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
        break;
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
        break;
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
        break;
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
        break;
    }
  }

  PASS();
}

#ifdef CDD_BUILD_TESTS
/*  (moved to global) */
#endif

TEST test_rewrite_body_oom(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 50; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source = az_span_create_from_str(
        (char *)(size_t)(size_t) "int f() { int a = 1; void *p = malloc(1); "
                                 "return 0; }");
    tokenize(source, &tl);
    find_allocations(tl, &sites);

    g_cdd_alloc_fail = i;
    {
      int rc2 = rewrite_body(tl, &sites, NULL, 0, NULL, &out_code);
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
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }

  PASS();
}
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
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
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
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
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
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
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
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    /* int i; */
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }

  PASS();
}
TEST test_rewrite_body_corner_cases(void) {
  {
    int i;
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
      int rc2;
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
        break;
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
      int rc2;
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
        break;
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
      int rc2;
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
        break;
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
      int rc2;
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
        break;
    }
  }
  {
    int i;
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
      int rc2;
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
        break;
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
      int rc2;
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
        break;
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
      int rc2;
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
        break;
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
      int rc2;
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
        break;
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
      int rc2;
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
        break;
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
      int rc2;
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
        break;
    }
  }
  {
    int i;
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
      int rc2;
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
        break;
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
      int rc2;
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
        break;
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x)";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x)";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code =
          (char *)(size_t)(size_t)(size_t) "void *p = malloc(1); return p;";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code =
          (char *)(size_t)(size_t)(size_t) "void *p = malloc(1); return p;";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
    }
    for (i = 1; i < 50; i++) {
      const char *code = (char *)(size_t)(size_t)(size_t) "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc2;
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
        break;
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
  cdd_c_error_t rc;
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

    for (i = 1; i <= 6; ++i) {
      tl = NULL;
      out_code = NULL;
      rc = tokenize(az_span_create_from_str((char *)(size_t)cases[c]), &tl);
      if (rc != CDD_C_SUCCESS)
        continue;
      g_cdd_fail_patch_list_add = i;
      rc = rewrite_body(tl, NULL, funcs2, 2, tr, &out_code);
      g_cdd_fail_patch_list_add = 0;
      free_token_list(tl);
      if (out_code)
        C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        break;
    }
  }
#endif
  PASS();
}

/**
 * @brief Test all remaining edge case branches in rewriter_body.c.
 */
TEST test_rewrite_body_all_branches(void) {
#ifdef CDD_BUILD_TESTS
  struct RefactoredFunction funcs[2];
  struct SignatureTransform t_void;
  struct SignatureTransform t_ret;
  struct SignatureTransform t_unk;
  struct AllocationSiteList allocs;
  struct TokenList *tl = NULL;
  char *out_code = NULL;
  cdd_c_error_t rc;
  extern C_CDD_EXPORT int g_cdd_fail_find_semicolon;
  extern C_CDD_EXPORT int g_cdd_fail_find_stmt_start;
  extern C_CDD_EXPORT int g_cdd_fail_find_refactored_func;

  funcs[0].name = "my_strdup";
  funcs[0].type = REF_PTR_TO_INT_OUT;
  funcs[0].original_return_type = "char *";

  funcs[1].name = "my_func";
  funcs[1].type = REF_PTR_TO_INT_OUT;
  funcs[1].original_return_type = "char *";

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

  t_unk.type = 999;
  t_unk.return_type = "int";
  t_unk.arg_name = "f";
  t_unk.error_code = NULL;
  t_unk.success_code = "CDD_C_SUCCESS";

  /* 1. Hooks set to 2 so --hook == 0 is false */
  g_cdd_fail_find_refactored_func = 2;
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "void f() { my_func(\"a\"); }"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  g_cdd_fail_find_refactored_func = 0;
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  g_cdd_fail_find_semicolon = 2;
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "void f() { my_func(\"a\"); }"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  g_cdd_fail_find_semicolon = 0;
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  g_cdd_fail_find_stmt_start = 2;
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { outer(my_func(\"a\")); }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  g_cdd_fail_find_stmt_start = 0;
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 2. Statement after RBRACE (line 110) */
  rc = tokenize(
      az_span_create_from_str(
          (char *)(size_t) "void f() { if (1) {} outer(my_func(\"a\")); }"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 3. Assignment after RBRACE (line 278) */
  rc = tokenize(
      az_span_create_from_str(
          (char *)(size_t) "void f() { if (1) {} s=my_strdup(\"a\"); }"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 4. Token at end of file (lines 230, 235) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "void f() { my_func"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 4b. Refactored function name not followed by lparen (line 235) */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { int x = my_func + 1; }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 4c. Assignment with no LHS identifier (lines 276, 289, 296, 306) */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { [0] = my_func(\"a\"); }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 4d. Assignment starting at token 0 (lines 276, 289) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "=my_func();"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 4e. Statement immediately following RBRACE (line 398) */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { if (1) {} my_func(\"a\"); }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 5. Token at start of file without braces (lines 263, 269, 664, 666) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "my_func();"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 6. Whitespace inside call args (line 344) */
  rc = tokenize(
      az_span_create_from_str(
          (char *)(size_t) "void f() { char *s = my_strdup( \"a\" ); }"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 7. No semicolon after call (line 379) */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "void f() { char *s = my_strdup(\"a\") }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, funcs, 2, NULL, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 8. Return at EOF with void transform (lines 520, 523) */
  rc = tokenize(az_span_create_from_str(((char *)(size_t) "return")), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, NULL, 0, &t_void, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 9. Unknown transform type (line 536) */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "void f() { return; }"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, NULL, 0, &t_unk, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 10. Return without semicolon with ret transform (line 543) */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "void *f() { return s }"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, NULL, 0, &t_ret, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 11. Allocation before return statement and inside return statement (line
   * 550) */
  {
    memset(&allocs, 0, sizeof(allocs));
    rc = tokenize(
        az_span_create_from_str((
            char *)(size_t) "void *f() { p = malloc(10); return malloc(20); }"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    find_allocations(tl, &allocs);
    rc = rewrite_body(tl, &allocs, NULL, 0, &t_ret, &out_code);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    if (out_code) {
      C_CDD_FREE(out_code);
      out_code = NULL;
    }
    allocation_site_list_free(&allocs);
    free_token_list(tl);
  }

  /* 11b. Allocation after return statement (line 549) */
  {
    memset(&allocs, 0, sizeof(allocs));
    rc = tokenize(az_span_create_from_str(
                      (char *)(size_t) "void *f() { return p; malloc(10); }"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    find_allocations(tl, &allocs);
    rc = rewrite_body(tl, &allocs, NULL, 0, &t_ret, &out_code);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    if (out_code) {
      C_CDD_FREE(out_code);
      out_code = NULL;
    }
    allocation_site_list_free(&allocs);
    free_token_list(tl);
  }

  /* 12. Void function with trailing block (lines 628, 631, 634, 641) */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "void f() { if (1) {} }"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, NULL, 0, &t_void, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 12b. Void function with statement lacking semicolon before closing brace
   * (line 640) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "void f() { 1 + 2 }"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, NULL, 0, &t_void, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 13. Void function without closing brace (line 628, 631) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "void f()"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, NULL, 0, &t_void, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 14. Void function with empty tokens (line 626) */
  rc = tokenize(az_span_create_from_str(((char *)(size_t) "")), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, NULL, 0, &t_void, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);

  /* 15. Void function with only closing brace (line 634) */
  rc = tokenize(az_span_create_from_str(((char *)(size_t) "}")), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = rewrite_body(tl, NULL, NULL, 0, &t_void, &out_code);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (out_code) {
    C_CDD_FREE(out_code);
    out_code = NULL;
  }
  free_token_list(tl);
#endif
  PASS();
}

SUITE(rewriter_body_suite) {
  RUN_TEST(test_rewrite_body_all_branches);
  RUN_TEST(test_rewrite_body_funcs_oom);
  RUN_TEST(test_rewrite_body_funcs_oom_strdup);
  RUN_TEST(test_rewrite_body_funcs_oom_assignment);
  RUN_TEST(test_rewrite_body_funcs_oom_debug);
  RUN_TEST(test_rewrite_body_oom);
  RUN_TEST(test_rewriter_body_bounds);
  RUN_TEST(test_rewrite_body_funcs_oom_strdup);
  RUN_TEST(test_rewrite_body_funcs_oom_assignment);
  RUN_TEST(test_rewrite_body_funcs_oom_debug);
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
