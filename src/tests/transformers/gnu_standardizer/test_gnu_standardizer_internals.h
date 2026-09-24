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
extern C_CDD_EXPORT volatile int g_gnu_alloc_fail;
extern C_CDD_EXPORT volatile int g_gnu_replace_fail;
extern C_CDD_EXPORT volatile int g_gnu_bld_fail;
extern C_CDD_EXPORT volatile int g_gnu_malloc_fail;
extern C_CDD_EXPORT volatile int g_gnu_find_fail;
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

  tokens[0].start = (const uint8_t *)"1.5F";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("float", out_type);

  /* Number literal double with p, P, e, E, L */
  tokens[0].start = (const uint8_t *)"2.5";
  tokens[0].length = 3;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("double", out_type);

  tokens[0].start = (const uint8_t *)"0x1p5";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("double", out_type);

  tokens[0].start = (const uint8_t *)"0x1P5";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("double", out_type);

  tokens[0].start = (const uint8_t *)"1e5";
  tokens[0].length = 3;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("double", out_type);

  tokens[0].start = (const uint8_t *)"1E5";
  tokens[0].length = 3;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("double", out_type);

  tokens[0].start = (const uint8_t *)"1.5L";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("double", out_type);

  /* Number literal unsigned long */
  tokens[0].start = (const uint8_t *)"123ul";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("unsigned long", out_type);

  tokens[0].start = (const uint8_t *)"123UL";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("unsigned long", out_type);

  tokens[0].start = (const uint8_t *)"123uL";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("unsigned long", out_type);

  tokens[0].start = (const uint8_t *)"123Ul";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("unsigned long", out_type);

  /* Number literal unsigned int */
  tokens[0].start = (const uint8_t *)"123u";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("unsigned int", out_type);

  tokens[0].start = (const uint8_t *)"123U";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("unsigned int", out_type);

  /* Number literal long */
  tokens[0].start = (const uint8_t *)"123l";
  tokens[0].length = 4;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("long", out_type);

  tokens[0].start = (const uint8_t *)"123L";
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

  /* Extra branch coverage: num_tokens == 0 with non-null tokens */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 0, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* Struct keyword */
  tokens[0].kind = CDD_TOKEN_KEYWORD_STRUCT;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_EQ(NULL, out_type);

  /* Length 6 identifier not 'signed' or 'double' */
  tokens[0].kind = CDD_TOKEN_IDENTIFIER;
  tokens[0].start = (const uint8_t *)"foobar";
  tokens[0].length = 6;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* Length 5 identifier not 'short', 'float', 'union' */
  tokens[0].start = (const uint8_t *)"dummy";
  tokens[0].length = 5;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 1, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* Arrow bitfield with non-matching identifier */
  tokens[0].start = (const uint8_t *)"ptr";
  tokens[0].length = 3;
  tokens[1].kind = CDD_TOKEN_ARROW;
  tokens[2].kind = CDD_TOKEN_IDENTIFIER;
  tokens[2].start = (const uint8_t *)"other_field";
  tokens[2].length = 11;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 3, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* 3 tokens where middle is PLUS (so DOT is false and ARROW is false) */
  tokens[0].kind = CDD_TOKEN_IDENTIFIER;
  tokens[1].kind = CDD_TOKEN_PLUS;
  tokens[2].kind = CDD_TOKEN_IDENTIFIER;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 3, &out_type));
  ASSERT_STR_EQ("int", out_type);

  /* 2 tokens so num_tokens == 1 is false for string and number */
  tokens[0].kind = CDD_TOKEN_IDENTIFIER;
  tokens[1].kind = CDD_TOKEN_IDENTIFIER;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tokens, 2, &out_type));
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

  /* Test cdd_magic_visitor with length 19 dummy identifier */
  {
    struct magic_ctx mctx;
    cdd_cst_tree_t mtree;
    cdd_cst_node_t mnode;
    cdd_cst_child_t mch;
    cdd_token_t mtok;
    memset(&mctx, 0, sizeof(mctx));
    memset(&mtree, 0, sizeof(mtree));
    memset(&mnode, 0, sizeof(mnode));
    memset(&mch, 0, sizeof(mch));
    memset(&mtok, 0, sizeof(mtok));
    mtok.kind = CDD_TOKEN_IDENTIFIER;
    mtok.start = (const uint8_t *)"dummy_identifier_19";
    mtok.length = 19;
    mch.kind = CDD_CST_CHILD_TOKEN;
    mch.val.token = &mtok;
    mnode.num_children = 1;
    mnode.children = &mch;
    mctx.tree = &mtree;
    mctx.func_name = (const uint8_t *)"f";
    mctx.func_len = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_magic_visitor(&mnode, &mctx));

    /* Test cdd_magic_visitor failure with g_gnu_malloc_fail */
    mtok.start = (const uint8_t *)"__FUNCTION__";
    mtok.length = 12;
    g_gnu_malloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_magic_visitor(&mnode, &mctx));
    g_gnu_malloc_fail = 0;

    g_gnu_standardizer_fail = 32;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_magic_visitor(&mnode, &mctx));
    g_gnu_standardizer_fail = 33;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_magic_visitor(&mnode, &mctx));
    g_gnu_standardizer_fail = 0;
  }

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

TEST test_gnu_standardizer_reach_100_percent_coverage(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_tree_t empty_tree;
  cdd_transform_config_t config;
  size_t i;
  static const char *const snippets[] = {
      "#define OBJ_MACRO 123\n"
      "#define EMPTY_MACRO\n"
      "#define ADD(a, b) ((a) + (b))\n"
      "#define LOG(my_arg...) fn(my_arg)\n"
      "#define FOO(args...) foo(##args, ##args_extra, args, args_extra)\n"
      "#define VA_MACRO(...) fn(__VA_ARGS__)\n"
      "#define TEST_MACRO(my_args, a, args...) foo(##a, ##args, ##args1, "
      "##args_extra, a, args, args1, args_extra, myargs, _args)\n",

      "int dummy_identifier19 = 1;\n"
      "int dummy_length_16_x = 2;\n"
      "int dummy_len_23_identifier = 3;\n"
      "int dummy_len_17_ident = 4;\n"
      "int dummy_len11 = 5;\n"
      "int dummy_len9 = 6;\n"
      "int dummy_len_13_x = 7;\n"
      "int alignof_x = 8;\n",

      "void __attribute__((noinline)) f1(void);\n"
      "void __attribute__((always)) f2(void);\n"
      "void __attribute__((transparent_unit_)) f3(void);\n"
      "void __attribute__((vector_wide)) f4(void);\n"
      "void __attribute__((cleanup)) f5(void);\n"
      "void __attribute__((pure)) f6(void);\n",

      "typedef int QItype __attribute__((mode(QI)));\n"
      "typedef int HItype __attribute__((mode(HI)));\n"
      "typedef int SItype __attribute__((mode(SI)));\n"
      "typedef int DItype __attribute__((mode(DI)));\n"
      "typedef int TItype __attribute__((mode(TI)));\n"
      "typedef int XXtype __attribute__((mode(XX)));\n"
      "typedef int Xtype __attribute__((mode(X)));\n",

      "unsigned __int128 h1 = 0X10000000000000000;\n"
      "unsigned __int128 o1 = 01777777777777777777777;\n"
      "volatile __int128 v128;\n"
      "__int128 i128;\n"
      "_Decimal32 d32; _Fract fr;\n"
      "_Decimal64 d64; _Decimal128 d128; _Accum acc;\n"
      "__fp16 f16; _Float16 fl16; __bf16 bf;\n"
      "typeof_unqual(const volatile restrict unsigned float) unq;\n",

      "union MyU { int a; }; union MyU u = {};\n"
      "enum MyE { E0 }; enum MyE e = {};\n"
      "int myvar = {};\n"
      "int item = {};\n",

      "int test_cases(int x) {\n"
      "  switch(x) { case 5 ... 1: break; default: break; }\n"
      "  return 0;\n"
      "}\n"
      "int test_neg(int x) {\n"
      "  switch(x) { case -5 ... -1: break; case 1 ... -1: break; }\n"
      "  return 0;\n"
      "}\n"
      "int r1[10] = { [5 ... 1] = 0 };\n"
      "int r2[10] = { [0 ... 3] };\n"
      "int r3[10] = { [-5 ... -1] = 1 };\n",

      "void test_elvis(int x) {\n"
      "  int item = x ?: 0;\n"
      "  int value = x ?: 0;\n"
      "  int num = x ?: 0;\n"
      "  int id = x ?: 0;\n"
      "  int result = x ?: 0;\n"
      "  if (x ?: 0) {}\n"
      "  while (x ?: 0) {}\n"
      "  for (x ?: 0; ; ) break;\n"
      "  switch (x ?: 0) { case 1 ?: 0: break; }\n"
      "  do {} while (x ?: 0);\n"
      "}\n",

      "int test_ret_cast(char c) { return (char)c; }\n"
      "int test_exp(int x) {\n"
      "  if (__builtin_expect(x, 1)) return 1;\n"
      "  if (__builtin_expect(x)) return 2;\n"
      "  return 0;\n"
      "}\n"
      "int test_amp(int x) { int *p = &x; return *p; }\n"
      "void f_nest(void) { { void g_nest(void) {} } }\n",

      "void clean_fn(int *p) {}\n"
      "void f_clean(void) { int __attribute__((cleanup(clean_fn))) a, b; }\n"
      "void f_addr_lbl(void) { __label__ l1; void *p = &&l1; goto l1; l1:; }\n"
      "void f_vla_params(int n, int a[n], int b[n]) {}\n"
      "int f_vla_ret(int n) { int a[n]; return 1; }\n"};

  memset(&config, 0, sizeof(config));
  config.fallback_vla_to_malloc = 1;
  memset(&empty_tree, 0, sizeof(empty_tree));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_gnu(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_gnu(&empty_tree, NULL));

  /* Test replace_token_with_text edge cases directly */
  {
    cdd_token_t dummy_t;
    cdd_cst_tree_t tr;
    cdd_cst_tree_t *tr_valid = NULL;
    memset(&dummy_t, 0, sizeof(dummy_t));
    memset(&tr, 0, sizeof(tr));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        replace_token_with_text(NULL, &dummy_t, CDD_TOKEN_IDENTIFIER, "", 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              replace_token_with_text(&tr, NULL, CDD_TOKEN_IDENTIFIER, "", 0));
    ASSERT_NEQ(CDD_C_SUCCESS, replace_token_with_text(
                                  &tr, &dummy_t, CDD_TOKEN_IDENTIFIER, "", 0));

    if (cdd_cst_parse(az_span_create_from_str((char *)(size_t) "int x;"),
                      &tr_valid) == 0 &&
        tr_valid) {
      g_gnu_standardizer_fail = 30;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                replace_token_with_text(tr_valid,
                                        &tr_valid->base_tokens->tokens[0],
                                        CDD_TOKEN_IDENTIFIER, "", 0));
      g_gnu_standardizer_fail = 0;
      cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(
            az_span_create_from_str((char *)(size_t) "void f(void) {}\n"),
            &tr_valid) == 0 &&
        tr_valid) {
      g_gnu_standardizer_fail = 34;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_gnu_standardizer_fail = 0;
      cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(
            az_span_create_from_str((char *)(size_t) "typeof(1) x;\n"),
            &tr_valid) == 0 &&
        tr_valid) {
      g_gnu_standardizer_fail = 35;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_gnu_standardizer_fail = 0;
      cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(
            az_span_create_from_str((
                char *)(size_t) "unsigned __int128 x = 0x10000000000000000;\n"),
            &tr_valid) == 0 &&
        tr_valid) {
      g_gnu_standardizer_fail = 36;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_gnu_standardizer_fail = 0;
      cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(
            az_span_create_from_str((
                char
                    *)(size_t) "unsigned __int128 x = 18446744073709551616;\n"),
            &tr_valid) == 0 &&
        tr_valid) {
      g_gnu_standardizer_fail = 37;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_gnu_standardizer_fail = 0;
      cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(
            az_span_create_from_str((char *)(size_t) "typeof(int[]) x;\n"),
            &tr_valid) == 0 &&
        tr_valid) {
      g_gnu_standardizer_fail = 15;
      g_gnu_bld_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_gnu_standardizer_fail = 0;
      g_gnu_bld_fail = 0;
      cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(
            az_span_create_from_str((char *)(size_t) "__auto_type x = 1;\n"),
            &tr_valid) == 0 &&
        tr_valid) {
      g_gnu_standardizer_fail = 39;
      cdd_transform_gnu(tr_valid, &config);
      g_gnu_standardizer_fail = 0;
      cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(
            az_span_create_from_str(
                (char *)(size_t) "int f(int n) { int a[n]; return 1; }\n"),
            &tr_valid) == 0 &&
        tr_valid) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tr_valid, NULL));
      cdd_cst_tree_free(tr_valid);
    }
  }

  {
    static const char *const test_codes[] = {
        "void f(void) __attribute__((pure));\n",
        "struct __attribute__((packed)) X { int a; };\n",
        "void f(void) __attribute__((noreturn));\n",
        "void f(void) __attribute__((unused));\n",
        "union __attribute__((transparent_union)) TU { int a; };\n",
        "typedef int v4si __attribute__((vector_size(16)));\n",
        "int x __attribute__((aligned(8)));\n",
        "typedef int SItype __attribute__((mode(SI)));\n",
        "void f(void) __attribute__((visibility(\"default\"), used));\n",
        "unsigned __int128 u128; __int128 s128;\n",
        "_Decimal32 d32; _Fract fr; _Decimal64 d64; _Float16 f16;\n",
        "void f(void) { return (void)0; }\n",
        "void cl(int *p) {}\n"
        "void f(void) { int __attribute__((cleanup(cl))) x = 1; return; }\n",
        "void cl(int *p) {}\n"
        "void f(void) { int __attribute__((cleanup(cl))) x = 1; return 1; }\n",
        "void cl(int *p) {}\n"
        "void f(void) {\n"
        "  int __attribute__((cleanup(cl))) x = 1;\n"
        "  goto l1;\n"
        "l1:;\n"
        "  longjmp(0, 1);\n"
        "}\n",
        "void f(void) { __label__ l1; goto l1; l1:; }\n",
        "int x = ({ int a = 1; a; });\n",
        "void f(int n) { int a[n]; }\n",
        "int f(int n) { int a[n]; return 1; }\n",
        "struct S { int a[0]; }; struct M { int a[0]; int b; };\n",
        "enum E { E1, }; struct S { int a, };\n",
        "int f(int x) { switch(x) { case 1 ... 5: break; } return 0; }\n",
        "int arr[10] = { [0 ... 4] = 1 };\n",
        "int f(int x) { return x ?: 0; }\n",
        "void f(void) { int v = 0; (char)v = 1; (int){ 2 }; }\n",
        "int f(int x) { return __builtin_expect(x, 1); }\n",
        "int f(void) { return __extension__ 1; }\n",
        "int f(void) { return __builtin_shuffle(1, 2); }\n",
        "int f(void) { return __builtin_shufflevector(1, 2); }\n",
        "int f(void) { return __alignof__(int); }\n",
        "__complex__ float cf;\n",
        "float rf = __real__ cf;\n",
        "float imf = __imag__ cf;\n",
        "int ty_v = 1; typeof(ty_v) ty;\n",
        "typeof(int[5]) tarr;\n",
        "typeof(int[]) tarr2;\n",
        "__auto_type ax = 1;\n",
        "union { int a; };\n",
        "struct { int a; };\n",
        "unsigned __int128 n128 = 0x10000000000000000;\n",
        "void f(void) { (int){ 2 }; }\n",
        "void f(void) { int v = 0; (union U)v = 1; }\n",
        "void f(int n) { int a[n]; goto l1; l1:; }\n",
        "void f(int *ptr) { goto *ptr; }\n",
        "void f(void) { __label__ l1, l2, l3; goto l1; l1:; }\n"};
    size_t j;
    int vla_mode;
    for (vla_mode = 0; vla_mode <= 1; vla_mode++) {
      config.fallback_vla_to_malloc = vla_mode;
      for (j = 0; j < sizeof(test_codes) / sizeof(test_codes[0]); j++) {
        int k;
        for (k = 1; k <= 12; k++) {
          g_gnu_replace_fail = k;
          if (cdd_cst_parse(
                  az_span_create_from_str((char *)(size_t)test_codes[j]),
                  &tree) == 0 &&
              tree) {
            cdd_transform_gnu(tree, &config);
            cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_gnu_replace_fail = 0;

        for (k = 1; k <= 12; k++) {
          g_gnu_bld_fail = k;
          if (cdd_cst_parse(
                  az_span_create_from_str((char *)(size_t)test_codes[j]),
                  &tree) == 0 &&
              tree) {
            cdd_transform_gnu(tree, &config);
            cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_gnu_bld_fail = 0;

        for (k = 1; k <= 12; k++) {
          g_gnu_alloc_fail = k;
          if (cdd_cst_parse(
                  az_span_create_from_str((char *)(size_t)test_codes[j]),
                  &tree) == 0 &&
              tree) {
            cdd_transform_gnu(tree, &config);
            cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_gnu_alloc_fail = 0;

        for (k = 1; k <= 12; k++) {
          g_gnu_malloc_fail = k;
          if (cdd_cst_parse(
                  az_span_create_from_str((char *)(size_t)test_codes[j]),
                  &tree) == 0 &&
              tree) {
            cdd_transform_gnu(tree, &config);
            cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_gnu_malloc_fail = 0;

        for (k = 1; k <= 12; k++) {
          g_gnu_find_fail = k;
          if (cdd_cst_parse(
                  az_span_create_from_str((char *)(size_t)test_codes[j]),
                  &tree) == 0 &&
              tree) {
            cdd_transform_gnu(tree, &config);
            cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_gnu_find_fail = 0;
      }
    }
  }

  for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); i++) {
    int k;
    for (k = 1; k <= 15; k++) {
      g_gnu_bld_fail = k;
      if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippets[i]),
                        &tree) == 0 &&
          tree) {
        cdd_transform_gnu(tree, &config);
        cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    g_gnu_bld_fail = 0;

    for (k = 1; k <= 15; k++) {
      g_gnu_malloc_fail = k;
      if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippets[i]),
                        &tree) == 0 &&
          tree) {
        cdd_transform_gnu(tree, &config);
        cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    g_gnu_malloc_fail = 0;

    for (k = 1; k <= 15; k++) {
      g_gnu_alloc_fail = k;
      if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippets[i]),
                        &tree) == 0 &&
          tree) {
        cdd_transform_gnu(tree, &config);
        cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    g_gnu_alloc_fail = 0;

    for (k = 1; k <= 15; k++) {
      g_gnu_replace_fail = k;
      if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippets[i]),
                        &tree) == 0 &&
          tree) {
        cdd_transform_gnu(tree, &config);
        cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    g_gnu_replace_fail = 0;

    for (k = 1; k <= 15; k++) {
      g_gnu_standardizer_fail = 100 + k;
      if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippets[i]),
                        &tree) == 0 &&
          tree) {
        cdd_transform_gnu(tree, &config);
        cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    g_gnu_standardizer_fail = 0;

    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippets[i]),
                         &tree));
    ASSERT(tree != NULL);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &config));
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  PASS();
}

TEST test_gnu_standardizer_all_grammar_branches(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config;
  size_t i;
  static const char *const codes[] = {
      "__complex__",
      "__real__",
      "__real__ 123",
      "__imag__",
      "__imag__ 123",
      "typeof",
      "typeof x;",
      "typeof(int * const) x;",
      "typeof(",
      "typeof((int))",
      "typeof(int",
      "typeof(int[5) x;",
      "__auto_type",
      "__auto_type x;",
      "__auto_type x = \"str\";",
      "__auto_type x = y;",
      "__auto_type s = struct;",
      "__auto_type x =",
      "017777777777777777777777777777777777777777ULL",
      "int __attribute__;",
      "int __attribute__((packed, x));",
      "int __attribute__ (noreturn) x;",
      "struct __attribute__((packed)) X { int a; }",
      "int __attribute__(((always))) x;",
      "int __attribute__((a, b)) x;",
      "int __attribute__((vector_wide(16))) x;",
      "int __attribute__((pure(16))) x;",
      "int __attribute__((cleanup(8))) x;",
      "__attribute__((",
      "__attribute__ x;",
      "union",
      "struct",
      "union U { int a; };",
      "struct S { int a; };",
      "void f_casts(void) { int v; (struct S)v = 1; (short)v = 1; (char)v = 1; "
      "}",
      "void f_c_eof(void) { (int)x = 1",
      "void f_c_blk(void) { { (int)x = 1; } }",
      "int f_tern(int x) { return x ? 1 : 2; }",
      "int f_ret_c(char c) { return (char)c; }",
      "#define \n",
      "#define\tFOO\t1\n",
      "#define FOO\n",
      "#define FOO(a\n",
      "#define FOO(a, b)\n",
      "#define FOO(x) ...\n",
      "#define FOO(...)\n",
      "#define FOO(args...) foo(##a, ##args, ##args1, ##args_extra, , "
      "\t##args, a, args, args1, args_extra, myargs, _args)\n",
      "#define FOO(a, args...) foo(a, #args)\n",
      "#define FOO(args...) foo(args_more)\n",
      "volatile __int128 v128;\n",
      "int __int128 x;\n",
      "; __int128 x_semi;\n",
      "signed __int128 x_sgn;\n",
      "int f_exp1(void) { return __builtin_expect; }\n",
      "int f_exp2(void) { return __builtin_expect",
      "int f_exp3(int a, int b) { return __builtin_expect(fn(a, b), 1); }\n",
      "int f_cases(int x) { switch(x) { case 1 ... 5: break; case -5 ... -1: "
      "break; case 5 ... 1: break; default: break; } return 0; }\n",
      "int f_cases2(int x) { switch (x) { case 1: ; case 2 ... 5: break; } "
      "return 0; }\n",
      "int f_cases3(int x) { switch (x) { { case 1 ... 5: break; } } return 0; "
      "}\n",
      "int f_c_dot(int x) { return x.y; }\n",
      "int f_c_dot2(int x) { return x..",
      "int a1[10] = { [0 ... 4] = 1 };\n",
      "int a2[10] = { [-5 ... -1] = 1 };\n",
      "int a3[10] = { [5 ... 1] = 1 };\n",
      "int a4[10] = { [0 ... 4] };\n",
      "int a5[10] = { [0 ... 1, 2] = 0 };\n",
      "void f_lbls(void) { __label__ l1, l2; void *p = &&l1; goto l1; l1:; }\n",
      "void f_goto_ptr(int *ptr) { goto *ptr; }\n",
      "void f_lj(void) { longjmp(0, 1); }\n",
      "void f_vla_alloca(int n) { int a[n]; }\n",
      "void f_vla_p(int n, int a[n], int b[n]) {}\n",
      "int f_vla_r(int n) { int a[n]; return 1; }\n",
      "int a_multi[n][m];\n",
      "int a_multi3[n][m][p];\n",
      "int a_multi_num[n][5];\n",
      "int a_empty[];\n",
      "int a_eof[n]",
      "struct S1 { int a[0]; };\n",
      "struct S2 { int a[0]; int b; };\n",
      "int arr_std[1];\n",
      "int arr_add[0 + 1];\n",
      "int arr_init[0] = {0};\n",
      "int arr_eof[0]",
      "enum E { E1, };\n",
      "enum E_eof { E1, ",
      "struct S3 { int a, };\n",
      "struct S1 s1 = {};\n",
      "union U u1 = {};\n",
      "struct dummy {} s_dummy;\n",
      "struct item {} s_item;\n",
      "int value_empty = {};\n",
      "int item_empty = {};\n",
      "struct S s_desig = { .a : {} };\n",
      "int x = __alignof(int);\n",
      "void f_lbl_ref(void) { int *p = &l1; l1 + 1; }\n",
      "void f_lbl_top(void) { l1:; &l1; }\n",
      "int f_par_one(void) { return (1); }\n",
      "void f_ret_par(void) { return (1); }\n",
      "int f_ret_char(void) { return (char)1; }\n",
      "void f_ret_voidptr(void) { return (void * )0; }\n",
      "void f_ret_void_eof(void) { return (void)0",
      "int __attribute__((cleanup(cl))) cl_a, cl_b;\n",
      "int __attribute__((cleanup(cl)));\n",
      "{ void file_level_nested() {} }\n",
      "int (*fp)(void) { return 0; }\n",
      "void f_c_lval_eof(void) { (int }",
      "(int)x = 1;\n",
      "my_label:;\n",
      "{}\n",
      "#define",
      "int __attribute__((mode(SI)) x_m2;\n",
      "int __attribute__((noreturn) x_n1;\n",
      "void f_elvis_kw(int b) {\n"
      "  int item = 1, value = 2, num = 3, id = 4, result = 5;\n"
      "  switch (result ?: b) { case item ?: b: break; }\n"
      "  while (value ?: b) {}\n"
      "  for (num ?: b; ; ) break;\n"
      "  while (id ?: b) {}\n"
      "}\n",
      "void f_lbl_norm(void) { goto norm_lbl; norm_lbl:; }\n",
      "[1];\n",
      "[0 ... 4] = 1;\n",
      "void cl(int *p) {}\n"
      "void f_5_c(void) {\n"
      "  int __attribute__((cleanup(cl))) c1;\n"
      "  int __attribute__((cleanup(cl))) c2;\n"
      "  int __attribute__((cleanup(cl))) c3;\n"
      "  int __attribute__((cleanup(cl))) c4;\n"
      "  int __attribute__((cleanup(cl))) c5;\n"
      "}\n",
      "void f_5_l(void) {\n"
      "  __label__ l1;\n"
      "  __label__ l2;\n"
      "  __label__ l3;\n"
      "  __label__ l4;\n"
      "  __label__ l5;\n"
      "  goto l1; l1:;\n"
      "}\n",
      "void f_5_v(int n) {\n"
      "  int v1[n]; int v2[n]; int v3[n]; int v4[n]; int v5[n];\n"
      "}\n",
      "void f_elvis_cases(int a, int b) {\n"
      "  int x = (a ?: b);\n"
      "  int arr[10];\n"
      "  arr[a ?: b] = 1;\n"
      "  int z = a ?: b;\n"
      "  ; a ?: b;\n"
      "  { a ?: b; }\n"
      "  if (a ?: b) {}\n"
      "  while (a ?: b) {}\n"
      "  for (a ?: b; ; ) break;\n"
      "  do {} while (a ?: b);\n"
      "  switch (a ?: b) { case 1 ?: 0: break; }\n"
      "  int item = a ?: b;\n"
      "  int value = a ?: b;\n"
      "  int num = a ?: b;\n"
      "  int id = a ?: b;\n"
      "  int switch_var = a ?: b;\n"
      "  a ?: b;\n"
      "}\n",
      "int f_elvis_tern(int cond, int a, int b, int c) {\n"
      "  return cond ? a : b ?: c;\n"
      "}\n",
      "void f_33_cleanups(void) {\n"
      "  int __attribute__((cleanup(cl))) c01, c02, c03, c04, c05, c06, c07, "
      "c08, c09, c10,\n"
      "                                  c11, c12, c13, c14, c15, c16, c17, "
      "c18, c19, c20,\n"
      "                                  c21, c22, c23, c24, c25, c26, c27, "
      "c28, c29, c30,\n"
      "                                  c31, c32, c33;\n"
      "}\n",
      "void f_33_labels(void) {\n"
      "  __label__ l01, l02, l03, l04, l05, l06, l07, l08, l09, l10,\n"
      "            l11, l12, l13, l14, l15, l16, l17, l18, l19, l20,\n"
      "            l21, l22, l23, l24, l25, l26, l27, l28, l29, l30,\n"
      "            l31, l32, l33;\n"
      "}\n",
      "void f_5_cleanups(void) {\n"
      "  int __attribute__((cleanup(cl))) c1;\n"
      "  int __attribute__((cleanup(cl))) c2;\n"
      "  int __attribute__((cleanup(cl))) c3;\n"
      "  int __attribute__((cleanup(cl))) c4;\n"
      "  int __attribute__((cleanup(cl))) c5;\n"
      "}\n",
      "void f_5_labels(void) {\n"
      "  __label__ l1;\n"
      "  __label__ l2;\n"
      "  __label__ l3;\n"
      "  __label__ l4;\n"
      "  __label__ l5;\n"
      "  goto l1; l1:;\n"
      "}\n",
      "void f_5_vlas(int n) {\n"
      "  int v1[n]; int v2[n]; int v3[n]; int v4[n]; int v5[n];\n"
      "}\n",
      "a ?: b;\n",
      "? : 0;\n",
      "#define FOO",
      "#define M1(a, args...) fn(a, ##\targs, ##other)\n",
      "__int128 x_at_zero;\n",
      "int __attribute__((mode(SI) x;\n",
      "int __attribute__((noreturn x;\n",
      "void f_c_u1(void) { (union U)v\n",
      "void f_c_u2(void) { (union U)v + 1; }\n",
      "x ?",
      "void f_outer(void) { int __attribute__((cleanup(cl))) x; { int dummy; } "
      "}\n",
      "void f_outer_lbl(void) { __label__ l1; { int dummy; } l1:; }\n",
      "void f_outer_vla(int n) { int a[n]; { int dummy; } }\n",
      "void f_ret_cl_eof(void) { int __attribute__((cleanup(cl))) x; return",
      "void f_goto_eof(void) { goto",
      "void f_lbl_eof(void) { __label__ l1",
      "void f_lbl_full(void) { __label__ l1; l1:; &l1; + &l1; l1",
      "[0]",
      "struct S_zero_eof { int a[0];",
      "{ 1 }",
      "int f_case_def(int x) { switch (x) { case 1: ; case 2 ... 5: break; "
      "default: break; } return 0; }\n",
      "1 ... 5:",
      "case 1 ... -",
      "case 1 ... -x:",
      "case 1 ... x:",
      "[0 ... 1] =",
      "int f_exp_eof(void) { return __builtin_expect(x, 1",
      "void f_exp_range(void) { int item = 1 ... 5; }\n",
      "void f_semi_range(void) { int x = 0; 1 ... 5; }\n",
      "void f_lbl_unmatched(void) { __label__ l1; goto norm_lbl; norm_lbl:; "
      "goto l1; l1:; }\n",
      "int __attribute__((cleanup(cl)));\n"};

  int vla_mode;
  memset(&config, 0, sizeof(config));

  for (vla_mode = 0; vla_mode <= 1; vla_mode++) {
    config.fallback_vla_to_malloc = vla_mode;
    for (i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
      if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)codes[i]),
                        &tree) == 0 &&
          tree) {
        cdd_transform_gnu(tree, &config);
        cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
  }

  PASS();
}

TEST test_gnu_standardizer_final_branches(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config;
  void *m_ptr = NULL;
  memset(&config, 0, sizeof(config));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, gnu_malloc(10, NULL));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, gnu_malloc((size_t)-1, &m_ptr));

  /* 1. L1710 & L1712: is_cast_lvalue == 2 loop without closing ; or = */
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "void f(void) { (union U)v = 1; }\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->tokens[11].kind = CDD_TOKEN_PLUS;
    tree->base_tokens->size = 12;
    cdd_transform_gnu(tree, &config);
    tree->base_tokens->tokens[11].kind = CDD_TOKEN_ASSIGN;
    tree->base_tokens->size = 16;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 2. L1855: return (void)expr loop without ; */
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "void f(void) { return (void)0; }\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->size -= 2;
    cdd_transform_gnu(tree, &config);
    tree->base_tokens->size += 2;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 3. L1936: cleanup loop without ; */
  if (cdd_cst_parse(az_span_create_from_str((
                        char *)(size_t) "void cl(int *p) {} void f(void) { int "
                                        "__attribute__((cleanup(cl))) x; }\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->size -= 2;
    cdd_transform_gnu(tree, &config);
    tree->base_tokens->size += 2;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 4. L2108: return with cleanups loop without ; */
  if (cdd_cst_parse(az_span_create_from_str((
                        char *)(size_t) "void cl(int *p) {} void f(void) { int "
                                        "__attribute__((cleanup(cl))) x = 1; "
                                        "return 1; }\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->size -= 3;
    cdd_transform_gnu(tree, &config);
    tree->base_tokens->size += 3;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 5. L2140: return with cleanups and NULL config */
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "void cl(int *p) {} int f(void) { int "
                                         "__attribute__((cleanup(cl))) x = 1; "
                                         "return 1; }\n"),
                    &tree) == 0 &&
      tree) {
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, NULL));
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 6. L2229: while (i < size) in __label__ reaches end */
  if (cdd_cst_parse(az_span_create_from_str((
                        char *)(size_t) "void f(void) { __label__ l1, l2; }\n"),
                    &tree) == 0 &&
      tree) {
    size_t ti;
    for (ti = 0; ti < tree->base_tokens->size; ti++) {
      if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_SEMICOLON ||
          tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RBRACE)
        tree->base_tokens->tokens[ti].kind = CDD_TOKEN_IDENTIFIER;
    }
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 7. L2541: zero-length array loop reaches end */
  if (cdd_cst_parse(
          az_span_create_from_str((char *)(size_t) "struct S { int a[0]; }\n"),
          &tree) == 0 &&
      tree) {
    g_gnu_standardizer_fail = 20;
    tree->base_tokens->size = 10;
    cdd_transform_gnu(tree, &config);
    g_gnu_standardizer_fail = 0;
    tree->base_tokens->size = 11;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 8. L2884: builtin expect loop reaches end without seeing ')' */
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "int f(int x) { return "
                                         "__builtin_expect(x, 1); }\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->tokens[13].kind = CDD_TOKEN_PLUS;
    cdd_transform_gnu(tree, &config);
    tree->base_tokens->tokens[13].kind = CDD_TOKEN_RPAREN;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "int f(int x) { return "
                                         "__builtin_expect(x, 1); }\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->size = 9;
    cdd_transform_gnu(tree, &config);
    tree->base_tokens->size = 17;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Empty enum and union defs */
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "void f_ed(void) { union dummy {}; "
                                         "enum item {}; }\n"),
                    &tree) == 0 &&
      tree) {
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Trailing comma loop reaches end */
  if (cdd_cst_parse(
          az_span_create_from_str((char *)(size_t) "enum E { E1, };\n"),
          &tree) == 0 &&
      tree) {
    tree->base_tokens->size = 5;
    cdd_transform_gnu(tree, &config);
    tree->base_tokens->size = 7;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Empty struct at EOF */
  if (cdd_cst_parse(az_span_create_from_str((char *)(size_t) "struct S {};\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->size = 3;
    cdd_transform_gnu(tree, &config);
    tree->base_tokens->size = 6;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Elvis with comma operator */
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "int f_c_elv(int a, int b) { "
                                         "return a, a ?: b; }\n"),
                    &tree) == 0 &&
      tree) {
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Range without case or delimiter */
  if (cdd_cst_parse(
          az_span_create_from_str((char *)(size_t) "int val = 1 ... 5;\n"),
          &tree) == 0 &&
      tree) {
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(
          az_span_create_from_str(
              (char *)(size_t) "int f_br(void) { { 1 ... 5; } return 0; }\n"),
          &tree) == 0 &&
      tree) {
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Single & before local label */
  if (cdd_cst_parse(
          az_span_create_from_str(
              (char *)(size_t) "void f_lbl_amp(void) { __label__ l1; void *p = "
                               "&l1; (void)p; l1:; }\n"),
          &tree) == 0 &&
      tree) {
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 9. L1676: cast closed by RBRACE instead of RPAREN */
  if (cdd_cst_parse(
          az_span_create_from_str(
              (char *)(size_t) "void f(void) { int x; (int)x = 1; }\n"),
          &tree) == 0 &&
      tree) {
    size_t ti;
    for (ti = 0; ti < tree->base_tokens->size; ti++) {
      if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RPAREN) {
        tree->base_tokens->tokens[ti].kind = CDD_TOKEN_RBRACE;
        break;
      }
    }
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 10. Elvis RBRACKET and RBRACE before ?: */
  if (cdd_cst_parse(
          az_span_create_from_str(
              (char *)(size_t) "int f(int a, int b) { int arr[10]; int z = "
                               "arr[a] ?: b; return z; }\n"),
          &tree) == 0 &&
      tree) {
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(
          az_span_create_from_str(
              (char *)(size_t) "int f(int a, int b) { int z = ({ int v = a; "
                               "v; }) ?: b; return z; }\n"),
          &tree) == 0 &&
      tree) {
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

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
  RUN_TEST(test_gnu_standardizer_reach_100_percent_coverage);
  RUN_TEST(test_gnu_standardizer_all_grammar_branches);
  RUN_TEST(test_gnu_standardizer_final_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_GNU_STANDARDIZER_INTERNALS_H */
