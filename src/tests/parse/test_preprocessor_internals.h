/**
 * @file test_preprocessor_internals.h
 * @brief Comprehensive unit tests for C preprocessor internals and edge cases.
 */

#ifndef TEST_PREPROCESSOR_INTERNALS_H
#define TEST_PREPROCESSOR_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "c_cdd/memory.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "functions/parse/preprocessor.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#define TEST_PP_SEP '\\'
#else
#define TEST_PP_SEP '/'
#endif

TEST test_pp_join_path_and_file_exists(void) {
  char *out = NULL;
  int exists = 0;
  cdd_c_error_t rc;

  /* Test pp_join_path NULL arguments */
  rc = pp_join_path(NULL, "file", &out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_join_path("dir", NULL, &out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_join_path("dir", "file", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Test pp_join_path success */
  rc = pp_join_path("dir", "file.h", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out != NULL);
  C_CDD_FREE(out);
  out = NULL;

  /* Test pp_join_path OOM */
  g_cdd_alloc_fail = 1;
  rc = pp_join_path("dir", "file.h", &out);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  ASSERT_EQ(NULL, out);
  g_cdd_alloc_fail = 0;

  /* Test pp_file_exists NULL arguments */
  rc = pp_file_exists(NULL, &exists);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_file_exists("CMakeLists.txt", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Test pp_file_exists on existing and nonexistent file */
  write_to_file("test_pp_exists.tmp", "content\n");
  rc = pp_file_exists("test_pp_exists.tmp", &exists);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, exists);
  remove("test_pp_exists.tmp");

  rc = pp_file_exists("nonexistent_test_12345_abc.tmp", &exists);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, exists);

  PASS();
}

TEST test_pp_token_and_reconstruct_path(void) {
  struct TokenList *tl = NULL;
  char *out = NULL;
  cdd_c_error_t rc;

  /* Test pp_token_to_string NULL arguments */
  rc = pp_token_to_string(NULL, &out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "sys / stat . h"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl != NULL);

  rc = pp_token_to_string(&tl->tokens[0], NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = pp_token_to_string(&tl->tokens[0], &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("sys", out);
  C_CDD_FREE(out);
  out = NULL;

  /* Test pp_token_to_string OOM */
  g_cdd_alloc_fail = 1;
  rc = pp_token_to_string(&tl->tokens[0], &out);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  ASSERT_EQ(NULL, out);
  g_cdd_alloc_fail = 0;

  /* Test pp_reconstruct_path NULL arguments */
  rc = pp_reconstruct_path(NULL, 0, tl->size, &out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_reconstruct_path(tl, 0, tl->size, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Test pp_reconstruct_path start >= end (empty string) */
  rc = pp_reconstruct_path(tl, 3, 2, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("", out);
  C_CDD_FREE(out);
  out = NULL;

  /* Test pp_reconstruct_path success */
  rc = pp_reconstruct_path(tl, 0, tl->size, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out != NULL);
  C_CDD_FREE(out);
  out = NULL;

  /* Test pp_reconstruct_path OOM */
  g_cdd_alloc_fail = 1;
  rc = pp_reconstruct_path(tl, 0, tl->size, &out);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  ASSERT_EQ(NULL, out);
  g_cdd_alloc_fail = 0;

  free_token_list(tl);
  PASS();
}

TEST test_pp_resolve_path_cases(void) {
  struct PreprocessorContext ctx;
  char *resolved = NULL;
  cdd_c_error_t rc;

  pp_context_init(&ctx);

  /* NULL argument checks */
  rc = pp_resolve_path(NULL, NULL, NULL, 0, &resolved);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_resolve_path(&ctx, NULL, "CMakeLists.txt", 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Not found anywhere */
  rc = pp_resolve_path(&ctx, ".", "nonexistent_file_99999.h", 0, &resolved);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, resolved);

  write_to_file("test_pp_exists.tmp", "content\n");

  /* Found in current_dir (quoted include) */
  rc = pp_resolve_path(&ctx, ".", "test_pp_exists.tmp", 0, &resolved);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(resolved != NULL);
  C_CDD_FREE(resolved);
  resolved = NULL;

  /* System include ignores current_dir */
  rc = pp_resolve_path(&ctx, ".", "test_pp_exists.tmp", 1, &resolved);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, resolved);

  /* Found in search_paths */
  pp_add_search_path(&ctx, ".");
  rc = pp_resolve_path(&ctx, NULL, "test_pp_exists.tmp", 1, &resolved);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(resolved != NULL);
  C_CDD_FREE(resolved);
  resolved = NULL;

  remove("test_pp_exists.tmp");

  /* Search path with nonexistent file */
  rc = pp_resolve_path(&ctx, NULL, "nonexistent_file_8888.h", 1, &resolved);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, resolved);

  pp_context_free(&ctx);
  PASS();
}

TEST test_pp_context_and_macro_operations(void) {
  struct PreprocessorContext ctx;
  struct EmbedParams params;
  cdd_c_error_t rc;
  int i;
  char buf[32];

  /* NULL context init */
  rc = pp_context_init(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Safe NULL context free */
  pp_context_free(NULL);

  rc = pp_context_init(&ctx);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* NULL checks for pp_add_search_path */
  rc = pp_add_search_path(NULL, "path");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_add_search_path(&ctx, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* pp_add_search_path OOM in copy */
  g_cdd_strdup_fail = 1;
  rc = pp_add_search_path(&ctx, "path");
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* Add search paths up to and past initial capacity 8 to trigger realloc */
  for (i = 0; i < 10; ++i) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, sizeof(buf), "dir_%d", i);
#else
    sprintf(buf, "dir_%d", i);
#endif
    rc = pp_add_search_path(&ctx, buf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }
  ASSERT_EQ(10, ctx.size);
  ASSERT(ctx.capacity >= 10);

  /* NULL checks for pp_add_macro */
  rc = pp_add_macro(NULL, "NAME", "VAL");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_add_macro(&ctx, NULL, "VAL");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* pp_add_macro without value */
  rc = pp_add_macro(&ctx, "EMPTY_MACRO", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* pp_add_macro with value */
  rc = pp_add_macro(&ctx, "VAL_MACRO", "123");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* pp_add_macro OOM on name */
  g_cdd_strdup_fail = 1;
  rc = pp_add_macro(&ctx, "FAIL_NAME", "VAL");
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* pp_add_macro OOM on value */
  g_cdd_strdup_fail = 2;
  rc = pp_add_macro(&ctx, "FAIL_VAL", "VAL");
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* Add macros up to and past initial capacity 16 to trigger realloc */
  for (i = 0; i < 20; ++i) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, sizeof(buf), "MACRO_%d", i);
#else
    sprintf(buf, "MACRO_%d", i);
#endif
    rc = pp_add_macro(&ctx, buf, "1");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }
  ASSERT(ctx.macro_count >= 20);
  ASSERT(ctx.macro_capacity >= 20);

  pp_context_free(&ctx);

  /* Test pp_embed_params_free NULL and populated */
  pp_embed_params_free(NULL);
  memset(&params, 0, sizeof(params));
  c_cdd_strdup("prefix", &params.prefix);
  c_cdd_strdup("suffix", &params.suffix);
  c_cdd_strdup("if_empty", &params.if_empty);
  pp_embed_params_free(&params);
  ASSERT_EQ(NULL, params.prefix);
  ASSERT_EQ(NULL, params.suffix);
  ASSERT_EQ(NULL, params.if_empty);

  PASS();
}

TEST test_pp_conditional_stack_operations(void) {
  struct ConditionalStack st;
  enum CondState s = COND_ACTIVE;
  int enabled = 0;
  int i;
  cdd_c_error_t rc;

  memset(&st, 0, sizeof(st));

  /* NULL argument checks */
  rc = pp_stack_push(NULL, COND_ACTIVE);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_stack_pop(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_stack_peek(NULL, &s);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_stack_peek(&st, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_is_enabled(NULL, &enabled);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_is_enabled(&st, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Empty stack tests */
  st.top = -1;
  rc = pp_stack_peek(&st, &s);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(COND_ACTIVE, s);

  rc = pp_is_enabled(&st, &enabled);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, enabled);

  /* Pop when empty (safe underflow guard) */
  rc = pp_stack_pop(&st);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(-1, st.top);

  /* Push states and test peek & is_enabled */
  rc = pp_stack_push(&st, COND_ACTIVE);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_is_enabled(&st, &enabled);
  ASSERT_EQ(1, enabled);

  rc = pp_stack_push(&st, COND_SKIPPING);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_is_enabled(&st, &enabled);
  ASSERT_EQ(0, enabled);

  rc = pp_stack_peek(&st, &s);
  ASSERT_EQ(COND_SKIPPING, s);

  rc = pp_stack_pop(&st);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_stack_peek(&st, &s);
  ASSERT_EQ(COND_ACTIVE, s);

  rc = pp_stack_push(&st, COND_SATISFIED);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_is_enabled(&st, &enabled);
  ASSERT_EQ(0, enabled);

  /* Overflow guard test (push 35 times past limit 31) */
  st.top = -1;
  for (i = 0; i < 35; ++i) {
    rc = pp_stack_push(&st, COND_ACTIVE);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }
  ASSERT_EQ(31, st.top);

  PASS();
}

TEST test_pp_is_defined_macro_operations(void) {
  struct PreprocessorContext ctx;
  struct Token tok;
  int is_def = 0;
  cdd_c_error_t rc;

  pp_context_init(&ctx);
  pp_add_macro(&ctx, "DEFINED_MACRO", "1");

  memset(&tok, 0, sizeof(tok));
  tok.kind = TOKEN_IDENTIFIER;
  tok.start = (const uint8_t *)"DEFINED_MACRO";
  tok.length = strlen("DEFINED_MACRO");

  /* NULL output */
  rc = pp_is_defined_macro(&ctx, &tok, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* NULL ctx or tok */
  rc = pp_is_defined_macro(NULL, &tok, &is_def);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_def);

  rc = pp_is_defined_macro(&ctx, NULL, &is_def);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_def);

  /* Found macro */
  rc = pp_is_defined_macro(&ctx, &tok, &is_def);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_def);

  /* Absent macro */
  tok.start = (const uint8_t *)"OTHER_MACRO";
  tok.length = strlen("OTHER_MACRO");
  rc = pp_is_defined_macro(&ctx, &tok, &is_def);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_def);

  pp_context_free(&ctx);
  PASS();
}

TEST test_pp_parse_embed_params_cases(void) {
  struct PreprocessorContext ctx;
  struct TokenList *tl = NULL;
  struct EmbedParams params;
  cdd_c_error_t rc;

  pp_context_init(&ctx);
  memset(&params, 0, sizeof(params));

  /* NULL arguments */
  rc = pp_parse_embed_params(NULL, 0, 0, &ctx, &params);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = tokenize(az_span_create_from_str((char *)(size_t) "limit(10)"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  /* Valid standard params: limit, prefix, suffix, if_empty */
  rc = tokenize(
      az_span_create_from_str(
          (char *)(size_t) "limit(5 * 2) prefix(\"PRE_\") suffix(\"_SUF\") "
                           "if_empty(\"EMPTY\") unknown(123)"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(10, params.limit);
  ASSERT(params.prefix != NULL);
  ASSERT(params.suffix != NULL);
  ASSERT(params.if_empty != NULL);
  pp_embed_params_free(&params);
  free_token_list(tl);

  /* Scoped attribute: vendor::custom_param(10) */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "vendor :: custom_param (10)"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_token_list(tl);

  /* Scoped attribute missing identifier after :: */
  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "vendor :: (10)"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  /* Missing LPAREN */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "limit 10"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  /* Unbalanced parens */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "limit((10)"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  pp_context_free(&ctx);
  PASS();
}

TEST test_pp_eval_all_branches(void) {
  struct PreprocessorContext ctx;
  struct TokenList *tl = NULL;
  long val = 0;
  cdd_c_error_t rc;

  pp_context_init(&ctx);
  pp_add_search_path(&ctx, ".");
  pp_add_macro(&ctx, "TEST_VAL_10", "10");
  pp_add_macro(&ctx, "EMPTY_MACRO", NULL);

  /* NULL argument checks */
  rc = pp_eval_expression(NULL, 0, 0, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = tokenize(az_span_create_from_str((char *)(size_t) "1 + 1"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_eval_expression(tl, 2, 1, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = pp_eval_expression(tl, 0, tl->size + 5, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  /* Binary literals 0b... and 0B... */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "0b101 + 0B010"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(7, val);
  free_token_list(tl);

  /* Division and modulo by zero */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "(10 / 0) + (10 % 0)"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, val);
  free_token_list(tl);

  /* Shifts: << and >> */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "(1 << 4) >> 2"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(4, val);
  free_token_list(tl);

  /* Relational operators <=, >=, <, > */
  rc = tokenize(
      az_span_create_from_str(
          (char *)(size_t) "(1 <= 2) && (2 >= 1) && (1 < 2) && (2 > 1)"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, val);
  free_token_list(tl);

  rc = tokenize(
      az_span_create_from_str(
          (char *)(size_t) "(2 <= 1) || (1 >= 2) || (2 < 1) || (1 > 2)"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, val);
  free_token_list(tl);

  /* Equality == and != */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "(1 == 1) && (1 != 2)"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, val);
  free_token_list(tl);

  /* Unary operators +, -, !, ~ */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "+5 - (-3) + !0 + (~0 == -1)"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(10, val);
  free_token_list(tl);

  /* Defined without parentheses and with parentheses */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "defined TEST_VAL_10 && "
                                     "defined(TEST_VAL_10) && !defined(NOPE)"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, val);
  free_token_list(tl);

  /* Defined missing identifier */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "defined"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  /* Defined missing RPAREN */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "defined(TEST_VAL_10"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  /* Macro evaluation lookup and undefined identifier resolving to 0 */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "TEST_VAL_10 + UNDEFINED_ID"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(10, val);
  free_token_list(tl);

  /* Unclosed paren in expression */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "(1 + 2"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  write_to_file("test_pp_exists.tmp", "content\n");

  /* __has_include cases */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "__has_include(\"test_pp_exists.tmp\")"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, val);
  free_token_list(tl);

  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "__has_include(\"nonexistent.h\")"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, val);
  free_token_list(tl);

  /* __has_include angle bracket */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "__has_include(<nonexistent.h>)"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, val);
  free_token_list(tl);

  /* __has_include errors */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "__has_include"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  rc = tokenize(az_span_create_from_str((char *)(size_t) "__has_include()"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  rc = tokenize(az_span_create_from_str((char *)(size_t) "__has_include(123)"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "__has_include(<unclosed"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "__has_include(\"test_pp_exists.tmp\""),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  /* __has_embed cases */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "__has_embed(\"test_pp_exists.tmp\")"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, val);
  free_token_list(tl);

  remove("test_pp_exists.tmp");

  /* __has_c_attribute all standard attributes */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "__has_c_attribute(deprecated) && "
                                     "__has_c_attribute(fallthrough) && "
                                     "__has_c_attribute(maybe_unused) && "
                                     "__has_c_attribute(nodiscard) && "
                                     "__has_c_attribute(noreturn) && "
                                     "__has_c_attribute(unsequenced) && "
                                     "__has_c_attribute(reproducible)"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, val);
  free_token_list(tl);

  /* __has_c_attribute scoped and keyword */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "__has_c_attribute(gnu::nonnull) == 0 && "
                                     "__has_c_attribute(inline) == 0"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, val);
  free_token_list(tl);

  /* __has_c_attribute error cases: missing LPAREN, missing RPAREN */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "__has_c_attribute"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "__has_c_attribute(nodiscard"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  pp_context_free(&ctx);
  PASS();
}

#include "parse/test_preprocessor_internals_branches.h"
#include "parse/test_preprocessor_internals_coverage.h"
#include "parse/test_preprocessor_internals_directives.h"

SUITE(preprocessor_internals_suite) {
  RUN_TEST(test_pp_100_percent_coverage);
  RUN_TEST(test_pp_100_cov_directives);
  RUN_TEST(test_pp_branch_coverage_maximizer);
  RUN_TEST(test_pp_final_edge_coverage);
  RUN_TEST(test_pp_scan_defines_oom_and_trimming);
  RUN_TEST(test_pp_eval_oom_and_peek_eof);
  RUN_TEST(test_pp_parse_embed_params_keywords_and_oom_paths);
  RUN_TEST(test_pp_scan_includes_elif_active_and_oom_paths);
  RUN_TEST(test_pp_arithmetic_all_operators);
  RUN_TEST(test_pp_realloc_and_null_internals);
  RUN_TEST(test_pp_eval_all_syntax_error_branches);
  RUN_TEST(test_pp_scan_defines_whitespace_trimming);
  RUN_TEST(test_pp_scan_includes_eof_and_else_active);
  RUN_TEST(test_pp_embed_params_error_cases);
  RUN_TEST(test_pp_scan_includes_nested_and_syntax_errors);
  RUN_TEST(test_pp_join_path_and_file_exists);
  RUN_TEST(test_pp_token_and_reconstruct_path);
  RUN_TEST(test_pp_resolve_path_cases);
  RUN_TEST(test_pp_context_and_macro_operations);
  RUN_TEST(test_pp_conditional_stack_operations);
  RUN_TEST(test_pp_is_defined_macro_operations);
  RUN_TEST(test_pp_parse_embed_params_cases);
  RUN_TEST(test_pp_eval_all_branches);
  RUN_TEST(test_pp_scan_includes_directives);
  RUN_TEST(test_pp_scan_defines_edge_cases);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_PREPROCESSOR_INTERNALS_H */
