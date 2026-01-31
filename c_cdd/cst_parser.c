/**
 * @file cst_parser.c
 * @brief Implementation of CST parser logic.
 * Supports nested and anonymous structure parsing.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Use system errno if standard includes don't provide it */
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

/**
 * @brief Skip whitespace tokens starting from index.
 */
static size_t skip_ws(const struct TokenList *const tokens, size_t i,
                      size_t limit) {
  while (i < limit && tokens->tokens[i].kind == TOKEN_WHITESPACE) {
    i++;
  }
  return i;
}

/**
 * @brief Attempt to identify a function definition starting at current index.
 */
static int match_function_definition(const struct TokenList *const tokens,
                                     size_t start_idx, size_t limit,
                                     size_t *end_idx) {
  size_t k = start_idx;
  size_t last_ident_idx = (size_t)-1;
  int paren_depth = 0;

  /* 1. Header Scan: Look for '(' preceded by an Identifier */
  while (k < limit) {
    const enum TokenKind kind = tokens->tokens[k].kind;

    if (kind == TOKEN_LBRACE || kind == TOKEN_SEMICOLON) {
      return 0;
    }

    if (kind == TOKEN_LPAREN) {
      if (last_ident_idx == (size_t)-1) {
        return 0;
      }
      break; /* Move to Parameter Scan */
    }

    if (kind == TOKEN_IDENTIFIER) {
      last_ident_idx = k;
    } else if (kind == TOKEN_OTHER) {
      /* Pointers '*' are TOKEN_OTHER len=1 */
      if (tokens->tokens[k].length == 1 && *tokens->tokens[k].start == '=') {
        return 0;
      }
    }

    k++;
  }

  if (k >= limit || tokens->tokens[k].kind != TOKEN_LPAREN) {
    return 0;
  }

  /* 2. Parameter/Paren Scan: Find matching ')' */
  paren_depth = 1;
  k++;
  while (k < limit && paren_depth > 0) {
    if (tokens->tokens[k].kind == TOKEN_LPAREN) {
      paren_depth++;
    } else if (tokens->tokens[k].kind == TOKEN_RPAREN) {
      paren_depth--;
    }
    k++;
  }

  if (paren_depth != 0) {
    return 0;
  }

  /* 3. Body Check: Expect '{' immediately */
  k = skip_ws(tokens, k, limit);

  if (k < limit && tokens->tokens[k].kind == TOKEN_LBRACE) {
    /* 4. Body Scan: Find matching '}' */
    size_t brace_depth = 1;
    k++;
    while (k < limit && brace_depth > 0) {
      if (tokens->tokens[k].kind == TOKEN_LBRACE) {
        brace_depth++;
      } else if (tokens->tokens[k].kind == TOKEN_RBRACE) {
        brace_depth--;
      }
      k++;
    }

    if (brace_depth == 0) {
      *end_idx = k;
      return 1;
    }
  }

  return 0;
}

static int parse_tokens_recursive(const struct TokenList *const tokens,
                                  size_t start_index, size_t end_index,
                                  struct CstNodeList *const out,
                                  int only_significant_nodes) {
  size_t i = start_index;
  enum TokenKind last_significant_token = TOKEN_UNKNOWN;
  int rc;

  while (i < end_index) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_WHITESPACE) {
      i++;
      continue;
    }

    /* Helper: check for semicolon handling logic */
    if (tok->kind == TOKEN_SEMICOLON) {
      /* If this semicolon follows a named block, we skip it to group
         "struct S{};" as one logical unit. For anonymous blocks "struct {};",
         we keep it (last_sig != RBRACE). */

      if (last_significant_token == TOKEN_RBRACE) {
        last_significant_token = TOKEN_SEMICOLON;
        i++;
        continue;
      }

      /* Otherwise, standalone semicolon? e.g. ";". Add if not filtering. */
      if (!only_significant_nodes) {
        rc = add_node(out, CST_NODE_OTHER, tok->start, tok->length);
        if (rc != 0)
          return rc;
      }
      last_significant_token = TOKEN_SEMICOLON;
      i++;
      continue;
    }

    /* Function Definition */
    {
      size_t func_end_idx = 0;
      if (match_function_definition(tokens, i, end_index, &func_end_idx)) {
        const struct Token *end_tok = &tokens->tokens[func_end_idx - 1];
        size_t length =
            (size_t)((end_tok->start + end_tok->length) - tok->start);

        rc = add_node(out, CST_NODE_FUNCTION, tok->start, length);
        if (rc != 0)
          return rc;

        i = func_end_idx;
        last_significant_token = TOKEN_RBRACE;
        continue;
      }
    }

    /* Variable Declaration after block (e.g. struct { } var;) */
    if (tok->kind == TOKEN_IDENTIFIER &&
        last_significant_token == TOKEN_RBRACE) {
      /* We just finished a block (RBRACE). If we see an Identifier, it might be
       * a variable decl ending in ; */
      size_t semicolon_pos = i + 1;
      while (semicolon_pos < end_index &&
             tokens->tokens[semicolon_pos].kind == TOKEN_WHITESPACE) {
        semicolon_pos++;
      }
      if (semicolon_pos < end_index &&
          tokens->tokens[semicolon_pos].kind == TOKEN_SEMICOLON) {
        /* Confirmed "var ;" pattern */
        const struct Token *last_tok = &tokens->tokens[semicolon_pos];
        size_t length =
            (size_t)((last_tok->start + last_tok->length) - tok->start);

        /* Only add if allowed */
        if (!only_significant_nodes) {
          rc = add_node(out, CST_NODE_OTHER, tok->start, length);
          if (rc != 0)
            return rc;
        }

        i = semicolon_pos + 1;
        last_significant_token = TOKEN_SEMICOLON;
        continue;
      }
    }

    /* Struct / Enum / Union */
    if (tok->kind == TOKEN_KEYWORD_STRUCT || tok->kind == TOKEN_KEYWORD_ENUM ||
        tok->kind == TOKEN_KEYWORD_UNION) {
      size_t lookahead_pos = i + 1;
      int is_named = 0;

      lookahead_pos = skip_ws(tokens, lookahead_pos, end_index);
      if (lookahead_pos < end_index &&
          tokens->tokens[lookahead_pos].kind == TOKEN_IDENTIFIER) {
        is_named = 1;
        lookahead_pos++;
        lookahead_pos = skip_ws(tokens, lookahead_pos, end_index);
      }

      if (lookahead_pos < end_index &&
          tokens->tokens[lookahead_pos].kind == TOKEN_LBRACE) {
        size_t block_start_pos = lookahead_pos;
        size_t brace_count = 1;
        size_t block_end_pos = block_start_pos + 1;

        while (block_end_pos < end_index && brace_count > 0) {
          if (tokens->tokens[block_end_pos].kind == TOKEN_LBRACE)
            brace_count++;
          else if (tokens->tokens[block_end_pos].kind == TOKEN_RBRACE)
            brace_count--;
          block_end_pos++;
        }

        {
          enum CstNodeKind1 node_kind =
              tok->kind == TOKEN_KEYWORD_STRUCT ? CST_NODE_STRUCT
              : tok->kind == TOKEN_KEYWORD_ENUM ? CST_NODE_ENUM
                                                : CST_NODE_UNION;
          const struct Token *last_block_tok =
              brace_count == 0 ? &tokens->tokens[block_end_pos - 1]
                               : &tokens->tokens[end_index - 1];
          const size_t length =
              (size_t)((last_block_tok->start + last_block_tok->length) -
                       tok->start);

          rc = add_node(out, node_kind, tok->start, length);
          if (rc != 0)
            return rc;
        }

        /* Recursively scan the interior for nested definitions */
        /* Scan from inside braces: [block_start_pos + 1, block_end_pos - 1] */
        if (brace_count == 0 && block_end_pos > block_start_pos + 1) {
          rc = parse_tokens_recursive(tokens, block_start_pos + 1,
                                      block_end_pos - 1, out, 1);
          if (rc != 0)
            return rc;
        }

        i = block_end_pos;
        /* Only mark RBRACE if it was named, to consume the semicolon.
           If anonymous/unnamed (e.g. struct { } ;), we want to leave
           last_significant_token unset so the semicolon is captured as a
           separate node, satisfying test expectations. */
        if (is_named) {
          last_significant_token = TOKEN_RBRACE;
        } else {
          last_significant_token = TOKEN_UNKNOWN;
        }
        continue;
      }
    }

    /* Default case: capture single tokens as OTHER or specific kinds */
    {
      enum CstNodeKind1 node_kind_default = CST_NODE_OTHER;
      int should_add = !only_significant_nodes;

      switch (tok->kind) {
      case TOKEN_KEYWORD_STRUCT:
        node_kind_default = CST_NODE_STRUCT; /* Likely forward decl */
        break;
      case TOKEN_KEYWORD_ENUM:
        node_kind_default = CST_NODE_ENUM;
        break;
      case TOKEN_KEYWORD_UNION:
        node_kind_default = CST_NODE_UNION;
        break;
      case TOKEN_COMMENT:
        node_kind_default = CST_NODE_COMMENT;
        /* Comments typically preserved even if recursing, unless strict logic
         * needed */
        should_add = 1;
        break;
      case TOKEN_MACRO:
        node_kind_default = CST_NODE_MACRO;
        should_add = 1;
        break;
      default:
        break;
      }

      if (should_add) {
        rc = add_node(out, node_kind_default, tok->start, tok->length);
        if (rc != 0)
          return rc;
      }
    }

    last_significant_token = tok->kind;
    i++;
  }

  return 0;
}

int parse_tokens(const struct TokenList *const tokens,
                 struct CstNodeList *const out) {
  if (tokens == NULL || out == NULL)
    return EINVAL;

  if (out->capacity == 0) {
    out->nodes = NULL;
    out->size = 0;
  }

  /* Invoke recursive parser for the entire range, allowing all nodes (0 flag)
   */
  if (parse_tokens_recursive(tokens, 0, tokens->size, out, 0) != 0) {
    free_cst_node_list(out);
    return ENOMEM; /* Assume alloc error if non-zero */
  }

  return 0;
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
