/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "dsl/cdd_dsl_parser.h"
#include "c_cdd/memory.h"
#include "../ffi/cdd_ffi_ir_internal.h"
#include "c_cdd/safe_crt.h"
#include "functions/ffi/cdd_ffi_ir_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct {
  const cdd_dsl_token_list_t *tokens;
  size_t current;
  cdd_ffi_trivia_t *pending_trivia;
} parser_state_t;

static cdd_c_error_t peek_raw(const parser_state_t *state,
                              cdd_dsl_token_t *out_tok) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_ffi_extractor_alloc_fail;
  if (g_ffi_extractor_alloc_fail && --g_ffi_extractor_alloc_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (state->current >= state->tokens->count) {
    *out_tok = state->tokens->tokens[state->tokens->count - 1];
    return CDD_C_SUCCESS;
  }
  *out_tok = state->tokens->tokens[state->current];
  return CDD_C_SUCCESS;
}

static cdd_c_error_t collect_trivia(parser_state_t *state);

static cdd_c_error_t peek(parser_state_t *state, cdd_dsl_token_t *out_tok) {
  cdd_c_error_t rc;
  rc = collect_trivia(state);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return peek_raw(state, out_tok);
}

static cdd_c_error_t advance_raw(parser_state_t *state,
                                 cdd_dsl_token_t *out_tok) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_ffi_extractor_alloc_fail;
  if (g_ffi_extractor_alloc_fail && --g_ffi_extractor_alloc_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  rc = peek_raw(state, out_tok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (out_tok->kind != CDD_DSL_TOKEN_EOF) {
    state->current++;
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t advance(parser_state_t *state, cdd_dsl_token_t *out_tok) {
  cdd_c_error_t rc;
  rc = peek_raw(state, out_tok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (out_tok->kind != CDD_DSL_TOKEN_EOF) {
    state->current++;
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t match(parser_state_t *state, cdd_dsl_token_kind_t kind,
                           int *out_match) {
  cdd_dsl_token_t tok;
  cdd_c_error_t rc;
  rc = peek(state, &tok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (tok.kind == kind) {
    cdd_dsl_token_t advanced;
    rc = advance_raw(state, &advanced);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_match = 1;
    return CDD_C_SUCCESS;
  }
  *out_match = 0;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t collect_trivia(parser_state_t *state) {
  cdd_dsl_token_t tok;
  cdd_c_error_t rc;
  rc = peek_raw(state, &tok);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (tok.kind == CDD_DSL_TOKEN_WHITESPACE ||
         tok.kind == CDD_DSL_TOKEN_COMMENT_LINE ||
         tok.kind == CDD_DSL_TOKEN_COMMENT_BLOCK ||
         tok.kind == CDD_DSL_TOKEN_ESCAPE_WS ||
         tok.kind == CDD_DSL_TOKEN_ESCAPE_COMMENT) {
    cdd_ffi_trivia_t *node;
    cdd_dsl_token_t advanced;
    rc = advance_raw(state, &advanced);
    if (rc != CDD_C_SUCCESS)
      return rc;

    node = (cdd_ffi_trivia_t *)CDD_CALLOC(1, sizeof(cdd_ffi_trivia_t));
    if (!node)
      return CDD_C_ERROR_MEMORY;

    if (tok.kind == CDD_DSL_TOKEN_WHITESPACE ||
        tok.kind == CDD_DSL_TOKEN_ESCAPE_WS) {
      node->kind = CDD_FFI_TRIVIA_WHITESPACE;
    } else if (tok.kind == CDD_DSL_TOKEN_COMMENT_LINE) {
      node->kind = CDD_FFI_TRIVIA_COMMENT_LINE;
    } else {
      node->kind = CDD_FFI_TRIVIA_COMMENT_BLOCK;
    }

    if (tok.kind == CDD_DSL_TOKEN_ESCAPE_WS ||
        tok.kind == CDD_DSL_TOKEN_ESCAPE_COMMENT) {
      cdd_dsl_token_t p_tok;
      rc = peek(state, &p_tok);
      if (rc != CDD_C_SUCCESS) {
        free(node);
        return rc;
      }
      if (p_tok.kind == CDD_DSL_TOKEN_STRING) {
        cdd_dsl_token_t s;
        rc = advance_raw(state, &s);
        if (rc != CDD_C_SUCCESS) {
          free(node);
          return rc;
        }
        if (s.length >= 2) {
          node->text = (char *)CDD_CALLOC(1, s.length - 1);
          if (!node->text) {
            free(node);
            return CDD_C_ERROR_MEMORY;
          }
          memcpy(node->text, s.text_copy + 1, s.length - 2);
        }
      }
    } else {
      node->text = CDD_STRDUP(tok.text_copy);
      if (!node->text) {
        free(node);
        return CDD_C_ERROR_MEMORY;
      }
    }

    if (!state->pending_trivia) {
      state->pending_trivia = node;
    } else {
      cdd_ffi_trivia_t *tail = state->pending_trivia;
      while (tail->next)
        tail = tail->next;
      tail->next = node;
    }

    rc = peek_raw(state, &tok);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t take_pending_trivia(parser_state_t *state,
                                         cdd_ffi_trivia_t **out_trivia) {
  *out_trivia = state->pending_trivia;
  state->pending_trivia = NULL;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_number(parser_state_t *state,
                                  struct cdd_ffi_number_t **out_num) {
  struct cdd_ffi_number_t *num;
  cdd_dsl_token_t tok;
  cdd_c_error_t rc;

  rc = peek(state, &tok);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (tok.kind != CDD_DSL_TOKEN_NUMBER) {
    return CDD_C_ERROR_PARSE;
  }
  rc = advance_raw(state, &tok);
  if (rc != CDD_C_SUCCESS)
    return rc;

  num =
      (struct cdd_ffi_number_t *)CDD_CALLOC(1, sizeof(struct cdd_ffi_number_t));
  if (!num)
    return CDD_C_ERROR_MEMORY;

  num->raw_spelling = CDD_STRDUP(tok.text_copy);
  if (!num->raw_spelling) {
    free(num);
    return CDD_C_ERROR_MEMORY;
  }

  if (tok.length >= 2 && tok.text_copy[0] == '0') {
    if (tok.text_copy[1] == 'x' || tok.text_copy[1] == 'X')
      num->prefix = CDD_FFI_NUM_PREFIX_HEX;
    else if (tok.text_copy[1] == 'b' || tok.text_copy[1] == 'B')
      num->prefix = CDD_FFI_NUM_PREFIX_BINARY;
    else
      num->prefix = CDD_FFI_NUM_PREFIX_OCTAL;
  } else {
    num->prefix = CDD_FFI_NUM_PREFIX_NONE;
  }

  if (strchr(tok.text_copy, 39)) { /* 39 is single quote */
    num->has_separators = 1;
  }

  *out_num = num;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_type(parser_state_t *state,
                                cdd_ffi_type_t *out_type) {
  cdd_c_error_t rc;
  int is_match;
  cdd_dsl_token_t tok;

  memset(out_type, 0, sizeof(*out_type));

  while (1) {
    rc = match(state, CDD_DSL_TOKEN_TYPE_MUT_PTR, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match) {
      out_type->pointer_depth++;
      continue;
    }

    rc = match(state, CDD_DSL_TOKEN_TYPE_CONST_PTR, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match) {
      out_type->pointer_depth++;
      continue;
    }

    break;
  }

  rc = match(state, CDD_DSL_TOKEN_LBRACKET, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (is_match) {
    rc = peek(state, &tok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (tok.kind == CDD_DSL_TOKEN_NUMBER) {
      out_type->array_size = atol(tok.text_copy);
      rc = advance_raw(state, &tok);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      out_type->array_size = -1;
    }
    rc = match(state, CDD_DSL_TOKEN_RBRACKET, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!is_match)
      return CDD_C_ERROR_PARSE;
  } else {
    out_type->array_size = 0;
  }

  rc = match(state, CDD_DSL_TOKEN_TYPE_I32, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_match) {
    out_type->kind = CDD_FFI_KIND_INT32;
  } else {
    rc = match(state, CDD_DSL_TOKEN_TYPE_U8, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match) {
      out_type->kind = CDD_FFI_KIND_UINT8;
    } else {
      rc = match(state, CDD_DSL_TOKEN_TYPE_VOID, &is_match);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (is_match) {
        out_type->kind = CDD_FFI_KIND_VOID;
      } else {
        rc = peek(state, &tok);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (tok.kind == CDD_DSL_TOKEN_IDENTIFIER) {
          out_type->kind = CDD_FFI_KIND_STRUCT_REF;
          rc = advance(state, &tok);
          if (rc != CDD_C_SUCCESS)
            return rc;
          out_type->ref_name = CDD_STRDUP(tok.text_copy);
          if (!out_type->ref_name)
            return CDD_C_ERROR_MEMORY;
        } else {
          return CDD_C_ERROR_PARSE;
        }
      }
    }
  }

  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_field(parser_state_t *state,
                                 cdd_ffi_field_t *out_field) {
  cdd_c_error_t rc;
  int is_match;
  cdd_dsl_token_t tok;

  memset(out_field, 0, sizeof(*out_field));

  /* Attributes */
  rc = match(state, CDD_DSL_TOKEN_LBRACKET, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (is_match) {
    while (1) {
      rc = match(state, CDD_DSL_TOKEN_RBRACKET, &is_match);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (is_match)
        break;

      rc = peek(state, &tok);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (tok.kind == CDD_DSL_TOKEN_EOF)
        break;

      rc = match(state, CDD_DSL_TOKEN_KW_IN, &is_match);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (is_match) {
        out_field->intent = CDD_FFI_INTENT_IN;
      } else {
        rc = match(state, CDD_DSL_TOKEN_KW_OUT, &is_match);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_match) {
          out_field->intent = CDD_FFI_INTENT_OUT;
        } else {
          rc = match(state, CDD_DSL_TOKEN_KW_INOUT, &is_match);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (is_match) {
            out_field->intent = CDD_FFI_INTENT_INOUT;
          } else {
            rc = match(state, CDD_DSL_TOKEN_KW_LEN, &is_match);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (is_match) {
              rc = match(state, CDD_DSL_TOKEN_EQUALS, &is_match);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (!is_match)
                return CDD_C_ERROR_PARSE;

              rc = peek(state, &tok);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (tok.kind == CDD_DSL_TOKEN_IDENTIFIER) {
                rc = advance(state, &tok);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                out_field->array_length_ref = CDD_STRDUP(tok.text_copy);
                if (!out_field->array_length_ref)
                  return CDD_C_ERROR_MEMORY;
              }
            } else {
              rc = advance_raw(state, &tok); /* skip unknown attr */
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
        }
      }
      rc = match(state, CDD_DSL_TOKEN_COMMA, &is_match);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  rc = parse_type(state, &out_field->type);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = peek(state, &tok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (tok.kind == CDD_DSL_TOKEN_IDENTIFIER) {
    rc = advance(state, &tok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    out_field->name = CDD_STRDUP(tok.text_copy);
    if (!out_field->name)
      return CDD_C_ERROR_MEMORY;
  }

  if (!out_field->name)
    return CDD_C_ERROR_PARSE;

  rc = match(state, CDD_DSL_TOKEN_SEMICOLON, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!is_match) {
    rc = match(state, CDD_DSL_TOKEN_COMMA, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_func_decl(parser_state_t *state, cdd_ffi_ir_t *ir);
static cdd_c_error_t parse_enum_decl(parser_state_t *state, cdd_ffi_ir_t *ir);
static cdd_c_error_t parse_macro_decl(parser_state_t *state, cdd_ffi_ir_t *ir);
static cdd_c_error_t parse_namespace_decl(parser_state_t *state,
                                          cdd_ffi_ir_t *ir);

static cdd_c_error_t parse_namespace_decl(parser_state_t *state,
                                          cdd_ffi_ir_t *ir) {
  cdd_c_error_t rc;
  cdd_dsl_token_t name_tok;
  cdd_ffi_trivia_t *trivia = NULL;
  int is_match;
  cdd_dsl_token_t tok;

  rc = collect_trivia(state);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = take_pending_trivia(state, &trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = match(state, CDD_DSL_TOKEN_KW_NAMESPACE, &is_match);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (!is_match) {
    rc = CDD_C_ERROR_PARSE;
    goto cleanup;
  }

  rc = peek(state, &tok);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (tok.kind != CDD_DSL_TOKEN_IDENTIFIER) {
    rc = CDD_C_ERROR_PARSE;
    goto cleanup;
  }
  rc = advance_raw(state, &name_tok);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = match(state, CDD_DSL_TOKEN_LBRACE, &is_match);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (!is_match) {
    rc = CDD_C_ERROR_PARSE;
    goto cleanup;
  }

  if (ir->module_name)
    free(ir->module_name);
  ir->module_name = CDD_STRDUP(name_tok.text_copy);
  if (!ir->module_name) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  if (trivia) {
    cdd_ffi_trivia_t *tmp;
    while (trivia) {
      tmp = trivia->next;
      if (trivia->text)
        free(trivia->text);
      free(trivia);
      trivia = tmp;
    }
    trivia = NULL;
  }

  while (1) {
    rc = match(state, CDD_DSL_TOKEN_RBRACE, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match)
      break;

    rc = peek(state, &tok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (tok.kind == CDD_DSL_TOKEN_EOF)
      break;

    rc = collect_trivia(state);
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = peek(state, &tok);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (tok.kind == CDD_DSL_TOKEN_KW_NAMESPACE) {
      rc = parse_namespace_decl(state, ir);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (tok.kind == CDD_DSL_TOKEN_KW_FUNC ||
               tok.kind == CDD_DSL_TOKEN_KW_VIRTUAL ||
               tok.kind == CDD_DSL_TOKEN_KW_PURE) {
      rc = parse_func_decl(state, ir);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      rc = advance_raw(state, &tok);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;

cleanup:
  if (trivia) {
    cdd_ffi_trivia_t *tmp;
    while (trivia) {
      tmp = trivia->next;
      if (trivia->text)
        free(trivia->text);
      free(trivia);
      trivia = tmp;
    }
  }
  return rc;
}

static cdd_c_error_t parse_enum_decl(parser_state_t *state, cdd_ffi_ir_t *ir) {
  cdd_ffi_ir_node_t *node;
  cdd_c_error_t rc;
  cdd_dsl_token_t name_tok;
  int is_match;
  cdd_dsl_token_t tok;

  rc = collect_trivia(state);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = match(state, CDD_DSL_TOKEN_KW_ENUM, &is_match);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (!is_match) {
    rc = CDD_C_ERROR_PARSE;
    goto cleanup;
  }

  rc = peek(state, &tok);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (tok.kind != CDD_DSL_TOKEN_IDENTIFIER) {
    rc = CDD_C_ERROR_PARSE;
    goto cleanup;
  }
  rc = advance_raw(state, &name_tok);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = ir_add_node(ir, CDD_FFI_NODE_ENUM, name_tok.text_copy, &node);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = take_pending_trivia(state, &node->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = match(state, CDD_DSL_TOKEN_LBRACE, &is_match);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (!is_match) {
    rc = CDD_C_ERROR_PARSE;
    goto cleanup;
  }

  while (1) {
    cdd_ffi_enum_variant_t variant;

    rc = match(state, CDD_DSL_TOKEN_RBRACE, &is_match);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (is_match)
      break;

    rc = peek(state, &tok);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (tok.kind == CDD_DSL_TOKEN_EOF)
      break;

    memset(&variant, 0, sizeof(variant));

    rc = collect_trivia(state);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    rc = peek(state, &tok);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (tok.kind != CDD_DSL_TOKEN_IDENTIFIER) {
      rc = CDD_C_ERROR_PARSE;
      goto cleanup;
    }

    rc = advance(state, &tok);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    variant.name = CDD_STRDUP(tok.text_copy);
    if (!variant.name) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup;
    }

    rc = match(state, CDD_DSL_TOKEN_EQUALS, &is_match);
    if (rc != CDD_C_SUCCESS) {
      free(variant.name);
      goto cleanup;
    }
    if (is_match) {
      rc = parse_number(state, &variant.number_details);
      if (rc != CDD_C_SUCCESS) {
        free(variant.name);
        goto cleanup;
      }
    }

    {
      cdd_ffi_enum_variant_t *nv = (cdd_ffi_enum_variant_t *)CDD_REALLOC(
          node->variants,
          (node->variants_count + 1) * sizeof(cdd_ffi_enum_variant_t));
      if (!nv) {
        free(variant.name);
        rc = CDD_C_ERROR_MEMORY;
        goto cleanup;
      }
      node->variants = nv;
      node->variants[node->variants_count++] = variant;
    }

    rc = match(state, CDD_DSL_TOKEN_COMMA, &is_match);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = CDD_C_SUCCESS;
cleanup:
  return rc;
}

static cdd_c_error_t parse_macro_decl(parser_state_t *state, cdd_ffi_ir_t *ir) {
  cdd_ffi_ir_node_t *node;
  cdd_c_error_t rc;
  cdd_dsl_token_kind_t kind;
  cdd_dsl_token_t name_tok;
  int is_match;
  cdd_dsl_token_t tok;

  rc = collect_trivia(state);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = peek(state, &tok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  kind = tok.kind;
  if (kind != CDD_DSL_TOKEN_KW_MACRO_CONST &&
      kind != CDD_DSL_TOKEN_KW_MACRO_EXPR &&
      kind != CDD_DSL_TOKEN_KW_MACRO_STMT) {
    return CDD_C_ERROR_PARSE;
  }
  rc = advance_raw(state, &tok);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = peek(state, &tok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (tok.kind != CDD_DSL_TOKEN_IDENTIFIER)
    return CDD_C_ERROR_PARSE;
  rc = advance_raw(state, &name_tok);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = ir_add_node(ir, CDD_FFI_NODE_MACRO, name_tok.text_copy, &node);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = take_pending_trivia(state, &node->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (kind == CDD_DSL_TOKEN_KW_MACRO_EXPR ||
      kind == CDD_DSL_TOKEN_KW_MACRO_STMT) {
    rc = match(state, CDD_DSL_TOKEN_LPAREN, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match) {
      while (1) {
        rc = match(state, CDD_DSL_TOKEN_RPAREN, &is_match);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_match)
          break;

        rc = peek(state, &tok);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (tok.kind == CDD_DSL_TOKEN_EOF)
          break;

        rc = match(state, CDD_DSL_TOKEN_IDENTIFIER, &is_match);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_match)
          continue;

        rc = match(state, CDD_DSL_TOKEN_COMMA, &is_match);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_match)
          continue;

        return CDD_C_ERROR_PARSE;
      }
    }
  }

  rc = match(state, CDD_DSL_TOKEN_EQUALS, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!is_match) {
    return CDD_C_ERROR_PARSE;
  }

  if (kind == CDD_DSL_TOKEN_KW_MACRO_CONST) {
    struct cdd_ffi_number_t *num;
    rc = parse_number(state, &num);
    if (rc != CDD_C_SUCCESS)
      return rc;
    node->evaluated_value = CDD_STRDUP(num->raw_spelling);
    free(num->raw_spelling);
    free(num);
    if (!node->evaluated_value)
      return CDD_C_ERROR_MEMORY;
  } else {
    rc = match(state, CDD_DSL_TOKEN_KW_BODY, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!is_match)
      return CDD_C_ERROR_PARSE;

    rc = match(state, CDD_DSL_TOKEN_RAW_START, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!is_match)
      return CDD_C_ERROR_PARSE;

    rc = peek(state, &tok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (tok.kind == CDD_DSL_TOKEN_RAW_TEXT) {
      rc = advance(state, &tok);
      if (rc != CDD_C_SUCCESS)
        return rc;
      node->raw_body = CDD_STRDUP(tok.text_copy);
      if (!node->raw_body)
        return CDD_C_ERROR_MEMORY;
    }
    rc = match(state, CDD_DSL_TOKEN_RAW_END, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!is_match)
      return CDD_C_ERROR_PARSE;
  }

  rc = match(state, CDD_DSL_TOKEN_SEMICOLON, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_func_decl(parser_state_t *state, cdd_ffi_ir_t *ir) {
  cdd_ffi_ir_node_t *node;
  cdd_c_error_t rc;
  cdd_dsl_token_t name_tok;
  int is_match;
  cdd_dsl_token_t tok;

  rc = collect_trivia(state);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (1) {
    rc = match(state, CDD_DSL_TOKEN_KW_VIRTUAL, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match)
      continue;

    rc = match(state, CDD_DSL_TOKEN_KW_PURE, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match)
      continue;

    break;
  }

  rc = match(state, CDD_DSL_TOKEN_KW_FUNC, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!is_match)
    return CDD_C_ERROR_PARSE;

  rc = peek(state, &tok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (tok.kind != CDD_DSL_TOKEN_IDENTIFIER)
    return CDD_C_ERROR_PARSE;

  rc = advance_raw(state, &name_tok);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = ir_add_node(ir, CDD_FFI_NODE_FUNCTION, name_tok.text_copy, &node);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = take_pending_trivia(state, &node->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = match(state, CDD_DSL_TOKEN_LPAREN, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!is_match)
    return CDD_C_ERROR_PARSE;

  while (1) {
    cdd_ffi_field_t field;

    rc = match(state, CDD_DSL_TOKEN_RPAREN, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match)
      break;

    rc = peek(state, &tok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (tok.kind == CDD_DSL_TOKEN_EOF)
      break;

    rc = parse_field(state, &field);
    if (rc != CDD_C_SUCCESS)
      return rc;

    {
      cdd_ffi_field_t *nf = (cdd_ffi_field_t *)CDD_REALLOC(
          node->fields, (node->fields_count + 1) * sizeof(cdd_ffi_field_t));
      if (!nf)
        return CDD_C_ERROR_MEMORY;
      node->fields = nf;
    }
    node->fields[node->fields_count++] = field;
  }

  rc = match(state, CDD_DSL_TOKEN_EQUALS, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_match) {
    rc = peek(state, &tok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (tok.kind == CDD_DSL_TOKEN_NUMBER) {
      rc = advance_raw(state, &tok);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  /* Body */
  rc = match(state, CDD_DSL_TOKEN_LBRACE, &is_match);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (is_match) {
    while (1) {
      rc = match(state, CDD_DSL_TOKEN_RBRACE, &is_match);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (is_match)
        break;

      rc = peek(state, &tok);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (tok.kind == CDD_DSL_TOKEN_EOF)
        break;

      rc = match(state, CDD_DSL_TOKEN_KW_BODY, &is_match);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (is_match) {
        rc = match(state, CDD_DSL_TOKEN_RAW_START, &is_match);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_match) {
          rc = peek(state, &tok);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (tok.kind == CDD_DSL_TOKEN_RAW_TEXT) {
            rc = advance(state, &tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
            node->raw_body = CDD_STRDUP(tok.text_copy);
            if (!node->raw_body)
              return CDD_C_ERROR_MEMORY;
          }
          rc = match(state, CDD_DSL_TOKEN_RAW_END, &is_match);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      } else {
        rc = advance_raw(state, &tok);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  } else {
    rc = match(state, CDD_DSL_TOKEN_SEMICOLON, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!is_match)
      return CDD_C_ERROR_PARSE;
  }

  return CDD_C_SUCCESS;
}

C_CDD_EXPORT cdd_c_error_t cdd_dsl_parse(const cdd_dsl_token_list_t *tokens,
                                         cdd_ffi_ir_t **out_ir) {
  parser_state_t state;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  cdd_dsl_token_t tok;

  {
    size_t i;
    for (i = 0; i < tokens->count; i++) {
      printf("T%zu:%d|%s ", i, tokens->tokens[i].kind,
             tokens->tokens[i].text_copy ? tokens->tokens[i].text_copy : "");
    }
    printf("\n");
  }
  if (!tokens || !out_ir)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_ir = (cdd_ffi_ir_t *)CDD_CALLOC(1, sizeof(cdd_ffi_ir_t));
  if (!*out_ir)
    return CDD_C_ERROR_MEMORY;

  memset(&state, 0, sizeof(state));
  state.tokens = tokens;
  state.current = 0;

  while (1) {
    rc = peek_raw(&state, &tok);
    if (rc != CDD_C_SUCCESS)
      break;
    if (tok.kind == CDD_DSL_TOKEN_EOF)
      break;

    rc = collect_trivia(&state);
    if (rc != CDD_C_SUCCESS)
      break;

    rc = peek(&state, &tok);
    if (rc != CDD_C_SUCCESS)
      break;

    if (tok.kind == CDD_DSL_TOKEN_KW_FUNC ||
        tok.kind == CDD_DSL_TOKEN_KW_VIRTUAL ||
        tok.kind == CDD_DSL_TOKEN_KW_PURE) {
      rc = parse_func_decl(&state, *out_ir);
      if (rc != CDD_C_SUCCESS)
        break;
    } else if (tok.kind == CDD_DSL_TOKEN_KW_ENUM) {
      rc = parse_enum_decl(&state, *out_ir);
      if (rc != CDD_C_SUCCESS)
        break;
    } else if (tok.kind == CDD_DSL_TOKEN_KW_MACRO_CONST ||
               tok.kind == CDD_DSL_TOKEN_KW_MACRO_EXPR ||
               tok.kind == CDD_DSL_TOKEN_KW_MACRO_STMT) {
      rc = parse_macro_decl(&state, *out_ir);
      if (rc != CDD_C_SUCCESS)
        break;
    } else if (tok.kind == CDD_DSL_TOKEN_KW_NAMESPACE) {
      rc = parse_namespace_decl(&state, *out_ir);
      if (rc != CDD_C_SUCCESS)
        break;
    } else {
      rc = advance(&state, &tok);
      if (rc != CDD_C_SUCCESS)
        break;
    }
  }

  if (rc != CDD_C_SUCCESS) {
    cdd_ffi_ir_free(*out_ir);
    free(*out_ir);
    *out_ir = NULL;
  }
  return rc;
}
