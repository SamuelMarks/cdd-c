#ifndef TEST_REWRITER_BODY_H
#define TEST_REWRITER_BODY_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/rewriter_body.h"
#include "functions/parse/analysis.h"
#include "functions/parse/tokenizer.h"

static int run_body_rewrite(const char *code,
                            const struct RefactoredFunction *funcs,
                            size_t n_funcs,
                            const struct SignatureTransform *transform,
                            char **out) {
  struct TokenList *tl = NULL;
  struct AllocationSiteList sites = {0};
  int rc;
  const az_span source = az_span_create_from_str((char *)code);

  if (tokenize(source, &tl) != 0)
    return -1;

  if (find_allocations(tl, &sites) != 0) {
    free_token_list(tl);
    return -2;
  }

  rc = rewrite_body(tl, &sites, funcs, n_funcs, transform, out);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  return rc;
}

/* --- Call-Site Propagation Tests --- */

TEST test_propagate_void_stmt(void) {
  const char *input = "void f() { do_work(); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"do_work", REF_VOID_TO_INT, NULL}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "int rc = 0;") != NULL);
  ASSERT(strstr(output, "rc = do_work(); if (rc != 0) return rc;") != NULL);

  free(output);
  PASS();
}

TEST test_propagate_ptr_assignment(void) {
  const char *input = "void f() { char *s; s = my_strdup(\"a\"); }";
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
  PASS();
}

TEST test_propagate_ptr_declaration(void) {
  const char *input = "void f() { char *s = my_strdup(\"a\"); }";
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
  PASS();
}

TEST test_propagate_nested_hoisting(void) {
  const char *input = "void f() { outer(inner(\"x\")); }";
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
  PASS();
}

/* --- Safety Tests (Repeat from Deliv 2 for Integration Check) --- */

TEST test_integration_safety_and_prop(void) {
  const char *input =
      "void f() { char *p = malloc(10); if(!p) return; do_work(); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"do_work", REF_VOID_TO_INT, NULL}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "int rc = 0;") != NULL);
  /* Malloc analysis finding check so no injection */
  /* do_work rewritten */
  ASSERT(strstr(output, "rc = do_work();") != NULL);

  free(output);
  PASS();
}

TEST test_realloc_safety_injection(void) {
  const char *input = "void f() { char *p; p = realloc(p, 100); }";
  char *output = NULL;
  int rc;

  rc = run_body_rewrite(input, NULL, 0, NULL, &output);
  ASSERT_EQ(0, rc);

  /* Should rewrite: p = realloc(p, 100); ->
     { void *_safe_tmp = realloc(p, 100); if (!_safe_tmp) return ENOMEM; p =
     _safe_tmp; } */
  ASSERT(strstr(output, "void *_safe_tmp = realloc(p, 100);") != NULL);
  ASSERT(strstr(output, "if (!_safe_tmp) return ENOMEM;") != NULL);
  ASSERT(strstr(output, "p = _safe_tmp;") != NULL);

  free(output);
  PASS();
}

SUITE(rewriter_body_suite) {
  RUN_TEST(test_propagate_void_stmt);
  RUN_TEST(test_propagate_ptr_assignment);
  RUN_TEST(test_propagate_ptr_declaration);
  RUN_TEST(test_propagate_nested_hoisting);
  RUN_TEST(test_integration_safety_and_prop);
  RUN_TEST(test_realloc_safety_injection);
}

#endif /* TEST_REWRITER_BODY_H */
