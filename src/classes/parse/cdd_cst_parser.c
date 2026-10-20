/**
 * @file cdd_cst_parser.c
 * @brief Concrete Syntax Tree (CST) parser implementation and utilities.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_parser_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "cdd_lexer.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Allocate a new CST node.
 *
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t alloc_node(enum cdd_cst_node_kind_t kind, cdd_cst_node_t *parent,
                         cdd_cst_node_t **out_node) {
  cdd_cst_node_t *n = (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
  if (!n) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }
  n->kind = kind;
  n->parent = parent;
  *out_node = n;
  return CDD_C_SUCCESS;
}

/**
 * @brief Append a child token to a node.
 *
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t append_child_token(cdd_cst_node_t *node, cdd_token_t *tok) {
  size_t new_cap = node->num_children + 1;
  cdd_cst_child_t *new_arr = (cdd_cst_child_t *)C_CDD_REALLOC(
      node->children, new_cap * sizeof(cdd_cst_child_t));
  if (!new_arr) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }
  node->children = new_arr;
  node->capacity = new_cap;
  node->children[node->num_children].kind = CDD_CST_CHILD_TOKEN;
  node->children[node->num_children].val.token = tok;
  node->num_children++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Consume the current token from parser state and append it to `node`.
 *
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t consume_token(parser_state_t *s, cdd_cst_node_t *node) {
  cdd_token_t *t = &s->list->tokens[s->pos++];
  return append_child_token(node, t);
}

/**
 * @brief Append a child node to a node.
 *
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t append_child_node(cdd_cst_node_t *node, cdd_cst_node_t *child) {
  size_t new_cap = node->num_children + 1;
  cdd_cst_child_t *new_arr = (cdd_cst_child_t *)C_CDD_REALLOC(
      node->children, new_cap * sizeof(cdd_cst_child_t));
  if (!new_arr) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }
  node->children = new_arr;
  node->capacity = new_cap;
  node->children[node->num_children].kind = CDD_CST_CHILD_NODE;
  node->children[node->num_children].val.node = child;
  node->num_children++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Free a node and all of its descendants recursively.
 */
void free_node(cdd_cst_node_t *node) {
  size_t i;
  if (!node)
    return;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      free_node(node->children[i].val.node);
    }
  }
  if (node->children)
    C_CDD_FREE(node->children);
  C_CDD_FREE(node);
}

/**
 * @brief Peek next token from parser state without advancing.
 *
 * @return CDD_C_SUCCESS or error code.
 */
C_CDD_EXPORT cdd_c_error_t peek(parser_state_t *s, cdd_token_t **out_tok) {
  if (!s || !out_tok)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos];
    return CDD_C_SUCCESS;
  }
  *out_tok = NULL;
  return CDD_C_ERROR_NOT_FOUND;
}

/**
 * @brief Advance parser state and return current token.
 *
 * @return CDD_C_SUCCESS or error code.
 */
C_CDD_EXPORT cdd_c_error_t advance(parser_state_t *s, cdd_token_t **out_tok) {
  if (!s || !out_tok)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos++];
    return CDD_C_SUCCESS;
  }
  *out_tok = NULL;
  return CDD_C_ERROR_NOT_FOUND;
}

static cdd_c_error_t parse_declaration_or_statement(parser_state_t *s,
                                                    cdd_cst_node_t *parent,
                                                    cdd_cst_node_t **out_node);

/**
 * @brief Parses a brace-delimited block (`{ ... }`) and its inner statements.
 *
 * @param[in,out] s Active parser state.
 * @param[in] parent Parent CST node.
 * @param[out] out_node Pointer to receive the parsed block node.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t parse_block_internal(parser_state_t *s,
                                          cdd_cst_node_t *parent,
                                          cdd_cst_node_t **out_node) {
  cdd_cst_node_t *b = NULL;
  cdd_token_t *t = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS, app_rc;

  rc = alloc_node(CDD_CST_BLOCK, parent, &b);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_node = b;

  rc = consume_token(s, b);
  if (rc != CDD_C_SUCCESS) {
    free_node(b);
    *out_node = NULL;
    return rc;
  }

  while (peek(s, &t) == CDD_C_SUCCESS && t->kind != CDD_TOKEN_RBRACE) {
    cdd_cst_node_t *child = NULL;
    rc = parse_declaration_or_statement(s, b, &child);
    if (rc != CDD_C_SUCCESS)
      return rc;
    app_rc = append_child_node(b, child);

    if (app_rc != CDD_C_SUCCESS) {
      free_node(child);
      s->err = (int)app_rc;
      free_node(b);
      *out_node = NULL;
      return app_rc;
    }
  }

  rc = advance(s, &t);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = append_child_token(b, t);
  if (rc != CDD_C_SUCCESS)
    return rc;

  *out_node = b;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parse a block delimited by braces.
 *
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t parse_block(parser_state_t *s, cdd_cst_node_t *parent,
                          cdd_cst_node_t **out_node) {
  cdd_c_error_t rc = parse_block_internal(s, parent, out_node);
  if (rc != CDD_C_SUCCESS) {
    free_node(*out_node);
    *out_node = NULL;
    s->err = (int)rc;
  }
  return rc;
}

/**
 * @brief Finds the identifier token of the nearest enclosing class/struct
 * declaration.
 *
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if not in a class, or
 * CDD_C_ERROR_INVALID_ARGUMENT if `out_tok` is `NULL`.
 */
C_CDD_EXPORT cdd_c_error_t get_class_name(cdd_cst_node_t *node,
                                          cdd_token_t **out_tok) {
  if (!out_tok)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_tok = NULL;

  while (node) {
    if (node->kind == CDD_CST_CLASS_DECLARATION) {
      size_t i;
      for (i = 0; i < node->num_children; i++) {
        if (node->children[i].val.token->kind == CDD_TOKEN_IDENTIFIER) {
          *out_tok = node->children[i].val.token;
          return CDD_C_SUCCESS;
        }
      }
    }
    node = node->parent;
  }
  return CDD_C_ERROR_NOT_FOUND;
}

static cdd_c_error_t parse_declaration_or_statement_internal(
    parser_state_t *s, cdd_cst_node_t *parent, cdd_cst_node_t **out_node);

/**
 * @brief Wrapper around `parse_declaration_or_statement_internal` that frees
 * partial nodes on error.
 *
 * @param[in,out] s Active parser state.
 * @param[in] parent Parent CST node.
 * @param[out] out_node Pointer to receive the parsed declaration or statement
 * node.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t parse_declaration_or_statement(parser_state_t *s,
                                                    cdd_cst_node_t *parent,
                                                    cdd_cst_node_t **out_node) {
  cdd_c_error_t rc =
      parse_declaration_or_statement_internal(s, parent, out_node);
  if (rc != CDD_C_SUCCESS) {
    free_node(*out_node);
    *out_node = NULL;
    s->err = (int)rc;
  }
  return rc;
}

/**
 * @brief Parses a single declaration, preprocessor directive, or statement from
 * the token stream.
 *
 * @param[in,out] s Active parser state.
 * @param[in] parent Parent CST node.
 * @param[out] out_node Pointer to receive the parsed CST node.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t parse_declaration_or_statement_internal(
    parser_state_t *s, cdd_cst_node_t *parent, cdd_cst_node_t **out_node) {
  cdd_c_error_t rc = CDD_C_SUCCESS, app_rc;
  cdd_token_t *t = NULL;
  cdd_cst_node_t *n = NULL;
  *out_node = NULL;

  rc = peek(s, &t);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (t->kind >= CDD_TOKEN_PREPROC_INCLUDE &&
      t->kind <= CDD_TOKEN_PREPROC_PRAGMA) {
    int is_cond = (t->kind >= CDD_TOKEN_PREPROC_IFDEF &&
                   t->kind <= CDD_TOKEN_PREPROC_IFNDEF);
    rc = alloc_node(is_cond ? CDD_CST_PREPROC_CONDITIONAL
                            : CDD_CST_PREPROC_DIRECTIVE,
                    parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_node = n;

    rc = consume_token(s, n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (is_cond) {
      cdd_token_t *nxt = NULL;
      while (peek(s, &nxt) == CDD_C_SUCCESS) {
        if (nxt->kind == CDD_TOKEN_PREPROC_ENDIF) {
          rc = consume_token(s, n);
          if (rc != CDD_C_SUCCESS)
            return rc;
          break;
        }
        if (nxt->kind == CDD_TOKEN_LBRACE) {
          cdd_cst_node_t *child = NULL;
          rc = parse_block(s, n, &child);
          if (rc != CDD_C_SUCCESS)
            return rc;
          app_rc = append_child_node(n, child);

          if (app_rc != CDD_C_SUCCESS) {
            free_node(child);
            s->err = (int)app_rc;
            free_node(n);
            *out_node = NULL;
            return app_rc;
          }
        } else {
          rc = consume_token(s, n);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_LBRACE) {
    return parse_block(s, parent, out_node);
  }

  if (t->kind == CDD_TOKEN_KEYWORD_TEMPLATE) {
    rc = alloc_node(CDD_CST_TEMPLATE_DECLARATION, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_node = n;

    rc = consume_token(s, n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (peek(s, &t) == CDD_C_SUCCESS && t->kind == CDD_TOKEN_LT) {
      cdd_cst_node_t *param_list;
      rc = alloc_node(CDD_CST_TEMPLATE_PARAMETER_LIST, n, &param_list);
      if (rc != CDD_C_SUCCESS)
        return rc;
      app_rc = append_child_node(n, param_list);
      if (app_rc != CDD_C_SUCCESS) {
        free_node(param_list);
        return app_rc;
      }

      rc = consume_token(s, param_list);
      if (rc != CDD_C_SUCCESS)
        return rc;

      while (peek(s, &t) == CDD_C_SUCCESS) {
        if (t->kind == CDD_TOKEN_GT) {
          rc = consume_token(s, param_list);
          if (rc != CDD_C_SUCCESS)
            return rc;
          break;
        }
        if (t->kind == CDD_TOKEN_COMMA) {
          rc = consume_token(s, param_list);
          if (rc != CDD_C_SUCCESS)
            return rc;
          continue;
        }

        if (t->kind == CDD_TOKEN_KEYWORD_CLASS ||
            t->kind == CDD_TOKEN_KEYWORD_TYPENAME ||
            t->kind == CDD_TOKEN_IDENTIFIER) {
          cdd_cst_node_t *param;
          rc = alloc_node(CDD_CST_TEMPLATE_PARAMETER, param_list, &param);
          if (rc != CDD_C_SUCCESS)
            return rc;
          app_rc = append_child_node(param_list, param);
          if (app_rc != CDD_C_SUCCESS) {
            free_node(param);
            return app_rc;
          }

          rc = consume_token(s, param);
          if (rc != CDD_C_SUCCESS)
            return rc;

          if (peek(s, &t) == CDD_C_SUCCESS && t->kind == CDD_TOKEN_IDENTIFIER) {
            rc = consume_token(s, param);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        } else {
          rc = consume_token(s, param_list);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }

    {
      cdd_cst_node_t *child = NULL;
      rc = parse_declaration_or_statement(s, n, &child);
      if (rc != CDD_C_SUCCESS)
        return rc;
      app_rc = append_child_node(n, child);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(child);
        s->err = (int)app_rc;
        free_node(n);
        *out_node = NULL;
        return app_rc;
      }
    }

    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_NAMESPACE) {
    rc = alloc_node(CDD_CST_NAMESPACE_DECLARATION, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_node = n;

    rc = consume_token(s, n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (peek(s, &t) == CDD_C_SUCCESS && t->kind == CDD_TOKEN_IDENTIFIER) {
      rc = consume_token(s, n);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }

    if (peek(s, &t) == CDD_C_SUCCESS && t->kind == CDD_TOKEN_LBRACE) {
      cdd_cst_node_t *child = NULL;
      rc = parse_block(s, n, &child);
      if (rc != CDD_C_SUCCESS)
        return rc;
      app_rc = append_child_node(n, child);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(child);
        s->err = (int)app_rc;
        free_node(n);
        *out_node = NULL;
        return app_rc;
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_USING) {
    cdd_token_t *nxt = NULL;
    rc = alloc_node(CDD_CST_USING_DIRECTIVE, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_node = n;
    while (peek(s, &nxt) == CDD_C_SUCCESS) {
      rc = consume_token(s, n);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (nxt->kind == CDD_TOKEN_SEMICOLON)
        break;
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_TRY) {
    rc = alloc_node(CDD_CST_TRY_BLOCK, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_node = n;

    rc = consume_token(s, n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (peek(s, &t) == CDD_C_SUCCESS && t->kind == CDD_TOKEN_LBRACE) {
      cdd_cst_node_t *child = NULL;
      rc = parse_block(s, n, &child);
      if (rc != CDD_C_SUCCESS)
        return rc;
      app_rc = append_child_node(n, child);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(child);
        s->err = (int)app_rc;
        free_node(n);
        *out_node = NULL;
        return app_rc;
      }
    }

    while (peek(s, &t) == CDD_C_SUCCESS && t->kind == CDD_TOKEN_KEYWORD_CATCH) {
      cdd_cst_node_t *catch_node = NULL;

      rc = alloc_node(CDD_CST_CATCH_BLOCK, n, &catch_node);
      if (rc != CDD_C_SUCCESS)
        return rc;
      app_rc = append_child_node(n, catch_node);
      if (app_rc != CDD_C_SUCCESS) {
        free_node(catch_node);
        return app_rc;
      }

      rc = consume_token(s, catch_node);
      if (rc != CDD_C_SUCCESS)
        return rc;

      if (peek(s, &t) == CDD_C_SUCCESS && t->kind == CDD_TOKEN_LPAREN) {
        while (peek(s, &t) == CDD_C_SUCCESS) {
          rc = consume_token(s, catch_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (t->kind == CDD_TOKEN_RPAREN)
            break;
        }
      }

      if (peek(s, &t) == CDD_C_SUCCESS && t->kind == CDD_TOKEN_LBRACE) {
        cdd_cst_node_t *child = NULL;
        rc = parse_block(s, catch_node, &child);
        if (rc != CDD_C_SUCCESS)
          return rc;
        app_rc = append_child_node(catch_node, child);

        if (app_rc != CDD_C_SUCCESS) {
          free_node(child);
          s->err = (int)app_rc;
          return app_rc;
        }
      }
    }

    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_THROW) {
    rc = alloc_node(CDD_CST_THROW_EXPRESSION, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_node = n;

    rc = consume_token(s, n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    while (peek(s, &t) == CDD_C_SUCCESS) {
      rc = consume_token(s, n);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (t->kind == CDD_TOKEN_SEMICOLON)
        break;
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  return parse_class_or_statement(s, parent, t, out_node);
}

/**
 * @brief Parses a C/C++ source span into a full-fidelity Concrete Syntax Tree
 * (CST).
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_parse(az_span source, cdd_cst_tree_t **out_tree) {
  cdd_c_error_t rc = CDD_C_SUCCESS, app_rc = CDD_C_SUCCESS;
  parser_state_t state = {0};
  cdd_cst_tree_t *tree;
  cdd_token_t *t = NULL;

  if (!out_tree)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  tree = (cdd_cst_tree_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_tree_t));
  if (!tree) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }

  rc = cdd_lexer_tokenize(source, &tree->base_tokens);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(tree);
    return rc;
  }

  state.list = tree->base_tokens;
  state.pos = 0;
  state.err = 0;

  rc = alloc_node(CDD_CST_TRANSLATION_UNIT, NULL, &tree->root);
  if (rc != CDD_C_SUCCESS) {
    cdd_cst_tree_free(tree);
    return rc;
  }

  while (peek(&state, &t) == CDD_C_SUCCESS) {
    if (t->kind == CDD_TOKEN_EOF) {
      rc = consume_token(&state, tree->root);
      if (rc != CDD_C_SUCCESS) {
        state.err = (int)rc;
      }
      break;
    }
    {
      cdd_cst_node_t *child = NULL;
      rc = parse_declaration_or_statement(&state, tree->root, &child);
      if (rc != CDD_C_SUCCESS) {
        state.err = (int)rc;
        break;
      }
      app_rc = append_child_node(tree->root, child);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(child);
        state.err = (int)app_rc;
        break;
      }
    }
  }

  if (state.err) {
    cdd_cst_tree_free(tree);
    return (cdd_c_error_t)state.err;
  }

  *out_tree = tree;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a CST tree, its root node hierarchy, base tokens, synthesized
 * tokens, and string pool.
 */
void cdd_cst_tree_free(cdd_cst_tree_t *tree) {
  size_t i;
  if (!tree)
    return;
  if (tree->root)
    free_node(tree->root);
  if (tree->base_tokens)
    cdd_lexer_free_token_list(tree->base_tokens);
  if (tree->synthesized_tokens) {
    for (i = 0; i < tree->num_synthesized; i++) {
      if (tree->synthesized_tokens[i]) {
        cdd_trivia_t *t = tree->synthesized_tokens[i]->leading_trivia;
        while (t) {
          cdd_trivia_t *n = t->next;
          C_CDD_FREE(t);
          t = n;
        }
        t = tree->synthesized_tokens[i]->trailing_trivia;
        while (t) {
          cdd_trivia_t *n = t->next;
          C_CDD_FREE(t);
          t = n;
        }
        C_CDD_FREE(tree->synthesized_tokens[i]);
      }
    }
    C_CDD_FREE(tree->synthesized_tokens);
  }
  if (tree->string_pool) {
    for (i = 0; i < tree->num_strings; i++) {
      C_CDD_FREE(tree->string_pool[i]);
    }
    C_CDD_FREE(tree->string_pool);
  }
  C_CDD_FREE(tree);
}
