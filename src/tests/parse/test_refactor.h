/**
 * @file test_refactor.h
 * @brief Unit tests for refactoring orchestration.
 */

#ifndef TEST_REFACTOR_H
#define TEST_REFACTOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <errno.h>
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/refactor.h"
/* clang-format on */

/**
 * @brief Tests the lifecycle of RefactorContext.
 *
 * @return The result of the test.
 */
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

/**
 * @brief Tests basic application of refactoring to a string.
 *
 * @return The result of the test.
 */
TEST test_apply_refactoring_to_string_basic(void) {
  struct RefactorContext ctx;
  int rc;
  const char *src = ""
                    "void my_func() { char * p = (char *)malloc(1); }";
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

/**
 * @brief Tests error handling of apply_refactoring_to_string.
 *
 * @return The result of the test.
 */
TEST test_apply_refactoring_to_string_errors(void) {
  struct RefactorContext ctx;
  char *out = NULL;

  refactor_context_init(&ctx);

  ASSERT_EQ(EINVAL, apply_refactoring_to_string(NULL, "int main() {}", &out));
  ASSERT_EQ(EINVAL, apply_refactoring_to_string(&ctx, NULL, &out));
  /* Tokenize failure. Unclosed string literal is actually not an error in our
   * lexer, but unclosed block comment might be? Wait, what fails tokenizer? */
  /* Actually, let's just make it return error by using ENOMEM if we could mock
     it. Instead, since we can't easily mock, let's skip testing
     `find_allocations` failure branch unless we can force it. Let's check if we
     can pass a malformed AST? No, this function tokenizes it itself. */

  refactor_context_free(&ctx);
  PASS();
}

/**
 * @brief Refactor test suite.
 */
SUITE(refactor_suite) {
  RUN_TEST(test_refactor_context_lifecycle);
  RUN_TEST(test_apply_refactoring_to_string_basic);
  RUN_TEST(test_apply_refactoring_to_string_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REFACTOR_H */
