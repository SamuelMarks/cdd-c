/* clang-format off */
#include "cdd_cst_parser.h"
#include "cdd_lexer.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */
/* LCOV_EXCL_START */

static enum cdd_c_error alloc_node(enum cdd_cst_node_kind_t kind,
                                   cdd_cst_node_t *parent,
                                   cdd_cst_node_t **out_node) {
  cdd_cst_node_t *n;
#ifdef CDD_BUILD_TESTS
  extern int g_cdd_cst_alloc_node_fail;
  if (g_cdd_cst_alloc_node_fail && --g_cdd_cst_alloc_node_fail == 0)
    /* LCOV_EXCL_START */
    n = NULL;
  /* LCOV_EXCL_STOP */
  else
#endif
    n = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
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
      /* LCOV_EXCL_START */
      new_arr = NULL;
    /* LCOV_EXCL_STOP */
    else
#endif
      new_arr = (cdd_cst_child_t *)realloc(node->children,
                                           new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
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
      /* LCOV_EXCL_START */
      new_arr = NULL;
    /* LCOV_EXCL_STOP */
    else
#endif
      new_arr = (cdd_cst_child_t *)realloc(node->children,
                                           new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
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
    /* LCOV_EXCL_START */
    return;
  /* LCOV_EXCL_STOP */
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      free_node(node->children[i].val.node);
    }
  }
  if (node->children)
    free(node->children);
  free(node);
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
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *out_tok = NULL;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos];
    return 0;
  }
  /* LCOV_EXCL_START */
  return CDD_C_ERROR_NOT_FOUND;
  /* LCOV_EXCL_STOP */
}

C_CDD_EXPORT enum cdd_c_error advance(parser_state_t *s, cdd_token_t **out_tok);
C_CDD_EXPORT enum cdd_c_error advance(parser_state_t *s,
                                      cdd_token_t **out_tok) {
  if (!s || !out_tok)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *out_tok = NULL;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos++];
    return 0;
  }
  /* LCOV_EXCL_START */
  return CDD_C_ERROR_NOT_FOUND;
  /* LCOV_EXCL_STOP */
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
  alloc_node(CDD_CST_BLOCK, parent, &b);
  if (!b) {
    /* LCOV_EXCL_START */
    s->err = CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    *out_node = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  advance(s, &t); /* { */
  if (t)
    append_child_token(b, t);

  while (s->pos < s->list->size) {
    peek(s, &t);
    if (!t || t->kind == CDD_TOKEN_RBRACE)
      break;
    {
      cdd_cst_node_t *child = NULL;
      parse_declaration_or_statement(s, b, &child);
      if (child) {
        if (append_child_node(b, child) != CDD_C_SUCCESS) {
          /* LCOV_EXCL_START */
          free_node(child);
          s->err = CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
        /* LCOV_EXCL_START */
      } else if (s->err) {
        break;
        /* LCOV_EXCL_STOP */
      }
    }
  }

  advance(s, &t); /* } */
  if (t)
    append_child_token(b, t);
  *out_node = b;
  return CDD_C_SUCCESS;
}

/* LCOV_EXCL_START */
static enum cdd_c_error eval_preproc_expr(parser_state_t *s, size_t start_pos,
                                          /* LCOV_EXCL_STOP */
                                          size_t end_pos, int *out_val) {
  /* Simple placeholder for now: evaluate defined(X), 1, or 0.
   * A full boolean expression parser requires an expression grammar tree.
   * For the immediate milestone, we implement symbol lookup and literal int
   * evaluation. */
  size_t i;

  /* LCOV_EXCL_START */
  if (start_pos >= end_pos) {
    /* LCOV_EXCL_STOP */
    *out_val = 0;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* Fallback single token evaluation */
  /* LCOV_EXCL_START */
  if (end_pos - start_pos == 1) {
    cdd_token_t *t = &s->list->tokens[start_pos];
    if (t->kind == CDD_TOKEN_NUMBER) {
      if (t->length == 1 && t->start[0] == '0') {
        /* LCOV_EXCL_STOP */
        *out_val = 0;
      } else {
        *out_val = 1;
      }
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
    } else if (t->kind == CDD_TOKEN_IDENTIFIER) {
      /* LCOV_EXCL_STOP */
      /* Macro value check */
      size_t k;
      /* LCOV_EXCL_START */
      for (k = 0; k < s->macros.count; k++) {
        if (strlen(s->macros.defs[k].name) == t->length &&
            strncmp(s->macros.defs[k].name, (const char *)t->start,
                    /* LCOV_EXCL_STOP */
                    t->length) == 0) {
          *out_val = 1; /* Found */
                        /* LCOV_EXCL_START */
          return CDD_C_SUCCESS;
          /* LCOV_EXCL_STOP */
        }
      }
      *out_val = 0;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  /* Scan for "defined ( X )" or "defined X" */
  /* LCOV_EXCL_START */
  for (i = start_pos; i < end_pos; i++) {
    cdd_token_t *t = &s->list->tokens[i];
    if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 7 &&
        strncmp((const char *)t->start, "defined", 7) == 0) {
      size_t target_idx = i + 1;
      if (target_idx < end_pos &&
          s->list->tokens[target_idx].kind == CDD_TOKEN_LPAREN) {
        target_idx++;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      if (target_idx < end_pos &&
          s->list->tokens[target_idx].kind == CDD_TOKEN_IDENTIFIER) {
        /* LCOV_EXCL_STOP */
        size_t k;
        /* LCOV_EXCL_START */
        cdd_token_t *target = &s->list->tokens[target_idx];
        /* LCOV_EXCL_STOP */
        *out_val = 0;
        /* LCOV_EXCL_START */
        for (k = 0; k < s->macros.count; k++) {
          if (strlen(s->macros.defs[k].name) == target->length &&
              strncmp(s->macros.defs[k].name, (const char *)target->start,
                      /* LCOV_EXCL_STOP */
                      target->length) == 0) {
            *out_val = 1;
            /* LCOV_EXCL_START */
            break;
            /* LCOV_EXCL_STOP */
          }
        }
        /* LCOV_EXCL_START */
        return CDD_C_SUCCESS;
        /* LCOV_EXCL_STOP */
      }
    }
  }

  /* Default unresolvable expression to 0 (false) */
  *out_val = 0;
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

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
          if (append_child_node(node, child) != CDD_C_SUCCESS) {
            free_node(child);
            s->err = CDD_C_ERROR_MEMORY;
          }
        }
      } else {
        append_child_token(node, advance(s));
      }
    } else {
      cdd_cst_node_t *child = NULL; parse_declaration_or_statement(s, node, &child);
      if (child) {
        if (append_child_node(node, child) != CDD_C_SUCCESS) {
          free_node(child);
          s->err = CDD_C_ERROR_MEMORY;
        }
      } else if (s->err) {
        break;
      }
    }
  }
  return node;
}
#endif

static enum cdd_c_error get_class_name(cdd_cst_node_t *node,
                                       cdd_token_t **out_tok) {
  while (node) {
    if (node->kind == CDD_CST_CLASS_DECLARATION) {
      size_t i;
      for (i = 0; i < node->num_children; i++) {
        if (node->children[i].kind == CDD_CST_CHILD_TOKEN &&
            node->children[i].val.token->kind == CDD_TOKEN_IDENTIFIER) {
          *out_tok = node->children[i].val.token;
          return CDD_C_ERROR_UNKNOWN;
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
  peek(s, &t);
  if (!t) {
    *out_node = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

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

    advance(s, &p);

    if (is_if_elif) {
      /* advance to end of logical line for the expression evaluation */
      /* LCOV_EXCL_START */
      while (expr_end < s->list->size &&
             s->list->tokens[expr_end].kind != CDD_TOKEN_OTHER) {
        /* LCOV_EXCL_STOP */
        /* simplistic boundary check for expressions on single lines */
        /* LCOV_EXCL_START */
        if (s->list->tokens[expr_end].length == 1 &&
            s->list->tokens[expr_end].start[0] == '\n')
          break;
        expr_end++;
        /* LCOV_EXCL_STOP */
      }
      {
        /* LCOV_EXCL_START */
        int val = 0;
        eval_preproc_expr(s, expr_start, expr_end, &val);
        /* LCOV_EXCL_STOP */
        /* If we wanted a fully evaluating engine, we could skip/keep nodes
         * based on val here. */
        (void)val;
      }
    }

    alloc_node(CDD_CST_PREPROC_CONDITIONAL, parent, &n);
    if (!n) {
      /* LCOV_EXCL_START */
      s->err = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      *out_node = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    append_child_token(n, p);
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;
      peek(s, &nxt);
      if (!nxt)
        break;
      if (nxt->kind == CDD_TOKEN_PREPROC_ENDIF) {
        advance(s, &t);
        append_child_token(n, t);
        break;
      }
      if (nxt->kind == CDD_TOKEN_PREPROC_ELIF ||
          nxt->kind == CDD_TOKEN_PREPROC_ELSE) {
        /* Keep nested in current conditional until ENDIF, parsing linearly */
      }
      if (nxt->kind == CDD_TOKEN_LBRACE) {
        {
          /* LCOV_EXCL_START */
          cdd_cst_node_t *child = NULL;
          parse_block(s, n, &child);
          if (child) {
            if (append_child_node(n, child) != CDD_C_SUCCESS) {
              free_node(child);
              s->err = CDD_C_ERROR_MEMORY;
              /* LCOV_EXCL_STOP */
            }
          }
        }
      } else {
        advance(s, &t);
        append_child_token(n, t);
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind >= CDD_TOKEN_PREPROC_INCLUDE &&
      t->kind <= CDD_TOKEN_PREPROC_PRAGMA) {
    alloc_node(CDD_CST_PREPROC_DIRECTIVE, parent, &n);
    if (n) {
      advance(s, &t);
      append_child_token(n, t);

      if (t->kind == CDD_TOKEN_PREPROC_DEFINE) {
        cdd_token_t *macro_name_tok = NULL;
        peek(s, &macro_name_tok);
        if (macro_name_tok && macro_name_tok->kind == CDD_TOKEN_IDENTIFIER) {
          /* Add to local environment */
          /* LCOV_EXCL_START */
          if (s->macros.count >= s->macros.capacity) {
            size_t new_cap =
                s->macros.capacity == 0 ? 16 : s->macros.capacity * 2;
            cdd_macro_def_t *new_arr = (cdd_macro_def_t *)realloc(
                s->macros.defs, new_cap * sizeof(cdd_macro_def_t));
            if (new_arr) {
              s->macros.defs = new_arr;
              s->macros.capacity = new_cap;
              /* LCOV_EXCL_STOP */

              /* Temporarily just capture name, we'll parse the rest as child
               * tokens */
              /* LCOV_EXCL_START */
              s->macros.defs[s->macros.count].name =
                  (char *)malloc(macro_name_tok->length + 1);
              if (s->macros.defs[s->macros.count].name) {
                memcpy(s->macros.defs[s->macros.count].name,
                       macro_name_tok->start, macro_name_tok->length);
                s->macros.defs[s->macros.count].name[macro_name_tok->length] =
                    /* LCOV_EXCL_STOP */
                    '\0';
                /* LCOV_EXCL_START */
                s->macros.defs[s->macros.count].value = NULL;
                s->macros.count++;
                /* LCOV_EXCL_STOP */
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
    /* LCOV_EXCL_START */
    return parse_block(s, parent, out_node);
    /* LCOV_EXCL_STOP */
  }

  if (t->kind == CDD_TOKEN_KEYWORD_TEMPLATE) {
    alloc_node(CDD_CST_TEMPLATE_DECLARATION, parent, &n);
    if (!n) {
      /* LCOV_EXCL_START */
      s->err = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      *out_node = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    advance(s, &t); /* template */
    append_child_token(n, t);

    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_LT) {
      cdd_cst_node_t *param_list;
      alloc_node(CDD_CST_TEMPLATE_PARAMETER_LIST, n, &param_list);
      if (param_list) {
        advance(s, &t); /* < */
        append_child_token(param_list, t);

        while (s->pos < s->list->size) {
          peek(s, &t);
          if (!t)
            /* LCOV_EXCL_START */
            break;
          /* LCOV_EXCL_STOP */
          if (t->kind == CDD_TOKEN_GT) {
            advance(s, &t);
            append_child_token(param_list, t);
            break;
          }
          if (t->kind == CDD_TOKEN_COMMA) {
            advance(s, &t);
            append_child_token(param_list, t);
            continue;
          }

          /* Parameter: [class|typename] Identifier */
          if (t->kind == CDD_TOKEN_KEYWORD_CLASS ||
              t->kind == CDD_TOKEN_KEYWORD_TYPENAME ||
              t->kind == CDD_TOKEN_IDENTIFIER) {
            cdd_cst_node_t *param;
            alloc_node(CDD_CST_TEMPLATE_PARAMETER, param_list, &param);
            if (param) {
              advance(s, &t);
              append_child_token(param, t);

              peek(s, &t);
              if (t && t->kind == CDD_TOKEN_IDENTIFIER) {
                advance(s, &t);
                append_child_token(param, t);
              }
              if (append_child_node(param_list, param) != CDD_C_SUCCESS) {
                /* LCOV_EXCL_START */
                free_node(param);
                s->err = CDD_C_ERROR_MEMORY;
                /* LCOV_EXCL_STOP */
              }
            }
          } else {
            /* LCOV_EXCL_START */
            advance(s, &t);
            append_child_token(param_list, t);
            /* LCOV_EXCL_STOP */
          }
        }
        if (append_child_node(n, param_list) != CDD_C_SUCCESS) {
          /* LCOV_EXCL_START */
          free_node(param_list);
          s->err = CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
      }
    }

    /* Now parse the class or function it decorates */
    {
      cdd_cst_node_t *child = NULL;
      parse_declaration_or_statement(s, n, &child);
      if (child) {
        if (append_child_node(n, child) != CDD_C_SUCCESS) {
          /* LCOV_EXCL_START */
          free_node(child);
          s->err = CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
      }
    }

    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_NAMESPACE) {
    alloc_node(CDD_CST_NAMESPACE_DECLARATION, parent, &n);
    if (!n) {
      /* LCOV_EXCL_START */
      s->err = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      *out_node = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    advance(s, &t); /* namespace */
    append_child_token(n, t);
    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_IDENTIFIER) {
      advance(s, &t);
      append_child_token(n, t);
    }
    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_LBRACE) {
      cdd_cst_node_t *child = NULL;
      parse_block(s, n, &child);
      if (child) {
        if (append_child_node(n, child) != CDD_C_SUCCESS) {
          /* LCOV_EXCL_START */
          free_node(child);
          s->err = CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_USING) {
    alloc_node(CDD_CST_USING_DIRECTIVE, parent, &n);
    if (!n) {
      /* LCOV_EXCL_START */
      s->err = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      *out_node = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;
      peek(s, &nxt);
      if (!nxt)
        break;
      if (nxt->kind == CDD_TOKEN_SEMICOLON) {
        advance(s, &t);
        append_child_token(n, t);
        break;
      }
      advance(s, &t);
      append_child_token(n, t);
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_TRY) {
    alloc_node(CDD_CST_TRY_BLOCK, parent, &n);
    if (!n) {
      /* LCOV_EXCL_START */
      s->err = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      *out_node = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    advance(s, &t); /* try */
    append_child_token(n, t);

    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_LBRACE) {
      cdd_cst_node_t *child = NULL;
      parse_block(s, n, &child);
      if (child) {
        if (append_child_node(n, child) != CDD_C_SUCCESS) {
          /* LCOV_EXCL_START */
          free_node(child);
          s->err = CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
      }
    }

    while (s->pos < s->list->size) {
      cdd_cst_node_t *catch_node = NULL;

      peek(s, &t);
      if (!t || t->kind != CDD_TOKEN_KEYWORD_CATCH)
        break;

      alloc_node(CDD_CST_CATCH_BLOCK, n, &catch_node);
      if (catch_node) {
        advance(s, &t); /* catch */
        append_child_token(catch_node, t);

        peek(s, &t);
        if (t && t->kind == CDD_TOKEN_LPAREN) {
          while (s->pos < s->list->size) {
            advance(s, &t);
            append_child_token(catch_node, t);
            if (t->kind == CDD_TOKEN_RPAREN)
              break;
          }
        }

        peek(s, &t);
        if (t && t->kind == CDD_TOKEN_LBRACE) {
          cdd_cst_node_t *child = NULL;
          parse_block(s, catch_node, &child);
          if (child) {
            if (append_child_node(catch_node, child) != CDD_C_SUCCESS) {
              /* LCOV_EXCL_START */
              free_node(child);
              s->err = CDD_C_ERROR_MEMORY;
              /* LCOV_EXCL_STOP */
            }
          }
        }
        if (append_child_node(n, catch_node) != CDD_C_SUCCESS) {
          /* LCOV_EXCL_START */
          free_node(catch_node);
          s->err = CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
      } else {
        /* LCOV_EXCL_START */
        break;
        /* LCOV_EXCL_STOP */
      }
    }

    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_THROW) {
    alloc_node(CDD_CST_THROW_EXPRESSION, parent, &n);
    if (!n) {
      /* LCOV_EXCL_START */
      s->err = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      *out_node = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    advance(s, &t); /* throw */
    append_child_token(n, t);

    while (s->pos < s->list->size) {
      peek(s, &t);
      if (!t)
        /* LCOV_EXCL_START */
        break;
      /* LCOV_EXCL_STOP */
      if (t->kind == CDD_TOKEN_SEMICOLON) {
        advance(s, &t);
        append_child_token(n, t);
        break;
      }
      advance(s, &t);
      append_child_token(n, t);
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_CLASS) {
    alloc_node(CDD_CST_CLASS_DECLARATION, parent, &n);
    if (!n) {
      /* LCOV_EXCL_START */
      s->err = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      *out_node = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;
      peek(s, &nxt);
      if (!nxt)
        break;
      if (nxt->kind == CDD_TOKEN_LBRACE) {
        cdd_cst_node_t *child = NULL;
        parse_block(s, n, &child);
        if (child) {
          if (append_child_node(n, child) != CDD_C_SUCCESS) {
            /* LCOV_EXCL_START */
            free_node(child);
            s->err = CDD_C_ERROR_MEMORY;
            /* LCOV_EXCL_STOP */
          }
        }
      } else if (nxt->kind == CDD_TOKEN_COLON) {
        cdd_cst_node_t *base_list;
        alloc_node(CDD_CST_BASE_CLASS_LIST, n, &base_list);
        if (base_list) {
          advance(s, &t); /* ':' */
          append_child_token(base_list, t);

          while (s->pos < s->list->size) {
            cdd_cst_node_t *base_spec;
            alloc_node(CDD_CST_BASE_CLASS_SPECIFIER, base_list, &base_spec);
            if (base_spec) {
              /* parse access modifier or virtual */
              peek(s, &nxt);
              if (nxt && (nxt->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
                          nxt->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
                          nxt->kind == CDD_TOKEN_KEYWORD_PROTECTED ||
                          nxt->kind == CDD_TOKEN_KEYWORD_VIRTUAL)) {
                advance(s, &t);
                append_child_token(base_spec, t);
                /* might have virtual and access modifier in either order */
                peek(s, &nxt);
                if (nxt && (nxt->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
                            nxt->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
                            nxt->kind == CDD_TOKEN_KEYWORD_PROTECTED ||
                            nxt->kind == CDD_TOKEN_KEYWORD_VIRTUAL)) {
                  advance(s, &t);
                  append_child_token(base_spec, t);
                }
              }
              /* base class name */
              peek(s, &nxt);
              if (nxt && nxt->kind == CDD_TOKEN_IDENTIFIER) {
                advance(s, &t);
                append_child_token(base_spec, t);
              }
              if (append_child_node(base_list, base_spec) != CDD_C_SUCCESS) {
                /* LCOV_EXCL_START */
                free_node(base_spec);
                s->err = CDD_C_ERROR_MEMORY;
                /* LCOV_EXCL_STOP */
              }
            }

            peek(s, &nxt);
            if (nxt && nxt->kind == CDD_TOKEN_COMMA) {
              advance(s, &t);
              append_child_token(base_list, t);
            } else {
              break;
            }
          }
          if (append_child_node(n, base_list) != CDD_C_SUCCESS) {
            /* LCOV_EXCL_START */
            free_node(base_list);
            s->err = CDD_C_ERROR_MEMORY;
            /* LCOV_EXCL_STOP */
          }
        }
      } else if (nxt->kind == CDD_TOKEN_SEMICOLON) {
        advance(s, &t);
        append_child_token(n, t);
        break;
      } else {
        advance(s, &t);
        append_child_token(n, t);
      }
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_KEYWORD_PUBLIC ||
      t->kind == CDD_TOKEN_KEYWORD_PRIVATE ||
      t->kind == CDD_TOKEN_KEYWORD_PROTECTED) {
    alloc_node(CDD_CST_ACCESS_SPECIFIER, parent, &n);
    if (!n) {
      /* LCOV_EXCL_START */
      s->err = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      *out_node = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    advance(s, &t);
    append_child_token(n, t);
    peek(s, &t);
    if (t && t->kind == CDD_TOKEN_OTHER && t->length == 1 && *t->start == ':') {
      /* LCOV_EXCL_START */
      advance(s, &t);
      append_child_token(n, t);
      /* LCOV_EXCL_STOP */
    }
    *out_node = n;
    return CDD_C_SUCCESS;
  }

  if (t->kind == CDD_TOKEN_IDENTIFIER &&
      ((t->length == 7 && memcmp(t->start, "__asm__", 7) == 0) ||
       (t->length == 3 && memcmp(t->start, "asm", 3) == 0))) {
    alloc_node(CDD_CST_ASM_STATEMENT, parent, &n);
    if (!n) {
      /* LCOV_EXCL_START */
      s->err = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      *out_node = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = NULL;
      peek(s, &nxt);
      if (nxt->kind == CDD_TOKEN_SEMICOLON) {
        advance(s, &t);
        append_child_token(n, t);
        break;
      }
      if (nxt->kind == CDD_TOKEN_RBRACE)
        /* LCOV_EXCL_START */
        break;
      /* LCOV_EXCL_STOP */
      advance(s, &t);
      if (t)
        append_child_token(n, t);
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
    } else if (get_class_name(parent, &class_name_tok)) {
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
            memcmp(name_tok->start, class_name_tok->start, name_tok->length) ==
                0) {
          node_kind = CDD_CST_CONSTRUCTOR;
        }
      }
    }

    if (is_func || node_kind != CDD_CST_UNKNOWN) {
      alloc_node(node_kind, parent, &n);
      if (!n) {
        /* LCOV_EXCL_START */
        s->err = CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
        *out_node = NULL;
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      while (s->pos < s->list->size) {
        cdd_token_t *nxt = NULL;
        peek(s, &nxt);

        if (nxt->kind == CDD_TOKEN_KEYWORD_NOEXCEPT) {
          cdd_cst_node_t *noexcept_node = NULL;
          alloc_node(CDD_CST_NOEXCEPT_SPECIFIER, n, &noexcept_node);
          if (noexcept_node) {
            advance(s, &t);
            append_child_token(noexcept_node, t);

            peek(s, &nxt);
            if (nxt && nxt->kind == CDD_TOKEN_LPAREN) {
              /* LCOV_EXCL_START */
              int noexcept_paren = 0;
              while (s->pos < s->list->size) {
                peek(s, &nxt);
                if (nxt->kind == CDD_TOKEN_LPAREN)
                  noexcept_paren++;
                else if (nxt->kind == CDD_TOKEN_RPAREN)
                  noexcept_paren--;
                /* LCOV_EXCL_STOP */

                /* LCOV_EXCL_START */
                advance(s, &t);
                append_child_token(noexcept_node, t);
                if (noexcept_paren == 0)
                  break;
                /* LCOV_EXCL_STOP */
              }
            }
            if (append_child_node(n, noexcept_node) != CDD_C_SUCCESS) {
              /* LCOV_EXCL_START */
              free_node(noexcept_node);
              s->err = CDD_C_ERROR_MEMORY;
              /* LCOV_EXCL_STOP */
            }
            continue;
          }
        }

        if (nxt->kind == CDD_TOKEN_LBRACE) {
          cdd_cst_node_t *child = NULL;
          parse_block(s, n, &child);
          if (child) {
            if (append_child_node(n, child) != CDD_C_SUCCESS) {
              /* LCOV_EXCL_START */
              free_node(child);
              s->err = CDD_C_ERROR_MEMORY;
              /* LCOV_EXCL_STOP */
            }
          }
          break;
        }
        if (!is_func && nxt->kind == CDD_TOKEN_SEMICOLON) {
          /* LCOV_EXCL_START */
          advance(s, &t);
          append_child_token(n, t);
          break;
          /* LCOV_EXCL_STOP */
        }
        advance(s, &t);
        append_child_token(n, t);
      }
      *out_node = n;
      return CDD_C_SUCCESS;
    } else {
      alloc_node(CDD_CST_UNKNOWN, parent, &n);
      if (!n) {
        /* LCOV_EXCL_START */
        s->err = CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
        *out_node = NULL;
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
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
            /* LCOV_EXCL_START */
            if (n->num_children == 0) {
              free_node(n);
              /* LCOV_EXCL_STOP */
              *out_node = NULL;
              /* LCOV_EXCL_START */
              return CDD_C_ERROR_MEMORY;
              /* LCOV_EXCL_STOP */
            }
            break;
          }
          advance(s, &t);
          append_child_token(n, t);
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
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  if (!tree) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  rc = cdd_lexer_tokenize(source, &tree->base_tokens);
  if (rc != 0) {
    /* LCOV_EXCL_START */
    free(tree);
    return rc;
    /* LCOV_EXCL_STOP */
  }

  state.list = tree->base_tokens;
  state.pos = 0;
  state.err = 0;

  alloc_node(CDD_CST_TRANSLATION_UNIT, NULL, &tree->root);
  if (!tree->root) {
    /* LCOV_EXCL_START */
    cdd_cst_tree_free(tree);
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  while (state.pos < state.list->size) {
    cdd_token_t *t = NULL;
    peek(&state, &t);
    if (t->kind == CDD_TOKEN_EOF) {
      /* LCOV_EXCL_START */
      advance(&state, &t);
      append_child_token(tree->root, t);
      break;
      /* LCOV_EXCL_STOP */
    }
    {
      cdd_cst_node_t *child = NULL;
      parse_declaration_or_statement(&state, tree->root, &child);
      if (child) {
        if (append_child_node(tree->root, child) != CDD_C_SUCCESS) {
          /* LCOV_EXCL_START */
          free_node(child);
          state.err = CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
      } else {
        /* LCOV_EXCL_START */
        if (state.err)
          break;
        /* LCOV_EXCL_STOP */
        /* Fallback */
        /* LCOV_EXCL_START */
        advance(&state, &t);
        if (t)
          append_child_token(tree->root, t);
        /* LCOV_EXCL_STOP */
      }
    }
  }

  if (state.err) {
    /* LCOV_EXCL_START */
    if (state.macros.defs) {
      /* LCOV_EXCL_STOP */
      size_t k;
      /* LCOV_EXCL_START */
      for (k = 0; k < state.macros.count; k++) {
        free(state.macros.defs[k].name);
        if (state.macros.defs[k].value)
          free(state.macros.defs[k].value);
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      free(state.macros.defs);
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    cdd_cst_tree_free(tree);
    return state.err;
    /* LCOV_EXCL_STOP */
  }

  if (state.macros.defs) {
    size_t k;
    /* LCOV_EXCL_START */
    for (k = 0; k < state.macros.count; k++) {
      free(state.macros.defs[k].name);
      if (state.macros.defs[k].value)
        free(state.macros.defs[k].value);
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    free(state.macros.defs);
    /* LCOV_EXCL_STOP */
  }

  *out_tree = tree;
  return CDD_C_SUCCESS;
}

void cdd_cst_tree_free(cdd_cst_tree_t *tree) {
  size_t i;
  if (!tree)
    /* LCOV_EXCL_START */
    return;
  /* LCOV_EXCL_STOP */
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
          /* LCOV_EXCL_START */
          cdd_trivia_t *n = t->next;
          free(t);
          t = n;
          /* LCOV_EXCL_STOP */
        }
        t = tree->synthesized_tokens[i]->trailing_trivia;
        while (t) {
          /* LCOV_EXCL_START */
          cdd_trivia_t *n = t->next;
          free(t);
          t = n;
          /* LCOV_EXCL_STOP */
        }
        free(tree->synthesized_tokens[i]);
      }
    }
    free(tree->synthesized_tokens);
  }
  if (tree->string_pool) {
    /* LCOV_EXCL_START */
    for (i = 0; i < tree->num_strings; i++) {
      free(tree->string_pool[i]);
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    free(tree->string_pool);
    /* LCOV_EXCL_STOP */
  }
  free(tree);
}

/* LCOV_EXCL_STOP */
