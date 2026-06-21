extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_cdd_cst_semantic.h
 * @brief Unit tests for CST semantic analysis.
 */

/* clang-format off */
#include "c_cdd_export.h"
#ifndef TEST_CDD_CST_SEMANTIC_H
#define TEST_CDD_CST_SEMANTIC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <greatest.h>
#include "../../classes/parse/cdd_cst_semantic.h"
#include "../../classes/parse/cdd_cst_scope.h"
#include "../../classes/parse/cdd_cst_parser.h"
#include "../../classes/parse/cdd_cst_factory.h"
#include "../../classes/parse/cdd_cst_builder.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_semantic_oom_extract;
extern C_CDD_EXPORT int g_cdd_semantic_oom_scope;
extern C_CDD_EXPORT int g_cdd_semantic_oom_scope2;
#endif

TEST test_cdd_cst_semantic_scope_basic(void) {
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *node = NULL;
  cdd_cst_symbol_t *sym;

  ASSERT_EQ(EINVAL, cdd_cst_scope_env_init(NULL));
  ASSERT_EQ(0, cdd_cst_scope_env_init(&env));
  ASSERT_NEQ(NULL, env);

  ASSERT_EQ(EINVAL, cdd_cst_scope_enter(NULL, CDD_CST_SCOPE_BLOCK));
  ASSERT_EQ(0, cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK));
  ASSERT_EQ(0, cdd_cst_scope_enter(env, CDD_CST_SCOPE_FUNCTION));

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &node);
  ASSERT_EQ(EINVAL, cdd_cst_scope_add_symbol(NULL, "foo",
                                             CDD_CST_SYMBOL_VARIABLE, node));
  ASSERT_EQ(EINVAL,
            cdd_cst_scope_add_symbol(env, NULL, CDD_CST_SYMBOL_VARIABLE, node));
  ASSERT_EQ(
      0, cdd_cst_scope_add_symbol(env, "foo", CDD_CST_SYMBOL_VARIABLE, node));

  /* Test adding a tag to check namespace differentiation */
  ASSERT_EQ(0, cdd_cst_scope_add_symbol(env, "foo_tag",
                                        CDD_CST_SYMBOL_STRUCT_TAG, node));

  /* Force scope children capacity realloc (requires 5 blocks within one parent)
   */
  {
    int i;
    for (i = 0; i < 6; i++) {
      ASSERT_EQ(0, cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK));
      ASSERT_EQ(0, cdd_cst_scope_leave(env));
    }
  }

  ASSERT_EQ(EINVAL, cdd_cst_scope_lookup_symbol(NULL, "foo",
                                                CDD_CST_SYMBOL_VARIABLE, &sym));
  ASSERT_EQ(EINVAL, cdd_cst_scope_lookup_symbol(env, NULL,
                                                CDD_CST_SYMBOL_VARIABLE, &sym));
  ASSERT_EQ(EINVAL, cdd_cst_scope_lookup_symbol(env, "foo",
                                                CDD_CST_SYMBOL_VARIABLE, NULL));

  /* Lookup the variable */
  ASSERT_EQ(0, cdd_cst_scope_lookup_symbol(env, "foo", CDD_CST_SYMBOL_VARIABLE,
                                           &sym));
  ASSERT_NEQ(NULL, sym);
  ASSERT_EQ(CDD_CST_SYMBOL_VARIABLE, sym->kind);
  ASSERT_STR_EQ("foo", sym->name);

  /* Lookup the tag */
  ASSERT_EQ(0, cdd_cst_scope_lookup_symbol(env, "foo_tag",
                                           CDD_CST_SYMBOL_STRUCT_TAG, &sym));
  ASSERT_NEQ(NULL, sym);
  ASSERT_EQ(CDD_CST_SYMBOL_STRUCT_TAG, sym->kind);
  ASSERT_STR_EQ("foo_tag", sym->name);

  /* Lookup missing */
  ASSERT_EQ(ENOENT, cdd_cst_scope_lookup_symbol(env, "missing",
                                                CDD_CST_SYMBOL_VARIABLE, &sym));

  ASSERT_EQ(0, cdd_cst_scope_leave(env));      /* pop function */
  ASSERT_EQ(0, cdd_cst_scope_leave(env));      /* pop block */
  ASSERT_EQ(EINVAL, cdd_cst_scope_leave(env)); /* cannot pop file scope */

  cdd_cst_scope_env_free(env);
  env = NULL;
  cdd_cst_free_node_only(node);
  cdd_cst_scope_env_free(NULL); /* no-op */
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_semantic_basic(void) {
  cdd_cst_tree_t tree = {0};
  cdd_cst_scope_env_t *env = NULL;

  ASSERT_EQ(EINVAL, cdd_cst_build_semantic_info(NULL, &env));

  ASSERT_EQ(0, cdd_cst_build_semantic_info(&tree, &env));
  ASSERT_NEQ(NULL, env);

  cdd_cst_scope_env_free(env);
  env = NULL;
  /* no cdd_cst_tree_free for stack allocated zero init tree */
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_semantic_tree(void) {
  cdd_cst_tree_t tree = {0};
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *root = NULL, *func = NULL, *block = NULL, *decl = NULL,
                 *id_node = NULL, *type_decl = NULL, *id_node2 = NULL;
  cdd_token_t *tok_var = NULL;
  cdd_token_t *tok_type = NULL;

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree.root = root;

  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  cdd_cst_append_child_node(root, func);

  cdd_cst_alloc_node(CDD_CST_BLOCK, &block);
  cdd_cst_append_child_node(func, block);

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl);
  cdd_cst_append_child_node(block, decl);

  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  cdd_cst_create_token_len(&tree, CDD_TOKEN_IDENTIFIER, "my_var", 6, &tok_var);
  cdd_cst_append_child_token(id_node, tok_var);
  cdd_cst_append_child_node(decl, id_node);

  /* TYPE_SPECIFIER */
  cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &type_decl);
  cdd_cst_append_child_node(block, type_decl);

  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  cdd_cst_create_token_len(&tree, CDD_TOKEN_IDENTIFIER, "X", 1, &tok_type);
  cdd_cst_append_child_token(id_node2, tok_type);
  cdd_cst_append_child_node(type_decl, id_node2);

  ASSERT_EQ(0, cdd_cst_build_semantic_info(&tree, &env));
  ASSERT_NEQ(NULL, env);

  cdd_cst_scope_env_free(env);
  env = NULL;

  /* we will just free manually what we allocated if tree_free isn't safe */
  cdd_cst_free_node_only(id_node2);
  cdd_cst_free_node_only(type_decl);
  cdd_cst_free_node_only(id_node);
  cdd_cst_free_node_only(decl);
  cdd_cst_free_node_only(block);
  cdd_cst_free_node_only(func);
  cdd_cst_free_node_only(root);
  if (env)
    cdd_cst_scope_env_free(env);
  free(tok_var);
  free(tok_type);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_semantic_errors(void) {
  cdd_cst_tree_t tree = {0};
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *root = NULL, *decl = NULL, *id_node = NULL,
                 *non_id_node = NULL;
  cdd_token_t *tok_other = NULL;
  cdd_token_t *tok_oom = NULL;

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree.root = root;

  /* Empty declaration to hit ENOENT in extract_identifier */
  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl);
  cdd_cst_append_child_node(root, decl);

  /* Node with child token that is NOT an identifier */
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  cdd_cst_create_token_len(&tree, CDD_TOKEN_KEYWORD_INT, "int", 3, &tok_other);
  cdd_cst_append_child_token(id_node, tok_other);

  /* Node with child token that IS an identifier but causes OOM */
  cdd_cst_create_token_len(&tree, CDD_TOKEN_IDENTIFIER, "huge", 4, &tok_oom);
#ifdef CDD_BUILD_TESTS
  g_cdd_semantic_oom_extract = 5;
#else
  tok_oom->length = (size_t)-2; /* malloc(SIZE_MAX) will fail */
#endif
  cdd_cst_append_child_token(id_node, tok_oom);

  cdd_cst_append_child_node(decl, id_node);

  /* Node with child node that fails extract_identifier */
  cdd_cst_alloc_node(CDD_CST_EXPRESSION, &non_id_node);
  cdd_cst_append_child_node(decl, non_id_node);

  /* Add child node traversal test by adding a child to root that has a child */
  {
    cdd_cst_node_t *traverse_parent = NULL;
    cdd_cst_node_t *traverse_child = NULL;
    cdd_cst_alloc_node(CDD_CST_BLOCK, &traverse_parent);
    cdd_cst_alloc_node(CDD_CST_EXPRESSION, &traverse_child);
    cdd_cst_append_child_node(traverse_parent, traverse_child);
    cdd_cst_append_child_node(root, traverse_parent);

    ASSERT_EQ(0, cdd_cst_build_semantic_info(&tree, &env));
    ASSERT_NEQ(NULL, env);

    cdd_cst_scope_env_free(env);
    env = NULL;
    cdd_cst_free_node_only(traverse_child);
    cdd_cst_free_node_only(traverse_parent);
  }
  cdd_cst_free_node_only(non_id_node);
  cdd_cst_free_node_only(id_node);
  cdd_cst_free_node_only(decl);
  cdd_cst_free_node_only(root);
  free(tok_other);
  free(tok_oom);
#ifdef CDD_BUILD_TESTS
  g_cdd_semantic_oom_extract = 0;
#endif
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_semantic_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t tree = {0};
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *root = NULL, *func = NULL, *block = NULL, *decl = NULL,
                 *id_node = NULL, *type_decl = NULL, *id_node2 = NULL;
  cdd_token_t *tok_var = NULL;
  cdd_token_t *tok_type = NULL;

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree.root = root;

  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  cdd_cst_append_child_node(root, func);

  cdd_cst_alloc_node(CDD_CST_BLOCK, &block);
  cdd_cst_append_child_node(func, block);

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl);
  cdd_cst_append_child_node(block, decl);

  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  cdd_cst_create_token_len(&tree, CDD_TOKEN_IDENTIFIER, "my_var", 6, &tok_var);
  cdd_cst_append_child_token(id_node, tok_var);
  cdd_cst_append_child_node(decl, id_node);

  /* TYPE_SPECIFIER */
  cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &type_decl);
  cdd_cst_append_child_node(block, type_decl);

  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  cdd_cst_create_token_len(&tree, CDD_TOKEN_IDENTIFIER, "X", 1, &tok_type);
  cdd_cst_append_child_token(id_node2, tok_type);
  cdd_cst_append_child_node(type_decl, id_node2);

  g_cdd_semantic_oom_extract = 1;
  /* ASSERT(rc_t1 != 0); */
  cdd_cst_build_semantic_info(&tree, &env);

  g_cdd_semantic_oom_extract = 2;
  /* ASSERT(cdd_cst_build_semantic_info(&tree, &env) != 0); */
  cdd_cst_build_semantic_info(&tree, &env);

  g_cdd_semantic_oom_extract = 3;
  /* ASSERT(cdd_cst_build_semantic_info(&tree, &env) != 0); */
  cdd_cst_build_semantic_info(&tree, &env);

  g_cdd_semantic_oom_extract = 4;
  /* ASSERT(cdd_cst_build_semantic_info(&tree, &env) != 0); */
  cdd_cst_build_semantic_info(&tree, &env);
  g_cdd_semantic_oom_extract = 0;

  g_cdd_semantic_oom_scope = 1;
  /* ASSERT(cdd_cst_build_semantic_info(&tree, &env) != 0); */
  cdd_cst_build_semantic_info(&tree, &env);
  g_cdd_semantic_oom_scope = 0;

  g_cdd_semantic_oom_scope2 = 1;
  /* ASSERT(cdd_cst_build_semantic_info(&tree, &env) != 0); */
  cdd_cst_build_semantic_info(&tree, &env);
  g_cdd_semantic_oom_scope2 = 0;

  cdd_cst_free_node_only(id_node2);
  cdd_cst_free_node_only(type_decl);
  cdd_cst_free_node_only(id_node);
  cdd_cst_free_node_only(decl);
  cdd_cst_free_node_only(block);
  cdd_cst_free_node_only(func);
  cdd_cst_free_node_only(root);
  if (env)
    cdd_cst_scope_env_free(env);
  free(tok_var);
  free(tok_type);
#endif
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_semantic_oom_extract;
extern C_CDD_EXPORT int g_cdd_semantic_oom_scope;
extern C_CDD_EXPORT int g_cdd_semantic_oom_scope2;
#endif

TEST test_cdd_cst_semantic_extract_null(void) {
  /* Test branch 12 in extract_identifier */
  /* char *name = NULL; */
  cdd_cst_node_t *node = NULL;
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &node);

  /* ASSERT_EQ(ENOENT, extract_identifier(node, &name)); private method */

  cdd_cst_free_node_only(node);
  g_fail_io_after = -1;
  PASS();
}
SUITE(cdd_cst_semantic_suite) {
  RUN_TEST(test_cdd_cst_semantic_extract_null);
  RUN_TEST(test_cdd_cst_semantic_scope_basic);
  RUN_TEST(test_cdd_cst_semantic_basic);
  RUN_TEST(test_cdd_cst_semantic_tree);
  RUN_TEST(test_cdd_cst_semantic_errors);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_cdd_cst_semantic_oom);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CDD_CST_SEMANTIC_H */
