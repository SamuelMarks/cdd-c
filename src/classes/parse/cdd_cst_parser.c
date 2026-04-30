/* clang-format off */
#include "cdd_cst_parser.h"
#include "cdd_lexer.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */

static int alloc_node(enum cdd_cst_node_kind_t kind, cdd_cst_node_t *parent,
                      cdd_cst_node_t **out_node) {
  cdd_cst_node_t *n = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
  if (n) {
    n->kind = kind;
    n->parent = parent;
  }
  *out_node = n;
  return 0;
}

static int append_child_token(cdd_cst_node_t *node, cdd_token_t *tok) {
  if (node->num_children >= node->capacity) {
    size_t new_cap = node->capacity == 0 ? 8 : node->capacity * 2;
    cdd_cst_child_t *new_arr = (cdd_cst_child_t *)realloc(
        node->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      LOG_DEBUG("ENOMEM: OOM in %s\n", __func__);
      return ENOMEM;
    }
    node->children = new_arr;
    node->capacity = new_cap;
  }
  node->children[node->num_children].kind = CDD_CST_CHILD_TOKEN;
  node->children[node->num_children].val.token = tok;
  node->num_children++;
  return 0;
}

static int append_child_node(cdd_cst_node_t *node, cdd_cst_node_t *child) {
  if (node->num_children >= node->capacity) {
    size_t new_cap = node->capacity == 0 ? 8 : node->capacity * 2;
    cdd_cst_child_t *new_arr = (cdd_cst_child_t *)realloc(
        node->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      LOG_DEBUG("ENOMEM: OOM in %s\n", __func__);
      return ENOMEM;
    }
    node->children = new_arr;
    node->capacity = new_cap;
  }
  child->parent = node;
  node->children[node->num_children].kind = CDD_CST_CHILD_NODE;
  node->children[node->num_children].val.node = child;
  node->num_children++;
  return 0;
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
    free(node->children);
  free(node);
}

/** @brief parser_state_t struct */
typedef struct parser_state_t {
  /** @brief pos field */
  cdd_token_list_t *list;
  /** @brief pos field */
  size_t pos;
  int err;
} parser_state_t;

static int peek(parser_state_t *s, cdd_token_t **out_tok) {
  if (!s || !out_tok)
    return EINVAL;
  *out_tok = NULL;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos];
    return 0;
  }
  return ENOENT;
}

static int advance(parser_state_t *s, cdd_token_t **out_tok) {
  if (!s || !out_tok)
    return EINVAL;
  *out_tok = NULL;
  if (s->pos < s->list->size) {
    *out_tok = &s->list->tokens[s->pos++];
    return 0;
  }
  return ENOENT;
}

static int parse_block(parser_state_t *s, cdd_cst_node_t *parent,
                       cdd_cst_node_t **out_node);
static int parse_declaration_or_statement(parser_state_t *s,
                                          cdd_cst_node_t *parent,
                                          cdd_cst_node_t **out_node);

static int parse_block(parser_state_t *s, cdd_cst_node_t *parent,
                       cdd_cst_node_t **out_node) {
  cdd_token_t *t;
  cdd_cst_node_t *b = NULL;
  alloc_node(CDD_CST_BLOCK, parent, &b);
  if (!b) {
    s->err = ENOMEM;
    *out_node = NULL;
    return ENOENT;
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
      if (child)
        append_child_node(b, child);
      else if (s->err)
        break;
    }
  }

  advance(s, &t); /* } */
  if (t)
    append_child_token(b, t);
  *out_node = b;
  return 0;
}

#if 0
static int parse_preproc_conditional(parser_state_t *s, cdd_cst_node_t *parent, cdd_cst_node_t **out_node) {
  cdd_cst_node_t *node = NULL; alloc_node(CDD_CST_PREPROC_CONDITIONAL, parent, &node);
  if (!node) {
    s->err = ENOMEM;
    *out_node = NULL; return ENOENT;
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
        if (child)
          append_child_node(node, child);
      } else {
        append_child_token(node, advance(s));
      }
    } else {
      cdd_cst_node_t *child = NULL; parse_declaration_or_statement(s, node, &child);
      if (child)
        append_child_node(node, child);
      else if (s->err)
        break;
    }
  }
  return node;
}
#endif

static int parse_declaration_or_statement(parser_state_t *s,
                                          cdd_cst_node_t *parent,
                                          cdd_cst_node_t **out_node) {
  cdd_cst_node_t *n;
  cdd_token_t *t = NULL;
  peek(s, &t);
  if (!t) {
    *out_node = NULL;
    return ENOENT;
  }

  if (t->kind == CDD_TOKEN_PREPROC_IFDEF ||
      t->kind == CDD_TOKEN_PREPROC_IFNDEF) {
    cdd_token_t *p = NULL;
    advance(s, &p);
    alloc_node(CDD_CST_PREPROC_CONDITIONAL, parent, &n);
    if (!n) {
      s->err = ENOMEM;
      *out_node = NULL;
      return ENOENT;
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
      if (nxt->kind == CDD_TOKEN_LBRACE) {
        {
          cdd_cst_node_t *child = NULL;
          parse_block(s, n, &child);
          if (child)
            append_child_node(n, child);
        }
      } else {
        advance(s, &t);
        append_child_token(n, t);
      }
    }
    *out_node = n;
    return 0;
  }

  if (t->kind >= CDD_TOKEN_PREPROC_INCLUDE &&
      t->kind <= CDD_TOKEN_PREPROC_PRAGMA) {
    alloc_node(CDD_CST_PREPROC_DIRECTIVE, parent, &n);
    if (n)
      advance(s, &t);
    append_child_token(n, t);
    *out_node = n;
    return 0;
  }

  if (t->kind == CDD_TOKEN_LBRACE) {
    return parse_block(s, parent, out_node);
  }

  if (t->kind == CDD_TOKEN_IDENTIFIER &&
      ((t->length == 7 && memcmp(t->start, "__asm__", 7) == 0) ||
       (t->length == 3 && memcmp(t->start, "asm", 3) == 0))) {
    alloc_node(CDD_CST_ASM_STATEMENT, parent, &n);
    if (!n) {
      s->err = ENOMEM;
      *out_node = NULL;
      return ENOENT;
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
        break;
      advance(s, &t);
      if (t)
        append_child_token(n, t);
    }
    *out_node = n;
    return 0;
  }

  {
    size_t i;

    int is_func = 0;
    for (i = s->pos; i < s->list->size; i++) {
      if (s->list->tokens[i].kind == CDD_TOKEN_SEMICOLON) {
        /* is_decl = 1; */
        break;
      }
      if (s->list->tokens[i].kind == CDD_TOKEN_LBRACE) {
        is_func = 1;
        break;
      }
      if (s->list->tokens[i].kind == CDD_TOKEN_RBRACE)
        break; /* syntax error fallback */
    }

    if (is_func) {
      alloc_node(CDD_CST_FUNCTION_DEFINITION, parent, &n);
      if (!n) {
        s->err = ENOMEM;
        *out_node = NULL;
        return ENOENT;
      }
      while (s->pos < s->list->size) {
        cdd_token_t *nxt = NULL;
        peek(s, &nxt);
        if (nxt->kind == CDD_TOKEN_LBRACE) {
          {
            cdd_cst_node_t *child = NULL;
            parse_block(s, n, &child);
            if (child)
              append_child_node(n, child);
          }
          break;
        }
        advance(s, &t);
        append_child_token(n, t);
      }
      *out_node = n;
      return 0;
    } else {
      alloc_node(CDD_CST_UNKNOWN, parent, &n);
      if (!n) {
        s->err = ENOMEM;
        *out_node = NULL;
        return ENOENT;
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
            if (n->num_children == 0) {
              free_node(n);
              *out_node = NULL;
              return ENOENT;
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
      return 0;
    }
  }
}

int cdd_cst_parse(az_span source, cdd_cst_tree_t **out_tree) {
  parser_state_t state = {0};
  cdd_cst_tree_t *tree;
  int rc;

  if (!out_tree)
    return EINVAL;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  if (!tree) {
    LOG_DEBUG("ENOMEM: OOM in %s\n", __func__);
    return ENOMEM;
  }

  rc = cdd_lexer_tokenize(source, &tree->base_tokens);
  if (rc != 0) {
    free(tree);
    return rc;
  }

  state.list = tree->base_tokens;
  state.pos = 0;
  state.err = 0;

  alloc_node(CDD_CST_TRANSLATION_UNIT, NULL, &tree->root);
  if (!tree->root) {
    cdd_cst_tree_free(tree);
    return ENOMEM;
  }

  while (state.pos < state.list->size) {
    cdd_token_t *t = NULL;
    peek(&state, &t);
    if (!t)
      break;
    if (t->kind == CDD_TOKEN_EOF) {
      advance(&state, &t);
      if (t)
        append_child_token(tree->root, t);
      break;
    }
    {
      cdd_cst_node_t *child = NULL;
      parse_declaration_or_statement(&state, tree->root, &child);
      if (child) {
        append_child_node(tree->root, child);
      } else {
        if (state.err)
          break;
        /* Fallback */
        advance(&state, &t);
        if (t)
          append_child_token(tree->root, t);
      }
    }
  }

  if (state.err) {
    cdd_cst_tree_free(tree);
    return state.err;
  }

  *out_tree = tree;
  return 0;
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
          free(t);
          t = n;
        }
        t = tree->synthesized_tokens[i]->trailing_trivia;
        while (t) {
          cdd_trivia_t *n = t->next;
          free(t);
          t = n;
        }
        free(tree->synthesized_tokens[i]);
      }
    }
    free(tree->synthesized_tokens);
  }
  if (tree->string_pool) {
    for (i = 0; i < tree->num_strings; i++) {
      free(tree->string_pool[i]);
    }
    free(tree->string_pool);
  }
  free(tree);
}
