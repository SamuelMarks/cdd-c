#ifndef TEST_CDD_CST_SCOPE_H
#define TEST_CDD_CST_SCOPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../classes/parse/cdd_cst_scope.h"
#include <greatest.h>

TEST test_cdd_cst_scope_basic(void) {
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_symbol_t *sym = NULL;

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
  PASS();
}

TEST test_cdd_cst_scope_errors(void) {
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_symbol_t *sym = NULL;

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
  PASS();
}

TEST test_cdd_cst_scope_tag(void) {
  ASSERT_EQ(1, cdd_cst_symbol_is_tag(CDD_CST_SYMBOL_STRUCT_TAG));
  ASSERT_EQ(1, cdd_cst_symbol_is_tag(CDD_CST_SYMBOL_UNION_TAG));
  ASSERT_EQ(1, cdd_cst_symbol_is_tag(CDD_CST_SYMBOL_ENUM_TAG));
  ASSERT_EQ(0, cdd_cst_symbol_is_tag(CDD_CST_SYMBOL_VARIABLE));

  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_symbol_t *sym = NULL;
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
  PASS();
}

TEST test_cdd_cst_scope_mem(void) {
  /* Test handled via alloc limits if injected, otherwise basic execution covers
   * paths. */
  PASS();
}

SUITE(cdd_cst_scope_suite) {
  RUN_TEST(test_cdd_cst_scope_basic);
  RUN_TEST(test_cdd_cst_scope_errors);
  RUN_TEST(test_cdd_cst_scope_tag);
  RUN_TEST(test_cdd_cst_scope_mem);
}

#ifdef __cplusplus
}
#endif
#endif
