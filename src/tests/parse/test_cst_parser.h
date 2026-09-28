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

#include "parse/test_cst_parser_branches.h"
#include "parse/test_cst_parser_coverage.h"
#include "parse/test_cst_parser_coverage_statements.h"
#include "parse/test_cst_parser_expressions.h"

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

  RUN_TEST(parse_c11_generic);
  RUN_TEST(test_cst_find_first);
  RUN_TEST(test_cst_full_coverage);
  RUN_TEST(test_cst_full_coverage_statements);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CST_PARSER_H */
