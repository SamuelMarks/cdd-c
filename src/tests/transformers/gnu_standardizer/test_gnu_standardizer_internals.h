/**
 * @file test_gnu_standardizer_internals.h
 * @brief Tests for gnu_standardizer helper functions.
 */

#ifndef TEST_GNU_STANDARDIZER_INTERNALS_H
#define TEST_GNU_STANDARDIZER_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern C_CDD_EXPORT int g_gnu_standardizer_fail;
extern C_CDD_EXPORT const char *pool_string_safe(cdd_cst_tree_t *tree,
                                                 const char *str);
extern C_CDD_EXPORT const char *
pool_string_safe_len(cdd_cst_tree_t *tree, const char *str, size_t len);

TEST test_gnu_pool_string_safe(void) {
  cdd_cst_tree_t tree;
  const char *pooled = NULL;
  cdd_c_error_t rc;
  size_t i;
  char buf[32];

  memset(&tree, 0, sizeof(tree));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_pool_string_safe(NULL, "test", &pooled));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_pool_string_safe(&tree, NULL, &pooled));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_pool_string_safe(&tree, "test", NULL));

  /* Test fail mocks */
  g_gnu_standardizer_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_pool_string_safe(&tree, "fail1", &pooled));
  g_gnu_standardizer_fail = 4;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_pool_string_safe(&tree, "fail4", &pooled));
  g_gnu_standardizer_fail = 0;

  rc = cdd_pool_string_safe(&tree, "first", &pooled);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("first", pooled);
  ASSERT_EQ(1, tree.num_strings);

  /* Force pool expansion */
  for (i = 0; i < 40; i++) {
    buf[0] = 'a';
    buf[1] = (char)('0' + (i % 10));
    buf[2] = '\0';
    rc = cdd_pool_string_safe(&tree, buf, &pooled);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }
  ASSERT(tree.string_capacity >= 41);

  for (i = 0; i < tree.num_strings; i++) {
    free(tree.string_pool[i]);
  }
  free(tree.string_pool);
  PASS();
}

TEST test_gnu_pool_string_safe_len(void) {
  cdd_cst_tree_t tree;
  const char *pooled = NULL;
  cdd_c_error_t rc;
  size_t i;
  char buf[32];

  memset(&tree, 0, sizeof(tree));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_pool_string_safe_len(NULL, "test", 4, &pooled));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_pool_string_safe_len(&tree, NULL, 4, &pooled));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_pool_string_safe_len(&tree, "test", 4, NULL));

  /* Test fail mocks */
  g_gnu_standardizer_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_pool_string_safe_len(&tree, "fail2", 5, &pooled));
  g_gnu_standardizer_fail = 5;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_pool_string_safe_len(&tree, "fail5", 5, &pooled));
  g_gnu_standardizer_fail = 0;

  rc = cdd_pool_string_safe_len(&tree, "hello world", 5, &pooled);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("hello", pooled);
  ASSERT_EQ(1, tree.num_strings);

  /* Force pool expansion */
  for (i = 0; i < 40; i++) {
    buf[0] = 'b';
    buf[1] = (char)('0' + (i % 10));
    buf[2] = '\0';
    rc = cdd_pool_string_safe_len(&tree, buf, 2, &pooled);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }
  ASSERT(tree.string_capacity >= 41);

  for (i = 0; i < tree.num_strings; i++) {
    free(tree.string_pool[i]);
  }
  free(tree.string_pool);
  PASS();
}

TEST test_gnu_append_int(void) {
  char buf[64];
  char *out_p = NULL;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_append_int(NULL, 0, &out_p));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_append_int(buf, 0, NULL));

  g_gnu_standardizer_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_append_int(buf, 10, &out_p));
  g_gnu_standardizer_fail = 0;

  rc = cdd_append_int(buf, 0, &out_p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("0", buf);
  ASSERT_EQ(buf + 1, out_p);

  rc = cdd_append_int(buf, 42, &out_p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("42", buf);
  ASSERT_EQ(buf + 2, out_p);

  rc = cdd_append_int(buf, -999, &out_p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("-999", buf);
  ASSERT_EQ(buf + 4, out_p);
  PASS();
}

TEST test_gnu_parse_128_literal(void) {
  uint64_t high = 0;
  uint64_t low = 0;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_parse_128_literal(NULL, 5, &high, &low));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_parse_128_literal("123", 3, NULL, &low));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_parse_128_literal("123", 3, &high, NULL));

  rc = cdd_parse_128_literal("12345ULL", 8, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, high);
  ASSERT_EQ(12345, low);

  /* Digit < '0' suffix */
  rc = cdd_parse_128_literal("123+4", 5, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(123, low);

  /* Digit > '9' suffix */
  rc = cdd_parse_128_literal("123:4", 5, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(123, low);

  /* Very large number exceeding 64 bits */
  rc = cdd_parse_128_literal("18446744073709551616", 20, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, high);
  ASSERT_EQ(0, low);
  PASS();
}

TEST test_gnu_parse_hex_128_literal(void) {
  uint64_t high = 0;
  uint64_t low = 0;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_parse_hex_128_literal(NULL, 5, &high, &low));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_parse_hex_128_literal("0x123", 5, NULL, &low));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_parse_hex_128_literal("0x123", 5, &high, NULL));

  rc = cdd_parse_hex_128_literal("0xaA09z", 7, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, high);
  ASSERT_EQ(0xAA09, low);

  rc = cdd_parse_hex_128_literal("0x123u", 6, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, high);
  ASSERT_EQ(0x123, low);

  rc = cdd_parse_hex_128_literal("0xABCDEFabcdef", 14, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ((((uint64_t)0xABCDEF) << 24) | (uint64_t)0xABCDEF, low);

  /* Branch tests: char < '0' */
  rc = cdd_parse_hex_128_literal("0x1/2", 5, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, low);

  /* Branch tests: char > '9' and < 'A' */
  rc = cdd_parse_hex_128_literal("0x1:2", 5, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, low);

  /* Branch tests: char == '@' */
  rc = cdd_parse_hex_128_literal("0x1@2", 5, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, low);

  /* Branch tests: char > 'F' and < 'a' */
  rc = cdd_parse_hex_128_literal("0x1G2", 5, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, low);

  /* Branch tests: char == '`' */
  rc = cdd_parse_hex_128_literal("0x1`2", 5, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, low);

  /* Branch tests: char > 'f' */
  rc = cdd_parse_hex_128_literal("0x1g2", 5, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, low);

  /* Hex exceeding 64-bit */
  rc = cdd_parse_hex_128_literal("0x10000000000000000", 19, &high, &low);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, high);
  ASSERT_EQ(0, low);
  PASS();
}

TEST test_gnu_infer_type(void) {
  const char *out_type = NULL;
  cdd_token_t tokens[4];

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_infer_type(NULL, 0, NULL));

  /* Empty token list defaults to int */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(NULL, 0, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* Int keyword is already a type */
  memset(tokens, 0, sizeof(tokens));
  tokens[0].kind = CDD_TOKEN_KEYWORD_INT;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  /* Primitive identifier: char, float, double, void, union, enum, unsigned,
   * signed, long, short */
  tokens[0].kind = CDD_TOKEN_IDENTIFIER;
  tokens[0].start = (const uint8_t *)"char";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  tokens[0].start = (const uint8_t *)"float";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  tokens[0].start = (const uint8_t *)"double";
  tokens[0].length = 6;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  tokens[0].start = (const uint8_t *)"void";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  tokens[0].start = (const uint8_t *)"union";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  tokens[0].start = (const uint8_t *)"enum";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  tokens[0].start = (const uint8_t *)"unsigned";
  tokens[0].length = 8;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  tokens[0].start = (const uint8_t *)"signed";
  tokens[0].length = 6;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  tokens[0].start = (const uint8_t *)"long";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  tokens[0].start = (const uint8_t *)"short";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  /* Bitfield inference: obj.flag */
  tokens[0].kind = CDD_TOKEN_IDENTIFIER;
  tokens[0].start = (const uint8_t *)"obj";
  tokens[0].length = 3;
  tokens[1].kind = CDD_TOKEN_DOT;
  tokens[1].start = (const uint8_t *)".";
  tokens[1].length = 1;
  tokens[2].kind = CDD_TOKEN_IDENTIFIER;
  tokens[2].start = (const uint8_t *)"flag";
  tokens[2].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 3, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* Bitfield inference: ptr->is_valid */
  tokens[1].kind = CDD_TOKEN_ARROW;
  tokens[2].start = (const uint8_t *)"is_valid";
  tokens[2].length = 8;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 3, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* Bitfield inference: ptr->has_data */
  tokens[2].start = (const uint8_t *)"has_data";
  tokens[2].length = 8;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 3, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* String literal */
  tokens[0].kind = CDD_TOKEN_STRING;
  tokens[0].start = (const uint8_t *)"\"hello\"";
  tokens[0].length = 7;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("const char *", out_type);

  /* Number literal float */
  tokens[0].kind = CDD_TOKEN_NUMBER;
  tokens[0].start = (const uint8_t *)"1.5f";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("float", out_type);

  /* Number literal double */
  tokens[0].start = (const uint8_t *)"2.5";
  tokens[0].length = 3;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("double", out_type);

  /* Number literal unsigned long */
  tokens[0].start = (const uint8_t *)"123ul";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("unsigned long", out_type);

  /* Number literal unsigned int */
  tokens[0].start = (const uint8_t *)"123u";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("unsigned int", out_type);

  /* Number literal long */
  tokens[0].start = (const uint8_t *)"123l";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("long", out_type);

  /* Number literal int */
  tokens[0].start = (const uint8_t *)"123";
  tokens[0].length = 3;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* Char literal */
  tokens[0].kind = CDD_TOKEN_CHAR;
  tokens[0].start = (const uint8_t *)"'a'";
  tokens[0].length = 3;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* Fallback */
  tokens[0].kind = CDD_TOKEN_IDENTIFIER;
  tokens[0].start = (const uint8_t *)"some_var";
  tokens[0].length = 8;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("int", out_type);

  PASS();
}

TEST test_gnu_asm_and_visitors(void) {
  struct tramp_ctx tctx;
  cdd_cst_node_t node;
  cdd_cst_child_t children[2];
  cdd_token_t tok1, tok2;

  /* Test cdd_asm_visitor */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_asm_visitor(NULL, NULL));

  memset(&node, 0, sizeof(node));
  node.kind = CDD_CST_ASM_STATEMENT;
  node.num_children = 2;
  node.children = children;

  memset(&tok1, 0, sizeof(tok1));
  tok1.kind = CDD_TOKEN_STRING;
  tok1.start = (const uint8_t *)"\"r\"";
  tok1.length = 3;

  memset(&tok2, 0, sizeof(tok2));
  tok2.kind = CDD_TOKEN_IDENTIFIER;
  tok2.start = (const uint8_t *)"my_sym";
  tok2.length = 6;

  children[0].kind = CDD_CST_CHILD_TOKEN;
  children[0].val.token = &tok1;
  children[1].kind = CDD_CST_CHILD_TOKEN;
  children[1].val.token = &tok2;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_asm_visitor(&node, NULL));

  /* Test cdd_tramp_visitor */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_tramp_visitor(NULL, NULL));
  memset(&tctx, 0, sizeof(tctx));
  tctx.is_tramp = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_tramp_visitor(NULL, &tctx));
  tctx.is_tramp = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_tramp_visitor(NULL, &tctx));

  /* Trampoline visitor matching name without LPAREN */
  tctx.name = (const uint8_t *)"my_sym";
  tctx.length = 6;
  tok2.length = 6;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_tramp_visitor(&node, &tctx));
  ASSERT_EQ(1, tctx.is_tramp);

  /* Trampoline visitor matching name when node == tctx.func_node */
  tctx.is_tramp = 0;
  tctx.func_node = &node;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_tramp_visitor(&node, &tctx));
  ASSERT_EQ(0, tctx.is_tramp);
  tctx.func_node = NULL;

  /* Trampoline visitor when followed by empty token and LPAREN */
  {
    cdd_token_t toks[3];
    cdd_cst_child_t ch_call[3];
    cdd_cst_node_t node_call;

    memset(toks, 0, sizeof(toks));
    toks[0].kind = CDD_TOKEN_IDENTIFIER;
    toks[0].start = (const uint8_t *)"my_sym";
    toks[0].length = 6;

    toks[1].length = 0;

    toks[2].kind = CDD_TOKEN_LPAREN;
    toks[2].length = 1;
    toks[2].start = (const uint8_t *)"(";

    memset(&node_call, 0, sizeof(node_call));
    node_call.num_children = 3;
    node_call.children = ch_call;
    ch_call[0].kind = CDD_CST_CHILD_TOKEN;
    ch_call[0].val.token = &toks[0];
    ch_call[1].kind = CDD_CST_CHILD_TOKEN;
    ch_call[1].val.token = &toks[1];
    ch_call[2].kind = CDD_CST_CHILD_TOKEN;
    ch_call[2].val.token = &toks[2];

    tctx.is_tramp = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_tramp_visitor(&node_call, &tctx));
    ASSERT_EQ(0, tctx.is_tramp);
  }

  /* Magic visitor NULL checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_magic_visitor(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_magic_visitor(&node, NULL));

  /* Magic visitor full transformation */
  {
    struct magic_ctx mctx;
    cdd_cst_tree_t mtree;
    cdd_token_t tok_func, tok_pretty, tok_c99;
    cdd_cst_child_t ch_magic[3];
    cdd_cst_node_t node_magic;

    memset(&mtree, 0, sizeof(mtree));
    memset(&mctx, 0, sizeof(mctx));
    mctx.tree = &mtree;
    mctx.func_name = (const uint8_t *)"test_fn";
    mctx.func_len = 7;

    memset(&tok_func, 0, sizeof(tok_func));
    tok_func.kind = CDD_TOKEN_IDENTIFIER;
    tok_func.start = (const uint8_t *)"__FUNCTION__";
    tok_func.length = 12;

    memset(&tok_pretty, 0, sizeof(tok_pretty));
    tok_pretty.kind = CDD_TOKEN_IDENTIFIER;
    tok_pretty.start = (const uint8_t *)"__PRETTY_FUNCTION__";
    tok_pretty.length = 19;

    memset(&tok_c99, 0, sizeof(tok_c99));
    tok_c99.kind = CDD_TOKEN_IDENTIFIER;
    tok_c99.start = (const uint8_t *)"__func__";
    tok_c99.length = 8;

    memset(&node_magic, 0, sizeof(node_magic));
    node_magic.num_children = 3;
    node_magic.children = ch_magic;
    ch_magic[0].kind = CDD_CST_CHILD_TOKEN;
    ch_magic[0].val.token = &tok_func;
    ch_magic[1].kind = CDD_CST_CHILD_TOKEN;
    ch_magic[1].val.token = &tok_pretty;
    ch_magic[2].kind = CDD_CST_CHILD_TOKEN;
    ch_magic[2].val.token = &tok_c99;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_magic_visitor(&node_magic, &mctx));

    {
      size_t mi;
      for (mi = 0; mi < mtree.num_strings; mi++) {
        free(mtree.string_pool[mi]);
      }
      free(mtree.string_pool);
      mtree.string_pool = NULL;
      mtree.num_strings = 0;
      mtree.string_capacity = 0;
    }

    /* Magic visitor fail mock (pooled == NULL) */
    {
      cdd_cst_node_t node_err;
      cdd_cst_child_t ch_err;
      cdd_token_t tok_err;

      memset(&tok_err, 0, sizeof(tok_err));
      tok_err.kind = CDD_TOKEN_IDENTIFIER;
      tok_err.start = (const uint8_t *)"__func__";
      tok_err.length = 8;

      memset(&node_err, 0, sizeof(node_err));
      node_err.num_children = 1;
      node_err.children = &ch_err;
      ch_err.kind = CDD_CST_CHILD_TOKEN;
      ch_err.val.token = &tok_err;

      g_gnu_standardizer_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_magic_visitor(&node_err, &mctx));
      g_gnu_standardizer_fail = 0;
    }
  }

  PASS();
}

TEST test_gnu_pool_helpers(void) {
  cdd_cst_tree_t tree;
  const char *res = NULL;
  size_t i;
  memset(&tree, 0, sizeof(tree));

  ASSERT(pool_string_safe(NULL, NULL) == NULL);
  ASSERT(pool_string_safe_len(NULL, NULL, 0) == NULL);

  res = pool_string_safe(&tree, "pooled_str");
  ASSERT(res != NULL);
  ASSERT_STR_EQ("pooled_str", res);

  res = pool_string_safe_len(&tree, "len_str_abc", 7);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("len_str", res);

  for (i = 0; i < tree.num_strings; i++) {
    free(tree.string_pool[i]);
  }
  free(tree.string_pool);
  PASS();
}

SUITE(transformer_gnu_standardizer_internals_suite) {
  RUN_TEST(test_gnu_pool_string_safe);
  RUN_TEST(test_gnu_pool_string_safe_len);
  RUN_TEST(test_gnu_append_int);
  RUN_TEST(test_gnu_parse_128_literal);
  RUN_TEST(test_gnu_parse_hex_128_literal);
  RUN_TEST(test_gnu_infer_type);
  RUN_TEST(test_gnu_asm_and_visitors);
  RUN_TEST(test_gnu_pool_helpers);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_GNU_STANDARDIZER_INTERNALS_H */
