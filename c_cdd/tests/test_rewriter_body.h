#ifndef TEST_REWRITER_BODY_H
#define TEST_REWRITER_BODY_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "analysis.h"
#include "rewriter_body.h"
#include "tokenizer.h"

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

TEST test_inject_malloc_check(void) {
  const char *input = "void f() { char *p = malloc(10); *p = 5; }";
  char *output = NULL;
  int rc;

  rc = run_body_rewrite(input, NULL, 0, NULL, &output);
  ASSERT_EQ(0, rc);
  ASSERT(output != NULL);

  ASSERT(strstr(output, "malloc(10); if (!p) { return ENOMEM; }") != NULL);

  free(output);
  PASS();
}

TEST test_skipped_checked_malloc(void) {
  const char *input = "void f() { char *p = malloc(10); if (!p) return; }";
  char *output = NULL;
  int rc;

  rc = run_body_rewrite(input, NULL, 0, NULL, &output);
  ASSERT_EQ(0, rc);

  {
    char *p = output;
    int count = 0;
    while ((p = strstr(p, "if"))) {
      count++;
      p++;
    }
    ASSERT_EQ(1, count);
  }

  free(output);
  PASS();
}

TEST test_rewrite_void_call_with_stack_injection(void) {
  const char *input = "void f() { do_something(1, 2); return; }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"do_something", REF_VOID_TO_INT}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  /* Matches injected var and propagated check */
  ASSERT(strstr(output, "int rc = 0;") != NULL);
  ASSERT(strstr(output, "rc = do_something(1, 2); if (rc != 0) return rc;") !=
         NULL);

  free(output);
  PASS();
}

TEST test_rewrite_ptr_call_assignment_stack_inject(void) {
  const char *input = "void f() { char *s; s = strdup(\"a\"); free(s); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"strdup", REF_PTR_TO_INT_OUT}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "int rc = 0;") != NULL);
  ASSERT(strstr(output, "rc = strdup(\"a\", &s); if (rc != 0) return rc;") !=
         NULL);

  free(output);
  PASS();
}

TEST test_rewrite_ptr_call_declaration_stack_inject(void) {
  const char *input = "void f() { char *s = strdup(\"a\"); free(s); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"strdup", REF_PTR_TO_INT_OUT}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, NULL, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "int rc = 0;") != NULL);
  /* Checks declaration split: 'char *s; rc = ...' */
  ASSERT(strstr(output, "char *s ; rc = strdup(\"a\", &s);") != NULL);
  ASSERT(strstr(output, "if (rc != 0) return rc;") != NULL);

  free(output);
  PASS();
}

TEST test_rewrite_return_void_to_int(void) {
  const char *input = "void f() { do_work(); return; }";
  char *output = NULL;
  struct SignatureTransform trans = {TRANSFORM_VOID_TO_INT, NULL, "0", NULL};
  int rc;

  rc = run_body_rewrite(input, NULL, 0, &trans, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "return 0;") != NULL);

  free(output);
  PASS();
}

TEST test_rewrite_return_val_to_arg(void) {
  const char *input = "char* f() { return strdup(\"x\"); }";
  char *output = NULL;
  struct SignatureTransform trans = {TRANSFORM_RET_PTR_TO_ARG, "out", "0",
                                     "ENOMEM"};
  int rc;

  rc = run_body_rewrite(input, NULL, 0, &trans, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "{ *out = strdup(\"x\"); return 0; }") != NULL);

  free(output);
  PASS();
}

TEST test_rewrite_return_null_error(void) {
  const char *input = "char* f() { return NULL; }";
  char *output = NULL;
  struct SignatureTransform trans = {TRANSFORM_RET_PTR_TO_ARG, "out", "0",
                                     "ENOMEM"};
  int rc;

  rc = run_body_rewrite(input, NULL, 0, &trans, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "return ENOMEM;") != NULL);

  free(output);
  PASS();
}

TEST test_rewrite_body_null_args(void) {
  ASSERT_EQ(EINVAL, rewrite_body(NULL, NULL, NULL, 0, NULL, NULL));
  PASS();
}

SUITE(rewriter_body_suite) {
  RUN_TEST(test_rewrite_body_null_args);
  RUN_TEST(test_inject_malloc_check);
  RUN_TEST(test_skipped_checked_malloc);
  RUN_TEST(test_rewrite_void_call_with_stack_injection);
  RUN_TEST(test_rewrite_ptr_call_assignment_stack_inject);
  RUN_TEST(test_rewrite_ptr_call_declaration_stack_inject);
  RUN_TEST(test_rewrite_return_void_to_int);
  RUN_TEST(test_rewrite_return_val_to_arg);
  RUN_TEST(test_rewrite_return_null_error);
}

#endif /* TEST_REWRITER_BODY_H */
