/**
 * @file cst.c
 * @brief Implementation of the Concrete Syntax Tree logic.
 *
 * Implements recursive descent parsing to group tokens into semantic blocks.
 * Supports C23 attributes, Bit-fields, Static Assertions, C99 Compound
 * Literals, C23 Fixed Enum Types, and C11 _Generic selections.
 *
 * Update: Verified Bit-field support. The `parse_recursive` loop handling
 * `CST_NODE_OTHER` consumes tokens until `;`, `}`, or `{` (expression start).
 * Since bit-fields use `:` (TOKEN_COLON), and colons are treated as regular
 * punctuation within statements (unless they match `is_expression_brace`,
 * which only triggers on `{` preceded by specific tokens), bit-field
 * declarations like `int x : 3;` are correctly grouped into a single statement
 * node.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd_export.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/parse/cst.h"
#include "functions/parse/cst_match.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_alloc;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws_back;
extern C_CDD_EXPORT int g_cdd_fail_is_type_start;
extern C_CDD_EXPORT int g_cdd_fail_consume_balanced_parens;
extern C_CDD_EXPORT int g_cdd_fail_consume_attributes;
extern C_CDD_EXPORT int g_cdd_fail_consume_static_assert;
extern C_CDD_EXPORT int g_cdd_fail_consume_generic_selection;
extern C_CDD_EXPORT int g_cdd_fail_is_expression_brace;
extern C_CDD_EXPORT int g_cdd_fail_consume_balanced_braces;
extern C_CDD_EXPORT int g_cdd_fail_match_function_definition;
extern C_CDD_EXPORT int g_cdd_fail_cst_list_add;
extern C_CDD_EXPORT int g_cdd_fail_token_matches_string;
#endif

/**
 * @brief Append a node to CST node list.
 */
cdd_c_error_t cst_list_add(struct CstNodeList *list, enum CstNodeKind kind,
                           const uint8_t *start, size_t length,
                           size_t start_tok, size_t end_tok) {
  struct CstNode *new_arr;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_cst_list_add && --g_cdd_fail_cst_list_add == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (list->size >= list->capacity) {
    const size_t new_cap = (list->capacity == 0) ? 64 : list->capacity * 2;
    new_arr = (struct CstNode *)C_CDD_REALLOC(list->nodes,
                                              new_cap * sizeof(struct CstNode));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    list->nodes = new_arr;
    list->capacity = new_cap;
  }

  list->nodes[list->size].kind = kind;
  list->nodes[list->size].start = start;
  list->nodes[list->size].length = length;
  list->nodes[list->size].start_token = start_tok;
  list->nodes[list->size].end_token = end_tok;
  list->size++;

  return CDD_C_SUCCESS;
}

/**
 * @brief Recursive Parser core logic.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start token index.
 * @param[in] end End token index.
 * @param[in,out] out CST node list to populate.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_recursive(const struct TokenList *tokens,
                                     size_t start, size_t end,
                                     struct CstNodeList *out) {
  size_t _ast_consume_attributes_7 = 0;
  size_t _ast_consume_static_assert_8 = 0;
  int _ast_token_matches_string_9 = 0;
  int _ast_token_matches_string_10 = 0;
  size_t _ast_consume_generic_selection_11 = 0;
  size_t _ast_skip_ws_back_12 = 0;
  size_t _ast_consume_balanced_braces_13 = 0;
  size_t _ast_skip_ws_14 = 0;
  size_t _ast_consume_balanced_braces_15 = 0;
  size_t _ast_skip_ws_back_16 = 0;
  int _ast_token_matches_string_17 = 0;
  size_t i = start;
  cdd_c_error_t rc;

  while (i < end) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_WHITESPACE) {
      i++;
      continue;
    }

    /* GCC Attributes */
    if (tok->kind == TOKEN_KEYWORD_ATTRIBUTE) {
      size_t attr_end = i + 1;
      int depth = 0;
      while (attr_end < end) {
        if (tokens->tokens[attr_end].kind == TOKEN_LPAREN)
          depth++;
        else if (tokens->tokens[attr_end].kind == TOKEN_RPAREN) {
          depth--;
          if (depth == 0) {
            attr_end++;
            break;
          }
        }
        attr_end++;
      }
      if (depth == 0) {
        const struct Token *last = &tokens->tokens[attr_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_GCC_ATTRIBUTE, tok->start, byte_len, i,
                          attr_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = attr_end;
        continue;
      }
    }

    /* MSVC Declspec */
    if (tok->kind == TOKEN_KEYWORD_DECLSPEC) {
      size_t attr_end = i + 1;
      int depth = 0;
      while (attr_end < end) {
        if (tokens->tokens[attr_end].kind == TOKEN_LPAREN)
          depth++;
        else if (tokens->tokens[attr_end].kind == TOKEN_RPAREN) {
          depth--;
          if (depth == 0) {
            attr_end++;
            break;
          }
        }
        attr_end++;
      }
      if (depth == 0) {
        const struct Token *last = &tokens->tokens[attr_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_DECLSPEC, tok->start, byte_len, i,
                          attr_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = attr_end;
        continue;
      }
    }

    /* C23 Attributes */
    if (tok->kind == TOKEN_LBRACKET && i + 1 < end &&
        tokens->tokens[i + 1].kind == TOKEN_LBRACKET) {
      size_t attr_end;
      {
        cdd_c_error_t rc_cst =
            consume_attributes(tokens, i, end, &_ast_consume_attributes_7);
        if (rc_cst != CDD_C_SUCCESS)
          return rc_cst;
      }
      attr_end = _ast_consume_attributes_7;
      if (attr_end > i) {
        const struct Token *last = &tokens->tokens[attr_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_ATTRIBUTE, tok->start, byte_len, i,
                          attr_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = attr_end;
        continue;
      }
    }

    /* Static Assert */
    if (tok->kind == TOKEN_KEYWORD_STATIC_ASSERT) {
      size_t sa_end;
      {
        cdd_c_error_t rc_cst = consume_static_assert(
            tokens, i, end, &_ast_consume_static_assert_8);
        if (rc_cst != CDD_C_SUCCESS)
          return rc_cst;
      }
      sa_end = _ast_consume_static_assert_8;
      if (sa_end > i) {
        const struct Token *last = &tokens->tokens[sa_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_STATIC_ASSERT, tok->start, byte_len, i,
                          sa_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = sa_end;
        continue;
      }
    }

    /* C11 _Generic */
    {
      cdd_c_error_t rc_cst =
          token_matches_string(tok, "_Generic", &_ast_token_matches_string_9);
      if (rc_cst != CDD_C_SUCCESS)
        return rc_cst;
    }
    {
      cdd_c_error_t rc_cst = token_matches_string(
          tok, "generic_selection", &_ast_token_matches_string_10);
      if (rc_cst != CDD_C_SUCCESS)
        return rc_cst;
    }
    if ((tok->kind == TOKEN_IDENTIFIER && _ast_token_matches_string_9) ||
        (tok->kind == TOKEN_IDENTIFIER && _ast_token_matches_string_10)) {
      size_t gen_end;
      {
        cdd_c_error_t rc_cst = consume_generic_selection(
            tokens, i, end, &_ast_consume_generic_selection_11);
        if (rc_cst != CDD_C_SUCCESS)
          return rc_cst;
      }
      gen_end = _ast_consume_generic_selection_11;

      if (gen_end > i) {
        const struct Token *last = &tokens->tokens[gen_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_GENERIC_SELECTION, tok->start, byte_len,
                          i, gen_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = gen_end;
        continue;
      }
    }

    /* Function Definitions */
    {
      int is_type = 0;
      {
        cdd_c_error_t rc_cst = is_type_start(tok, &is_type);
        if (rc_cst != CDD_C_SUCCESS)
          return rc_cst;
      }
      if (is_type || tok->kind == TOKEN_STAR) {
        size_t func_end = 0;
        int is_match = 0;
        {
          cdd_c_error_t rc_cst =
              match_function_definition(tokens, i, end, &func_end, &is_match);
          if (rc_cst != CDD_C_SUCCESS)
            return rc_cst;
        }
        if (is_match) {
          const struct Token *last = &tokens->tokens[func_end - 1];
          size_t byte_len = (size_t)((last->start + last->length) - tok->start);
          rc = cst_list_add(out, CST_NODE_FUNCTION, tok->start, byte_len, i,
                            func_end);
          if (rc != CDD_C_SUCCESS)
            return rc;
          i = func_end;
          continue;
        }
      }
    }

    /* Struct/Enum/Union Blocks */
    if (tok->kind == TOKEN_KEYWORD_STRUCT || tok->kind == TOKEN_KEYWORD_ENUM ||
        tok->kind == TOKEN_KEYWORD_UNION) {
      int is_literal = 0;
      {
        size_t prev;
        {
          cdd_c_error_t rc_cst = skip_ws_back(tokens, i, &_ast_skip_ws_back_12);
          if (rc_cst != CDD_C_SUCCESS)
            return rc_cst;
        }
        prev = _ast_skip_ws_back_12;
        if (prev < i && tokens->tokens[prev].kind == TOKEN_LPAREN) {
          is_literal = 1;
        }
      }

      if (!is_literal) {
        size_t k = i + 1;
        size_t block_end = 0;
        size_t body_start_idx = 0;
        int found_block = 0;

        while (k < end) {
          if (tokens->tokens[k].kind == TOKEN_SEMICOLON)
            break;
          if (tokens->tokens[k].kind == TOKEN_LBRACE) {
            cdd_c_error_t rc_cst;
            body_start_idx = k + 1;
            rc_cst = consume_balanced_braces(tokens, k, end,
                                             &_ast_consume_balanced_braces_13);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
            block_end = _ast_consume_balanced_braces_13;
            found_block = (block_end > k);
            break;
          }
          k++;
        }

        if (found_block) {
          enum CstNodeKind nk;
          size_t byte_len;
          const struct Token *last;
          size_t next_probe;
          size_t brace_close_idx = _ast_consume_balanced_braces_13 - 1;
          {
            cdd_c_error_t rc_cst =
                skip_ws(tokens, block_end, end, &_ast_skip_ws_14);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
          }
          next_probe = _ast_skip_ws_14;

          if (next_probe < end &&
              tokens->tokens[next_probe].kind == TOKEN_SEMICOLON) {
            block_end = next_probe + 1;
          }

          if (tok->kind == TOKEN_KEYWORD_STRUCT)
            nk = CST_NODE_STRUCT;
          else if (tok->kind == TOKEN_KEYWORD_ENUM)
            nk = CST_NODE_ENUM;
          else
            nk = CST_NODE_UNION;

          last = &tokens->tokens[block_end - 1];
          byte_len = (size_t)((last->start + last->length) - tok->start);

          rc = cst_list_add(out, nk, tok->start, byte_len, i, block_end);
          if (rc != CDD_C_SUCCESS)
            return rc;

          if (brace_close_idx > body_start_idx) {
            cdd_c_error_t rc_cst =
                parse_recursive(tokens, body_start_idx, brace_close_idx, out);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
          }
          i = block_end;
          continue;
        } else {
          /* Forward Decl */
          size_t decl_end = k;
          if (decl_end < end)
            decl_end++;
          {
            enum CstNodeKind nk =
                (tok->kind == TOKEN_KEYWORD_STRUCT) ? CST_NODE_STRUCT
                : (tok->kind == TOKEN_KEYWORD_ENUM) ? CST_NODE_ENUM
                                                    : CST_NODE_UNION;
            const struct Token *last = &tokens->tokens[decl_end - 1];
            size_t byte_len =
                (size_t)((last->start + last->length) - tok->start);
            rc = cst_list_add(out, nk, tok->start, byte_len, i, decl_end);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          i = decl_end;
          continue;
        }
      }
    }

    if (tok->kind == TOKEN_COMMENT) {
      rc = cst_list_add(out, CST_NODE_COMMENT, tok->start, tok->length, i,
                        i + 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i++;
      continue;
    }
    if (tok->kind == TOKEN_MACRO || tok->kind == TOKEN_HASH) {
      size_t j = i + 1;
      while (j < end) {
        if (tokens->tokens[j - 1].kind == TOKEN_WHITESPACE &&
            memchr(tokens->tokens[j - 1].start, '\n',
                   tokens->tokens[j - 1].length)) {
          break;
        }
        j++;
      }
      rc = cst_list_add(out, CST_NODE_MACRO, tok->start, tok->length, i, j);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i = j;
      continue;
    }

    /* Group CST_NODE_OTHER (Statements / Vars / Bitfields) */
    {
      size_t j = i + 1;
      while (j < end) {
        enum TokenKind kind = tokens->tokens[j].kind;

        if (kind == TOKEN_SEMICOLON) {
          j++;
          break;
        }

        if (kind == TOKEN_RBRACE) {
          break;
        }

        if (kind == TOKEN_LBRACE) {
          int is_expr = 0;
          {
            cdd_c_error_t rc_cst = is_expression_brace(tokens, j, &is_expr);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
          }
          if (is_expr) {
            cdd_c_error_t rc_cst = consume_balanced_braces(
                tokens, j, end, &_ast_consume_balanced_braces_15);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
            j = _ast_consume_balanced_braces_15;
            continue;
          } else {
            break;
          }
        }

        if (kind == TOKEN_KEYWORD_STRUCT || kind == TOKEN_KEYWORD_ENUM ||
            kind == TOKEN_KEYWORD_UNION) {
          size_t prev;
          {
            cdd_c_error_t rc_cst =
                skip_ws_back(tokens, j, &_ast_skip_ws_back_16);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
          }
          prev = _ast_skip_ws_back_16;

          if (tokens->tokens[prev].kind == TOKEN_LPAREN) {
            j++;
            continue;
          }
          break;
        }

        if (kind == TOKEN_COMMENT || kind == TOKEN_MACRO ||
            kind == TOKEN_HASH || kind == TOKEN_KEYWORD_STATIC_ASSERT) {
          break;
        }
        {
          cdd_c_error_t rc_cst = token_matches_string(
              &tokens->tokens[j], "_Generic", &_ast_token_matches_string_17);
          if (rc_cst != CDD_C_SUCCESS)
            return rc_cst;
        }
        if (kind == TOKEN_IDENTIFIER && _ast_token_matches_string_17) {
          break;
        }

        if (kind == TOKEN_LBRACKET && j + 1 < end &&
            tokens->tokens[j + 1].kind == TOKEN_LBRACKET) {
          break;
        }
        j++;
      }

      {
        const struct Token *last = &tokens->tokens[j - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_OTHER, tok->start, byte_len, i, j);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = j;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses tokens from the given input.
 */
cdd_c_error_t parse_tokens(const struct TokenList *tokens,
                           struct CstNodeList *out) {
  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (out->capacity == 0) {
    out->nodes = NULL;
    out->size = 0;
  }

  return parse_recursive(tokens, 0, tokens->size, out);
}

/**
 * @brief Frees the memory associated with cst node list.
 */
void free_cst_node_list(struct CstNodeList *list) {
  if (!list)
    return;
  if (list->nodes) {
    C_CDD_FREE(list->nodes);
    list->nodes = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

/**
 * @brief Executes the cst find first operation.
 */
cdd_c_error_t cst_find_first(struct CstNodeList *list,
                             const enum CstNodeKind kind,
                             struct CstNode **out_node) {
  size_t i;
  if (!out_node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_node = NULL;
  if (!list) {
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < list->size; i++) {
    if (list->nodes[i].kind == kind) {
      *out_node = &list->nodes[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
cdd_c_error_t cdd_test_cst_parse_recursive(const struct TokenList *tokens,
                                           size_t start, size_t end,
                                           struct CstNodeList *out) {
  return parse_recursive(tokens, start, end, out);
}
#endif /* CDD_BUILD_TESTS */
