#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cst_parser.h"

#ifdef _MSC_VER
#include <errno.h>
#else
#include <sys/errno.h>
#endif

int add_node(struct CstNodeList *const list, const enum CstNodeKind1 kind,
             const uint8_t *const start, const size_t length) {
  if (list->size >= list->capacity) {
    const size_t new_capacity = list->capacity == 0 ? 64 : list->capacity * 2;
    struct CstNode1 *new_nodes =
        realloc(list->nodes, new_capacity * sizeof(*new_nodes));
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

  while (i < tokens->size) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_WHITESPACE) {
      i++;
      continue;
    }

    /* ====== CONTEXT-SENSITIVE GROUPING FOR an IDENTIFIER; after a struct
     * ====== */
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
        size_t length = (last_tok->start + last_tok->length) - tok->start;
        if (add_node(out, CST_NODE_OTHER, tok->start, length) != 0)
          return -1;

        i = semicolon_pos + 1;
        last_significant_token = TOKEN_SEMICOLON;
        continue;
      }
    }

    /* ====== GREEDY PARSING FOR struct/enum/union with a BODY {...} ====== */
    if (tok->kind == TOKEN_KEYWORD_STRUCT || tok->kind == TOKEN_KEYWORD_ENUM ||
        tok->kind == TOKEN_KEYWORD_UNION) {
      size_t lookahead_pos = i + 1;
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
          const struct Token *last_block_tok =
              brace_count == 0 ? &tokens->tokens[block_end_pos - 1]
                               : &tokens->tokens[tokens->size - 1];
          size_t k;
          const size_t length =
              (last_block_tok->start + last_block_tok->length) - tok->start;
          if (add_node(out, node_kind, tok->start, length) != 0)
            return -1;

          /* Secondary scan for nested struct/enum/union KEYWORDS inside the
           * block
           */
          for (k = block_start_pos + 1; k < block_end_pos - 1; k++) {
            if (tokens->tokens[k].kind == TOKEN_KEYWORD_STRUCT) {
              if (add_node(out, CST_NODE_STRUCT, tokens->tokens[k].start,
                           tokens->tokens[k].length) != 0)
                return -1;
            } else if (tokens->tokens[k].kind == TOKEN_KEYWORD_ENUM) {
              if (add_node(out, CST_NODE_ENUM, tokens->tokens[k].start,
                           tokens->tokens[k].length) != 0)
                return -1;
            } else if (tokens->tokens[k].kind == TOKEN_KEYWORD_UNION) {
              if (add_node(out, CST_NODE_UNION, tokens->tokens[k].start,
                           tokens->tokens[k].length) != 0)
                return -1;
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
      if (add_node(out, node_kind_default, tok->start, tok->length) != 0)
        return -1;
    }

    last_significant_token = tok->kind;
    i++;
  }

  return 0;
}

void free_cst_node_list(struct CstNodeList *const list) {
  free(list->nodes);
  list->nodes = NULL, list->size = 0, list->capacity = 0;
}
