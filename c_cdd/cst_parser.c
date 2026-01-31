/**
 * @file cst_parser.c
 * @brief Implementation of the Concrete Syntax Tree logic.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "cst_parser.h"

static size_t skip_ws(const struct TokenList *const tokens, size_t i,
                      size_t limit) {
  while (i < limit && tokens->tokens[i].kind == TOKEN_WHITESPACE)
    i++;
  return i;
}

int cst_list_add(struct CstNodeList *const list, const enum CstNodeKind kind,
                 const uint8_t *const start, const size_t length,
                 const size_t start_tok, const size_t end_tok) {
  struct CstNode *new_arr;
  if (!list)
    return EINVAL;

  if (list->size >= list->capacity) {
    const size_t new_cap = (list->capacity == 0) ? 64 : list->capacity * 2;
    new_arr = (struct CstNode *)realloc(list->nodes,
                                        new_cap * sizeof(struct CstNode));
    if (!new_arr)
      return ENOMEM;
    list->nodes = new_arr;
    list->capacity = new_cap;
  }

  list->nodes[list->size].kind = kind;
  list->nodes[list->size].start = start;
  list->nodes[list->size].length = length;
  list->nodes[list->size].start_token = start_tok;
  list->nodes[list->size].end_token = end_tok;
  list->size++;

  return 0;
}

static int token_is(const struct Token *tok, const char *s) {
  size_t len = strlen(s);
  return (tok->length == len && strncmp((const char *)tok->start, s, len) == 0);
}

static int match_function_definition(const struct TokenList *const tokens,
                                     size_t start_idx, size_t limit,
                                     size_t *end_idx_out) {
  size_t k = start_idx;
  int paren_depth;
  int brace_depth;
  int seen_lparen = 0;
  int seen_ident = 0;

  /* 1. Header Scan: Find `( ... )` */
  while (k < limit) {
    const enum TokenKind kind = tokens->tokens[k].kind;

    if (kind == TOKEN_SEMICOLON || kind == TOKEN_LBRACE ||
        kind == TOKEN_RBRACE) {
      return 0;
    }

    if (kind == TOKEN_IDENTIFIER) {
      /* Exclude keywords that might look like function calls/defs */
      if (token_is(&tokens->tokens[k], "if") ||
          token_is(&tokens->tokens[k], "while") ||
          token_is(&tokens->tokens[k], "for") ||
          token_is(&tokens->tokens[k], "switch") ||
          token_is(&tokens->tokens[k], "return")) {
        return 0;
      }
      seen_ident = 1;
    }

    if (kind == TOKEN_LPAREN) {
      if (!seen_ident)
        return 0; /* Require identifier before params */
      seen_lparen = 1;
      break;
    }
    k++;
  }

  if (!seen_lparen || k >= limit)
    return 0;

  /* 2. Skip over parameters (...) */
  paren_depth = 1;
  k++;
  while (k < limit && paren_depth > 0) {
    if (tokens->tokens[k].kind == TOKEN_LPAREN)
      paren_depth++;
    else if (tokens->tokens[k].kind == TOKEN_RPAREN)
      paren_depth--;
    k++;
  }
  if (k >= limit)
    return 0;

  /* 3. Check for body start `{` */
  k = skip_ws(tokens, k, limit);
  if (k >= limit || tokens->tokens[k].kind != TOKEN_LBRACE)
    return 0;

  /* 4. Skip body { ... } */
  brace_depth = 1;
  k++;
  while (k < limit && brace_depth > 0) {
    if (tokens->tokens[k].kind == TOKEN_LBRACE)
      brace_depth++;
    else if (tokens->tokens[k].kind == TOKEN_RBRACE)
      brace_depth--;
    k++;
  }

  if (brace_depth == 0) {
    *end_idx_out = k;
    return 1;
  }
  return 0;
}

static int parse_recursive(const struct TokenList *const tokens, size_t start,
                           size_t end, struct CstNodeList *const out) {
  size_t i = start;
  int rc;

  while (i < end) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_WHITESPACE) {
      i++;
      continue;
    }

    /* Attempt to match function definition first (if identifier or type) */
    /* Optimization: Only try if it looks like a declaration start */
    if (tok->kind == TOKEN_IDENTIFIER ||
        (tok->kind == TOKEN_OTHER && *tok->start == '*')) {
      size_t func_end = 0;
      if (match_function_definition(tokens, i, end, &func_end)) {
        const struct Token *last = &tokens->tokens[func_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);

        rc = cst_list_add(out, CST_NODE_FUNCTION, tok->start, byte_len, i,
                          func_end);
        if (rc != 0)
          return rc;

        i = func_end;
        continue;
      }
    }

    if (tok->kind == TOKEN_KEYWORD_STRUCT || tok->kind == TOKEN_KEYWORD_ENUM ||
        tok->kind == TOKEN_KEYWORD_UNION) {
      size_t k = i + 1;
      size_t block_end = 0;
      size_t body_start_idx = 0;
      int found_block = 0;

      /* Scan for Brace or Semicolon */
      while (k < end) {
        if (tokens->tokens[k].kind == TOKEN_SEMICOLON) {
          break; /* Forward decl */
        }
        if (tokens->tokens[k].kind == TOKEN_LBRACE) {
          int depth = 1;
          body_start_idx = k + 1;
          k++;
          while (k < end && depth > 0) {
            if (tokens->tokens[k].kind == TOKEN_LBRACE)
              depth++;
            else if (tokens->tokens[k].kind == TOKEN_RBRACE)
              depth--;
            k++;
          }
          /* k is now at token AFTER the RBRACE */
          block_end = k;
          found_block = 1;
          break;
        }
        k++;
      }

      if (found_block) {
        enum CstNodeKind nk;
        size_t byte_len;
        const struct Token *last;

        /* Try to absorb ONE trailing semicolon ONLY if it follows immediately
         */
        /* Be careful not to swallow a variable declaration like `struct S {}
         * var;` */
        size_t next_probe = skip_ws(tokens, block_end, end);
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
        if (rc != 0)
          return rc;

        /* Recurse into body if valid */
        if (body_start_idx > 0) {
          /* Find matching RBRACE index again for recursion bounds */
          size_t inner_scan = body_start_idx;
          int d = 1;
          while (inner_scan < block_end) { /* block_end is RBRACE+1 or SEMI+1 */
            if (tokens->tokens[inner_scan].kind == TOKEN_LBRACE)
              d++;
            else if (tokens->tokens[inner_scan].kind == TOKEN_RBRACE)
              d--;
            if (d == 0)
              break;
            inner_scan++;
          }
          if (inner_scan <= block_end) {
            parse_recursive(tokens, body_start_idx, inner_scan, out);
          }
        }

        i = block_end;
        continue;
      } else {
        /* Forward Decl: struct S; */
        size_t decl_end = k;
        /* If we hit semicolon, include it */
        if (decl_end < end &&
            tokens->tokens[decl_end].kind == TOKEN_SEMICOLON) {
          decl_end++;
        }

        {
          enum CstNodeKind nk =
              (tok->kind == TOKEN_KEYWORD_STRUCT) ? CST_NODE_STRUCT
              : (tok->kind == TOKEN_KEYWORD_ENUM) ? CST_NODE_ENUM
                                                  : CST_NODE_UNION;

          /* If decl_end <= i, clamp */
          if (decl_end <= i)
            decl_end = i + 1;

          {
            const struct Token *last = &tokens->tokens[decl_end - 1];
            size_t byte_len =
                (size_t)((last->start + last->length) - tok->start);

            rc = cst_list_add(out, nk, tok->start, byte_len, i, decl_end);
            if (rc != 0)
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
      if (rc != 0)
        return rc;
      i++;
      continue;
    }
    if (tok->kind == TOKEN_MACRO) {
      rc = cst_list_add(out, CST_NODE_MACRO, tok->start, tok->length, i, i + 1);
      if (rc != 0)
        return rc;
      i++;
      continue;
    }

    /* Group other tokens by statement */
    {
      size_t j = i + 1;
      while (j < end) {
        enum TokenKind kind = tokens->tokens[j].kind;
        if (kind == TOKEN_SEMICOLON) {
          j++;
          break;
        }
        if (kind == TOKEN_LBRACE || kind == TOKEN_RBRACE)
          break;
        /* Break on start of next structure */
        if (kind == TOKEN_KEYWORD_STRUCT || kind == TOKEN_KEYWORD_ENUM ||
            kind == TOKEN_KEYWORD_UNION || kind == TOKEN_COMMENT ||
            kind == TOKEN_MACRO) {
          break;
        }
        j++;
      }
      {
        const struct Token *last = &tokens->tokens[j - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_OTHER, tok->start, byte_len, i, j);
        if (rc != 0)
          return rc;
        i = j;
      }
    }
  }
  return 0;
}

int parse_tokens(const struct TokenList *const tokens,
                 struct CstNodeList *const out) {
  if (!tokens || !out)
    return EINVAL;

  if (out->capacity == 0) {
    out->nodes = NULL;
    out->size = 0;
  }

  return parse_recursive(tokens, 0, tokens->size, out);
}

void free_cst_node_list(struct CstNodeList *const list) {
  if (!list)
    return;
  if (list->nodes) {
    free(list->nodes);
    list->nodes = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

struct CstNode *cst_find_first(struct CstNodeList *const list,
                               const enum CstNodeKind kind) {
  size_t i;
  if (!list)
    return NULL;

  for (i = 0; i < list->size; i++) {
    if (list->nodes[i].kind == kind) {
      return &list->nodes[i];
    }
  }
  return NULL;
}
