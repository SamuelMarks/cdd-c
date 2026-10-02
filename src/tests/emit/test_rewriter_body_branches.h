/**
 * @file test_rewriter_body_branches.h
 * @brief Unit tests for rewriter body all branches and coverage.
 */

#ifndef TEST_REWRITER_BODY_BRANCHES_H
#define TEST_REWRITER_BODY_BRANCHES_H

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

TEST test_rewrite_body_all_branches(void) {
#ifdef CDD_BUILD_TESTS
  struct RefactoredFunction funcs[2];
  struct SignatureTransform t_void;
  struct SignatureTransform t_ret;
  struct SignatureTransform t_unk;
  struct AllocationSiteList allocs;
  struct TokenList *tl = NULL;
  char *out_code = NULL;
  cdd_c_error_t rc = 0;
  extern C_CDD_EXPORT int g_cdd_fail_find_stmt_start;

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

TEST test_run_body_rewrite_coverage(void) {
  char *out = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            run_body_rewrite(NULL, NULL, 0, NULL, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            run_body_rewrite("void f();", NULL, 0, NULL, NULL));
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REWRITER_BODY_BRANCHES_H */
