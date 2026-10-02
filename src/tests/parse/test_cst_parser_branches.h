/**
 * @file test_cst_parser_branches.h
 * @brief Unit tests for CST parser branch coverage.
 */

#ifndef TEST_CST_PARSER_BRANCHES_H
#define TEST_CST_PARSER_BRANCHES_H

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

TEST parse_tokens_oom(void) {
#ifdef CDD_BUILD_TESTS
  {
    struct TokenList *tl = NULL;
    struct CstNodeList cst_nodes;
    /*  (moved to global) */
    int i;
    int rc = 0;

    rc += 0;
    tokenize(
        az_span_create_from_str(
            (char *)(size_t) "int main() { char *p = malloc(10); return 0; }"),
        &tl);

    memset(&cst_nodes, 0, sizeof(cst_nodes));

    for (i = 1; i < 50; i++) {
      memset(&cst_nodes, 0, sizeof(cst_nodes));
      g_cdd_alloc_fail = (int)i;
      rc = parse_tokens(tl, &cst_nodes);
      g_cdd_alloc_fail = 0;
      if (rc == 0) {
        free_cst_node_list(&cst_nodes);
        break;
      }
      free_cst_node_list(&cst_nodes);
    }

    free_token_list(tl);
  }
#endif
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc = 0;
      /*  (moved to global) */
      rc += 0;
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
      int rc = 0;
      /*  (moved to global) */
      rc += 0;
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

TEST test_cst_branches(void) {
  struct CstNode *out_node_ptr = NULL;
  ASSERT_EQ(0, cst_find_first(NULL, 0, &out_node_ptr));
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc = 0;
      /*  (moved to global) */
      rc += 0;
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
      int rc = 0;
      /*  (moved to global) */
      rc += 0;
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

TEST test_parse_tokens_attributes(void) {
  struct TokenList *tl = NULL, *tl2 = NULL, *tl3 = NULL, *tl4 = NULL,
                   *tl5 = NULL;
  struct CstNodeList cst = {0};
  tokenize(az_span_create_from_str((char *)(size_t) "[[nodiscard]] int x;"),
           &tl);
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl);

  tokenize(az_span_create_from_str(
               (char *)(size_t) "__attribute__((unused)) int x;"),
           &tl5);
  ASSERT_EQ(0, parse_tokens(tl5, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl5);

  tokenize(az_span_create_from_str(
               (char *)(size_t) "[[unknown_attr(1, 2, 3)]] void f() {}"),
           &tl2);
  ASSERT_EQ(0, parse_tokens(tl2, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl2);

  tokenize(az_span_create_from_str((char *)(size_t) "[["), &tl3);
  ASSERT_EQ(0, parse_tokens(tl3, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl3);

  tokenize(az_span_create_from_str(
               (char *)(size_t) "[[unknown_attr[1]]] void f() {}"),
           &tl4);
  {
    tl = NULL;
    memset(&cst, 0, sizeof(cst));
    tokenize(az_span_create_from_str(
                 (char *)(size_t) "__attribute__((always_inline))"),
             &tl);
    ASSERT_EQ(0, parse_tokens(tl, &cst));
    free_cst_node_list(&cst);
    free_token_list(tl);
  }
  {
    tl = NULL;
    memset(&cst, 0, sizeof(cst));
    tokenize(az_span_create_from_str((char *)(size_t) "__declspec(dllexport)"),
             &tl);
    ASSERT_EQ(0, parse_tokens(tl, &cst));
    free_cst_node_list(&cst);
    free_token_list(tl);

    tokenize(az_span_create_from_str(
                 (char *)(size_t) "__declspec(align(16)) int x;"),
             &tl);
    ASSERT_EQ(0, parse_tokens(tl, &cst));
    free_cst_node_list(&cst);
    free_token_list(tl);
  }

  ASSERT_EQ(0, parse_tokens(tl4, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl4);

  {
    int i;
    for (i = 1; i < 50; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc = 0;
      /*  (moved to global) */
      rc += 0;
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
      int rc = 0;
      /*  (moved to global) */
      rc += 0;
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
TEST test_parse_tokens_static_assert(void) {
  struct TokenList *tl = NULL, *tl2 = NULL, *tl3 = NULL, *tl4 = NULL,
                   *tl5 = NULL;
  struct CstNodeList cst = {0};
  tokenize(az_span_create_from_str(
               (char *)(size_t) "_Static_assert(1 == 1, \"msg\");"),
           &tl);
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl);

  tokenize(az_span_create_from_str((char *)(size_t) "_Static_assert(1 == 1);"),
           &tl2);
  ASSERT_EQ(0, parse_tokens(tl2, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl2);

  tokenize(az_span_create_from_str((char *)(size_t) "_Static_assert"), &tl3);
  ASSERT_EQ(0, parse_tokens(tl3, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl3);

  tokenize(az_span_create_from_str(
               (char *)(size_t) "_Static_assert((1 == 1), \"msg\");"),
           &tl4);
  ASSERT_EQ(0, parse_tokens(tl4, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl4);

  tokenize(az_span_create_from_str((char *)(size_t) "_Static_assert(1 == 1)"),
           &tl5);
  ASSERT_EQ(0, parse_tokens(tl5, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl5);

  {
    struct TokenList *tl6 = NULL;
    tokenize(az_span_create_from_str((char *)(size_t) "_Static_assert(1 == 1;"),
             &tl6);
    ASSERT_EQ(0, parse_tokens(tl6, &cst));
    free_cst_node_list(&cst);
    free_token_list(tl6);

    {
      int i;
      for (i = 1; i < 50; i++) {
        struct TokenList *tl_oom = NULL;
        struct CstNodeList cst_oom = {0};
        int rc = 0;
        /*  (moved to global) */
        rc += 0;
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
        int rc = 0;
        /*  (moved to global) */
        rc += 0;
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

/**
 * @brief Comprehensive test covering all edge cases, branches, and failure
 * injection paths in cst.c.
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CST_PARSER_BRANCHES_H */
