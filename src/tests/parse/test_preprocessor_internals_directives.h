/**
 * @file test_preprocessor_internals_directives.h
 * @brief Unit tests for preprocessor internals 100% coverage (embed &
 * directives).
 */

#ifndef TEST_PREPROCESSOR_INTERNALS_DIRECTIVES_H
#define TEST_PREPROCESSOR_INTERNALS_DIRECTIVES_H

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

TEST test_pp_100_cov_directives(void) {
  struct PreprocessorContext ctx;
  struct ExprState s;
  struct TokenList *tl = NULL;
  struct MacroDef def;
  char *out = NULL;
  long val = 0;
  int matched = 0;
  int count = 0;
  size_t i;
  cdd_c_error_t rc = 0;
  const char *test_file = "test_pp_100_cov.tmp";

  rc = pp_context_init(&ctx);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
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

    rc =
        tokenize(az_span_create_from_str((char *)(size_t) "gnu::attr(1)"), &tl);
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
        az_span_create_from_str((char *)(size_t) "suffix(end) (nested)"), &tl);
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

    rc = tokenize(az_span_create_from_str((char *)(size_t) "gnu::123(1)"), &tl);
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

    rc = tokenize(az_span_create_from_str((char *)(size_t) "FUNC_MACRO"), &tl);
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
  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "( gnu : pure )"), &tl);
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

  s.ctx = NULL;
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
  rc = tokenize(az_span_create_from_str((char *)(size_t) "UNKNOWN_MACRO"), &tl);
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_PREPROCESSOR_INTERNALS_DIRECTIVES_H */
