/**
 * @file test_preprocessor_internals_branches.h
 * @brief Unit tests for preprocessor branch coverage.
 */

#ifndef TEST_PREPROCESSOR_INTERNALS_BRANCHES_H
#define TEST_PREPROCESSOR_INTERNALS_BRANCHES_H

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

#ifndef CDD_TEST_PP_SCAN_CB_DEFINED
#define CDD_TEST_PP_SCAN_CB_DEFINED
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
#endif /* CDD_TEST_PP_SCAN_CB_DEFINED */
TEST test_pp_scan_includes_directives(void) {
  struct PreprocessorContext ctx;
  const char *test_file = "test_pp_scan_cond.c";
  const char *inc_file = "test_inc_target.h";
  int count = 0;
  cdd_c_error_t rc = 0;

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
  cdd_c_error_t rc = 0;

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
  cdd_c_error_t rc = 0;

  pp_context_init(&ctx);
  memset(&params, 0, sizeof(params));

  /* Scoped param missing LPAREN */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "vendor :: custom 10"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  /* Scoped param unbalanced parens */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "vendor :: custom ((10)"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);

  /* limit with syntax error in expression */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "limit((1 +))"), &tl);
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
  cdd_c_error_t rc = 0;

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
  cdd_c_error_t rc = 0;
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
  cdd_c_error_t rc = 0;

  write_to_file(test_file, "#define CR_VAL 123\r\n"
                           "#define SPACES_VAL    \n");

  pp_context_init(&ctx);
  rc = pp_scan_defines(&ctx, test_file);
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
  cdd_c_error_t rc = 0;

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
  cdd_c_error_t rc = 0;

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
  cdd_c_error_t rc = 0;

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
  cdd_c_error_t rc = 0;

  pp_context_init(&ctx);

  /* Multi-token with trailing CR and spaces, and empty spaces */
  write_to_file(test_file, "#define CR_MACRO 100 /* c */ \r\n"
                           "#define EMPTY_TRIM   /* c */\n"
                           "#define FN_OOM(a, b) a + b\n");
  rc = pp_scan_defines(&ctx, test_file);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OOM tests for pp_scan_defines args and macro addition */
  write_to_file(test_file, "#define FN_ARGS(a, b) a\n");
  g_cdd_alloc_fail = 5;
  rc = pp_scan_defines(&ctx, test_file);
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 6;
  rc = pp_scan_defines(&ctx, test_file);
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 7;
  rc = pp_scan_defines(&ctx, test_file);
  g_cdd_alloc_fail = 0;

  pp_context_free(&ctx);
  remove(test_file);
  PASS();
}

TEST test_pp_eval_oom_and_peek_eof(void) {
  struct PreprocessorContext ctx;
  struct TokenList *tl = NULL;
  long val = 0;
  cdd_c_error_t rc = 0;

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
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "__has_c_attribute(nodiscard)"),
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
  cdd_c_error_t rc = 0;

  pp_context_init(&ctx);
  memset(&params, 0, sizeof(params));

  /* Keyword param and trailing whitespace */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "inline(10)   "), &tl);
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
  rc = tokenize(az_span_create_from_str((char *)(size_t) "vendor::custom(10)"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_alloc_fail = 2;
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;
  free_token_list(tl);

  /* OOM on prefix */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "prefix(\"A\")"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_alloc_fail = 2;
  rc = pp_parse_embed_params(tl, 0, tl->size, &ctx, &params);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;
  free_token_list(tl);

  /* OOM on suffix */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "suffix(\"A\")"), &tl);
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
  cdd_c_error_t rc = 0;

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
  cdd_c_error_t rc = 0;

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
    rc = pp_scan_defines(&ctx, test_defs);
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
  cdd_c_error_t rc = 0;

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
  rc = tokenize(az_span_create_from_str((char *)(size_t) "__has_include(\"\")"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = pp_eval_expression(tl, 0, tl->size, &ctx, &val);
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_PREPROCESSOR_INTERNALS_BRANCHES_H */
