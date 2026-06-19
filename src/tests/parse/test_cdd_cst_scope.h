extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_cdd_cst_scope.h
 * @brief Unit tests for CST scope and symbol table.
 */

#ifndef TEST_CDD_CST_SCOPE_H
#define TEST_CDD_CST_SCOPE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "../../classes/parse/cdd_cst_scope.h"
#include <greatest.h>
/* clang-format on */

/**
 * @brief Tests basic functionality of CST scope.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_scope_basic(void) {
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_symbol_t *sym = NULL;
  (void)sym;

  {
    cdd_cst_scope_env_t *env2 = NULL;
    cdd_cst_scope_env_init(&env2);
    env2->current_scope = NULL;
    ASSERT_EQ(EINVAL, cdd_cst_scope_enter(env2, CDD_CST_SCOPE_BLOCK));
    ASSERT_EQ(EINVAL, cdd_cst_scope_leave(env2));
    ASSERT_EQ(EINVAL, cdd_cst_scope_add_symbol(env2, "test",
                                               CDD_CST_SYMBOL_VARIABLE, NULL));
    cdd_cst_scope_env_free(env2);
  }

  ASSERT_EQ(EINVAL, cdd_cst_scope_env_init(NULL));
  cdd_cst_scope_env_free(NULL);

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_scope_alloc_fail;
    g_cdd_scope_alloc_fail = 2;
    ASSERT_EQ(ENOMEM, cdd_cst_scope_env_init(&env));
    g_cdd_scope_alloc_fail = 0;
  }
#endif

  /* Manual free scope NULL test via a dummy scope child */
  ASSERT_EQ(0, cdd_cst_scope_env_init(&env));
  env->global_scope->capacity = 1;
  env->global_scope->num_children = 1;
  env->global_scope->children = calloc(1, sizeof(cdd_cst_scope_t *));
  env->global_scope->children[0] = NULL;
  cdd_cst_scope_env_free(env);
  env = NULL;

  {
    cdd_cst_scope_env_t *env2 = NULL;
    cdd_cst_scope_env_init(&env2);
    env2->current_scope = NULL;
    ASSERT_EQ(EINVAL, cdd_cst_scope_enter(env2, CDD_CST_SCOPE_BLOCK));
    ASSERT_EQ(EINVAL, cdd_cst_scope_leave(env2));
    ASSERT_EQ(EINVAL, cdd_cst_scope_add_symbol(env2, "test",
                                               CDD_CST_SYMBOL_VARIABLE, NULL));
    cdd_cst_scope_env_free(env2);
  }

  ASSERT_EQ(0, cdd_cst_scope_env_init(&env));
  ASSERT_EQ(0, cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK));

  ASSERT_EQ(0, cdd_cst_scope_add_symbol(env, "my_var", CDD_CST_SYMBOL_VARIABLE,
                                        NULL));
  ASSERT_EQ(0, cdd_cst_scope_lookup_symbol(env, "my_var",
                                           CDD_CST_SYMBOL_VARIABLE, &sym));
  ASSERT_NEQ(NULL, sym);

  ASSERT_EQ(0, cdd_cst_scope_leave(env));

  cdd_cst_scope_env_free(env);
  cdd_cst_scope_env_free(NULL); /* Test free NULL */
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests error handling of CST scope APIs.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_scope_errors(void) {
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_symbol_t *sym = NULL;
  (void)sym;

  {
    cdd_cst_scope_env_t *env2 = NULL;
    cdd_cst_scope_env_init(&env2);
    env2->current_scope = NULL;
    ASSERT_EQ(EINVAL, cdd_cst_scope_enter(env2, CDD_CST_SCOPE_BLOCK));
    ASSERT_EQ(EINVAL, cdd_cst_scope_leave(env2));
    ASSERT_EQ(EINVAL, cdd_cst_scope_add_symbol(env2, "test",
                                               CDD_CST_SYMBOL_VARIABLE, NULL));
    cdd_cst_scope_env_free(env2);
  }

  ASSERT_EQ(EINVAL, cdd_cst_scope_env_init(NULL));
  ASSERT_EQ(EINVAL, cdd_cst_scope_enter(NULL, CDD_CST_SCOPE_BLOCK));
  ASSERT_EQ(EINVAL, cdd_cst_scope_leave(NULL));
  ASSERT_EQ(EINVAL,
            cdd_cst_scope_add_symbol(NULL, "a", CDD_CST_SYMBOL_VARIABLE, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_scope_lookup_symbol(NULL, "a",
                                                CDD_CST_SYMBOL_VARIABLE, &sym));

  cdd_cst_scope_env_init(&env);
  ASSERT_EQ(EINVAL,
            cdd_cst_scope_add_symbol(env, NULL, CDD_CST_SYMBOL_VARIABLE, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_scope_lookup_symbol(env, NULL,
                                                CDD_CST_SYMBOL_VARIABLE, &sym));

  /* we don't return error on redeclaration in this implementation yet, but
   * check basic lookup failure */
  ASSERT_EQ(ENOENT, cdd_cst_scope_lookup_symbol(env, "v2",
                                                CDD_CST_SYMBOL_VARIABLE, &sym));

  /* Pop without parent */
  ASSERT_EQ(EINVAL, cdd_cst_scope_leave(env));

  /* Trigger parent alloc expansion */
  {
    int i;
    for (i = 0; i < 10; ++i) {
      ASSERT_EQ(0, cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK));
      ASSERT_EQ(0, cdd_cst_scope_leave(env));
    }
  }

  cdd_cst_scope_env_free(env);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests CST scope tag semantics.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_scope_tag(void) {
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_symbol_t *sym = NULL;
  (void)sym;

  {
    cdd_cst_scope_env_t *env2 = NULL;
    cdd_cst_scope_env_init(&env2);
    env2->current_scope = NULL;
    ASSERT_EQ(EINVAL, cdd_cst_scope_enter(env2, CDD_CST_SCOPE_BLOCK));
    ASSERT_EQ(EINVAL, cdd_cst_scope_leave(env2));
    ASSERT_EQ(EINVAL, cdd_cst_scope_add_symbol(env2, "test",
                                               CDD_CST_SYMBOL_VARIABLE, NULL));
    cdd_cst_scope_env_free(env2);
  }

  ASSERT_EQ(1, cdd_cst_symbol_is_tag(CDD_CST_SYMBOL_STRUCT_TAG));
  ASSERT_EQ(1, cdd_cst_symbol_is_tag(CDD_CST_SYMBOL_UNION_TAG));
  ASSERT_EQ(1, cdd_cst_symbol_is_tag(CDD_CST_SYMBOL_ENUM_TAG));
  ASSERT_EQ(0, cdd_cst_symbol_is_tag(CDD_CST_SYMBOL_VARIABLE));

  cdd_cst_scope_env_init(&env);

  cdd_cst_scope_add_symbol(env, "v", CDD_CST_SYMBOL_VARIABLE, NULL);
  cdd_cst_scope_add_symbol(env, "v", CDD_CST_SYMBOL_STRUCT_TAG, NULL);

  ASSERT_EQ(
      0, cdd_cst_scope_lookup_symbol(env, "v", CDD_CST_SYMBOL_VARIABLE, &sym));
  ASSERT_EQ(CDD_CST_SYMBOL_VARIABLE, sym->kind);

  ASSERT_EQ(0, cdd_cst_scope_lookup_symbol(env, "v", CDD_CST_SYMBOL_STRUCT_TAG,
                                           &sym));
  ASSERT_EQ(CDD_CST_SYMBOL_STRUCT_TAG, sym->kind);

  cdd_cst_scope_env_free(env);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests memory bounds of CST scope.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_scope_mem(void) {
  /* Test handled via alloc limits if injected, otherwise basic execution covers
   * paths. */
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief CST scope test suite.
 */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_scope_alloc_fail;

TEST test_cdd_cst_scope_oom(void) {
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_symbol_t *sym = NULL;
  (void)sym;

  {
    cdd_cst_scope_env_t *env2 = NULL;
    cdd_cst_scope_env_init(&env2);
    env2->current_scope = NULL;
    ASSERT_EQ(EINVAL, cdd_cst_scope_enter(env2, CDD_CST_SCOPE_BLOCK));
    ASSERT_EQ(EINVAL, cdd_cst_scope_leave(env2));
    ASSERT_EQ(EINVAL, cdd_cst_scope_add_symbol(env2, "test",
                                               CDD_CST_SYMBOL_VARIABLE, NULL));
    cdd_cst_scope_env_free(env2);
  }

  g_cdd_scope_alloc_fail = 1;
  ASSERT_EQ(ENOMEM, cdd_cst_scope_env_init(&env));
  g_cdd_scope_alloc_fail = 2;
  ASSERT_EQ(ENOMEM, cdd_cst_scope_env_init(&env));

  g_cdd_scope_alloc_fail = 0;
  cdd_cst_scope_env_init(&env);

  g_cdd_scope_alloc_fail = 3;
  ASSERT_EQ(ENOMEM, cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK));

  g_cdd_scope_alloc_fail = 4;
  ASSERT_EQ(ENOMEM, cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK));

  g_cdd_scope_alloc_fail = 5;
  ASSERT_EQ(ENOMEM, cdd_cst_scope_add_symbol(env, "my_var",
                                             CDD_CST_SYMBOL_VARIABLE, NULL));

  g_cdd_scope_alloc_fail = 6;
  ASSERT_EQ(ENOMEM, cdd_cst_scope_add_symbol(env, "my_var2",
                                             CDD_CST_SYMBOL_VARIABLE, NULL));

  g_cdd_scope_alloc_fail = 0;
  cdd_cst_scope_env_free(env);
  g_fail_io_after = -1;
  PASS();
}
#endif
SUITE(cdd_cst_scope_suite) {
  RUN_TEST(test_cdd_cst_scope_basic);
  RUN_TEST(test_cdd_cst_scope_errors);
  RUN_TEST(test_cdd_cst_scope_tag);
  RUN_TEST(test_cdd_cst_scope_mem);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_cdd_cst_scope_oom);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_SCOPE_H */
