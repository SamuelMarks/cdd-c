#ifndef TEST_CST_PARSER_H
#define TEST_CST_PARSER_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "cst_parser.h"
#include "tokenizer.h"

/* Helper to create a fake token list for testing */
static void make_simple_token_list(struct TokenList *tl) {
  static const char code[] = "struct MyStruct { }";
  tl->size = 0;
  tl->capacity = 4;
  tl->tokens = (struct Token *)malloc(sizeof(struct Token) * tl->capacity);
  /* Use assert or similar if malloc failed, but in test normally ok. */
  if (!tl->tokens)
    return;

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
  ASSERT_EQ(EINVAL, cst_list_add(NULL, CST_NODE_STRUCT, NULL, 0, 0, 0));
  PASS();
}

TEST parse_tokens_basic(void) {
  struct TokenList *tokens =
      (struct TokenList *)malloc(sizeof(struct TokenList));
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  struct CstNodeList copy_nodes = {NULL, 0, 0};

  if (!tokens)
    FAILm("Memory allocation failed");
  memset(tokens, 0, sizeof(*tokens));

  make_simple_token_list(tokens);
  if (!tokens->tokens) {
    free(tokens);
    FAILm("Setup failed");
  }

  ASSERT_EQ(0, parse_tokens(tokens, &cst_nodes));
  ASSERT_GT(cst_nodes.size, 0);

  {
    size_t found_struct = 0;
    size_t i;
    for (i = 0; i < cst_nodes.size; i++) {
      if (cst_nodes.nodes[i].kind == CST_NODE_STRUCT) {
        found_struct = 1;
        break;
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
  PASS();
}

TEST parse_tokens_empty(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  ASSERT_EQ(0, parse_tokens(&tokens, &cst_nodes));
  ASSERT_EQ(0, cst_nodes.size);
  ASSERT(cst_nodes.nodes == NULL);
  free_cst_node_list(&cst_nodes);
  PASS();
}

TEST parse_tokens_null_args(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  ASSERT_EQ(EINVAL, parse_tokens(NULL, &cst_nodes));
  ASSERT_EQ(EINVAL, parse_tokens(&tokens, NULL));
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
  PASS();
}

TEST parse_tokens_anonymous_struct(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct { int x; };");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  /*
   * Expect 2 nodes due to recursive parsing:
   * 1. CST_NODE_STRUCT (parent)
   * 2. CST_NODE_OTHER (child: "int x;")
   */
  ASSERT_EQ(2, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
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
  /*
   * Expect:
   * 1. STRUCT node (parent)
   * 2. OTHER node (child: "int x;")
   * 3. OTHER node (variable decl: "s;")
   */
  ASSERT_EQ(1, struct_nodes);
  ASSERT_EQ(2, other_nodes);

  free_token_list(tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_struct_followed_by_keyword(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct S{}; enum E{A};");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  /*
   * Expect 3 nodes:
   * 1. STRUCT (empty body, no children)
   * 2. ENUM (parent)
   * 3. OTHER (child "A" inside enum)
   */
  ASSERT_EQ(3, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);
  ASSERT_EQ(CST_NODE_ENUM, cst.nodes[1].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_function_def_simple(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("int main(void) { return 0; }");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_FUNCTION, cst.nodes[0].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_function_in_mix(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code =
      AZ_SPAN_FROM_STR("struct S { int x; }; int f() {} enum E {A};");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));

  /*
   * Expect 5 nodes:
   * 1. STRUCT (parent)
   * 2. OTHER (child "int x;")
   * 3. FUNCTION (leaf, no children added for body)
   * 4. ENUM (parent)
   * 5. OTHER (child "A")
   */
  ASSERT_EQ(5, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);
  /* Node 1 is the inner "int x" (OTHER) */
  /* Node 2 is the Function */
  ASSERT_EQ(CST_NODE_FUNCTION, cst.nodes[2].kind);
  ASSERT_EQ(CST_NODE_ENUM, cst.nodes[3].kind);

  free_token_list(tl);
  free_cst_node_list(&cst);
  PASS();
}

SUITE(cst_parser_suite) {
  RUN_TEST(add_node_basic);
  RUN_TEST(parse_tokens_basic);
  RUN_TEST(parse_tokens_empty);
  RUN_TEST(parse_tokens_null_args);
  RUN_TEST(parse_tokens_forward_declaration);
  RUN_TEST(parse_tokens_anonymous_struct);
  RUN_TEST(parse_tokens_struct_variable_declaration);
  RUN_TEST(parse_tokens_struct_followed_by_keyword);
  RUN_TEST(parse_tokens_function_def_simple);
  RUN_TEST(parse_tokens_function_in_mix);
}

#endif /* !TEST_CST_PARSER_H */
