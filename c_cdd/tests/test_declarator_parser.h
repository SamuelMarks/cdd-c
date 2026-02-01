#ifndef TEST_DECLARATOR_PARSER_H
#define TEST_DECLARATOR_PARSER_H

#include <greatest.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declarator_parser.h"
#include "tokenizer.h"

#include <assert.h>

static struct TokenList *setup_tokens(const char *code) {
  struct TokenList *tl = NULL;
  (void)tokenize(az_span_create_from_str((char *)code), &tl);
  return tl;
}

/**
 * @brief Helper to verify a Type Chain against expected Kinds.
 *
 * @param head The head of the parsed type chain.
 * @param n Number of expected nodes.
 * @param ... Variadic list of `enum DeclTypeKind`.
 */
static void verify_chain(struct DeclType *head, int n, ...) {
  va_list args;
  int i;
  struct DeclType *curr = head;

  va_start(args, n);
  for (i = 0; i < n; i++) {
    int expected_kind = va_arg(args, int);
    assert(curr != NULL);
    assert(expected_kind == curr->kind);
    curr = curr->inner;
  }
  va_end(args);
  assert(curr == NULL); /* Should end exactly here */
}

TEST test_parse_basic_int(void) {
  const char *code = "int x";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc;

  ASSERT(tl);
  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("x", info.identifier);
  /* Type: BASE(int) */
  verify_chain(info.type, 1, DECL_BASE);
  ASSERT_STR_EQ("int", info.type->data.base.name);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_ptr(void) {
  const char *code = "char *p";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc;

  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("p", info.identifier);
  /* Chain: PTR -> BASE(char) */
  verify_chain(info.type, 2, DECL_PTR, DECL_BASE);
  ASSERT_STR_EQ("char", info.type->inner->data.base.name);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_array(void) {
  const char *code = "int arr[10]";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("arr", info.identifier);
  /* Chain: ARRAY -> BASE(int) */
  verify_chain(info.type, 2, DECL_ARRAY, DECL_BASE);
  ASSERT_STR_EQ("10", info.type->data.array.size_expr);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_ptr_to_array(void) {
  /* int (*pa)[5] */
  /* Pivot: pa */
  /* Right: nothing (hit paren) */
  /* Left: * -> PTR */
  /* Unnest parens. */
  /* Right: [5] -> ARRAY */
  /* Left: int -> BASE */
  const char *code = "int (*pa)[5]";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("pa", info.identifier);
  /* Chain: PTR -> ARRAY -> BASE */
  verify_chain(info.type, 3, DECL_PTR, DECL_ARRAY, DECL_BASE);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_array_of_ptrs(void) {
  /* int *ap[5] */
  /* Pivot: ap */
  /* Right: [5] -> ARRAY */
  /* Left: * -> PTR */
  /* Left: int -> BASE */
  const char *code = "int *ap[5]";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("ap", info.identifier);
  /* Chain: ARRAY -> PTR -> BASE */
  verify_chain(info.type, 3, DECL_ARRAY, DECL_PTR, DECL_BASE);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_func_ptr(void) {
  /* void (*fp)(int) */
  /* Pivot: fp */
  /* Inner: * -> PTR */
  /* Unnest */
  /* Right: (int) -> FUNC */
  /* Left: void -> BASE */
  const char *code = "void (*fp)(int)";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("fp", info.identifier);
  /* Chain: PTR -> FUNC -> BASE */
  verify_chain(info.type, 3, DECL_PTR, DECL_FUNC, DECL_BASE);
  ASSERT_STR_EQ("void", info.type->inner->inner->data.base.name);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_typeof(void) {
  /* typeof(X) y */
  const char *code = "typeof(X) y";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("y", info.identifier);
  verify_chain(info.type, 1, DECL_BASE);
  ASSERT_STR_EQ("typeof(X)", info.type->data.base.name);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_complex_spiral(void) {
  /* void (*(*f[])(void))(int) */
  /* f is:
     Array []
     of Pointers *
     to Function (void)
     returning Pointers *
     to Function (int)
     returning void.
  */
  const char *code = "void (*(*f[])(void))(int)";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("f", info.identifier);

  /* Verify Chain Sequence */
  verify_chain(info.type, 6, DECL_ARRAY, DECL_PTR, DECL_FUNC, DECL_PTR,
               DECL_FUNC, DECL_BASE);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

SUITE(declarator_parser_suite) {
  RUN_TEST(test_parse_basic_int);
  RUN_TEST(test_parse_ptr);
  RUN_TEST(test_parse_array);
  RUN_TEST(test_parse_ptr_to_array);
  RUN_TEST(test_parse_array_of_ptrs);
  RUN_TEST(test_parse_func_ptr);
  RUN_TEST(test_parse_typeof);
  RUN_TEST(test_parse_complex_spiral);
}

#endif /* TEST_DECLARATOR_PARSER_H */
