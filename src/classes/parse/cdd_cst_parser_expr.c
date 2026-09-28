/**
 * @file cdd_cst_parser_expr.c
 * @brief CST parser rules for expressions, declarations, classes, and
 * statements.
 *
 * @author Samuel Marks
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

cdd_c_error_t parse_class_or_statement(parser_state_t *s,
                                       cdd_cst_node_t *parent, cdd_token_t *t,
                                       cdd_cst_node_t **out_node) {
  cdd_c_error_t rc = CDD_C_SUCCESS, app_rc, class_rc;
  cdd_cst_node_t *n = NULL;
  *out_node = NULL;
  if (t->kind == CDD_TOKEN_KEYWORD_CLASS ||
      t->kind == CDD_TOKEN_KEYWORD_STRUCT) {
    rc = alloc_node(CDD_CST_CLASS_DECLARATION, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_node = n;
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;

      rc = peek(s, &nxt);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
      } else if (nxt->kind == CDD_TOKEN_COLON) {
        cdd_cst_node_t *base_list;
        rc = alloc_node(CDD_CST_BASE_CLASS_LIST, n, &base_list);
        if (rc != CDD_C_SUCCESS)
          return rc;
        app_rc = append_child_node(n, base_list);
        if (app_rc != CDD_C_SUCCESS) {
          free_node(base_list);
          return app_rc;
        }

        rc = advance(s, &t);
        if (rc != CDD_C_SUCCESS)
          return rc; /* ':' */
        rc = append_child_token(base_list, t);
        if (rc != CDD_C_SUCCESS)
          return rc;

        while (s->pos < s->list->size) {
          cdd_cst_node_t *base_spec;
          rc = alloc_node(CDD_CST_BASE_CLASS_SPECIFIER, base_list, &base_spec);
          if (rc != CDD_C_SUCCESS)
            return rc;
          app_rc = append_child_node(base_list, base_spec);
          if (app_rc != CDD_C_SUCCESS) {
            free_node(base_spec);
            return app_rc;
          }

          /* parse access modifier or virtual */

          rc = peek(s, &nxt);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if ((nxt->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
               nxt->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
               nxt->kind == CDD_TOKEN_KEYWORD_PROTECTED ||
               nxt->kind == CDD_TOKEN_KEYWORD_VIRTUAL)) {

            rc = advance(s, &t);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = append_child_token(base_spec, t);
            if (rc != CDD_C_SUCCESS)
              return rc;
            /* might have virtual and access modifier in either order */

            rc = peek(s, &nxt);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if ((nxt->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
                 nxt->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
                 nxt->kind == CDD_TOKEN_KEYWORD_PROTECTED ||
                 nxt->kind == CDD_TOKEN_KEYWORD_VIRTUAL)) {

              rc = advance(s, &t);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = append_child_token(base_spec, t);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
          /* base class name */

          rc = peek(s, &nxt);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (nxt->kind == CDD_TOKEN_IDENTIFIER) {

            rc = advance(s, &t);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = append_child_token(base_spec, t);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }

          rc = peek(s, &nxt);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (nxt->kind == CDD_TOKEN_COMMA) {

            rc = advance(s, &t);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = append_child_token(base_list, t);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            break;
          }
        }
      } else if (nxt->kind == CDD_TOKEN_SEMICOLON) {

        rc = advance(s, &t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        break;
      } else {

        rc = advance(s, &t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if ((t->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
       t->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
       t->kind == CDD_TOKEN_KEYWORD_PROTECTED)) {
    rc = alloc_node(CDD_CST_ACCESS_SPECIFIER, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_node = n;

    rc = advance(s, &t);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = append_child_token(n, t);
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = peek(s, &t);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (t->kind == CDD_TOKEN_COLON) {

      rc = advance(s, &t);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = append_child_token(n, t);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_IDENTIFIER &&
      ((t->length == 7 && memcmp(t->start, "__asm__", 7) == 0) ||
       (t->length == 3 && memcmp(t->start, "asm", 3) == 0))) {
    rc = alloc_node(CDD_CST_ASM_STATEMENT, parent, &n);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_node = n;
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;

      rc = peek(s, &nxt);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (nxt->kind == CDD_TOKEN_SEMICOLON) {

        rc = advance(s, &t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = append_child_token(n, t);
        if (rc != CDD_C_SUCCESS)
          return rc;
        break;
      }
      if (nxt->kind == CDD_TOKEN_RBRACE)
        break;

      rc = advance(s, &t);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = append_child_token(n, t);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
      *out_node = n;
      while (s->pos < s->list->size) {
        cdd_token_t *nxt = NULL;

        rc = peek(s, &nxt);
        if (rc != CDD_C_SUCCESS)
          return rc;

        if (nxt->kind == CDD_TOKEN_KEYWORD_NOEXCEPT) {
          cdd_cst_node_t *noexcept_node = NULL;
          rc = alloc_node(CDD_CST_NOEXCEPT_SPECIFIER, n, &noexcept_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
          app_rc = append_child_node(n, noexcept_node);
          if (app_rc != CDD_C_SUCCESS) {
            free_node(noexcept_node);
            return app_rc;
          }

          rc = advance(s, &t);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = append_child_token(noexcept_node, t);
          if (rc != CDD_C_SUCCESS)
            return rc;

          rc = peek(s, &nxt);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (nxt->kind == CDD_TOKEN_LPAREN) {
            int noexcept_paren = 0;
            while (s->pos < s->list->size) {

              rc = peek(s, &nxt);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (nxt->kind == CDD_TOKEN_LPAREN)
                noexcept_paren++;
              else if (nxt->kind == CDD_TOKEN_RPAREN)
                noexcept_paren--;

              rc = advance(s, &t);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = append_child_token(noexcept_node, t);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (noexcept_paren == 0)
                break;
            }
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
            s->err = (int)app_rc;
            free_node(n);
            *out_node = NULL;
            return app_rc;
          }
          break;
        }
        if (!is_func && nxt->kind == CDD_TOKEN_SEMICOLON) {

          rc = advance(s, &t);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = append_child_token(n, t);
          if (rc != CDD_C_SUCCESS)
            return rc;
          break;
        }

        rc = advance(s, &t);
        if (rc != CDD_C_SUCCESS)
          return rc;
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
      *out_node = n;
      {
        int paren_depth = 0;
        while (s->pos < s->list->size) {
          cdd_token_t *nxt = NULL;

          rc = peek(s, &nxt);
          if (rc != CDD_C_SUCCESS)
            return rc;
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

          rc = advance(s, &t);
          if (rc != CDD_C_SUCCESS)
            return rc;
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
}
