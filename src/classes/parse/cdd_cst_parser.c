/* clang-format off */
#include "c_cdd/memory.h"
#include "cdd_cst_parser.h"
#include "cdd_lexer.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */

static enum cdd_c_error alloc_node(enum cdd_cst_node_kind_t kind,
                                   cdd_cst_node_t *parent,
                                   cdd_cst_node_t **out_node) {
  cdd_cst_node_t *n;
#ifdef CDD_BUILD_TESTS
  extern int g_cdd_cst_alloc_node_fail;
  if (g_cdd_cst_alloc_node_fail && --g_cdd_cst_alloc_node_fail == 0)
    n = NULL;
  else
#endif
    n = (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
  if (n) {
    n->kind = kind;
    n->parent = parent;
  }
  *out_node = n;
  return CDD_C_SUCCESS;
}

static enum cdd_c_error append_child_token(cdd_cst_node_t *node,
                                           cdd_token_t *tok) {
  if (node->num_children >= node->capacity) {
    size_t new_cap = node->capacity == 0 ? 8 : node->capacity * 2;
    cdd_cst_child_t *new_arr;
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_realloc_fail;
    if (g_cdd_cst_realloc_fail && --g_cdd_cst_realloc_fail == 0)
      new_arr = NULL;
    else
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

static enum cdd_c_error append_child_node(cdd_cst_node_t *node,
                                          cdd_cst_node_t *child) {
  if (node->num_children >= node->capacity) {
    size_t new_cap = node->capacity == 0 ? 8 : node->capacity * 2;
    cdd_cst_child_t *new_arr;
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_realloc_fail;
    if (g_cdd_cst_realloc_fail && --g_cdd_cst_realloc_fail == 0)
      new_arr = NULL;
    else
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

typedef struct cdd_macro_def_t {
  char *name;
  char *value;
} cdd_macro_def_t;

typedef struct cdd_macro_env_t {
  cdd_macro_def_t *defs;
  size_t count;
  size_t capacity;
} cdd_macro_env_t;

/** @brief parser_state_t struct */
typedef struct parser_state_t {
  /** @brief pos field */
  cdd_token_list_t *list;
  /** @brief pos field */
  size_t pos;
  int err;                /**< err */
  cdd_macro_env_t macros; /**< local macro evaluation environment */
} parser_state_t;

C_CDD_EXPORT enum cdd_c_error peek(parser_state_t *s, cdd_token_t **out_tok);
C_CDD_EXPORT enum cdd_c_error peek(parser_state_t *s, cdd_token_t **out_tok) {
  if (!s || !out_tok)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_tok = NULL;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos];
    return 0;
  }
  return CDD_C_ERROR_NOT_FOUND;
}

C_CDD_EXPORT enum cdd_c_error advance(parser_state_t *s, cdd_token_t **out_tok);
C_CDD_EXPORT enum cdd_c_error advance(parser_state_t *s,
                                      cdd_token_t **out_tok) {
  if (!s || !out_tok)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_tok = NULL;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos++];
    return 0;
  }
  return CDD_C_ERROR_NOT_FOUND;
}

static enum cdd_c_error parse_block(parser_state_t *s, cdd_cst_node_t *parent,
                                    cdd_cst_node_t **out_node);
static enum cdd_c_error
parse_declaration_or_statement(parser_state_t *s, cdd_cst_node_t *parent,
                               cdd_cst_node_t **out_node);

static enum cdd_c_error parse_block(parser_state_t *s, cdd_cst_node_t *parent,
                                    cdd_cst_node_t **out_node) {
  cdd_token_t *t = NULL;
  cdd_cst_node_t *b = NULL;
  {
    enum cdd_c_error _rc = alloc_node(CDD_CST_BLOCK, parent, &b);
    if (_rc != CDD_C_SUCCESS) {
      s->err = _rc;
      return _rc;
    }
  }

  {
    enum cdd_c_error _rc = advance(s, &t);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  } /* { */
  if (t) {
    enum cdd_c_error _rc = append_child_token(b, t);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  while (s->pos < s->list->size) {
    {
      enum cdd_c_error _rc = peek(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    if (!t || t->kind == CDD_TOKEN_RBRACE)
      break;
    {
      cdd_cst_node_t *child = NULL;
      {
        enum cdd_c_error _rc = parse_declaration_or_statement(s, b, &child);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (child) {
        {
          enum cdd_c_error _rc = append_child_node(b, child);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      } else if (s->err) {
        break;
      }
    }
  }

  {
    enum cdd_c_error _rc = advance(s, &t);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  } /* } */
  if (t) {
    enum cdd_c_error _rc = append_child_token(b, t);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }
  *out_node = b;
  return CDD_C_SUCCESS;
}

#if 0
static enum cdd_c_error eval_preproc_expr(parser_state_t *s, size_t start_pos,
                                          size_t end_pos, int *out_val) {
  /* Simple placeholder for now: evaluate defined(X), 1, or 0.
   * A full boolean expression parser requires an expression grammar tree.
   * For the immediate milestone, we implement symbol lookup and literal int
   * evaluation. */
  size_t i;

  if (start_pos >= end_pos) {
    *out_val = 0;
    return CDD_C_SUCCESS;
  }

  /* Fallback single token evaluation */
  if (end_pos - start_pos == 1) {
    cdd_token_t *t = &s->list->tokens[start_pos];
    if (t->kind == CDD_TOKEN_NUMBER) {
      if (t->length == 1 && t->start[0] == '0') {
        *out_val = 0;
      } else {
        *out_val = 1;
      }
      return CDD_C_SUCCESS;
    } else if (t->kind == CDD_TOKEN_IDENTIFIER) {
      /* Macro value check */
      size_t k;
      for (k = 0; k < s->macros.count; k++) {
        if (strlen(s->macros.defs[k].name) == t->length &&
            strncmp(s->macros.defs[k].name, (const char *)t->start,
                    t->length) == 0) {
          *out_val = 1; /* Found */
          return CDD_C_SUCCESS;
        }
      }
      *out_val = 0;
      return CDD_C_SUCCESS;
    }
  }

  /* Scan for "defined ( X )" or "defined X" */
  for (i = start_pos; i < end_pos; i++) {
    cdd_token_t *t = &s->list->tokens[i];
    if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 7 &&
        strncmp((const char *)t->start, "defined", 7) == 0) {
      size_t target_idx = i + 1;
      if (target_idx < end_pos &&
          s->list->tokens[target_idx].kind == CDD_TOKEN_LPAREN) {
        target_idx++;
      }
      if (target_idx < end_pos &&
          s->list->tokens[target_idx].kind == CDD_TOKEN_IDENTIFIER) {
        size_t k;
        cdd_token_t *target = &s->list->tokens[target_idx];
        *out_val = 0;
        for (k = 0; k < s->macros.count; k++) {
          if (strlen(s->macros.defs[k].name) == target->length &&
              strncmp(s->macros.defs[k].name, (const char *)target->start,
                      target->length) == 0) {
            *out_val = 1;
            break;
          }
        }
        return CDD_C_SUCCESS;
      }
    }
  }

  /* Default unresolvable expression to 0 (false) */
  *out_val = 0;
  return CDD_C_SUCCESS;
}

#endif

#if 0
static enum cdd_c_error parse_preproc_conditional(parser_state_t *s, cdd_cst_node_t *parent, cdd_cst_node_t **out_node) {
  cdd_cst_node_t *node = NULL; alloc_node(CDD_CST_PREPROC_CONDITIONAL, parent, &node);
  if (!node) {
    s->err = CDD_C_ERROR_MEMORY;
    *out_node = NULL; return CDD_C_ERROR_MEMORY;
  }

  while (s->pos < s->list->size) {
    cdd_token_t *t = NULL; peek(s, &t);
    if (!t)
      break;
    if (t->kind == CDD_TOKEN_PREPROC_ENDIF) {
      append_child_token(node, advance(s));
      break;
    } else if (t->kind == CDD_TOKEN_PREPROC_ELSE ||
               t->kind == CDD_TOKEN_PREPROC_IFDEF ||
               t->kind == CDD_TOKEN_PREPROC_IFNDEF) {
      if (t->kind == CDD_TOKEN_PREPROC_IFDEF ||
          t->kind == CDD_TOKEN_PREPROC_IFNDEF) {
        cdd_cst_node_t *child = NULL; parse_preproc_conditional(s, node, &child);
        if (child) {
          { enum cdd_c_error _rc = append_child_node(node, child); if (_rc != CDD_C_SUCCESS) return _rc; }
        }
      } else {
        append_child_token(node, advance(s));
      }
    } else {
      cdd_cst_node_t *child = NULL; parse_declaration_or_statement(s, node, &child);
      if (child) {
        { enum cdd_c_error _rc = append_child_node(node, child); if (_rc != CDD_C_SUCCESS) return _rc; }
      } else if (s->err) {
        break;
      }
    }
  }
  return node;
}
#endif

static enum cdd_c_error get_class_name(cdd_cst_node_t *node,
                                       cdd_token_t **out_tok, int *found) {
  if (!found)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *found = 0;
  while (node) {
    if (node->kind == CDD_CST_CLASS_DECLARATION) {
      size_t i;
      for (i = 0; i < node->num_children; i++) {
        if (node->children[i].kind == CDD_CST_CHILD_TOKEN &&
            node->children[i].val.token->kind == CDD_TOKEN_IDENTIFIER) {
          *out_tok = node->children[i].val.token;
          *found = 1;
          return CDD_C_SUCCESS;
        }
      }
    }
    node = node->parent;
  }
  return CDD_C_SUCCESS;
}

static enum cdd_c_error
parse_declaration_or_statement(parser_state_t *s, cdd_cst_node_t *parent,
                               cdd_cst_node_t **out_node) {
  cdd_cst_node_t *n;
  cdd_token_t *t = NULL;
  {
    enum cdd_c_error _rc = peek(s, &t);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }
#if 0
  if (!t) {
    *out_node = NULL;
    return CDD_C_ERROR_MEMORY;
  }
#endif

  /* Note: cdd_token.h does not currently map #if specifically to its own token;
   * they come through as identifiers `# if` or `#if`. For now, we process
   * IFDEF/IFNDEF and ELIF. */
  if (t->kind == CDD_TOKEN_PREPROC_IFDEF ||
      t->kind == CDD_TOKEN_PREPROC_IFNDEF ||
      t->kind == CDD_TOKEN_PREPROC_ELIF) {
    cdd_token_t *p = NULL;
    int is_if_elif = (t->kind == CDD_TOKEN_PREPROC_ELIF);
    size_t expr_start = s->pos + 1;
    size_t expr_end = expr_start;

    {
      enum cdd_c_error _rc = advance(s, &p);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }

#if 0
    if (is_if_elif) {
      /* advance to end of logical line for the expression evaluation */
      while (expr_end < s->list->size &&
             s->list->tokens[expr_end].kind != CDD_TOKEN_OTHER) {
        /* simplistic boundary check for expressions on single lines */
        if (s->list->tokens[expr_end].length == 1 &&
            s->list->tokens[expr_end].start[0] == '\n')
          break;
        expr_end++;
      }
      {
        int val = 0;
        eval_preproc_expr(s, expr_start, expr_end, &val);
        /* If we wanted a fully evaluating engine, we could skip/keep nodes
         * based on val here. */
        (void)val;
      }
    }

#endif
    {
      enum cdd_c_error _rc =
          alloc_node(CDD_CST_PREPROC_CONDITIONAL, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    {
      enum cdd_c_error _rc = append_child_token(n, p);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;
      {
        enum cdd_c_error _rc = peek(s, &nxt);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
#if 0
      if (!nxt)
        break;
#endif
      if (nxt->kind == CDD_TOKEN_PREPROC_ENDIF) {
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        {
          enum cdd_c_error _rc = append_child_token(n, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        break;
      }
      if (nxt->kind == CDD_TOKEN_PREPROC_ELIF ||
          nxt->kind == CDD_TOKEN_PREPROC_ELSE) {
        /* Keep nested in current conditional until ENDIF, parsing linearly */
      }
      if (nxt->kind == CDD_TOKEN_LBRACE) {
        {
          cdd_cst_node_t *child = NULL;
          {
            enum cdd_c_error _rc = parse_block(s, n, &child);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
          if (child) {
            {
              enum cdd_c_error _rc = append_child_node(n, child);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
          }
        }
      } else {
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        {
          enum cdd_c_error _rc = append_child_token(n, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind >= CDD_TOKEN_PREPROC_INCLUDE &&
      t->kind <= CDD_TOKEN_PREPROC_PRAGMA) {
    {
      enum cdd_c_error _rc = alloc_node(CDD_CST_PREPROC_DIRECTIVE, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    if (n) {
      {
        enum cdd_c_error _rc = advance(s, &t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        enum cdd_c_error _rc = append_child_token(n, t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }

      if (t->kind == CDD_TOKEN_PREPROC_DEFINE) {
        cdd_token_t *macro_name_tok = NULL;
        {
          enum cdd_c_error _rc = peek(s, &macro_name_tok);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        if (macro_name_tok && macro_name_tok->kind == CDD_TOKEN_IDENTIFIER) {
          /* Add to local environment */
          if (s->macros.count >= s->macros.capacity) {
            size_t new_cap =
                s->macros.capacity == 0 ? 16 : s->macros.capacity * 2;
            cdd_macro_def_t *new_arr = (cdd_macro_def_t *)C_CDD_REALLOC(
                s->macros.defs, new_cap * sizeof(cdd_macro_def_t));
            if (new_arr) {
              s->macros.defs = new_arr;
              s->macros.capacity = new_cap;

              /* Temporarily just capture name, we'll parse the rest as child
               * tokens */
              s->macros.defs[s->macros.count].name =
                  (char *)C_CDD_MALLOC(macro_name_tok->length + 1);
              if (s->macros.defs[s->macros.count].name) {
                memcpy(s->macros.defs[s->macros.count].name,
                       macro_name_tok->start, macro_name_tok->length);
                s->macros.defs[s->macros.count].name[macro_name_tok->length] =
                    '\0';
                s->macros.defs[s->macros.count].value = NULL;
                s->macros.count++;
              }
            }
          }
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
    {
      enum cdd_c_error _rc =
          alloc_node(CDD_CST_TEMPLATE_DECLARATION, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    {
      enum cdd_c_error _rc = advance(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    } /* template */
    {
      enum cdd_c_error _rc = append_child_token(n, t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }

    {
      enum cdd_c_error _rc = peek(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    if (t && t->kind == CDD_TOKEN_LT) {
      cdd_cst_node_t *param_list;
      {
        enum cdd_c_error _rc =
            alloc_node(CDD_CST_TEMPLATE_PARAMETER_LIST, n, &param_list);
        if (_rc != CDD_C_SUCCESS) {
          s->err = _rc;
          return _rc;
        }
      }
      if (param_list) {
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        } /* < */
        {
          enum cdd_c_error _rc = append_child_token(param_list, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }

        while (s->pos < s->list->size) {
          {
            enum cdd_c_error _rc = peek(s, &t);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
          if (!t)
            break;
          if (t->kind == CDD_TOKEN_GT) {
            {
              enum cdd_c_error _rc = advance(s, &t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            {
              enum cdd_c_error _rc = append_child_token(param_list, t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            break;
          }
          if (t->kind == CDD_TOKEN_COMMA) {
            {
              enum cdd_c_error _rc = advance(s, &t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            {
              enum cdd_c_error _rc = append_child_token(param_list, t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            continue;
          }

          /* Parameter: [class|typename] Identifier */
          if (t->kind == CDD_TOKEN_KEYWORD_CLASS ||
              t->kind == CDD_TOKEN_KEYWORD_TYPENAME ||
              t->kind == CDD_TOKEN_IDENTIFIER) {
            cdd_cst_node_t *param;
            {
              enum cdd_c_error _rc =
                  alloc_node(CDD_CST_TEMPLATE_PARAMETER, param_list, &param);
              if (_rc != CDD_C_SUCCESS) {
                s->err = _rc;
                return _rc;
              }
            }
            if (param) {
              {
                enum cdd_c_error _rc = advance(s, &t);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
              {
                enum cdd_c_error _rc = append_child_token(param, t);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }

              {
                enum cdd_c_error _rc = peek(s, &t);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
              if (t && t->kind == CDD_TOKEN_IDENTIFIER) {
                {
                  enum cdd_c_error _rc = advance(s, &t);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
                {
                  enum cdd_c_error _rc = append_child_token(param, t);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
              }
              {
                enum cdd_c_error _rc = append_child_node(param_list, param);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
            }
          } else {
            {
              enum cdd_c_error _rc = advance(s, &t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            {
              enum cdd_c_error _rc = append_child_token(param_list, t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
          }
        }
        {
          enum cdd_c_error _rc = append_child_node(n, param_list);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }

    /* Now parse the class or function it decorates */
    {
      cdd_cst_node_t *child = NULL;
      {
        enum cdd_c_error _rc = parse_declaration_or_statement(s, n, &child);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (child) {
        {
          enum cdd_c_error _rc = append_child_node(n, child);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }

    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_NAMESPACE) {
    {
      enum cdd_c_error _rc =
          alloc_node(CDD_CST_NAMESPACE_DECLARATION, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    {
      enum cdd_c_error _rc = advance(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    } /* namespace */
    {
      enum cdd_c_error _rc = append_child_token(n, t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    {
      enum cdd_c_error _rc = peek(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    if (t && t->kind == CDD_TOKEN_IDENTIFIER) {
      {
        enum cdd_c_error _rc = advance(s, &t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        enum cdd_c_error _rc = append_child_token(n, t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
    {
      enum cdd_c_error _rc = peek(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    if (t && t->kind == CDD_TOKEN_LBRACE) {
      cdd_cst_node_t *child = NULL;
      {
        enum cdd_c_error _rc = parse_block(s, n, &child);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (child) {
        {
          enum cdd_c_error _rc = append_child_node(n, child);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_USING) {
    {
      enum cdd_c_error _rc = alloc_node(CDD_CST_USING_DIRECTIVE, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;
      {
        enum cdd_c_error _rc = peek(s, &nxt);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
#if 0
      if (!nxt)
        break;
#endif
      if (nxt->kind == CDD_TOKEN_SEMICOLON) {
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        {
          enum cdd_c_error _rc = append_child_token(n, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        break;
      }
      {
        enum cdd_c_error _rc = advance(s, &t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        enum cdd_c_error _rc = append_child_token(n, t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_TRY) {
    {
      enum cdd_c_error _rc = alloc_node(CDD_CST_TRY_BLOCK, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    {
      enum cdd_c_error _rc = advance(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    } /* try */
    {
      enum cdd_c_error _rc = append_child_token(n, t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }

    {
      enum cdd_c_error _rc = peek(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    if (t && t->kind == CDD_TOKEN_LBRACE) {
      cdd_cst_node_t *child = NULL;
      {
        enum cdd_c_error _rc = parse_block(s, n, &child);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (child) {
        {
          enum cdd_c_error _rc = append_child_node(n, child);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }

    while (s->pos < s->list->size) {
      cdd_cst_node_t *catch_node = NULL;

      {
        enum cdd_c_error _rc = peek(s, &t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (!t || t->kind != CDD_TOKEN_KEYWORD_CATCH)
        break;

      {
        enum cdd_c_error _rc = alloc_node(CDD_CST_CATCH_BLOCK, n, &catch_node);
        if (_rc != CDD_C_SUCCESS) {
          s->err = _rc;
          return _rc;
        }
      }
      if (catch_node) {
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        } /* catch */
        {
          enum cdd_c_error _rc = append_child_token(catch_node, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }

        {
          enum cdd_c_error _rc = peek(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        if (t && t->kind == CDD_TOKEN_LPAREN) {
          while (s->pos < s->list->size) {
            {
              enum cdd_c_error _rc = advance(s, &t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            {
              enum cdd_c_error _rc = append_child_token(catch_node, t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            if (t->kind == CDD_TOKEN_RPAREN)
              break;
          }
        }

        {
          enum cdd_c_error _rc = peek(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        if (t && t->kind == CDD_TOKEN_LBRACE) {
          cdd_cst_node_t *child = NULL;
          {
            enum cdd_c_error _rc = parse_block(s, catch_node, &child);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
          if (child) {
            {
              enum cdd_c_error _rc = append_child_node(catch_node, child);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
          }
        }
        {
          enum cdd_c_error _rc = append_child_node(n, catch_node);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      } else {
        break;
      }
    }

    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_THROW) {
    {
      enum cdd_c_error _rc = alloc_node(CDD_CST_THROW_EXPRESSION, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    {
      enum cdd_c_error _rc = advance(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    } /* throw */
    {
      enum cdd_c_error _rc = append_child_token(n, t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }

    while (s->pos < s->list->size) {
      {
        enum cdd_c_error _rc = peek(s, &t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (!t)
        break;
      if (t->kind == CDD_TOKEN_SEMICOLON) {
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        {
          enum cdd_c_error _rc = append_child_token(n, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        break;
      }
      {
        enum cdd_c_error _rc = advance(s, &t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        enum cdd_c_error _rc = append_child_token(n, t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_CLASS) {
    {
      enum cdd_c_error _rc = alloc_node(CDD_CST_CLASS_DECLARATION, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;
      {
        enum cdd_c_error _rc = peek(s, &nxt);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
#if 0
      if (!nxt)
        break;
#endif
      if (nxt->kind == CDD_TOKEN_LBRACE) {
        cdd_cst_node_t *child = NULL;
        {
          enum cdd_c_error _rc = parse_block(s, n, &child);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        if (child) {
          {
            enum cdd_c_error _rc = append_child_node(n, child);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
        }
      } else if (nxt->kind == CDD_TOKEN_COLON) {
        cdd_cst_node_t *base_list;
        {
          enum cdd_c_error _rc =
              alloc_node(CDD_CST_BASE_CLASS_LIST, n, &base_list);
          if (_rc != CDD_C_SUCCESS) {
            s->err = _rc;
            return _rc;
          }
        }
        if (base_list) {
          {
            enum cdd_c_error _rc = advance(s, &t);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          } /* ':' */
          {
            enum cdd_c_error _rc = append_child_token(base_list, t);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }

          while (s->pos < s->list->size) {
            cdd_cst_node_t *base_spec;
            {
              enum cdd_c_error _rc = alloc_node(CDD_CST_BASE_CLASS_SPECIFIER,
                                                base_list, &base_spec);
              if (_rc != CDD_C_SUCCESS) {
                s->err = _rc;
                return _rc;
              }
            }
            if (base_spec) {
              /* parse access modifier or virtual */
              {
                enum cdd_c_error _rc = peek(s, &nxt);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
              if (nxt && (nxt->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
                          nxt->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
                          nxt->kind == CDD_TOKEN_KEYWORD_PROTECTED ||
                          nxt->kind == CDD_TOKEN_KEYWORD_VIRTUAL)) {
                {
                  enum cdd_c_error _rc = advance(s, &t);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
                {
                  enum cdd_c_error _rc = append_child_token(base_spec, t);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
                /* might have virtual and access modifier in either order */
                {
                  enum cdd_c_error _rc = peek(s, &nxt);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
                if (nxt && (nxt->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
                            nxt->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
                            nxt->kind == CDD_TOKEN_KEYWORD_PROTECTED ||
                            nxt->kind == CDD_TOKEN_KEYWORD_VIRTUAL)) {
                  {
                    enum cdd_c_error _rc = advance(s, &t);
                    if (_rc != CDD_C_SUCCESS)
                      return _rc;
                  }
                  {
                    enum cdd_c_error _rc = append_child_token(base_spec, t);
                    if (_rc != CDD_C_SUCCESS)
                      return _rc;
                  }
                }
              }
              /* base class name */
              {
                enum cdd_c_error _rc = peek(s, &nxt);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
              if (nxt && nxt->kind == CDD_TOKEN_IDENTIFIER) {
                {
                  enum cdd_c_error _rc = advance(s, &t);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
                {
                  enum cdd_c_error _rc = append_child_token(base_spec, t);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
              }
              {
                enum cdd_c_error _rc = append_child_node(base_list, base_spec);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
            }

            {
              enum cdd_c_error _rc = peek(s, &nxt);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            if (nxt && nxt->kind == CDD_TOKEN_COMMA) {
              {
                enum cdd_c_error _rc = advance(s, &t);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
              {
                enum cdd_c_error _rc = append_child_token(base_list, t);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
            } else {
              break;
            }
          }
          {
            enum cdd_c_error _rc = append_child_node(n, base_list);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
        }
      } else if (nxt->kind == CDD_TOKEN_SEMICOLON) {
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        {
          enum cdd_c_error _rc = append_child_token(n, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        break;
      } else {
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        {
          enum cdd_c_error _rc = append_child_token(n, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
      t->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
      t->kind == CDD_TOKEN_KEYWORD_PROTECTED) {
    {
      enum cdd_c_error _rc = alloc_node(CDD_CST_ACCESS_SPECIFIER, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    {
      enum cdd_c_error _rc = advance(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    {
      enum cdd_c_error _rc = append_child_token(n, t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    {
      enum cdd_c_error _rc = peek(s, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    if (t && t->kind == CDD_TOKEN_OTHER && t->length == 1 && *t->start == ':') {
      {
        enum cdd_c_error _rc = advance(s, &t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        enum cdd_c_error _rc = append_child_token(n, t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_IDENTIFIER &&
      ((t->length == 7 && memcmp(t->start, "__asm__", 7) == 0) ||
       (t->length == 3 && memcmp(t->start, "asm", 3) == 0))) {
    {
      enum cdd_c_error _rc = alloc_node(CDD_CST_ASM_STATEMENT, parent, &n);
      if (_rc != CDD_C_SUCCESS) {
        s->err = _rc;
        return _rc;
      }
    }
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;
      {
        enum cdd_c_error _rc = peek(s, &nxt);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (nxt->kind == CDD_TOKEN_SEMICOLON) {
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        {
          enum cdd_c_error _rc = append_child_token(n, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        break;
      }
      if (nxt->kind == CDD_TOKEN_RBRACE)
        break;
      {
        enum cdd_c_error _rc = advance(s, &t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (t) {
        enum cdd_c_error _rc = append_child_token(n, t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
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
      int _found = 0;
      enum cdd_c_error _rc = get_class_name(parent, &class_name_tok, &_found);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
      if (_found) {
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
      {
        enum cdd_c_error _rc = alloc_node(node_kind, parent, &n);
        if (_rc != CDD_C_SUCCESS) {
          s->err = _rc;
          return _rc;
        }
      }
      while (s->pos < s->list->size) {
        cdd_token_t *nxt = NULL;
        {
          enum cdd_c_error _rc = peek(s, &nxt);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }

        if (nxt->kind == CDD_TOKEN_KEYWORD_NOEXCEPT) {
          cdd_cst_node_t *noexcept_node = NULL;
          {
            enum cdd_c_error _rc =
                alloc_node(CDD_CST_NOEXCEPT_SPECIFIER, n, &noexcept_node);
            if (_rc != CDD_C_SUCCESS) {
              s->err = _rc;
              return _rc;
            }
          }
          if (noexcept_node) {
            {
              enum cdd_c_error _rc = advance(s, &t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            {
              enum cdd_c_error _rc = append_child_token(noexcept_node, t);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }

            {
              enum cdd_c_error _rc = peek(s, &nxt);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            if (nxt && nxt->kind == CDD_TOKEN_LPAREN) {
              int noexcept_paren = 0;
              while (s->pos < s->list->size) {
                {
                  enum cdd_c_error _rc = peek(s, &nxt);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
                if (nxt->kind == CDD_TOKEN_LPAREN)
                  noexcept_paren++;
                else if (nxt->kind == CDD_TOKEN_RPAREN)
                  noexcept_paren--;

                {
                  enum cdd_c_error _rc = advance(s, &t);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
                {
                  enum cdd_c_error _rc = append_child_token(noexcept_node, t);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
                if (noexcept_paren == 0)
                  break;
              }
            }
            {
              enum cdd_c_error _rc = append_child_node(n, noexcept_node);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
            continue;
          }
        }

        if (nxt->kind == CDD_TOKEN_LBRACE) {
          cdd_cst_node_t *child = NULL;
          {
            enum cdd_c_error _rc = parse_block(s, n, &child);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
          if (child) {
            {
              enum cdd_c_error _rc = append_child_node(n, child);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
          }
          break;
        }
        if (!is_func && nxt->kind == CDD_TOKEN_SEMICOLON) {
          {
            enum cdd_c_error _rc = advance(s, &t);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
          {
            enum cdd_c_error _rc = append_child_token(n, t);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
          break;
        }
        {
          enum cdd_c_error _rc = advance(s, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        {
          enum cdd_c_error _rc = append_child_token(n, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
      *out_node = n;
      return CDD_C_SUCCESS;
    } else {
      {
        enum cdd_c_error _rc = alloc_node(CDD_CST_UNKNOWN, parent, &n);
        if (_rc != CDD_C_SUCCESS) {
          s->err = _rc;
          return _rc;
        }
      }
      {
        int paren_depth = 0;
        while (s->pos < s->list->size) {
          cdd_token_t *nxt = NULL;
          {
            enum cdd_c_error _rc = peek(s, &nxt);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
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
          {
            enum cdd_c_error _rc = advance(s, &t);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
          {
            enum cdd_c_error _rc = append_child_token(n, t);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
          if (nxt->kind == CDD_TOKEN_SEMICOLON && paren_depth <= 0)
            break;
        }
      }
      *out_node = n;
      return CDD_C_SUCCESS;
    }
  }
}

enum cdd_c_error cdd_cst_parse(az_span source, cdd_cst_tree_t **out_tree) {
  parser_state_t state = {0};
  cdd_cst_tree_t *tree;
  int rc;

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

  alloc_node(CDD_CST_TRANSLATION_UNIT, NULL, &tree->root);
  if (!tree->root) {
    cdd_cst_tree_free(tree);
    return CDD_C_ERROR_MEMORY;
  }

  while (state.pos < state.list->size) {
    cdd_token_t *t = NULL;
    {
      enum cdd_c_error _rc = peek(&state, &t);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    if (t->kind == CDD_TOKEN_EOF) {
      {
        enum cdd_c_error _rc = advance(&state, &t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        enum cdd_c_error _rc = append_child_token(tree->root, t);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      break;
    }
    {
      cdd_cst_node_t *child = NULL;
      {
        enum cdd_c_error _rc =
            parse_declaration_or_statement(&state, tree->root, &child);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (child) {
        {
          enum cdd_c_error _rc = append_child_node(tree->root, child);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      } else {
        if (state.err)
          break;
        /* Fallback */
        {
          enum cdd_c_error _rc = advance(&state, &t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        if (t) {
          enum cdd_c_error _rc = append_child_token(tree->root, t);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
  }

  if (state.err) {
    if (state.macros.defs) {
      size_t k;
      for (k = 0; k < state.macros.count; k++) {
        C_CDD_FREE(state.macros.defs[k].name);
        if (state.macros.defs[k].value)
          C_CDD_FREE(state.macros.defs[k].value);
      }
      C_CDD_FREE(state.macros.defs);
    }
    cdd_cst_tree_free(tree);
    return state.err;
  }

  if (state.macros.defs) {
    size_t k;
    for (k = 0; k < state.macros.count; k++) {
      C_CDD_FREE(state.macros.defs[k].name);
      if (state.macros.defs[k].value)
        C_CDD_FREE(state.macros.defs[k].value);
    }
    C_CDD_FREE(state.macros.defs);
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
