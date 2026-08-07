/* clang-format off */
#include "cdd_cst_parser.h"
#include "cdd_lexer.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
int g_cdd_cst_parser_fast_grow = 0;
#endif

static cdd_c_error_t alloc_node(enum cdd_cst_node_kind_t kind,
                                cdd_cst_node_t *parent,
                                cdd_cst_node_t **out_node) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  cdd_cst_node_t *n;
  n = (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
  if (n) {
    n->kind = kind;
    n->parent = parent;
    *out_node = n;
    return CDD_C_SUCCESS;
  }
  *out_node = NULL;
  return CDD_C_ERROR_MEMORY;
}

static cdd_c_error_t append_child_token(cdd_cst_node_t *node,
                                        cdd_token_t *tok) {
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (node->num_children >= node->capacity) {
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_parser_fast_grow;
    size_t new_cap = g_cdd_cst_parser_fast_grow
                         ? (node->capacity == 0 ? 8 : node->capacity * 2)
                         : node->capacity + 1;
#else
    size_t new_cap = node->capacity == 0 ? 8 : node->capacity * 2;
#endif
    cdd_cst_child_t *new_arr;
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_realloc_fail;
    if (g_cdd_cst_realloc_fail && --g_cdd_cst_realloc_fail == 0) {
      new_arr = NULL;
    } else
#endif
      new_arr = (cdd_cst_child_t *)C_CDD_REALLOC(
          node->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    node->children = new_arr;
    node->capacity = new_cap;
  }
  node->children[node->num_children].kind = CDD_CST_CHILD_TOKEN;
  node->children[node->num_children].val.token = tok;
  node->num_children++;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t append_child_node(cdd_cst_node_t *node,
                                       cdd_cst_node_t *child) {
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (node->num_children >= node->capacity) {
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_parser_fast_grow;
    size_t new_cap = g_cdd_cst_parser_fast_grow
                         ? (node->capacity == 0 ? 8 : node->capacity * 2)
                         : node->capacity + 1;
#else
    size_t new_cap = node->capacity == 0 ? 8 : node->capacity * 2;
#endif
    cdd_cst_child_t *new_arr;
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_realloc_fail;
    if (g_cdd_cst_realloc_fail && --g_cdd_cst_realloc_fail == 0) {
      new_arr = NULL;
    } else
#endif
      new_arr = (cdd_cst_child_t *)C_CDD_REALLOC(
          node->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    node->children = new_arr;
    node->capacity = new_cap;
  }
  child->parent = node;
  node->children[node->num_children].kind = CDD_CST_CHILD_NODE;
  node->children[node->num_children].val.node = child;
  node->num_children++;
  return CDD_C_SUCCESS;
}

static void free_node(cdd_cst_node_t *node) {
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

/** @brief parser_state_t struct */
typedef struct parser_state_t {
  /** @brief pos field */
  cdd_token_list_t *list;
  /** @brief pos field */
  size_t pos;
  int err; /**< err */
} parser_state_t;

C_CDD_EXPORT cdd_c_error_t peek(parser_state_t *s, cdd_token_t **out_tok);
C_CDD_EXPORT cdd_c_error_t peek(parser_state_t *s, cdd_token_t **out_tok) {
  cdd_c_error_t rc = CDD_C_SUCCESS;

  *out_tok = NULL;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos];
    return 0;
  }
  return CDD_C_ERROR_NOT_FOUND;
}

C_CDD_EXPORT cdd_c_error_t advance(parser_state_t *s, cdd_token_t **out_tok);
C_CDD_EXPORT cdd_c_error_t advance(parser_state_t *s, cdd_token_t **out_tok) {
  cdd_c_error_t rc = CDD_C_SUCCESS;

  *out_tok = NULL;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos++];
    return 0;
  }
  return CDD_C_ERROR_NOT_FOUND;
}

static cdd_c_error_t parse_block(parser_state_t *s, cdd_cst_node_t *parent,
                                 cdd_cst_node_t **out_node);
static cdd_c_error_t parse_declaration_or_statement(parser_state_t *s,
                                                    cdd_cst_node_t *parent,
                                                    cdd_cst_node_t **out_node);

static cdd_c_error_t parse_block(parser_state_t *s, cdd_cst_node_t *parent,
                                 cdd_cst_node_t **out_node) {
  cdd_c_error_t rc = CDD_C_SUCCESS, app_rc;

  cdd_token_t *t = NULL;
  cdd_cst_node_t *b = NULL;
  rc = alloc_node(CDD_CST_BLOCK, parent, &b);
  if (rc != CDD_C_SUCCESS) {
    s->err = rc;
    return rc;
  }

  advance(s, &t); /* { */
  rc = append_child_token(b, t);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (s->pos < s->list->size) {

    peek(s, &t);
    if (t->kind == CDD_TOKEN_RBRACE)
      break;
    {
      cdd_cst_node_t *child = NULL;
      rc = parse_declaration_or_statement(s, b, &child);
      if (rc != CDD_C_SUCCESS)
        return rc;
      app_rc = append_child_node(b, child);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(child);
        s->err = app_rc;
        free_node(b);
        *out_node = NULL;
        return app_rc;
      }
    }
  }

  advance(s, &t); /* } */
  if (t) {
    rc = append_child_token(b, t);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  *out_node = b;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t get_class_name(cdd_cst_node_t *node,
                                    cdd_token_t **out_tok) {
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
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_declaration_or_statement(parser_state_t *s,
                                                    cdd_cst_node_t *parent,
                                                    cdd_cst_node_t **out_node) {
  cdd_c_error_t rc = CDD_C_SUCCESS, app_rc, class_rc;

  cdd_cst_node_t *n;
  cdd_token_t *t = NULL;

  peek(s, &t);

  /* Note: cdd_token.h does not currently map #if specifically to its own token;
   * they come through as identifiers `# if` or `#if`. For now, we process
   * IFDEF/IFNDEF and ELIF. */
  if (t && (t->kind == CDD_TOKEN_PREPROC_IFDEF ||
            t->kind == CDD_TOKEN_PREPROC_IFNDEF ||
            t->kind == CDD_TOKEN_PREPROC_ELIF)) {
    cdd_token_t *p = NULL;

    advance(s, &p);

    rc = alloc_node(CDD_CST_PREPROC_CONDITIONAL, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = append_child_token(n, p);
    if (rc != CDD_C_SUCCESS)
      return rc;
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;

      peek(s, &nxt);
      if (nxt->kind == CDD_TOKEN_PREPROC_ENDIF) {

        advance(s, &t);
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        break;
      }
      if (nxt->kind == CDD_TOKEN_PREPROC_ELIF ||
          nxt->kind == CDD_TOKEN_PREPROC_ELSE) {
        /* Keep nested in current conditional until ENDIF, parsing linearly */
      }
      if (nxt->kind == CDD_TOKEN_LBRACE) {
        {
          cdd_cst_node_t *child = NULL;
          rc = parse_block(s, n, &child);
          if (rc != CDD_C_SUCCESS)
            return rc;
          app_rc = append_child_node(n, child);

          if (app_rc != CDD_C_SUCCESS) {
            free_node(child);
            s->err = app_rc;
            free_node(n);
            *out_node = NULL;
            return app_rc;
          }
        }
      } else {

        advance(s, &t);
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t && t->kind >= CDD_TOKEN_PREPROC_INCLUDE &&
      t->kind <= CDD_TOKEN_PREPROC_PRAGMA) {
    rc = alloc_node(CDD_CST_PREPROC_DIRECTIVE, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    advance(s, &t);
    rc = append_child_token(n, t);
    if (rc != CDD_C_SUCCESS)
      return rc;

    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t && t->kind == CDD_TOKEN_LBRACE) {
    return parse_block(s, parent, out_node);
  }

  if (t && t->kind == CDD_TOKEN_KEYWORD_TEMPLATE) {
    rc = alloc_node(CDD_CST_TEMPLATE_DECLARATION, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    advance(s, &t); /* template */
    rc = append_child_token(n, t);
    if (rc != CDD_C_SUCCESS)
      return rc;

    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_LT) {
      cdd_cst_node_t *param_list;
      rc = alloc_node(CDD_CST_TEMPLATE_PARAMETER_LIST, n, &param_list);
      if (rc != CDD_C_SUCCESS)
        return rc;

      advance(s, &t); /* < */
      rc = append_child_token(param_list, t);
      if (rc != CDD_C_SUCCESS)
        return rc;

      while (s->pos < s->list->size) {

        peek(s, &t);
        if (t->kind == CDD_TOKEN_GT) {

          advance(s, &t);
          rc = append_child_token(param_list, t);
          if (rc != CDD_C_SUCCESS)
            return rc;
          break;
        }
        if (t->kind == CDD_TOKEN_COMMA) {

          advance(s, &t);
          rc = append_child_token(param_list, t);
          if (rc != CDD_C_SUCCESS)
            return rc;
          continue;
        }

        /* Parameter: [class|typename] Identifier */
        if (t->kind == CDD_TOKEN_KEYWORD_CLASS ||
            t->kind == CDD_TOKEN_KEYWORD_TYPENAME ||
            t->kind == CDD_TOKEN_IDENTIFIER) {
          cdd_cst_node_t *param;
          rc = alloc_node(CDD_CST_TEMPLATE_PARAMETER, param_list, &param);
          if (rc != CDD_C_SUCCESS)
            return rc;

          advance(s, &t);
          rc = append_child_token(param, t);
          if (rc != CDD_C_SUCCESS)
            return rc;

          peek(s, &t);
          if (t && t->kind == CDD_TOKEN_IDENTIFIER) {

            advance(s, &t);
            rc = append_child_token(param, t);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          app_rc = append_child_node(param_list, param);

          if (app_rc != CDD_C_SUCCESS) {
            free_node(param);
            s->err = app_rc;
            free_node(param_list);
            *out_node = NULL;
            return app_rc;
          }
        } else {

          advance(s, &t);
          rc = append_child_token(param_list, t);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
      app_rc = append_child_node(n, param_list);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(param_list);
        s->err = app_rc;
        free_node(n);
        *out_node = NULL;
        return app_rc;
      }
    }

    /* Now parse the class or function it decorates */
    {
      cdd_cst_node_t *child = NULL;
      rc = parse_declaration_or_statement(s, n, &child);
      if (rc != CDD_C_SUCCESS)
        return rc;
      app_rc = append_child_node(n, child);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(child);
        s->err = app_rc;
        free_node(n);
        *out_node = NULL;
        return app_rc;
      }
    }

    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t && t->kind == CDD_TOKEN_KEYWORD_NAMESPACE) {
    rc = alloc_node(CDD_CST_NAMESPACE_DECLARATION, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    advance(s, &t); /* namespace */
    rc = append_child_token(n, t);
    if (rc != CDD_C_SUCCESS)
      return rc;

    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_IDENTIFIER) {

      advance(s, &t);
      rc = append_child_token(n, t);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }

    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_LBRACE) {
      cdd_cst_node_t *child = NULL;
      rc = parse_block(s, n, &child);
      if (rc != CDD_C_SUCCESS)
        return rc;
      app_rc = append_child_node(n, child);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(child);
        s->err = app_rc;
        free_node(n);
        *out_node = NULL;
        return app_rc;
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t && t->kind == CDD_TOKEN_KEYWORD_USING) {
    rc = alloc_node(CDD_CST_USING_DIRECTIVE, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;

      peek(s, &nxt);
      if (nxt->kind == CDD_TOKEN_SEMICOLON) {

        advance(s, &t);
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        break;
      }

      advance(s, &t);
      rc = append_child_token(n, t);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t && t->kind == CDD_TOKEN_KEYWORD_TRY) {
    rc = alloc_node(CDD_CST_TRY_BLOCK, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    advance(s, &t); /* try */
    rc = append_child_token(n, t);
    if (rc != CDD_C_SUCCESS)
      return rc;

    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_LBRACE) {
      cdd_cst_node_t *child = NULL;
      rc = parse_block(s, n, &child);
      if (rc != CDD_C_SUCCESS)
        return rc;
      app_rc = append_child_node(n, child);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(child);
        s->err = app_rc;
        free_node(n);
        *out_node = NULL;
        return app_rc;
      }
    }

    while (s->pos < s->list->size) {
      cdd_cst_node_t *catch_node = NULL;

      peek(s, &t);
      if (t->kind != CDD_TOKEN_KEYWORD_CATCH)
        break;

      rc = alloc_node(CDD_CST_CATCH_BLOCK, n, &catch_node);
      if (rc != CDD_C_SUCCESS)
        return rc;

      advance(s, &t); /* catch */
      rc = append_child_token(catch_node, t);
      if (rc != CDD_C_SUCCESS)
        return rc;

      peek(s, &t);
      if (t && t->kind == CDD_TOKEN_LPAREN) {
        while (s->pos < s->list->size) {

          advance(s, &t);
          rc = append_child_token(catch_node, t);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (t->kind == CDD_TOKEN_RPAREN)
            break;
        }
      }

      peek(s, &t);
      if (t && t->kind == CDD_TOKEN_LBRACE) {
        cdd_cst_node_t *child = NULL;
        rc = parse_block(s, catch_node, &child);
        if (rc != CDD_C_SUCCESS)
          return rc;
        app_rc = append_child_node(catch_node, child);

        if (app_rc != CDD_C_SUCCESS) {
          free_node(child);
          s->err = app_rc;
          free_node(catch_node);
          *out_node = NULL;
          return app_rc;
        }
      }
      app_rc = append_child_node(n, catch_node);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(catch_node);
        s->err = app_rc;
        free_node(n);
        *out_node = NULL;
        return app_rc;
      }
    }

    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t && t->kind == CDD_TOKEN_KEYWORD_THROW) {
    rc = alloc_node(CDD_CST_THROW_EXPRESSION, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    advance(s, &t); /* throw */
    rc = append_child_token(n, t);
    if (rc != CDD_C_SUCCESS)
      return rc;

    while (s->pos < s->list->size) {

      peek(s, &t);
      if (t->kind == CDD_TOKEN_SEMICOLON) {

        advance(s, &t);
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        break;
      }

      advance(s, &t);
      rc = append_child_token(n, t);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t && t->kind == CDD_TOKEN_KEYWORD_CLASS) {
    rc = alloc_node(CDD_CST_CLASS_DECLARATION, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;

      peek(s, &nxt);
      if (nxt->kind == CDD_TOKEN_LBRACE) {
        cdd_cst_node_t *child = NULL;
        rc = parse_block(s, n, &child);
        if (rc != CDD_C_SUCCESS)
          return rc;
        app_rc = append_child_node(n, child);

        if (app_rc != CDD_C_SUCCESS) {
          free_node(child);
          s->err = app_rc;
          free_node(n);
          *out_node = NULL;
          return app_rc;
        }
      } else if (nxt->kind == CDD_TOKEN_COLON) {
        cdd_cst_node_t *base_list;
        rc = alloc_node(CDD_CST_BASE_CLASS_LIST, n, &base_list);
        if (rc != CDD_C_SUCCESS)
          return rc;

        advance(s, &t); /* ':' */
        rc = append_child_token(base_list, t);
        if (rc != CDD_C_SUCCESS)
          return rc;

        while (s->pos < s->list->size) {
          cdd_cst_node_t *base_spec;
          rc = alloc_node(CDD_CST_BASE_CLASS_SPECIFIER, base_list, &base_spec);
          if (rc != CDD_C_SUCCESS)
            return rc;

          /* parse access modifier or virtual */

          peek(s, &nxt);
          if (nxt && (nxt->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
                      nxt->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
                      nxt->kind == CDD_TOKEN_KEYWORD_PROTECTED ||
                      nxt->kind == CDD_TOKEN_KEYWORD_VIRTUAL)) {

            advance(s, &t);
            rc = append_child_token(base_spec, t);
            if (rc != CDD_C_SUCCESS)
              return rc;
            /* might have virtual and access modifier in either order */

            peek(s, &nxt);
            if (nxt && (nxt->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
                        nxt->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
                        nxt->kind == CDD_TOKEN_KEYWORD_PROTECTED ||
                        nxt->kind == CDD_TOKEN_KEYWORD_VIRTUAL)) {

              advance(s, &t);
              rc = append_child_token(base_spec, t);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
          /* base class name */

          peek(s, &nxt);
          if (nxt && nxt->kind == CDD_TOKEN_IDENTIFIER) {

            advance(s, &t);
            rc = append_child_token(base_spec, t);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          app_rc = append_child_node(base_list, base_spec);

          if (app_rc != CDD_C_SUCCESS) {
            free_node(base_spec);
            s->err = app_rc;
            free_node(base_list);
            *out_node = NULL;
            return app_rc;
          }

          peek(s, &nxt);
          if (nxt && nxt->kind == CDD_TOKEN_COMMA) {

            advance(s, &t);
            rc = append_child_token(base_list, t);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            break;
          }
        }
        app_rc = append_child_node(n, base_list);

        if (app_rc != CDD_C_SUCCESS) {
          free_node(base_list);
          s->err = app_rc;
          free_node(n);
          *out_node = NULL;
          return app_rc;
        }
      } else if (nxt->kind == CDD_TOKEN_SEMICOLON) {

        advance(s, &t);
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        break;
      } else {

        advance(s, &t);
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t && (t->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
            t->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
            t->kind == CDD_TOKEN_KEYWORD_PROTECTED)) {
    rc = alloc_node(CDD_CST_ACCESS_SPECIFIER, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;

    advance(s, &t);
    rc = append_child_token(n, t);
    if (rc != CDD_C_SUCCESS)
      return rc;

    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_COLON) {

      advance(s, &t);
      rc = append_child_token(n, t);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t && t->kind == CDD_TOKEN_IDENTIFIER &&
      ((t->length == 7 && memcmp(t->start, "__asm__", 7) == 0) ||
       (t->length == 3 && memcmp(t->start, "asm", 3) == 0))) {
    rc = alloc_node(CDD_CST_ASM_STATEMENT, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;

      peek(s, &nxt);
      if (nxt->kind == CDD_TOKEN_SEMICOLON) {

        advance(s, &t);
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        break;
      }
      if (nxt->kind == CDD_TOKEN_RBRACE)
        break;

      advance(s, &t);
      if (t) {
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  {
    size_t i;
    int is_func = 0;
    int is_destructor = 0;
    int is_operator = 0;
    enum cdd_cst_node_kind_t node_kind = CDD_CST_UNKNOWN;
    cdd_token_t *class_name_tok = NULL;

    for (i = s->pos; i < s->list->size; i++) {
      if (s->list->tokens[i].kind == CDD_TOKEN_SEMICOLON) {
        break;
      }
      if (s->list->tokens[i].kind == CDD_TOKEN_LBRACE) {
        is_func = 1;
        break;
      }
      if (s->list->tokens[i].kind == CDD_TOKEN_RBRACE)
        break; /* syntax error fallback */
    }

    for (i = s->pos; i < s->list->size; i++) {
      if (s->list->tokens[i].kind == CDD_TOKEN_TILDE) {
        is_destructor = 1;
        break;
      }
      if (s->list->tokens[i].kind == CDD_TOKEN_KEYWORD_OPERATOR) {
        is_operator = 1;
        break;
      }
      if (s->list->tokens[i].kind == CDD_TOKEN_SEMICOLON ||
          s->list->tokens[i].kind == CDD_TOKEN_LBRACE) {
        break;
      }
    }

    node_kind = is_func ? CDD_CST_FUNCTION_DEFINITION : CDD_CST_UNKNOWN;
    if (is_destructor) {
      node_kind = CDD_CST_DESTRUCTOR;
    } else if (is_operator) {
      node_kind = CDD_CST_OPERATOR_OVERLOAD;
    } else {
      class_rc = get_class_name(parent, &class_name_tok);
      if (class_rc != CDD_C_SUCCESS) {
        return class_rc;
      }
      if (class_name_tok) {
        size_t paren_idx = 0;
        for (i = s->pos; i < s->list->size; i++) {
          if (s->list->tokens[i].kind == CDD_TOKEN_LPAREN) {
            paren_idx = i;
            break;
          }
          if (s->list->tokens[i].kind == CDD_TOKEN_SEMICOLON ||
              s->list->tokens[i].kind == CDD_TOKEN_LBRACE)
            break;
        }
        if (paren_idx > s->pos &&
            s->list->tokens[paren_idx - 1].kind == CDD_TOKEN_IDENTIFIER) {
          cdd_token_t *name_tok = &s->list->tokens[paren_idx - 1];
          if (name_tok->length == class_name_tok->length &&
              memcmp(name_tok->start, class_name_tok->start,
                     name_tok->length) == 0) {
            node_kind = CDD_CST_CONSTRUCTOR;
          }
        }
      }
    }

    if (is_func || node_kind != CDD_CST_UNKNOWN) {
      rc = alloc_node(node_kind, parent, &n);
      if (rc != CDD_C_SUCCESS)
        return rc;
      while (s->pos < s->list->size) {
        cdd_token_t *nxt = NULL;

        peek(s, &nxt);

        if (nxt->kind == CDD_TOKEN_KEYWORD_NOEXCEPT) {
          cdd_cst_node_t *noexcept_node = NULL;
          rc = alloc_node(CDD_CST_NOEXCEPT_SPECIFIER, n, &noexcept_node);
          if (rc != CDD_C_SUCCESS)
            return rc;

          advance(s, &t);
          rc = append_child_token(noexcept_node, t);
          if (rc != CDD_C_SUCCESS)
            return rc;

          peek(s, &nxt);
          if (nxt && nxt->kind == CDD_TOKEN_LPAREN) {
            int noexcept_paren = 0;
            while (s->pos < s->list->size) {

              peek(s, &nxt);
              if (nxt->kind == CDD_TOKEN_LPAREN)
                noexcept_paren++;
              else if (nxt->kind == CDD_TOKEN_RPAREN)
                noexcept_paren--;

              advance(s, &t);
              rc = append_child_token(noexcept_node, t);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (noexcept_paren == 0)
                break;
            }
          }
          app_rc = append_child_node(n, noexcept_node);

          if (app_rc != CDD_C_SUCCESS) {
            free_node(noexcept_node);
            s->err = app_rc;
            free_node(n);
            *out_node = NULL;
            return app_rc;
          }
          continue;
        }

        if (nxt->kind == CDD_TOKEN_LBRACE) {
          cdd_cst_node_t *child = NULL;
          rc = parse_block(s, n, &child);
          if (rc != CDD_C_SUCCESS)
            return rc;
          app_rc = append_child_node(n, child);

          if (app_rc != CDD_C_SUCCESS) {
            free_node(child);
            s->err = app_rc;
            free_node(n);
            *out_node = NULL;
            return app_rc;
          }
          break;
        }
        if (!is_func && nxt->kind == CDD_TOKEN_SEMICOLON) {

          advance(s, &t);
          rc = append_child_token(n, t);
          if (rc != CDD_C_SUCCESS)
            return rc;
          break;
        }

        advance(s, &t);
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      *out_node = n;
      return CDD_C_SUCCESS;
    } else {
      rc = alloc_node(CDD_CST_UNKNOWN, parent, &n);
      if (rc != CDD_C_SUCCESS)
        return rc;
      {
        int paren_depth = 0;
        while (s->pos < s->list->size) {
          cdd_token_t *nxt = NULL;

          peek(s, &nxt);
          if (nxt->kind == CDD_TOKEN_LPAREN) {
            paren_depth++;
          } else if (nxt->kind == CDD_TOKEN_RPAREN) {
            paren_depth--;
          }
          if (nxt->kind == CDD_TOKEN_RBRACE && paren_depth <= 0) {
            if (n->num_children == 0) {
              free_node(n);
              *out_node = NULL;
              return CDD_C_ERROR_MEMORY;
            }
            break;
          }

          advance(s, &t);
          rc = append_child_token(n, t);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (nxt->kind == CDD_TOKEN_SEMICOLON && paren_depth <= 0)
            break;
        }
      }
      *out_node = n;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_cst_parse(az_span source, cdd_cst_tree_t **out_tree) {
  cdd_c_error_t rc = CDD_C_SUCCESS, app_rc;

  parser_state_t state = {0};
  cdd_cst_tree_t *tree;

  if (!out_tree)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  tree = (cdd_cst_tree_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_tree_t));
  if (!tree) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  rc = cdd_lexer_tokenize(source, &tree->base_tokens);
  if (rc != 0) {
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

  while (state.pos < state.list->size) {
    cdd_token_t *t = NULL;

    peek(&state, &t);
    if (t->kind == CDD_TOKEN_EOF) {

      advance(&state, &t);
      rc = append_child_token(tree->root, t);
      if (rc != CDD_C_SUCCESS) {
        state.err = rc;
      }
      break;
    }
    {
      cdd_cst_node_t *child = NULL;
      rc = parse_declaration_or_statement(&state, tree->root, &child);
      if (rc != CDD_C_SUCCESS) {
        state.err = rc;
        break;
      }
      app_rc = append_child_node(tree->root, child);

      if (app_rc != CDD_C_SUCCESS) {
        free_node(child);
        state.err = app_rc;
        break;
      }
    }
  }

  if (state.err) {
    cdd_cst_tree_free(tree);
    return state.err;
  }

  *out_tree = tree;
  return CDD_C_SUCCESS;
}

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
        /* Need to free trivia attached */
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
