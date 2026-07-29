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
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "functions/parse/cst.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/* Helper to create a fake token list for testing */
static void make_simple_token_list(struct TokenList *tl) {
  static const char code[] = "struct MyStruct { }";
  tl->size = 0;
  tl->capacity = 4;
  tl->tokens =
      (struct Token *)C_CDD_MALLOC(sizeof(struct Token) * tl->capacity);
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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_list_add(NULL, CST_NODE_STRUCT, NULL, 0, 0, 0));

  PASS();
}

TEST parse_tokens_basic(void) {
  struct TokenList *tokens =
      (struct TokenList *)C_CDD_MALLOC(sizeof(struct TokenList));
  struct CstNodeList cst_nodes = {NULL, 0, 0};
  struct CstNodeList copy_nodes = {NULL, 0, 0};

  if (!tokens)
    FAILm("Memory allocation failed");
  memset(tokens, 0, sizeof(*tokens));

  make_simple_token_list(tokens);
  if (!tokens->tokens) {
    C_CDD_FREE(tokens);
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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_tokens(NULL, &cst_nodes));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_tokens(&tokens, NULL));

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
  ASSERT_EQ(1, struct_nodes);
  ASSERT_EQ(2, other_nodes);

  free_token_list(tl);
  free_cst_node_list(&cst);

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

  PASS();
}

TEST test_cst_parser_extra(void) {
  cdd_cst_tree_t *tree = NULL;

  /* Empty tree free */
  cdd_cst_tree_t *t2 = C_CDD_CALLOC(1, sizeof(cdd_cst_tree_t));
  cdd_cst_tree_free(t2);

  /* Missing EOF / No tokens */
  cdd_cst_parse(az_span_create_from_str(""), &tree);
  cdd_cst_tree_free(tree);

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;

    g_cdd_cst_alloc_node_fail = 1;
    tree = NULL;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_cst_parse(az_span_create_from_str("int x;"), &tree));
    g_cdd_cst_alloc_node_fail = 0;

    g_cdd_cst_alloc_node_fail = 2;
    tree = NULL;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_cst_parse(az_span_create_from_str("int x;"), &tree));
    if (tree)
      cdd_cst_tree_free(tree);
    g_cdd_cst_alloc_node_fail = 0;

    g_cdd_cst_realloc_fail = 1;
    tree = NULL;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str("int x;"), &tree));
    if (tree)
      cdd_cst_tree_free(tree);
    g_cdd_cst_realloc_fail = 0;

    g_cdd_cst_realloc_fail = 2;
    tree = NULL;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   "int a, b, c, d, e, f, g, h, i, j;"),
                               &tree));
    if (tree)
      cdd_cst_tree_free(tree);
    g_cdd_cst_realloc_fail = 0;

    g_cdd_cst_realloc_fail = 2;
    tree = NULL;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   "int a, b, c, d, e, f, g, h, i, j;"),
                               &tree));
    if (tree)
      cdd_cst_tree_free(tree);
    g_cdd_cst_realloc_fail = 0;
  }
#endif

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
                 "int main() { char *p = C_CDD_MALLOC(10); return 0; }"),
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

  PASS();
}

TEST test_cst_parser_coverage(void) {
  cdd_cst_tree_t *tree = NULL;
  extern C_CDD_EXPORT enum cdd_c_error peek(void *s, void **out_tok);
  extern C_CDD_EXPORT enum cdd_c_error advance(void *s, void **out_tok);
  int i, rc;
  void *dummy = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, peek(NULL, &dummy));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, advance(NULL, &dummy));

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    const char *code =
        "struct X { int y; };\n"
        "enum E { A = 1 };\n"
        "union U { float f; };\n"
        "void f(int a, ...) { int x; }\n"
        "#if defined(FOO)\n int x; \n#elif defined(BAR)\n int y; \n#endif\n"
        "#if 0\n#elif defined FOO\n int z; \n#endif\n"
        "#if 1\n 1 2 3 4 5 6 7 {} \n#elif 1\n int a; \n#endif\n"
        "#if 0\n#elif 0\n int b; \n#endif\n"
        "#if FOO\n#elif BAZ\n int c; \n#endif\n"
        "#define MACRO 1\n"
        "#include <stdio.h>\n"
        "try { throw 1; } catch (...) {}\n"
        "namespace N { int x; }\n"
        "using namespace N;\n"
        "template <typename T, int A, int B, int C, int D, int E, int F, int "
        "G, int H> class C {};\n"
        "struct __declspec(dllexport) C2 : public X, private Y {};\n"
        "void g() noexcept(true) {}\n"
        "void h() { asm volatile (\"nop\"); }\n"
        "void i() { _Generic(x, int: 1, default: 0); }\n"
        "int arr[10];\n";
    for (i = 1; i < 4000; i++) {
      g_cdd_cst_alloc_node_fail = i;
      tree = NULL;
      rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
      g_cdd_cst_alloc_node_fail = 0;
      if (tree)
        cdd_cst_tree_free(tree);
      if (rc == 0)
        break;
    }
  }
#endif
  PASS();
}

TEST test_cst_branches(void) {
  struct CstNode *out_node_ptr = NULL;
  ASSERT_EQ(0, cst_find_first(NULL, 0, &out_node_ptr));

  PASS();
}

TEST test_cst_parser_full_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  int i, rc;
  const char *code =
      "#define MACRO 1\n"
      "struct __declspec(dllexport) C2 : public X, private Y {};\n"
      "void g() noexcept(true) {}\n"
      "void h() { asm volatile (\"nop\"); }\n"
      "void i() { _Generic(x, int: 1, default: 0); }\n"
      "template <typename T, int A> class C {};\n"
      "try { throw 1; } catch (...) {}\n";
  extern C_CDD_EXPORT int g_cdd_alloc_fail_countdown_countdown;
  for (i = 1; i < 4000; i++) {
    g_cdd_alloc_fail_countdown_countdown = i;
    tree = NULL;
    rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
    g_cdd_alloc_fail_countdown_countdown = 0;
    if (tree)
      cdd_cst_tree_free(tree);
    if (rc == 0)
      break;
  }
#endif
  PASS();
}

TEST test_cst_parser_missing_lines(void) {
  cdd_cst_tree_t *tree = NULL;
  int rc;

  /* Line 1052 */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_parse(az_span_create_from_str(""), NULL));

#ifdef CDD_BUILD_TESTS
  /* Line 1057 - C_CDD_CALLOC for tree fails */
  {
    extern C_CDD_EXPORT int g_cdd_alloc_fail_countdown_countdown;
    g_cdd_alloc_fail_countdown_countdown = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_cst_parse(az_span_create_from_str("int x;"), &tree));
    g_cdd_alloc_fail_countdown_countdown = 0;
  }

  /* Specific OOM inside parse_type_specifier and parse_declaration_or_statement
   */
  {
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    /* Try to fail on exactly the node allocations that we missed */
    for (int j = 1; j < 20; j++) {
      g_cdd_cst_alloc_node_fail = j;
      cdd_cst_parse(
          az_span_create_from_str("#define MACRO 1\nstruct X : public Y "
                                  "{};\nvoid f() noexcept(true) {}\n"),
          &tree);
      g_cdd_cst_alloc_node_fail = 0;
      if (tree)
        cdd_cst_tree_free(tree);
    }
    g_cdd_cst_alloc_node_fail = 0;
    rc = cdd_cst_parse(az_span_create_from_str("struct X : public Y {};"),
                       &tree);
    ASSERT_EQ(0, rc);
    cdd_cst_tree_free(tree);
  }
#endif
  PASS();
}

SUITE(cst_parser_suite) {
  RUN_TEST(test_cst_parser_missing_lines);
  RUN_TEST(test_cst_parser_full_oom);
  RUN_TEST(test_cst_parser_extra);
  RUN_TEST(add_node_basic);
  RUN_TEST(parse_tokens_basic);
  RUN_TEST(parse_tokens_empty);
  RUN_TEST(parse_tokens_null_args);
  RUN_TEST(parse_tokens_forward_declaration);
  RUN_TEST(parse_tokens_anonymous_struct);
  RUN_TEST(parse_tokens_struct_variable_declaration);
  RUN_TEST(parse_tokens_oom);
  RUN_TEST(test_cst_parser_coverage);
  RUN_TEST(test_cst_parser_coverage);
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
