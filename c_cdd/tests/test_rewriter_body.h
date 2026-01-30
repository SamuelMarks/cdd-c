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
                            size_t n_funcs, char **out) {
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

  rc = rewrite_body(tl, &sites, funcs, n_funcs, out);

  allocation_site_list_free(&sites);
  free_token_list(tl);
  return rc;
}

TEST test_inject_malloc_check(void) {
  const char *input = "void f() { char *p = malloc(10); *p = 5; }";
  char *output = NULL;
  int rc;

  rc = run_body_rewrite(input, NULL, 0, &output);
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

  rc = run_body_rewrite(input, NULL, 0, &output);
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

TEST test_rewrite_void_call(void) {
  const char *input = "void f() { do_something(1, 2); return; }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"do_something", REF_VOID_TO_INT}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "if (do_something(1, 2) != 0) return EIO;") != NULL);

  free(output);
  PASS();
}

TEST test_rewrite_ptr_call(void) {
  const char *input = "void f() { char *s = strdup(\"a\"); free(s); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"strdup", REF_PTR_TO_INT_OUT}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, &output);
  ASSERT_EQ(0, rc);

  /* Expect: char *s; if (strdup("a", &s) != 0) return EIO; */
  /* We check usage of the injected semicolon. */
  ASSERT(strstr(output, "char *s ; if (strdup(\"a\", &s) != 0) return EIO;") !=
         NULL);

  free(output);
  PASS();
}

TEST test_rewrite_ptr_call_spaces(void) {
  const char *input = "void f() { char *s; s = get_str(); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"get_str", REF_PTR_TO_INT_OUT}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, &output);
  ASSERT_EQ(0, rc);

  /* Expect: s ; if (get_str(&s) != 0) return EIO; */
  ASSERT(strstr(output, "s ; if (get_str(&s) != 0) return EIO;") != NULL);

  free(output);
  PASS();
}

TEST test_mix_alloc_and_rewrite(void) {
  const char *input = "void f() { char *p = malloc(10); func(); }";
  char *output = NULL;
  struct RefactoredFunction funcs[] = {{"func", REF_VOID_TO_INT}};
  int rc;

  rc = run_body_rewrite(input, funcs, 1, &output);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(output, "malloc(10); if (!p) { return ENOMEM; }") != NULL);
  ASSERT(strstr(output, "if (func() != 0) return EIO;") != NULL);

  free(output);
  PASS();
}

TEST test_rewrite_body_null_args(void) {
  ASSERT_EQ(EINVAL, rewrite_body(NULL, NULL, NULL, 0, NULL));
  PASS();
}

SUITE(rewriter_body_suite) {
  RUN_TEST(test_rewrite_body_null_args);
  RUN_TEST(test_inject_malloc_check);
  RUN_TEST(test_skipped_checked_malloc);
  RUN_TEST(test_rewrite_void_call);
  RUN_TEST(test_rewrite_ptr_call);
  RUN_TEST(test_rewrite_ptr_call_spaces);
  RUN_TEST(test_mix_alloc_and_rewrite);
}

#endif /* TEST_REWRITER_BODY_H */
