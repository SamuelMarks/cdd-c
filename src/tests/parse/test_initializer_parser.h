#ifndef TEST_INITIALIZER_PARSER_H
#define TEST_INITIALIZER_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "classes/parse/initializer.h"
#include "functions/parse/tokenizer.h"
#include "c_cdd/test_allocator.h"
/* clang-format on */

static enum cdd_c_error tokenize_str(const char *s,
                                     struct TokenList **_out_val) {
  struct TokenList *tl = NULL;
  (void)tokenize(az_span_create_from_str((char *)s), &tl);
  {
    *_out_val = tl;
    return 0;
  }
}

TEST test_init_oom(void) {
  struct TokenList *_ast_tokenize_str_0;
  const char *code = "{ .pt = { .x = 1, .y = 2 }, .flag = 1, 5, { 6 }, 7, 8, "
                     "9, 10, .foo /* c */ = 2 }";
  struct TokenList *tl =
      (tokenize_str(code, &_ast_tokenize_str_0), _ast_tokenize_str_0);
  int i;
  ASSERT(tl);

  /* Cover missing parser branches */
  {
    struct TokenList *_ast_tokenize_str_1;
    struct InitList err_list;
    init_list_init(&err_list);
    struct TokenList *tl_err1 =
        (tokenize_str("{ .x ; = 1 }", &_ast_tokenize_str_1),
         _ast_tokenize_str_1);
    parse_initializer(tl_err1, 0, tl_err1->size, &err_list, NULL);
    free_token_list(tl_err1);

    struct TokenList *_ast_tokenize_str_2;
    struct TokenList *tl_err2 =
        (tokenize_str("{ .x 1 }", &_ast_tokenize_str_2), _ast_tokenize_str_2);
    parse_initializer(tl_err2, 0, tl_err2->size, &err_list, NULL);
    free_token_list(tl_err2);

    struct TokenList *_ast_tokenize_str_3;
    struct TokenList *tl_err3 =
        (tokenize_str("  ", &_ast_tokenize_str_3), _ast_tokenize_str_3);
    parse_initializer(tl_err3, 0, tl_err3->size, &err_list, NULL);
    free_token_list(tl_err3);
    init_list_free(&err_list);
  }

  for (i = 1; i < 50; ++i) {
    struct InitList list;
    init_list_init(&list);

    g_cdd_alloc_fail_countdown_countdown = i;
    if (parse_initializer(tl, 0, tl->size, &list, NULL) == 0) {
      /* Success */
    }
    g_cdd_alloc_fail_countdown_countdown = 0;

    init_list_free(&list);
  }

  for (i = 1; i < 50; ++i) {
    struct InitList list;
    init_list_init(&list);

    extern C_CDD_EXPORT int g_cdd_strdup_fail;
    g_cdd_strdup_fail = i;
    if (parse_initializer(tl, 0, tl->size, &list, NULL) == 0) {
      /* Success */
    }
    g_cdd_strdup_fail = 0;

    init_list_free(&list);
  }

  free_token_list(tl);
  PASS();
}

/**
 * @brief test_init_simple_positional
 * @return TEST
 */
TEST test_init_simple_positional(void) {
  struct TokenList *_ast_tokenize_str_0;
  const char *code = "{ 1, 2, 3 }";
  struct TokenList *tl =
      (tokenize_str(code, &_ast_tokenize_str_0), _ast_tokenize_str_0);
  struct InitList list;
  size_t consumed = 0;
  int rc;

  ASSERT(tl);
  init_list_init(&list);

  rc = parse_initializer(tl, 0, tl->size, &list, &consumed);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(3, list.count);

  ASSERT(list.items[0].designator == NULL);
  ASSERT_EQ(INIT_KIND_SCALAR, list.items[0].value->kind);
  ASSERT_STR_EQ("1", list.items[0].value->data.scalar);

  ASSERT_STR_EQ("2", list.items[1].value->data.scalar);
  ASSERT_STR_EQ("3", list.items[2].value->data.scalar);

  init_list_free(&list);
  free_token_list(tl);

  PASS();
}

/**
 * @brief test_init_designated_fields
 * @return TEST
 */
TEST test_init_designated_fields(void) {
  struct TokenList *_ast_tokenize_str_1;
  const char *code = "{ .x = 10, .y = 20 }";
  struct TokenList *tl =
      (tokenize_str(code, &_ast_tokenize_str_1), _ast_tokenize_str_1);
  struct InitList list;
  int rc;

  ASSERT(tl);
  init_list_init(&list);

  rc = parse_initializer(tl, 0, tl->size, &list, NULL);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(2, list.count);

  ASSERT_STR_EQ(".x", list.items[0].designator);
  ASSERT_STR_EQ("10", list.items[0].value->data.scalar);

  ASSERT_STR_EQ(".y", list.items[1].designator);
  ASSERT_STR_EQ("20", list.items[1].value->data.scalar);

  init_list_free(&list);
  free_token_list(tl);

  PASS();
}

/**
 * @brief test_init_array_index
 * @return TEST
 */
TEST test_init_array_index(void) {
  struct TokenList *_ast_tokenize_str_2;
  const char *code = "{ [0] = 1, [5] = 2 }";
  struct TokenList *tl =
      (tokenize_str(code, &_ast_tokenize_str_2), _ast_tokenize_str_2);
  struct InitList list;
  int rc;

  ASSERT(tl);
  init_list_init(&list);

  rc = parse_initializer(tl, 0, tl->size, &list, NULL);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(2, list.count);

  ASSERT_STR_EQ("[0]", list.items[0].designator);
  ASSERT_STR_EQ("1", list.items[0].value->data.scalar);

  ASSERT_STR_EQ("[5]", list.items[1].designator);
  ASSERT_STR_EQ("2", list.items[1].value->data.scalar);

  init_list_free(&list);
  free_token_list(tl);

  PASS();
}

/**
 * @brief test_init_nested
 * @return TEST
 */
TEST test_init_nested(void) {
  struct TokenList *_ast_tokenize_str_3;
  const char *code = "{ .pt = { .x = 1, .y = 2 }, .flag = 1 }";
  struct TokenList *tl =
      (tokenize_str(code, &_ast_tokenize_str_3), _ast_tokenize_str_3);
  struct InitList list;
  int rc;

  ASSERT(tl);
  init_list_init(&list);

  rc = parse_initializer(tl, 0, tl->size, &list, NULL);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(2, list.count);

  /* Item 0: .pt = { ... } */
  ASSERT_STR_EQ(".pt", list.items[0].designator);
  ASSERT_EQ(INIT_KIND_COMPOUND, list.items[0].value->kind);

  {
    struct InitList *sub = list.items[0].value->data.compound;
    ASSERT_EQ(2, sub->count);
    ASSERT_STR_EQ(".x", sub->items[0].designator);
    ASSERT_STR_EQ("1", sub->items[0].value->data.scalar);
  }

  /* Item 1: .flag = 1 */
  ASSERT_STR_EQ(".flag", list.items[1].designator);
  ASSERT_STR_EQ("1", list.items[1].value->data.scalar);

  init_list_free(&list);
  free_token_list(tl);

  PASS();
}

/**
 * @brief test_init_mixed_expressions
 * @return TEST
 */
TEST test_init_mixed_expressions(void) {
  struct TokenList *_ast_tokenize_str_4;
  /* Test complex expressions */
  const char *code = "{ .a = 1 + 2, .b = func(x, y), .c = (int){ 0 } }";
  struct TokenList *tl =
      (tokenize_str(code, &_ast_tokenize_str_4), _ast_tokenize_str_4);
  struct InitList list;
  int rc;

  ASSERT(tl);
  init_list_init(&list);

  rc = parse_initializer(tl, 0, tl->size, &list, NULL);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(3, list.count);

  ASSERT_STR_EQ(".a", list.items[0].designator);
  ASSERT_STR_EQ("1+2", list.items[0].value->data.scalar); /* join removes WS */

  ASSERT_STR_EQ(".b", list.items[1].designator);
  /* "func(x, y)" contains comma, parser must respect parens */
  ASSERT(strstr(list.items[1].value->data.scalar, "func("));
  ASSERT(strstr(list.items[1].value->data.scalar, "y)"));

  ASSERT_STR_EQ(".c", list.items[2].designator);
  /* "(int){ 0 }" nested braces inside expression?
     Standard parser logic treats LBRACE as compound literal IF it's not the
     start. Our parser sees LBRACE at top level as sub-init. But here LBRACE is
     after (int). Current simple logic recurses on LBRACE. So it might parse `{
     0 }` as a compound sub-list, consuming that range. Is that correct?
     Strictly, C syntax `(int){0}` is a compound literal expression.
     Our parser sees: `(int)` then `{`.
     It consumes `(int)` as scalar part?
     Actually loop: `parse_expression_str`. It counts braces. */

  /* `parse_expression_str` loop:
     ( -> depth_paren=1
     ) -> depth_paren=0
     { -> depth_brace=1
     } -> depth_brace=0
     Eventually hits ',' or '}' with depth 0.
     So it should capture `(int){0}` as a SINGLE SCALAR string. */

  ASSERT(strstr(list.items[2].value->data.scalar, "(int){0}"));

  init_list_free(&list);
  free_token_list(tl);

  PASS();
}

/**
 * @brief test_init_trailing_comma
 * @return TEST
 */
TEST test_init_trailing_comma(void) {
  struct TokenList *_ast_tokenize_str_5;
  const char *code = "{ 1, }";
  struct TokenList *tl =
      (tokenize_str(code, &_ast_tokenize_str_5), _ast_tokenize_str_5);
  struct InitList list;
  int rc;

  ASSERT(tl);
  init_list_init(&list);

  rc = parse_initializer(tl, 0, tl->size, &list, NULL);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, list.count); /* 1 is value, comma consumed, } ends loop */

  init_list_free(&list);
  free_token_list(tl);

  PASS();
}

/**
 * @brief test_init_errors
 * @return TEST
 */
TEST test_init_errors(void) {
  struct TokenList *_ast_tokenize_str_6;
  struct TokenList *_ast_tokenize_str_7;
  struct TokenList *tl;
  struct InitList list;

  init_list_init(&list);

  /* Missing brace */
  tl = (tokenize_str("1, 2", &_ast_tokenize_str_6), _ast_tokenize_str_6);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  /* Unterminated */
  tl = (tokenize_str("{ 1, 2", &_ast_tokenize_str_7), _ast_tokenize_str_7);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  /* Manually add a trailing whitespace token to guarantee it's not stripped */
  tl = (tokenize_str("{ 1,", &_ast_tokenize_str_7), _ast_tokenize_str_7);
  {
    struct Token ws_token;
    memset(&ws_token, 0, sizeof(ws_token));
    ws_token.kind = TOKEN_WHITESPACE;

    tl->tokens =
        C_CDD_REALLOC(tl->tokens, sizeof(struct Token) * (tl->size + 1));
    tl->tokens[tl->size] = ws_token;
    tl->size++;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_initializer(tl, 0, tl->size, &list, NULL));
  }
  free_token_list(tl);

  /* Designator without '=' before comma */
  tl = (tokenize_str("{ .x , 2 }", &_ast_tokenize_str_7), _ast_tokenize_str_7);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  /* Designator reaching end without '=' */
  tl = (tokenize_str("{ .x }", &_ast_tokenize_str_7), _ast_tokenize_str_7);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  /* Designator without '=' before a non-delimiter (e.g. '+') */
  tl = (tokenize_str("{ .x + 2", &_ast_tokenize_str_7), _ast_tokenize_str_7);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  /* Empty expression */
  tl =
      (tokenize_str("{ .x = , 2 }", &_ast_tokenize_str_7), _ast_tokenize_str_7);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  /* Expression with nested brackets */
  tl = (tokenize_str("{ .x = a[1] }", &_ast_tokenize_str_7),
        _ast_tokenize_str_7);
  ASSERT_EQ(0, parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  /* Empty list parsing */
  tl = (tokenize_str("{ }", &_ast_tokenize_str_7), _ast_tokenize_str_7);
  ASSERT_EQ(0, parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  /* Invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_initializer(NULL, 0, 0, &list, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_initializer(tl, 0, 0, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, init_list_init(NULL));

  /* NULL frees */
  init_list_free(NULL);

  PASS();
}

/**
 * @brief initializer_parser_suite
 */
SUITE(initializer_parser_suite) {
  RUN_TEST(test_init_oom);
  RUN_TEST(test_init_simple_positional);
  RUN_TEST(test_init_designated_fields);
  RUN_TEST(test_init_array_index);
  RUN_TEST(test_init_nested);
  RUN_TEST(test_init_mixed_expressions);
  RUN_TEST(test_init_trailing_comma);
  RUN_TEST(test_init_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_INITIALIZER_PARSER_H */
