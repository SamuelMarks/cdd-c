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
  ASSERT_STRN_EQ("abc", (const char *)list.nodes[0].start, 3);
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
  copy_nodes.nodes = cst_nodes.nodes;
  copy_nodes.size = cst_nodes.size;
  copy_nodes.capacity = cst_nodes.capacity;
  free_cst_node_list(&copy_nodes);
  /* After free, size and nodes must be zero/null */
  ASSERT_EQ(0, copy_nodes.size);
  ASSERT_EQ(0, copy_nodes.capacity);
  ASSERT(copy_nodes.nodes == NULL);

  /* Cleanup */
  free_token_list(&tokens);
  /* cst_nodes.nodes was freed with copy_nodes */

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

TEST test_free_cst_node_list_edge(void) {
  struct CstNodeList list = {NULL, 0, 0};
  free_cst_node_list(&list);
  /* Call again to check idempotence */
  free_cst_node_list(&list);
  PASS();
}

TEST free_cst_node_list_null(void) {
  struct CstNodeList list = {NULL, 0, 0};
  free_cst_node_list(&list);
  PASS();
}

TEST parse_tokens_forward_declaration(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct MyStruct;");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));
  ASSERT_EQ(3, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_anonymous_struct(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct { int x; };");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  /* Expects [CST_NODE_STRUCT for "struct {...}"] and [CST_NODE_OTHER for ";"]
   */
  ASSERT_EQ(2, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_union(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  const char code_str[] = "union MyUnion { int i; float f; };";
  const az_span code = az_span_create_from_str((char *)code_str);

  ASSERT_EQ(0, tokenize(code, &tokens));
  ASSERT_EQ(0, parse_tokens(&tokens, &cst_nodes));

  ASSERT_GTE(cst_nodes.size, 1);
  ASSERT_EQ(CST_NODE_UNION, cst_nodes.nodes[0].kind);

  free_token_list(&tokens);
  free_cst_node_list(&cst_nodes);
  PASS();
}

TEST parse_tokens_nested_struct(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code =
      AZ_SPAN_FROM_STR("struct Outer { struct Inner { int y; } in; };");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  {
    size_t struct_count = 0;
    size_t i;
    for (i = 0; i < cst.size; ++i) {
      if (cst.nodes[i].kind == CST_NODE_STRUCT) {
        struct_count++;
      }
    }
    ASSERT_EQ(2, struct_count);
  }

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_other_tokens(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("int x = 5;");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  ASSERT(cst.size > 0);
  ASSERT_EQ(CST_NODE_OTHER,
            cst.nodes[0].kind); /* for "int" which is not a handled keyword */

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_unclosed_struct(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct S { int x;");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));
  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_with_empty_struct_body(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct S{};");
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  ASSERT_GTE(cst.size, 1);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_struct_variable_declaration(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct S { int x; } s;");
  size_t i, struct_nodes = 0, other_nodes = 0;

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_STRUCT) {
      struct_nodes++;
    } else if (cst.nodes[i].kind == CST_NODE_OTHER) {
      other_nodes++;
    }
  }
  ASSERT_EQ(1, struct_nodes);
  ASSERT_EQ(1, other_nodes);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_struct_variable_declaration_no_semicolon(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct S { int x; } s");
  size_t i, struct_nodes = 0, other_nodes = 0;

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_STRUCT) {
      struct_nodes++;
    } else if (cst.nodes[i].kind == CST_NODE_OTHER) {
      other_nodes++;
    }
  }
  ASSERT_EQ(1, struct_nodes);
  ASSERT_EQ(1, other_nodes);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_nested_enum_and_union(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code =
      AZ_SPAN_FROM_STR("struct S { enum E {A}; union U {int i;}; };");
  size_t i, struct_count = 0, enum_count = 0, union_count = 0;

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  for (i = 0; i < cst.size; i++) {
    switch (cst.nodes[i].kind) {
    case CST_NODE_STRUCT:
      struct_count++;
      break;
    case CST_NODE_ENUM:
      enum_count++;
      break;
    case CST_NODE_UNION:
      union_count++;
      break;
    default:
      break;
    }
  }

  ASSERT_EQ(1, struct_count);
  ASSERT_EQ(1, enum_count);
  ASSERT_EQ(1, union_count);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_anonymous_enum(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("enum { V1, V2 };");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  ASSERT_GTE(cst.size, 2);
  ASSERT_EQ(CST_NODE_ENUM, cst.nodes[0].kind);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[1].kind); /* for the semicolon */

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_unclosed_block_at_end(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("union U { int i;");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_UNION, cst.nodes[0].kind);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_comment_and_macro(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("/* hi */ #define FOO");
  int comment_nodes = 0, macro_nodes = 0;
  size_t i;

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_COMMENT)
      comment_nodes++;
    else if (cst.nodes[i].kind == CST_NODE_MACRO)
      macro_nodes++;
  }
  ASSERT_EQ(1, comment_nodes);
  ASSERT_EQ(1, macro_nodes);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_struct_followed_by_keyword(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct S{}; enum E{A};");

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  /* Expected: struct node, semicolon, enum node, semicolon */
  ASSERT_EQ(4, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[1].kind); /* for the first semicolon */
  ASSERT_EQ(CST_NODE_ENUM, cst.nodes[2].kind);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[3].kind); /* for the second semicolon */

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_struct_var_with_space(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct S { int x; } s ;");
  size_t i, struct_nodes = 0, other_nodes = 0;

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_STRUCT) {
      struct_nodes++;
    } else if (cst.nodes[i].kind == CST_NODE_OTHER) {
      other_nodes++;
    }
  }
  ASSERT_EQ(1, struct_nodes);
  ASSERT_EQ(1, other_nodes);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

TEST parse_tokens_deeply_nested(void) {
  struct TokenList tl = {0};
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR(
      "struct O { struct M { union I { int i; } in; } mid; };");
  size_t i, struct_count = 0, union_count = 0;

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(&tl, &cst));

  for (i = 0; i < cst.size; i++) {
    switch (cst.nodes[i].kind) {
    case CST_NODE_STRUCT:
      struct_count++;
      break;
    case CST_NODE_UNION:
      union_count++;
      break;
    default:
      break;
    }
  }

  ASSERT_EQ(2, struct_count);
  ASSERT_EQ(1, union_count);

  free_token_list(&tl);
  free_cst_node_list(&cst);
  PASS();
}

/* Suite definition */
SUITE(cst_parser_suite) {
  RUN_TEST(add_node_basic);
  RUN_TEST(parse_tokens_basic);
  RUN_TEST(parse_tokens_empty);
  RUN_TEST(parse_tokens_enum);
  RUN_TEST(test_free_cst_node_list_edge);
  RUN_TEST(free_cst_node_list_null);
  RUN_TEST(parse_tokens_forward_declaration);
  RUN_TEST(parse_tokens_anonymous_struct);
  RUN_TEST(parse_tokens_union);
  RUN_TEST(parse_tokens_nested_struct);
  RUN_TEST(parse_tokens_other_tokens);
  RUN_TEST(parse_tokens_unclosed_struct);
  RUN_TEST(parse_tokens_with_empty_struct_body);
  RUN_TEST(parse_tokens_struct_variable_declaration_no_semicolon);
  RUN_TEST(parse_tokens_nested_enum_and_union);
  RUN_TEST(parse_tokens_anonymous_enum);
  RUN_TEST(parse_tokens_unclosed_block_at_end);
  RUN_TEST(parse_tokens_comment_and_macro);
  RUN_TEST(parse_tokens_struct_followed_by_keyword);
  RUN_TEST(parse_tokens_struct_var_with_space);
  RUN_TEST(parse_tokens_struct_variable_declaration);
  RUN_TEST(parse_tokens_deeply_nested);
}

#endif /* !TEST_CST_PARSER_H */
