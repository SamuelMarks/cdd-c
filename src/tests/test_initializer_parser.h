#ifndef TEST_INITIALIZER_PARSER_H
#define TEST_INITIALIZER_PARSER_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "classes/parse_initializer.h"
#include "functions/parse_tokenizer.h"

static struct TokenList *tokenize_str(const char *s) {
  struct TokenList *tl = NULL;
  (void)tokenize(az_span_create_from_str((char *)s), &tl);
  return tl;
}

TEST test_init_simple_positional(void) {
  const char *code = "{ 1, 2, 3 }";
  struct TokenList *tl = tokenize_str(code);
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

TEST test_init_designated_fields(void) {
  const char *code = "{ .x = 10, .y = 20 }";
  struct TokenList *tl = tokenize_str(code);
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

TEST test_init_array_index(void) {
  const char *code = "{ [0] = 1, [5] = 2 }";
  struct TokenList *tl = tokenize_str(code);
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

TEST test_init_nested(void) {
  const char *code = "{ .pt = { .x = 1, .y = 2 }, .flag = 1 }";
  struct TokenList *tl = tokenize_str(code);
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

TEST test_init_mixed_expressions(void) {
  /* Test complex expressions */
  const char *code = "{ .a = 1 + 2, .b = func(x, y), .c = (int){ 0 } }";
  struct TokenList *tl = tokenize_str(code);
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

TEST test_init_trailing_comma(void) {
  const char *code = "{ 1, }";
  struct TokenList *tl = tokenize_str(code);
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

TEST test_init_errors(void) {
  struct TokenList *tl;
  struct InitList list;

  init_list_init(&list);

  /* Missing brace */
  tl = tokenize_str("1, 2");
  ASSERT_EQ(EINVAL, parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  /* Unterminated */
  tl = tokenize_str("{ 1, 2");
  ASSERT_EQ(EINVAL, parse_initializer(tl, 0, tl->size, &list, NULL));
  init_list_free(&list);
  free_token_list(tl);

  PASS();
}

SUITE(initializer_parser_suite) {
  RUN_TEST(test_init_simple_positional);
  RUN_TEST(test_init_designated_fields);
  RUN_TEST(test_init_array_index);
  RUN_TEST(test_init_nested);
  RUN_TEST(test_init_mixed_expressions);
  RUN_TEST(test_init_trailing_comma);
  RUN_TEST(test_init_errors);
}

#endif /* TEST_INITIALIZER_PARSER_H */
