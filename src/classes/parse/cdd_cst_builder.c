/**
 * @file cdd_cst_builder.c
 * @brief Concrete Syntax Tree (CST) builder implementation and utilities.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_builder.h"
#include "cdd_cst_builder_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_cst_factory.h"
#include "cdd_cst_mutate.h"
#include "cdd_lexer.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Initializes a CST builder bound to a tree and initial target node.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_cst_builder_init(cdd_cst_builder_t *builder,
                                   cdd_cst_tree_t *tree,
                                   cdd_cst_node_t *target_node) {
  if (!builder || !tree || !target_node) {

    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  builder->tree = tree;
  builder->target_node = target_node;
  builder->error_state = 0;
  builder->indent_level = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Clears and releases resources associated with a CST builder.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_cst_builder_free(cdd_cst_builder_t *builder) {
  if (!builder) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  builder->tree = NULL;
  builder->target_node = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if the CST builder has encountered an error.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_cst_builder_has_error(const cdd_cst_builder_t *builder,
                                        int *out_has_error) {
  if (out_has_error)
    *out_has_error = 1;
  if (!builder) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (out_has_error)
    *out_has_error = (builder->error_state != 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets the current insertion point node for the CST builder.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_builder_set_insert_point(cdd_cst_builder_t *builder,
                                               cdd_cst_node_t *node) {
  if (!builder || !node) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (builder->error_state != 0) {
    return builder->error_state;
  }
  builder->target_node = node;
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends a synthetic lexical token to the builder's current target
 * node.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_token(cdd_cst_builder_t *builder,
                                enum cdd_token_kind_t kind, const char *text) {
  cdd_token_t *tok;
  cdd_c_error_t rc;
  if (!builder)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;

  if (cdd_cst_create_token(builder->tree, kind, text, &tok) != 0)
    tok = NULL;
  if (!tok) {
    builder->error_state = CDD_C_ERROR_MEMORY;
    return CDD_C_ERROR_MEMORY;
  }

  rc = cdd_cst_append_child_token(builder->target_node, tok);
  if (rc != CDD_C_SUCCESS) {
    builder->error_state = rc;
    return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Appends a single space token to the builder's target node.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_space(cdd_cst_builder_t *builder) {
  {
    cdd_c_error_t rc = cdd_cst_bld_token(builder, CDD_TOKEN_OTHER, " ");
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Appends a newline token to the builder's target node.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_newline(cdd_cst_builder_t *builder) {
  {
    cdd_c_error_t rc = cdd_cst_bld_token(builder, CDD_TOKEN_OTHER, "\n");
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Appends indentation spaces for `depth_level` (2 spaces per level).
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_indent(cdd_cst_builder_t *builder, int depth_level) {
  int i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!builder)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;

  for (i = 0; i < depth_level * 2; i++) {
    rc = cdd_cst_bld_space(builder);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends an identifier token to the builder's target node.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_ident(cdd_cst_builder_t *builder, const char *text) {
  {
    cdd_c_error_t rc = cdd_cst_bld_token(builder, CDD_TOKEN_IDENTIFIER, text);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Appends a string literal token to the builder's target node.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_string(cdd_cst_builder_t *builder, const char *text) {
  {
    cdd_c_error_t rc = cdd_cst_bld_token(builder, CDD_TOKEN_STRING, text);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Formats and appends an integer literal token to the builder's target
 * node.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_int(cdd_cst_builder_t *builder, int value) {
  char buf[32];
  const char *pooled;
  if (!builder)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;

#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s(buf, sizeof(buf), "%d", value);
#else
  CDD_SNPRINTF(buf, sizeof(buf), "%d", value);
#endif
  {
    cdd_c_error_t pool_rc = cdd_cst_pool_string(builder->tree, buf, &pooled);
    if (pool_rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return pool_rc;
    }
  }
  {
    cdd_c_error_t rc = cdd_cst_bld_token(builder, CDD_TOKEN_NUMBER, pooled);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Classifies and appends a punctuation token to the builder's target
 * node.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_punct(cdd_cst_builder_t *builder, const char *text) {
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
  {
    cdd_c_error_t rc = cdd_cst_bld_token(builder, kind, text);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Appends an `#include` preprocessor directive to the builder's target
 * node.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_include(cdd_cst_builder_t *builder, const char *path,
                                  int is_system) {
  cdd_c_error_t rc;
  size_t path_len;
  char *buf;
  const char *pooled = NULL;
  cdd_c_error_t pool_rc;
  if (!builder || !path)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;

  rc = cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_INCLUDE, "#include");
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_space(builder);
  if (rc != CDD_C_SUCCESS)
    return rc;

  path_len = strlen(path);
  buf = (char *)C_CDD_MALLOC(path_len + 3);
  if (!buf) {
    builder->error_state = CDD_C_ERROR_MEMORY;
    return CDD_C_ERROR_MEMORY;
  }
  buf[0] = is_system ? '<' : '"';
  if (path_len > 0) {
    memcpy(buf + 1, path, path_len);
  }
  buf[path_len + 1] = is_system ? '>' : '"';
  buf[path_len + 2] = '\0';

  pool_rc = cdd_cst_pool_string_len(builder->tree, buf, path_len + 2, &pooled);
  C_CDD_FREE(buf);
  if (pool_rc != CDD_C_SUCCESS) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return pool_rc;
  }
  if (is_system) {
    rc = cdd_cst_bld_token(builder, CDD_TOKEN_STRING, pooled);
  } else {
    rc = cdd_cst_bld_string(builder, pooled);
  }
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_newline(builder);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends an `#ifndef <macro_name>` preprocessor directive.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_ifndef(cdd_cst_builder_t *builder,
                                 const char *macro_name) {
  cdd_c_error_t rc;
  if (!builder)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_IFNDEF, "#ifndef");
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_space(builder);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_ident(builder, macro_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_newline(builder);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends an `#ifdef <macro_name>` preprocessor directive.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_ifdef(cdd_cst_builder_t *builder,
                                const char *macro_name) {
  if (!builder)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;
  cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_IFDEF, "#ifdef");
  cdd_cst_bld_space(builder);
  cdd_cst_bld_ident(builder, macro_name);
  return cdd_cst_bld_newline(builder);
}

/**
 * @brief Appends an `#else` preprocessor directive.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_else(cdd_cst_builder_t *builder) {
  cdd_c_error_t rc;
  if (!builder)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_ELSE, "#else");
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_newline(builder);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends an `#endif` preprocessor directive.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_endif(cdd_cst_builder_t *builder) {
  cdd_c_error_t rc;
  if (!builder)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_token(builder, CDD_TOKEN_PREPROC_ENDIF, "#endif");
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_newline(builder);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends an `#ifdef __cplusplus extern "C" { #endif` opening guard.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_extern_c_open(cdd_cst_builder_t *builder) {
  return cdd_cst_bld_snippet(builder,
                             "#ifdef __cplusplus\nextern \"C\" {\n#endif\n");
}

/**
 * @brief Appends an `#ifdef __cplusplus } #endif` closing guard.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_extern_c_close(cdd_cst_builder_t *builder) {
  return cdd_cst_bld_snippet(builder, "#ifdef __cplusplus\n}\n#endif\n");
}

/**
 * @brief Appends a `{` token, a newline, and increments
 * `builder->indent_level`.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_block_open(cdd_cst_builder_t *builder) {
  cdd_c_error_t rc;
  if (!builder)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;
  rc = cdd_cst_bld_punct(builder, "{");
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_newline(builder);
  if (rc != CDD_C_SUCCESS)
    return rc;
  builder->indent_level++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Decrements `builder->indent_level` and appends an indented `}` and
 * newline.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_block_close(cdd_cst_builder_t *builder) {
  cdd_c_error_t rc;
  if (!builder)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;
  if (builder->indent_level > 0)
    builder->indent_level--;
  rc = cdd_cst_bld_indent(builder, builder->indent_level);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_punct(builder, "}");
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_bld_newline(builder);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Duplicates a string of explicit length and registers it in the CST
 * tree's string pool.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_pool_string_len(cdd_cst_tree_t *tree, const char *str,
                                      size_t len, const char **out_str) {
  char *dup;
  *out_str = NULL;
  if (!tree || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  dup = (char *)C_CDD_MALLOC(len + 1);
  if (!dup)
    return CDD_C_ERROR_MEMORY;
  if (len > 0) {
    memcpy(dup, str, len);
  }
  dup[len] = '\0';

  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool =
        (char **)C_CDD_REALLOC(tree->string_pool, new_cap * sizeof(char *));
    if (!new_pool) {
      C_CDD_FREE(dup);
      return CDD_C_ERROR_MEMORY;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;
  *out_str = dup;
  return CDD_C_SUCCESS;
}

/**
 * @brief Duplicates a string and registers it in the CST tree's string pool.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_pool_string(cdd_cst_tree_t *tree, const char *str,
                                  const char **out_str) {
  if (!str) {
    *out_str = NULL;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  return cdd_cst_pool_string_len(tree, str, strlen(str), out_str);
}

/**
 * @brief Lexes a C snippet and appends its tokens and trivia to the builder's
 * current target node.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_bld_snippet(cdd_cst_builder_t *builder,
                                  const char *snippet) {
  cdd_token_list_t *list = NULL;
  az_span span;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!builder || !snippet)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;

  span = az_span_create_from_str(snippet);
  rc = cdd_lexer_tokenize(span, &list);
  if (rc != CDD_C_SUCCESS) {
    builder->error_state = rc;
    return rc;
  }

  for (i = 0; i < list->size; i++) {
    cdd_token_t *t = &list->tokens[i];
    if (t->kind != CDD_TOKEN_EOF) {
      const char *pooled = NULL;
      {
        cdd_c_error_t pool_rc = cdd_cst_pool_string_len(
            builder->tree, (const char *)t->start, t->length, &pooled);
        if (pool_rc != CDD_C_SUCCESS) {
          rc = CDD_C_ERROR_MEMORY;
          break;
        }
      }
      rc = cdd_cst_bld_token(builder, t->kind, pooled);
      if (rc != CDD_C_SUCCESS)
        break;
      {
        cdd_token_t *last = NULL;
        rc = cdd_cst_get_last_token(builder->target_node, &last);
        {
          cdd_trivia_t *tr;
          last->leading_trivia = t->leading_trivia;
          last->trailing_trivia = t->trailing_trivia;
          t->leading_trivia = NULL;
          t->trailing_trivia = NULL;
          for (tr = last->leading_trivia; tr; tr = tr->next) {
            const char *tr_pooled = NULL;
            if (cdd_cst_pool_string_len(builder->tree, (const char *)tr->start,
                                        tr->length,
                                        &tr_pooled) == CDD_C_SUCCESS) {
              tr->start = (const uint8_t *)tr_pooled;
            }
          }
          for (tr = last->trailing_trivia; tr; tr = tr->next) {
            const char *tr_pooled = NULL;
            if (cdd_cst_pool_string_len(builder->tree, (const char *)tr->start,
                                        tr->length,
                                        &tr_pooled) == CDD_C_SUCCESS) {
              tr->start = (const uint8_t *)tr_pooled;
            }
          }
        }
      }
    }
  }

  cdd_lexer_free_token_list(list);
  if (rc != CDD_C_SUCCESS)
    builder->error_state = rc;
  return rc;
}

/**
 * @brief Quasi-quotation formatter that parses a format string and appends CST
 * fragments (`%s`, `%d`, `%n`, `%%`) to `builder`.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_quote(cdd_cst_builder_t *builder,
                            const char *format_string, ...) {
  va_list args;
  const char *p;
  char *snippet_buf = NULL;
  size_t snippet_len = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!builder || !format_string)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;

  snippet_buf = (char *)C_CDD_MALLOC(strlen(format_string) + 1);
  if (!snippet_buf) {
    builder->error_state = CDD_C_ERROR_MEMORY;
    return CDD_C_ERROR_MEMORY;
  }

  va_start(args, format_string);
  for (p = format_string; *p; p++) {
    if (*p == '%' && *(p + 1) != '\0') {
      p++;
      if (*p == '%') {
        snippet_buf[snippet_len++] = '%';
      } else {
        if (snippet_len > 0) {
          snippet_buf[snippet_len] = '\0';
          rc = cdd_cst_bld_snippet(builder, snippet_buf);
          snippet_len = 0;
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(snippet_buf);
            va_end(args);
            builder->error_state = rc;
            return rc;
          }
        }

        if (*p == 's') {
          const char *s = va_arg(args, const char *);
          rc = cdd_cst_bld_snippet(builder, s);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(snippet_buf);
            va_end(args);
            builder->error_state = rc;
            return rc;
          }
        } else if (*p == 'd') {
          int d = va_arg(args, int);
          rc = cdd_cst_bld_int(builder, d);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(snippet_buf);
            va_end(args);
            builder->error_state = rc;
            return rc;
          }
        } else if (*p == 'n') {
          cdd_cst_node_t *node = va_arg(args, cdd_cst_node_t *);
          if (node) {
            rc = cdd_cst_append_child_node(builder->target_node, node);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(snippet_buf);
              va_end(args);
              builder->error_state = rc;
              return rc;
            }
          }
        }
      }
    } else {
      snippet_buf[snippet_len++] = *p;
    }
  }

  if (snippet_len > 0) {
    snippet_buf[snippet_len] = '\0';
    rc = cdd_cst_bld_snippet(builder, snippet_buf);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(snippet_buf);
      va_end(args);
      builder->error_state = rc;
      return rc;
    }
  }

  C_CDD_FREE(snippet_buf);
  va_end(args);
  return CDD_C_SUCCESS;
}

/**
 * @brief Splices an array of CST nodes into `parent` starting at `index`.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_splice_nodes(cdd_cst_builder_t *builder,
                                   cdd_cst_node_t *parent, size_t index,
                                   cdd_cst_node_t **new_nodes, size_t count) {
  cdd_cst_child_t *children_wrappers;
  size_t i;
  cdd_c_error_t rc;

  if (!builder || !parent || (!new_nodes && count > 0))
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return builder->error_state;
  if (count == 0)
    return CDD_C_SUCCESS;

  children_wrappers =
      (cdd_cst_child_t *)C_CDD_CALLOC(count, sizeof(cdd_cst_child_t));
  if (!children_wrappers) {
    builder->error_state = CDD_C_ERROR_MEMORY;
    return CDD_C_ERROR_MEMORY;
  }

  for (i = 0; i < count; i++) {
    children_wrappers[i].kind = CDD_CST_CHILD_NODE;
    children_wrappers[i].val.node = new_nodes[i];
  }

  /* 0 elements to consume, we are just inserting them */
  rc = cdd_cst_splice_children(builder->tree, &parent, index, 0,
                               children_wrappers, count);
  C_CDD_FREE(children_wrappers);

  if (rc != CDD_C_SUCCESS) {
    builder->error_state = rc;
  }
  return rc;
}
