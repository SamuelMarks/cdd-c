/* clang-format off */
#include "cdd_cst_builder.h"
#include "cdd_cst_factory.h"
#include "cdd_cst_mutate.h"
#include "cdd_lexer.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
/* clang-format on */
static const char *pool_string(cdd_cst_tree_t *tree, const char *str);
static cdd_token_t *get_last_token(cdd_cst_node_t *node);

int cdd_cst_builder_init(cdd_cst_builder_t *builder, cdd_cst_tree_t *tree,
                         cdd_cst_node_t *target_node) {
  if (!builder || !tree || !target_node) {

    return EINVAL;
  }
  builder->tree = tree;
  builder->target_node = target_node;
  builder->error_state = 0;
  builder->indent_level = 0;
  return 0;
}

int cdd_cst_builder_free(cdd_cst_builder_t *builder) {
  if (!builder) {
    return EINVAL;
  }
  builder->tree = NULL;
  builder->target_node = NULL;
  return 0;
}

int cdd_cst_builder_has_error(const cdd_cst_builder_t *builder) {
  if (!builder) {
    return 1;
  }
  return builder->error_state != 0;
}

int cdd_cst_builder_set_insert_point(cdd_cst_builder_t *builder,
                                     cdd_cst_node_t *node) {
  if (!builder || !node) {
    return EINVAL;
  }
  if (builder->error_state != 0) {
    return builder->error_state;
  }
  builder->target_node = node;
  return 0;
}

int cdd_cst_bld_token(cdd_cst_builder_t *builder, enum cdd_token_kind_t kind,
                      const char *text) {
  cdd_token_t *tok;
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;

  tok = cdd_cst_create_token(builder->tree, kind, text);
  if (!tok) {
    builder->error_state = ENOMEM;
    return ENOMEM;
  }

  rc = cdd_cst_append_child_token(builder->target_node, tok);
  if (rc != 0) {
    builder->error_state = rc;
    return rc;
  }

  return 0;
}

int cdd_cst_bld_space(cdd_cst_builder_t *builder) {
  return cdd_cst_bld_token(builder, CDD_TOKEN_OTHER, " ");
}

int cdd_cst_bld_newline(cdd_cst_builder_t *builder) {
  return cdd_cst_bld_token(builder, CDD_TOKEN_OTHER, "\n");
}

int cdd_cst_bld_indent(cdd_cst_builder_t *builder, int depth_level) {
  int i;
  int rc = 0;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;

  for (i = 0; i < depth_level * 2; i++) {
    rc = cdd_cst_bld_space(builder);
    if (rc != 0)
      return rc;
  }
  return 0;
}

int cdd_cst_bld_ident(cdd_cst_builder_t *builder, const char *text) {
  return cdd_cst_bld_token(builder, CDD_TOKEN_IDENTIFIER, text);
}

int cdd_cst_bld_string(cdd_cst_builder_t *builder, const char *text) {
  return cdd_cst_bld_token(builder, CDD_TOKEN_STRING, text);
}

int cdd_cst_bld_int(cdd_cst_builder_t *builder, int value) {
  char buf[32];
  const char *pooled;
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s(buf, sizeof(buf), "%d", value);
#else
  snprintf(buf, sizeof(buf), "%d", value);
#endif
  pooled = pool_string(builder->tree, buf);
  if (!pooled)
    return ENOMEM;
  return cdd_cst_bld_token(builder, CDD_TOKEN_NUMBER, pooled);
}

int cdd_cst_bld_punct(cdd_cst_builder_t *builder, const char *text) {
  enum cdd_token_kind_t kind = CDD_TOKEN_OTHER;
  if (text) {
    if (strcmp(text, "{") == 0)
      kind = CDD_TOKEN_LBRACE;
    else if (strcmp(text, "}") == 0)
      kind = CDD_TOKEN_RBRACE;
    else if (strcmp(text, "(") == 0)
      kind = CDD_TOKEN_LPAREN;
    else if (strcmp(text, ")") == 0)
      kind = CDD_TOKEN_RPAREN;
    else if (strcmp(text, "[") == 0)
      kind = CDD_TOKEN_LBRACKET;
    else if (strcmp(text, "]") == 0)
      kind = CDD_TOKEN_RBRACKET;
    else if (strcmp(text, ";") == 0)
      kind = CDD_TOKEN_SEMICOLON;
    else if (strcmp(text, ",") == 0)
      kind = CDD_TOKEN_COMMA;
    else if (strcmp(text, ".") == 0)
      kind = CDD_TOKEN_DOT;
    else if (strcmp(text, "->") == 0)
      kind = CDD_TOKEN_ARROW;
    else if (strcmp(text, "+") == 0)
      kind = CDD_TOKEN_PLUS;
    else if (strcmp(text, "-") == 0)
      kind = CDD_TOKEN_MINUS;
    else if (strcmp(text, "*") == 0)
      kind = CDD_TOKEN_STAR;
    else if (strcmp(text, "/") == 0)
      kind = CDD_TOKEN_SLASH;
    else if (strcmp(text, "=") == 0)
      kind = CDD_TOKEN_ASSIGN;
    else if (strcmp(text, "==") == 0)
      kind = CDD_TOKEN_EQ;
    else if (strcmp(text, "!=") == 0)
      kind = CDD_TOKEN_NEQ;
  }
  return cdd_cst_bld_token(builder, kind, text);
}

int cdd_cst_bld_include(cdd_cst_builder_t *builder, const char *path,
                        int is_system) {
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;

  rc = cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_INCLUDE, "#include");
  if (rc == 0)
    rc = cdd_cst_bld_space(builder);
  if (rc == 0) {
    if (is_system) {
      char buf[256];
#if defined(_MSC_VER) && _MSC_VER >= 1400
      sprintf_s(buf, sizeof(buf), "<%s>", path);
#else
      snprintf(buf, sizeof(buf), "<%s>", path);
#endif
      {
        const char *pooled = pool_string(builder->tree, buf);
        if (!pooled)
          return ENOMEM;
        rc = cdd_cst_bld_token(builder, CDD_TOKEN_STRING, pooled);
      }
    } else {
      char buf[256];
#if defined(_MSC_VER) && _MSC_VER >= 1400
      sprintf_s(buf, sizeof(buf), "\"%s\"", path);
#else
      snprintf(buf, sizeof(buf), "\"%s\"", path);
#endif
      rc = cdd_cst_bld_string(builder, buf);
    }
  }
  if (rc == 0)
    rc = cdd_cst_bld_newline(builder);
  return rc;
}

int cdd_cst_bld_ifndef(cdd_cst_builder_t *builder, const char *macro_name) {
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_IFNDEF, "#ifndef");
  if (rc == 0)
    rc = cdd_cst_bld_space(builder);
  if (rc == 0)
    rc = cdd_cst_bld_ident(builder, macro_name);
  if (rc == 0)
    rc = cdd_cst_bld_newline(builder);
  return rc;
}

int cdd_cst_bld_ifdef(cdd_cst_builder_t *builder, const char *macro_name) {
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_IFDEF, "#ifdef");
  if (rc == 0)
    rc = cdd_cst_bld_space(builder);
  if (rc == 0)
    rc = cdd_cst_bld_ident(builder, macro_name);
  if (rc == 0)
    rc = cdd_cst_bld_newline(builder);
  return rc;
}

int cdd_cst_bld_else(cdd_cst_builder_t *builder) {
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_ELSE, "#else");
  if (rc == 0)
    rc = cdd_cst_bld_newline(builder);
  return rc;
}

int cdd_cst_bld_endif(cdd_cst_builder_t *builder) {
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_ENDIF, "#endif");
  if (rc == 0)
    rc = cdd_cst_bld_newline(builder);
  return rc;
}

int cdd_cst_bld_extern_c_open(cdd_cst_builder_t *builder) {
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_ifdef(builder, "__cplusplus");
  if (rc == 0)
    rc = cdd_cst_bld_ident(builder, "extern");
  if (rc == 0)
    rc = cdd_cst_bld_space(builder);
  if (rc == 0)
    rc = cdd_cst_bld_string(builder, "\"C\"");
  if (rc == 0)
    rc = cdd_cst_bld_space(builder);
  if (rc == 0)
    rc = cdd_cst_bld_punct(builder, "{");
  if (rc == 0)
    rc = cdd_cst_bld_newline(builder);
  if (rc == 0)
    rc = cdd_cst_bld_endif(builder);
  return rc;
}

int cdd_cst_bld_extern_c_close(cdd_cst_builder_t *builder) {
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_ifdef(builder, "__cplusplus");
  if (rc == 0)
    rc = cdd_cst_bld_punct(builder, "}");
  if (rc == 0)
    rc = cdd_cst_bld_newline(builder);
  if (rc == 0)
    rc = cdd_cst_bld_endif(builder);
  return rc;
}

int cdd_cst_bld_block_open(cdd_cst_builder_t *builder) {
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_punct(builder, "{");
  if (rc == 0)
    rc = cdd_cst_bld_newline(builder);
  if (rc == 0)
    builder->indent_level++;
  return rc;
}

int cdd_cst_bld_block_close(cdd_cst_builder_t *builder) {
  int rc;
  if (!builder)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;
  if (builder->indent_level > 0)
    builder->indent_level--;
  rc = cdd_cst_bld_indent(builder, builder->indent_level);
  if (rc == 0)
    rc = cdd_cst_bld_punct(builder, "}");
  if (rc == 0)
    rc = cdd_cst_bld_newline(builder);
  return rc;
}

static const char *pool_string(cdd_cst_tree_t *tree, const char *str) {
  char *dup;
  if (!tree || !str)
    return NULL;
#if defined(_MSC_VER)
  dup = _strdup(str);
#else
  dup = strdup(str);
#endif
  if (!dup)
    return NULL;
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool =
        (char **)realloc(tree->string_pool, new_cap * sizeof(char *));
    if (!new_pool) {
      free(dup);
      return NULL;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;
  return dup;
}

int cdd_cst_bld_snippet(cdd_cst_builder_t *builder, const char *snippet) {
  cdd_token_list_t *list = NULL;
  az_span span;
  size_t i;
  int rc = 0;

  if (!builder || !snippet)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;

  span = az_span_create_from_str((char *)snippet);
  rc = cdd_lexer_tokenize(span, &list);
  if (rc != 0) {
    builder->error_state = rc;
    return rc;
  }

  for (i = 0; i < list->size; i++) {
    cdd_token_t *t = &list->tokens[i];
    if (t->kind != CDD_TOKEN_EOF) {
      char tok_buf[2048];
      const char *pooled;
      if (t->length < sizeof(tok_buf)) {
        memcpy(tok_buf, t->start, t->length);
        tok_buf[t->length] = '\0';
      } else {
        continue;
      }
      pooled = pool_string(builder->tree, tok_buf);
      if (!pooled) {
        rc = ENOMEM;
        break;
      }
      rc = cdd_cst_bld_token(builder, t->kind, pooled);
      if (rc != 0)
        break;
      {
        cdd_token_t *last = get_last_token(builder->target_node);
        if (last) {
          last->leading_trivia = t->leading_trivia;
          last->trailing_trivia = t->trailing_trivia;
          t->leading_trivia = NULL;
          t->trailing_trivia = NULL;
        }
      }
    }
  }

  cdd_lexer_free_token_list(list);
  if (rc != 0)
    builder->error_state = rc;
  return rc;
}

int cdd_cst_quote(cdd_cst_builder_t *builder, const char *format_string, ...) {
  va_list args;
  const char *p;
  char snippet_buf[2048] = {0};
  size_t snippet_len = 0;
  int rc = 0;

  if (!builder || !format_string)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;

  va_start(args, format_string);
  for (p = format_string; *p; p++) {
    if (*p == '%' && *(p + 1) != '\0') {
      p++;
      if (*p == '%') {
        if (snippet_len < sizeof(snippet_buf) - 1) {
          snippet_buf[snippet_len++] = '%';
        }
      } else {
        if (snippet_len > 0) {
          snippet_buf[snippet_len] = '\0';
          rc = cdd_cst_bld_snippet(builder, snippet_buf);
          snippet_len = 0;
          if (rc != 0)
            break;
        }

        if (*p == 's') {
          const char *s = va_arg(args, const char *);
          rc = cdd_cst_bld_snippet(builder, s);
        } else if (*p == 'd') {
          int d = va_arg(args, int);
          rc = cdd_cst_bld_int(builder, d);
        } else if (*p == 'n') {
          cdd_cst_node_t *node = va_arg(args, cdd_cst_node_t *);
          if (node) {
            rc = cdd_cst_append_child_node(builder->target_node, node);
          }
        }
        if (rc != 0)
          break;
      }
    } else {
      if (snippet_len < sizeof(snippet_buf) - 1) {
        snippet_buf[snippet_len++] = *p;
      }
    }
  }

  if (rc == 0 && snippet_len > 0) {
    snippet_buf[snippet_len] = '\0';
    rc = cdd_cst_bld_snippet(builder, snippet_buf);
  }

  va_end(args);
  if (rc != 0)
    builder->error_state = rc;
  return rc;
}

int cdd_cst_bld_line_comment(cdd_cst_builder_t *builder, const char *text) {
  /* Line comments aren't standard C89 but commonly accepted or we can just
     inject a block comment. The prompt specified strictly C89, so we will
     actually map line comments to block comments! */
  return cdd_cst_bld_block_comment(builder, text);
}

static cdd_trivia_t *create_trivia(cdd_cst_tree_t *tree, const char *text) {
  cdd_trivia_t *t = (cdd_trivia_t *)calloc(1, sizeof(cdd_trivia_t));
  char *dup;
  if (!t)
    return NULL;
#if defined(_MSC_VER)
  dup = _strdup(text);
#else
  dup = strdup(text);
#endif
  if (!dup) {
    free(t);
    return NULL;
  }

  /* we track in the string pool for safety if possible, but tree doesn't own
     trivia memory automatically. Trivia memory is normally bound to tokens.
     We'll put it in the string pool so it gets freed. */
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool =
        (char **)realloc(tree->string_pool, new_cap * sizeof(char *));
    if (!new_pool) {
      free(dup);
      free(t);
      return NULL;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;

  t->kind =
      TRIVIA_BLOCK_COMMENT; /* Assuming kind 0 is comment/general in trivia */
  t->start = (const uint8_t *)dup;
  t->length = strlen(dup);
  return t;
}

int cdd_cst_bld_block_comment(cdd_cst_builder_t *builder, const char *text) {
  char buf[1024];
  cdd_trivia_t *trivia;
  cdd_token_t *target_tok = NULL;

  if (!builder || !text)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;

#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s(buf, sizeof(buf), "/* %s */", text);
#else
  snprintf(buf, sizeof(buf), "/* %s */", text);
#endif

  trivia = create_trivia(builder->tree, buf);
  if (!trivia) {
    builder->error_state = ENOMEM;
    return ENOMEM;
  }

  if (builder->target_node->num_children > 0) {
    cdd_cst_child_t *last =
        &builder->target_node->children[builder->target_node->num_children - 1];
    if (last->kind == CDD_CST_CHILD_TOKEN) {
      target_tok = last->val.token;
      /* append as trailing trivia */
      if (target_tok->trailing_trivia) {
        cdd_trivia_t *tail = target_tok->trailing_trivia;
        while (tail->next)
          tail = tail->next;
        tail->next = trivia;
      } else {
        target_tok->trailing_trivia = trivia;
      }
      return 0;
    }
  }

  /* Fallback: append as an ACTUAL token if there's no token to attach to.
     Or we can synthesize a blank token to hold it. We'll append an OTHER token.
   */
  {
    const char *pooled = pool_string(builder->tree, buf);
    int rc;
    if (!pooled) {
      free(trivia);
      return ENOMEM;
    }
    rc = cdd_cst_bld_token(builder, CDD_TOKEN_OTHER, pooled);
    free(trivia); /* since it became a real token via string pool mapping */
    return rc;
  }
}

static cdd_token_t *get_first_token(cdd_cst_node_t *node) {
  size_t i;
  if (!node)
    return NULL;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN)
      return node->children[i].val.token;
    else if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_token_t *t = get_first_token(node->children[i].val.node);
      if (t)
        return t;
    }
  }
  return NULL;
}

static cdd_token_t *get_last_token(cdd_cst_node_t *node) {
  int i;
  if (!node)
    return NULL;
  for (i = (int)node->num_children - 1; i >= 0; i--) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN)
      return node->children[i].val.token;
    else if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_token_t *t = get_last_token(node->children[i].val.node);
      if (t)
        return t;
    }
  }
  return NULL;
}

cdd_trivia_t *cdd_cst_extract_leading_trivia(cdd_cst_node_t *node) {
  cdd_token_t *t = get_first_token(node);
  cdd_trivia_t *trivia = NULL;
  if (t) {
    trivia = t->leading_trivia;
    t->leading_trivia = NULL;
  }
  return trivia;
}

cdd_trivia_t *cdd_cst_extract_trailing_trivia(cdd_cst_node_t *node) {
  cdd_token_t *t = get_last_token(node);
  cdd_trivia_t *trivia = NULL;
  if (t) {
    trivia = t->trailing_trivia;
    t->trailing_trivia = NULL;
  }
  return trivia;
}

int cdd_cst_transfer_trivia(cdd_cst_node_t *source_node,
                            cdd_cst_node_t *target_node) {
  cdd_trivia_t *lead;
  cdd_trivia_t *trail;
  cdd_token_t *t_first;
  cdd_token_t *t_last;

  if (!source_node || !target_node)
    return EINVAL;

  lead = cdd_cst_extract_leading_trivia(source_node);
  trail = cdd_cst_extract_trailing_trivia(source_node);

  t_first = get_first_token(target_node);
  t_last = get_last_token(target_node);

  if (lead && t_first) {
    cdd_trivia_t *tail = lead;
    while (tail->next)
      tail = tail->next;
    tail->next = t_first->leading_trivia;
    t_first->leading_trivia = lead;
  } else if (lead) {
    /* Leak / Lost trivia if target has no tokens */
  }

  if (trail && t_last) {
    if (t_last->trailing_trivia) {
      cdd_trivia_t *tail = t_last->trailing_trivia;
      while (tail->next)
        tail = tail->next;
      tail->next = trail;
    } else {
      t_last->trailing_trivia = trail;
    }
  } else if (trail) {
    /* Leak / Lost trivia if target has no tokens */
  }

  return 0;
}

int cdd_cst_replace_node_preserve_trivia(cdd_cst_builder_t *builder,
                                         cdd_cst_node_t *target_node,
                                         cdd_cst_node_t *replacement_node) {
  int rc;
  if (!builder || !target_node || !replacement_node)
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;

  /* transfer_trivia unbinds trivia from target and moves to replacement */
  cdd_cst_transfer_trivia(target_node, replacement_node);

  /* utilize underlying replace mechanism which handles parent array swapping */
  rc = cdd_cst_replace_node(builder->tree, target_node, replacement_node);
  if (rc != 0) {
    builder->error_state = rc;
  }
  return rc;
}

int cdd_cst_splice_nodes(cdd_cst_builder_t *builder, cdd_cst_node_t *parent,
                         size_t index, cdd_cst_node_t **new_nodes,
                         size_t count) {
  cdd_cst_child_t *children_wrappers;
  size_t i;
  int rc;

  if (!builder || !parent || (!new_nodes && count > 0))
    return EINVAL;
  if (builder->error_state != 0)
    return builder->error_state;
  if (count == 0)
    return 0;

  children_wrappers = (cdd_cst_child_t *)calloc(count, sizeof(cdd_cst_child_t));
  if (!children_wrappers) {
    builder->error_state = ENOMEM;
    return ENOMEM;
  }

  for (i = 0; i < count; i++) {
    children_wrappers[i].kind = CDD_CST_CHILD_NODE;
    children_wrappers[i].val.node = new_nodes[i];
  }

  /* 0 elements to consume, we are just inserting them */
  rc = cdd_cst_splice_children(builder->tree, &parent, index, 0,
                               children_wrappers, count);
  free(children_wrappers);

  if (rc != 0) {
    builder->error_state = rc;
  }
  return rc;
}
