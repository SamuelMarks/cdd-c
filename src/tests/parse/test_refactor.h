#ifndef TEST_REFACTOR_H
#define TEST_REFACTOR_H

#include <errno.h>
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/refactor.h"

TEST test_refactor_context_lifecycle(void) {
  struct RefactorContext ctx;

  ASSERT_EQ(EINVAL, refactor_context_init(NULL));
  ASSERT_EQ(0, refactor_context_init(&ctx));

  ASSERT_EQ(EINVAL, refactor_context_add_function(NULL, "func", REF_VOID_TO_INT,
                                                  "void"));
  ASSERT_EQ(EINVAL,
            refactor_context_add_function(&ctx, NULL, REF_VOID_TO_INT, "void"));

  ASSERT_EQ(0, refactor_context_add_function(&ctx, "my_func", REF_VOID_TO_INT,
                                             "void"));
  ASSERT_EQ(1, ctx.func_count);
  ASSERT_STR_EQ("my_func", ctx.funcs[0].name);

  refactor_context_free(&ctx);
  refactor_context_free(NULL); /* Safe */

  PASS();
}

TEST test_apply_refactoring_to_string_basic(void) {
  struct RefactorContext ctx;
  int rc;
  const char *src = "void my_func() { char *p = malloc(1); }";
  char *out = NULL;

  refactor_context_init(&ctx);
  refactor_context_add_function(&ctx, "my_func", REF_VOID_TO_INT, "void");

  rc = apply_refactoring_to_string(&ctx, src, &out);
  ASSERT_EQ(0, rc);
  ASSERT(out != NULL);

  free(out);
  refactor_context_free(&ctx);
  PASS();
}

TEST test_apply_refactoring_to_string_errors(void) {

  /* Bad syntax: Unclosed comment fails tokenization only if strictly checked,
   * but a dangling quote works */
  /* We will just use an unclosed string literal */
  ASSERT_NEQ(0, apply_refactoring_to_string(NULL, NULL, NULL));

  PASS();
}

SUITE(refactor_suite) {
  RUN_TEST(test_refactor_context_lifecycle);
  RUN_TEST(test_apply_refactoring_to_string_basic);
  RUN_TEST(test_apply_refactoring_to_string_errors);
}

#endif /* TEST_REFACTOR_H */
