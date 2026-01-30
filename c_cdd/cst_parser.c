/**
 * @file cst_parser.c
 * @brief Implementation of CST parser logic.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Use system errno if standard includes don't provide it,
   though stdlib/errno.h usually suffice. */
#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <sys/errno.h>
#endif

#include "cst_parser.h"

int add_node(struct CstNodeList *const list, const enum CstNodeKind1 kind,
             const uint8_t *const start, const size_t length) {
  if (list == NULL)
    return EINVAL;

  if (list->size >= list->capacity) {
    const size_t new_capacity = list->capacity == 0 ? 64 : list->capacity * 2;
    struct CstNode1 *new_nodes = (struct CstNode1 *)realloc(
        list->nodes, new_capacity * sizeof(struct CstNode1));
    if (!new_nodes)
      return ENOMEM;
    list->nodes = new_nodes;
    list->capacity = new_capacity;
  }
  list->nodes[list->size].kind = kind;
  list->nodes[list->size].start = start;
  list->nodes[list->size].length = length;
  list->size++;
  return 0;
}

int parse_tokens(const struct TokenList *const tokens,
                 struct CstNodeList *const out) {
  size_t i = 0;
  enum TokenKind last_significant_token = TOKEN_UNKNOWN;
  int rc;

  if (tokens == NULL || out == NULL)
    return EINVAL;

  /* Initialize output list safely */
  if (out->capacity == 0) {
    out->nodes = NULL;
    out->size = 0;
  }

  while (i < tokens->size) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_WHITESPACE) {
      i++;
      continue;
    }

    /* ====== CONTEXT-SENSITIVE GROUPING FOR an IDENTIFIER; after a struct
     * ====== */
    /* e.g. "struct S { ... } varname;" -> handle "varname;" as CST_NODE_OTHER
     */
    if (tok->kind == TOKEN_IDENTIFIER &&
        last_significant_token == TOKEN_RBRACE) {
      size_t semicolon_pos = i + 1;
      while (semicolon_pos < tokens->size &&
             tokens->tokens[semicolon_pos].kind == TOKEN_WHITESPACE) {
        semicolon_pos++;
      }
      if (semicolon_pos < tokens->size &&
          tokens->tokens[semicolon_pos].kind == TOKEN_SEMICOLON) {
        const struct Token *last_tok = &tokens->tokens[semicolon_pos];
        size_t length =
            (size_t)((last_tok->start + last_tok->length) - tok->start);
        rc = add_node(out, CST_NODE_OTHER, tok->start, length);
        if (rc != 0)
          goto cleanup;

        i = semicolon_pos + 1;
        last_significant_token = TOKEN_SEMICOLON;
        continue;
      }
    }

    /* ====== GREEDY PARSING FOR struct/enum/union with a BODY {...} ====== */
    if (tok->kind == TOKEN_KEYWORD_STRUCT || tok->kind == TOKEN_KEYWORD_ENUM ||
        tok->kind == TOKEN_KEYWORD_UNION) {
      size_t lookahead_pos = i + 1;
      /* Skip optional identifier */
      while (lookahead_pos < tokens->size &&
             tokens->tokens[lookahead_pos].kind == TOKEN_WHITESPACE)
        lookahead_pos++;
      if (lookahead_pos < tokens->size &&
          tokens->tokens[lookahead_pos].kind == TOKEN_IDENTIFIER) {
        lookahead_pos++;
        while (lookahead_pos < tokens->size &&
               tokens->tokens[lookahead_pos].kind == TOKEN_WHITESPACE)
          lookahead_pos++;
      }

      /* Check for start of body */
      if (lookahead_pos < tokens->size &&
          tokens->tokens[lookahead_pos].kind == TOKEN_LBRACE) {
        size_t block_start_pos = lookahead_pos;
        size_t brace_count = 1;
        size_t block_end_pos = block_start_pos + 1;

        while (block_end_pos < tokens->size && brace_count > 0) {
          if (tokens->tokens[block_end_pos].kind == TOKEN_LBRACE)
            brace_count++;
          else if (tokens->tokens[block_end_pos].kind == TOKEN_RBRACE)
            brace_count--;
          block_end_pos++;
        }

        {
          /* Create a single node for the entire `struct {...}` block */
          enum CstNodeKind1 node_kind =
              tok->kind == TOKEN_KEYWORD_STRUCT ? CST_NODE_STRUCT
              : tok->kind == TOKEN_KEYWORD_ENUM ? CST_NODE_ENUM
                                                : CST_NODE_UNION;
          /* If brace_count > 0, we hit end of stream (unterminated), take up to
           * end */
          const struct Token *last_block_tok =
              brace_count == 0 ? &tokens->tokens[block_end_pos - 1]
                               : &tokens->tokens[tokens->size - 1];
          size_t k;
          const size_t length =
              (size_t)((last_block_tok->start + last_block_tok->length) -
                       tok->start);

          rc = add_node(out, node_kind, tok->start, length);
          if (rc != 0)
            goto cleanup;

          /* Secondary scan for nested struct/enum/union KEYWORDS inside the
           * block */
          for (k = block_start_pos + 1;
               k < block_end_pos - (brace_count == 0 ? 1 : 0); k++) {
            if (tokens->tokens[k].kind == TOKEN_KEYWORD_STRUCT) {
              rc = add_node(out, CST_NODE_STRUCT, tokens->tokens[k].start,
                            tokens->tokens[k].length);
              if (rc != 0)
                goto cleanup;
            } else if (tokens->tokens[k].kind == TOKEN_KEYWORD_ENUM) {
              rc = add_node(out, CST_NODE_ENUM, tokens->tokens[k].start,
                            tokens->tokens[k].length);
              if (rc != 0)
                goto cleanup;
            } else if (tokens->tokens[k].kind == TOKEN_KEYWORD_UNION) {
              rc = add_node(out, CST_NODE_UNION, tokens->tokens[k].start,
                            tokens->tokens[k].length);
              if (rc != 0)
                goto cleanup;
            }
          }
        }

        i = block_end_pos;
        last_significant_token =
            brace_count == 0 ? TOKEN_RBRACE : TOKEN_UNKNOWN;
        continue;
      }
    }

    /* ====== DEFAULT: HANDLE ALL OTHER TOKENS INDIVIDUALLY ====== */
    {
      enum CstNodeKind1 node_kind_default = CST_NODE_OTHER;
      switch (tok->kind) {
      case TOKEN_KEYWORD_STRUCT:
        node_kind_default = CST_NODE_STRUCT;
        break;
      case TOKEN_KEYWORD_ENUM:
        node_kind_default = CST_NODE_ENUM;
        break;
      case TOKEN_KEYWORD_UNION:
        node_kind_default = CST_NODE_UNION;
        break;
      case TOKEN_COMMENT:
        node_kind_default = CST_NODE_COMMENT;
        break;
      case TOKEN_MACRO:
        node_kind_default = CST_NODE_MACRO;
        break;
      default:
        break;
      }
      rc = add_node(out, node_kind_default, tok->start, tok->length);
      if (rc != 0)
        goto cleanup;
    }

    last_significant_token = tok->kind;
    i++;
  }

  return 0;

cleanup:
  free_cst_node_list(out);
  return rc;
}

void free_cst_node_list(struct CstNodeList *const list) {
  if (list == NULL)
    return;
  if (list->nodes != NULL) {
    free(list->nodes);
    list->nodes = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}
