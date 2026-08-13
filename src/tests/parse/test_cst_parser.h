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

  for (i = 1; i < 100; i++) {
    ASSERT_EQ(0, cst_list_add(&list, CST_NODE_COMMENT, (const uint8_t *)"x", 1,
                              0, 0));
  }
  ASSERT_EQ(100, list.size);

  free_cst_node_list(&list);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_list_add(NULL, CST_NODE_STRUCT, NULL, 0, 0, 0));
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
#include <c_cdd_export.h>
  extern C_CDD_EXPORT int g_cdd_alloc_fail;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  const az_span code = AZ_SPAN_FROM_STR("struct MyStruct;");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  const az_span code = AZ_SPAN_FROM_STR("struct { int x; };");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  ASSERT_EQ(2, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  const az_span code = AZ_SPAN_FROM_STR("struct S { int x; } s;");
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
    int i;
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  const az_span code = AZ_SPAN_FROM_STR("int a[] = { 1, 2, 3 };");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[0].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  const az_span code = AZ_SPAN_FROM_STR("var = (struct S){ .x = 1 };");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[0].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  const az_span code = AZ_SPAN_FROM_STR("if (1) { x=1; }");

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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  const az_span code = AZ_SPAN_FROM_STR("func((struct Point){0,0});");

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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  const az_span code = AZ_SPAN_FROM_STR("return (int[]){1,2};");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  ASSERT_EQ(1, cst.size);

  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  {
    int i;
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  const az_span code =
      AZ_SPAN_FROM_STR("#define cbrt(X) _Generic((X), long double: cbrtl, "
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
    const az_span code2 =
        AZ_SPAN_FROM_STR("int x = _Generic(1.0, float: 1, default: 0);");

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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  cdd_cst_parse(az_span_create_from_str(""), &tree);
  cdd_cst_tree_free(tree);

  /* NULL out_tree */
  {
    cdd_c_error_t out_rc = cdd_cst_parse(az_span_create_from_str(""), NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, out_rc);
  }

  {
    cdd_cst_tree_t *t_stray = NULL;
    cdd_cst_parse(az_span_create_from_str("}"), &t_stray);
    cdd_cst_tree_free(t_stray);
  }
  {
    cdd_cst_tree_t *t_stray = NULL;
    cdd_cst_parse(az_span_create_from_str("}}"), &t_stray);
    cdd_cst_tree_free(t_stray);
  }
  {
    cdd_cst_tree_t *t_stray = NULL;
    cdd_cst_parse(az_span_create_from_str("{"), &t_stray);
    if (t_stray)
      cdd_cst_tree_free(t_stray);
  }
  {
    cdd_cst_tree_t *t_empty = NULL;
    cdd_cst_parse(az_span_create_from_str("{}"), &t_empty);
    if (t_empty)
      cdd_cst_tree_free(t_empty);
  }
  {
    cdd_cst_tree_t *t_empty = NULL;
    cdd_cst_parse(az_span_create_from_str("void f() noexcept(true);"),
                  &t_empty);
    if (t_empty)
      cdd_cst_tree_free(t_empty);
  }

  cdd_cst_tree_t *t_macro = NULL;
  cdd_cst_parse(az_span_create_from_str("#define D 1\n"), &t_macro);
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
      cdd_cst_parse(az_span_create_from_str((char *)abrupts[j]), &t_abrupt);
      if (t_abrupt)
        cdd_cst_tree_free(t_abrupt);
    }
  }
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    extern C_CDD_EXPORT int g_cdd_alloc_fail;
    extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
    extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
    int i;
    const char *snippet =
        "#ifdef A\n"
        "#elif B\n"
        "#else\n"
        "{ int z1; }\n"
        "{ int z2; }\n"
        "{ int z3; }\n"
        "{ int z4; }\n"
        "{ int z5; }\n"
        "{ int z6; }\n"
        "{ int z7; }\n"
        "{ int z8; }\n"
        "{ int z9; }\n"
        "{ int z10; }\n"
        "#endif\n"
        "#ifndef C\n"
        "{ int w; }\n"
        "#endif\n"
        "#define D1 1;\n"
        "#define D2 2;\n"
        "#define D3 3;\n"
        "#define D4 4;\n"
        "#define D5 5;\n"
        "#define D6 6;\n"
        "#define D7 7;\n"
        "#define D8 8;\n"
        "#define D9 9;\n"
        "#define D10 10;\n"
        "#define D11 11;\n"
        "#define D12 12;\n"
        "#define D13 13;\n"
        "#define D14 14;\n"
        "#define D15 15;\n"
        "#define D16 16;\n"
        "#define D17 17;\n"
        "#define D18 18;\n"
        "#define D19 19;\n"
        "#include <stdio.h>\n"
        "#pragma once\n"
        "#unknown_directive\n"
        "[[nodiscard]] struct AttributedStruct { int x; };\n"
        "[[nodiscard]] class AttributedClass { int x; };\n"
        "template class Foo;\n"
        "template <typename T1, typename T2, typename T3, typename T4, "
        "typename T5, typename T6, typename T7, typename T8, typename T9, "
        "typename T10>\n"
        "class Foo : public virtual Bar, virtual private Baz {\n"
        "public:\n"
        "  void baz() noexcept(true) {}\n"
        "  ~Foo();\n"
        "  int operator+(int);\n"
        "protected:\n"
        "  class { int anon; } anon_var;\n"
        "  int x;\n"
        "private:\n"
        "  int y;\n"
        "};\n"
        "namespace N {\n"
        "  using namespace std;\n"
        "  void f() {\n"
        "    try {\n"
        "      throw 1;\n"
        "    } catch (int e) {\n"
        "    } catch (...) {\n"
        "    }\n"
        "  }\n"
        "}\n"
        "int main() { asm(\"nop\"); return 0; }";

    cdd_cst_tree_t *t_dummy = NULL;
    g_cdd_cst_realloc_fail = 1000000;
    cdd_cst_parse(az_span_create_from_str((char *)snippet), &t_dummy);
    printf("Total REALLOCs in snippet: %d\n", 1000000 - g_cdd_cst_realloc_fail);
    g_cdd_cst_realloc_fail = 0;
    if (t_dummy)
      cdd_cst_tree_free(t_dummy);

    g_cdd_alloc_fail = 1000000;
    t_dummy = NULL;
    {
      int debug_rc =
          cdd_cst_parse(az_span_create_from_str((char *)snippet), &t_dummy);
      printf("Total ALLOCs in snippet: %d, RC=%d\n", 1000000 - g_cdd_alloc_fail,
             debug_rc);
    }
    g_cdd_alloc_fail = 0;
    if (t_dummy)
      cdd_cst_tree_free(t_dummy);

    {
      extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;
      g_cdd_cst_parser_fast_grow = 1;

      for (i = 1; i < 60000; i++) {
        cdd_c_error_t rc;
        tree = NULL;
        g_cdd_cst_alloc_node_fail = i;
        rc = cdd_cst_parse(az_span_create_from_str((char *)snippet), &tree);
        if (tree)
          cdd_cst_tree_free(tree);
        if (rc == CDD_C_SUCCESS)
          break;
      }
      g_cdd_cst_alloc_node_fail = 0;

      for (i = 1; i < 60000; i++) {
        cdd_c_error_t rc;
        tree = NULL;
        g_cdd_alloc_fail = i;
        rc = cdd_cst_parse(az_span_create_from_str((char *)snippet), &tree);
        if (tree)
          cdd_cst_tree_free(tree);
        if (rc == CDD_C_SUCCESS)
          break;
      }
      g_cdd_alloc_fail = 0;

      for (i = 1; i < 60000; i++) {
        cdd_c_error_t rc;
        tree = NULL;
        g_cdd_cst_alloc_token_fail = i;
        rc = cdd_cst_parse(az_span_create_from_str((char *)snippet), &tree);
        if (tree)
          cdd_cst_tree_free(tree);
        if (rc == CDD_C_SUCCESS)
          break;
      }
      g_cdd_cst_alloc_token_fail = 0;

      for (i = 1; i < 60000; i++) {
        cdd_c_error_t parse_rc;
        tree = NULL;
        g_cdd_cst_realloc_fail = i;
        parse_rc =
            cdd_cst_parse(az_span_create_from_str((char *)snippet), &tree);
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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

TEST parse_tokens_oom(void) {
#ifdef CDD_BUILD_TESTS
  {
    struct TokenList *tl = NULL;
    struct CstNodeList cst_nodes;
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    int i;
    int rc;

    tokenize(az_span_create_from_str(
                 "int main() { char *p = malloc(10); return 0; }"),
             &tl);

    memset(&cst_nodes, 0, sizeof(cst_nodes));

    for (i = 1; i < 200; i++) {
      memset(&cst_nodes, 0, sizeof(cst_nodes));
      g_cdd_fail_alloc = i;
      rc = parse_tokens(tl, &cst_nodes);
      g_cdd_fail_alloc = 0;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  tokenize(az_span_create_from_str("[[nodiscard]] int x;"), &tl);
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl);

  tokenize(az_span_create_from_str("__attribute__((unused)) int x;"), &tl5);
  ASSERT_EQ(0, parse_tokens(tl5, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl5);

  tokenize(az_span_create_from_str("[[unknown_attr(1, 2, 3)]] void f() {}"),
           &tl2);
  ASSERT_EQ(0, parse_tokens(tl2, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl2);

  tokenize(az_span_create_from_str("[["), &tl3);
  ASSERT_EQ(0, parse_tokens(tl3, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl3);

  tokenize(az_span_create_from_str("[[unknown_attr[1]]] void f() {}"), &tl4);
  {
    struct TokenList *tl = NULL;
    struct CstNodeList cst = {0};
    tokenize(az_span_create_from_str("__attribute__((always_inline))"), &tl);
    ASSERT_EQ(0, parse_tokens(tl, &cst));
    free_cst_node_list(&cst);
    free_token_list(tl);
  }
  {
    struct TokenList *tl = NULL;
    struct CstNodeList cst = {0};
    tokenize(az_span_create_from_str("__declspec(dllexport)"), &tl);
    ASSERT_EQ(0, parse_tokens(tl, &cst));
    free_cst_node_list(&cst);
    free_token_list(tl);

    tokenize(az_span_create_from_str("__declspec(align(16)) int x;"), &tl);
    ASSERT_EQ(0, parse_tokens(tl, &cst));
    free_cst_node_list(&cst);
    free_token_list(tl);
  }

  ASSERT_EQ(0, parse_tokens(tl4, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl4);

  {
    int i;
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
  tokenize(az_span_create_from_str("_Static_assert(1 == 1, \"msg\");"), &tl);
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl);

  tokenize(az_span_create_from_str("_Static_assert(1 == 1);"), &tl2);
  ASSERT_EQ(0, parse_tokens(tl2, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl2);

  tokenize(az_span_create_from_str("_Static_assert"), &tl3);
  ASSERT_EQ(0, parse_tokens(tl3, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl3);

  tokenize(az_span_create_from_str("_Static_assert((1 == 1), \"msg\");"), &tl4);
  ASSERT_EQ(0, parse_tokens(tl4, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl4);

  tokenize(az_span_create_from_str("_Static_assert(1 == 1)"), &tl5);
  ASSERT_EQ(0, parse_tokens(tl5, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl5);

  struct TokenList *tl6 = NULL;
  tokenize(az_span_create_from_str("_Static_assert(1 == 1;"), &tl6);
  ASSERT_EQ(0, parse_tokens(tl6, &cst));
  free_cst_node_list(&cst);
  free_token_list(tl6);

  {
    int i;
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(az_span_create_from_str(
                   "void f() { int x = 1; if(x) { _Static_assert(1); } else { "
                   "[[nodiscard]] int y; } }"),
               &tl_oom);
      g_cdd_alloc_fail = i;
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
    for (i = 1; i < 500; i++) {
      struct TokenList *tl_oom = NULL;
      struct CstNodeList cst_oom = {0};
      int rc;
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      tokenize(
          az_span_create_from_str("struct A { int a: 1; }; enum E { X }; union "
                                  "U { int b; }; _Generic((1), int: 1);"),
          &tl_oom);
      g_cdd_alloc_fail = i;
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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CST_PARSER_H */
