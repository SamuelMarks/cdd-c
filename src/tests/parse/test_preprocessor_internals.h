/**
 * @file test_preprocessor_internals.h
 * @brief Comprehensive unit tests for C preprocessor internals and edge cases.
 */

#ifndef TEST_PREPROCESSOR_INTERNALS_H
#define TEST_PREPROCESSOR_INTERNALS_H

#ifdef __cplusplus
extern C {
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

    rc = tokenize(az_span_create_from_str((char *)(size_t) "sys / stat . h"),
                  &tl);
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
    rc = tokenize(az_span_create_from_str((char *)(size_t) "vendor :: (10)"),
                  &tl);
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
    rc = tokenize(az_span_create_from_str((char *)(size_t) "0b101 + 0B010"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(7, val);
    free_token_list(tl);

    /* Division and modulo by zero */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "(10 / 0) + (10 % 0)"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, val);
    free_token_list(tl);

    /* Shifts: << and >> */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "(1 << 4) >> 2"),
                  &tl);
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
    rc = tokenize(az_span_create_from_str((
                      char *)(size_t) "defined TEST_VAL_10 && "
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
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "defined(TEST_VAL_10"), &tl);
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
    rc = tokenize(az_span_create_from_str((char *)(size_t) "__has_include"),
                  &tl);
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

    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_include(123)"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);

    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_include(<unclosed"),
        &tl);
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
    rc = tokenize(az_span_create_from_str((
                      char *)(size_t) "__has_c_attribute(gnu::nonnull) == 0 && "
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

  static cdd_c_error_t test_scan_inc_cb(const struct IncludeInfo *info,
                                        void *user_data) {
    int *count = (int *)user_data;
    (*count)++;
    (void)info;
    return CDD_C_SUCCESS;
  }

  static cdd_c_error_t test_scan_inc_abort_cb(const struct IncludeInfo *info,
                                              void *user_data) {
    int *count = (int *)user_data;
    (*count)++;
    (void)info;
    return CDD_C_ERROR_UNKNOWN;
  }

  TEST test_pp_scan_includes_directives(void) {
    struct PreprocessorContext ctx;
    const char *test_file = "test_pp_scan_cond.c";
    const char *inc_file = "test_inc_target.h";
    int count = 0;
    cdd_c_error_t rc;

    write_to_file(inc_file, "/* header */\n");

    /* Test NULL arguments */
    rc = pp_scan_includes(NULL, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_scan_includes(test_file, NULL, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* Non-existent file */
    rc = pp_scan_includes("nonexistent_scan_file_xyz.c", &ctx, test_scan_inc_cb,
                          &count);
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* Complex conditional matrix:
     * #ifdef, #ifndef, #if, #elif, #else, #endif, #embed, #include,
     * #include_next
     */
    write_to_file(test_file, "#define DEF_1\n"
                             "#ifdef DEF_1\n"
                             "#include \"test_inc_target.h\"\n"
                             "#else\n"
                             "#include \"nonexistent.h\"\n"
                             "#endif\n"
                             "#ifndef DEF_1\n"
                             "#include \"nonexistent.h\"\n"
                             "#elif 1\n"
                             "#include \"test_inc_target.h\"\n"
                             "#else\n"
                             "#include \"nonexistent.h\"\n"
                             "#endif\n"
                             "#if 0\n"
                             "#include \"nonexistent.h\"\n"
                             "#elif 0\n"
                             "#include \"nonexistent.h\"\n"
                             "#else\n"
                             "#include \"test_inc_target.h\"\n"
                             "#endif\n"
                             "#embed \"test_inc_target.h\" limit(10) "
                             "prefix(\"P\") suffix(\"S\") if_empty(\"E\")\n"
                             "#include_next \"test_inc_target.h\"\n");

    pp_context_init(&ctx);
    pp_add_macro(&ctx, "DEF_1", "1");
    count = 0;
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(5, count);

    /* Abort callback on embed */
    write_to_file(test_file, "#embed \"test_inc_target.h\" limit(5)\n"
                             "#include \"test_inc_target.h\"\n");
    count = 0;
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_abort_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, count);

    pp_context_free(&ctx);
    remove(test_file);
    remove(inc_file);
    PASS();
  }

  TEST test_pp_scan_defines_edge_cases(void) {
    struct PreprocessorContext ctx;
    const char *test_file = "test_pp_scan_defs_edge.h";
    cdd_c_error_t rc;

    /* NULL argument checks */
    rc = pp_scan_defines(NULL, test_file);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    pp_context_init(&ctx);
    rc = pp_scan_defines(&ctx, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* Nonexistent file */
    rc = pp_scan_defines(&ctx, "nonexistent_defs_file_xyz.h");
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* Scan diverse define constructs */
    write_to_file(test_file, "#   define OBJ_EMPTY\n"
                             "#define   OBJ_VAL   42   \n"
                             "#define FN_ZERO() 100\n"
                             "#define FN_RECOVERY(a; b) a\n"
                             "#define FN_VAR_STANDARD(a, ...) a\n"
                             "#define FN_VAR_GCC(args...) args\n"
                             "#define UNCLOSED(a, b\n");

    rc = pp_scan_defines(&ctx, test_file);
    if (rc != CDD_C_SUCCESS) {
    }
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(ctx.macro_count >= 5);

    pp_context_free(&ctx);
    remove(test_file);
    PASS();
  }

  TEST test_pp_embed_params_error_cases(void) {
    struct PreprocessorContext ctx;
    struct TokenList *tl = NULL;
    struct EmbedParams params;
    cdd_c_error_t rc;

    pp_context_init(&ctx);
    memset(&params, 0, sizeof(params));

    /* Scoped param missing LPAREN */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "vendor :: custom 10"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);

    /* Scoped param unbalanced parens */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "vendor :: custom ((10)"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);

    /* limit with syntax error in expression */
    rc =
        tokenize(az_span_create_from_str((char *)(size_t) "limit((1 +))"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);

    pp_context_free(&ctx);
    PASS();
  }

  TEST test_pp_scan_includes_nested_and_syntax_errors(void) {
    struct PreprocessorContext ctx;
    const char *test_file = "test_pp_nested.c";
    const char *inc_file = "test_inc_target.h";
    int count = 0;
    cdd_c_error_t rc;

    write_to_file(inc_file, "/* header */\n");

    /* Nested conditionals in inactive blocks, invalid #if and #elif, and
     * invalid #embed */
    write_to_file(test_file, "#if 0\n"
                             "#ifdef NESTED_DEF\n"
                             "#endif\n"
                             "#if NESTED_EXPR\n"
                             "#endif\n"
                             "#elif 0\n"
                             "#else\n"
                             "#include \"test_inc_target.h\"\n"
                             "#endif\n"
                             "#if 0\n#include \"test_inc_target.h\"\n#endif\n"
                             "#define A 1 #define B 2\n"
                             "#embed \"test_inc_target.h\" limit((1 +))\n");

    pp_context_init(&ctx);
    count = 0;
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* Test invalid #if syntax */
    write_to_file(test_file, "#if (1 + 2\n");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* Test invalid #elif syntax */
    write_to_file(test_file, "#if 0\n"
                             "#elif (1 + 2\n"
                             "#endif\n");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    pp_context_free(&ctx);
    remove(test_file);
    remove(inc_file);
    PASS();
  }

  TEST test_pp_eval_all_syntax_error_branches(void) {
    struct PreprocessorContext ctx;
    struct TokenList *tl = NULL;
    long val = 0;
    cdd_c_error_t rc;
    const char *err_exprs[] = {"+ )",
                               "- )",
                               "! )",
                               "~ )",
                               "1 * )",
                               "1 / )",
                               "1 % )",
                               "1 + )",
                               "1 - )",
                               "1 << )",
                               "1 >> )",
                               "1 <= )",
                               "1 >= )",
                               "1 < )",
                               "1 > )",
                               "1 == )",
                               "1 != )",
                               "1 && )",
                               "1 || )",
                               "__has_include(",
                               "__has_include(\"test.h\", 1, 2)",
                               "1 < ",
                               "@"};
    size_t i;

    pp_context_init(&ctx);
    pp_add_search_path(&ctx, ".");

    for (i = 0; i < sizeof(err_exprs) / sizeof(err_exprs[0]); ++i) {
      rc = tokenize(az_span_create_from_str((char *)(size_t)err_exprs[i]), &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
      free_token_list(tl);
    }

    /* Empty token list */
    rc = tokenize(az_span_create_from_str((char *)(size_t) ""), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);

    pp_context_free(&ctx);
    PASS();
  }

  TEST test_pp_scan_defines_whitespace_trimming(void) {
    struct PreprocessorContext ctx;
    const char *test_file = "test_pp_trim.h";
    cdd_c_error_t rc;

    write_to_file(test_file, "#define CR_VAL 123\r\n"
                             "#define SPACES_VAL    \n");

    pp_context_init(&ctx);
    rc = pp_scan_defines(&ctx, test_file);
    if (rc != CDD_C_SUCCESS) {
    }
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(ctx.macro_count >= 1);

    pp_context_free(&ctx);
    remove(test_file);
    PASS();
  }

  TEST test_pp_scan_includes_eof_and_else_active(void) {
    struct PreprocessorContext ctx;
    const char *test_file = "test_pp_eof.c";
    const char *inc_file = "test_inc_target.h";
    int count = 0;
    cdd_c_error_t rc;

    write_to_file(inc_file, "/* header */\n");

    /* #else after #if 1, and #include at EOF without trailing newline */
    write_to_file(test_file, "#if 1\n"
                             "#   include \"test_inc_target.h\"\n"
                             "#else\n"
                             "#include \"nonexistent.h\"\n"
                             "#endif\n"
                             "#include \"test_inc_target.h\"");

    pp_context_init(&ctx);
    count = 0;
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(2, count);

    pp_context_free(&ctx);
    remove(test_file);
    remove(inc_file);
    PASS();
  }

  TEST test_pp_arithmetic_all_operators(void) {
    struct PreprocessorContext ctx;
    struct TokenList *tl = NULL;
    long val = 0;
    cdd_c_error_t rc;

    pp_context_init(&ctx);

#define EVAL_ASSERT(expr_str, expected)                                        \
  do {                                                                         \
    rc = tokenize(az_span_create_from_str((char *)(size_t)(expr_str)), &tl);   \
    ASSERT_EQ(CDD_C_SUCCESS, rc);                                              \
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);                      \
    ASSERT_EQ(CDD_C_SUCCESS, rc);                                              \
    ASSERT_EQ((expected), val);                                                \
    free_token_list(tl);                                                       \
  } while (0)

    EVAL_ASSERT("(20 / 4) == 5", 1);
    EVAL_ASSERT("(20 % 6) == 2", 1);
    EVAL_ASSERT("(10 - 3) == 7", 1);
    EVAL_ASSERT("(10 + 3) == 13", 1);
    EVAL_ASSERT("(1 << 3) == 8", 1);
    EVAL_ASSERT("(8 >> 2) == 2", 1);
    EVAL_ASSERT("(5 <= 5) && (5 >= 5) && (3 < 5) && (5 > 3)", 1);
    EVAL_ASSERT("(5 <= 4) || (4 >= 5) || (5 < 3) || (3 > 5)", 0);
    EVAL_ASSERT("(5 == 5) && (5 != 4)", 1);
    EVAL_ASSERT("(5 == 4) || (5 != 5)", 0);
    EVAL_ASSERT("(1 && 1) && !(1 && 0) && !(0 && 1)", 1);
    EVAL_ASSERT("(1 || 0) && (0 || 1) && !(0 || 0)", 1);

#undef EVAL_ASSERT

    pp_context_free(&ctx);
    PASS();
  }

  TEST test_pp_realloc_and_null_internals(void) {
    struct PreprocessorContext ctx;
    struct MacroDef def;
    char *resolved = NULL;
    cdd_c_error_t rc;

    pp_free_macro_def(NULL);
    rc = pp_add_macro_internal(NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    pp_context_init(&ctx);
    memset(&def, 0, sizeof(def));
    c_cdd_strdup("TEST_MACRO", &def.name);
    rc = pp_add_macro_internal(&ctx, &def);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Trigger capacity realloc failure on search_paths */
    rc = pp_add_search_path(&ctx, "init_path");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    while (ctx.size < ctx.capacity) {
      rc = pp_add_search_path(&ctx, "filler");
      ASSERT_EQ(CDD_C_SUCCESS, rc);
    }
    g_cdd_alloc_fail = 1;
    rc = pp_add_search_path(&ctx, "overflow");
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    /* Trigger capacity realloc failure on macros */
    while (ctx.macro_count < ctx.macro_capacity) {
      rc = pp_add_macro(&ctx, "M_FILL", "1");
      ASSERT_EQ(CDD_C_SUCCESS, rc);
    }
    g_cdd_alloc_fail = 1;
    rc = pp_add_macro(&ctx, "M_OVERFLOW", "1");
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    /* pp_resolve_path OOM during join_path on current_dir */
    g_cdd_alloc_fail = 1;
    rc = pp_resolve_path(&ctx, ".", "target.h", 0, &resolved);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    /* pp_resolve_path OOM during join_path on search_paths */
    rc = pp_add_search_path(&ctx, "filler_dir");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 1;
    rc = pp_resolve_path(&ctx, NULL, "target.h", 1, &resolved);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    pp_context_free(&ctx);
    PASS();
  }

  TEST test_pp_scan_defines_oom_and_trimming(void) {
    struct PreprocessorContext ctx;
    const char *test_file = "test_pp_scan_oom.h";
    cdd_c_error_t rc;

    pp_context_init(&ctx);

    /* Multi-token with trailing CR and spaces, and empty spaces */
    write_to_file(test_file, "#define CR_MACRO 100 /* c */ \r\n"
                             "#define EMPTY_TRIM   /* c */\n"
                             "#define FN_OOM(a, b) a + b\n");
    rc = pp_scan_defines(&ctx, test_file);
    if (rc != CDD_C_SUCCESS) {
    }
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* OOM tests for pp_scan_defines args and macro addition */
    write_to_file(test_file, "#define FN_ARGS(a, b) a\n");
    g_cdd_alloc_fail = 5;
    (void)pp_scan_defines(&ctx, test_file);
    g_cdd_alloc_fail = 0;

    g_cdd_alloc_fail = 6;
    (void)pp_scan_defines(&ctx, test_file);
    g_cdd_alloc_fail = 0;

    g_cdd_alloc_fail = 7;
    (void)pp_scan_defines(&ctx, test_file);
    g_cdd_alloc_fail = 0;

    pp_context_free(&ctx);
    remove(test_file);
    PASS();
  }

  TEST test_pp_eval_oom_and_peek_eof(void) {
    struct PreprocessorContext ctx;
    struct TokenList *tl = NULL;
    long val = 0;
    cdd_c_error_t rc;

    pp_context_init(&ctx);
    pp_add_search_path(&ctx, ".");

    /* preprocessor_peek at EOF with trailing whitespace: '1   ' */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "1   "), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, val);
    free_token_list(tl);

    /* __has_include(<stdio.h>) with reconstruct_path OOM */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_include(<stdio.h>)"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 1;
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* __has_include("test.h") with path malloc OOM */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_include(\"test.h\")"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 1;
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* __has_include("test.h") with resolve_path OOM */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_include(\"test.h\")"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 2;
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* __has_c_attribute(nodiscard) with token_to_string OOM */
    rc = tokenize(az_span_create_from_str(
                      (char *)(size_t) "__has_c_attribute(nodiscard)"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 1;
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* __has_c_attribute(gnu::nonnull) with token_to_string OOM on scoped name
     */
    rc = tokenize(az_span_create_from_str(
                      (char *)(size_t) "__has_c_attribute(gnu::nonnull)"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 2;
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* number literal 42 with token_to_string OOM */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "42"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 1;
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* __has_c_attribute(inline) with token_to_string OOM on keyword */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_c_attribute(inline)"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 1;
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* pp_preprocessor_peek direct NULL and EOF tests */
    {
      enum TokenKind peek_k = TOKEN_UNKNOWN;
      struct ExprState es;
      rc = pp_preprocessor_peek(NULL, &peek_k);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      rc = pp_preprocessor_peek(&es, NULL);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

      rc = tokenize(az_span_create_from_str((char *)(size_t) "   "), &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      memset(&es, 0, sizeof(es));
      es.tokens = tl;
      es.pos = 0;
      es.end = tl->size;
      rc = pp_preprocessor_peek(&es, &peek_k);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(TOKEN_UNKNOWN, peek_k);
      free_token_list(tl);
    }

    pp_context_free(&ctx);
    PASS();
  }

  TEST test_pp_parse_embed_params_keywords_and_oom_paths(void) {
    struct PreprocessorContext ctx;
    struct TokenList *tl = NULL;
    struct EmbedParams params;
    cdd_c_error_t rc;

    pp_context_init(&ctx);
    memset(&params, 0, sizeof(params));

    /* Keyword param and trailing whitespace */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "inline(10)   "),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    pp_embed_params_free(&params);
    free_token_list(tl);

    /* OOM on param name token_to_string */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "limit(10)"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 1;
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* OOM on scoped name token_to_string */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "vendor::custom(10)"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 2;
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* OOM on prefix */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "prefix(\"A\")"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 2;
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* OOM on suffix */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "suffix(\"A\")"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 2;
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    /* OOM on if_empty */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "if_empty(\"A\")"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_alloc_fail = 2;
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    free_token_list(tl);

    pp_context_free(&ctx);
    PASS();
  }

  TEST test_pp_scan_includes_elif_active_and_oom_paths(void) {
    struct PreprocessorContext ctx;
    const char *test_file = "test_pp_elif_active.c";
    const char *inc_file = "test_inc_target.h";
    int count = 0;
    cdd_c_error_t rc;

    write_to_file(inc_file, "/* header */\n");

    /* #if 1 followed by #elif 1 (hits active -> satisfied transition) */
    write_to_file(test_file, "#if 1\n"
                             "#elif 1\n"
                             "#endif\n");

    pp_context_init(&ctx);
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* OOM on read_to_file during scan_includes */
    g_cdd_alloc_fail = 1;
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    /* OOM on tokenize during scan_includes */
    g_cdd_alloc_fail = 2;
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    /* OOM on get_dirname during scan_includes */
    g_cdd_alloc_fail = 4;
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    pp_context_free(&ctx);
    remove(test_file);
    remove(inc_file);
    PASS();
  }

  TEST test_pp_final_edge_coverage(void) {
    struct PreprocessorContext ctx;
    const char *test_defs = "test_final_defs.h";
    const char *test_inc = "test_final_inc.c";
    int count = 0;
    int k = 0;
    cdd_c_error_t rc;

    pp_context_init(&ctx);

    /* 1. Trailing whitespace token before comment: triggers val_end_idx-- */
    write_to_file(test_defs, "#define TRAIL_WS 123   \t   /* comment */\n");
    rc = pp_scan_defines(&ctx, test_defs);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* 2. add_macro_internal realloc failure in pp_scan_defines */
    pp_context_free(&ctx);
    pp_context_init(&ctx);
    rc = pp_add_macro(&ctx, "SEED", "1");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    while (ctx.macro_count < ctx.macro_capacity) {
      rc = pp_add_macro(&ctx, "M_CAP", "1");
      ASSERT_EQ(CDD_C_SUCCESS, rc);
    }
    write_to_file(test_defs, "#define ONE_MACRO\n");
    for (k = 3; k <= 7; ++k) {
      g_cdd_alloc_fail = k;
      (void)pp_scan_defines(&ctx, test_defs);
      g_cdd_alloc_fail = 0;
    }

    /* 3. get_dirname failure in pp_scan_includes */
    write_to_file(test_inc, "");
    g_cdd_strdup_fail = 1;
    rc = pp_scan_includes(test_inc, &ctx, test_scan_inc_cb, &count);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    pp_context_free(&ctx);
    remove(test_defs);
    remove(test_inc);
    PASS();
  }

  TEST test_pp_branch_coverage_maximizer(void) {
    struct PreprocessorContext ctx;
    struct MacroDef def;
    struct TokenList *tl = NULL;
    struct EmbedParams params;
    const char *test_file = "test_branch_cov.c";
    int count = 0;
    long val = 0;
    cdd_c_error_t rc;

    /* 1. pp_free_macro_def with various combinations */
    memset(&def, 0, sizeof(def));
    pp_free_macro_def(&def); /* all NULL */

    c_cdd_strdup("NAME", &def.name);
    c_cdd_strdup("VAL", &def.value);
    def.args = (char **)C_CDD_MALLOC(2 * sizeof(char *));
    c_cdd_strdup("arg1", &def.args[0]);
    def.args[1] = NULL;
    def.arg_count = 2;
    pp_free_macro_def(&def);

    /* 2. pp_context_free with NULL search_paths and macros */
    memset(&ctx, 0, sizeof(ctx));
    pp_context_free(&ctx);

    /* Context with search_path containing NULL entry */
    pp_context_init(&ctx);
    pp_add_search_path(&ctx, ".");
    C_CDD_FREE(ctx.search_paths[0]);
    ctx.search_paths[0] = NULL;
    pp_context_free(&ctx);

    /* 3. String literal with length < 2 in __has_include */
    pp_context_init(&ctx);
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_include(\"\")"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    (void)pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    free_token_list(tl);

    /* 4. Hex, octal, binary number literals */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "0x10 + 010 + 0b10 + 0B11"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(16 + 8 + 2 + 3, val);
    free_token_list(tl);

    /* 5. Scoped embed parameters vendor::limit, vendor::prefix, etc. */
    memset(&params, 0, sizeof(params));
    rc = tokenize(az_span_create_from_str(
                      (char *)(size_t) "vendor::limit(1) vendor::prefix(2) "
                                       "vendor::suffix(3) vendor::if_empty(4)"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    pp_embed_params_free(&params);
    free_token_list(tl);

    /* 6. Scan defines with # alone, #pragma, and #define without name */
    write_to_file(test_file, "#\n"
                             "#pragma once\n"
                             "#define\n"
                             "#define 123\n");
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* 7. Scan includes with # alone, and elif false */
    write_to_file(test_file, "#\n"
                             "#if 0\n"
                             "#elif 0\n"
                             "#elif 1\n"
                             "#endif\n");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    pp_context_free(&ctx);
    remove(test_file);
    PASS();
  }

  TEST test_pp_100_percent_coverage(void) {
    struct PreprocessorContext ctx;
    struct ExprState s;
    struct TokenList *tl = NULL;
    struct MacroDef def;
    char *out = NULL;
    long val = 0;
    int matched = 0;
    int count = 0;
    size_t i;
    cdd_c_error_t rc;
    const char *test_file = "test_pp_100_cov.tmp";

    /* 1. Context init failure and capacity growth in pp_add_macro_internal */
    g_cdd_pp_context_init_fail = 1;
    rc = pp_context_init(&ctx);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_pp_context_init_fail = 0;

    rc = pp_context_init(&ctx);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    memset(&def, 0, sizeof(def));
    def.name = C_CDD_STRDUP("MACRO0");
    def.value = C_CDD_STRDUP("42");
    rc = pp_add_macro_internal(&ctx, &def);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(16, ctx.macro_capacity);

    for (i = 1; i < 32; i++) {
      char buf[32];
#if defined(_MSC_VER)
      sprintf_s(buf, sizeof(buf), "MACRO%lu", (unsigned long)i);
#else
    sprintf(buf, "MACRO%lu", (unsigned long)i);
#endif
      memset(&def, 0, sizeof(def));
      def.name = C_CDD_STRDUP(buf);
      rc = pp_add_macro_internal(&ctx, &def);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
    }
    ASSERT_EQ(32, ctx.macro_capacity);
    ASSERT_EQ(32, ctx.macro_count);

    /* OOM on pp_add_macro_internal when growing beyond 32 */
    memset(&def, 0, sizeof(def));
    def.name = C_CDD_STRDUP("OOM_MACRO");
    g_cdd_alloc_fail = 1;
    rc = pp_add_macro_internal(&ctx, &def);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    C_CDD_FREE(def.name);

    /* 2. pp_file_exists failure hook in pp_resolve_path */
    g_cdd_pp_file_exists_fail = 1;
    rc = pp_resolve_path(&ctx, "current_dir", "header.h", 0, &out);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
    ASSERT_EQ(NULL, out);
    g_cdd_pp_file_exists_fail = 0;

    rc = pp_add_search_path(&ctx, "search_dir");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_pp_file_exists_fail = 1;
    rc = pp_resolve_path(&ctx, NULL, "header.h", 1, &out);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
    ASSERT_EQ(NULL, out);
    g_cdd_pp_file_exists_fail = 0;

    /* 3. Helper functions NULL argument checks and hooks */
    rc = pp_expr_skip_ws(NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    memset(&s, 0, sizeof(s));
    rc = pp_expr_skip_ws(&s);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 + 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;

    g_cdd_pp_skip_ws_fail = 1;
    rc = pp_expr_skip_ws(&s);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    rc = pp_expr_match(NULL, TOKEN_PLUS, &matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_expr_match(&s, TOKEN_PLUS, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 1;
    rc = pp_expr_match(&s, TOKEN_PLUS, &matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;

    g_cdd_pp_skip_ws_fail = 1;
    rc = pp_expr_match(&s, TOKEN_PLUS, &matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    rc = pp_preprocessor_peek(NULL, (enum TokenKind *)&matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_preprocessor_peek(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_peek_fail = 1;
    rc = pp_preprocessor_peek(&s, (enum TokenKind *)&matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_peek_fail = 0;

    rc = pp_is_defined_macro(NULL, NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_is_defined_macro_fail = 1;
    rc = pp_is_defined_macro(&ctx, &tl->tokens[0], &matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_is_defined_macro_fail = 0;

    g_cdd_fail_token_matches_string = 1;
    rc = pp_is_defined_macro(&ctx, &tl->tokens[0], &matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_token_matches_string = 0;

    free_token_list(tl);
    tl = NULL;

    /* 4. pp_handle_has_include_embed NULL and hook branches */
    rc = pp_handle_has_include_embed(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_handle_has_include_embed(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    rc = tokenize(az_span_create_from_str((char *)(size_t) "( \"file.h\" )"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_skip_ws_fail = 1;
    rc = pp_handle_has_include_embed(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_match_fail = 1;
    rc = pp_handle_has_include_embed(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 2;
    rc = pp_handle_has_include_embed(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 3;
    rc = pp_handle_has_include_embed(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_match_fail = 2;
    rc = pp_handle_has_include_embed(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 4;
    rc = pp_handle_has_include_embed(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_resolve_path_fail = 1;
    rc = pp_handle_has_include_embed(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
    g_cdd_pp_resolve_path_fail = 0;

    free_token_list(tl);
    tl = NULL;

    /* 5. pp_handle_has_c_attribute NULL and hook branches */
    rc = pp_handle_has_c_attribute(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_handle_has_c_attribute(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    rc = tokenize(az_span_create_from_str((char *)(size_t) "( deprecated )"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_skip_ws_fail = 1;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_match_fail = 1;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 2;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 3;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 4;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 5;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 6;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_match_fail = 2;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "( const )"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_fail_identify_keyword_or_id = 1;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_identify_keyword_or_id = 0;
    free_token_list(tl);
    tl = NULL;

    /* Empty parens, keyword as attr, scoped attr edge cases */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "( )"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, val);
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "( const )"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "( gnu :: 123 )"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "( gnu :: pure )"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_skip_ws_fail = 3;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 5;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_token_to_string_fail = 2;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_pp_token_to_string_fail = 0;
    free_token_list(tl);
    tl = NULL;

    /* 6. Primary parsing branches */
    rc = pp_parse_primary(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_primary(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_primary_fail = 1;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_primary_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "0b1011"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(11, val);
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "( 1 )"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_skip_ws_fail = 1;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    for (i = 1; i <= 25; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_primary(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_include ( \"a.h\" )"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_fail_token_matches_string = 1;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_token_matches_string = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_embed ( \"a.bin\" )"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_fail_token_matches_string = 2;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_token_matches_string = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "__has_c_attribute ( dep )"),
        &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_fail_token_matches_string = 3;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_token_matches_string = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "MACRO0"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    s.ctx = &ctx;
    g_cdd_fail_token_matches_string = 4;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_token_matches_string = 0;
    free_token_list(tl);
    tl = NULL;

    /* 7. Unary parsing branches */
    rc = pp_parse_unary(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_unary(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    rc = tokenize(az_span_create_from_str((char *)(size_t) "! 1"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_skip_ws_fail = 1;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_match_fail = 1;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "~ 1"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_match_fail = 2;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "- 1"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_match_fail = 3;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "+ 1"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_match_fail = 4;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "defined ( MACRO0 )"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_fail_token_matches_string = 1;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_token_matches_string = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 2;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_match_fail = 5;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 3;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 6;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_skip_ws_fail = 8;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_is_defined_macro_fail = 1;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_is_defined_macro_fail = 0;

    s.pos = 0;
    s.error = 0;
    g_cdd_pp_match_fail = 6;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;
    free_token_list(tl);
    tl = NULL;

    /* 8. Binary operators error percolation */
    rc = pp_parse_multiplicative(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_multiplicative(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 * 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 8; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_multiplicative(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_unary_fail = 1;
    rc = pp_parse_multiplicative(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_unary_fail = 0;
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_unary_fail = 2;
    rc = pp_parse_multiplicative(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_unary_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 / 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 8; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_multiplicative(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_unary_fail = 2;
    rc = pp_parse_multiplicative(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_unary_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 % 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 8; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_multiplicative(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_unary_fail = 2;
    rc = pp_parse_multiplicative(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_unary_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = pp_parse_additive(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_additive(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 + 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 10; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_additive(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_multiplicative_fail = 1;
    rc = pp_parse_additive(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_multiplicative_fail = 0;
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_multiplicative_fail = 2;
    rc = pp_parse_additive(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_multiplicative_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 - 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 10; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_additive(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    free_token_list(tl);
    tl = NULL;

    rc = pp_parse_shift(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_shift(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 << 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 12; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_shift(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_additive_fail = 1;
    rc = pp_parse_shift(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_additive_fail = 0;
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_additive_fail = 2;
    rc = pp_parse_shift(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_additive_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 >> 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 12; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_shift(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    free_token_list(tl);
    tl = NULL;

    rc = pp_parse_relational(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_relational(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 <= 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 14; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_relational(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_peek_fail = 1;
    rc = pp_parse_relational(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_peek_fail = 0;
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_shift_fail = 2;
    rc = pp_parse_relational(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_shift_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 >= 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 14; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_relational(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_shift_fail = 2;
    rc = pp_parse_relational(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_shift_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 < 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 14; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_relational(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_shift_fail = 2;
    rc = pp_parse_relational(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_shift_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 > 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 14; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_relational(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_shift_fail = 2;
    rc = pp_parse_relational(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_shift_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = pp_parse_equality(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_equality(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 == 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 14; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_equality(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_equality_fail = 1;
    rc = pp_parse_equality(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_equality_fail = 0;
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_relational_fail = 2;
    rc = pp_parse_equality(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_relational_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 != 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 14; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_equality(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    free_token_list(tl);
    tl = NULL;

    rc = pp_parse_logic_and(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_logic_and(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 && 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 15; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_logic_and(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    s.pos = 0;
    s.error = 0;
    g_cdd_pp_logic_and_fail = 1;
    rc = pp_parse_logic_and(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_logic_and_fail = 0;
    free_token_list(tl);
    tl = NULL;

    rc = pp_parse_logic_or(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_logic_or(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 || 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    for (i = 1; i <= 16; i++) {
      s.pos = 0;
      s.error = 0;
      g_cdd_pp_match_fail = (int)i;
      rc = pp_parse_logic_or(&s, &val);
      g_cdd_pp_match_fail = 0;
    }
    free_token_list(tl);
    tl = NULL;

    rc = pp_parse_expr(NULL, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = pp_parse_expr(&s, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* 9. Embed params edge cases and error hooks */
    {
      struct EmbedParams params;
      memset(&params, 0, sizeof(params));

      rc = tokenize(az_span_create_from_str((char *)(size_t) "const(1)"), &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      g_cdd_fail_identify_keyword_or_id = 1;
      rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      g_cdd_fail_identify_keyword_or_id = 0;
      free_token_list(tl);
      tl = NULL;

      rc = tokenize(az_span_create_from_str((char *)(size_t) "gnu::attr(1)"),
                    &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      g_cdd_pp_token_to_string_fail = 2;
      rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      g_cdd_pp_token_to_string_fail = 0;
      free_token_list(tl);
      tl = NULL;

      rc = tokenize(az_span_create_from_str((char *)(size_t) "limit(10)"), &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      g_cdd_pp_eval_expr_fail = 1;
      rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      g_cdd_pp_eval_expr_fail = 0;
      free_token_list(tl);
      tl = NULL;

      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "suffix(end) (nested)"),
          &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      pp_embed_params_free(&params);
      free_token_list(tl);
      tl = NULL;

      rc = tokenize(az_span_create_from_str((char *)(size_t) "limit"), &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      free_token_list(tl);
      tl = NULL;

      rc = tokenize(az_span_create_from_str((char *)(size_t) "gnu::123(1)"),
                    &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      free_token_list(tl);
      tl = NULL;

      rc = tokenize(az_span_create_from_str((char *)(size_t) "gnu::unknown(1)"),
                    &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_token_list(tl);
      tl = NULL;

      rc = tokenize(az_span_create_from_str((char *)(size_t) "gnu::"), &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      free_token_list(tl);
      tl = NULL;
    }

    /* 10. Scan defines: g_cdd_pp_scan_defines_fail and GNU variadic macro */
    g_cdd_pp_scan_defines_fail = 1;
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
    g_cdd_pp_scan_defines_fail = 0;

    write_to_file(test_file, "#define GNU_VAR(a...) a\n");
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* token_matches_string error on scan defines */
    write_to_file(test_file, "#define\n");
    g_cdd_fail_token_matches_string = 5;
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_token_matches_string = 0;

    /* 11. Scan includes directives and error hooks */
    write_to_file(test_file, "#if 0\n"
                             "#include \"ignored.h\"\n"
                             "#endif\n"
                             "#include <sys/stat.h\n"
                             "#ifdef FOO\n"
                             "#endif\n");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Error in pp_is_defined_macro during #ifdef */
    write_to_file(test_file, "#ifdef FOO\n#endif\n");
    g_cdd_pp_is_defined_macro_fail = 1;
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_is_defined_macro_fail = 0;

    /* System include with closing > */
    write_to_file(test_file, "#include <stdio.h>\n");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Error in C_CDD_MALLOC during #include raw_path */
    write_to_file(test_file, "#include \"header.h\"\n");
    for (i = 1; i <= 10; i++) {
      g_cdd_alloc_fail = (int)i;
      rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
      g_cdd_alloc_fail = 0;
    }

    /* Error in pp_resolve_path during #include */
    g_cdd_pp_resolve_path_fail = 1;
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
    g_cdd_pp_resolve_path_fail = 0;

    /* 12. Hook count = 2 branches for --hook == 0 */
    g_cdd_pp_file_exists_fail = 2;
    rc = pp_file_exists("path", &matched);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_file_exists("path", &matched);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
    g_cdd_pp_file_exists_fail = 0;

    g_cdd_pp_resolve_path_fail = 2;
    rc = pp_resolve_path(&ctx, "dir", "h.h", 0, &out);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_resolve_path(&ctx, "dir", "h.h", 0, &out);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
    g_cdd_pp_resolve_path_fail = 0;

    g_cdd_pp_context_init_fail = 2;
    rc = pp_context_init(&ctx);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_context_init(&ctx);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_pp_context_init_fail = 0;

    g_cdd_pp_scan_defines_fail = 2;
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
    g_cdd_pp_scan_defines_fail = 0;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "1 + 2"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;

    g_cdd_pp_skip_ws_fail = 2;
    rc = pp_expr_skip_ws(&s);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_expr_skip_ws(&s);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;

    g_cdd_pp_match_fail = 2;
    rc = pp_expr_match(&s, TOKEN_NUMBER_LITERAL, &matched);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_expr_match(&s, TOKEN_NUMBER_LITERAL, &matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_match_fail = 0;

    g_cdd_pp_peek_fail = 2;
    rc = pp_preprocessor_peek(&s, (enum TokenKind *)&matched);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_preprocessor_peek(&s, (enum TokenKind *)&matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_peek_fail = 0;

    g_cdd_pp_is_defined_macro_fail = 2;
    rc = pp_is_defined_macro(&ctx, &tl->tokens[0], &matched);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_is_defined_macro(&ctx, &tl->tokens[0], &matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_is_defined_macro_fail = 0;

    g_cdd_pp_primary_fail = 2;
    s.pos = 0;
    s.error = 0;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.pos = 0;
    s.error = 0;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_primary_fail = 0;

    g_cdd_pp_equality_fail = 2;
    s.pos = 0;
    s.error = 0;
    rc = pp_parse_equality(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.pos = 0;
    s.error = 0;
    rc = pp_parse_equality(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_equality_fail = 0;

    g_cdd_pp_logic_and_fail = 2;
    s.pos = 0;
    s.error = 0;
    rc = pp_parse_logic_and(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.pos = 0;
    s.error = 0;
    rc = pp_parse_logic_and(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_logic_and_fail = 0;

    g_cdd_fail_identify_keyword_or_id = 2;
    rc = identify_keyword_or_id((const uint8_t *)"auto", 4,
                                (enum TokenKind *)&matched);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = identify_keyword_or_id((const uint8_t *)"auto", 4,
                                (enum TokenKind *)&matched);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_identify_keyword_or_id = 0;

    /* pp_resolve_path with NULL ctx */
    rc = pp_resolve_path(NULL, "dir", "header.h", 0, &out);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_resolve_path(NULL, ".", "CMakeLists.txt", 0, &out);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    if (out) {
      C_CDD_FREE(out);
      out = NULL;
    }

    /* parse_primary with ctx == NULL, is_function_like, and value == NULL */
    s.ctx = NULL;
    s.pos = 0;
    s.error = 0;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    {
      struct PreprocessorContext macro_ctx;
      pp_context_init(&macro_ctx);
      memset(&def, 0, sizeof(def));
      def.name = C_CDD_STRDUP("FUNC_MACRO");
      def.is_function_like = 1;
      pp_add_macro_internal(&macro_ctx, &def);
      memset(&def, 0, sizeof(def));
      def.name = C_CDD_STRDUP("NULL_VAL_MACRO");
      def.value = NULL;
      pp_add_macro_internal(&macro_ctx, &def);

      s.ctx = &macro_ctx;
      free_token_list(tl);
      tl = NULL;

      rc =
          tokenize(az_span_create_from_str((char *)(size_t) "FUNC_MACRO"), &tl);
      s.tokens = tl;
      s.pos = 0;
      s.error = 0;
      s.end = tl->size;
      rc = pp_parse_primary(&s, &val);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_token_list(tl);
      tl = NULL;

      rc = tokenize(az_span_create_from_str((char *)(size_t) "NULL_VAL_MACRO"),
                    &tl);
      s.tokens = tl;
      s.pos = 0;
      s.error = 0;
      s.end = tl->size;
      rc = pp_parse_primary(&s, &val);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_token_list(tl);
      tl = NULL;
      pp_context_free(&macro_ctx);
    }

    /* Single colon edge cases for ( gnu : pure ) and embed gnu : attr(1) */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "( gnu : pure )"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_handle_has_c_attribute(&s, &val);
    free_token_list(tl);
    tl = NULL;

    {
      struct EmbedParams params;
      memset(&params, 0, sizeof(params));
      rc = tokenize(az_span_create_from_str((char *)(size_t) "gnu : attr(1)"),
                    &tl);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
      free_token_list(tl);
      tl = NULL;
    }

    /* Scan defines edge cases: trailing #, trailing #define, #define FOO at EOF
     */
    write_to_file(test_file, "#");
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    write_to_file(test_file, "#define");
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    write_to_file(test_file, "#define FOO");
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    write_to_file(test_file, "#define FOO(a)");
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* OOM on macro value allocation in scan_defines */
    write_to_file(test_file, "#define FOO 123\n");
    for (i = 1; i <= 6; i++) {
      g_cdd_alloc_fail = (int)i;
      rc = pp_scan_defines(&ctx, test_file);
      g_cdd_alloc_fail = 0;
    }

    /* Scan includes edge cases: # without cmd, directive at EOF, #ifdef without
     * id, #ifdef 123, #include without path, #include 123, #else after
     * satisfied */
    write_to_file(test_file, "#\n"
                             "#include\n"
                             "#include 123\n"
                             "#ifdef\n"
                             "#ifdef 123\n"
                             "#endif\n"
                             "#endif\n"
                             "#if 1\n"
                             "#elif 1\n"
                             "#else\n"
                             "#endif\n");
    rc = pp_scan_includes(test_file, &ctx, NULL, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Scan includes embed with params */
    write_to_file(test_file, "#embed \"file.bin\" limit(10)\n");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Trailing # at end of file */
    write_to_file(test_file, "#");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Trailing #include at end of file */
    write_to_file(test_file, "#include");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* pp_add_macro_internal NULL def */
    rc = pp_add_macro_internal(&ctx, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* #define FOO(a at EOF */
    write_to_file(test_file, "#define FOO(a");
    rc = pp_scan_defines(&ctx, test_file);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* __has_c_attribute( at EOF */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "("), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_handle_has_c_attribute(&s, &val);
    free_token_list(tl);
    tl = NULL;

    /* ( gnu :: at EOF */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "( gnu ::"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_handle_has_c_attribute(&s, &val);
    free_token_list(tl);
    tl = NULL;

    /* Hex literal 0x10 in parse_primary */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "0x10"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(16, val);
    free_token_list(tl);
    tl = NULL;

    /* Non-ident, non-keyword token in parse_primary */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "+"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);
    tl = NULL;

    /* Keyword token in parse_primary */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "const"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, val);
    free_token_list(tl);
    tl = NULL;

    /* defined at EOF */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "defined"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);
    tl = NULL;

    /* Empty parens in __has_c_attribute with skip_ws fail at line 877 */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "( )"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_skip_ws_fail = 3;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;
    free_token_list(tl);
    tl = NULL;

    /* Double colon without scope before it: ( :: pure ) */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "( :: pure )"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_handle_has_c_attribute(&s, &val);
    free_token_list(tl);
    tl = NULL;

    rc = tokenize(az_span_create_from_str((char *)(size_t) "( :: pure )"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    g_cdd_pp_skip_ws_fail = 3;
    rc = pp_handle_has_c_attribute(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_skip_ws_fail = 0;
    free_token_list(tl);
    tl = NULL;

    /* Decimal literal length > 2 not starting with 0 in parse_primary */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "123"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(123, val);
    free_token_list(tl);
    tl = NULL;

    /* Semicolon token in parse_primary */
    rc = tokenize(az_span_create_from_str((char *)(size_t) ";"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);
    tl = NULL;

    /* Identifier in parse_primary when ctx is NULL */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "UNKNOWN_MACRO"),
                  &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    s.ctx = NULL;
    rc = pp_parse_primary(&s, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, val);
    free_token_list(tl);
    tl = NULL;

    /* defined followed by non-identifier */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "defined 123"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    s.tokens = tl;
    s.pos = 0;
    s.error = 0;
    s.end = tl->size;
    rc = pp_parse_unary(&s, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    free_token_list(tl);
    tl = NULL;

    /* g_cdd_pp_eval_expr_fail = 2 for --hook == 0 */
    rc = tokenize(az_span_create_from_str((char *)(size_t) "1"), &tl);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_pp_eval_expr_fail = 2;
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_pp_eval_expr_fail = 0;
    free_token_list(tl);
    tl = NULL;

    /* Scan includes with cb == NULL on existing file */
    write_to_file("test_inc_exist.h", "\n");
    write_to_file(test_file, "#include \"test_inc_exist.h\"\n");
    rc = pp_scan_includes(test_file, &ctx, NULL, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Scan includes embed without parameters after filename */
    write_to_file("test_embed.bin", "\n");
    write_to_file(test_file, "#embed \"test_embed.bin\"\n");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    remove("test_embed.bin");

    /* Scan includes callback abort on include */
    write_to_file(test_file, "#include \"test_inc_exist.h\"\n");
    rc = pp_scan_includes(test_file, &ctx, test_scan_inc_abort_cb, &count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    remove("test_inc_exist.h");

    pp_context_free(&ctx);
    remove(test_file);
    PASS();
  }

  SUITE(preprocessor_internals_suite) {
    RUN_TEST(test_pp_100_percent_coverage);
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
