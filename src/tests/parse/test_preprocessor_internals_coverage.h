/**
 * @file test_preprocessor_internals_coverage.h
 * @brief Unit tests for preprocessor internals 100% coverage (expressions &
 * builtins).
 */

#ifndef TEST_PREPROCESSOR_INTERNALS_COVERAGE_H
#define TEST_PREPROCESSOR_INTERNALS_COVERAGE_H

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

TEST test_pp_100_percent_coverage(void) {
  struct PreprocessorContext ctx;
  struct ExprState s;
  struct TokenList *tl = NULL;
  struct MacroDef def;
  char *out = NULL;
  long val = 0;
  int matched = 0;
  size_t i;
  cdd_c_error_t rc;
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

  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "( \"file.h\" )"), &tl);
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

  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "( deprecated )"), &tl);
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

  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "( gnu :: 123 )"), &tl);
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

  rc = tokenize(az_span_create_from_str((char *)(size_t) "defined ( MACRO0 )"),
                &tl);
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

  pp_context_free(&ctx);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_PREPROCESSOR_INTERNALS_COVERAGE_H */
