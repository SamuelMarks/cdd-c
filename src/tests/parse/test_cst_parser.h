/**
 * @file test_cst_parser.h
 * @brief Unit tests for CST parser.
 */

#ifndef TEST_CST_PARSER_H
#define TEST_CST_PARSER_H

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

/* Helper to create a fake token list for testing */
static int make_simple_token_list(struct TokenList *tl) {
  static const char code[] = "struct MyStruct { }";
  tl->size = 0;
  tl->capacity = 4;
  tl->tokens =
      (struct Token *)C_CDD_MALLOC(sizeof(struct Token) * tl->capacity);
  if (tl->tokens == NULL) {
    return -1;
  }

  tl->tokens[0].kind = TOKEN_KEYWORD_STRUCT;
  tl->tokens[0].start = (const uint8_t *)code;
  tl->tokens[0].length = 6;

  tl->tokens[1].kind = TOKEN_IDENTIFIER;
  tl->tokens[1].start = (const uint8_t *)(code + 7);
  tl->tokens[1].length = 8;

  tl->tokens[2].kind = TOKEN_LBRACE;
  tl->tokens[2].start = (const uint8_t *)(code + 16);
  tl->tokens[2].length = 1;

  tl->tokens[3].kind = TOKEN_RBRACE;
  tl->tokens[3].start = (const uint8_t *)(code + 18);
  tl->tokens[3].length = 1;

  tl->size = 4;
  return 0;
}

TEST add_node_basic(void) {
  struct CstNodeList list = {NULL, 0, 0};
  size_t i;

  ASSERT_EQ(
      0, cst_list_add(&list, CST_NODE_STRUCT, (const uint8_t *)"abc", 3, 0, 0));
  ASSERT_EQ(1, list.size);
  ASSERT(list.nodes != NULL);
  ASSERT_EQ(CST_NODE_STRUCT, list.nodes[0].kind);

  ASSERT(strncmp("abc", (const char *)list.nodes[0].start, 3) == 0);
  ASSERT_EQ(3, list.nodes[0].length);

  for (i = 1; i < 50; i++) {
    ASSERT_EQ(0, cst_list_add(&list, CST_NODE_COMMENT, (const uint8_t *)"x", 1,
                              0, 0));
  }
  ASSERT_EQ(50, list.size);

  free_cst_node_list(&list);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_list_add(NULL, CST_NODE_STRUCT, NULL, 0, 0, 0));
  g_fail_io_after = -1;
  {
    /* int i; */
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
    /* int i; */
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

TEST parse_tokens_basic(void) {
  struct TokenList *tokens =
      (struct TokenList *)C_CDD_MALLOC(sizeof(struct TokenList));
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  struct CstNodeList copy_nodes = {NULL, 0, 0};

  ASSERT_NEQ(NULL, tokens);
  memset(tokens, 0, sizeof(*tokens));

  ASSERT_EQ(0, make_simple_token_list(tokens));
  ASSERT(tokens->tokens != NULL);

  ASSERT_EQ(0, parse_tokens(tokens, &cst_nodes));
  ASSERT_GT(cst_nodes.size, 0);

  {
    size_t found_struct = 0;
    size_t i;
    for (i = 0; i < cst_nodes.size; i++) {
      if (cst_nodes.nodes[i].kind == CST_NODE_STRUCT) {
        found_struct = 1;
      }
    }
    ASSERT(found_struct);
  }

  copy_nodes.nodes = cst_nodes.nodes;
  copy_nodes.size = cst_nodes.size;
  copy_nodes.capacity = cst_nodes.capacity;
  free_cst_node_list(&copy_nodes);
  ASSERT_EQ(0, copy_nodes.size);
  ASSERT_EQ(0, copy_nodes.capacity);
  ASSERT(copy_nodes.nodes == NULL);

  free_token_list(tokens);
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

TEST parse_tokens_empty(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  ASSERT_EQ(0, parse_tokens(&tokens, &cst_nodes));
  ASSERT_EQ(0, cst_nodes.size);
  ASSERT(cst_nodes.nodes == NULL);
  free_cst_node_list(&cst_nodes);
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

TEST parse_tokens_oom_make(void) {
  struct TokenList *tokens;
#ifdef CDD_BUILD_TESTS
  tokens = (struct TokenList *)C_CDD_MALLOC(sizeof(struct TokenList));
  ASSERT_NEQ(NULL, tokens);
  memset(tokens, 0, sizeof(*tokens));

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(-1, make_simple_token_list(tokens));
  g_cdd_alloc_fail = 0;
  C_CDD_FREE(tokens);
#endif
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

TEST parse_tokens_null_args(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_tokens(NULL, &cst_nodes));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_tokens(&tokens, NULL));
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

TEST parse_tokens_forward_declaration(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code =
      az_span_create_from_str((char *)(size_t)(size_t) "struct MyStruct;");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);

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

TEST parse_tokens_anonymous_struct(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code =
      az_span_create_from_str((char *)(size_t)(size_t) "struct { int x; };");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  ASSERT_EQ(2, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);

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

TEST parse_tokens_struct_variable_declaration(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code = az_span_create_from_str(
      (char *)(size_t)(size_t) "struct S { int x; } s;");
  size_t i, struct_nodes = 0, other_nodes = 0;

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_STRUCT) {
      struct_nodes++;
    } else if (cst.nodes[i].kind == CST_NODE_OTHER) {
      other_nodes++;
    }
  }
  ASSERT_EQ(1, struct_nodes);
  ASSERT_EQ(2, other_nodes);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    /* int i; */
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
    /* int i; */
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

TEST parse_simple_array_init(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* Should parse as ONE node due to assignment brace detection */
  const az_span code = az_span_create_from_str(
      (char *)(size_t)(size_t) "int a[] = { 1, 2, 3 };");

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
    if (t_stray)
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
          cdd_c_error_t rc;
          tree = NULL;
          g_cdd_alloc_fail = (int)i;
          rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippet),
                             &tree);
          if (tree)
            cdd_cst_tree_free(tree);
          if (rc == CDD_C_SUCCESS)
            break;
        }
        g_cdd_alloc_fail = 0;

        for (i = 1; i < 50; i++) {
          cdd_c_error_t rc;
          tree = NULL;
          g_cdd_alloc_fail = (int)i;
          rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippet),
                             &tree);
          if (tree)
            cdd_cst_tree_free(tree);
          if (rc == CDD_C_SUCCESS)
            break;
        }
        g_cdd_alloc_fail = 0;

        for (i = 1; i < 50; i++) {
          cdd_c_error_t rc;
          tree = NULL;
          g_cdd_cst_alloc_token_fail = i;
          rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippet),
                             &tree);
          if (tree)
            cdd_cst_tree_free(tree);
          if (rc == CDD_C_SUCCESS)
            break;
        }
        g_cdd_cst_alloc_token_fail = 0;

        for (i = 1; i < 50; i++) {
          cdd_c_error_t parse_rc;
          tree = NULL;
          g_cdd_cst_realloc_fail = i;
          parse_rc = cdd_cst_parse(
              az_span_create_from_str((char *)(size_t)snippet), &tree);
          if (tree)
            cdd_cst_tree_free(tree);
          if (parse_rc == CDD_C_SUCCESS)
            break;
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

TEST parse_tokens_oom(void) {
#ifdef CDD_BUILD_TESTS
  {
    struct TokenList *tl = NULL;
    struct CstNodeList cst_nodes;
    /*  (moved to global) */
    int i;
    int rc;

    (void)rc;
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

TEST test_cst_branches(void) {
  struct CstNode *out_node_ptr = NULL;
  ASSERT_EQ(0, cst_find_first(NULL, 0, &out_node_ptr));
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

/**
 * @brief Comprehensive test covering all edge cases, branches, and failure
 * injection paths in cst.c.
 */
TEST test_cst_full_coverage(void) {
  struct CstNodeList list;
  struct CstNode *found_node = NULL;
  struct TokenList *tl = NULL;
  struct TokenList *tl_comma = NULL;
  struct TokenList *tl_bracket = NULL;
  struct TokenList tl_hash;
  struct Token h_tok;
  cdd_c_error_t rc;

  /* 1. Direct cst_list_add tests */
  memset(&list, 0, sizeof(list));
  rc = cst_list_add(NULL, CST_NODE_OTHER, NULL, 0, 0, 0);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_cdd_fail_cst_list_add = 2;
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);

  memset(&list, 0, sizeof(list));
  g_cdd_fail_alloc = 2;
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_fail_alloc = 0;
  free_cst_node_list(&list);

  memset(&list, 0, sizeof(list));

  /* First allocation (capacity was 0) */
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, list.size);
  ASSERT_EQ(64, list.capacity);

  /* Force realloc growth (capacity > 0) */
  list.size = list.capacity;
  rc = cst_list_add(&list, CST_NODE_FUNCTION, (const uint8_t *)"f", 1, 1, 2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(128, list.capacity);

  /* 2. cst_find_first tests */
  rc = cst_find_first(&list, CST_NODE_FUNCTION, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = cst_find_first(NULL, CST_NODE_FUNCTION, &found_node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_node == NULL);

  rc = cst_find_first(&list, CST_NODE_UNKNOWN, &found_node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_node == NULL);

  rc = cst_find_first(&list, CST_NODE_FUNCTION, &found_node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_node != NULL);
  ASSERT_EQ(CST_NODE_FUNCTION, found_node->kind);

  /* 3. free_cst_node_list tests */
  free_cst_node_list(NULL);
  free_cst_node_list(&list);
  ASSERT(list.nodes == NULL);
  ASSERT_EQ(0, list.size);
  ASSERT_EQ(0, list.capacity);
  free_cst_node_list(&list); /* Second free on empty list */

  /* 4. Function definition edge branches */
  /* EOF right after closing paren (k >= limit) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int foo()"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Unclosed brace at EOF (brace_depth > 0) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int foo() {"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Unclosed paren at EOF */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int foo(int x"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Non-brace after closing paren */
  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "int foo() = 0;"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Assignment / Number / Semicolon before paren */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int a = 1; int 42; int c;"),
      &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Failure injections in match_function_definition */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int foo() { return 0; }"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_is_type_start = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_is_type_start = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_match_function_definition = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_match_function_definition = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 5. GCC Attributes */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "__attribute__((unused)) int x;"),
                &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "__attribute__((unused"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 6. MSVC Declspec */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "__declspec(dllexport) int x;"),
      &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "__declspec(dllexport"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 7. C23 Attributes */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "[[nodiscard]] int x;"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_consume_attributes = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_attributes = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "[[nodiscard"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 8. Static Assert */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "_Static_assert(1, \"msg\");"),
      &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_skip_ws = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws = 2;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_static_assert = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_static_assert = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 9. C11 _Generic */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "_Generic(1, int: 2);"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_token_matches_string = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_token_matches_string = 0;
  free_cst_node_list(&list);

  g_cdd_fail_token_matches_string = 2;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_token_matches_string = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_balanced_parens = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_balanced_parens = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_generic_selection = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_generic_selection = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 10. Struct / Enum / Union */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "struct S { int x; }; enum E { "
                                               "A }; union U { int y; };"),
      &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Struct with preceding tokens not LPAREN */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int z; struct S { int x; };"),
      &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Failures on struct parsing */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "struct S { int x; };"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_skip_ws_back = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_balanced_braces = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_balanced_braces = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 2; /* Fails in inner recursive parse */
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Forward declaration with and without semicolon */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "struct ForwardDecl;"),
                &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "struct ForwardNoSemi"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "struct"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "enum ForwardEnum;"),
                &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "union ForwardUnion;"),
                &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 11. Comments and Macros */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "/* comment */"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "#define FOO 1\nint x;\n"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "#define FOO 1\n"),
                &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 12. Expression braces and statements */
  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "int x = { 1 };"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_is_expression_brace = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_is_expression_brace = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws_back = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_balanced_braces = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_balanced_braces = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Expression brace preceded by paren: bpk == TOKEN_KEYWORD_IF */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "if (x) { y = 1; }"),
                &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Expression brace preceded by paren: not if */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int x = ((struct S){ 1 });"),
      &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_skip_ws_back =
      2; /* Fails second skip_ws_back inside is_expression_brace */
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Statement with cast: (struct S *)p; */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int y; (struct S *)p;"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "int y; struct S *p;"),
                &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_skip_ws_back = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int y; _Generic(1, int: 1);"),
      &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_token_matches_string = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_token_matches_string = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 13. Statement with struct not preceded by paren: typedef struct S S_t; */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "typedef struct S S_t;"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);

  /* Test skip_ws_back failure inside CST_NODE_OTHER when encountering struct */
  g_cdd_fail_skip_ws_back = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 14. Fallthrough when _Generic is not followed by parens */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "_Generic;"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 15. token_matches_string failure inside CST_NODE_OTHER loop */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int x = 1;"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_token_matches_string = 3;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_token_matches_string = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 16. C23 attribute [[ inside statement */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int a [[nodiscard]] int b;"),
      &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 14. parse_tokens parameter validation */
  rc = parse_tokens(NULL, &list);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int a;"), &tl);
  ASSERT_EQ(0, rc);
  rc = parse_tokens(tl, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);
  tl = NULL;

  /* 15. Direct testing of wrapper functions for 100% parameter validation and
   * internal branches */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "  int a; void f(); ((struct S){ 1 });"),
                &tl);
  ASSERT_EQ(0, rc);
  {
    size_t val = 0;
    int is_match = 0;
    int is_type = 0;
    size_t end_idx = 0;

    /* cdd_test_cst_skip_ws */
    rc = cdd_test_cst_skip_ws(NULL, 0, 1, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_skip_ws(tl, 0, 1, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_skip_ws = 1;
    rc = cdd_test_cst_skip_ws(tl, 0, 1, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_skip_ws = 0;
    rc = cdd_test_cst_skip_ws(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* cdd_test_cst_skip_ws_back */
    rc = cdd_test_cst_skip_ws_back(NULL, 1, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_skip_ws_back(tl, 1, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_skip_ws_back(tl, 0, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, val);
    g_cdd_fail_skip_ws_back = 1;
    rc = cdd_test_cst_skip_ws_back(tl, 1, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_skip_ws_back = 0;
    rc = cdd_test_cst_skip_ws_back(tl, 2, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* cdd_test_cst_is_type_start */
    rc = cdd_test_cst_is_type_start(NULL, &is_type);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_is_type_start(&tl->tokens[0], NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_is_type_start = 2;
    rc = cdd_test_cst_is_type_start(&tl->tokens[0], &is_type);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_is_type_start(&tl->tokens[0], &is_type);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_is_type_start = 0;

    /* Test is_type_start with various token kinds */
    {
      struct Token dummy;
      dummy.kind = TOKEN_KEYWORD_VOID;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_CHAR;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_INT;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_FLOAT;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_DOUBLE;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_LONG;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_SHORT;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_SIGNED;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_UNSIGNED;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_STRUCT;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_ENUM;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_UNION;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_STATIC;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_INLINE;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_EXTERN;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_CONST;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_VOLATILE;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_AUTO;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_REGISTER;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_BOOL;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_SEMICOLON;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, is_type);
    }

    /* cdd_test_cst_match_function_definition */
    rc =
        cdd_test_cst_match_function_definition(NULL, 0, 0, &end_idx, &is_match);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_match_function_definition(tl, 0, 0, NULL, &is_match);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_match_function_definition(tl, 0, 0, &end_idx, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_match_function_definition = 2;
    rc = cdd_test_cst_match_function_definition(tl, 0, tl->size, &end_idx,
                                                &is_match);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_match_function_definition(tl, 0, tl->size, &end_idx,
                                                &is_match);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_match_function_definition = 0;

    /* Fail is_type_start inside match_function_definition */
    g_cdd_fail_is_type_start = 1;
    rc = cdd_test_cst_match_function_definition(tl, 0, tl->size, &end_idx,
                                                &is_match);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_is_type_start = 0;

    /* cdd_test_cst_consume_balanced_parens */
    rc = cdd_test_cst_consume_balanced_parens(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_balanced_parens(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_balanced_parens(tl, tl->size, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_fail_consume_balanced_parens = 2;
    rc = cdd_test_cst_consume_balanced_parens(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_balanced_parens(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_balanced_parens = 0;
    {
      struct TokenList *tl_unclosed_paren = NULL;
      rc = tokenize(az_span_create_from_str((char *)(size_t) "(a + b"),
                    &tl_unclosed_paren);
      ASSERT_EQ(0, rc);
      rc = cdd_test_cst_consume_balanced_parens(tl_unclosed_paren, 0,
                                                tl_unclosed_paren->size, &val);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, val);
      /* Test starting at non-LPAREN token (token 1 is 'a') */
      rc = cdd_test_cst_consume_balanced_parens(tl_unclosed_paren, 1,
                                                tl_unclosed_paren->size, &val);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, val);
      free_token_list(tl_unclosed_paren);
    }
    {
      struct TokenList *tl_unclosed_gen = NULL;
      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "_Generic(1, int: 2"),
          &tl_unclosed_gen);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_unclosed_gen, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_unclosed_gen);
    }

    /* cdd_test_cst_consume_attributes */
    rc = cdd_test_cst_consume_attributes(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_attributes(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_consume_attributes = 2;
    rc = cdd_test_cst_consume_attributes(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_attributes(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_attributes = 0;

    /* cdd_test_cst_consume_static_assert */
    rc = cdd_test_cst_consume_static_assert(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_static_assert(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_consume_static_assert = 2;
    rc = cdd_test_cst_consume_static_assert(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_static_assert(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_static_assert = 0;

    /* cdd_test_cst_consume_generic_selection */
    rc = cdd_test_cst_consume_generic_selection(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_generic_selection(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_generic_selection(tl, tl->size, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_fail_consume_generic_selection = 2;
    rc = cdd_test_cst_consume_generic_selection(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_generic_selection(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_generic_selection = 0;

    /* cdd_test_cst_is_expression_brace */
    rc = cdd_test_cst_is_expression_brace(NULL, 0, &is_match);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_is_expression_brace(tl, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_is_expression_brace = 2;
    rc = cdd_test_cst_is_expression_brace(tl, 0, &is_match);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_is_expression_brace(tl, 0, &is_match);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_is_expression_brace = 0;

    /* cdd_test_cst_consume_balanced_braces */
    rc = cdd_test_cst_consume_balanced_braces(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_balanced_braces(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_consume_balanced_braces = 2;
    rc = cdd_test_cst_consume_balanced_braces(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_balanced_braces(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_balanced_braces = 0;

    /* Call is_expression_brace on token sequence with paren before brace */
    {
      struct TokenList *tl_expr = NULL;
      size_t b_idx = 0;
      rc = tokenize(az_span_create_from_str(
                        (char *)(size_t) "int x = ((struct S){ 1 });"),
                    &tl_expr);
      ASSERT_EQ(0, rc);
      for (val = 0; val < tl_expr->size; val++) {
        if (tl_expr->tokens[val].kind == TOKEN_LBRACE) {
          b_idx = val;
          break;
        }
      }
      /* Test failure of second skip_ws_back inside is_expression_brace */
      g_cdd_fail_skip_ws_back = 2;
      rc = cdd_test_cst_is_expression_brace(tl_expr, b_idx, &is_match);
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
      g_cdd_fail_skip_ws_back = 0;

      rc = cdd_test_cst_is_expression_brace(tl_expr, b_idx, &is_match);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_match);
      free_token_list(tl_expr);
    }

    /* Test cdd_test_cst_parse_recursive starting at struct keyword immediately
     * preceded by LPAREN */
    {
      struct TokenList *tl_lit = NULL;
      memset(&list, 0, sizeof(list));
      rc =
          tokenize(az_span_create_from_str((char *)(size_t) "(struct S){ 1 };"),
                   &tl_lit);
      ASSERT_EQ(0, rc);
      /* Token 0 is LPAREN, Token 1 is struct */
      rc = cdd_test_cst_parse_recursive(tl_lit, 1, tl_lit->size, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_lit);
    }

    /* Test pre-allocated list (capacity != 0) */
    {
      struct CstNodeList pre_list;
      pre_list.capacity = 16;
      pre_list.size = 0;
      pre_list.nodes = (struct CstNode *)malloc(16 * sizeof(struct CstNode));
      ASSERT(pre_list.nodes != NULL);
      rc = parse_tokens(tl, &pre_list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&pre_list);
    }

    /* Test isolated grammar constructs */
    {
      struct TokenList *tl_isolated = NULL;

      /* 1. *func_star() {} to test is_type == 0 && tok->kind == TOKEN_STAR */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "*func_star() {}"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      /* 2. struct S int a; to test decl_end < end && tokens[decl_end].kind !=
       * TOKEN_SEMICOLON */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "struct S int a;"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      /* 3. int } foo() {} to test kind == TOKEN_RBRACE before ( */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "int } foo() {}"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      /* 4. Comma expression brace, empty struct, no-semi struct, cast
       * union/enum */
      rc = tokenize(az_span_create_from_str(
                        (char *)(size_t) "int arr[2][2] = { {1, 2}, {3, 4} };"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "struct Empty { };"),
          &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(az_span_create_from_str(
                        (char *)(size_t) "(union U *)p; (enum E *)q;"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(az_span_create_from_str(
                        (char *)(size_t) "_Static_assert((1), \"\");"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "_Static_assert(1)"),
          &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "struct NoSemi { int x; }"),
          &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      /* pk == TOKEN_COMMA before brace */
      rc = tokenize(az_span_create_from_str((char *)(size_t) ", {"), &tl_comma);
      ASSERT_EQ(0, rc);
      rc = cdd_test_cst_is_expression_brace(tl_comma, 1, &is_match);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_match);
      free_token_list(tl_comma);

      /* pk == TOKEN_KEYWORD_RETURN before brace */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "return {"),
                    &tl_comma);
      ASSERT_EQ(0, rc);
      rc = cdd_test_cst_is_expression_brace(tl_comma, 1, &is_match);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_match);
      free_token_list(tl_comma);

      /* Non-type top level statement: 42; (void)0; */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "42; (void)0;"),
                    &tl_comma);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_comma, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_comma);

      /* Struct forward decl in block without semi: { struct S } */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "{ struct S }"),
                    &tl_comma);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_comma, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_comma);

      /* Static assert without semicolon after parens (tokens[i].kind !=
       * TOKEN_SEMICOLON) */
      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "_Static_assert(1) int x;"),
          &tl_comma);
      ASSERT_EQ(0, rc);
      rc =
          cdd_test_cst_consume_static_assert(tl_comma, 0, tl_comma->size, &val);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, val);
      free_token_list(tl_comma);

      /* Single bracket at EOF (i + 1 >= end) */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "["), &tl_bracket);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_bracket, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_bracket);

      /* [x]; tests tokens[i + 1].kind != TOKEN_LBRACKET */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "[x];"),
                    &tl_bracket);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_bracket, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_bracket);

      /* TOKEN_HASH and TOKEN_MACRO tests */
      memset(&tl_hash, 0, sizeof(tl_hash));
      memset(&h_tok, 0, sizeof(h_tok));
      h_tok.kind = TOKEN_HASH;
      h_tok.start = (const uint8_t *)"#";
      h_tok.length = 1;
      tl_hash.tokens = &h_tok;
      tl_hash.size = 1;
      tl_hash.capacity = 1;
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(&tl_hash, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);

      h_tok.kind = TOKEN_MACRO;
      h_tok.start = (const uint8_t *)"#define X 1\n";
      h_tok.length = 12;
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(&tl_hash, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);

      /* Statement with TOKEN_COMMENT, TOKEN_MACRO, TOKEN_HASH,
       * TOKEN_KEYWORD_STATIC_ASSERT breaking CST_NODE_OTHER */
      {
        struct Token stmt_pair[2];
        memset(stmt_pair, 0, sizeof(stmt_pair));
        stmt_pair[0].kind = TOKEN_KEYWORD_INT;
        stmt_pair[0].start = (const uint8_t *)"int";
        stmt_pair[0].length = 3;

        tl_hash.tokens = stmt_pair;
        tl_hash.size = 2;
        tl_hash.capacity = 2;

        stmt_pair[1].kind = TOKEN_COMMENT;
        stmt_pair[1].start = (const uint8_t *)"/*c*/";
        stmt_pair[1].length = 5;
        memset(&list, 0, sizeof(list));
        rc = parse_tokens(&tl_hash, &list);
        ASSERT_EQ(CDD_C_SUCCESS, rc);
        free_cst_node_list(&list);

        stmt_pair[1].kind = TOKEN_MACRO;
        stmt_pair[1].start = (const uint8_t *)"#\n";
        stmt_pair[1].length = 2;
        memset(&list, 0, sizeof(list));
        rc = parse_tokens(&tl_hash, &list);
        ASSERT_EQ(CDD_C_SUCCESS, rc);
        free_cst_node_list(&list);

        stmt_pair[1].kind = TOKEN_HASH;
        stmt_pair[1].start = (const uint8_t *)"#\n";
        stmt_pair[1].length = 2;
        memset(&list, 0, sizeof(list));
        rc = parse_tokens(&tl_hash, &list);
        ASSERT_EQ(CDD_C_SUCCESS, rc);
        free_cst_node_list(&list);

        stmt_pair[1].kind = TOKEN_KEYWORD_STATIC_ASSERT;
        stmt_pair[1].start = (const uint8_t *)"_Static_assert";
        stmt_pair[1].length = 14;
        memset(&list, 0, sizeof(list));
        rc = parse_tokens(&tl_hash, &list);
        ASSERT_EQ(CDD_C_SUCCESS, rc);
        free_cst_node_list(&list);
      }
    }
  }
  free_token_list(tl);
  tl = NULL;

  PASS();
}

SUITE(cst_parser_suite) {
  RUN_TEST(test_cst_parser_extra);
  RUN_TEST(add_node_basic);
  RUN_TEST(parse_tokens_basic);
  RUN_TEST(parse_tokens_oom_make);
  RUN_TEST(parse_tokens_oom);
  RUN_TEST(parse_tokens_empty);
  RUN_TEST(parse_tokens_null_args);
  RUN_TEST(parse_tokens_forward_declaration);
  RUN_TEST(parse_tokens_anonymous_struct);
  RUN_TEST(test_parse_tokens_attributes);
  RUN_TEST(test_parse_tokens_static_assert);

  RUN_TEST(parse_tokens_struct_variable_declaration);
  RUN_TEST(test_cst_branches);

  RUN_TEST(parse_simple_array_init);
  RUN_TEST(parse_compound_literal);
  RUN_TEST(parse_control_block_split);
  RUN_TEST(parse_nested_compound_literal);
  RUN_TEST(parse_return_compound);

  RUN_TEST(parse_c11_generic); /* Added */
  RUN_TEST(test_cst_find_first);
  RUN_TEST(test_cst_full_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CST_PARSER_H */
