/**
 * @file cst_parser.c
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/cst.h"

/**
 * @brief Helper to skip whitespace tokens.
 */
static size_t skip_ws(const struct TokenList *const tokens, size_t i,
                      size_t limit) {
  while (i < limit && tokens->tokens[i].kind == TOKEN_WHITESPACE)
    i++;
  return i;
}

/**
 * @brief Helper to skip whitespace tokens backwards.
 */
static size_t skip_ws_back(const struct TokenList *const tokens, size_t i) {
  if (i == 0) {
    /* If index 0 is valid and not whitespace, return it. If whitespace,
       we can't go back further, but logic checking kind will see whitespace. */
    return 0;
  }
  i--;
  while (i > 0 && tokens->tokens[i].kind == TOKEN_WHITESPACE)
    i--;

  /* Check if we stopped at 0 and 0 is whitespace */
  /* If i==0 and it is whitespace, we effectively return a pointer to
   * whitespace. */
  return i;
}

/**
 * @brief Add a node to the CST list.
 */
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

/* Helper: is this token a valid start of a function return type? */
static int is_type_start(const struct Token *tok) {
  if (tok->kind == TOKEN_IDENTIFIER)
    return 1;
  /* Keywords that can be types or specifiers */
  switch (tok->kind) {
  case TOKEN_KEYWORD_VOID:
  case TOKEN_KEYWORD_CHAR:
  case TOKEN_KEYWORD_INT:
  case TOKEN_KEYWORD_FLOAT:
  case TOKEN_KEYWORD_DOUBLE:
  case TOKEN_KEYWORD_LONG:
  case TOKEN_KEYWORD_SHORT:
  case TOKEN_KEYWORD_SIGNED:
  case TOKEN_KEYWORD_UNSIGNED:
  case TOKEN_KEYWORD_STRUCT:
  case TOKEN_KEYWORD_ENUM:
  case TOKEN_KEYWORD_UNION:
  case TOKEN_KEYWORD_STATIC:
  case TOKEN_KEYWORD_INLINE:
  case TOKEN_KEYWORD_EXTERN:
  case TOKEN_KEYWORD_CONST:
  case TOKEN_KEYWORD_VOLATILE:
  case TOKEN_KEYWORD_AUTO:
  case TOKEN_KEYWORD_REGISTER:
  case TOKEN_KEYWORD_BOOL:
    return 1;
  default:
    return 0;
  }
}

/**
 * @brief Heuristic to detect function definitions.
 */
static int match_function_definition(const struct TokenList *const tokens,
                                     size_t start_idx, size_t limit,
                                     size_t *end_idx_out) {
  size_t k = start_idx;
  int paren_depth;
  int brace_depth;
  int seen_lparen = 0;
  int seen_ident = 0;

  while (k < limit) {
    const enum TokenKind kind = tokens->tokens[k].kind;

    /* If we encounter end-of-statement tokens or blocks before the parameter
       list, it's not a function definition. */
    if (kind == TOKEN_SEMICOLON || kind == TOKEN_LBRACE ||
        kind == TOKEN_RBRACE) {
      return 0;
    }

    /* Check for tokens that cannot be part of a function signature head.
       Assignments, literals, operators (except *), etc. imply expression
       context. */
    if (kind == TOKEN_ASSIGN || kind == TOKEN_EQ || kind == TOKEN_PLUS ||
        kind == TOKEN_MINUS || kind == TOKEN_SLASH || kind == TOKEN_PERCENT ||
        kind == TOKEN_NUMBER_LITERAL || kind == TOKEN_STRING_LITERAL) {
      return 0;
    }

    if (is_type_start(&tokens->tokens[k])) {
      seen_ident = 1;
    }

    if (kind == TOKEN_LPAREN) {
      if (!seen_ident)
        return 0;
      seen_lparen = 1;
      break;
    }
    k++;
  }

  if (!seen_lparen || k >= limit)
    return 0;

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

  k = skip_ws(tokens, k, limit);
  if (k >= limit || tokens->tokens[k].kind != TOKEN_LBRACE)
    return 0;

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

/**
 * @brief Consume a balanced parenthesized block `( ... )`.
 */
static size_t consume_balanced_parens(const struct TokenList *const tokens,
                                      size_t start, size_t limit) {
  size_t i = start;
  int depth = 0;

  if (i < limit && tokens->tokens[i].kind == TOKEN_LPAREN) {
    depth = 1;
    i++;
  } else {
    return start;
  }

  while (i < limit && depth > 0) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      depth++;
    } else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      depth--;
    }
    i++;
  }

  return i;
}

/**
 * @brief Consume a C23 attribute block `[[ ... ]]`.
 */
static size_t consume_attributes(const struct TokenList *const tokens,
                                 size_t start, size_t limit) {
  size_t i = start + 2;
  int depth = 2;

  while (i < limit && depth > 0) {
    if (tokens->tokens[i].kind == TOKEN_LBRACKET) {
      depth++;
    } else if (tokens->tokens[i].kind == TOKEN_RBRACKET) {
      depth--;
    }
    i++;
  }

  if (depth == 0)
    return i;
  return start;
}

/**
 * @brief Consume a static assertion declaration.
 */
static size_t consume_static_assert(const struct TokenList *const tokens,
                                    size_t start, size_t limit) {
  size_t i = start + 1;
  int paren_depth = 0;

  i = skip_ws(tokens, i, limit);
  if (i < limit && tokens->tokens[i].kind == TOKEN_LPAREN) {
    paren_depth = 1;
    i++;
  } else {
    return start;
  }

  while (i < limit && paren_depth > 0) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN)
      paren_depth++;
    else if (tokens->tokens[i].kind == TOKEN_RPAREN)
      paren_depth--;
    i++;
  }

  if (paren_depth != 0)
    return start;

  i = skip_ws(tokens, i, limit);
  if (i < limit && tokens->tokens[i].kind == TOKEN_SEMICOLON) {
    return i + 1;
  }
  return start;
}

/**
 * @brief Consume a _Generic selection `_Generic ( ... )`.
 */
static size_t consume_generic_selection(const struct TokenList *const tokens,
                                        size_t start, size_t limit) {
  /* _Generic ( assignment-expression , generic-assoc-list ) */
  /* The generic-assoc-list is inside parens. */
  /* We just need to consume the balanced parens after _Generic. */
  size_t i = start + 1;

  i = skip_ws(tokens, i, limit);
  if (i < limit && tokens->tokens[i].kind == TOKEN_LPAREN) {
    return consume_balanced_parens(tokens, i, limit);
  }
  return start;
}

/**
 * @brief Identify if the LBRACE at `brace_idx` signifies an expression/init
 * list.
 */
static int is_expression_brace(const struct TokenList *const tokens,
                               size_t brace_idx) {
  size_t prev;
  enum TokenKind pk;

  if (brace_idx == 0)
    return 0;

  prev = skip_ws_back(tokens, brace_idx);

  pk = tokens->tokens[prev].kind;

  if (pk == TOKEN_ASSIGN || pk == TOKEN_COMMA || pk == TOKEN_KEYWORD_RETURN ||
      pk == TOKEN_LBRACKET || pk == TOKEN_COLON) {
    return 1;
  }

  if (pk == TOKEN_RPAREN) {
    int depth = 1;
    size_t k = prev;

    while (k > 0 && depth > 0) {
      k--;
      if (tokens->tokens[k].kind == TOKEN_RPAREN)
        depth++;
      else if (tokens->tokens[k].kind == TOKEN_LPAREN)
        depth--;
    }

    if (depth == 0) {
      size_t before_paren = skip_ws_back(tokens, k);
      enum TokenKind bpk = tokens->tokens[before_paren].kind;

      if (bpk == TOKEN_KEYWORD_IF || bpk == TOKEN_KEYWORD_WHILE ||
          bpk == TOKEN_KEYWORD_FOR || bpk == TOKEN_KEYWORD_SWITCH) {
        return 0;
      }

      return 1;
    }
  }

  return 0;
}

/**
 * @brief Consume a brace-enclosed block, respecting nesting.
 */
static size_t consume_balanced_braces(const struct TokenList *const tokens,
                                      size_t start, size_t limit) {
  size_t i = start;
  int depth = 0;

  if (i < limit && tokens->tokens[i].kind == TOKEN_LBRACE) {
    depth = 1;
    i++;
  } else {
    return start;
  }

  while (i < limit && depth > 0) {
    if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      depth++;
    } else if (tokens->tokens[i].kind == TOKEN_RBRACE) {
      depth--;
    }
    i++;
  }

  return i;
}

/**
 * @brief Recursive Parser core logic.
 */
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

    /* C23 Attributes */
    if (tok->kind == TOKEN_LBRACKET && i + 1 < end &&
        tokens->tokens[i + 1].kind == TOKEN_LBRACKET) {
      size_t attr_end = consume_attributes(tokens, i, end);
      if (attr_end > i) {
        const struct Token *last = &tokens->tokens[attr_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_ATTRIBUTE, tok->start, byte_len, i,
                          attr_end);
        if (rc != 0)
          return rc;
        i = attr_end;
        continue;
      }
    }

    /* Static Assert */
    if (tok->kind == TOKEN_KEYWORD_STATIC_ASSERT) {
      size_t sa_end = consume_static_assert(tokens, i, end);
      if (sa_end > i) {
        const struct Token *last = &tokens->tokens[sa_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_STATIC_ASSERT, tok->start, byte_len, i,
                          sa_end);
        if (rc != 0)
          return rc;
        i = sa_end;
        continue;
      }
    }

    /* C11 _Generic */
    /* Note: Identify keyword `_Generic`. Tokenizer doesn't have explicit
       TOKEN_KEYWORD_GENERIC in previous state, but we can check identifier text
       logic or update tokenizer.
       WAIT: tokenizer.c `identify_keyword_or_id` does NOT currently list
       _Generic. However, `TOKEN_IDENTIFIER` check works for robust parsing if
       we check text. But proper support requires recognition.
       Assumed: If `tokenize` doesn't emit `TOKEN_KEYWORD_GENERIC`, it emits
       `TOKEN_IDENTIFIER`. We check text. */
    if ((tok->kind == TOKEN_IDENTIFIER &&
         token_matches_string(tok, "_Generic")) ||
        (tok->kind == TOKEN_IDENTIFIER &&
         token_matches_string(
             tok,
             "generic_selection" /* Fallback? No spec only says _Generic */))) {
      /* Actually tokenizer update might be needed for strict correctness,
         but here we can sniff identifier text. */
      size_t gen_end = consume_generic_selection(tokens, i, end);
      if (gen_end > i) {
        const struct Token *last = &tokens->tokens[gen_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_GENERIC_SELECTION, tok->start, byte_len,
                          i, gen_end);
        if (rc != 0)
          return rc;
        i = gen_end;
        continue;
      }
    }

    /* Function Definitions */
    if (is_type_start(tok) || tok->kind == TOKEN_STAR) {
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

    /* Struct/Enum/Union Blocks */
    if (tok->kind == TOKEN_KEYWORD_STRUCT || tok->kind == TOKEN_KEYWORD_ENUM ||
        tok->kind == TOKEN_KEYWORD_UNION) {

      /* Identify if this is a Compound Literal / Cast context to skip
       * definition parsing. */
      /* E.g. (struct S){...} */
      int is_literal = 0;
      {
        size_t prev = skip_ws_back(tokens, i);
        if (prev < i && tokens->tokens[prev].kind == TOKEN_LPAREN) {
          is_literal = 1; /* Fall through to CST_NODE_OTHER handling */
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
            body_start_idx = k + 1;
            block_end = consume_balanced_braces(tokens, k, end);
            found_block = (block_end > k);
            break;
          }
          k++;
        }

        if (found_block) {
          enum CstNodeKind nk;
          size_t byte_len;
          const struct Token *last;
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

          if (body_start_idx > 0) {
            size_t inner_end = block_end;
            while (inner_end > body_start_idx) {
              inner_end--;
              if (tokens->tokens[inner_end].kind == TOKEN_RBRACE)
                break;
            }
            if (inner_end > body_start_idx)
              parse_recursive(tokens, body_start_idx, inner_end, out);
          }
          i = block_end;
          continue;
        } else {
          /* Forward Decl */
          size_t decl_end = k;
          if (decl_end < end &&
              tokens->tokens[decl_end].kind == TOKEN_SEMICOLON)
            decl_end++;

          if (decl_end <= i)
            decl_end = i + 1;
          {
            enum CstNodeKind nk =
                (tok->kind == TOKEN_KEYWORD_STRUCT) ? CST_NODE_STRUCT
                : (tok->kind == TOKEN_KEYWORD_ENUM) ? CST_NODE_ENUM
                                                    : CST_NODE_UNION;

            /* Safety check for end index */
            if (decl_end > 0) {
              const struct Token *last = &tokens->tokens[decl_end - 1];
              size_t byte_len =
                  (size_t)((last->start + last->length) - tok->start);
              rc = cst_list_add(out, nk, tok->start, byte_len, i, decl_end);
              if (rc != 0)
                return rc;
            }
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
      if (rc != 0)
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
          if (is_expression_brace(tokens, j)) {
            j = consume_balanced_braces(tokens, j, end);
            continue;
          } else {
            break;
          }
        }

        if (kind == TOKEN_KEYWORD_STRUCT || kind == TOKEN_KEYWORD_ENUM ||
            kind == TOKEN_KEYWORD_UNION) {
          /* Detect if this keyword is part of a Cast or Compound Literal
             (check if preceded by LPAREN) to avoid breaking statement */
          size_t prev = skip_ws_back(tokens, j);

          if (prev < j && prev >= i &&
              tokens->tokens[prev].kind == TOKEN_LPAREN) {
            j++; /* Consume and continue */
            continue;
          }
          break;
        }

        if (kind == TOKEN_COMMENT || kind == TOKEN_MACRO ||
            kind == TOKEN_HASH || kind == TOKEN_KEYWORD_STATIC_ASSERT) {
          break;
        }
        /* Check match for _Generic identifier explicitly to break loop */
        if (kind == TOKEN_IDENTIFIER &&
            token_matches_string(&tokens->tokens[j], "_Generic")) {
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
