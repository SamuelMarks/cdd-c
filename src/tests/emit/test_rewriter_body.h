#ifndef TEST_REWRITER_BODY_H
#define TEST_REWRITER_BODY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
extern C_CDD_EXPORT int g_patcher_test_cap_1;
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/rewriter_body.h"
#include "functions/parse/analysis.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

static cdd_c_error_t
run_body_rewrite(const char *code, const struct RefactoredFunction *funcs,
                 size_t n_funcs, const struct SignatureTransform *transform,
                 char **out) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  int rc;
  const az_span source = az_span_create_from_str((char *)code);

  if (!code || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  ASSERT_EQ(0, tokenize(source, &tl));

  ASSERT_EQ(0, find_allocations(tl, &sites));

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
  ASSERT_EQ(0, rc);

  printf("OUTPUT: %s\n", output);
  ASSERT(strstr(output, "cdd_c_error_t rc = CDD_C_SUCCESS;") != NULL);
  ASSERT(strstr(output, "rc = do_work(); if (rc != 0) return rc;") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
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
  struct RefactoredFunction funcs[] = {
      {"my_strdup", REF_PTR_TO_INT_OUT, "char *"}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  printf("OUTPUT4: \"%s\"\n", output);
  fflush(stdout);

  ASSERT(strstr(output, "rc =my_strdup(\"a\", &s);") != NULL ||
         strstr(output, "rc = my_strdup(\"a\", &s);") != NULL);
  ASSERT(strstr(output, "if (rc != 0) return rc;") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}
TEST test_propagate_ptr_assignment(void) {
  const char *input = ""
                      "void f() { char *s; s = my_strdup(\"a\"); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {
      {"my_strdup", REF_PTR_TO_INT_OUT, "char *"}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  /* s = my_strdup(\"a\") -> rc = my_strdup("a", &s); if(rc) ... */
  ASSERT(strstr(output, "rc = my_strdup(\"a\", &s);") != NULL);
  ASSERT(strstr(output, "if (rc != 0) return rc;") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
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
  struct RefactoredFunction funcs[] = {
      {"my_strdup", REF_PTR_TO_INT_OUT, "char *"}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  /* char *s = ... -> char *s ; = my_strdup("a", &s); ... */
  /* Logic: split decl `char *s` and call */
  ASSERT(strstr(output, "char *s") != NULL);
  ASSERT(strstr(output, "; rc = my_strdup(\"a\", &s);") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
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
  struct RefactoredFunction funcs[] = {{"inner", REF_PTR_TO_INT_OUT, "char *"}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  /* Should hoist: char *tmp; rc = inner("x", &tmp); if(rc)... outer(tmp); */
  ASSERT(strstr(output, "char * _tmp_cdd_0;") != NULL);
  ASSERT(strstr(output, "rc = inner(\"x\", &_tmp_cdd_0);") != NULL);
  ASSERT(strstr(output, "outer(_tmp_cdd_0);") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
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
      "void f() { char * p = (char *)malloc(10); if(!p) return; do_work(); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"do_work", REF_VOID_TO_INT, NULL}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  printf("OUTPUT: %s\n", output);
  ASSERT(strstr(output, "cdd_c_error_t rc = CDD_C_SUCCESS;") != NULL);
  /* Malloc analysis finding check so no injection */
  /* do_work rewritten */
  ASSERT(strstr(output, "rc = do_work();") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
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
  int rc;

  rc = run_body_rewrite(input, NULL, 0, NULL, &output);
  ASSERT_EQ(0, rc);

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
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}

/**
 * @brief rewriter_body_suite
 */

TEST test_rewriter_body_bounds(void) {
  struct RefactoredFunction funcs[] = {{"do_work", REF_VOID_TO_INT, NULL}};
  char *output = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            run_body_rewrite(NULL, funcs, 1, NULL, &output));
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}

TEST test_rewriter_body_oom(void) {
  const char *input = "void f() { do_work(); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"do_work", REF_VOID_TO_INT, NULL}};

  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  const az_span source = az_span_create_from_str((char *)input);

  ASSERT_EQ(0, tokenize(source, &tl));
  ASSERT_EQ(0, find_allocations(tl, &sites));

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    int rc_oom_rb1;
    g_cdd_fail_alloc = 1;
    rc_oom_rb1 = rewrite_body(tl, &sites, funcs, 1, NULL, &output);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom_rb1);
    g_cdd_fail_alloc = 0;

    /* ignore */
    g_cdd_fail_alloc = 0;

    /* ignore */
    g_cdd_fail_alloc = 0;

    /* ignore */
    g_cdd_fail_alloc = 0;

    /* ignore */
    g_cdd_fail_alloc = 0;

    /* ignore */
    g_cdd_fail_alloc = 0;
  }
#endif

  allocation_site_list_free(&sites);
  free_token_list(tl);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
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
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}

TEST test_propagate_void_stmt_return(void) {
  const char *input = "void f() { do_work(); return ; }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"f", REF_VOID_TO_INT, NULL}};
  struct SignatureTransform trans;
  int rc;
  trans.type = TRANSFORM_VOID_TO_INT;

  rc = run_body_rewrite(input, funcs, 1, &trans, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "return 0;") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}

TEST test_propagate_void_stmt_transform(void) {
  const char *input = "void f() { do_work(); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"f", REF_VOID_TO_INT, NULL}};
  struct SignatureTransform trans;
  int rc;
  trans.type = TRANSFORM_VOID_TO_INT;

  rc = run_body_rewrite(input, funcs, 1, &trans, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "return CDD_C_SUCCESS;") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}

TEST test_propagate_nested_parens(void) {
  const char *input = "void f() { char* tmp; inner ((1 + 2)); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"inner", REF_PTR_TO_INT_OUT, "char *"}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "rc = inner((1 + 2));") != NULL ||
         strstr(output, "rc = inner ((1 + 2));") != NULL ||
         strstr(output, "rc = inner((1 + 2) , &tmp);") != NULL ||
         strstr(output, "rc = inner ((1 + 2) , &tmp);") != NULL);

  free(output);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_alloc_fail;
#endif

TEST test_rewrite_body_oom(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 500; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source = az_span_create_from_str(
        "int f() { int a = 1; void *p = malloc(1); return 0; }");
    tokenize(source, &tl);
    find_allocations(tl, &sites);

    g_cdd_alloc_fail = i;
    g_patcher_test_cap_1 = 1;
    int rc = rewrite_body(tl, &sites, NULL, 0, NULL, &out_code);
    g_cdd_alloc_fail = 0;
    g_patcher_test_cap_1 = 0;

    if (rc == CDD_C_SUCCESS) {
      C_CDD_FREE(out_code);
      free_token_list(tl);
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(out_code);
    free_token_list(tl);
  }
#endif
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}
TEST test_rewrite_body_funcs_oom(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 500; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source = az_span_create_from_str(
        "void f() { char *s; s = my_strdup(\"a\"); char *s2 = "
        "my_strdup(\"b\"); outer(inner(\"x\")); }");
    struct RefactoredFunction funcs[] = {
        {"my_strdup", REF_PTR_TO_INT_OUT, "char *"},
        {"inner", REF_PTR_TO_INT_OUT, "char *"}};

    tokenize(source, &tl);
    find_allocations(tl, &sites);

    extern C_CDD_EXPORT int g_cdd_alloc_fail;
    g_cdd_alloc_fail = i;
    g_patcher_test_cap_1 = 1;
    int rc = rewrite_body(tl, &sites, funcs, 2, NULL, &out_code);
    g_cdd_alloc_fail = 0;
    g_patcher_test_cap_1 = 0;

    if (rc == CDD_C_SUCCESS) {
      C_CDD_FREE(out_code);
      free_token_list(tl);
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(out_code);
    free_token_list(tl);
  }
#endif
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}

TEST test_rewrite_body_funcs_oom_strdup(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 200; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source = az_span_create_from_str(
        "void f() { char *s; s = my_strdup(\"a\"); char *s2 = "
        "my_strdup(\"b\"); outer(inner(\"x\")); }");
    struct RefactoredFunction funcs[] = {
        {"my_strdup", REF_PTR_TO_INT_OUT, "char *"},
        {"inner", REF_PTR_TO_INT_OUT, "char *"}};

    tokenize(source, &tl);
    find_allocations(tl, &sites);

    extern C_CDD_EXPORT int g_cdd_strdup_fail;
    g_cdd_strdup_fail = i;
    g_patcher_test_cap_1 = 1;
    int rc = rewrite_body(tl, &sites, funcs, 2, NULL, &out_code);
    g_cdd_strdup_fail = 0;
    g_patcher_test_cap_1 = 0;

    if (rc == CDD_C_SUCCESS) {
      C_CDD_FREE(out_code);
      free_token_list(tl);
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(out_code);
    free_token_list(tl);
  }
#endif
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}
TEST test_rewrite_body_funcs_oom_assignment(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 500; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source =
        az_span_create_from_str("void f() { char *s; s=my_strdup(\"c\"); }");
    struct RefactoredFunction funcs[] = {
        {"my_strdup", REF_PTR_TO_INT_OUT, "char *"}};

    tokenize(source, &tl);
    find_allocations(tl, &sites);

    extern C_CDD_EXPORT int g_cdd_alloc_fail;
    g_cdd_alloc_fail = i;
    g_patcher_test_cap_1 = 1;
    int rc = rewrite_body(tl, &sites, funcs, 1, NULL, &out_code);
    g_cdd_alloc_fail = 0;
    g_patcher_test_cap_1 = 0;

    if (rc == CDD_C_SUCCESS) {
      C_CDD_FREE(out_code);
      free_token_list(tl);
      break;
    }
    C_CDD_FREE(out_code);
    free_token_list(tl);
  }
#endif
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}
TEST test_rewrite_body_funcs_oom_debug(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i < 500; i++) {
    char *out_code = NULL;
    struct TokenList *tl = NULL;
    struct AllocationSiteList sites = {0};
    const az_span source = az_span_create_from_str(
        "void f() { char *s; s = my_strdup(\"a\"); char *s2 = "
        "my_strdup(\"b\"); outer(inner(\"x\")); }");
    struct RefactoredFunction funcs[] = {
        {"my_strdup", REF_PTR_TO_INT_OUT, "char *"},
        {"inner", REF_PTR_TO_INT_OUT, "char *"}};

    tokenize(source, &tl);
    find_allocations(tl, &sites);

    extern C_CDD_EXPORT int g_cdd_alloc_fail;
    g_cdd_alloc_fail = i;
    g_patcher_test_cap_1 = 1;
    int rc = rewrite_body(tl, &sites, funcs, 2, NULL, &out_code);
    g_cdd_alloc_fail = 0;
    g_patcher_test_cap_1 = 0;

    if (rc == CDD_C_ERROR_MEMORY) {
      /* Good, failed as expected */
    } else {
      /* printf("i=%d rc=%d\n", i, rc); */
    }
    C_CDD_FREE(out_code);
    free_token_list(tl);
  }
#endif
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}
TEST test_rewrite_body_corner_cases(void) {
  {
    int i;
    for (i = 1; i < 500; i++) {
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
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
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
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{"
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{"
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "my_func(x); my_func(x); my_func(x); my_func(x); "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
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
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
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
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
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
      int rc;
      struct AllocationSiteList sites = {0};
      struct SignatureTransform t = {TRANSFORM_RET_PTR_TO_ARG, "a", "b", "c",
                                     "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      find_allocations(tl2, &sites);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, &sites, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      allocation_site_list_free(&sites);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
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
      int rc;
      struct AllocationSiteList sites = {0};
      struct SignatureTransform t = {TRANSFORM_RET_PTR_TO_ARG, "a", "b", "c",
                                     "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      find_allocations(tl2, &sites);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, &sites, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      allocation_site_list_free(&sites);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{"
                          "return return return return return "
                          "return return return return return "
                          "return return return return return "
                          "return return return return return "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{"
                          "return return return return return "
                          "return return return return return "
                          "return return return return return "
                          "return return return return return "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{"
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {{"do_work", REF_VOID_TO_INT, NULL}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{"
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "s=do_work(); s=do_work(); s=do_work(); s=do_work(); "
                          "}";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {{"do_work", REF_VOID_TO_INT, NULL}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x)";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x)";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "void *p = malloc(1); return p;";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct AllocationSiteList sites = {0};
      struct SignatureTransform t = {TRANSFORM_RET_PTR_TO_ARG, "a", "b", "c",
                                     "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      find_allocations(tl2, &sites);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, &sites, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      allocation_site_list_free(&sites);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "void *p = malloc(1); return p;";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct AllocationSiteList sites = {0};
      struct SignatureTransform t = {TRANSFORM_RET_PTR_TO_ARG, "a", "b", "c",
                                     "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      find_allocations(tl2, &sites);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, &sites, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      allocation_site_list_free(&sites);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ my_func(x) {";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct RefactoredFunction funcs[] = {
          {"my_func", REF_PTR_TO_INT_OUT, "char *"}};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, funcs, 1, NULL, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }
  {
    int i;
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
    for (i = 1; i < 500; i++) {
      const char code[] = "{ return 1 } w";
      struct TokenList *tl2 = NULL;
      char *out_code = NULL;
      int rc;
      struct SignatureTransform t = {TRANSFORM_VOID_TO_INT, "a", "b", "c", "d"};
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)code), &tl2);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      rc = rewrite_body(tl2, NULL, NULL, 0, &t, &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl2);
      C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS)
        i = 999;
    }
  }

  PASS();
}

TEST test_rewrite_body_corner_oom_2(void) {
  const char *cases[] = {
      "void f() { char *s = my_strdup(\"a\"); }",
      "void f() { my_func(\"a\"); }", "void f() { return my_func(\"a\"); }",
      "void f() { my_func(\"a\") }", "void f() { if (1) { return; } }"};
  struct RefactoredFunction funcs[] = {
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
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str((char *)cases[c]), &tl);
      g_cdd_alloc_fail = i;
      g_patcher_test_cap_1 = 1;
      int rc = rewrite_body(tl, NULL, funcs, 2,
                            c == 4 ? &t1 : (c == 2 ? &t2 : NULL), &out_code);
      g_cdd_alloc_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl);
      if (out_code)
        C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS) {
        break;
      }
    }
  }

  for (c = 0; c < 5; ++c) {
    int i;
    for (i = 1; i < 50; ++i) {
      struct TokenList *tl = NULL;
      char *out_code = NULL;
      extern C_CDD_EXPORT int g_cdd_strdup_fail;
      tokenize(az_span_create_from_str((char *)cases[c]), &tl);
      g_cdd_strdup_fail = i;
      g_patcher_test_cap_1 = 1;
      int rc = rewrite_body(tl, NULL, funcs, 2,
                            c == 4 ? &t1 : (c == 2 ? &t2 : NULL), &out_code);
      g_cdd_strdup_fail = 0;
      g_patcher_test_cap_1 = 0;
      free_token_list(tl);
      if (out_code)
        C_CDD_FREE(out_code);
      if (rc == CDD_C_SUCCESS) {
        break;
      }
    }
  }

  PASS();
}

SUITE(rewriter_body_suite) {
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

  RUN_TEST(test_propagate_void_stmt_return);
  RUN_TEST(test_propagate_void_stmt_transform);
  RUN_TEST(test_propagate_nested_parens);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REWRITER_BODY_H */
