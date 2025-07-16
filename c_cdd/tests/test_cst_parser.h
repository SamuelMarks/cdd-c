#ifndef TEST_CST_PARSER_H
#define TEST_CST_PARSER_H

#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "cst_parser.h"
#include "tokenizer.h"

/* Helper to create a fake token list for testing */
static void make_simple_token_list(struct TokenList *tl) {
  tl->size = 0;
  tl->capacity = 4;
  tl->tokens = (struct Token *)malloc(sizeof(struct Token) * tl->capacity);

  /* Add a struct + identifier + brace + close brace tokens */
  tl->tokens[0].kind = TOKEN_KEYWORD_STRUCT;
  tl->tokens[0].start = (const uint8_t *)"struct";
  tl->tokens[0].length = strlen("struct");

  tl->tokens[1].kind = TOKEN_IDENTIFIER;
  tl->tokens[1].start = (const uint8_t *)"MyStruct";
  tl->tokens[1].length = strlen("MyStruct");

  tl->tokens[2].kind = TOKEN_LBRACE;
  tl->tokens[2].start = (const uint8_t *)"{";
  tl->tokens[2].length = 1;

  tl->tokens[3].kind = TOKEN_RBRACE;
  tl->tokens[3].start = (const uint8_t *)"}";
  tl->tokens[3].length = 1;

  tl->size = 4;
}

/* Test add_node: Adding nodes increases size, capacity expands */
TEST add_node_basic(void) {
  struct CstNodeList list = {NULL, 0, 0};
  size_t i;

  ASSERT_EQ(0, add_node(&list, CST_NODE_STRUCT, (const uint8_t *)"abc", 3));
  ASSERT_EQ(1, list.size);
  ASSERT(list.nodes != NULL);
  ASSERT_EQ(CST_NODE_STRUCT, list.nodes[0].kind);
  ASSERT_STR_EQ("abc", (const char *)list.nodes[0].start);
  ASSERT_EQ(3, list.nodes[0].length);

  /* Add more to force realloc */
  for (i = 1; i < 100; i++) {
    ASSERT_EQ(0, add_node(&list, CST_NODE_COMMENT, (const uint8_t *)"x", 1));
  }
  ASSERT_EQ(100, list.size);

  free_cst_node_list(&list);

  PASS();
}

/* Test parse_tokens processes tokens as expected */
TEST parse_tokens_basic(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  struct CstNodeList copy_nodes = {NULL, 0, 0};

  make_simple_token_list(&tokens);

  /* parse_tokens should return 0 */
  ASSERT_EQ(0, parse_tokens(&tokens, &cst_nodes));
  ASSERT_GT(cst_nodes.size, 0);

  /* Check that we have at least 1 CST node being STRUCT */
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

  /* Test free_cst_node_list clears nodes */
  /* Prepare copy to test free */
  copy_nodes = cst_nodes;
  free_cst_node_list(&copy_nodes);
  /* After free, size and nodes must be zero/null */
  ASSERT_EQ(0, copy_nodes.size);
  ASSERT_EQ(0, copy_nodes.capacity);
  ASSERT(copy_nodes.nodes == NULL);

  /* Cleanup */
  free_token_list(&tokens);
  /*free_cst_node_list(&cst_nodes);*/

  PASS();
}

/* Test parse_tokens with empty list */
TEST parse_tokens_empty(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};

  ASSERT_EQ(0, parse_tokens(&tokens, &cst_nodes));
  ASSERT_EQ(0, cst_nodes.size);
  ASSERT(cst_nodes.nodes == NULL);

  free_cst_node_list(&cst_nodes);
  PASS();
}

/* Helper to create tokens for enum parsing */
static void make_enum_tokens(struct TokenList *tl) {
  tl->size = 0;
  tl->capacity = 6;
  tl->tokens = (struct Token *)malloc(sizeof(struct Token) * tl->capacity);

  tl->tokens[0].kind = TOKEN_KEYWORD_ENUM;
  tl->tokens[0].start = (const uint8_t *)"enum";
  tl->tokens[0].length = strlen("enum");

  tl->tokens[1].kind = TOKEN_IDENTIFIER;
  tl->tokens[1].start = (const uint8_t *)"Color";
  tl->tokens[1].length = strlen("Color");

  tl->tokens[2].kind = TOKEN_LBRACE;
  tl->tokens[2].start = (const uint8_t *)"{";
  tl->tokens[2].length = 1;

  tl->tokens[3].kind = TOKEN_IDENTIFIER;
  tl->tokens[3].start = (const uint8_t *)"RED";
  tl->tokens[3].length = strlen("RED");

  tl->tokens[4].kind = TOKEN_COMMA;
  tl->tokens[4].start = (const uint8_t *)",";
  tl->tokens[4].length = 1;

  tl->tokens[5].kind = TOKEN_IDENTIFIER;
  tl->tokens[5].start = (const uint8_t *)"GREEN";
  tl->tokens[5].length = strlen("GREEN");

  tl->size = 6;
}

/* Test parse_tokens for enum token list */
TEST parse_tokens_enum(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};

  make_enum_tokens(&tokens);

  ASSERT_EQ(0, parse_tokens(&tokens, &cst_nodes));
  ASSERT_GT(cst_nodes.size, 0);

  /* We want to see at least one node with kind CST_NODE_ENUM */
  {
    size_t found_enum = 0;
    size_t i;
    for (i = 0; i < cst_nodes.size; i++) {
      if (cst_nodes.nodes[i].kind == CST_NODE_ENUM) {
        found_enum = 1;
        break;
      }
    }
    ASSERT(found_enum);
  }

  free_token_list(&tokens);
  free_cst_node_list(&cst_nodes);

  PASS();
}

/* Suite definition */
SUITE(cst_parser_suite) {
  RUN_TEST(add_node_basic);
  RUN_TEST(parse_tokens_basic);
  RUN_TEST(parse_tokens_empty);
  RUN_TEST(parse_tokens_enum);
}

#endif /* !TEST_CST_PARSER_H */
