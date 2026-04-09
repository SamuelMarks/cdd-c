/* clang-format off */
#include "cdd_cst_parser.h"
#include "cdd_lexer.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static cdd_cst_node_t *alloc_node(enum cdd_cst_node_kind_t kind,
                                  cdd_cst_node_t *parent) {
  cdd_cst_node_t *n = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
  if (n) {
    n->kind = kind;
    n->parent = parent;
  }
  return n;
}

static int append_child_token(cdd_cst_node_t *node, cdd_token_t *tok) {
  if (node->num_children >= node->capacity) {
    size_t new_cap = node->capacity == 0 ? 8 : node->capacity * 2;
    cdd_cst_child_t *new_arr = (cdd_cst_child_t *)realloc(
        node->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr)
      return ENOMEM;
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
    if (!new_arr)
      return ENOMEM;
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

typedef struct parser_state_t {
  cdd_token_list_t *list;
  size_t pos;
  int err;
} parser_state_t;

static cdd_token_t *peek(parser_state_t *s) {
  if (s->pos < s->list->size)
    return &s->list->tokens[s->pos];
  return NULL;
}

static cdd_token_t *advance(parser_state_t *s) {
  if (s->pos < s->list->size)
    return &s->list->tokens[s->pos++];
  return NULL;
}

static cdd_cst_node_t *parse_block(parser_state_t *s, cdd_cst_node_t *parent);
static cdd_cst_node_t *parse_declaration_or_statement(parser_state_t *s,
                                                      cdd_cst_node_t *parent);

static cdd_cst_node_t *parse_block(parser_state_t *s, cdd_cst_node_t *parent) {
  cdd_cst_node_t *b = alloc_node(CDD_CST_BLOCK, parent);
  cdd_token_t *t;
  if (!b) {
    s->err = ENOMEM;
    return NULL;
  }

  t = advance(s); /* { */
  if (t)
    append_child_token(b, t);

  while (s->pos < s->list->size) {
    t = peek(s);
    if (!t || t->kind == CDD_TOKEN_RBRACE)
      break;
    {
      cdd_cst_node_t *child = parse_declaration_or_statement(s, b);
      if (child)
        append_child_node(b, child);
      else if (s->err)
        break;
    }
  }

  t = advance(s); /* } */
  if (t)
    append_child_token(b, t);
  return b;
}

static cdd_cst_node_t *parse_preproc_conditional(parser_state_t *s,
                                                 cdd_cst_node_t *parent) {
  cdd_cst_node_t *node = alloc_node(CDD_CST_PREPROC_CONDITIONAL, parent);
  if (!node) {
    s->err = ENOMEM;
    return NULL;
  }

  while (s->pos < s->list->size) {
    cdd_token_t *t = peek(s);
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
        cdd_cst_node_t *child = parse_preproc_conditional(s, node);
        if (child)
          append_child_node(node, child);
      } else {
        append_child_token(node, advance(s));
      }
    } else {
      cdd_cst_node_t *child = parse_declaration_or_statement(s, node);
      if (child)
        append_child_node(node, child);
      else if (s->err)
        break;
    }
  }
  return node;
}

static cdd_cst_node_t *parse_declaration_or_statement(parser_state_t *s,
                                                      cdd_cst_node_t *parent) {
  cdd_cst_node_t *n;
  cdd_token_t *t = peek(s);
  if (!t)
    return NULL;

  if (t->kind == CDD_TOKEN_PREPROC_IFDEF ||
      t->kind == CDD_TOKEN_PREPROC_IFNDEF) {
    cdd_token_t *p = advance(s);
    n = alloc_node(CDD_CST_PREPROC_CONDITIONAL, parent);
    if (!n) {
      s->err = ENOMEM;
      return NULL;
    }
    append_child_token(n, p);
    while (s->pos < s->list->size) {
      cdd_token_t *nxt = peek(s);
      if (!nxt)
        break;
      if (nxt->kind == CDD_TOKEN_PREPROC_ENDIF) {
        append_child_token(n, advance(s));
        break;
      }
      if (nxt->kind == CDD_TOKEN_LBRACE) {
        append_child_node(n, parse_block(s, n));
      } else {
        append_child_token(n, advance(s));
      }
    }
    return n;
  }

  if (t->kind >= CDD_TOKEN_PREPROC_INCLUDE &&
      t->kind <= CDD_TOKEN_PREPROC_PRAGMA) {
    n = alloc_node(CDD_CST_PREPROC_DIRECTIVE, parent);
    if (n)
      append_child_token(n, advance(s));
    return n;
  }

  if (t->kind == CDD_TOKEN_LBRACE) {
    return parse_block(s, parent);
  }

  {
    size_t i;
    int is_decl = 0;
    int is_func = 0;
    for (i = s->pos; i < s->list->size; i++) {
      if (s->list->tokens[i].kind == CDD_TOKEN_SEMICOLON) {
        is_decl = 1;
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
      n = alloc_node(CDD_CST_FUNCTION_DEFINITION, parent);
      if (!n) {
        s->err = ENOMEM;
        return NULL;
      }
      while (s->pos < s->list->size) {
        cdd_token_t *nxt = peek(s);
        if (nxt->kind == CDD_TOKEN_LBRACE) {
          append_child_node(n, parse_block(s, n));
          break;
        }
        append_child_token(n, advance(s));
      }
      return n;
    } else {
      n = alloc_node(CDD_CST_UNKNOWN, parent);
      if (!n) {
        s->err = ENOMEM;
        return NULL;
      }
      while (s->pos < s->list->size) {
        cdd_token_t *nxt = peek(s);
        if (nxt->kind == CDD_TOKEN_RBRACE)
          break;
        append_child_token(n, advance(s));
        if (nxt->kind == CDD_TOKEN_SEMICOLON)
          break;
      }
      return n;
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
  if (!tree)
    return ENOMEM;

  rc = cdd_lexer_tokenize(source, &tree->base_tokens);
  if (rc != 0) {
    free(tree);
    return rc;
  }

  state.list = tree->base_tokens;
  state.pos = 0;
  state.err = 0;

  tree->root = alloc_node(CDD_CST_TRANSLATION_UNIT, NULL);
  if (!tree->root) {
    cdd_cst_tree_free(tree);
    return ENOMEM;
  }

  while (state.pos < state.list->size) {
    cdd_token_t *t = peek(&state);
    if (!t)
      break;
    if (t->kind == CDD_TOKEN_EOF) {
      append_child_token(tree->root, advance(&state));
      break;
    }
    {
      cdd_cst_node_t *child =
          parse_declaration_or_statement(&state, tree->root);
      if (child) {
        append_child_node(tree->root, child);
      } else {
        if (state.err)
          break;
        /* Fallback */
        append_child_token(tree->root, advance(&state));
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
  free(tree);
}
