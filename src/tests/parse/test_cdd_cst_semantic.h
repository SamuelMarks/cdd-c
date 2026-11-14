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
/** @brief Test definition g_cdd_semantic_oom_scope2. */
extern C_CDD_EXPORT int g_cdd_semantic_oom_scope2;
/** @brief Test definition g_cdd_semantic_oom_scope. */
extern C_CDD_EXPORT int g_cdd_semantic_oom_scope;

#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_cdd_semantic_oom_scope; (moved to global) */
/* extern C_CDD_EXPORT int g_cdd_semantic_oom_scope2; (moved to global) */

#endif

/**
 * @brief Unit test test_cdd_cst_semantic_scope_basic.
 * @return Greatest test result.
 */
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

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_DECLARATION, &node);
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

  g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
  env = NULL;
  g_cdd_last_alloc_err = cdd_cst_free_node(node);
  g_cdd_last_alloc_err = cdd_cst_scope_env_free(NULL); /* no-op */
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_cdd_cst_semantic_basic.
 * @return Greatest test result.
 */
TEST test_cdd_cst_semantic_basic(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_build_semantic_info(NULL, &env));

  ASSERT_EQ(0, cdd_cst_build_semantic_info(tree, &env));
  ASSERT_NEQ(NULL, env);

  g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
  env = NULL;
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_cdd_cst_semantic_tree.
 * @return Greatest test result.
 */
TEST test_cdd_cst_semantic_tree(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *root = NULL, *func = NULL, *block = NULL, *decl = NULL,
                 *id_node = NULL, *type_decl = NULL, *id_node2 = NULL,
                 *ns_node = NULL;
  cdd_token_t *tok_var = NULL;
  cdd_token_t *tok_type = NULL;

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(root, func);

  g_cdd_last_alloc_err =
      cdd_cst_alloc_node(CDD_CST_NAMESPACE_DECLARATION, &ns_node);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(func, ns_node);

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_BLOCK, &block);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(ns_node, block);

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(block, decl);

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  g_cdd_last_alloc_err = cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER,
                                                  "my_var", 6, &tok_var);
  g_cdd_last_alloc_err = cdd_cst_append_child_token(id_node, tok_var);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(decl, id_node);

  /* TYPE_SPECIFIER */
  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &type_decl);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(block, type_decl);

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  g_cdd_last_alloc_err =
      cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "X", 1, &tok_type);
  g_cdd_last_alloc_err = cdd_cst_append_child_token(id_node2, tok_type);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(type_decl, id_node2);

  ASSERT_EQ(0, cdd_cst_build_semantic_info(tree, &env));
  ASSERT_NEQ(NULL, env);

  g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
  env = NULL;
  /* tree->base_tokens automatically tracks generated tokens, so freeing tree
   * frees them. */
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_cdd_cst_semantic_errors.
 * @return Greatest test result.
 */
TEST test_cdd_cst_semantic_errors(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *root = NULL, *decl = NULL, *id_node = NULL,
                 *non_id_node = NULL;
  cdd_token_t *tok_other = NULL;

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  /* Empty declaration to hit CDD_C_ERROR_NOT_FOUND in extract_identifier */
  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(root, decl);

  /* Node with child token that is NOT an identifier */
  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  g_cdd_last_alloc_err = cdd_cst_create_token_len(tree, CDD_TOKEN_KEYWORD_INT,
                                                  "int", 3, &tok_other);
  g_cdd_last_alloc_err = cdd_cst_append_child_token(id_node, tok_other);

  g_cdd_last_alloc_err = cdd_cst_append_child_node(decl, id_node);

  /* Node with child node that fails extract_identifier */
  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_EXPRESSION, &non_id_node);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(decl, non_id_node);

  /* Add child node traversal test by adding a child to root that has a child */
  {
    cdd_cst_node_t *traverse_parent = NULL;
    cdd_cst_node_t *traverse_child = NULL;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_BLOCK, &traverse_parent);
    g_cdd_last_alloc_err =
        cdd_cst_alloc_node(CDD_CST_EXPRESSION, &traverse_child);
    g_cdd_last_alloc_err =
        cdd_cst_append_child_node(traverse_parent, traverse_child);
    g_cdd_last_alloc_err = cdd_cst_append_child_node(root, traverse_parent);

    ASSERT_EQ(0, cdd_cst_build_semantic_info(tree, &env));
    ASSERT_NEQ(NULL, env);

    g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
    env = NULL;
  }
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_cdd_cst_semantic_oom.
 * @return Greatest test result.
 */
TEST test_cdd_cst_semantic_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *root = NULL, *func = NULL, *block = NULL, *decl = NULL,
                 *id_node = NULL, *type_decl = NULL, *id_node2 = NULL;
  cdd_token_t *tok_var = NULL;
  cdd_token_t *tok_type = NULL;
  cdd_c_error_t rc = 0;

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(root, func);

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_BLOCK, &block);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(func, block);

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(block, decl);

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node);
  g_cdd_last_alloc_err = cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER,
                                                  "my_var", 6, &tok_var);
  g_cdd_last_alloc_err = cdd_cst_append_child_token(id_node, tok_var);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(decl, id_node);

  /* TYPE_SPECIFIER */
  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &type_decl);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(block, type_decl);

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id_node2);
  g_cdd_last_alloc_err =
      cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "X", 1, &tok_type);
  g_cdd_last_alloc_err = cdd_cst_append_child_token(id_node2, tok_type);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(type_decl, id_node2);

#ifdef CDD_BUILD_TESTS
  {
    /*  (moved to global) */
    int i;
    for (i = 1; i < 30; i++) {
      g_cdd_alloc_fail = i;
      rc = cdd_cst_build_semantic_info(tree, &env);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
        env = NULL;
        break;
      }
    }
  }
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &r2);
    t2->root = r2;

    {
      cdd_cst_node_t *id2 = NULL;
      cdd_token_t *tok2 = NULL;
      g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id2);
      g_cdd_last_alloc_err =
          cdd_cst_create_token_len(t2, CDD_TOKEN_IDENTIFIER, "var", 3, &tok2);
      g_cdd_last_alloc_err = cdd_cst_append_child_token(id2, tok2);
      g_cdd_last_alloc_err = cdd_cst_append_child_node(r2, id2);

      /*  (moved to global) */
      {
        int i;
        for (i = 1; i < 30; i++) {
          g_cdd_alloc_fail = i;
          rc = cdd_cst_build_semantic_info(t2, &env);
          g_cdd_alloc_fail = 0;
          if (rc == CDD_C_SUCCESS) {
            g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
            env = NULL;
            break;
          }
        }
        g_cdd_last_alloc_err = cdd_cst_tree_free(t2);
      }
    }
  }

  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    g_cdd_last_alloc_err =
        cdd_cst_alloc_node(CDD_CST_NAMESPACE_DECLARATION, &r2);
    t2->root = r2;

    /*  (moved to global) */
    {
      int i;
      for (i = 1; i < 20; i++) {
        g_cdd_alloc_fail = i;
        rc = cdd_cst_build_semantic_info(t2, &env);
        g_cdd_alloc_fail = 0;
        if (rc == CDD_C_SUCCESS) {
          g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
          env = NULL;
          break;
        }
      }
      g_cdd_last_alloc_err = cdd_cst_tree_free(t2);
    }
  }
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_BLOCK, &r2);
    t2->root = r2;

    /*  (moved to global) */
    {
      int i;
      /* test leave fail by doing a large index that fails scope leave */
      /* scope enter is 1 or 2 allocations, so we start i around 3 */
      for (i = 1; i < 20; i++) {
        g_cdd_alloc_fail = i;
        rc = cdd_cst_build_semantic_info(t2, &env);
        g_cdd_alloc_fail = 0;
        if (rc == CDD_C_SUCCESS) {
          g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
          env = NULL;
          break;
        }
      }
      g_cdd_last_alloc_err = cdd_cst_tree_free(t2);
    }
  }

  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_BLOCK, &r2);
    t2->root = r2;

    rc = cdd_cst_build_semantic_info(t2, &env);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
    env = NULL;
    g_cdd_last_alloc_err = cdd_cst_tree_free(t2);
  }

#endif

  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_cdd_cst_semantic_extract_null.
 * @return Greatest test result.
 */
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
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_BLOCK, &r2);
    t2->root = r2;
    g_cdd_last_alloc_err = cdd_cst_tree_free(t2);
  }

  /* CDD_CST_NAMESPACE_DECLARATION failure */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL;
    g_cdd_last_alloc_err =
        cdd_cst_alloc_node(CDD_CST_NAMESPACE_DECLARATION, &r2);
    t2->root = r2;
    g_cdd_last_alloc_err = cdd_cst_tree_free(t2);
  }

  /* TYPE_SPECIFIER OOM failure */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL, *ts = NULL, *id2 = NULL;
    cdd_token_t *tok2 = NULL;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &r2);
    t2->root = r2;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &ts);
    g_cdd_last_alloc_err = cdd_cst_append_child_node(r2, ts);
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id2);
    g_cdd_last_alloc_err =
        cdd_cst_create_token_len(t2, CDD_TOKEN_IDENTIFIER, "T", 1, &tok2);
    g_cdd_last_alloc_err = cdd_cst_append_child_token(id2, tok2);
    g_cdd_last_alloc_err = cdd_cst_append_child_node(ts, id2);

    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_build_semantic_info(t2, &env));
    g_cdd_alloc_fail = 0;

    g_cdd_last_alloc_err = cdd_cst_tree_free(t2);
  }

  /* Test malloc failure in extract_identifier via g_fail_io_after */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL, *id2 = NULL;
    cdd_token_t *tok2 = NULL;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_DECLARATION, &r2);
    t2->root = r2;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id2);
    g_cdd_last_alloc_err =
        cdd_cst_create_token_len(t2, CDD_TOKEN_IDENTIFIER, "var", 3, &tok2);
    g_cdd_last_alloc_err = cdd_cst_append_child_token(id2, tok2);
    g_cdd_last_alloc_err = cdd_cst_append_child_node(r2, id2);

    /* Try to hit the malloc inside extract_identifier.
       We loop over different values of g_fail_io_after to ensure we hit it. */
    {
      int i;
      for (i = 0; i < 10; i++) {
        g_fail_io_after = i;
        g_cdd_last_alloc_err = cdd_cst_build_semantic_info(t2, &env);
        if (env) {
          g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
          env = NULL;
        }
      }
    }
    g_fail_io_after = -1;

    g_cdd_last_alloc_err = cdd_cst_tree_free(t2);
  }

  /* Test identifier node with a child node to hit branch */
  {
    cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *r2 = NULL, *id2 = NULL, *dummy = NULL;
    cdd_token_t *tok2 = NULL;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_DECLARATION, &r2);
    t2->root = r2;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &id2);
    g_cdd_last_alloc_err =
        cdd_cst_create_token_len(t2, CDD_TOKEN_IDENTIFIER, "var", 3, &tok2);
    g_cdd_last_alloc_err = cdd_cst_append_child_token(id2, tok2);
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_EXPRESSION, &dummy);
    g_cdd_last_alloc_err =
        cdd_cst_append_child_node(id2, dummy); /* Identifier with child node */
    g_cdd_last_alloc_err = cdd_cst_append_child_node(r2, id2);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_build_semantic_info(t2, &env));
    g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
    env = NULL;
    g_cdd_last_alloc_err = cdd_cst_tree_free(t2);
  }

  /* Test cdd_cst_scope_env_init failing via g_cdd_scope_enter_fails for mock,
   * wait we can just do env_init=NULL check */
  {
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_build_semantic_info(tree, NULL));
  }

  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Comprehensive test for C declaration/aggregate/enum/header semantic
 * analysis and OOM handling.
 * @return Greatest test result.
 */
TEST test_cdd_cst_semantic_c_declarations_and_headers(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_symbol_t *sym = NULL;
  const char *inc_dirs[2];
  const char *hdr_a = "#include <sem_test_b.h>\n"
                      "typedef unsigned long sem_sz_t;\n"
                      "struct SemHdr { int h_field; };\n";
  const char *hdr_b =
      "#include \"sem_test_a.h\"\n"
      "enum SemHdrEnum { SH_ZERO = '\\0', SH_NL = '\\n', SH_ESC = '\\t' };\n";
  const char *src1 =
      "#include \"sem_test_a.h\"\n"
      "#include \"sem_test_a.h\"\n"
      "#include \"sem_missing_hdr.h\"\n"
      "#include \"\"\n"
      "typedef int __attribute__((aligned(4))) sem_int_t, *sem_int_ptr_t;\n"
      "typedef int (*sem_fn_ptr_t)(int);\n"
      "__declspec(align(8)) struct SemRec {\n"
      "  const int * restrict ptr;\n"
      "  char buf[16];\n"
      "  char flex[];\n"
      "  unsigned int bf : 3;\n"
      "} g_rec;\n"
      "union SemUni { int u_i; double u_d; } g_uni;\n";
  const char *src2 =
      "enum SemEn {\n"
      "  SE_A, SE_BIN = 0b101, SE_NEG = -2, SE_POS = +(4), SE_PAR = (7),\n"
      "  SE_CH = 'A', SE_REF = SE_BIN, SE_DANGLING = -, SE_BLK = ({ 3; }), "
      "SE_EMPTY =\n"
      "};\n"
      "class CppOnly { int m; };\n"
      "typedef enum { ANON_E0 = 1 } anon_enum_t;\n"
      "typedef struct { __int128 big; } anon_struct_t;\n";
  const char *src3 =
      "const char *proto_fn(void);\n"
      "static inline int *compute(sem_int_t x, int arr[4]) {\n"
      "  struct LocalAgg { int lx; } loc_agg;\n"
      "  for (int i = 0; i < 4; i++) { if (i == 2) return &arr[0]; }\n"
      "  switch (x) { case 0: goto done; default: break; }\n"
      "done: return 0;\n"
      "}\n"
      "implicit_ret_fn(int a) { return a; }\n";
  char src[1280];
  const char *src_p = src;
  FILE *fa = fopen("sem_test_a.h", "wb");
  FILE *fb = fopen("sem_test_b.h", "wb");
  ASSERT(fa != NULL && fb != NULL);
  fwrite(hdr_a, 1, strlen(hdr_a), fa);
  fclose(fa);
  fwrite(hdr_b, 1, strlen(hdr_b), fb);
  fclose(fb);
  CDD_SNPRINTF(src, sizeof(src), "%s%s%s", src1, src2, src3);

  inc_dirs[0] = NULL;
  inc_dirs[1] = ".";
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_build_semantic_info_ex(NULL, inc_dirs, 2, &env));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(src_p), &tree));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_build_semantic_info_ex(tree, inc_dirs, 2, &env));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_lookup_symbol(
                               env, "sem_sz_t", CDD_CST_SYMBOL_TYPEDEF, &sym));
  ASSERT_STR_EQ("unsigned long", sym->type_str);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_scope_lookup_symbol(env, "SE_BIN",
                                        CDD_CST_SYMBOL_ENUM_CONSTANT, &sym));
  ASSERT_EQ(5, sym->enum_value);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_scope_lookup_symbol(env, "SE_NEG",
                                        CDD_CST_SYMBOL_ENUM_CONSTANT, &sym));
  ASSERT_EQ(-2, sym->enum_value);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_lookup_symbol(
                               env, "compute", CDD_CST_SYMBOL_FUNCTION, &sym));
  ASSERT_STR_EQ("int *", sym->type_str);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(env));

#ifdef CDD_BUILD_TESTS
  {
    int k;
    const char *oom_snippets[] = {
        "struct S { int x; int y; } s;\n", "#include \"sem_test_b.h\"\n",
        "void fn(void) { struct Inner { int x; }; }\n"};
    size_t si;
    for (si = 0; si < sizeof(oom_snippets) / sizeof(oom_snippets[0]); si++) {
      cdd_cst_tree_t *otree = NULL;
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(az_span_create_from_str(oom_snippets[si]), &otree));
      for (k = 1; k <= 250; k++) {
        cdd_cst_scope_env_t *oom_env = NULL;
        cdd_c_error_t orc;
        g_cdd_alloc_fail = k;
        orc = cdd_cst_build_semantic_info_ex(otree, inc_dirs, 2, &oom_env);
        g_cdd_alloc_fail = 0;
        if (orc == CDD_C_SUCCESS) {
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(oom_env));
        }
      }
      for (k = 1; k <= 40; k++) {
        cdd_cst_scope_env_t *oom_env = NULL;
        cdd_c_error_t orc;
        g_cdd_strdup_fail = k;
        orc = cdd_cst_build_semantic_info_ex(otree, inc_dirs, 2, &oom_env);
        g_cdd_strdup_fail = 0;
        if (orc == CDD_C_SUCCESS) {
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(oom_env));
        }
      }
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(otree));
    }
  }
#endif

  /* Additional enum, declarator, synthetic token, and include-depth cases */
  {
    cdd_cst_tree_t *etree = NULL;
    cdd_cst_scope_env_t *eenv = NULL;
    cdd_token_t *etok = NULL;
    const char *extra_src =
        "enum EExtra {\n"
        "  EE_OPEN = (),\n"
        "  EE_BUP = 0B101,\n"
        "  EE_BBAD = 0b12,\n"
        "  EE_NCHAR = -'a',\n"
        "  EE_ESC = '\\n',\n"
        "  EE_UNK = NO_SUCH_ENUM,\n"
        "  EE_PLUS = ((+3)),\n"
        "  EE_MINUS_ONLY = -\n"
        "};\n"
        "int g_brace_init = { 1 };\n"
        "int g_chain = 1 = 2;\n"
        "struct SBit { int bf : 1 : 2; int arr[1234567890123456789012345]; };\n"
        "void f_anon_p(char) {}\n"
        "void f_loops(int i) {\n"
        "  for (; ; ) { break; }\n"
        "  for (i = 0; ; ) { break; }\n"
        "}\n";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(extra_src), &etree));
    /* Append direct IDENTIFIER token to first enum block */
    if (etree->root->num_children > 0 &&
        etree->root->children[0].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *enode = etree->root->children[0].val.node;
      size_t ci;
      for (ci = 0; ci < enode->num_children; ci++) {
        if (enode->children[ci].kind == CDD_CST_CHILD_NODE &&
            enode->children[ci].val.node->kind == CDD_CST_BLOCK) {
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_cst_create_token(etree, CDD_TOKEN_IDENTIFIER,
                                         "EE_DIRECT", &etok));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(
                                       enode->children[ci].val.node, etok));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_cst_create_token(etree, CDD_TOKEN_ASSIGN, "=", &etok));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(
                                       enode->children[ci].val.node, etok));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_cst_create_token(etree, CDD_TOKEN_LPAREN, "(", &etok));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(
                                       enode->children[ci].val.node, etok));
          break;
        }
      }
    }
    /* Append synthetic CASE and DEFAULT unknown statement nodes */
    {
      cdd_cst_node_t *un1 = NULL;
      cdd_cst_node_t *un2 = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &un1));
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_create_token(etree, CDD_TOKEN_KEYWORD_CASE, "case", &etok));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(un1, etok));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(etree->root, un1));

      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &un2));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(etree, CDD_TOKEN_KEYWORD_DEFAULT,
                                     "default", &etok));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(un2, etok));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(etree->root, un2));
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(etree, NULL, 0, &eenv));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(eenv));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(etree));
  }

  /* Zero-length token inside struct declaration */
  {
    cdd_cst_tree_t *ztree = NULL;
    cdd_cst_scope_env_t *zenv = NULL;
    cdd_token_t *zt = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("struct Z { int a; } zv;"),
                            &ztree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_create_token(ztree, CDD_TOKEN_OTHER, "", &zt));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_replace_token_child(
                                 ztree->root->children[0].val.node, 1, zt));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(ztree, NULL, 0, &zenv));
    zenv->bound_tree = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ztree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(zenv));
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  remove("sem_test_a.h");
  remove("sem_test_b.h");
  PASS();
}

/**
 * @brief Unit test test_cdd_cst_semantic_remaining_edge_branches.
 * @return Greatest test result.
 */
TEST test_cdd_cst_semantic_remaining_edge_branches(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_scope_env_t *env = NULL;
  const char *inc_dirs[1];
  int i;

  inc_dirs[0] = ".";

  /* 1. External header with function + for-init (hits is_external=1 &
     bind_ast=0) plus 9-deep include chain (hits depth >= MAX_INCLUDE_DEPTH)
        plus >64 unique headers (hits num_visited >= MAX_VISITED_HEADERS) */
  {
    char dname[32];
    char dbody[128];
    for (i = 0; i < 10; i++) {
      FILE *fd = NULL;
      CDD_SNPRINTF(dname, sizeof(dname), "sem_dep_%d.h", i);
      if (i < 9) {
        CDD_SNPRINTF(dbody, sizeof(dbody),
                     "#include \"sem_dep_%d.h\"\n"
                     "int ext_fn_%d(int a) { for (int j = 0; j < a; j++) {} "
                     "return a; }\n",
                     i + 1, i);
      } else {
        CDD_SNPRINTF(dbody, sizeof(dbody), "int ext_leaf;\n");
      }
#if defined(_MSC_VER)
      ASSERT_EQ(0, fopen_s(&fd, dname, "wb"));
#else
      fd = fopen(dname, "wb");
#endif
      ASSERT(fd != NULL);
      fwrite(dbody, 1, strlen(dbody), fd);
      fclose(fd);
    }

    {
      char many_inc[4096];
      size_t mpos = 0;
      char hname[32];
      mpos += (size_t)CDD_SNPRINTF(many_inc + mpos, sizeof(many_inc) - mpos,
                                   "#include \"sem_dep_0.h\"\n"
                                   "#include MACRO_INC\n"
                                   "#define SOME_MACRO 1\n");
      for (i = 0; i < 66; i++) {
        FILE *fh = NULL;
        CDD_SNPRINTF(hname, sizeof(hname), "sem_vh_%d.h", i);
#if defined(_MSC_VER)
        ASSERT_EQ(0, fopen_s(&fh, hname, "wb"));
#else
        fh = fopen(hname, "wb");
#endif
        ASSERT(fh != NULL);
        fwrite("int vh_x;\n", 1, 10, fh);
        fclose(fh);
        mpos += (size_t)CDD_SNPRINTF(many_inc + mpos, sizeof(many_inc) - mpos,
                                     "#include \"%s\"\n", hname);
      }
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(many_inc), &tree));
      /* Also test num_include_dirs == 0 with non-NULL include_dirs */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, inc_dirs, 0, &env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(env));
      env = NULL;

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, inc_dirs, 1, &env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(env));
      env = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      for (i = 0; i < 66; i++) {
        CDD_SNPRINTF(hname, sizeof(hname), "sem_vh_%d.h", i);
        remove(hname);
      }
    }
    for (i = 0; i < 10; i++) {
      CDD_SNPRINTF(dname, sizeof(dname), "sem_dep_%d.h", i);
      remove(dname);
    }
  }

  /* 1b. Synthetic tree with base_tokens == NULL and PREPROC_DIRECTIVE child */
  {
    cdd_cst_tree_t *ntree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *nroot = NULL;
    cdd_cst_node_t *npd = NULL;
    ASSERT(ntree != NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &nroot));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &npd));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(nroot, npd));
    ntree->root = nroot;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_build_semantic_info(ntree, &env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(env));
    env = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ntree));
  }

  /* 2. Specifier buffer cap, trailing __attribute__, enum edges, unnamed fn
     ptr, unclosed bracket, large enum (>256 tokens), long identifiers, struct
        with >32 pre-tokens & >64 post-tokens, struct followed by function */
  {
    char big_src[16384];
    char long_inc[300];
    size_t p = 0;
    char id125[130];
    memset(id125, 'Q', 125);
    id125[125] = '\0';

    /* Long specifier > 128 bytes */
    for (i = 0; i < 20; i++) {
      p += (size_t)CDD_SNPRINTF(big_src + p, sizeof(big_src) - p,
                                "const volatile ");
    }
    p += (size_t)CDD_SNPRINTF(big_src + p, sizeof(big_src) - p, "int g_cv;\n");

    /* Unnamed fn-ptr declarator (top-level UNKNOWN), long leading id */
    p += (size_t)CDD_SNPRINTF(big_src + p, sizeof(big_src) - p,
                              "int (*)(int);\n"
                              "void f_fp(int (*)(int)) {}\n"
                              "%s g_unknown_ty;\n",
                              id125);
    {
      char id132[136];
      memset(id132, 'Z', 132);
      id132[132] = '\0';
      p += (size_t)CDD_SNPRINTF(big_src + p, sizeof(big_src) - p,
                                "int (*fp_%s)(int);\n"
                                "int var_%s;\n"
                                "struct tag_%s { int m; };\n",
                                id132, id132, id132);
    }

    /* Enum with hex 0x10, >256 tokens, long id, negative enum ref, >60-digit
     * num */
    p += (size_t)CDD_SNPRINTF(
        big_src + p, sizeof(big_src) - p,
        "enum EBig { EB0 = 3, EB_HX = 0x10, EB_NEG = -EB0, EB_DEC = 123, "
        "EB_BIG = "
        "12345678901234567890123456789012345678901234567890123456789012345, "
        "EB_R = %s, %s = 1",
        id125, id125);
    for (i = 0; i < 140; i++) {
      p += (size_t)CDD_SNPRINTF(big_src + p, sizeof(big_src) - p, ", E_%d", i);
    }
    p += (size_t)CDD_SNPRINTF(big_src + p, sizeof(big_src) - p, " };\n");

    /* Struct with >32 pre-tokens and >64 post-tokens, plus struct * non-id,
       plus struct without trailing unknown before function */
    for (i = 0; i < 35; i++) {
      p += (size_t)CDD_SNPRINTF(big_src + p, sizeof(big_src) - p, "const ");
    }
    p += (size_t)CDD_SNPRINTF(big_src + p, sizeof(big_src) - p,
                              "struct SMany { int a; } v0");
    for (i = 1; i < 40; i++) {
      p += (size_t)CDD_SNPRINTF(big_src + p, sizeof(big_src) - p, ", v%d", i);
    }
    p += (size_t)CDD_SNPRINTF(
        big_src + p, sizeof(big_src) - p,
        ";\n"
        "struct { int x; }\n"
        "void f_after_agg(void) { if (1) { } else { } }\n");

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(big_src), &tree));

    /* Synthetic node & token attachments for malformed lexer/AST edge cases */
    {
      cdd_token_t *tk = NULL;
      cdd_cst_node_t *un = NULL;
      cdd_cst_node_t *en = NULL;
      cdd_cst_node_t *eblk = NULL;
      cdd_cst_node_t *fn = NULL;
      cdd_cst_node_t *pd = NULL;
      cdd_cst_node_t *sn = NULL;
      cdd_cst_node_t *sblk1 = NULL;
      cdd_cst_node_t *sblk2 = NULL;

      /* 1) Declarator with trailing `__attribute__` in specifiers + non-BLOCK
       * child */
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &un));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_KEYWORD_INT, "int", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(un, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER,
                                                    "__attribute__", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(un, tk));
      {
        cdd_cst_node_t *uchild = NULL;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &uchild));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(un, uchild));
      }
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, un));

      /* 1b) Unclosed `[` in declarator */
      un = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &un));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_KEYWORD_INT, "int", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(un, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "x_tr", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(un, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_LBRACKET, "[", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(un, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, un));

      /* 2) Empty CDD_CST_UNKNOWN node (un == 0) */
      un = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &un));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, un));

      /* 3) Enum block with E_LP = ( , E_PL = + , '' , '\' , >256 direct tokens
       */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_CLASS_DECLARATION, &en));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_create_token(
                                   tree, CDD_TOKEN_KEYWORD_ENUM, "enum", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(en, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_BLOCK, &eblk));
      /* Leading ',' and ')' */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_COMMA, ",", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_RPAREN, ")", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_COMMA, ",", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      /* E_PL = + , E_MN = - , E_C2 = '' , E_C3 = '\' , then E_LP = ( last */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "E_PL", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_ASSIGN, "=", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_PLUS, "+", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_COMMA, ",", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "E_MN", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_ASSIGN, "=", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_MINUS, "-", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_COMMA, ",", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "E_C2", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_ASSIGN, "=", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token_len(tree, CDD_TOKEN_CHAR, "''", 2, &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_COMMA, ",", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "E_C3", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_ASSIGN, "=", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token_len(tree, CDD_TOKEN_CHAR, "'\\'", 3, &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_COMMA, ",", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));

      for (i = 0; i < 260; i++) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_create_token(tree, CDD_TOKEN_COMMA, ",", &tk));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      }
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(en, eblk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, en));

      /* 3b) Dedicated enum block with `E_LP = (` as final token (n == 1) */
      en = NULL;
      eblk = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_CLASS_DECLARATION, &en));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_create_token(
                                   tree, CDD_TOKEN_KEYWORD_ENUM, "enum", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(en, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_BLOCK, &eblk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "E_LP2", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_ASSIGN, "=", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_LPAREN, "(", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(eblk, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(en, eblk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, en));

      /* 4) Struct with >32 pre-tokens, non-BLOCK child, 2 BLOCKs, >64
       * post-tokens */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_CLASS_DECLARATION, &sn));
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_create_token(tree, CDD_TOKEN_KEYWORD_STRUCT, "struct", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(sn, tk));
      for (i = 0; i < 35; i++) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_create_token(tree, CDD_TOKEN_STAR, "*", &tk));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(sn, tk));
      }
      {
        cdd_cst_node_t *s_unk = NULL;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &s_unk));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(sn, s_unk));
      }
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_BLOCK, &sblk1));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_BLOCK, &sblk2));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(sn, sblk1));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(sn, sblk2));
      for (i = 0; i < 66; i++) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_create_token(tree, CDD_TOKEN_COMMA, ",", &tk));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(sn, tk));
      }
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, sn));

      /* 5) Function with unmatched ']' inside param list and `for + ...` */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &fn));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_KEYWORD_INT, "int", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(fn, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "f_ub", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(fn, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_LPAREN, "(", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(fn, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_RBRACKET, "]", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(fn, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_RPAREN, ")", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(fn, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, fn));

      fn = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &fn));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "for", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(fn, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_PLUS, "+", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(fn, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_NUMBER, "1", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(fn, tk));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_SEMICOLON, ";", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(fn, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, fn));

      /* 6) Preproc directive variants: empty, child node, non-include,
            unclosed quote, and >=256-byte header path */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &pd));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, pd));

      pd = NULL;
      un = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &pd));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &un));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(pd, un));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, pd));

      pd = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &pd));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_create_token(tree, CDD_TOKEN_OTHER,
                                                    "#pragma once", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(pd, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, pd));

      pd = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &pd));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_create_token(tree, CDD_TOKEN_PREPROC_INCLUDE,
                                     "#include \"unclosed", &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(pd, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, pd));

      memset(long_inc, 'h', sizeof(long_inc) - 1);
      memcpy(long_inc, "#include \"", 10);
      long_inc[sizeof(long_inc) - 2] = '"';
      long_inc[sizeof(long_inc) - 1] = '\0';
      pd = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &pd));
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_create_token(tree, CDD_TOKEN_PREPROC_INCLUDE, long_inc, &tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(pd, tk));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, pd));
    }

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, inc_dirs, 1, &env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(env));
    env = NULL;

    /* Also test tree->base_tokens != NULL with tree->root == NULL */
    {
      cdd_cst_node_t *saved_root = tree->root;
      tree->root = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, NULL, 0, &env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(env));
      env = NULL;
      tree->root = saved_root;
    }

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  PASS();
}

/**
 * @brief Executes the cdd_cst_semantic_suite test suite.
 */
SUITE(cdd_cst_semantic_suite) {
  RUN_TEST(test_cdd_cst_semantic_extract_null);
  RUN_TEST(test_cdd_cst_semantic_scope_basic);
  RUN_TEST(test_cdd_cst_semantic_basic);
  RUN_TEST(test_cdd_cst_semantic_tree);
  RUN_TEST(test_cdd_cst_semantic_errors);
  RUN_TEST(test_cdd_cst_semantic_c_declarations_and_headers);
  RUN_TEST(test_cdd_cst_semantic_remaining_edge_branches);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_cdd_cst_semantic_oom);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CDD_CST_SEMANTIC_H */
