#ifndef TEST_REWRITER_BODY_H
#define TEST_REWRITER_BODY_H

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

static enum cdd_c_error
run_body_rewrite(const char *code, const struct RefactoredFunction *funcs,
                 size_t n_funcs, const struct SignatureTransform *transform,
                 char **out) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  int rc;
  const az_span source = az_span_create_from_str((char *)code);

  if (!code || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (tokenize(source, &tl) != 0)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  if (find_allocations(tl, &sites) != 0) {
    /* LCOV_EXCL_START */
    free_token_list(tl);
    return CDD_C_ERROR_PARSE;
    /* LCOV_EXCL_STOP */
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
  ASSERT_EQ(0, rc);

  printf("OUTPUT: %s\n", output);
  ASSERT(strstr(output, "enum cdd_c_error rc = CDD_C_SUCCESS;") != NULL);
  ASSERT(strstr(output, "rc = do_work(); if (rc != 0) return rc;") != NULL);

  free(output);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_propagate_ptr_assignment
 * @return TEST
 */
TEST test_propagate_ptr_assignment(void) {
  const char *input = ""
                      "void f() { char *s; s = my_strdup(\"a\"); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {
      {"my_strdup", REF_PTR_TO_INT_OUT, "char *"}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  /* s = my_strdup("a") -> rc = my_strdup("a", &s); if(rc) ... */
  ASSERT(strstr(output, "rc = my_strdup(\"a\", &s);") != NULL);
  ASSERT(strstr(output, "if (rc != 0) return rc;") != NULL);

  free(output);
  g_fail_io_after = -1;
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
  ASSERT(strstr(output, "enum cdd_c_error rc = CDD_C_SUCCESS;") != NULL);
  /* Malloc analysis finding check so no injection */
  /* do_work rewritten */
  ASSERT(strstr(output, "rc = do_work();") != NULL);

  free(output);
  g_fail_io_after = -1;
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
  PASS();
}

SUITE(rewriter_body_suite) {
  RUN_TEST(test_rewriter_body_bounds);
  RUN_TEST(test_propagate_void_stmt);
  RUN_TEST(test_propagate_ptr_assignment);
  RUN_TEST(test_propagate_ptr_declaration);
  RUN_TEST(test_propagate_nested_hoisting);
  RUN_TEST(test_integration_safety_and_prop);
  RUN_TEST(test_realloc_safety_injection);
  RUN_TEST(test_rewriter_body_bounds2);
  RUN_TEST(test_rewriter_body_oom);

  RUN_TEST(test_propagate_void_stmt_return);
  RUN_TEST(test_propagate_void_stmt_transform);
  RUN_TEST(test_propagate_nested_parens);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REWRITER_BODY_H */
