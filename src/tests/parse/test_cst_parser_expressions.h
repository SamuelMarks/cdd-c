/**
 * @file test_cst_parser_expressions.h
 * @brief Unit tests for CST parser expressions and generics.
 */

#ifndef TEST_CST_PARSER_EXPRESSIONS_H
#define TEST_CST_PARSER_EXPRESSIONS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "functions/parse/cst.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
extern C_CDD_EXPORT int g_cdd_fail_alloc;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws_back;
extern C_CDD_EXPORT int g_cdd_fail_is_type_start;
extern C_CDD_EXPORT int g_cdd_fail_consume_balanced_parens;
extern C_CDD_EXPORT int g_cdd_fail_consume_attributes;
extern C_CDD_EXPORT int g_cdd_fail_consume_static_assert;
extern C_CDD_EXPORT int g_cdd_fail_consume_generic_selection;
extern C_CDD_EXPORT int g_cdd_fail_is_expression_brace;
extern C_CDD_EXPORT int g_cdd_fail_consume_balanced_braces;
extern C_CDD_EXPORT int g_cdd_fail_match_function_definition;
extern C_CDD_EXPORT int g_cdd_fail_cst_list_add;
extern C_CDD_EXPORT int g_cdd_fail_token_matches_string;
#endif

TEST parse_compound_literal(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* Should parse as ONE node due to (type) { ... } detection */
  const az_span code = az_span_create_from_str(
      (char *)(size_t)(size_t) "var = (struct S){ .x = 1 };");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[0].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(az_span_create_from_str(
                   (char *)(size_t) "void f() { int x = 1; if(x) { "
                                    "_Static_assert(1); } else { "
                                    "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(
          az_span_create_from_str(
              (char *)(size_t)(size_t) "struct A { int a: 1; }; enum E { X }; "
                                       "union "
                                       "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }

  PASS();
}

TEST parse_control_block_split(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* Should scan 'if(1)' as one node, then stop at brace. */
  /* The scanner breaks on block-start braces unless matched as expr. */
  const az_span code =
      az_span_create_from_str((char *)(size_t)(size_t) "if (1) { x=1; }");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  /* Expectation:
     Node 1: OTHER "if (1) " (whitespace included)
     Node 2: OTHER "{ x=1; }" -> Scanner sees {, is_expression=false, breaks.
     Next iter: sees {, consumes balanced brace block as one OTHER.
  */
  ASSERT_NEQ(
      1,
      cst.size); /* Should NOT be lumped into one if possible without logic */

  /* Verify at least 2 nodes */
  ASSERT_GTE(cst.size, 2);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(az_span_create_from_str(
                   (char *)(size_t) "void f() { int x = 1; if(x) { "
                                    "_Static_assert(1); } else { "
                                    "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(
          az_span_create_from_str(
              (char *)(size_t)(size_t) "struct A { int a: 1; }; enum E { X }; "
                                       "union "
                                       "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }

  PASS();
}

TEST parse_nested_compound_literal(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* Function call with compound literal argument */
  const az_span code = az_span_create_from_str(
      (char *)(size_t)(size_t) "func((struct Point){0,0});");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  /* Should be 1 statement node */
  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[0].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(az_span_create_from_str(
                   (char *)(size_t) "void f() { int x = 1; if(x) { "
                                    "_Static_assert(1); } else { "
                                    "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(
          az_span_create_from_str(
              (char *)(size_t)(size_t) "struct A { int a: 1; }; enum E { X }; "
                                       "union "
                                       "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }

  PASS();
}

TEST parse_return_compound(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* Return compound literal */
  const az_span code =
      az_span_create_from_str((char *)(size_t)(size_t) "return (int[]){1,2};");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  ASSERT_EQ(1, cst.size);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(az_span_create_from_str(
                   (char *)(size_t) "void f() { int x = 1; if(x) { "
                                    "_Static_assert(1); } else { "
                                    "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(
          az_span_create_from_str(
              (char *)(size_t)(size_t) "struct A { int a: 1; }; enum E { X }; "
                                       "union "
                                       "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }

  PASS();
}

TEST parse_c11_generic(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* _Generic selection */
  const az_span code = az_span_create_from_str(
      (char *)(size_t)(size_t) "#define cbrt(X) _Generic((X), long double: "
                               "cbrtl, "
                               "default: cbrt, float: cbrtf)(X)");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  /*
     Node 0: MACRO (#define...) - cst_parser handles top level macros.
     BUT _Generic often inside a macro or function.
     Tokenizer makes #define a MACRO token only at start.
     Here it is one line macro.
     Wait, cst_parser lumps macros into CST_NODE_MACRO.
     So we need to test _Generic in a non-macro context (expression).
  */

  free_token_list(tl);
  free_cst_node_list(&cst);

  {
    const az_span code2 = az_span_create_from_str((
        char *)(size_t)(size_t) "int x = _Generic(1.0, float: 1, default: 0);");

    tl = NULL;
    memset(&cst, 0, sizeof(cst));

    ASSERT_EQ(0, tokenize(code2, &tl));
    ASSERT_EQ(0, parse_tokens(tl, &cst));

    /*
      Nodes expected:
      1. OTHER "int x = "
      2. GENERIC_SELECTION "_Generic(1.0, ...)"
      3. OTHER ";"
    */
    ASSERT_EQ(3, cst.size);
    ASSERT_EQ(CST_NODE_OTHER, cst.nodes[0].kind);
    ASSERT_EQ(CST_NODE_GENERIC_SELECTION, cst.nodes[1].kind);
    ASSERT_EQ(CST_NODE_OTHER, cst.nodes[2].kind);
  }

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(az_span_create_from_str(
                   (char *)(size_t) "void f() { int x = 1; if(x) { "
                                    "_Static_assert(1); } else { "
                                    "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(
          az_span_create_from_str(
              (char *)(size_t)(size_t) "struct A { int a: 1; }; enum E { X }; "
                                       "union "
                                       "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }

  PASS();
}

/* Standard suite runner */

TEST test_cst_find_first(void) {
  struct CstNodeList list = {0};
  struct CstNode *found = NULL;

  cst_list_add(&list, CST_NODE_STRUCT, (const uint8_t *)"a", 1, 0, 0);
  cst_list_add(&list, CST_NODE_ENUM, (const uint8_t *)"b", 1, 0, 0);

  ASSERT_EQ(0, cst_find_first(&list, CST_NODE_ENUM, &found));
  ASSERT(found != NULL);
  ASSERT_EQ(CST_NODE_ENUM, found->kind);

  found = NULL;
  ASSERT_EQ(0, cst_find_first(&list, CST_NODE_MACRO, &found));
  ASSERT(found == NULL);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_find_first(&list, CST_NODE_STRUCT, NULL));

  free_cst_node_list(&list);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(az_span_create_from_str(
                   (char *)(size_t) "void f() { int x = 1; if(x) { "
                                    "_Static_assert(1); } else { "
                                    "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      /*  (moved to global) */
      (void)rc;
      tokenize(
          az_span_create_from_str(
              (char *)(size_t)(size_t) "struct A { int a: 1; }; enum E { X }; "
                                       "union "
                                       "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl_oom, &cst_oom);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
        break;
      }
      free_token_list(tl_oom);
      free_cst_node_list(&cst_oom);
    }
  }

  PASS();
}

TEST test_cst_parser_extra(void) {
  cdd_cst_tree_t *tree = NULL;

  /* Empty tree free */
  cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_tree_free(t2);

  /* Missing EOF / No tokens */
  cdd_cst_parse(az_span_create_from_str((char *)(size_t) ""), &tree);
  cdd_cst_tree_free(tree);

  /* NULL out_tree */
  {
    cdd_c_error_t out_rc =
        cdd_cst_parse(az_span_create_from_str((char *)(size_t) ""), NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, out_rc);
  }

  {
    cdd_cst_tree_t *t_stray = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)(size_t) "}"), &t_stray);
    cdd_cst_tree_free(t_stray);
  }
  {
    cdd_cst_tree_t *t_stray = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)(size_t) "}}"), &t_stray);
    cdd_cst_tree_free(t_stray);
  }
  {
    cdd_cst_tree_t *t_stray = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)(size_t) "{"), &t_stray);
    cdd_cst_tree_free(t_stray);
  }
  {
    cdd_cst_tree_t *t_empty = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)(size_t) "{}"), &t_empty);
    if (t_empty)
      cdd_cst_tree_free(t_empty);
  }
  {
    cdd_cst_tree_t *t_empty = NULL;
    cdd_cst_parse(az_span_create_from_str(
                      (char *)(size_t)(size_t) "void f() noexcept(true);"),
                  &t_empty);
    if (t_empty)
      cdd_cst_tree_free(t_empty);
  }

  {
    cdd_cst_tree_t *t_macro = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)(size_t) "#define D 1\n"),
                  &t_macro);
    if (t_macro)
      cdd_cst_tree_free(t_macro);
    {
      const char *abrupts[] = {
          "#ifdef A",  "template",    "template <",   "template <class",
          "namespace", "namespace N", "namespace N;", "namespace N {",
          "try {",     "throw ",      "class Foo {",  "class Foo { public:",
          "void f() {"};
      size_t j;
      for (j = 0; j < sizeof(abrupts) / sizeof(abrupts[0]); j++) {
        cdd_cst_tree_t *t_abrupt = NULL;
        cdd_cst_parse(az_span_create_from_str((char *)(size_t)abrupts[j]),
                      &t_abrupt);
        if (t_abrupt)
          cdd_cst_tree_free(t_abrupt);
      }
    }
#ifdef CDD_BUILD_TESTS
    {
      /*  (moved to global) */
      /*  (moved to global) */
      /* extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail; (moved to global)
       */
      /* extern C_CDD_EXPORT int g_cdd_cst_realloc_fail; (moved to global) */
      int i;
      const char snippet[] = {
          35,  105, 102, 100, 101, 102, 32,  65,  10,  35,  101, 108, 105, 102,
          32,  66,  10,  35,  101, 108, 115, 101, 10,  123, 32,  105, 110, 116,
          32,  122, 49,  59,  32,  125, 10,  123, 32,  105, 110, 116, 32,  122,
          50,  59,  32,  125, 10,  123, 32,  105, 110, 116, 32,  122, 51,  59,
          32,  125, 10,  123, 32,  105, 110, 116, 32,  122, 52,  59,  32,  125,
          10,  123, 32,  105, 110, 116, 32,  122, 53,  59,  32,  125, 10,  123,
          32,  105, 110, 116, 32,  122, 54,  59,  32,  125, 10,  123, 32,  105,
          110, 116, 32,  122, 55,  59,  32,  125, 10,  123, 32,  105, 110, 116,
          32,  122, 56,  59,  32,  125, 10,  123, 32,  105, 110, 116, 32,  122,
          57,  59,  32,  125, 10,  123, 32,  105, 110, 116, 32,  122, 49,  48,
          59,  32,  125, 10,  35,  101, 110, 100, 105, 102, 10,  35,  105, 102,
          110, 100, 101, 102, 32,  67,  10,  123, 32,  105, 110, 116, 32,  119,
          59,  32,  125, 10,  35,  101, 110, 100, 105, 102, 10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  49,  32,  49,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  50,  32,  50,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  51,  32,  51,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  52,  32,  52,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  53,  32,  53,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  54,  32,  54,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  55,  32,  55,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  56,  32,  56,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  57,  32,  57,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  49,  48,  32,  49,  48,  59,  10,  35,
          100, 101, 102, 105, 110, 101, 32,  68,  49,  49,  32,  49,  49,  59,
          10,  35,  100, 101, 102, 105, 110, 101, 32,  68,  49,  50,  32,  49,
          50,  59,  10,  35,  100, 101, 102, 105, 110, 101, 32,  68,  49,  51,
          32,  49,  51,  59,  10,  35,  100, 101, 102, 105, 110, 101, 32,  68,
          49,  52,  32,  49,  52,  59,  10,  35,  100, 101, 102, 105, 110, 101,
          32,  68,  49,  53,  32,  49,  53,  59,  10,  35,  100, 101, 102, 105,
          110, 101, 32,  68,  49,  54,  32,  49,  54,  59,  10,  35,  100, 101,
          102, 105, 110, 101, 32,  68,  49,  55,  32,  49,  55,  59,  10,  35,
          100, 101, 102, 105, 110, 101, 32,  68,  49,  56,  32,  49,  56,  59,
          10,  35,  100, 101, 102, 105, 110, 101, 32,  68,  49,  57,  32,  49,
          57,  59,  10,  35,  105, 110, 99,  108, 117, 100, 101, 32,  60,  115,
          116, 100, 105, 111, 46,  104, 62,  10,  35,  112, 114, 97,  103, 109,
          97,  32,  111, 110, 99,  101, 10,  35,  117, 110, 107, 110, 111, 119,
          110, 95,  100, 105, 114, 101, 99,  116, 105, 118, 101, 10,  91,  91,
          110, 111, 100, 105, 115, 99,  97,  114, 100, 93,  93,  32,  115, 116,
          114, 117, 99,  116, 32,  65,  116, 116, 114, 105, 98,  117, 116, 101,
          100, 83,  116, 114, 117, 99,  116, 32,  123, 32,  105, 110, 116, 32,
          120, 59,  32,  125, 59,  10,  91,  91,  110, 111, 100, 105, 115, 99,
          97,  114, 100, 93,  93,  32,  99,  108, 97,  115, 115, 32,  65,  116,
          116, 114, 105, 98,  117, 116, 101, 100, 67,  108, 97,  115, 115, 32,
          123, 32,  105, 110, 116, 32,  120, 59,  32,  125, 59,  10,  116, 101,
          109, 112, 108, 97,  116, 101, 32,  99,  108, 97,  115, 115, 32,  70,
          111, 111, 59,  10,  116, 101, 109, 112, 108, 97,  116, 101, 32,  60,
          116, 121, 112, 101, 110, 97,  109, 101, 32,  84,  49,  44,  32,  116,
          121, 112, 101, 110, 97,  109, 101, 32,  84,  50,  44,  32,  116, 121,
          112, 101, 110, 97,  109, 101, 32,  84,  51,  44,  32,  116, 121, 112,
          101, 110, 97,  109, 101, 32,  84,  52,  44,  32,  116, 121, 112, 101,
          110, 97,  109, 101, 32,  84,  53,  44,  32,  116, 121, 112, 101, 110,
          97,  109, 101, 32,  84,  54,  44,  32,  116, 121, 112, 101, 110, 97,
          109, 101, 32,  84,  55,  44,  32,  116, 121, 112, 101, 110, 97,  109,
          101, 32,  84,  56,  44,  32,  116, 121, 112, 101, 110, 97,  109, 101,
          32,  84,  57,  44,  32,  116, 121, 112, 101, 110, 97,  109, 101, 32,
          84,  49,  48,  62,  10,  99,  108, 97,  115, 115, 32,  70,  111, 111,
          32,  58,  32,  112, 117, 98,  108, 105, 99,  32,  118, 105, 114, 116,
          117, 97,  108, 32,  66,  97,  114, 44,  32,  118, 105, 114, 116, 117,
          97,  108, 32,  112, 114, 105, 118, 97,  116, 101, 32,  66,  97,  122,
          32,  123, 10,  112, 117, 98,  108, 105, 99,  58,  10,  32,  32,  118,
          111, 105, 100, 32,  98,  97,  122, 40,  41,  32,  110, 111, 101, 120,
          99,  101, 112, 116, 40,  116, 114, 117, 101, 41,  32,  123, 125, 10,
          32,  32,  126, 70,  111, 111, 40,  41,  59,  10,  32,  32,  105, 110,
          116, 32,  111, 112, 101, 114, 97,  116, 111, 114, 43,  40,  105, 110,
          116, 41,  59,  10,  112, 114, 111, 116, 101, 99,  116, 101, 100, 58,
          10,  32,  32,  99,  108, 97,  115, 115, 32,  123, 32,  105, 110, 116,
          32,  97,  110, 111, 110, 59,  32,  125, 32,  97,  110, 111, 110, 95,
          118, 97,  114, 59,  10,  32,  32,  105, 110, 116, 32,  120, 59,  10,
          112, 114, 105, 118, 97,  116, 101, 58,  10,  32,  32,  105, 110, 116,
          32,  121, 59,  10,  125, 59,  10,  110, 97,  109, 101, 115, 112, 97,
          99,  101, 32,  78,  32,  123, 10,  32,  32,  117, 115, 105, 110, 103,
          32,  110, 97,  109, 101, 115, 112, 97,  99,  101, 32,  115, 116, 100,
          59,  10,  32,  32,  118, 111, 105, 100, 32,  102, 40,  41,  32,  123,
          10,  32,  32,  32,  32,  116, 114, 121, 32,  123, 10,  32,  32,  32,
          32,  32,  32,  116, 104, 114, 111, 119, 32,  49,  59,  10,  32,  32,
          32,  32,  125, 32,  99,  97,  116, 99,  104, 32,  40,  105, 110, 116,
          32,  101, 41,  32,  123, 10,  32,  32,  32,  32,  125, 32,  99,  97,
          116, 99,  104, 32,  40,  46,  46,  46,  41,  32,  123, 10,  32,  32,
          32,  32,  125, 10,  32,  32,  125, 10,  125, 10,  105, 110, 116, 32,
          109, 97,  105, 110, 40,  41,  32,  123, 32,  97,  115, 109, 40,  34,
          110, 111, 112, 34,  41,  59,  32,  114, 101, 116, 117, 114, 110, 32,
          48,  59,  32,  125, 0};

      cdd_cst_tree_t *t_dummy = NULL;
      g_cdd_cst_realloc_fail = 1000000;
      cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippet), &t_dummy);
      printf("Total REALLOCs in snippet: %d\n",
             1000000 - g_cdd_cst_realloc_fail);
      g_cdd_cst_realloc_fail = 0;
      if (t_dummy)
        cdd_cst_tree_free(t_dummy);

      g_cdd_alloc_fail = 1000000;
      t_dummy = NULL;
      {
        int debug_rc = cdd_cst_parse(
            az_span_create_from_str((char *)(size_t)snippet), &t_dummy);
        printf("Total ALLOCs in snippet: %d, RC=%d\n",
               1000000 - g_cdd_alloc_fail, debug_rc);
      }
      g_cdd_alloc_fail = 0;
      if (t_dummy)
        cdd_cst_tree_free(t_dummy);

      {
        /* extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow; (moved to global)
         */
        g_cdd_cst_parser_fast_grow = 1;

        for (i = 1; i < 50; i++) {
          tree = NULL;
          g_cdd_alloc_fail = (int)i;
          cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippet),
                        &tree);
          cdd_cst_tree_free(tree);
        }
        g_cdd_alloc_fail = 0;

        for (i = 1; i < 50; i++) {
          tree = NULL;
          g_cdd_cst_alloc_token_fail = i;
          cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippet),
                        &tree);
          cdd_cst_tree_free(tree);
        }
        g_cdd_cst_alloc_token_fail = 0;

        for (i = 1; i < 50; i++) {
          tree = NULL;
          g_cdd_cst_realloc_fail = i;
          cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippet),
                        &tree);
          cdd_cst_tree_free(tree);
        }
        g_cdd_cst_realloc_fail = 0;

        g_cdd_cst_parser_fast_grow = 0;
      }
    }
#endif
    g_fail_io_after = -1;

    {
      int i;
      for (i = 1; i < 50; i++) {
        struct TokenList *tl_oom = NULL;
        struct CstNodeList cst_oom = {0};
        int rc;
        /*  (moved to global) */
        (void)rc;
        tokenize(az_span_create_from_str(
                     (char *)(size_t)(size_t) "void f() { int x = 1; if(x) { "
                                              "_Static_assert(1); } else { "
                                              "[[nodiscard]] int y; } }"),
                 &tl_oom);
        g_cdd_alloc_fail = (int)i;
        rc = parse_tokens(tl_oom, &cst_oom);
        g_cdd_alloc_fail = 0;
        if (rc == CDD_C_SUCCESS) {
          free_token_list(tl_oom);
          free_cst_node_list(&cst_oom);
          break;
        }
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
      }
    }
    {
      int i;
      for (i = 1; i < 50; i++) {
        struct TokenList *tl_oom = NULL;
        struct CstNodeList cst_oom = {0};
        int rc;
        /*  (moved to global) */
        (void)rc;
        tokenize(
            az_span_create_from_str((
                char *)(size_t)(size_t) "struct A { int a: 1; }; enum E { X }; "
                                        "union "
                                        "U { int b; }; _Generic((1), int: 1);"),
            &tl_oom);
        g_cdd_alloc_fail = (int)i;
        rc = parse_tokens(tl_oom, &cst_oom);
        g_cdd_alloc_fail = 0;
        if (rc == CDD_C_SUCCESS) {
          free_token_list(tl_oom);
          free_cst_node_list(&cst_oom);
          break;
        }
        free_token_list(tl_oom);
        free_cst_node_list(&cst_oom);
      }
    }

    PASS();
  }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CST_PARSER_EXPRESSIONS_H */
