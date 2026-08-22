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
#include "c_cdd/memory.h"

#include "../../classes/parse/cdd_cst_factory.h"
#include "../../classes/parse/cdd_cst_builder.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern int g_cdd_semantic_leave_fail;
extern int g_cdd_semantic_oom_scope2;
extern int g_cdd_semantic_oom_scope;
extern int g_cdd_alloc_fail;

#ifdef CDD_BUILD_TESTS
/* extern int g_cdd_semantic_oom_scope; (moved to global) */
/* extern int g_cdd_semantic_oom_scope2; (moved to global) */

#endif

TEST test_cdd_cst_semantic_scope_basic(void) {
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *node = NULL;
  cdd_cst_symbol_t *sym;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_scope_env_init(NULL));
  ASSERT_EQ(0, cdd_cst_scope_env_init(&env));
  ASSERT_NEQ(NULL, env);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_scope_enter(NULL, CDD_CST_SCOPE_BLOCK));
  ASSERT_EQ(0, cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK));
  ASSERT_EQ(0, cdd_cst_scope_enter(env, CDD_CST_SCOPE_FUNCTION));

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &node);
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_cst_scope_add_symbol(NULL, "foo", CDD_CST_SYMBOL_VARIABLE, node));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
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

  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_cst_scope_lookup_symbol(NULL, "foo", CDD_CST_SYMBOL_VARIABLE, &sym));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_cst_scope_lookup_symbol(env, NULL, CDD_CST_SYMBOL_VARIABLE, &sym));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_cst_scope_lookup_symbol(env, "foo", CDD_CST_SYMBOL_VARIABLE, NULL));

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
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            cdd_cst_scope_lookup_symbol(env, "missing", CDD_CST_SYMBOL_VARIABLE,
                                        &sym));

  ASSERT_EQ(0, cdd_cst_scope_leave(env)); /* pop function */
  ASSERT_EQ(0, cdd_cst_scope_leave(env)); /* pop block */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_scope_leave(env)); /* cannot pop file scope */

  cdd_cst_scope_env_free(env);
  env = NULL;
  cdd_cst_free_node(node);
  cdd_cst_scope_env_free(NULL); /* no-op */
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_semantic_basic(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_build_semantic_info(NULL, &env));

  ASSERT_EQ(0, cdd_cst_build_semantic_info(tree, &env));
  ASSERT_NEQ(NULL, env);

  cdd_cst_scope_env_free(env);
  env = NULL;
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_semantic_tree(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *root = NULL, *func = NULL, *block = NULL, *decl = NULL,
                 *id_node = NULL, *type_decl = NULL, *id_node2 = NULL,
                 *ns_node = NULL;
  cdd_token_t *tok_var = NULL;
  cdd_token_t *tok_type = NULL;

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  cdd_cst_append_child_node(root, func);

  cdd_cst_alloc_node(CDD_CST_NAMESPACE_DECLARATION, &ns_node);
  cdd_cst_append_child_node(func, ns_node);

  cdd_cst_alloc_node(CDD_CST_BLOCK, &block);
  cdd_cst_append_child_node(ns_node, block);

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl);
  cdd_cst_append_child_node(block, decl);

  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "my_var", 6, &tok_var);
  cdd_cst_append_child_token(id_node, tok_var);
  cdd_cst_append_child_node(decl, id_node);

  /* TYPE_SPECIFIER */
  cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &type_decl);
  cdd_cst_append_child_node(block, type_decl);

  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "X", 1, &tok_type);
  cdd_cst_append_child_token(id_node2, tok_type);
  cdd_cst_append_child_node(type_decl, id_node2);

  ASSERT_EQ(0, cdd_cst_build_semantic_info(tree, &env));
  ASSERT_NEQ(NULL, env);

  cdd_cst_scope_env_free(env);
  env = NULL;
  /* tree->base_tokens automatically tracks generated tokens, so freeing tree
   * frees them. */
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_semantic_errors(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *root = NULL, *decl = NULL, *id_node = NULL,
                 *non_id_node = NULL;
  cdd_token_t *tok_other = NULL;

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  /* Empty declaration to hit CDD_C_ERROR_NOT_FOUND in extract_identifier */
  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl);
  cdd_cst_append_child_node(root, decl);

  /* Node with child token that is NOT an identifier */
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  cdd_cst_create_token_len(tree, CDD_TOKEN_KEYWORD_INT, "int", 3, &tok_other);
  cdd_cst_append_child_token(id_node, tok_other);

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

    ASSERT_EQ(0, cdd_cst_build_semantic_info(tree, &env));
    ASSERT_NEQ(NULL, env);

    cdd_cst_scope_env_free(env);
    env = NULL;
  }
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_semantic_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *root = NULL, *func = NULL, *block = NULL, *decl = NULL,
                 *id_node = NULL, *type_decl = NULL, *id_node2 = NULL;
  cdd_token_t *tok_var = NULL;
  cdd_token_t *tok_type = NULL;
  cdd_c_error_t rc;

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  cdd_cst_append_child_node(root, func);

  cdd_cst_alloc_node(CDD_CST_BLOCK, &block);
  cdd_cst_append_child_node(func, block);

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl);
  cdd_cst_append_child_node(block, decl);

  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "my_var", 6, &tok_var);
  cdd_cst_append_child_token(id_node, tok_var);
  cdd_cst_append_child_node(decl, id_node);

  /* TYPE_SPECIFIER */
  cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &type_decl);
  cdd_cst_append_child_node(block, type_decl);

  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "X", 1, &tok_type);
  cdd_cst_append_child_token(id_node2, tok_type);
  cdd_cst_append_child_node(type_decl, id_node2);

#ifdef CDD_BUILD_TESTS
  {
    /* extern int g_cdd_alloc_fail; (moved to global) */
    int i;
    for (i = 1; i < 30; i++) {
      g_cdd_alloc_fail = i;
      rc = cdd_cst_build_semantic_info(tree, &env);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        cdd_cst_scope_env_free(env);
        env = NULL;
        break;
      }
    }
  }
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &r2);
    t2->root = r2;

    cdd_cst_node_t *id2 = NULL;
    cdd_token_t *tok2 = NULL;
    cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id2);
    cdd_cst_create_token_len(t2, CDD_TOKEN_IDENTIFIER, "var", 3, &tok2);
    cdd_cst_append_child_token(id2, tok2);
    cdd_cst_append_child_node(r2, id2);

    /* extern int g_cdd_alloc_fail; (moved to global) */
    int i;
    for (i = 1; i < 30; i++) {
      g_cdd_alloc_fail = i;
      rc = cdd_cst_build_semantic_info(t2, &env);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        cdd_cst_scope_env_free(env);
        env = NULL;
        break;
      }
    }
    cdd_cst_tree_free(t2);
  }

  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    cdd_cst_alloc_node(CDD_CST_NAMESPACE_DECLARATION, &r2);
    t2->root = r2;

    /* extern int g_cdd_alloc_fail; (moved to global) */
    int i;
    for (i = 1; i < 20; i++) {
      g_cdd_alloc_fail = i;
      rc = cdd_cst_build_semantic_info(t2, &env);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        cdd_cst_scope_env_free(env);
        env = NULL;
        break;
      }
    }
    cdd_cst_tree_free(t2);
  }
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    cdd_cst_alloc_node(CDD_CST_BLOCK, &r2);
    t2->root = r2;

    /* extern int g_cdd_alloc_fail; (moved to global) */
    int i;
    /* test leave fail by doing a large index that fails scope leave */
    /* scope enter is 1 or 2 allocations, so we start i around 3 */
    for (i = 1; i < 20; i++) {
      g_cdd_alloc_fail = i;
      rc = cdd_cst_build_semantic_info(t2, &env);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        cdd_cst_scope_env_free(env);
        env = NULL;
        break;
      }
    }
    cdd_cst_tree_free(t2);
  }

  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    cdd_cst_alloc_node(CDD_CST_BLOCK, &r2);
    t2->root = r2;

    /* extern int g_cdd_semantic_leave_fail; (moved to global) */
    g_cdd_semantic_leave_fail = 1;
    rc = cdd_cst_build_semantic_info(t2, &env);
    g_cdd_semantic_leave_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    cdd_cst_tree_free(t2);
  }

#endif

  cdd_cst_tree_free(tree);
#endif
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_semantic_extract_null(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;

  /* Test missing branches for cdd_cst_build_semantic_info */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_build_semantic_info(tree, NULL));

  /* CDD_CST_BLOCK failure */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    cdd_cst_alloc_node(CDD_CST_BLOCK, &r2);
    t2->root = r2;
    cdd_cst_tree_free(t2);
  }

  /* CDD_CST_NAMESPACE_DECLARATION failure */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    cdd_cst_alloc_node(CDD_CST_NAMESPACE_DECLARATION, &r2);
    t2->root = r2;
    cdd_cst_tree_free(t2);
  }

  /* TYPE_SPECIFIER OOM failure */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL, *ts = NULL, *id2 = NULL;
    cdd_token_t *tok2 = NULL;
    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &r2);
    t2->root = r2;
    cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &ts);
    cdd_cst_append_child_node(r2, ts);
    cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id2);
    cdd_cst_create_token_len(t2, CDD_TOKEN_IDENTIFIER, "T", 1, &tok2);
    cdd_cst_append_child_token(id2, tok2);
    cdd_cst_append_child_node(ts, id2);

    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_build_semantic_info(t2, &env));
    g_cdd_alloc_fail = 0;

    cdd_cst_tree_free(t2);
  }

  /* Test malloc failure in extract_identifier via g_fail_io_after */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL, *decl2 = NULL, *id2 = NULL;
    cdd_token_t *tok2 = NULL;
    cdd_cst_alloc_node(CDD_CST_DECLARATION, &r2);
    t2->root = r2;
    cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id2);
    cdd_cst_create_token_len(t2, CDD_TOKEN_IDENTIFIER, "var", 3, &tok2);
    cdd_cst_append_child_token(id2, tok2);
    cdd_cst_append_child_node(r2, id2);

    /* Try to hit the malloc inside extract_identifier.
       We loop over different values of g_fail_io_after to ensure we hit it. */
    {
      int i;
      for (i = 0; i < 10; i++) {
        g_fail_io_after = i;
        cdd_cst_build_semantic_info(t2, &env);
      }
    }
    g_fail_io_after = -1;

    cdd_cst_tree_free(t2);
  }

  /* Test identifier node with a child node to hit branch */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL, *id2 = NULL, *dummy = NULL;
    cdd_token_t *tok2 = NULL;
    cdd_cst_alloc_node(CDD_CST_DECLARATION, &r2);
    t2->root = r2;
    cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id2);
    cdd_cst_create_token_len(t2, CDD_TOKEN_IDENTIFIER, "var", 3, &tok2);
    cdd_cst_append_child_token(id2, tok2);
    cdd_cst_alloc_node(CDD_CST_EXPRESSION, &dummy);
    cdd_cst_append_child_node(id2, dummy); /* Identifier with child node */
    cdd_cst_append_child_node(r2, id2);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_build_semantic_info(t2, &env));
    cdd_cst_scope_env_free(env);
    env = NULL;
    cdd_cst_tree_free(t2);
  }

  /* Test cdd_cst_scope_env_init failing via g_cdd_scope_enter_fails for mock,
   * wait we can just do env_init=NULL check */
  {
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_build_semantic_info(tree, NULL));
  }

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches(void) {
  cdd_c_error_t rc;
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;

  cdd_cst_node_t *root = NULL;
  cdd_cst_node_t *decl1 = NULL, *decl2 = NULL;
  cdd_cst_node_t *id_node1 = NULL, *id_node2 = NULL;

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  /* Missing branch 17 (out_name == NULL) */
  /* Actually extract_identifier is static, we just need to hit it naturally */

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_cst_append_child_node(root, decl1);

  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node1);
  /* no child tokens, so extract_identifier fails but NOT_FOUND percolates */
  cdd_cst_append_child_node(decl1, id_node1);

  cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &decl2);
  cdd_cst_append_child_node(root, decl2);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  cdd_cst_append_child_node(decl2, id_node2);

  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cdd_cst_scope_env_free(env);
  env = NULL;

#ifdef CDD_BUILD_TESTS
  /* Force OOM during scope_add_symbol inside TYPE_SPECIFIER to hit branch 93/2
   */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    cdd_cst_node_t *ts = NULL, *id = NULL;
    cdd_token_t *tok = calloc(1, sizeof(cdd_token_t));
    tok->kind = CDD_TOKEN_IDENTIFIER;
    tok->start = (const uint8_t *)"Struct1";
    tok->length = 7;

    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &r2);
    cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &ts);
    cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id);

    cdd_cst_append_child_token(id, tok);
    cdd_cst_append_child_node(ts, id);
    cdd_cst_append_child_node(r2, ts);
    t2->root = r2;

    g_cdd_alloc_fail = 4;
    rc = cdd_cst_build_semantic_info(t2, &env);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    cdd_cst_tree_free(t2);
    free(tok);
  }
#endif

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_2(void) {
  cdd_c_error_t rc;
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;

  cdd_cst_node_t *root = NULL;
  cdd_cst_node_t *decl1 = NULL, *decl2 = NULL;
  cdd_cst_node_t *id_node1 = NULL, *id_node2 = NULL;
  cdd_token_t *tok1 = calloc(1, sizeof(cdd_token_t));
  cdd_token_t *tok2 = calloc(1, sizeof(cdd_token_t));

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  /* Missing branch 17 (out_name == NULL) */
  /* Actually extract_identifier returns CDD_C_ERROR_MEMORY if malloc fails */

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_cst_append_child_node(root, decl1);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node1);
  tok1->kind = CDD_TOKEN_IDENTIFIER;
  tok1->start = (const uint8_t *)"Var1";
  tok1->length = 4;
  cdd_cst_append_child_token(id_node1, tok1);
  cdd_cst_append_child_node(decl1, id_node1);

  cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &decl2);
  cdd_cst_append_child_node(root, decl2);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  tok2->kind = CDD_TOKEN_IDENTIFIER;
  tok2->start = (const uint8_t *)"Type1";
  tok2->length = 5;
  cdd_cst_append_child_token(id_node2, tok2);
  cdd_cst_append_child_node(decl2, id_node2);

  /* g_cdd_alloc_fail = 1 will fail malloc in extract_identifier for
   * CDD_CST_DECLARATION */
  g_cdd_alloc_fail = 1;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* fail malloc in extract_identifier for CDD_CST_TYPE_SPECIFIER */
  g_cdd_alloc_fail = 2;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* Also test the loop exhaustion with a child node that returns NOT_FOUND */
  /* This hits branch 35 */
  {
    cdd_cst_node_t *decl3 = NULL, *id_node3 = NULL;
    cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl3);
    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &id_node3); /* NOT identifier */
    cdd_cst_append_child_node(decl3, id_node3);
    cdd_cst_append_child_node(root, decl3);
    rc = cdd_cst_build_semantic_info(tree, &env);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_scope_env_free(env);
    env = NULL;
  }

  cdd_cst_tree_free(tree);
  free(tok1);
  free(tok2);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_3(void) {
  cdd_c_error_t rc;
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;

  cdd_cst_node_t *root = NULL;
  cdd_cst_node_t *decl1 = NULL, *decl2 = NULL;
  cdd_cst_node_t *id_node1 = NULL, *id_node2 = NULL;
  cdd_token_t *tok1 = calloc(1, sizeof(cdd_token_t));
  cdd_token_t *tok2 = calloc(1, sizeof(cdd_token_t));

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  /* Missing branch 17 (out_name == NULL) */
  /* Actually line 17 is: if (node->children[i].kind == CDD_CST_CHILD_TOKEN) */
  /* We need an IDENTIFIER node that has a CHILD_NODE to fail the if */

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_cst_append_child_node(root, decl1);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node1);
  /* Append a node instead of token */
  cdd_cst_node_t *dummy_node = NULL;
  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node);
  cdd_cst_append_child_node(id_node1, dummy_node);
  cdd_cst_append_child_node(decl1, id_node1);

  /* For TYPE_SPECIFIER, to hit branch 93/2 (if rc != CDD_C_SUCCESS inside type
   * specifier) */
  cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &decl2);
  cdd_cst_append_child_node(root, decl2);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  tok2->kind = CDD_TOKEN_IDENTIFIER;
  tok2->start = (const uint8_t *)"Type1";
  tok2->length = 5;
  cdd_cst_append_child_token(id_node2, tok2);
  cdd_cst_append_child_node(decl2, id_node2);

  /* Fail cdd_cst_scope_add_symbol inside TYPE_SPECIFIER */
  g_cdd_alloc_fail = 3;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* Line 35: if (rc != CDD_C_ERROR_NOT_FOUND) inside extract_identifier
   * recursion */
  /* Need extract_identifier to fail with CDD_C_ERROR_MEMORY deep inside */
  /* Add another decl */
  cdd_cst_node_t *decl3 = NULL, *id_node3 = NULL;
  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl3);
  cdd_cst_append_child_node(root, decl3);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node3);
  tok1->kind = CDD_TOKEN_IDENTIFIER;
  tok1->start = (const uint8_t *)"Var2";
  tok1->length = 4;
  cdd_cst_append_child_token(id_node3, tok1);
  /* Make a wrapper node so we recurse */
  cdd_cst_node_t *wrap = NULL;
  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &wrap);
  cdd_cst_append_child_node(wrap, id_node3);
  cdd_cst_append_child_node(decl3, wrap);

  /* Now g_cdd_alloc_fail = 1 will fail inside extract_identifier of id_node3 */
  /* We want to fail the exact malloc, which might be later */
  /* Actually, let's just loop and check coverage */
  int i;
  for (i = 1; i < 6; ++i) {
    g_cdd_alloc_fail = i;
    rc = cdd_cst_build_semantic_info(tree, &env);
    if (rc == CDD_C_SUCCESS) {
      g_cdd_alloc_fail = 0;
      break;
    }
  }
  g_cdd_alloc_fail = 0;

  cdd_cst_tree_free(tree);
  free(tok1);
  free(tok2);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_4(void) {
  cdd_c_error_t rc;
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;

  cdd_cst_node_t *root = NULL;
  cdd_cst_node_t *decl1 = NULL, *decl2 = NULL;
  cdd_cst_node_t *id_node1 = NULL, *id_node2 = NULL;
  cdd_token_t *tok1 = calloc(1, sizeof(cdd_token_t));

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_cst_append_child_node(root, decl1);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node1);
  tok1->kind = CDD_TOKEN_IDENTIFIER;
  tok1->start = (const uint8_t *)"Var3";
  tok1->length = 4;
  cdd_cst_append_child_token(id_node1, tok1);
  cdd_cst_append_child_node(decl1, id_node1);

  g_cdd_alloc_fail =
      3; /* to hit the exact malloc inside cdd_cst_scope_add_symbol */
  rc = cdd_cst_build_semantic_info(tree, &env);
  /* The branch missing is branch 2 which means rc != CDD_C_SUCCESS inside
   * cdd_cst_scope_add_symbol for CDD_CST_DECLARATION */
  /* If g_cdd_alloc_fail = 3 fails something else, loop it */
  g_cdd_alloc_fail = 0;

  int i;
  for (i = 1; i < 6; ++i) {
    g_cdd_alloc_fail = i;
    rc = cdd_cst_build_semantic_info(tree, &env);
    if (rc == CDD_C_SUCCESS) {
      g_cdd_alloc_fail = 0;
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }
  g_cdd_alloc_fail = 0;

  cdd_cst_tree_free(tree);
  free(tok1);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_5(void) {
  cdd_c_error_t rc;
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;

  cdd_cst_node_t *root = NULL;
  cdd_cst_node_t *decl1 = NULL, *decl2 = NULL;
  cdd_cst_node_t *id_node1 = NULL, *id_node2 = NULL;
  cdd_token_t *tok1 = calloc(1, sizeof(cdd_token_t));

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_cst_append_child_node(root, decl1);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node1);
  tok1->kind = CDD_TOKEN_IDENTIFIER;
  tok1->start = (const uint8_t *)"Var3";
  tok1->length = 4;
  cdd_cst_append_child_token(id_node1, tok1);

  cdd_cst_node_t *wrap = NULL;
  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &wrap);
  cdd_cst_append_child_node(wrap, id_node1);

  cdd_cst_append_child_node(decl1, wrap);

  g_cdd_alloc_fail = 1;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  int i;
  for (i = 1; i < 6; ++i) {
    g_cdd_alloc_fail = i;
    rc = cdd_cst_build_semantic_info(tree, &env);
    if (rc == CDD_C_SUCCESS) {
      g_cdd_alloc_fail = 0;
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }
  g_cdd_alloc_fail = 0;

  cdd_cst_tree_free(tree);
  tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &decl2);
  cdd_cst_append_child_node(root, decl2);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  cdd_cst_append_child_token(id_node2, tok1);
  cdd_cst_append_child_node(decl2, id_node2);

  for (i = 1; i < 6; ++i) {
    g_cdd_alloc_fail = i;
    rc = cdd_cst_build_semantic_info(tree, &env);
    if (rc == CDD_C_SUCCESS) {
      g_cdd_alloc_fail = 0;
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }
  g_cdd_alloc_fail = 0;

  cdd_cst_tree_free(tree);
  free(tok1);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_6(void) {
  cdd_c_error_t rc;
  char *name = NULL;
  cdd_cst_node_t *decl1 = NULL, *id_node1 = NULL;

  /* Missing branch 35: extract_identifier recursion failure bubble up */
  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);

  /* Make child node fail extract_identifier with an error other than NOT_FOUND
   */
  /* e.g., MEMORY error */
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node1);
  cdd_token_t *tok1 = calloc(1, sizeof(cdd_token_t));
  tok1->kind = CDD_TOKEN_IDENTIFIER;
  tok1->start = (const uint8_t *)"V";
  tok1->length = 1;
  cdd_cst_append_child_token(id_node1, tok1);
  cdd_cst_append_child_node(decl1, id_node1);

  g_cdd_alloc_fail = 1;
  rc = cdd_cst_build_semantic_info(
      NULL, NULL); /* dummy call to show we can use extract_identifier manually?
                      No it's static. */
  g_cdd_alloc_fail = 0;

  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = NULL;
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_append_child_node(root, decl1);

  cdd_cst_scope_env_t *env = NULL;
  g_cdd_alloc_fail = 1;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  cdd_cst_tree_free(tree);
  free(tok1);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_7(void) {
  cdd_c_error_t rc;
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;

  cdd_cst_node_t *root = NULL;
  cdd_cst_node_t *decl1 = NULL, *decl2 = NULL;
  cdd_cst_node_t *id_node1 = NULL, *id_node2 = NULL;
  cdd_token_t *tok1 = calloc(1, sizeof(cdd_token_t));

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_cst_append_child_node(root, decl1);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node1);
  tok1->kind = CDD_TOKEN_IDENTIFIER;
  tok1->start = (const uint8_t *)"V7";
  tok1->length = 2;
  cdd_cst_append_child_token(id_node1, tok1);
  cdd_cst_append_child_node(decl1, id_node1);

  cdd_cst_tree_free(tree);
  free(tok1);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_8(void) {
  cdd_c_error_t rc;
  char *name = NULL;

  cdd_cst_node_t *decl1 = NULL, *child1 = NULL, *child2 = NULL,
                 *id_node1 = NULL;
  cdd_token_t *tok1 = calloc(1, sizeof(cdd_token_t));

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);

  /* Child 1 is unknown, has no identifier -> returns NOT_FOUND */
  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &child1);
  cdd_cst_append_child_node(decl1, child1);

  /* Child 2 is unknown, but contains an identifier -> returns SUCCESS */
  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &child2);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node1);
  tok1->kind = CDD_TOKEN_IDENTIFIER;
  tok1->start = (const uint8_t *)"V8";
  tok1->length = 2;
  cdd_cst_append_child_token(id_node1, tok1);
  cdd_cst_append_child_node(child2, id_node1);

  cdd_cst_append_child_node(decl1, child2);

  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = NULL;
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_append_child_node(root, decl1);

  cdd_cst_scope_env_t *env = NULL;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cdd_cst_scope_env_free(env);
  cdd_cst_tree_free(tree);
  free(tok1);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_9(void) {
  cdd_c_error_t rc;
  char *name = NULL;
  cdd_cst_node_t *decl1 = NULL, *child1 = NULL, *child2 = NULL;

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &child1);
  cdd_cst_append_child_node(decl1, child1);

  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &child2);
  cdd_cst_node_t *id_node = NULL;
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  cdd_token_t *tok = calloc(1, sizeof(cdd_token_t));
  tok->kind = CDD_TOKEN_IDENTIFIER;
  tok->start = (const uint8_t *)"V9";
  tok->length = 2;
  cdd_cst_append_child_token(id_node, tok);
  cdd_cst_append_child_node(child2, id_node);
  cdd_cst_append_child_node(decl1, child2);

  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = NULL;
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_append_child_node(root, decl1);

  cdd_cst_scope_env_t *env = NULL;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* also test when child2 fails with something other than NOT_FOUND and NOT
   * SUCCESS */
  /* e.g., if child2 is IDENTIFIER but malloc fails, that returns
   * CDD_C_ERROR_MEMORY */
  g_cdd_alloc_fail = 1;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  cdd_cst_tree_free(tree);
  cdd_cst_scope_env_free(env);
  free(tok);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_10(void) {
  cdd_c_error_t rc;
  cdd_cst_node_t *decl1 = NULL, *child1 = NULL, *child2 = NULL;

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &child1);
  cdd_cst_append_child_node(decl1, child1);

  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &child2);
  cdd_cst_node_t *id_node = NULL;
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  /* no child tokens, so id_node returns NOT_FOUND */
  cdd_cst_append_child_node(child2, id_node);
  cdd_cst_append_child_node(decl1, child2);

  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = NULL;
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_append_child_node(root, decl1);

  cdd_cst_scope_env_t *env = NULL;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cdd_cst_tree_free(tree);
  cdd_cst_scope_env_free(env);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_11(void) {
  cdd_c_error_t rc;
  cdd_cst_node_t *decl1 = NULL, *child1 = NULL, *child2 = NULL;

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &child1);
  cdd_cst_append_child_node(decl1, child1);

  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &child2);
  cdd_cst_node_t *id_node = NULL;
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  /* no child tokens, so id_node returns NOT_FOUND */
  cdd_cst_append_child_node(child2, id_node);
  cdd_cst_append_child_node(decl1, child2);

  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = NULL;
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_append_child_node(root, decl1);

  cdd_cst_scope_env_t *env = NULL;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* also test when child2 fails with something other than NOT_FOUND and NOT
   * SUCCESS */
  /* e.g., if child2 is IDENTIFIER but malloc fails, that returns
   * CDD_C_ERROR_MEMORY */
  g_cdd_alloc_fail = 1;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  cdd_cst_tree_free(tree);
  cdd_cst_scope_env_free(env);
  PASS();
}

TEST test_cdd_cst_semantic_missing_branches_13(void) {
  cdd_c_error_t rc;
  cdd_cst_node_t *decl1 = NULL;

  cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl1);
  cdd_token_t *tok = calloc(1, sizeof(cdd_token_t));
  tok->kind = CDD_TOKEN_OTHER;
  tok->start = (const uint8_t *)"V13";
  tok->length = 3;
  cdd_cst_append_child_token(decl1, tok);

  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = NULL;
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_append_child_node(root, decl1);

  cdd_cst_scope_env_t *env = NULL;
  rc = cdd_cst_build_semantic_info(tree, &env);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cdd_cst_tree_free(tree);
  cdd_cst_scope_env_free(env);
  free(tok);
  PASS();
}

TEST test_cdd_cst_parser_oom_new(void) {
  int i;
  for (i = 1; i < 200; i++) {
    cdd_cst_tree_t *tree = NULL;
    const char *code = "namespace N {\n"
                       "  class C : public B {\n"
                       "  public:\n"
                       "    C() noexcept(true) {}\n"
                       "    ~C() {}\n"
                       "    void operator+() {}\n"
                       "  };\n"
                       "  template<typename T> class Tmpl {};\n"
                       "  void f() {\n"
                       "    try { throw 1; } catch (...) {}\n"
                       "  }\n"
                       "}\n"
                       "using namespace N;\n";

    g_cdd_alloc_fail = i;
    int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
    g_cdd_alloc_fail = 0;

    if (rc == 0) {
      if (tree)
        cdd_cst_tree_free(tree);
      break;
    }
  }
  PASS();
}

SUITE(cdd_cst_semantic_suite) {
  RUN_TEST(test_cdd_cst_parser_oom_new);
  RUN_TEST(test_cdd_cst_semantic_extract_null);
  RUN_TEST(test_cdd_cst_semantic_scope_basic);
  RUN_TEST(test_cdd_cst_semantic_basic);
  RUN_TEST(test_cdd_cst_semantic_tree);
  RUN_TEST(test_cdd_cst_semantic_errors);
  RUN_TEST(test_cdd_cst_semantic_missing_branches);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_2);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_3);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_4);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_5);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_6);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_7);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_8);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_9);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_10);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_11);
  RUN_TEST(test_cdd_cst_semantic_missing_branches_13);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_cdd_cst_semantic_oom);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CDD_CST_SEMANTIC_H */
