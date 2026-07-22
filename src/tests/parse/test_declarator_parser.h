#ifndef TEST_DECLARATOR_PARSER_H
#define TEST_DECLARATOR_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif
extern int g_io_calls;
extern C_CDD_EXPORT int g_fail_io_after;
extern int g_cdd_strdup_fail;
#ifdef __cplusplus
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <assert.h>
#include <greatest.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/declarator.h"
#include "functions/parse/tokenizer.h"

extern enum cdd_c_error add_type_node(struct DeclInfo *info, struct DeclType **current_tail, struct DeclType *node);


extern enum cdd_c_error is_grouping_paren(const struct TokenList *tokens, size_t paren_idx, size_t limit, int *out_is_grouping);
/* clang-format on */

/**
 * @brief Executes the setup tokens operation.
 */
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
    ASSERT_EQ_FMT((int)expected_kind, curr ? (int)curr->kind : -1, "%d");
    curr = curr->inner;
  }
  va_end(args);
  ASSERT_EQ(NULL, curr);
  g_fail_io_after = -1;
  PASS();
}

/* --- Concrete Declarator Tests (Named) --- */

/**
 * @brief Executes the corresponding declarator parser test.
 */
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
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the corresponding declarator parser test.
 */
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
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the corresponding declarator parser test.
 */
TEST test_parse_pointer_qualifiers(void) { /* int * const volatile p */
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
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the corresponding declarator parser test.
 */
TEST test_parse_atomic_specifier(void) { /* _Atomic(int) ax */
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
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the corresponding declarator parser test.
 */
TEST test_parse_complex_specifier(void) { /* double _Complex c */
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
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the corresponding declarator parser test.
 */
TEST test_parse_atomic_qualifier_on_ptr(void) { /* int * _Atomic ap */
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
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the corresponding declarator parser test.
 */
TEST test_parse_atomic_qualifier_on_base(void) { /* _Atomic int x */
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
  g_fail_io_after = -1;
  PASS();
}

/* --- Abstract Declarator Tests --- */

/**
 * @brief Executes the corresponding declarator parser test.
 */
TEST test_abstract_atomic_ptr(void) { /* _Atomic(int) * */
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_parse_func_ptr(void) {
  const char *code = "int (*func)(void)";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("func", info.identifier);
  CHECK_CALL(verify_chain(info.type, 3, DECL_PTR, DECL_FUNC, DECL_BASE));

  decl_info_free(&info);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_parse_func_array(void) {
  const char *code = "int (*a[5])(void)";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("a", info.identifier);
  CHECK_CALL(
      verify_chain(info.type, 4, DECL_ARRAY, DECL_PTR, DECL_FUNC, DECL_BASE));

  decl_info_free(&info);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_abstract_func_ptr(void) {
  const char *code = "int (*)(void)";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(NULL, info.identifier);
  CHECK_CALL(verify_chain(info.type, 3, DECL_PTR, DECL_FUNC, DECL_BASE));

  decl_info_free(&info);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_abstract_array(void) {
  const char *code = "int[5]";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(NULL, info.identifier);
  CHECK_CALL(verify_chain(info.type, 2, DECL_ARRAY, DECL_BASE));

  decl_info_free(&info);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_parse_decl_errors(void) {
  struct DeclInfo info;
  struct TokenList *tl = setup_tokens("int x");

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_declaration(NULL, 0, 0, &info));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_declaration(tl, 0, tl->size, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, decl_info_init(NULL));

  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_add_type_node_nulls(void) {
  struct DeclInfo info;
  struct DeclType *tail = NULL;
  (void)decl_info_init(&info);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_type_node(&info, &tail, NULL));
  PASS();
}

TEST test_parse_declarator_oom(void) {
  const char *code = "int * const volatile p[10](void)";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int i, rc;

  for (i = 0; i < 20; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    g_cdd_strdup_fail = i;
    rc = parse_declaration(tl, 0, tl->size, &info);
    g_fail_io_after = -1;
    g_cdd_strdup_fail = -1;

    if (rc == 0) {
      decl_info_free(&info);
      break; /* Reached success */
    } else {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
  }

  free_token_list(tl);
  PASS();
}

TEST test_parse_declarator_more_edge_cases(void) {
  struct TokenList *tl;
  struct DeclInfo info;
  int rc;
  int is_group;

  tl = setup_tokens("enum { A, B } x");
  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("struct MyStruct { int a; } x");
  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("(*)");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            is_grouping_paren(tl, 0, tl->size, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, tl->size, &is_group));
  ASSERT_EQ(1, is_group);
  free_token_list(tl);

  tl = setup_tokens("enum { A, B } *");
  rc = parse_declaration(tl, 0, tl->size, &info);
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("enum Name { A, B } *");
  rc = parse_declaration(tl, 0, tl->size, &info);
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("struct { int a; } *");
  rc = parse_declaration(tl, 0, tl->size, &info);
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("_Atomic(int) *");
  rc = parse_declaration(tl, 0, tl->size, &info);
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("(int)");
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, tl->size, &is_group));
  ASSERT_EQ(0, is_group);
  free_token_list(tl);

  tl = setup_tokens("()");
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, tl->size, &is_group));
  ASSERT_EQ(0, is_group);
  free_token_list(tl);

  tl = setup_tokens("((int))");
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, tl->size, &is_group));
  ASSERT_EQ(1, is_group);
  free_token_list(tl);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_grouping_paren(tl, 0, 0, NULL));
  decl_info_free(NULL);

  tl = setup_tokens("int (*)(int)");
  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("int (*( *x() )())()");
  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);
  decl_info_free(&info);
  free_token_list(tl);

  PASS();
}

TEST test_parse_declarator_empty_array(void) {
  const char *code = "int x[]";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_declarator_just_x(void) {
  const char *code = "x";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);
  ASSERT(info.type != NULL);
  ASSERT_STR_EQ("int", info.type->data.base.name);
  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_declarator_edge_cases(void) {
  struct TokenList *tl;
  struct DeclInfo info;
  int rc;

  /* No explicit base type (implicit int) */
  tl = setup_tokens("*p");
  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);
  decl_info_free(&info);
  free_token_list(tl);

  /* Abstract declarator pointer */
  tl = setup_tokens("*");
  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(NULL, info.identifier);
  decl_info_free(&info);
  free_token_list(tl);

  /* Empty tokens -> Parses as implicit abstract int */
  tl = setup_tokens("");
  rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(info.type != NULL);
  ASSERT(info.type->kind == DECL_BASE);
  decl_info_free(&info);
  free_token_list(tl);

  PASS();
}

SUITE(declarator_parser_suite) {
#if defined(_MSC_VER) && _MSC_VER <= 1400
  return;
#endif

  RUN_TEST(test_parse_basic_int);
  RUN_TEST(test_parse_ptr);
  RUN_TEST(test_parse_pointer_qualifiers);
  RUN_TEST(test_parse_atomic_specifier);
  RUN_TEST(test_parse_complex_specifier);
  RUN_TEST(test_parse_atomic_qualifier_on_ptr);
  RUN_TEST(test_parse_atomic_qualifier_on_base);
  RUN_TEST(test_abstract_atomic_ptr);
  RUN_TEST(test_parse_func_ptr);
  RUN_TEST(test_parse_func_array);
  RUN_TEST(test_abstract_func_ptr);
  RUN_TEST(test_abstract_array);
  RUN_TEST(test_parse_decl_errors);
  RUN_TEST(test_add_type_node_nulls);
  RUN_TEST(test_parse_declarator_oom);
  RUN_TEST(test_parse_declarator_edge_cases);
  RUN_TEST(test_parse_declarator_more_edge_cases);
  RUN_TEST(test_parse_declarator_empty_array);
  RUN_TEST(test_parse_declarator_just_x);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DECLARATOR_PARSER_H */
