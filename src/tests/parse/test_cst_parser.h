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
/* LCOV_EXCL_START */
static void make_simple_token_list(struct TokenList *tl) {
  /* LCOV_EXCL_STOP */
  static const char code[] = "struct MyStruct { }";
  /* LCOV_EXCL_START */
  tl->size = 0;
  tl->capacity = 4;
  tl->tokens = (struct Token *)malloc(sizeof(struct Token) * tl->capacity);
  if (!tl->tokens)
    return;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  tl->tokens[0].kind = TOKEN_KEYWORD_STRUCT;
  tl->tokens[0].start = (const uint8_t *)code;
  tl->tokens[0].length = 6;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  tl->tokens[1].kind = TOKEN_IDENTIFIER;
  tl->tokens[1].start = (const uint8_t *)(code + 7);
  tl->tokens[1].length = 8;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  tl->tokens[2].kind = TOKEN_LBRACE;
  tl->tokens[2].start = (const uint8_t *)(code + 16);
  tl->tokens[2].length = 1;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  tl->tokens[3].kind = TOKEN_RBRACE;
  tl->tokens[3].start = (const uint8_t *)(code + 18);
  tl->tokens[3].length = 1;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  tl->size = 4;
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST add_node_basic(void) {
  struct CstNodeList list = {NULL, 0, 0};
  /* LCOV_EXCL_STOP */
  size_t i;

  /* LCOV_EXCL_START */
  ASSERT_EQ(
      /* LCOV_EXCL_STOP */
      0, cst_list_add(&list, CST_NODE_STRUCT, (const uint8_t *)"abc", 3, 0, 0));
  /* LCOV_EXCL_START */
  ASSERT_EQ(1, list.size);
  ASSERT(list.nodes != NULL);
  ASSERT_EQ(CST_NODE_STRUCT, list.nodes[0].kind);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT(strncmp("abc", (const char *)list.nodes[0].start, 3) == 0);
  ASSERT_EQ(3, list.nodes[0].length);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  for (i = 1; i < 100; i++) {
    ASSERT_EQ(0, cst_list_add(&list, CST_NODE_COMMENT, (const uint8_t *)"x", 1,
                              /* LCOV_EXCL_STOP */
                              0, 0));
  }
  /* LCOV_EXCL_START */
  ASSERT_EQ(100, list.size);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_cst_node_list(&list);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            /* LCOV_EXCL_STOP */
            cst_list_add(NULL, CST_NODE_STRUCT, NULL, 0, 0, 0));
  /* LCOV_EXCL_START */
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_tokens_basic(void) {
  /* LCOV_EXCL_STOP */
  struct TokenList *tokens =
      /* LCOV_EXCL_START */
      (struct TokenList *)malloc(sizeof(struct TokenList));
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  struct CstNodeList copy_nodes = {NULL, 0, 0};
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!tokens)
    FAILm("Memory allocation failed");
  memset(tokens, 0, sizeof(*tokens));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  make_simple_token_list(tokens);
  if (!tokens->tokens) {
    free(tokens);
    FAILm("Setup failed");
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, parse_tokens(tokens, &cst_nodes));
  ASSERT_GT(cst_nodes.size, 0);
  /* LCOV_EXCL_STOP */

  {
    /* LCOV_EXCL_START */
    size_t found_struct = 0;
    /* LCOV_EXCL_STOP */
    size_t i;
    /* LCOV_EXCL_START */
    for (i = 0; i < cst_nodes.size; i++) {
      if (cst_nodes.nodes[i].kind == CST_NODE_STRUCT) {
        found_struct = 1;
        break;
        /* LCOV_EXCL_STOP */
      }
    }
    /* LCOV_EXCL_START */
    ASSERT(found_struct);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  copy_nodes.nodes = cst_nodes.nodes;
  copy_nodes.size = cst_nodes.size;
  copy_nodes.capacity = cst_nodes.capacity;
  free_cst_node_list(&copy_nodes);
  ASSERT_EQ(0, copy_nodes.size);
  ASSERT_EQ(0, copy_nodes.capacity);
  ASSERT(copy_nodes.nodes == NULL);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_token_list(tokens);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_tokens_empty(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  ASSERT_EQ(0, parse_tokens(&tokens, &cst_nodes));
  ASSERT_EQ(0, cst_nodes.size);
  ASSERT(cst_nodes.nodes == NULL);
  free_cst_node_list(&cst_nodes);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_tokens_null_args(void) {
  struct TokenList tokens = {NULL, 0, 0};
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_tokens(NULL, &cst_nodes));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_tokens(&tokens, NULL));
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_tokens_forward_declaration(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct MyStruct;");
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_tokens_anonymous_struct(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct { int x; };");
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  ASSERT_EQ(2, cst.size);
  ASSERT_EQ(CST_NODE_STRUCT, cst.nodes[0].kind);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_tokens_struct_variable_declaration(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  const az_span code = AZ_SPAN_FROM_STR("struct S { int x; } s;");
  size_t i, struct_nodes = 0, other_nodes = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_STRUCT) {
      struct_nodes++;
    } else if (cst.nodes[i].kind == CST_NODE_OTHER) {
      other_nodes++;
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  ASSERT_EQ(1, struct_nodes);
  ASSERT_EQ(2, other_nodes);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_simple_array_init(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* LCOV_EXCL_STOP */
  /* Should parse as ONE node due to assignment brace detection */
  /* LCOV_EXCL_START */
  const az_span code = AZ_SPAN_FROM_STR("int a[] = { 1, 2, 3 };");
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[0].kind);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_compound_literal(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* LCOV_EXCL_STOP */
  /* Should parse as ONE node due to (type) { ... } detection */
  /* LCOV_EXCL_START */
  const az_span code = AZ_SPAN_FROM_STR("var = (struct S){ .x = 1 };");
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[0].kind);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_control_block_split(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* LCOV_EXCL_STOP */
  /* Should scan 'if(1)' as one node, then stop at brace. */
  /* The scanner breaks on block-start braces unless matched as expr. */
  /* LCOV_EXCL_START */
  const az_span code = AZ_SPAN_FROM_STR("if (1) { x=1; }");
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  /* LCOV_EXCL_STOP */

  /* Expectation:
     Node 1: OTHER "if (1) " (whitespace included)
     Node 2: OTHER "{ x=1; }" -> Scanner sees {, is_expression=false, breaks.
     Next iter: sees {, consumes balanced brace block as one OTHER.
  */
  /* LCOV_EXCL_START */
  ASSERT_NEQ(
      /* LCOV_EXCL_STOP */
      1,
      cst.size); /* Should NOT be lumped into one if possible without logic */

  /* Verify at least 2 nodes */
  /* LCOV_EXCL_START */
  ASSERT_GTE(cst.size, 2);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_nested_compound_literal(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* LCOV_EXCL_STOP */
  /* Function call with compound literal argument */
  /* LCOV_EXCL_START */
  const az_span code = AZ_SPAN_FROM_STR("func((struct Point){0,0});");
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  /* LCOV_EXCL_STOP */

  /* Should be 1 statement node */
  /* LCOV_EXCL_START */
  ASSERT_EQ(1, cst.size);
  ASSERT_EQ(CST_NODE_OTHER, cst.nodes[0].kind);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_return_compound(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* LCOV_EXCL_STOP */
  /* Return compound literal */
  /* LCOV_EXCL_START */
  const az_span code = AZ_SPAN_FROM_STR("return (int[]){1,2};");
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(1, cst.size);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_c11_generic(void) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  /* LCOV_EXCL_STOP */
  /* _Generic selection */
  const az_span code =
      /* LCOV_EXCL_START */
      AZ_SPAN_FROM_STR("#define cbrt(X) _Generic((X), long double: cbrtl, "
                       /* LCOV_EXCL_STOP */
                       "default: cbrt, float: cbrtf)(X)");

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT_EQ(0, parse_tokens(tl, &cst));
  /* LCOV_EXCL_STOP */

  /*
     Node 0: MACRO (#define...) - cst_parser handles top level macros.
     BUT _Generic often inside a macro or function.
     Tokenizer makes #define a MACRO token only at start.
     Here it is one line macro.
     Wait, cst_parser lumps macros into CST_NODE_MACRO.
     So we need to test _Generic in a non-macro context (expression).
  */

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  /* LCOV_EXCL_STOP */

  {
    const az_span code2 =
        /* LCOV_EXCL_START */
        AZ_SPAN_FROM_STR("int x = _Generic(1.0, float: 1, default: 0);");
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    tl = NULL;
    memset(&cst, 0, sizeof(cst));
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    ASSERT_EQ(0, tokenize(code2, &tl));
    ASSERT_EQ(0, parse_tokens(tl, &cst));
    /* LCOV_EXCL_STOP */

    /*
      Nodes expected:
      1. OTHER "int x = "
      2. GENERIC_SELECTION "_Generic(1.0, ...)"
      3. OTHER ";"
    */
    /* LCOV_EXCL_START */
    ASSERT_EQ(3, cst.size);
    ASSERT_EQ(CST_NODE_OTHER, cst.nodes[0].kind);
    ASSERT_EQ(CST_NODE_GENERIC_SELECTION, cst.nodes[1].kind);
    ASSERT_EQ(CST_NODE_OTHER, cst.nodes[2].kind);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  free_token_list(tl);
  free_cst_node_list(&cst);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* Standard suite runner */

/* LCOV_EXCL_START */
TEST test_cst_find_first(void) {
  struct CstNodeList list = {0};
  struct CstNode *found = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  cst_list_add(&list, CST_NODE_STRUCT, (const uint8_t *)"a", 1, 0, 0);
  cst_list_add(&list, CST_NODE_ENUM, (const uint8_t *)"b", 1, 0, 0);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, cst_find_first(&list, CST_NODE_ENUM, &found));
  ASSERT(found != NULL);
  ASSERT_EQ(CST_NODE_ENUM, found->kind);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  found = NULL;
  ASSERT_EQ(0, cst_find_first(&list, CST_NODE_MACRO, &found));
  ASSERT(found == NULL);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            /* LCOV_EXCL_STOP */
            cst_find_first(&list, CST_NODE_STRUCT, NULL));

  /* LCOV_EXCL_START */
  free_cst_node_list(&list);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_cst_parser_extra(void) {
  cdd_cst_tree_t *tree = NULL;
  /* LCOV_EXCL_STOP */

  /* Empty tree free */
  /* LCOV_EXCL_START */
  cdd_cst_tree_t *t2 = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_tree_free(t2);
  /* LCOV_EXCL_STOP */

  /* Missing EOF / No tokens */
  /* LCOV_EXCL_START */
  cdd_cst_parse(az_span_create_from_str(""), &tree);
  cdd_cst_tree_free(tree);
  /* LCOV_EXCL_STOP */

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;

    /* LCOV_EXCL_START */
    g_cdd_cst_alloc_node_fail = 1;
    tree = NULL;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              /* LCOV_EXCL_STOP */
              cdd_cst_parse(az_span_create_from_str("int x;"), &tree));
    /* LCOV_EXCL_START */
    g_cdd_cst_alloc_node_fail = 0;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_cdd_cst_alloc_node_fail = 2;
    tree = NULL;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              /* LCOV_EXCL_STOP */
              cdd_cst_parse(az_span_create_from_str("int x;"), &tree));
    /* LCOV_EXCL_START */
    if (tree)
      cdd_cst_tree_free(tree);
    g_cdd_cst_alloc_node_fail = 0;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_cdd_cst_realloc_fail = 1;
    tree = NULL;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str("int x;"), &tree));
    if (tree)
      cdd_cst_tree_free(tree);
    g_cdd_cst_realloc_fail = 0;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_cdd_cst_realloc_fail = 2;
    tree = NULL;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   /* LCOV_EXCL_STOP */
                                   "int a, b, c, d, e, f, g, h, i, j;"),
                               &tree));
    /* LCOV_EXCL_START */
    if (tree)
      cdd_cst_tree_free(tree);
    g_cdd_cst_realloc_fail = 0;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_cdd_cst_realloc_fail = 2;
    tree = NULL;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   /* LCOV_EXCL_STOP */
                                   "int a, b, c, d, e, f, g, h, i, j;"),
                               &tree));
    /* LCOV_EXCL_START */
    if (tree)
      cdd_cst_tree_free(tree);
    g_cdd_cst_realloc_fail = 0;
    /* LCOV_EXCL_STOP */
  }
#endif
  /* LCOV_EXCL_START */
  g_fail_io_after = -1;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST parse_tokens_oom(void) {
/* LCOV_EXCL_STOP */
#ifdef CDD_BUILD_TESTS
  {
    /* LCOV_EXCL_START */
    struct TokenList *tl = NULL;
    /* LCOV_EXCL_STOP */
    struct CstNodeList cst_nodes;
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    int i;
    int rc;

    /* LCOV_EXCL_START */
    tokenize(az_span_create_from_str(
                 /* LCOV_EXCL_STOP */
                 "int main() { char *p = malloc(10); return 0; }"),
             &tl);

    /* LCOV_EXCL_START */
    memset(&cst_nodes, 0, sizeof(cst_nodes));
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    for (i = 1; i < 200; i++) {
      memset(&cst_nodes, 0, sizeof(cst_nodes));
      g_cdd_fail_alloc = i;
      rc = parse_tokens(tl, &cst_nodes);
      g_cdd_fail_alloc = 0;
      if (rc == 0) {
        free_cst_node_list(&cst_nodes);
        break;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      free_cst_node_list(&cst_nodes);
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    free_token_list(tl);
    /* LCOV_EXCL_STOP */
  }
#endif
  /* LCOV_EXCL_START */
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_cst_branches(void) {
  struct CstNode *out_node_ptr = NULL;
  ASSERT_EQ(0, cst_find_first(NULL, 0, &out_node_ptr));
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
SUITE(cst_parser_suite) {
  RUN_TEST(test_cst_parser_extra);
  RUN_TEST(add_node_basic);
  RUN_TEST(parse_tokens_basic);
  RUN_TEST(parse_tokens_empty);
  RUN_TEST(parse_tokens_null_args);
  RUN_TEST(parse_tokens_forward_declaration);
  RUN_TEST(parse_tokens_anonymous_struct);
  RUN_TEST(parse_tokens_struct_variable_declaration);
  RUN_TEST(parse_tokens_oom);
  RUN_TEST(test_cst_branches);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  RUN_TEST(parse_simple_array_init);
  RUN_TEST(parse_compound_literal);
  RUN_TEST(parse_control_block_split);
  RUN_TEST(parse_nested_compound_literal);
  RUN_TEST(parse_return_compound);
  /* LCOV_EXCL_STOP */

  RUN_TEST(parse_c11_generic); /* Added */
                               /* LCOV_EXCL_START */
  RUN_TEST(test_cst_find_first);
}
/* LCOV_EXCL_STOP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CST_PARSER_H */
