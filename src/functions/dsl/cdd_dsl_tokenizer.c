/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "dsl/cdd_dsl_tokenizer.h"
#include "c_cdd/memory.h"
#include "../ffi/cdd_ffi_ir_internal.h"
#include "c_cdd/safe_crt.h"
#include "functions/ffi/cdd_ffi_ir_internal.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct {
  const char *source;
  size_t current_pos;
  size_t line;
  size_t col;
} tokenizer_state_t;

static cdd_c_error_t advance(tokenizer_state_t *state) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_ffi_extractor_alloc_fail;
  if (g_ffi_extractor_alloc_fail && --g_ffi_extractor_alloc_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (state->source[state->current_pos] == '\n') {
    state->line++;
    state->col = 1;
  } else {
    state->col++;
  }
  state->current_pos++;
  return CDD_C_SUCCESS;
}

static int is_at_end(const tokenizer_state_t *state) {
  return state->source[state->current_pos] == '\0';
}

static cdd_c_error_t add_token(cdd_dsl_token_list_t *list,
                               cdd_dsl_token_kind_t kind,
                               const tokenizer_state_t *state, size_t start_pos,
                               size_t length) {
  if (list->count >= list->capacity) {
    size_t new_cap = list->capacity == 0 ? 32 : list->capacity * 2;
    cdd_dsl_token_t *new_tokens = (cdd_dsl_token_t *)CDD_REALLOC(
        list->tokens, new_cap * sizeof(cdd_dsl_token_t));
    if (!new_tokens)
      return CDD_C_ERROR_MEMORY;
    list->tokens = new_tokens;
    list->capacity = new_cap;
  }

  list->tokens[list->count].kind = kind;
  list->tokens[list->count].start = state->source + start_pos;
  list->tokens[list->count].length = length;

  list->tokens[list->count].text_copy = (char *)CDD_CALLOC(1, length + 1);
  if (!list->tokens[list->count].text_copy)
    return CDD_C_ERROR_MEMORY;
  memcpy(list->tokens[list->count].text_copy, state->source + start_pos,
         length);
  list->tokens[list->count].text_copy[length] = '\0';

  /* Estimate start col based on current pos */
  list->tokens[list->count].line = state->line;
  list->tokens[list->count].col = state->col > length ? state->col - length : 1;

  list->count++;
  return CDD_C_SUCCESS;
}

static cdd_dsl_token_kind_t match_keyword(const char *text, size_t length) {
  if (length == 4 && strncmp(text, "func", 4) == 0)
    return CDD_DSL_TOKEN_KW_FUNC;
  if (length == 6 && strncmp(text, "struct", 6) == 0)
    return CDD_DSL_TOKEN_KW_STRUCT;
  if (length == 4 && strncmp(text, "enum", 4) == 0)
    return CDD_DSL_TOKEN_KW_ENUM;
  if (length == 5 && strncmp(text, "union", 5) == 0)
    return CDD_DSL_TOKEN_KW_UNION;
  if (length == 7 && strncmp(text, "typedef", 7) == 0)
    return CDD_DSL_TOKEN_KW_TYPEDEF;
  if (length == 9 && strncmp(text, "namespace", 9) == 0)
    return CDD_DSL_TOKEN_KW_NAMESPACE;
  if (length == 11 && strncmp(text, "macro_const", 11) == 0)
    return CDD_DSL_TOKEN_KW_MACRO_CONST;
  if (length == 10 && strncmp(text, "macro_expr", 10) == 0)
    return CDD_DSL_TOKEN_KW_MACRO_EXPR;
  if (length == 10 && strncmp(text, "macro_stmt", 10) == 0)
    return CDD_DSL_TOKEN_KW_MACRO_STMT;
  if (length == 4 && strncmp(text, "body", 4) == 0)
    return CDD_DSL_TOKEN_KW_BODY;
  if (length == 7 && strncmp(text, "virtual", 7) == 0)
    return CDD_DSL_TOKEN_KW_VIRTUAL;
  if (length == 4 && strncmp(text, "pure", 4) == 0)
    return CDD_DSL_TOKEN_KW_PURE;
  if (length == 2 && strncmp(text, "in", 2) == 0)
    return CDD_DSL_TOKEN_KW_IN;
  if (length == 3 && strncmp(text, "out", 3) == 0)
    return CDD_DSL_TOKEN_KW_OUT;
  if (length == 5 && strncmp(text, "inout", 5) == 0)
    return CDD_DSL_TOKEN_KW_INOUT;
  if (length == 3 && strncmp(text, "len", 3) == 0)
    return CDD_DSL_TOKEN_KW_LEN;

  if (length == 4 && strncmp(text, "void", 4) == 0)
    return CDD_DSL_TOKEN_TYPE_VOID;
  if (length == 2 && strncmp(text, "i8", 2) == 0)
    return CDD_DSL_TOKEN_TYPE_I8;
  if (length == 2 && strncmp(text, "u8", 2) == 0)
    return CDD_DSL_TOKEN_TYPE_U8;
  if (length == 3 && strncmp(text, "i16", 3) == 0)
    return CDD_DSL_TOKEN_TYPE_I16;
  if (length == 3 && strncmp(text, "u16", 3) == 0)
    return CDD_DSL_TOKEN_TYPE_U16;
  if (length == 3 && strncmp(text, "i32", 3) == 0)
    return CDD_DSL_TOKEN_TYPE_I32;
  if (length == 3 && strncmp(text, "u32", 3) == 0)
    return CDD_DSL_TOKEN_TYPE_U32;
  if (length == 3 && strncmp(text, "i64", 3) == 0)
    return CDD_DSL_TOKEN_TYPE_I64;
  if (length == 3 && strncmp(text, "u64", 3) == 0)
    return CDD_DSL_TOKEN_TYPE_U64;
  if (length == 3 && strncmp(text, "f32", 3) == 0)
    return CDD_DSL_TOKEN_TYPE_F32;
  if (length == 3 && strncmp(text, "f64", 3) == 0)
    return CDD_DSL_TOKEN_TYPE_F64;
  if (length == 4 && strncmp(text, "bool", 4) == 0)
    return CDD_DSL_TOKEN_TYPE_BOOL;
  if (length == 6 && strncmp(text, "size_t", 6) == 0)
    return CDD_DSL_TOKEN_TYPE_SIZE_T;
  if (length == 5 && strncmp(text, "usize", 5) == 0)
    return CDD_DSL_TOKEN_TYPE_USIZE;
  if (length == 5 && strncmp(text, "isize", 5) == 0)
    return CDD_DSL_TOKEN_TYPE_ISIZE;
  if (length == 4 && strncmp(text, "char", 4) == 0)
    return CDD_DSL_TOKEN_TYPE_CHAR;

  return CDD_DSL_TOKEN_IDENTIFIER;
}

C_CDD_EXPORT void cdd_dsl_token_list_free(cdd_dsl_token_list_t *list) {
  if (list) {
    size_t i;
    for (i = 0; i < list->count; i++) {
      if (list->tokens[i].text_copy) {
        free(list->tokens[i].text_copy);
      }
    }
    if (list->tokens)
      free(list->tokens);
    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
  }
}

C_CDD_EXPORT cdd_c_error_t cdd_dsl_tokenize(const char *source,
                                            cdd_dsl_token_list_t *out_tokens) {
  tokenizer_state_t state;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!source || !out_tokens)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(&state, 0, sizeof(state));
  state.source = source;
  state.line = 1;
  state.col = 1;
  out_tokens->tokens = NULL;
  out_tokens->count = 0;
  out_tokens->capacity = 0;

  while (!is_at_end(&state)) {
    char c = state.source[state.current_pos];
    size_t start_pos = state.current_pos;

    if (isspace(c)) {
      while (!is_at_end(&state) && isspace(state.source[state.current_pos])) {
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_WHITESPACE, &state, start_pos,
                     state.current_pos - start_pos);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    if (c == '/' && state.source[state.current_pos + 1] == '/') {
      while (!is_at_end(&state) && state.source[state.current_pos] != '\n') {
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_COMMENT_LINE, &state, start_pos,
                     state.current_pos - start_pos);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    if (c == '/' && state.source[state.current_pos + 1] == '*') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } /* / */
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } /* * */
      while (!is_at_end(&state)) {
        if (state.source[state.current_pos] == '*' &&
            state.source[state.current_pos + 1] == '/') {
          {
            rc = advance(&state);
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
          {
            rc = advance(&state);
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
          break;
        }
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_COMMENT_BLOCK, &state, start_pos,
                     state.current_pos - start_pos);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    if (c == '#' && state.source[state.current_pos + 1] == '!') {
      if (strncmp(state.source + state.current_pos, "#!ws(", 5) == 0) {
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = add_token(out_tokens, CDD_DSL_TOKEN_ESCAPE_WS, &state, start_pos,
                       5);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        continue;
      }
      if (strncmp(state.source + state.current_pos, "#!comment(", 10) == 0) {
        int i;
        for (i = 0; i < 10; i++) {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = add_token(out_tokens, CDD_DSL_TOKEN_ESCAPE_COMMENT, &state,
                       start_pos, 10);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        continue;
      }
    }

    if (c == '@') {
      if (strncmp(state.source + state.current_pos, "@expr", 5) == 0) {
        int i;
        for (i = 0; i < 5; i++) {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = add_token(out_tokens, CDD_DSL_TOKEN_MACRO_INVOKE_EXPR, &state,
                       start_pos, 5);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        continue;
      }
      if (strncmp(state.source + state.current_pos, "@stmt", 5) == 0) {
        int i;
        for (i = 0; i < 5; i++) {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = add_token(out_tokens, CDD_DSL_TOKEN_MACRO_INVOKE_STMT, &state,
                       start_pos, 5);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        continue;
      }
    }

    if (c == '%' && state.source[state.current_pos + 1] == '{') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_RAW_START, &state, start_pos, 2);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      start_pos = state.current_pos;
      while (!is_at_end(&state)) {
        if (state.source[state.current_pos] == '}' &&
            state.source[state.current_pos + 1] == '%') {
          break;
        }
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_RAW_TEXT, &state, start_pos,
                     state.current_pos - start_pos);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      if (!is_at_end(&state)) {
        start_pos = state.current_pos;
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = add_token(out_tokens, CDD_DSL_TOKEN_RAW_END, &state, start_pos, 2);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      continue;
    }

    if (c == '"') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      while (!is_at_end(&state) && state.source[state.current_pos] != '"') {
        if (state.source[state.current_pos] == '\\') {
          {
            rc = advance(&state);
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
          if (is_at_end(&state))
            break;
        }
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      if (!is_at_end(&state)) {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } /* closing quote */
      rc = add_token(out_tokens, CDD_DSL_TOKEN_STRING, &state, start_pos,
                     state.current_pos - start_pos);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    if (c == '{') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_LBRACE, &state, start_pos, 1);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }
    if (c == '}') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_RBRACE, &state, start_pos, 1);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }
    if (c == '(') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_LPAREN, &state, start_pos, 1);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }
    if (c == ')') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_RPAREN, &state, start_pos, 1);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }
    if (c == '[') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_LBRACKET, &state, start_pos, 1);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }
    if (c == ']') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_RBRACKET, &state, start_pos, 1);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }
    if (c == ';') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_SEMICOLON, &state, start_pos, 1);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }
    if (c == ',') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_COMMA, &state, start_pos, 1);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }
    if (c == '=') {
      {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_EQUALS, &state, start_pos, 1);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    if (c == '*' && strncmp(state.source + state.current_pos, "*mut", 4) == 0) {
      int i;
      for (i = 0; i < 4; i++) {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_TYPE_MUT_PTR, &state, start_pos,
                     4);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    if (c == '*' &&
        strncmp(state.source + state.current_pos, "*const", 6) == 0) {
      int i;
      for (i = 0; i < 6; i++) {
        rc = advance(&state);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_TYPE_CONST_PTR, &state,
                     start_pos, 6);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    if (isalpha(c) || c == '_') {
      while (!is_at_end(&state) && (isalnum(state.source[state.current_pos]) ||
                                    state.source[state.current_pos] == '_')) {
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      rc = add_token(out_tokens,
                     match_keyword(state.source + start_pos,
                                   state.current_pos - start_pos),
                     &state, start_pos, state.current_pos - start_pos);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    if (isdigit(c) ||
        (c == '\'' && isdigit(state.source[state.current_pos + 1]))) {
      while (!is_at_end(&state) && (isalnum(state.source[state.current_pos]) ||
                                    state.source[state.current_pos] == '\'' ||
                                    state.source[state.current_pos] == '.')) {
        {
          rc = advance(&state);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      rc = add_token(out_tokens, CDD_DSL_TOKEN_NUMBER, &state, start_pos,
                     state.current_pos - start_pos);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    {
      rc = advance(&state);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    rc = add_token(out_tokens, CDD_DSL_TOKEN_UNKNOWN, &state, start_pos, 1);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = add_token(out_tokens, CDD_DSL_TOKEN_EOF, &state, state.current_pos, 0);

cleanup:
  if (rc != CDD_C_SUCCESS) {
    cdd_dsl_token_list_free(out_tokens);
  }
  return rc;
}
