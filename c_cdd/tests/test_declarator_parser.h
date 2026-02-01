#ifndef TEST_DECLARATOR_PARSER_H
#define TEST_DECLARATOR_PARSER_H

#include <assert.h>
#include <greatest.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declarator_parser.h"
#include "tokenizer.h"

static struct TokenList *setup_tokens(const char *code) {
  struct TokenList *tl = NULL;
  (void)tokenize(az_span_create_from_str((char *)code), &tl);
  return tl;
}

/**
 * @brief Helper to verify a Type Chain against expected Kinds.
 */
static enum greatest_test_res verify_chain(struct DeclType *head, int n, ...) {
  va_list args;
  int i;
  struct DeclType *curr = head;

  va_start(args, n);
  for (i = 0; i < n; i++) {
    int expected_kind = va_arg(args, int);
    ASSERT_EQ_FMT(expected_kind, curr ? curr->kind : -1, "%d");
    curr = curr->inner;
  }
  va_end(args);
  ASSERT_EQ(NULL, curr);
  PASS();
}

/* --- Concrete Declarator Tests (Named) --- */

TEST test_parse_basic_int(void) {
  const char *code = "int x";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc;

  ASSERT(tl);
  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("x", info.identifier);
  CHECK_CALL(verify_chain(info.type, 1, DECL_BASE));
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
  CHECK_CALL(verify_chain(info.type, 2, DECL_PTR, DECL_BASE));
  ASSERT_STR_EQ("char", info.type->inner->data.base.name);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_pointer_qualifiers(void) {
  /* int * const volatile p */
  const char *code = "int * const volatile p";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("p", info.identifier);
  CHECK_CALL(verify_chain(info.type, 2, DECL_PTR, DECL_BASE));

  /* Verify qualifiers are captured */
  ASSERT(info.type->data.ptr.qualifiers != NULL);
  /* The order depends on scan direction (leftward).
     x -> volatile -> const -> *.
     Join range [volatile_start, const_end].
     Should contain "const volatile" or "volatile const" depending on original
     string order? Code is "const volatile". Range is [const, volatile].
  */
  ASSERT(strstr(info.type->data.ptr.qualifiers, "const") != NULL);
  ASSERT(strstr(info.type->data.ptr.qualifiers, "volatile") != NULL);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_atomic_specifier(void) {
  /* _Atomic(int) ax */
  const char *code = "_Atomic(int) ax";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("ax", info.identifier);
  /* Should parse `_Atomic(int)` as the base type part */
  CHECK_CALL(verify_chain(info.type, 1, DECL_BASE));
  ASSERT_STR_EQ("_Atomic(int)", info.type->data.base.name);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_complex_specifier(void) {
  /* double _Complex c */
  const char *code = "double _Complex c";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("c", info.identifier);
  CHECK_CALL(verify_chain(info.type, 1, DECL_BASE));
  ASSERT(strstr(info.type->data.base.name, "double") != NULL);
  ASSERT(strstr(info.type->data.base.name, "_Complex") != NULL);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_atomic_qualifier_on_ptr(void) {
  /* int * _Atomic ap */
  const char *code = "int * _Atomic ap";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("ap", info.identifier);
  CHECK_CALL(verify_chain(info.type, 2, DECL_PTR, DECL_BASE));

  ASSERT(info.type->data.ptr.qualifiers != NULL);
  ASSERT(strstr(info.type->data.ptr.qualifiers, "_Atomic") != NULL);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_atomic_qualifier_on_base(void) {
  /* _Atomic int x */
  const char *code = "_Atomic int x";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("x", info.identifier);
  CHECK_CALL(verify_chain(info.type, 1, DECL_BASE));
  /* Since it's not on a pointer, it remains in base string */
  ASSERT(strstr(info.type->data.base.name, "_Atomic") != NULL);
  ASSERT(strstr(info.type->data.base.name, "int") != NULL);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

/* --- Abstract Declarator Tests --- */

TEST test_abstract_atomic_ptr(void) {
  /* _Atomic(int) * */
  const char *code = "_Atomic(int) *";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(NULL, info.identifier);
  CHECK_CALL(verify_chain(info.type, 2, DECL_PTR, DECL_BASE));
  ASSERT_STR_EQ("_Atomic(int)", info.type->inner->data.base.name);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

SUITE(declarator_parser_suite) {
  RUN_TEST(test_parse_basic_int);
  RUN_TEST(test_parse_ptr);
  RUN_TEST(test_parse_pointer_qualifiers);
  RUN_TEST(test_parse_atomic_specifier);
  RUN_TEST(test_parse_complex_specifier);
  RUN_TEST(test_parse_atomic_qualifier_on_ptr);
  RUN_TEST(test_parse_atomic_qualifier_on_base);
  RUN_TEST(test_abstract_atomic_ptr);
}

#endif /* TEST_DECLARATOR_PARSER_H */
