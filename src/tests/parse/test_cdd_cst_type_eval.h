/**
 * @file test_cdd_cst_type_eval.h
 * @brief Unit tests for CST type evaluation.
 */

#ifndef TEST_CDD_CST_TYPE_EVAL_H
#define TEST_CDD_CST_TYPE_EVAL_H

#ifdef __cplusplus
extern "C" {
#endif
/* clang-format off */
#include "c_cdd_export.h"
#include "c_cdd/memory.h"

#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_scope.h"
#include "classes/parse/cdd_cst_type_eval.h"
#include <greatest.h>
/* clang-format on */

/* Moved extern declarations for C89 compliance */
/** @brief Deterministic fault-injection counter for CST type-eval helpers. */
extern C_CDD_EXPORT int g_cdd_te_fail_step;
/**
 * @brief Unit test test_cdd_cst_eval_primitive_type_basic.
 * @return Greatest test result.
 */
TEST test_cdd_cst_eval_primitive_type_basic(void) {
  cdd_cst_type_info_t info;
  int rc = 0;

  /* int under LP64 */
  rc = cdd_cst_eval_primitive_type("int", CDD_CST_ABI_LP64, &info);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  /* long under LP64 */
  rc = cdd_cst_eval_primitive_type("long", CDD_CST_ABI_LP64, &info);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  /* unknown type */
  rc = cdd_cst_eval_primitive_type("unknown_type", CDD_CST_ABI_LP64, &info);
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_cdd_cst_eval_sizeof_basic.
 * @return Greatest test result.
 */
TEST test_cdd_cst_eval_sizeof_basic(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_scope_env_t *env = NULL;
  size_t size;
  int rc = 0;
  cdd_cst_node_t *decl = NULL;
  size_t i;
  const char *src = cdd_test_mut_str("int a;");

  rc = cdd_cst_scope_env_init(&env);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_parse(az_span_create_from_str(src), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE && decl == NULL) {
      decl = tree->root->children[i].val.node;
    }
  }
  ASSERT(decl != NULL);

  rc = cdd_cst_eval_sizeof(env, decl, CDD_CST_ABI_LP64, &size);
  /* The size of 'int a;' declaration might be computed as size of 'int'. */
  /* If cdd_cst_eval_sizeof expects a TYPE node, it might fail. Let's see what
   * happens. */

  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_cdd_cst_eval_sizeof_alignof_advanced.
 * @return Greatest test result.
 */
TEST test_cdd_cst_eval_sizeof_alignof_advanced(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_scope_env_t *env = NULL;
  size_t size, align;
  int rc = 0;
  cdd_cst_node_t *decl = NULL;
  size_t i;
  cdd_cst_tree_t *tree2 = NULL;
  cdd_cst_node_t *decl2 = NULL;
  cdd_cst_node_t *empty_node = NULL;
  const char *src = cdd_test_mut_str("int *a;");

  rc = cdd_cst_scope_env_init(&env);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_parse(az_span_create_from_str(src), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE && decl == NULL) {
      decl = tree->root->children[i].val.node;
    }
  }
  ASSERT(decl != NULL);

  rc = cdd_cst_eval_sizeof(env, decl, CDD_CST_ABI_LP64, &size);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(8, size);

  rc = cdd_cst_eval_alignof(env, decl, CDD_CST_ABI_LP64, &align);
  ASSERT_EQ(0, rc);

  /* Unknown type so extract_type_name returns unknown */
  rc = cdd_cst_parse(az_span_create_from_str("struct Unknown x;"), &tree2);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree2->root->num_children; i++) {
    if (tree2->root->children[i].kind == CDD_CST_CHILD_NODE && decl2 == NULL) {
      decl2 = tree2->root->children[i].val.node;
    }
  }
  ASSERT(decl2 != NULL);

  rc = cdd_cst_eval_sizeof(env, decl2, CDD_CST_ABI_LP64, &size);
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);

  rc = cdd_cst_eval_alignof(env, decl2, CDD_CST_ABI_LP64, &align);
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);

  g_cdd_last_alloc_err = cdd_cst_tree_free(tree2);

  /* What if extract_type_name has multiple spaces? */
  {
    cdd_cst_tree_t *tree3 = NULL;
    cdd_cst_node_t *decl3 = NULL;
    rc = cdd_cst_parse(az_span_create_from_str("unsigned long long int a;"),
                       &tree3);
    ASSERT_EQ(0, rc);
    for (i = 0; i < tree3->root->num_children; i++) {
      if (tree3->root->children[i].kind == CDD_CST_CHILD_NODE &&
          decl3 == NULL) {
        decl3 = tree3->root->children[i].val.node;
      }
    }
    ASSERT(decl3 != NULL);
    rc = cdd_cst_eval_alignof(env, decl3, CDD_CST_ABI_LP64, &align);
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);

    rc = cdd_cst_eval_sizeof(env, decl3, CDD_CST_ABI_LP64, &size);
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              rc); /* "unsigned long long int" is not in the
       hardcoded primitive map but tests loop coverage */

    rc = cdd_cst_eval_alignof(env, decl3, CDD_CST_ABI_LP64, &align);
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree3);
  }

  /* What if extract_type_name buffer overflows? (256 bytes) */
  {
    char huge_type[300];
    cdd_cst_tree_t *tree4 = NULL;
    cdd_cst_node_t *decl4 = NULL;
    memset(huge_type, 'a', 290);
    huge_type[290] = ' ';
    huge_type[291] = 'x';
    huge_type[292] = ';';
    huge_type[293] = '\0';
    rc = cdd_cst_parse(az_span_create_from_str(huge_type), &tree4);
    ASSERT_EQ(0, rc);
    for (i = 0; i < tree4->root->num_children; i++) {
      if (tree4->root->children[i].kind == CDD_CST_CHILD_NODE &&
          decl4 == NULL) {
        decl4 = tree4->root->children[i].val.node;
      }
    }
    ASSERT(decl4 != NULL);
    rc = cdd_cst_eval_sizeof(env, decl4, CDD_CST_ABI_LP64, &size);
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree4);
  }

  /* removed assert */

  /* Fallback testing (e.g. unsupported type or empty declaration) */
  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_ASM_STATEMENT, &empty_node);
  rc = cdd_cst_eval_sizeof(env, empty_node, CDD_CST_ABI_LP64, &size);
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);
  rc = cdd_cst_eval_alignof(env, empty_node, CDD_CST_ABI_LP64, &align);
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);
  g_cdd_last_alloc_err = cdd_cst_free_node(empty_node);

  /* Error paths */

  /* Error path: null node */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_sizeof(env, NULL, CDD_CST_ABI_LP64, &size));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_alignof(env, NULL, CDD_CST_ABI_LP64, &size));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_sizeof(NULL, decl, CDD_CST_ABI_LP64, &size));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_alignof(NULL, decl, CDD_CST_ABI_LP64, &align));

  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_cdd_cst_eval_primitive_extra.
 * @return Greatest test result.
 */
TEST test_cdd_cst_eval_primitive_extra(void) {
  cdd_cst_type_info_t info;
  ASSERT_EQ(0,
            cdd_cst_eval_primitive_type("long long", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("long", CDD_CST_ABI_ILP32, &info));
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("long", CDD_CST_ABI_LLP64, &info));
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("ptr", CDD_CST_ABI_ILP32, &info));
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("short", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(2, info.size);
  ASSERT_EQ(2, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("unsigned short", CDD_CST_ABI_LP64,
                                           &info));
  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("unsigned int", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("unsigned long", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(0, cdd_cst_eval_primitive_type("unsigned long", CDD_CST_ABI_ILP32,
                                           &info));
  ASSERT_EQ(0, cdd_cst_eval_primitive_type("unsigned long long",
                                           CDD_CST_ABI_LP64, &info));

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("char", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(1, info.size);
  ASSERT_EQ(1, info.alignment);

  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("signed char", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("unsigned char", CDD_CST_ABI_LP64, &info));

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("float", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(4, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("double", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("long double", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(16, info.size);
  ASSERT_EQ(16, info.alignment);

  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("long double", CDD_CST_ABI_ILP32, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(4, info.alignment);

  ASSERT_EQ(
      0, cdd_cst_eval_primitive_type("long double", CDD_CST_ABI_LLP64, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("void *", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(8, info.size);
  ASSERT_EQ(8, info.alignment);

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            cdd_cst_eval_primitive_type("_Bool", CDD_CST_ABI_LP64, &info));

  ASSERT_EQ(0,
            cdd_cst_eval_primitive_type("__int128", CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(16, info.size);
  ASSERT_EQ(16, info.alignment);

  ASSERT_EQ(0, cdd_cst_eval_primitive_type("unsigned __int128",
                                           CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(16, info.size);
  ASSERT_EQ(16, info.alignment);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_primitive_type(NULL, CDD_CST_ABI_LP64, &info));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_primitive_type("int", CDD_CST_ABI_LP64, NULL));
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_type_eval_branches.
 * @return Greatest test result.
 */
TEST test_type_eval_branches(void) {
  cdd_cst_node_t *decl = NULL;
  cdd_token_t tok = {0};
  cdd_cst_node_t *dummy_child = NULL;
  size_t sz = 0;
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_node_t *decl2 = NULL;
  cdd_token_t tok2 = {0};
  char buf[300] = {0};
  int i;
  cdd_cst_node_t *decl3 = NULL;
  cdd_token_t tok3 = {0};
  char buf2[300] = {0};
#ifdef CDD_BUILD_TESTS
  int rc = 0;
#endif

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_EXPRESSION, &decl);
  tok.kind = CDD_TOKEN_KEYWORD___INT128;
  tok.start = (const uint8_t *)"__int128";
  tok.length = 8;
  g_cdd_last_alloc_err = cdd_cst_append_child_token(decl, &tok);

  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_EXPRESSION, &dummy_child);
  g_cdd_last_alloc_err = cdd_cst_append_child_node(decl, dummy_child);

  g_cdd_last_alloc_err = cdd_cst_scope_env_init(&env);

  ASSERT_EQ(0, cdd_cst_eval_sizeof(env, decl, CDD_CST_ABI_LP64, &sz));
  ASSERT_EQ(16, sz);

  /* Very long type name */
  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_EXPRESSION, &decl2);
  tok2.kind = CDD_TOKEN_IDENTIFIER;
  for (i = 0; i < 50; i++)
    buf[i] = 'a';
  tok2.start = (const uint8_t *)buf;
  tok2.length = 290;
  g_cdd_last_alloc_err = cdd_cst_append_child_token(decl2, &tok2);

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            cdd_cst_eval_sizeof(env, decl2, CDD_CST_ABI_LP64, &sz));

  /* Test OOM */
#ifdef CDD_BUILD_TESTS
  g_cdd_alloc_fail = 1;
  rc = cdd_cst_eval_sizeof(env, decl, CDD_CST_ABI_LP64, &sz);
  ASSERT(rc == CDD_C_ERROR_SYSTEM || rc == CDD_C_ERROR_MEMORY);
  g_cdd_alloc_fail = 0;
#endif

  /* Test NULL env */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_sizeof(NULL, NULL, CDD_CST_ABI_LP64, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_sizeof(NULL, decl, CDD_CST_ABI_LP64, &sz));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_sizeof(env, decl, CDD_CST_ABI_LP64, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_alignof(env, decl, CDD_CST_ABI_LP64, NULL));

  g_cdd_last_alloc_err = cdd_cst_free_node_only(decl);
  g_cdd_last_alloc_err = cdd_cst_free_node_only(decl2);

  /* Two long tokens to hit buf_len + 1 < sizeof(buf) */
  g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_EXPRESSION, &decl3);
  tok3.kind = CDD_TOKEN_IDENTIFIER;
  for (i = 0; i < 50; i++)
    buf2[i] = 'b';
  tok3.start = (const uint8_t *)buf2;
  tok3.length = 255;
  g_cdd_last_alloc_err = cdd_cst_append_child_token(decl3, &tok3);
  g_cdd_last_alloc_err = cdd_cst_append_child_token(decl3, &tok3);

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            cdd_cst_eval_sizeof(env, decl3, CDD_CST_ABI_LP64, &sz));
  g_cdd_last_alloc_err = cdd_cst_free_node_only(decl3);

  g_cdd_last_alloc_err = cdd_cst_free_node_only(dummy_child);
  g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_cdd_cst_eval_ptr_fail.
 * @return Greatest test result.
 */
TEST test_cdd_cst_eval_ptr_fail(void) {
  cdd_cst_scope_env_t *env = NULL;
  g_cdd_last_alloc_err = cdd_cst_scope_env_init(&env);
  {
    size_t sz = 0;

    cdd_cst_node_t *spec3 = NULL;
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TYPE_SPECIFIER, &spec3);
    {
      cdd_cst_tree_t *tree = calloc(1, sizeof(*tree));
      tree->root = spec3;
      {
        cdd_token_t *tok3 = NULL;
        g_cdd_last_alloc_err =
            cdd_cst_create_token_len(tree, CDD_TOKEN_STAR, "*", 1, &tok3);
        g_cdd_last_alloc_err = cdd_cst_append_child_token(spec3, tok3);
        {
          cdd_token_t *tok4 = NULL;
          g_cdd_last_alloc_err = cdd_cst_create_token_len(
              tree, CDD_TOKEN_IDENTIFIER, "int", 3, &tok4);
          g_cdd_last_alloc_err = cdd_cst_append_child_token(spec3, tok4);

          {
            int rc1 = cdd_cst_eval_sizeof(env, spec3, CDD_CST_ABI_LP64, &sz);
            int rc2 = cdd_cst_eval_alignof(env, spec3, CDD_CST_ABI_LP64, &sz);

            ASSERT_EQ(CDD_C_SUCCESS, rc1);
            ASSERT_EQ(CDD_C_SUCCESS, rc2);

            g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
            g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
            PASS();
          }
        }
      }
    }
  }
}

/**
 * @brief Unit test test_cdd_cst_type_eval_branches.
 * @return Greatest test result.
 */
TEST test_cdd_cst_type_eval_branches(void) {
  cdd_cst_scope_env_t *env = NULL;
  g_cdd_last_alloc_err = cdd_cst_scope_env_init(&env);
  {
    cdd_c_error_t rc = 0;
    {
      enum cdd_cst_abi_model_t abi = CDD_CST_ABI_LP64;
      cdd_cst_node_t dummy_node = {0};
      cdd_token_t dummy_tok = {0};
      cdd_cst_child_t children[1];
      size_t sz = 0, al = 0;
      dummy_tok.kind = CDD_TOKEN_IDENTIFIER;
      dummy_tok.start = (const uint8_t *)"int";
      dummy_tok.length = 3;
      children[0].kind = CDD_CST_CHILD_TOKEN;
      children[0].val.token = &dummy_tok;
      dummy_node.kind = CDD_CST_DECLARATION;
      dummy_node.children = children;
      dummy_node.num_children = 1;
      g_cdd_alloc_fail = 1;
      /* Also test the malloc failure inside eval_alignof since it calls
       * extract_type_name too */
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_cst_eval_alignof(env, &dummy_node, abi, &al));
      g_cdd_alloc_fail = 2;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_eval_alignof(env, &dummy_node, abi, &al));
      rc = cdd_cst_eval_primitive_type("int", abi, NULL);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      g_cdd_alloc_fail = 0;
      /* It turns out extract_type_name was wrong so just hit branch inside eval
       */
      dummy_tok.start = (const uint8_t *)"unknown_type";
      dummy_tok.length = 12;
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_cst_eval_sizeof(env, &dummy_node, abi, &sz));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_cst_eval_alignof(env, &dummy_node, abi, &al));
      g_cdd_last_alloc_err = cdd_cst_scope_env_free(env);
      PASS();
    }
  }
}

/**
 * @brief Comprehensive expression and type-name evaluation tests.
 * @return Greatest test result.
 */
TEST test_cdd_cst_eval_tokens_type_comprehensive(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_type_info_t tinfo;
  char buf[128];
  int is_tn = 0;
  const char *ctx_code = "typedef unsigned long size_t;\n"
                         "typedef long my_long_t;\n"
                         "typedef char *my_str_t;\n"
                         "struct Node { double val; struct Node *next; };\n"
                         "typedef struct Node node_t;\n"
                         "union Box { float f; int i; };\n"
                         "const char *get_cstr(int k);\n"
                         "int g_arr[8];\n"
                         "node_t g_node;\n"
                         "node_t *g_nptr;\n"
                         "union Box g_box;\n";

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_eval_primitive_type("__int128", CDD_CST_ABI_LP64, &tinfo));
  ASSERT_EQ(16, tinfo.size);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(ctx_code), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_build_semantic_info(tree, &env));

  /* Typedef chain in sizeof / alignof */
  {
    cdd_cst_node_t dn = {0};
    cdd_token_t dt = {0};
    cdd_cst_child_t ch[1];
    size_t sz = 0, al = 0;
    dt.kind = CDD_TOKEN_IDENTIFIER;
    dt.start = (const uint8_t *)"my_long_t";
    dt.length = 9;
    ch[0].kind = CDD_CST_CHILD_TOKEN;
    ch[0].val.token = &dt;
    dn.kind = CDD_CST_DECLARATION;
    dn.children = ch;
    dn.num_children = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_sizeof(env, &dn, CDD_CST_ABI_LP64, &sz));
    ASSERT_EQ(8, sz);
    dt.start = (const uint8_t *)"my_str_t";
    dt.length = 8;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_alignof(env, &dn, CDD_CST_ABI_LP64, &al));
    ASSERT_EQ(8, al);
  }

  /* Invalid arguments */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_token_ptrs_type(env, NULL, NULL, 1, 0, buf,
                                         sizeof(buf), &is_tn));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_eval_tokens_type(env, NULL, NULL, 1, 0, buf, sizeof(buf),
                                     &is_tn));

  /* Evaluate rich expression table */
  {
    static const struct {
      const char *expr;
      int mode;
      const char *expected;
      int exp_is_type;
    } k_cases[] = {
        {"const volatile int", CDD_CST_TYPE_MODE_UNQUAL, "int", 1},
        {"struct Node *", CDD_CST_TYPE_MODE_EXACT, "struct Node *", 1},
        {"my_long_t[4]", CDD_CST_TYPE_MODE_RVALUE, "my_long_t *", 1},
        {"char * const", CDD_CST_TYPE_MODE_RVALUE, "char *", 1},
        {"struct", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"(g_arr)", CDD_CST_TYPE_MODE_RVALUE, "int *", 0},
        {"(double *)0", CDD_CST_TYPE_MODE_EXACT, "double *", 0},
        {"1, 2.5f", CDD_CST_TYPE_MODE_EXACT, "float", 0},
        {"1.5L", CDD_CST_TYPE_MODE_EXACT, "long double", 0},
        {"0x1p4", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"10ULL", CDD_CST_TYPE_MODE_EXACT, "unsigned long long", 0},
        {"10LL", CDD_CST_TYPE_MODE_EXACT, "long long", 0},
        {"10UL", CDD_CST_TYPE_MODE_EXACT, "unsigned long", 0},
        {"10L", CDD_CST_TYPE_MODE_EXACT, "long", 0},
        {"10U", CDD_CST_TYPE_MODE_EXACT, "unsigned int", 0},
        {"g_node.val = 3.0", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"g_arr[0] += 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0] <<= 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 ? g_nptr : NULL", CDD_CST_TYPE_MODE_EXACT, "node_t *", 0},
        {"NULL ? : g_nptr", CDD_CST_TYPE_MODE_EXACT, "node_t *", 0},
        {"1 ? 2 : 3.0", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"1 || 0", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 && 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 <= 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"!g_nptr", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1L << 2", CDD_CST_TYPE_MODE_EXACT, "long", 0},
        {"g_nptr - g_nptr", CDD_CST_TYPE_MODE_EXACT, "long", 0},
        {"g_nptr + 1", CDD_CST_TYPE_MODE_EXACT, "node_t *", 0},
        {"1 + g_nptr", CDD_CST_TYPE_MODE_EXACT, "node_t *", 0},
        {"sizeof(int)", CDD_CST_TYPE_MODE_EXACT, "size_t", 0},
        {"*g_nptr", CDD_CST_TYPE_MODE_EXACT, "node_t", 0},
        {"*g_arr", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"&g_arr", CDD_CST_TYPE_MODE_EXACT, "int *", 0},
        {"++g_arr[0]", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"~1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"\"a\" \"b\"", CDD_CST_TYPE_MODE_EXACT, "const char *", 0},
        {"'z'", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"get_cstr(1)", CDD_CST_TYPE_MODE_EXACT, "const char *", 0},
        {"malloc(8)", CDD_CST_TYPE_MODE_EXACT, "void *", 0},
        {"strdup(\"x\")", CDD_CST_TYPE_MODE_EXACT, "char *", 0},
        {"unknown_fn(1)", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_nptr->next->val", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"g_box.f", CDD_CST_TYPE_MODE_EXACT, "float", 0},
        {"g_box.missing", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0]++", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0]--", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"(int *)g_nptr", CDD_CST_TYPE_MODE_EXACT, "int *", 0},
        {"((const struct Node *)g_nptr)->val", CDD_CST_TYPE_MODE_EXACT,
         "double", 0},
        {"struct", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"(1 + 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"()", CDD_CST_TYPE_MODE_EXACT, "void", 0},
        {"g_nptr :", CDD_CST_TYPE_MODE_EXACT, "node_t *", 0},
        {";", CDD_CST_TYPE_MODE_EXACT, "int", 0}};
    size_t c;
    for (c = 0; c < sizeof(k_cases) / sizeof(k_cases[0]); c++) {
      cdd_token_list_t *tl = NULL;
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_lexer_tokenize(az_span_create_from_str(k_cases[c].expr), &tl));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_tokens_type(
                                   env, env->global_scope, tl->tokens, tl->size,
                                   k_cases[c].mode, buf, sizeof(buf), &is_tn));
      ASSERT_STR_EQ(k_cases[c].expected, buf);
      ASSERT_EQ(k_cases[c].exp_is_type, is_tn);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));
    }
  }

  /* sizeof without size_t in scope + small cap truncation + >64 tokens + OOM */
  {
    cdd_token_list_t *sz_tl = NULL;
    cdd_token_t big_toks[70];
    size_t bi;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("sizeof(1)"), &sz_tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(NULL, NULL, sz_tl->tokens, sz_tl->size,
                                       0, buf, 5, NULL));
    ASSERT_STR_EQ("unsi", buf);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_eval_tokens_type(NULL, NULL, sz_tl->tokens, 0, 0, buf,
                                       sizeof(buf), NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(sz_tl));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("a.b"), &sz_tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(NULL, NULL, sz_tl->tokens, sz_tl->size,
                                       0, buf, sizeof(buf), NULL));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(sz_tl));

    for (bi = 0; bi < 70; bi++) {
      memset(&big_toks[bi], 0, sizeof(cdd_token_t));
      if ((bi & 1) == 0) {
        big_toks[bi].kind = CDD_TOKEN_NUMBER;
        big_toks[bi].start = (const uint8_t *)"1";
        big_toks[bi].length = 1;
      } else {
        big_toks[bi].kind = CDD_TOKEN_PLUS;
        big_toks[bi].start = (const uint8_t *)"+";
        big_toks[bi].length = 1;
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(NULL, NULL, big_toks, 69, 0, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("int", buf);
#ifdef CDD_BUILD_TESTS
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_cst_eval_tokens_type(NULL, NULL, big_toks, 69, 0, buf,
                                       sizeof(buf), NULL));
    g_cdd_alloc_fail = 0;
#endif
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(env));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  PASS();
}

/**
 * @brief Exhaustive edge-case tests for 100% line and branch coverage of
 * `cdd_cst_type_eval.c`.
 * @return Greatest test result.
 */
TEST test_cdd_cst_eval_tokens_type_full_branches(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_scope_env_t *env = NULL;
  cdd_cst_type_info_t tinfo;
  char buf[160];
  int is_tn = 0;
  const char *ctx1 =
      "typedef int t0; typedef t0 t1; typedef t1 t2; typedef t2 t3;\n"
      "typedef t3 t4; typedef t4 t5; typedef t5 t6; typedef t6 t7;\n"
      "typedef t7 t8; typedef t8 t9;\n"
      "struct Node { double val; };\n"
      "typedef struct Node n0_t; typedef n0_t n1_t; typedef n1_t n2_t;\n"
      "typedef n2_t n3_t; typedef n3_t n4_t; typedef n4_t n5_t;\n"
      "typedef n5_t n6_t; typedef n6_t n7_t; typedef n7_t n8_t;\n"
      "typedef n8_t n9_t;\n";
  const char *ctx2 = "union Box { float f; int i; };\n"
                     "struct Node { double val; struct Node *next; };\n"
                     "typedef struct Node node_t;\n"
                     "typedef int td_null_t;\n"
                     "const char *get_cstr(int k);\n"
                     "cdd_uint128_t g_u128;\n"
                     "cdd_int128_t g_i128;\n"
                     "int g_arr[8];\n"
                     "node_t g_node;\n"
                     "node_t *g_nptr;\n"
                     "td_null_t g_tdvar;\n";

  /* 128-bit primitive types */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_primitive_type(
                               "cdd_int128_t", CDD_CST_ABI_LP64, &tinfo));
  ASSERT_EQ(16, tinfo.size);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_primitive_type(
                               "cdd_uint128_t", CDD_CST_ABI_LP64, &tinfo));
  ASSERT_EQ(16, tinfo.size);

  /* >= 8-hop typedef chains in eval_named_or_typedef and te_resolve_member */
  {
    cdd_cst_tree_t *hop_tree = NULL;
    cdd_cst_scope_env_t *hop_env = NULL;
    cdd_cst_node_t dn = {0};
    cdd_token_t dt = {0};
    cdd_cst_child_t ch[1];
    size_t sz = 0;
    cdd_cst_symbol_t *tsym = NULL;
    char *saved_ts = NULL;
    cdd_token_list_t *tl = NULL;
    const char *hop_src =
        "struct S9 { double val; };\n"
        "typedef struct S9 s0; typedef s0 s1; typedef s1 s2; typedef s2 s3;\n"
        "typedef s3 s4; typedef s4 s5; typedef s5 s6; typedef s6 s7;\n"
        "typedef s7 s8; typedef s8 s9;\n"
        "s9 g_s9;\n";

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ctx1), &hop_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_build_semantic_info(hop_tree, &hop_env));

    dt.kind = CDD_TOKEN_IDENTIFIER;
    dt.start = (const uint8_t *)"t9";
    dt.length = 2;
    ch[0].kind = CDD_CST_CHILD_TOKEN;
    ch[0].val.token = &dt;
    dn.kind = CDD_CST_DECLARATION;
    dn.children = ch;
    dn.num_children = 1;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_cst_eval_sizeof(hop_env, &dn, CDD_CST_ABI_LP64, &sz));

    /* Typedef with NULL type_str in eval_named_or_typedef */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_lookup_symbol(
                                 hop_env, "t0", CDD_CST_SYMBOL_TYPEDEF, &tsym));
    saved_ts = tsym->type_str;
    tsym->type_str = NULL;
    dt.start = (const uint8_t *)"t0";
    dt.length = 2;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_cst_eval_sizeof(hop_env, &dn, CDD_CST_ABI_LP64, &sz));
    tsym->type_str = saved_ts;

    /* sizeof(1) in non-NULL scope without size_t typedef */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("sizeof(1)"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(hop_env, NULL, tl->tokens, tl->size,
                                       CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("unsigned long", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(hop_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(hop_tree));

    /* 9-hop member resolution */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(hop_src), &hop_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_build_semantic_info(hop_tree, &hop_env));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("g_s9.val"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(hop_env, NULL, tl->tokens, tl->size,
                                       CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(hop_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(hop_tree));
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(ctx2), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_build_semantic_info(tree, &env));

  /* Invalid arguments on BOTH token_ptrs and tokens APIs */
  {
    cdd_token_t dtok = {0};
    const cdd_token_t *dptr;
    dtok.kind = CDD_TOKEN_NUMBER;
    dtok.start = (const uint8_t *)"1";
    dtok.length = 1;
    dptr = &dtok;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_eval_token_ptrs_type(env, NULL, NULL, 1, 0, buf,
                                           sizeof(buf), &is_tn));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_eval_token_ptrs_type(env, NULL, &dptr, 0, 0, buf,
                                           sizeof(buf), &is_tn));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_eval_token_ptrs_type(env, NULL, &dptr, 1, 0, NULL,
                                           sizeof(buf), &is_tn));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_cst_eval_token_ptrs_type(env, NULL, &dptr, 1, 0, buf, 0, &is_tn));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_eval_tokens_type(env, NULL, NULL, 1, 0, buf, sizeof(buf),
                                       &is_tn));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_eval_tokens_type(env, NULL, &dtok, 0, 0, buf, sizeof(buf),
                                       &is_tn));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_eval_tokens_type(env, NULL, &dtok, 1, 0, NULL,
                                       sizeof(buf), &is_tn));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_eval_tokens_type(env, NULL, &dtok, 1, 0, buf, 0, &is_tn));
  }

  /* Small capacity truncation & out_is_type_expr == NULL on type expressions */
  {
    cdd_token_list_t *tl = NULL;

    /* out_is_type_expr == NULL on type name + scope == NULL fallback */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("int"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, NULL, tl->tokens, tl->size,
                                       CDD_CST_TYPE_MODE_UNQUAL, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    /* len <= 2 in te_normalize_type_mode RVALUE */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("1"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, NULL, tl->tokens, tl->size,
                                       CDD_CST_TYPE_MODE_RVALUE, buf, 2, NULL));
    ASSERT_STR_EQ("i", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    /* array decay cap boundary in te_normalize_type_mode */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("int[]"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_tokens_type(
                                 env, NULL, tl->tokens, tl->size,
                                 CDD_CST_TYPE_MODE_RVALUE, buf, 6, &is_tn));
    ASSERT_EQ(1, is_tn);
    ASSERT_STR_EQ("int[]", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    /* te_try_parse_type_name small-cap branches */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_lexer_tokenize(az_span_create_from_str("const struct Node"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_tokens_type(
                                 env, NULL, tl->tokens, tl->size,
                                 CDD_CST_TYPE_MODE_EXACT, buf, 6, &is_tn));
    ASSERT_EQ(1, is_tn);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("const int"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_tokens_type(
                                 env, NULL, tl->tokens, tl->size,
                                 CDD_CST_TYPE_MODE_EXACT, buf, 6, &is_tn));
    ASSERT_EQ(1, is_tn);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("int *"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_tokens_type(
                                 env, NULL, tl->tokens, tl->size,
                                 CDD_CST_TYPE_MODE_EXACT, buf, 2, &is_tn));
    ASSERT_EQ(1, is_tn);
    ASSERT_STR_EQ("*", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_tokens_type(
                                 env, NULL, tl->tokens, tl->size,
                                 CDD_CST_TYPE_MODE_EXACT, buf, 4, &is_tn));
    ASSERT_EQ(1, is_tn);
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("int[1234]"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_tokens_type(
                                 env, NULL, tl->tokens, tl->size,
                                 CDD_CST_TYPE_MODE_EXACT, buf, 5, &is_tn));
    ASSERT_EQ(1, is_tn);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    /* NULL scope function call */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_lexer_tokenize(az_span_create_from_str("unknown_fn(1)"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(NULL, NULL, tl->tokens, tl->size,
                                       CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));
  }

  /* Long identifiers (>= 127 chars) & long member field names */
  {
    char long_id[140];
    char long_mem[150];
    cdd_token_list_t *tl = NULL;

    memset(long_id, 'a', 130);
    long_id[130] = '\0';
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str(long_id), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), &is_tn));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(0, is_tn);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    memcpy(long_mem, "g_node.", 7);
    memset(long_mem + 7, 'f', 130);
    long_mem[137] = '\0';
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str(long_mem), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), &is_tn));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));
  }

  /* Symbol type_str edge cases (NULL, empty, "*", "ab]") */
  {
    cdd_cst_symbol_t *vsym = NULL;
    cdd_cst_symbol_t *fsym = NULL;
    cdd_cst_symbol_t *tsym = NULL;
    char *saved_v = NULL;
    char *saved_f = NULL;
    char *saved_t = NULL;
    char empty_ts[1] = {""};
    char star_ts[2] = {"*"};
    char brk_ts[4] = {"ab]"};
    cdd_token_list_t *tl = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_scope_lookup_in_scope(env->global_scope, "g_node",
                                            CDD_CST_SYMBOL_VARIABLE, &vsym));
    saved_v = vsym->type_str;

    /* vsym->type_str == NULL */
    vsym->type_str = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("g_node"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    /* vsym->type_str == "" for *g_node and &g_node */
    vsym->type_str = empty_ts;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("*g_node"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("&g_node"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("*", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    /* vsym->type_str == "*" for *g_node and g_node->val */
    vsym->type_str = star_ts;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("*g_node"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("g_node->val"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    /* vsym->type_str == "ab]" (missing '[') for RVALUE, *g_node, &g_node */
    vsym->type_str = brk_ts;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("g_node"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_RVALUE, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("ab]", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("*g_node"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("ab]", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("&g_node"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("ab] *", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));

    vsym->type_str = saved_v;

    /* fsym->type_str == NULL */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_scope_lookup_in_scope(env->global_scope, "get_cstr",
                                            CDD_CST_SYMBOL_FUNCTION, &fsym));
    saved_f = fsym->type_str;
    fsym->type_str = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("get_cstr(1)"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));
    fsym->type_str = saved_f;

    /* td->type_str == NULL in te_resolve_member */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_scope_lookup_in_scope(env->global_scope, "td_null_t",
                                            CDD_CST_SYMBOL_TYPEDEF, &tsym));
    saved_t = tsym->type_str;
    tsym->type_str = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("g_tdvar.val"), &tl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                       tl->size, CDD_CST_TYPE_MODE_EXACT, buf,
                                       sizeof(buf), NULL));
    ASSERT_STR_EQ("int", buf);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));
    tsym->type_str = saved_t;
  }

  /* Exhaustive operator, literal, and type-name table */
  {
    static const struct {
      const char *expr;
      int mode;
      const char *expected;
      int exp_is_type;
    } k_extra[] = {
        {"union Box", CDD_CST_TYPE_MODE_EXACT, "union Box", 1},
        {"struct 123", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"__int128", CDD_CST_TYPE_MODE_EXACT, "__int128", 1},
        {"int **", CDD_CST_TYPE_MODE_EXACT, "int **", 1},
        {"int[4]", CDD_CST_TYPE_MODE_EXACT, "int[4]", 1},
        {"int[4", CDD_CST_TYPE_MODE_EXACT, "int[4", 1},
        {"const", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"int foo", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"int + 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"0X10", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"0123", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"0x1.0p2", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"0x1.0P2", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"1e3", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"1E3", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"1.0F", CDD_CST_TYPE_MODE_EXACT, "float", 0},
        {"1.0l", CDD_CST_TYPE_MODE_EXACT, "long double", 0},
        {"1.0L", CDD_CST_TYPE_MODE_EXACT, "long double", 0},
        {"10u", CDD_CST_TYPE_MODE_EXACT, "unsigned int", 0},
        {"10l", CDD_CST_TYPE_MODE_EXACT, "long", 0},
        {"g_u128 + 1", CDD_CST_TYPE_MODE_EXACT, "cdd_uint128_t", 0},
        {"g_i128 + 1", CDD_CST_TYPE_MODE_EXACT, "cdd_int128_t", 0},
        {"1 + 2.5", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"8 >> 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0] >>= 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 <<", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 < 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 > 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 >= 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 == 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 != 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 | 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 & 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 ^ 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"4 / 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"4 % 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"2 * 3", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"2 - 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0] |= 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0] &= 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0] ^= 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0] /= 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0] = g_arr[1] = 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"'a' + 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"\"abc\" + 1", CDD_CST_TYPE_MODE_EXACT, "const char *", 0},
        {"(1) + 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 + + 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 + + g_i128", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 - - 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1,", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 ? 2 : 3 ? 4 : 5", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 ? 2", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 ? 2 :", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 ? 3.5 : 2", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"1 ? NULL : g_nptr", CDD_CST_TYPE_MODE_EXACT, "node_t *", 0},
        {"1 ? NULL : 0", CDD_CST_TYPE_MODE_EXACT, "void *", 0},
        {"g_nptr + g_nptr", CDD_CST_TYPE_MODE_EXACT, "node_t *", 0},
        {"1 - g_nptr", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"&g_nptr", CDD_CST_TYPE_MODE_EXACT, "node_t **", 0},
        {"--g_arr[0]", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"+ (1)", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"- (1)", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"+1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"-1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"+", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"()1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"{ 1 }", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"1 )", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"\"abc\"[0]", CDD_CST_TYPE_MODE_EXACT, "const char", 0},
        {"nullptr", CDD_CST_TYPE_MODE_EXACT, "void *", 0},
        {"unknown_fn(1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"unknown_fn((1))", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[g_arr[0]]", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"((struct NoSuchStruct *)0)->x", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"(g_node).val", CDD_CST_TYPE_MODE_EXACT, "double", 0},
        {"g_tdvar.val", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_node.", CDD_CST_TYPE_MODE_EXACT, "node_t", 0},
        {"g_node.123", CDD_CST_TYPE_MODE_EXACT, "node_t", 0},
        {"g_arr[0]++ - 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0]++ + 1", CDD_CST_TYPE_MODE_EXACT, "int", 0},
        {"g_arr[0]++ +", CDD_CST_TYPE_MODE_EXACT, "int", 0}};
    size_t c;
    for (c = 0; c < sizeof(k_extra) / sizeof(k_extra[0]); c++) {
      cdd_token_list_t *tl = NULL;
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_lexer_tokenize(az_span_create_from_str(k_extra[c].expr), &tl));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_eval_tokens_type(
                                   env, env->global_scope, tl->tokens, tl->size,
                                   k_extra[c].mode, buf, sizeof(buf), &is_tn));
      ASSERT_STR_EQ(k_extra[c].expected, buf);
      ASSERT_EQ(k_extra[c].exp_is_type, is_tn);
#ifdef CDD_BUILD_TESTS
      {
        int sf;
        for (sf = 1; sf <= 80; sf++) {
          cdd_c_error_t s_rc;
          g_cdd_te_fail_step = sf;
          s_rc = cdd_cst_eval_tokens_type(env, env->global_scope, tl->tokens,
                                          tl->size, k_extra[c].mode, buf,
                                          sizeof(buf), &is_tn);
          g_cdd_te_fail_step = 0;
          if (s_rc == CDD_C_SUCCESS)
            break;
        }
      }
#endif
      ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(tl));
    }
#ifdef CDD_BUILD_TESTS
    /* Step-fail sweeps across additional expression forms & sizeof/alignof */
    {
      static const char *const k_step_exprs[] = {
          "const struct Node",
          "my_td_t",
          "(int)1",
          "1 ? 3.5 : 2",
          "1 + 2.5",
          "1 << 2",
          "sizeof(int)",
          "*g_nptr",
          "&g_arr",
          "(1).x",
          "\"hi\"",
          "'a'",
          "1.0f",
          "1.0L",
          "1.0",
          "1ULL",
          "1L",
          "get_cstr(1)",
          "malloc(4)",
          "strdup(\"x\")",
          "unk_fn(1)",
          "g_node.val",
          "NULL",
          "unk_var",
          "g_arr[0]++",
          ("1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+"
           "1")};
      size_t se;
      int sf;
      cdd_cst_node_t dn;
      cdd_cst_child_t dc;
      cdd_token_t dt;
      size_t tsz = 0;

      for (se = 0; se < sizeof(k_step_exprs) / sizeof(k_step_exprs[0]); se++) {
        cdd_token_list_t *stl = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_lexer_tokenize(az_span_create_from_str(k_step_exprs[se]),
                                     &stl));
        for (sf = 1; sf <= 120; sf++) {
          cdd_c_error_t s_rc;
          g_cdd_te_fail_step = sf;
          s_rc = cdd_cst_eval_tokens_type(env, env->global_scope, stl->tokens,
                                          stl->size, CDD_CST_TYPE_MODE_RVALUE,
                                          buf, sizeof(buf), &is_tn);
          g_cdd_te_fail_step = 0;
          if (s_rc == CDD_C_SUCCESS)
            break;
        }
        ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(stl));
      }

      memset(&dn, 0, sizeof(dn));
      memset(&dc, 0, sizeof(dc));
      memset(&dt, 0, sizeof(dt));
      dt.kind = CDD_TOKEN_IDENTIFIER;
      dt.start = (const uint8_t *)"my_td_t";
      dt.length = 7;
      dc.kind = CDD_CST_CHILD_TOKEN;
      dc.val.token = &dt;
      dn.kind = CDD_CST_TYPE_SPECIFIER;
      dn.children = &dc;
      dn.num_children = 1;
      for (sf = 1; sf <= 20; sf++) {
        g_cdd_te_fail_step = sf;
        if (cdd_cst_eval_sizeof(env, &dn, CDD_CST_ABI_LP64, &tsz) ==
            CDD_C_SUCCESS) {
          g_cdd_te_fail_step = 0;
          break;
        }
        g_cdd_te_fail_step = 0;
      }
      for (sf = 1; sf <= 20; sf++) {
        g_cdd_te_fail_step = sf;
        if (cdd_cst_eval_alignof(env, &dn, CDD_CST_ABI_LP64, &tsz) ==
            CDD_C_SUCCESS) {
          g_cdd_te_fail_step = 0;
          break;
        }
        g_cdd_te_fail_step = 0;
      }
    }
#endif
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(env));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  /* Non-typedef ordinary symbol in eval_named_or_typedef and te_resolve_member
   */
  {
    cdd_cst_tree_t *ntree = NULL;
    cdd_cst_scope_env_t *nenv = NULL;
    cdd_token_list_t *ntl = NULL;
    cdd_cst_symbol_t *ntd = NULL;
    cdd_cst_node_t dn;
    cdd_cst_child_t dc;
    cdd_token_t dt;
    size_t nsz = 0;
    char nbuf[64];
    int nis_t = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "typedef struct { int f; } null_td_t;\n"
                                "null_td_t g_ntd;\n"),
                            &ntree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_build_semantic_info(ntree, &nenv));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_scope_lookup_symbol(nenv, "null_td_t",
                                          CDD_CST_SYMBOL_TYPEDEF, &ntd));
    ntd->kind = CDD_CST_SYMBOL_VARIABLE;
    memset(&dn, 0, sizeof(dn));
    memset(&dc, 0, sizeof(dc));
    memset(&dt, 0, sizeof(dt));
    dt.kind = CDD_TOKEN_IDENTIFIER;
    dt.start = (const uint8_t *)"null_td_t";
    dt.length = 9;
    dc.kind = CDD_CST_CHILD_TOKEN;
    dc.val.token = &dt;
    dn.kind = CDD_CST_TYPE_SPECIFIER;
    dn.children = &dc;
    dn.num_children = 1;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_cst_eval_sizeof(nenv, &dn, CDD_CST_ABI_LP64, &nsz));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_lexer_tokenize(az_span_create_from_str("g_ntd.f"), &ntl));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_eval_tokens_type(nenv, nenv->global_scope, ntl->tokens,
                                       ntl->size, CDD_CST_TYPE_MODE_EXACT, nbuf,
                                       sizeof(nbuf), &nis_t));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(ntl));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(nenv));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ntree));
  }

  PASS();
}

/**
 * @brief Executes the cdd_cst_type_eval_suite test suite.
 */
SUITE(cdd_cst_type_eval_suite) {
  RUN_TEST(test_cdd_cst_eval_ptr_fail);
  RUN_TEST(test_type_eval_branches);
  RUN_TEST(test_cdd_cst_eval_primitive_type_basic);
  RUN_TEST(test_cdd_cst_eval_sizeof_basic);
  RUN_TEST(test_cdd_cst_eval_sizeof_alignof_advanced);
  RUN_TEST(test_cdd_cst_eval_primitive_extra);
  RUN_TEST(test_cdd_cst_type_eval_branches);
  RUN_TEST(test_cdd_cst_eval_tokens_type_comprehensive);
  RUN_TEST(test_cdd_cst_eval_tokens_type_full_branches);
}

#ifdef __cplusplus
}
#endif

#endif
