#ifndef TEST_DECLARATOR_PARSER_H
#define TEST_DECLARATOR_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif
/* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
/* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
#include <c_cdd_export.h>
/* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
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

extern cdd_c_error_t add_type_node(struct DeclInfo *info, struct DeclType **current_tail, struct DeclType *node);


extern cdd_c_error_t is_grouping_paren(const struct TokenList *tokens, size_t paren_idx, size_t limit, int *out_is_grouping);
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws_back;
extern C_CDD_EXPORT int g_cdd_fail_skip_group;
extern C_CDD_EXPORT int g_cdd_fail_add_type_node;
extern C_CDD_EXPORT int g_cdd_fail_create_node;
extern C_CDD_EXPORT int g_cdd_fail_find_pivot;
extern C_CDD_EXPORT int g_cdd_fail_is_grouping_paren;

/**
 * @brief Resets mock failure counters for declarator tests.
 */
static void reset_decl_mocks(void) {
  g_cdd_fail_skip_ws = 0;
  g_cdd_fail_skip_ws_back = 0;
  g_cdd_fail_skip_group = 0;
  g_cdd_fail_add_type_node = 0;
  g_cdd_fail_create_node = 0;
  g_cdd_fail_find_pivot = 0;
  g_cdd_fail_is_grouping_paren = 0;
  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 0;
  g_fail_io_after = -1;
}

/**
 * @brief Executes the setup tokens operation.
 */
static struct TokenList *setup_tokens(const char *code) {
  struct TokenList *tl = NULL;
  reset_decl_mocks();
  (void)tokenize(az_span_create_from_str((char *)(size_t)(size_t)code), &tl);
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
  const char *code = (char *)(size_t)(size_t) "int x";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc;

  (void)rc;
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
  const char *code = (char *)(size_t)(size_t) "char *p";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc;

  rc = parse_declaration(tl, 0, tl->size, &info);
  (void)rc;
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
  const char *code = (char *)(size_t)(size_t) "int * const volatile restrict p";
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
  const char *code = (char *)(size_t)(size_t) "_Atomic(int) ax";
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
  const char *code = (char *)(size_t)(size_t) "double _Complex c";
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
  const char *code = (char *)(size_t)(size_t) "int * _Atomic ap";
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
  const char *code = (char *)(size_t)(size_t) "_Atomic int x";
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
  const char *code = (char *)(size_t)(size_t) "_Atomic(int) *";
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
  const char *code = (char *)(size_t)(size_t) "int (*func)(void)";
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
  const char *code = (char *)(size_t)(size_t) "int (*a[5])(void)";
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
  const char *code = (char *)(size_t)(size_t) "int (*)(void)";
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
  const char *code = (char *)(size_t)(size_t) "int[5]";
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
  const char *code =
      (char *)(size_t)(size_t) "int * const volatile p[10](void)";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int i, rc;

  for (i = 1; i <= 10; ++i) {
    g_cdd_alloc_fail = i;
    rc = parse_declaration(tl, 0, tl->size, &info);
    reset_decl_mocks();

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
  (void)rc;
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
  const char *code = (char *)(size_t)(size_t) "int x[]";
  struct TokenList *tl = setup_tokens(code);
  struct DeclInfo info;
  int rc = parse_declaration(tl, 0, tl->size, &info);
  ASSERT_EQ(0, rc);

  decl_info_free(&info);
  free_token_list(tl);
  PASS();
}

TEST test_parse_declarator_just_x(void) {
  const char *code = (char *)(size_t)(size_t) "x";
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
  (void)rc;
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

TEST test_parse_declarator_uncovered(void) {
  struct TokenList *tl = setup_tokens("(");
  int is_grouping = 0;
  struct DeclInfo info;

  /* is_grouping_paren: i >= limit */
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, tl->size, &is_grouping));
  ASSERT_EQ(0, is_grouping);
  free_token_list(tl);

  /* skip_group: limit reached */
  tl = setup_tokens("(");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);

  /* typeof with unmatched paren */
  tl = setup_tokens("typeof");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);
  tl = setup_tokens("struct");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);
  tl = setup_tokens("typeof((x))");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);
  tl = setup_tokens("struct { struct { int a; } b; }");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);
  tl = setup_tokens("typeof( x");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);

  /* struct body skip edge case */
  tl = setup_tokens("struct { x");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);

  /* skip_ws_back hit limit and is space */
  tl = setup_tokens(" x");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);

  /* pivot best_pivot logic edge case */
  tl = setup_tokens("void ((x))");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("void ((x)())");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  decl_info_free(&info);
  free_token_list(tl);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, decl_info_init(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_declaration(NULL, 0, 0, &info));

  tl = setup_tokens("");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_declaration(tl, 0, 0, NULL));
  free_token_list(tl);

  PASS();
}

TEST test_parse_declarator_uncovered_2(void) {
  struct DeclInfo info;
  struct TokenList *tl;
  int is_grouping = 0;

  decl_info_init(&info);
  tl = setup_tokens("/* a */ /* b */");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, 2, &info));
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("(^)");
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, tl->size, &is_grouping));
  ASSERT_EQ(1, is_grouping);
  free_token_list(tl);

  tl = setup_tokens("([])");
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, tl->size, &is_grouping));
  ASSERT_EQ(1, is_grouping);
  free_token_list(tl);

  tl = setup_tokens("()");
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, tl->size, &is_grouping));
  ASSERT_EQ(0, is_grouping);
  free_token_list(tl);

  tl = setup_tokens("union U");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, 2, &info));
  decl_info_free(&info);
  free_token_list(tl);

  tl = setup_tokens("enum E");
  ASSERT_EQ(CDD_C_SUCCESS, parse_declaration(tl, 0, 2, &info));
  decl_info_free(&info);
  free_token_list(tl);

  PASS();
}

/**
 * @brief Tests unit helper functions for declarator parser.
 */
TEST test_declarator_unit_helpers(void) {
  struct TokenList *tl;
  char *str = NULL;
  size_t val = 0;
  struct DeclType *node = NULL;
  struct DeclInfo info;
  struct DeclType *tail = NULL;
  int is_group = 0;
  int is_abstract = 0;

  tl = setup_tokens("int x;");
  ASSERT(tl != NULL);

  /* join_tokens_range */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_join_tokens_range(NULL, 0, 1, &str));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_join_tokens_range(tl, 0, 1, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_join_tokens_range(tl, 2, 1, &str));
  ASSERT(str != NULL);
  C_CDD_FREE(str);
  str = NULL;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_join_tokens_range(tl, 2, 1, &str));
  reset_decl_mocks();

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_join_tokens_range(tl, 0, 2, &str));
  ASSERT(str != NULL);
  C_CDD_FREE(str);
  str = NULL;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_join_tokens_range(tl, 0, 2, &str));
  reset_decl_mocks();

  /* skip_ws */
  g_cdd_fail_skip_ws = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_skip_ws(tl, 0, 1, &val));
  reset_decl_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_skip_ws(NULL, 0, 1, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_skip_ws(tl, 0, 1, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws(tl, 0, tl->size, &val));

  /* skip_ws_back */
  g_cdd_fail_skip_ws_back = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_skip_ws_back(tl, 1, 0, &val));
  reset_decl_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_skip_ws_back(NULL, 1, 0, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_skip_ws_back(tl, 1, 0, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws_back(tl, 0, 1, &val));
  ASSERT_EQ(SIZE_MAX, val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws_back(tl, 1, 0, &val));

  /* skip_group */
  g_cdd_fail_skip_group = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_skip_group(tl, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, &val));
  reset_decl_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_skip_group(NULL, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_skip_group(tl, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, NULL));

  /* create_node */
  g_cdd_fail_create_node = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, &node));
  reset_decl_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_create_node(DECL_BASE, NULL));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, &node));
  reset_decl_mocks();
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  ASSERT(node != NULL);
  cdd_test_free_decl_type(node);
  node = NULL;

  /* add_type_node */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  (void)decl_info_init(&info);
  g_cdd_fail_add_type_node = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, add_type_node(&info, &tail, node));
  reset_decl_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_type_node(NULL, &tail, node));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_type_node(&info, NULL, node));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_type_node(&info, &tail, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, add_type_node(&info, &tail, node));
  node = NULL;
  decl_info_free(&info);

  /* is_grouping_paren */
  g_cdd_fail_is_grouping_paren = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, is_grouping_paren(tl, 0, 1, &is_group));
  reset_decl_mocks();
  g_cdd_fail_skip_ws = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, is_grouping_paren(tl, 0, 1, &is_group));
  reset_decl_mocks();

  /* find_pivot */
  g_cdd_fail_find_pivot = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_find_pivot(tl, 0, 1, &is_abstract, &val));
  reset_decl_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_pivot(NULL, 0, 1, &is_abstract, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_pivot(tl, 0, 1, NULL, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_pivot(tl, 0, 1, &is_abstract, NULL));

  /* find_abstract_pivot */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_abstract_pivot(NULL, 0, 1, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_abstract_pivot(tl, 0, 1, NULL));

  /* cdd_test_free_decl_type branches */
  cdd_test_free_decl_type(NULL);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_PTR, &node));
  node->data.ptr.qualifiers = (char *)C_CDD_MALLOC(5);
  memcpy(node->data.ptr.qualifiers, "test", 5);
  cdd_test_free_decl_type(node);
  node = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_ARRAY, &node));
  node->data.array.size_expr = (char *)C_CDD_MALLOC(5);
  memcpy(node->data.array.size_expr, "test", 5);
  cdd_test_free_decl_type(node);
  node = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_FUNC, &node));
  node->data.func.args_str = (char *)C_CDD_MALLOC(5);
  memcpy(node->data.func.args_str, "test", 5);
  cdd_test_free_decl_type(node);
  node = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  node->data.base.name = (char *)C_CDD_MALLOC(5);
  memcpy(node->data.base.name, "test", 5);
  cdd_test_free_decl_type(node);
  node = NULL;

  free_token_list(tl);
  PASS();
}

/**
 * @brief Tests declarator parser mock failures and edge cases.
 */
TEST test_declarator_mock_failures(void) {
  struct TokenList *tl;
  struct DeclInfo info;

  /* find_pivot fail */
  tl = setup_tokens("int x");
  g_cdd_fail_find_pivot = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  /* join_tokens_range fail for identifier */
  g_cdd_alloc_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  /* skip_ws_back fail on left */
  g_cdd_fail_skip_ws_back = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  /* skip_ws fail on right */
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();
  free_token_list(tl);

  /* abstract declarator skip_ws_back on left */
  tl = setup_tokens("int *");
  g_cdd_fail_skip_ws_back = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  /* abstract declarator skip_ws on right */
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();
  free_token_list(tl);

  /* array failures */
  tl = setup_tokens("int a[10]");
  g_cdd_fail_create_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_alloc_fail = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_add_type_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();
  free_token_list(tl);

  /* function failures */
  tl = setup_tokens("int f(void)");
  g_cdd_fail_create_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_alloc_fail = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_add_type_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();
  free_token_list(tl);

  /* pointer and qualifier failures */
  tl = setup_tokens("int * const p");
  g_cdd_fail_create_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_alloc_fail = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_add_type_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_skip_ws_back = 2;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_skip_ws_back = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();
  free_token_list(tl);

  /* grouping paren unnesting */
  tl = setup_tokens("int (*p)");
  g_cdd_fail_skip_ws_back = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();
  free_token_list(tl);

  /* base type failures */
  tl = setup_tokens("int x");
  g_cdd_fail_create_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_alloc_fail = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();

  g_cdd_fail_add_type_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();
  free_token_list(tl);

  tl = setup_tokens("x");
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_mocks();
  free_token_list(tl);

  PASS();
}

/**
 * @brief Tests abstract pivot error branches.
 */
TEST test_declarator_abstract_pivot_branches(void) {
  struct TokenList *tl;
  size_t val = 0;

  tl = setup_tokens("struct S { int a; } *");
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_mocks();
  free_token_list(tl);

  tl = setup_tokens("typeof(int) *");
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_mocks();
  free_token_list(tl);

  tl = setup_tokens("int (*)(int)");
  g_cdd_fail_is_grouping_paren = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_mocks();
  free_token_list(tl);

  tl = setup_tokens("struct S *");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  free_token_list(tl);

  tl = setup_tokens("typeof *");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  free_token_list(tl);

  tl = setup_tokens("struct");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  free_token_list(tl);

  tl = setup_tokens("typeof");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  free_token_list(tl);

  tl = setup_tokens("int [5]");
  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_mocks();
  free_token_list(tl);

  PASS();
}

/**
 * @brief Tests find_pivot error branches.
 */
TEST test_declarator_find_pivot_branches(void) {
  struct TokenList *tl;
  int is_abstract = 0;
  size_t val = 0;

  tl = setup_tokens("struct S { int a; } x;");
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_mocks();
  free_token_list(tl);

  tl = setup_tokens("typeof(int) x;");
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_mocks();
  free_token_list(tl);

  tl = setup_tokens("struct S x;");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  free_token_list(tl);

  tl = setup_tokens("typeof x;");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  free_token_list(tl);

  tl = setup_tokens("struct");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  free_token_list(tl);

  tl = setup_tokens("typeof");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  free_token_list(tl);

  PASS();
}

/**
 * @brief Tests mock cycling for 100% branch coverage of mock conditions.
 */
TEST test_declarator_mock_cycles(void) {
  struct TokenList *tl;
  size_t val = 0;
  int is_abstract = 0;
  int is_group = 0;
  struct DeclType *node = NULL;
  struct DeclType *tail = NULL;
  struct DeclInfo info;
  char *str = NULL;

  tl = setup_tokens("int x;");

  /* skip_ws cycle */
  g_cdd_fail_skip_ws = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws(tl, 0, 1, &val));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_skip_ws(tl, 0, 1, &val));
  reset_decl_mocks();

  /* skip_ws_back cycle */
  g_cdd_fail_skip_ws_back = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws_back(tl, 1, 0, &val));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_skip_ws_back(tl, 1, 0, &val));
  reset_decl_mocks();

  /* skip_group cycle */
  g_cdd_fail_skip_group = 2;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_skip_group(tl, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, &val));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_skip_group(tl, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, &val));
  reset_decl_mocks();

  /* add_type_node cycle */
  (void)decl_info_init(&info);
  (void)cdd_test_create_node(DECL_BASE, &node);
  g_cdd_fail_add_type_node = 2;
  ASSERT_EQ(CDD_C_SUCCESS, add_type_node(&info, &tail, node));
  node = NULL;
  (void)cdd_test_create_node(DECL_BASE, &node);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, add_type_node(&info, &tail, node));
  cdd_test_free_decl_type(node);
  node = NULL;
  decl_info_free(&info);
  reset_decl_mocks();

  /* create_node cycle */
  g_cdd_fail_create_node = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  cdd_test_free_decl_type(node);
  node = NULL;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, &node));
  reset_decl_mocks();

  /* create_node alloc fail cycle */
  g_cdd_fail_create_node = 0;
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  cdd_test_free_decl_type(node);
  node = NULL;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, &node));
  reset_decl_mocks();

  /* create_node with NULL out_val under mock */
  g_cdd_fail_create_node = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, NULL));
  reset_decl_mocks();

  /* is_grouping_paren cycle */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            is_grouping_paren(NULL, 0, 1, &is_group));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_grouping_paren(tl, 0, 1, NULL));
  g_cdd_fail_is_grouping_paren = 2;
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, 1, &is_group));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, is_grouping_paren(tl, 0, 1, &is_group));
  reset_decl_mocks();

  /* find_pivot cycle */
  g_cdd_fail_find_pivot = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_pivot(tl, 0, 1, &is_abstract, &val));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_find_pivot(tl, 0, 1, &is_abstract, &val));
  reset_decl_mocks();

  /* C_CDD_MALLOC cycle for start >= end */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_join_tokens_range(tl, 2, 1, &str));
  C_CDD_FREE(str);
  str = NULL;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_join_tokens_range(tl, 2, 1, &str));
  reset_decl_mocks();

  /* C_CDD_MALLOC cycle for start < end */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_join_tokens_range(tl, 0, 2, &str));
  C_CDD_FREE(str);
  str = NULL;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_join_tokens_range(tl, 0, 2, &str));
  reset_decl_mocks();

  /* default switch branch in free_decl_type */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node((enum DeclTypeKind)99, &node));
  cdd_test_free_decl_type(node);
  node = NULL;

  free_token_list(tl);
  PASS();
}

SUITE(declarator_parser_suite) {
#if defined(_MSC_VER) && _MSC_VER <= 1400
  /* skipped on old msvc */
#else

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
  RUN_TEST(test_parse_declarator_uncovered);
  RUN_TEST(test_parse_declarator_uncovered_2);
  RUN_TEST(test_declarator_unit_helpers);
  RUN_TEST(test_declarator_mock_failures);
  RUN_TEST(test_declarator_abstract_pivot_branches);
  RUN_TEST(test_declarator_find_pivot_branches);
  RUN_TEST(test_declarator_mock_cycles);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DECLARATOR_PARSER_H */
