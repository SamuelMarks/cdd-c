/**
 * @file preprocessor.c
 * @brief Implementation of preprocessor logic including expression evaluation
 * and conditional scanning.
 *
 * Includes support for C23 processor introspection operators:
 * `__has_include`, `__has_embed`, and `__has_c_attribute`.
 * Parses `#embed` parameters (limit, prefix, suffix, if_empty).
 *
 * @author Samuel Marks
 */

#include <ctype.h>

#include <errno.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "c_cdd_stdbool.h"

#include "functions/parse/fs.h"

#include "functions/parse/preprocessor.h"

#include "functions/parse/str.h"

#include "functions/parse/tokenizer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

#define strdup _strdup

#endif

#define PATH_SEP_CHAR '\\'

#else

/** @brief PATH_SEP_CHAR definition */
#define PATH_SEP_CHAR '/'

#include <errno.h>

#endif

/* Standard IO / FS helpers */

static int join_path(const char *dir, const char *file, char **_out_val) {

  char *out;

  size_t len;

  if (!dir || !file)

  {
    *_out_val = NULL;
    return 0;
  }

  len = strlen(dir) + strlen(file) + 2;

  out = (char *)malloc(len);

  if (!out)

  {
    *_out_val = NULL;
    return 0;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

  sprintf_s(out, len, "%s%c%s", dir, PATH_SEP_CHAR, file);

#else

  sprintf(out, "%s%c%s", dir, PATH_SEP_CHAR, file);

#endif

  {
    *_out_val = out;
    return 0;
  }
}

static int file_exists(const char *path) {

  FILE *f;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

  if (fopen_s(&f, path, "r") == 0 && f) {

    fclose(f);

    return 1;
  }

#else
#if defined(_MSC_VER)
  fopen_s(&f, path, "r");
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, "r");
#else
  f = fopen(path, "r");
#endif
#endif

  if (f) {

    fclose(f);

    return 1;
  }

#endif

  return 0;
}

static void free_macro_def(struct MacroDef *def) {

  size_t i;

  if (def->name)

    free(def->name);

  if (def->value)

    free(def->value);

  if (def->args) {

    for (i = 0; i < def->arg_count; i++) {

      free(def->args[i]);
    }

    free(def->args);
  }
}

static int token_to_string(const struct Token *t, char **_out_val) {

  char *s = malloc(t->length + 1);

  if (!s)

  {
    *_out_val = NULL;
    return 0;
  }

  memcpy(s, t->start, t->length);

  s[t->length] = '\0';

  {
    *_out_val = s;
    return 0;
  }
}

static int add_macro_internal(struct PreprocessorContext *ctx,

                              const struct MacroDef *def) {

  if (ctx->macro_count >= ctx->macro_capacity) {

    size_t new_cap = (ctx->macro_capacity == 0) ? 16 : ctx->macro_capacity * 2;

    struct MacroDef *new_arr = (struct MacroDef *)realloc(

        ctx->macros, new_cap * sizeof(struct MacroDef));

    if (!new_arr)

      return ENOMEM;

    ctx->macros = new_arr;

    ctx->macro_capacity = new_cap;
  }

  ctx->macros[ctx->macro_count++] = *def;

  return 0;
}

/**
 * @brief Helper to resolve include paths.
 */

static int resolve_path(const struct PreprocessorContext *ctx,

                        const char *current_dir, const char *include_path,

                        int is_system, char **_out_val) {
  char *_ast_join_path_0;
  char *_ast_join_path_1;

  size_t i;

  char *candidate;

  if (!is_system && current_dir) {

    candidate = (join_path(current_dir, include_path, &_ast_join_path_0),
                 _ast_join_path_0);

    if (candidate) {

      if (file_exists(candidate)) {

        {
          *_out_val = candidate;
          return 0;
        }
      }

      free(candidate);
    }
  }

  if (ctx) {

    for (i = 0; i < ctx->size; ++i) {

      candidate =
          (join_path(ctx->search_paths[i], include_path, &_ast_join_path_1),
           _ast_join_path_1);

      if (candidate) {

        if (file_exists(candidate)) {

          {
            *_out_val = candidate;
            return 0;
          }
        }

        free(candidate);
      }
    }
  }

  {
    *_out_val = NULL;
    return 0;
  }
}

static int reconstruct_path(const struct TokenList *tokens, size_t start,

                            size_t end, char **_out_val) {
  char *_ast_strdup_0 = NULL;

  size_t len = 0;

  size_t i;

  char *buf, *p;

  if (start >= end)

  {
    *_out_val = (c_cdd_strdup("", &_ast_strdup_0), _ast_strdup_0);
    return 0;
  }

  for (i = start; i < end; ++i) {

    len += tokens->tokens[i].length;
  }

  buf = (char *)malloc(len + 1);

  if (!buf)

  {
    *_out_val = NULL;
    return 0;
  }

  p = buf;

  for (i = start; i < end; ++i) {

    const struct Token *t = &tokens->tokens[i];

    memcpy(p, t->start, t->length);

    p += t->length;
  }

  *p = '\0';

  {
    *_out_val = buf;
    return 0;
  }
}

/* --- Public API Implementation --- */

int pp_context_init(struct PreprocessorContext *ctx) {

  if (!ctx)

    return EINVAL;

  memset(ctx, 0, sizeof(*ctx));

  return 0;
}

void pp_context_free(struct PreprocessorContext *ctx) {

  size_t i;

  if (!ctx)

    return;

  for (i = 0; i < ctx->size; ++i)

    free(ctx->search_paths[i]);

  if (ctx->search_paths)

    free(ctx->search_paths);

  for (i = 0; i < ctx->macro_count; ++i)

    free_macro_def(&ctx->macros[i]);

  if (ctx->macros)

    free(ctx->macros);

  memset(ctx, 0, sizeof(*ctx));
}

int pp_add_search_path(struct PreprocessorContext *ctx, const char *path) {
  char *_ast_strdup_1 = NULL;

  char *copy;

  char **new_paths;

  if (!ctx || !path)

    return EINVAL;

  copy = (c_cdd_strdup(path, &_ast_strdup_1), _ast_strdup_1);

  if (!copy)

    return ENOMEM;

  if (ctx->size >= ctx->capacity) {

    size_t new_cap = (ctx->capacity == 0) ? 8 : ctx->capacity * 2;

    new_paths = (char **)realloc(ctx->search_paths, new_cap * sizeof(char *));

    if (!new_paths) {

      free(copy);

      return ENOMEM;
    }

    ctx->search_paths = new_paths;

    ctx->capacity = new_cap;
  }

  ctx->search_paths[ctx->size++] = copy;

  return 0;
}

int pp_add_macro(struct PreprocessorContext *ctx, const char *name,

                 const char *value) {
  char *_ast_strdup_2 = NULL;
  char *_ast_strdup_3 = NULL;

  struct MacroDef def;

  if (!ctx || !name)

    return EINVAL;

  memset(&def, 0, sizeof(def));

  def.name = (c_cdd_strdup(name, &_ast_strdup_2), _ast_strdup_2);

  if (!def.name)

    return ENOMEM;

  if (value) {

    def.value = (c_cdd_strdup(value, &_ast_strdup_3), _ast_strdup_3);

    if (!def.value) {

      free(def.name);

      return ENOMEM;
    }
  }

  def.is_function_like = 0;

  if (add_macro_internal(ctx, &def) != 0) {

    free_macro_def(&def);

    return ENOMEM;
  }

  return 0;
}

int pp_scan_defines(struct PreprocessorContext *ctx, const char *filename) {
  bool _ast_token_matches_string_2;
  char *_ast_token_to_string_3;
  char *_ast_token_to_string_4;

  char *content = NULL;

  size_t sz = 0;

  struct TokenList *tokens = NULL;

  int rc = 0;

  size_t i;

  if (!ctx || !filename)

    return EINVAL;

  rc = read_to_file(filename, "r", &content, &sz);

  if (rc != 0)

    return rc;

  rc = tokenize(az_span_create_from_str(content), &tokens);

  if (rc != 0) {

    free(content);

    return rc;
  }

  for (i = 0; i < tokens->size; ++i) {

    if (tokens->tokens[i].kind == TOKEN_HASH) {

      size_t next = i + 1;

      while (next < tokens->size &&

             tokens->tokens[next].kind == TOKEN_WHITESPACE)

        next++;

      if (next < tokens->size &&

          (token_matches_string(&tokens->tokens[next], "define",
                                &_ast_token_matches_string_2),
           _ast_token_matches_string_2)) {

        size_t name_idx = next + 1;

        while (name_idx < tokens->size &&

               tokens->tokens[name_idx].kind == TOKEN_WHITESPACE)

          name_idx++;

        if (name_idx < tokens->size &&

            tokens->tokens[name_idx].kind == TOKEN_IDENTIFIER) {

          struct MacroDef def;

          memset(&def, 0, sizeof(def));

          def.name = (token_to_string(&tokens->tokens[name_idx],
                                      &_ast_token_to_string_3),
                      _ast_token_to_string_3);

          if (name_idx + 1 < tokens->size &&

              tokens->tokens[name_idx + 1].kind == TOKEN_LPAREN) {

            /* Function-like macro */

            def.is_function_like = 1;

            {

              size_t curr = name_idx + 2;

              int done_args = 0;

              while (curr < tokens->size && !done_args) {

                while (curr < tokens->size &&

                       tokens->tokens[curr].kind == TOKEN_WHITESPACE)

                  curr++;

                if (curr >= tokens->size)

                  break;

                if (tokens->tokens[curr].kind == TOKEN_RPAREN) {

                  done_args = 1;

                  curr++;

                } else if (tokens->tokens[curr].kind == TOKEN_IDENTIFIER) {

                  char *argName = (token_to_string(&tokens->tokens[curr],
                                                   &_ast_token_to_string_4),
                                   _ast_token_to_string_4);

                  char **new_args = (char **)realloc(

                      def.args, (def.arg_count + 1) * sizeof(char *));

                  if (new_args) {

                    def.args = new_args;

                    def.args[def.arg_count++] = argName;

                  } else {

                    free(argName);
                  }

                  curr++;

                  /* Check for GCC variadic "args..." */

                  if (curr < tokens->size &&

                      tokens->tokens[curr].kind == TOKEN_ELLIPSIS) {

                    def.is_variadic = 1;

                    curr++;
                  }

                } else if (tokens->tokens[curr].kind == TOKEN_ELLIPSIS) {

                  def.is_variadic = 1;

                  curr++;

                } else if (tokens->tokens[curr].kind == TOKEN_COMMA) {

                  curr++;

                } else {

                  curr++; /* Error recovery */
                }
              }
            }

          } else {

            /* Object-like */

            def.is_function_like = 0;
          }

          add_macro_internal(ctx, &def);
        }

        i = name_idx; /* Advance */
      }
    }
  }

  free_token_list(tokens);

  free(content);

  return rc;
}

void pp_embed_params_free(struct EmbedParams *params) {

  if (!params)

    return;

  if (params->prefix)

    free(params->prefix);

  if (params->suffix)

    free(params->suffix);

  if (params->if_empty)

    free(params->if_empty);

  params->prefix = NULL;

  params->suffix = NULL;

  params->if_empty = NULL;
}

/* --- Expression Evaluator Implementation --- */

/* Parser State */

/** @brief ExprState structure */
struct ExprState {

  /** @brief tokens */
  /** @brief tokens */
  const struct TokenList *tokens;

  /** @brief pos */

  /** @brief pos */
  size_t pos;

  /** @brief ctx */
  /** @brief end */
  size_t end;

  /** @brief error */

  /** @brief ctx */
  const struct PreprocessorContext *ctx;

  /** @brief error */
  int error;
};

/* Forward Declarations for Recursive Descent */

static int parse_expr(struct ExprState *s, long *_out_val);

static int parse_logic_or(struct ExprState *s, long *_out_val);

static int parse_logic_and(struct ExprState *s, long *_out_val);

static int parse_equality(struct ExprState *s, long *_out_val);

static int parse_relational(struct ExprState *s, long *_out_val);

static int parse_shift(struct ExprState *s, long *_out_val);

static int parse_additive(struct ExprState *s, long *_out_val);

static int parse_multiplicative(struct ExprState *s, long *_out_val);

static int parse_unary(struct ExprState *s, long *_out_val);

static int parse_primary(struct ExprState *s, long *_out_val);

/* Helper: skip whitespace */

static int skip_ws(struct ExprState *s, size_t *_out_val) {
  (void)_out_val;
  while (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_WHITESPACE)
    s->pos++;
  return 0;
}

/* Helper: Check current token kind */

static int match(struct ExprState *s, enum TokenKind kind, bool *_out_val) {
  size_t _ast_skip_ws_5;

  (skip_ws(s, &_ast_skip_ws_5), _ast_skip_ws_5);

  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == kind) {

    s->pos++;

    {
      *_out_val = true;
      return 0;
    }
  }

  {
    *_out_val = false;
    return 0;
  }
}

/* Helper: Peek current token kind */

static int peek(struct ExprState *s, enum TokenKind *_out_val) {

  size_t p = s->pos;

  while (p < s->end && s->tokens->tokens[p].kind == TOKEN_WHITESPACE)

    p++;

  if (p >= s->end)

  {
    *_out_val = TOKEN_UNKNOWN;
    return 0;
  }

  {
    *_out_val = s->tokens->tokens[p].kind;
    return 0;
  }
}

static int is_defined_macro(const struct PreprocessorContext *ctx,

                            const struct Token *tok, bool *_out_val) {
  bool _ast_token_matches_string_6;

  size_t i;

  if (!ctx)

  {
    *_out_val = false;
    return 0;
  }

  for (i = 0; i < ctx->macro_count; ++i) {

    if ((token_matches_string(tok, ctx->macros[i].name,
                              &_ast_token_matches_string_6),
         _ast_token_matches_string_6))

    {
      *_out_val = true;
      return 0;
    }
  }

  {
    *_out_val = false;
    return 0;
  }
}

/**
 * @brief Handle __has_include and __has_embed logic.
 *
 * Syntax: `__has_include ( header-name )` or `__has_include ( "header" )` etc.
 * Spec: The operands are processed as in phase 3. The result is 1 if found, 0
 * otherwise.
 */

static int handle_has_include_embed(struct ExprState *s, long *_out_val) {
  size_t _ast_skip_ws_7;
  bool _ast_match_8;
  size_t _ast_skip_ws_9;
  char *_ast_reconstruct_path_10;
  size_t _ast_skip_ws_11;
  bool _ast_match_12;
  char *_ast_resolve_path_13;

  int is_header = false;

  char *path = NULL;

  char *resolved = NULL;

  long result = 0;

  /* Expect '(' */

  (skip_ws(s, &_ast_skip_ws_7), _ast_skip_ws_7);

  if (!(match(s, TOKEN_LPAREN, &_ast_match_8), _ast_match_8)) {

    s->error = 1;

    {
      *_out_val = 0;
      return 0;
    }
  }

  (skip_ws(s, &_ast_skip_ws_9), _ast_skip_ws_9);

  if (s->pos >= s->end) {

    s->error = 1;

    {
      *_out_val = 0;
      return 0;
    }
  }

  /* Parse Header Name */

  if (s->tokens->tokens[s->pos].kind == TOKEN_STRING_LITERAL) {

    const struct Token *t = &s->tokens->tokens[s->pos];

    if (t->length >= 2) {

      path = (char *)malloc(t->length - 1);

      if (path) {

        memcpy(path, t->start + 1, t->length - 2);

        path[t->length - 2] = '\0';
      }
    }

    s->pos++;

  } else if (s->tokens->tokens[s->pos].kind == TOKEN_LESS) {

    size_t start_p = s->pos + 1;

    size_t end_p = start_p;

    while (end_p < s->end && s->tokens->tokens[end_p].kind != TOKEN_GREATER) {

      end_p++;
    }

    if (end_p < s->end) {

      path = (reconstruct_path(s->tokens, start_p, end_p,
                               &_ast_reconstruct_path_10),
              _ast_reconstruct_path_10);

      s->pos = end_p + 1; /* Skip closing '>' */

      is_header = 1;

    } else {

      s->error = 1;

      {
        *_out_val = 0;
        return 0;
      }
    }

  } else {

    s->error = 1;

    {
      *_out_val = 0;
      return 0;
    }
  }

  /* Consume remaining parameters until RPAREN */

  while (s->pos < s->end) {

    if (s->tokens->tokens[s->pos].kind == TOKEN_RPAREN) {

      break;
    }

    s->pos++;
  }

  (skip_ws(s, &_ast_skip_ws_11), _ast_skip_ws_11);

  if (!(match(s, TOKEN_RPAREN, &_ast_match_12), _ast_match_12)) {

    s->error = 1;

    if (path)

      free(path);

    {
      *_out_val = 0;
      return 0;
    }
  }

  if (path) {

    resolved = (resolve_path(s->ctx, s->ctx ? s->ctx->current_file_dir : NULL,

                             path, is_header, &_ast_resolve_path_13),
                _ast_resolve_path_13);

    result = (resolved != NULL);

    free(resolved);

    free(path);
  }

  {
    *_out_val = result;
    return 0;
  }
}

/**
 * @brief Handle __has_c_attribute logic.
 */

static int handle_has_c_attribute(struct ExprState *s, long *_out_val) {
  size_t _ast_skip_ws_14;
  bool _ast_match_15;
  size_t _ast_skip_ws_16;
  char *_ast_token_to_string_17;
  enum TokenKind _ast_identify_keyword_or_id_18;
  char *_ast_token_to_string_19;
  size_t _ast_skip_ws_20;
  size_t _ast_skip_ws_21;
  char *_ast_token_to_string_22;
  size_t _ast_skip_ws_23;
  bool _ast_match_24;

  long result = 0;

  char *attr_name = NULL;

  (skip_ws(s, &_ast_skip_ws_14), _ast_skip_ws_14);

  if (!(match(s, TOKEN_LPAREN, &_ast_match_15), _ast_match_15)) {

    s->error = 1;

    {
      *_out_val = 0;
      return 0;
    }
  }

  (skip_ws(s, &_ast_skip_ws_16), _ast_skip_ws_16);

  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {

    attr_name =
        (token_to_string(&s->tokens->tokens[s->pos], &_ast_token_to_string_17),
         _ast_token_to_string_17);

    s->pos++;

  } else if (s->pos < s->end &&

             (identify_keyword_or_id(s->tokens->tokens[s->pos].start,

                                     s->tokens->tokens[s->pos].length,
                                     &_ast_identify_keyword_or_id_18),
              _ast_identify_keyword_or_id_18) !=

                 TOKEN_IDENTIFIER) {

    /* Handle keywords treated as attributes */

    attr_name =
        (token_to_string(&s->tokens->tokens[s->pos], &_ast_token_to_string_19),
         _ast_token_to_string_19);

    s->pos++;
  }

  /* Support scoping `::` */

  (skip_ws(s, &_ast_skip_ws_20), _ast_skip_ws_20);

  if (s->pos + 1 < s->end && s->tokens->tokens[s->pos].kind == TOKEN_COLON &&

      s->tokens->tokens[s->pos + 1].kind == TOKEN_COLON) {

    char *scope = attr_name;

    char *name = NULL;

    s->pos += 2; /* Skip :: */

    (skip_ws(s, &_ast_skip_ws_21), _ast_skip_ws_21);

    if (s->pos < s->end &&

        (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER)) {

      name = (token_to_string(&s->tokens->tokens[s->pos],
                              &_ast_token_to_string_22),
              _ast_token_to_string_22);

      s->pos++;
    }

    if (name)

      free(name);

    if (scope)

      free(scope);

    attr_name = NULL;

    result = 0; /* Scoped ignored currently */
  }

  if (attr_name) {

    if (strcmp(attr_name, "deprecated") == 0)

      result = 201904L;

    else if (strcmp(attr_name, "fallthrough") == 0)

      result = 201904L;

    else if (strcmp(attr_name, "maybe_unused") == 0)

      result = 201904L;

    else if (strcmp(attr_name, "nodiscard") == 0)

      result = 201904L;

    else if (strcmp(attr_name, "noreturn") == 0)

      result = 202202L;

    else if (strcmp(attr_name, "unsequenced") == 0)

      result = 202311L;

    else if (strcmp(attr_name, "reproducible") == 0)

      result = 202311L;

    else

      result = 0;

    free(attr_name);
  }

  (skip_ws(s, &_ast_skip_ws_23), _ast_skip_ws_23);

  if (!(match(s, TOKEN_RPAREN, &_ast_match_24), _ast_match_24)) {

    s->error = 1;
  }

  {
    *_out_val = result;
    return 0;
  }
}

static int parse_primary(struct ExprState *s, long *_out_val) {
  size_t _ast_skip_ws_25;
  bool _ast_match_26;
  long _ast_parse_expr_27;
  bool _ast_match_28;
  char *_ast_token_to_string_29;
  bool _ast_token_matches_string_30;
  long _ast_handle_has_include_embed_31;
  bool _ast_token_matches_string_32;
  long _ast_handle_has_include_embed_33;
  bool _ast_token_matches_string_34;
  long _ast_handle_has_c_attribute_35;
  bool _ast_token_matches_string_36;

  (skip_ws(s, &_ast_skip_ws_25), _ast_skip_ws_25);

  if (s->pos >= s->end) {

    s->error = 1;

    {
      *_out_val = 0;
      return 0;
    }
  }

  if ((match(s, TOKEN_LPAREN, &_ast_match_26), _ast_match_26)) {

    long val = (parse_expr(s, &_ast_parse_expr_27), _ast_parse_expr_27);

    if (!(match(s, TOKEN_RPAREN, &_ast_match_28), _ast_match_28))

      s->error = 1;

    {
      *_out_val = val;
      return 0;
    }
  }

  if (s->tokens->tokens[s->pos].kind == TOKEN_NUMBER_LITERAL) {

    char *txt =
        (token_to_string(&s->tokens->tokens[s->pos], &_ast_token_to_string_29),
         _ast_token_to_string_29);

    long val = 0;

    if (txt) {

      if (strlen(txt) > 2 && txt[0] == '0' &&

          (txt[1] == 'b' || txt[1] == 'B')) {

        char *endptr;

        val = strtol(txt + 2, &endptr, 2);

      } else {

        val = strtol(txt, NULL, 0);
      }

      free(txt);
    }

    s->pos++;

    {
      *_out_val = val;
      return 0;
    }
  }

  if (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER ||

      s->tokens->tokens[s->pos].kind >= TOKEN_KEYWORD_AUTO) {

    const struct Token *tok = &s->tokens->tokens[s->pos];

    if ((token_matches_string(tok, "__has_include",
                              &_ast_token_matches_string_30),
         _ast_token_matches_string_30)) {

      s->pos++;

      {
        *_out_val =
            (handle_has_include_embed(s, &_ast_handle_has_include_embed_31),
             _ast_handle_has_include_embed_31);
        return 0;
      }
    }

    if ((token_matches_string(tok, "__has_embed",
                              &_ast_token_matches_string_32),
         _ast_token_matches_string_32)) {

      s->pos++;

      {
        *_out_val =
            (handle_has_include_embed(s, &_ast_handle_has_include_embed_33),
             _ast_handle_has_include_embed_33);
        return 0;
      }
    }

    if ((token_matches_string(tok, "__has_c_attribute",
                              &_ast_token_matches_string_34),
         _ast_token_matches_string_34)) {

      s->pos++;

      {
        *_out_val = (handle_has_c_attribute(s, &_ast_handle_has_c_attribute_35),
                     _ast_handle_has_c_attribute_35);
        return 0;
      }
    }

    /* Macro value lookup */

    {

      size_t i;

      long val = 0;

      if (s->ctx) {

        for (i = 0; i < s->ctx->macro_count; ++i) {

          if (!s->ctx->macros[i].is_function_like && s->ctx->macros[i].value &&

              (token_matches_string(tok, s->ctx->macros[i].name,
                                    &_ast_token_matches_string_36),
               _ast_token_matches_string_36)) {

            char *endptr;

            val = strtol(s->ctx->macros[i].value, &endptr, 0);

            break;
          }
        }
      }

      s->pos++;

      {
        *_out_val = val;
        return 0;
      }
    }
  }

  s->pos++;

  {
    *_out_val = 0;
    return 0;
  }
}

static int parse_unary(struct ExprState *s, long *_out_val) {
  size_t _ast_skip_ws_37;
  bool _ast_match_38;
  long _ast_parse_unary_39;
  bool _ast_match_40;
  long _ast_parse_unary_41;
  bool _ast_match_42;
  long _ast_parse_unary_43;
  bool _ast_match_44;
  long _ast_parse_unary_45;
  bool _ast_token_matches_string_46;
  size_t _ast_skip_ws_47;
  bool _ast_match_48;
  size_t _ast_skip_ws_49;
  bool _ast_is_defined_macro_50;
  bool _ast_match_51;
  long _ast_parse_primary_52;

  (skip_ws(s, &_ast_skip_ws_37), _ast_skip_ws_37);

  if ((match(s, TOKEN_BANG, &_ast_match_38), _ast_match_38)) {

    {
      *_out_val = !(parse_unary(s, &_ast_parse_unary_39), _ast_parse_unary_39);
      return 0;
    }
  }

  if ((match(s, TOKEN_TILDE, &_ast_match_40), _ast_match_40)) {

    {
      *_out_val = ~(parse_unary(s, &_ast_parse_unary_41), _ast_parse_unary_41);
      return 0;
    }
  }

  if ((match(s, TOKEN_MINUS, &_ast_match_42), _ast_match_42)) {

    {
      *_out_val = -(parse_unary(s, &_ast_parse_unary_43), _ast_parse_unary_43);
      return 0;
    }
  }

  if ((match(s, TOKEN_PLUS, &_ast_match_44), _ast_match_44)) {

    {
      *_out_val = +(parse_unary(s, &_ast_parse_unary_45), _ast_parse_unary_45);
      return 0;
    }
  }

  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER &&

      (token_matches_string(&s->tokens->tokens[s->pos], "defined",
                            &_ast_token_matches_string_46),
       _ast_token_matches_string_46)) {

    bool has_paren = false;

    long result = 0;

    s->pos++;

    (skip_ws(s, &_ast_skip_ws_47), _ast_skip_ws_47);

    if ((match(s, TOKEN_LPAREN, &_ast_match_48), _ast_match_48))

      has_paren = true;

    (skip_ws(s, &_ast_skip_ws_49), _ast_skip_ws_49);

    if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {

      if ((is_defined_macro(s->ctx, &s->tokens->tokens[s->pos],
                            &_ast_is_defined_macro_50),
           _ast_is_defined_macro_50)) {

        result = 1;
      }

      s->pos++;

    } else {

      s->error = 1;
    }

    if (has_paren) {

      if (!(match(s, TOKEN_RPAREN, &_ast_match_51), _ast_match_51))

        s->error = 1;
    }

    {
      *_out_val = result;
      return 0;
    }
  }

  {
    *_out_val =
        (parse_primary(s, &_ast_parse_primary_52), _ast_parse_primary_52);
    return 0;
  }
}

static int parse_multiplicative(struct ExprState *s, long *_out_val) {
  long _ast_parse_unary_53;
  bool _ast_match_54;
  long _ast_parse_unary_55;
  bool _ast_match_56;
  long _ast_parse_unary_57;
  bool _ast_match_58;
  long _ast_parse_unary_59;

  long val = (parse_unary(s, &_ast_parse_unary_53), _ast_parse_unary_53);

  while (s->pos < s->end && !s->error) {

    if ((match(s, TOKEN_STAR, &_ast_match_54), _ast_match_54)) {

      val *= (parse_unary(s, &_ast_parse_unary_55), _ast_parse_unary_55);

    } else if ((match(s, TOKEN_SLASH, &_ast_match_56), _ast_match_56)) {

      long divisor =
          (parse_unary(s, &_ast_parse_unary_57), _ast_parse_unary_57);

      if (divisor == 0)

        val = 0;

      else

        val /= divisor;

    } else if ((match(s, TOKEN_PERCENT, &_ast_match_58), _ast_match_58)) {

      long divisor =
          (parse_unary(s, &_ast_parse_unary_59), _ast_parse_unary_59);

      if (divisor == 0)

        val = 0;

      else

        val %= divisor;

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return 0;
  }
}

static int parse_additive(struct ExprState *s, long *_out_val) {
  long _ast_parse_multiplicative_60;
  bool _ast_match_61;
  long _ast_parse_multiplicative_62;
  bool _ast_match_63;
  long _ast_parse_multiplicative_64;

  long val = (parse_multiplicative(s, &_ast_parse_multiplicative_60),
              _ast_parse_multiplicative_60);

  while (s->pos < s->end && !s->error) {

    if ((match(s, TOKEN_PLUS, &_ast_match_61), _ast_match_61)) {

      val += (parse_multiplicative(s, &_ast_parse_multiplicative_62),
              _ast_parse_multiplicative_62);

    } else if ((match(s, TOKEN_MINUS, &_ast_match_63), _ast_match_63)) {

      val -= (parse_multiplicative(s, &_ast_parse_multiplicative_64),
              _ast_parse_multiplicative_64);

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return 0;
  }
}

static int parse_shift(struct ExprState *s, long *_out_val) {
  long _ast_parse_additive_65;
  bool _ast_match_66;
  long _ast_parse_additive_67;
  bool _ast_match_68;
  long _ast_parse_additive_69;

  long val =
      (parse_additive(s, &_ast_parse_additive_65), _ast_parse_additive_65);

  while (s->pos < s->end && !s->error) {

    if ((match(s, TOKEN_LSHIFT, &_ast_match_66), _ast_match_66)) {

      val <<=
          (parse_additive(s, &_ast_parse_additive_67), _ast_parse_additive_67);

    } else if ((match(s, TOKEN_RSHIFT, &_ast_match_68), _ast_match_68)) {

      val >>=
          (parse_additive(s, &_ast_parse_additive_69), _ast_parse_additive_69);

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return 0;
  }
}

static int parse_relational(struct ExprState *s, long *_out_val) {
  long _ast_parse_shift_70;
  enum TokenKind _ast_peek_71;
  bool _ast_match_72;
  long _ast_parse_shift_73;
  bool _ast_match_74;
  long _ast_parse_shift_75;
  bool _ast_match_76;
  long _ast_parse_shift_77;
  bool _ast_match_78;
  long _ast_parse_shift_79;

  long val = (parse_shift(s, &_ast_parse_shift_70), _ast_parse_shift_70);

  while (s->pos < s->end && !s->error) {

    enum TokenKind k = (peek(s, &_ast_peek_71), _ast_peek_71);

    if (k == TOKEN_LEQ) {

      (match(s, k, &_ast_match_72), _ast_match_72);

      val =
          (val <= (parse_shift(s, &_ast_parse_shift_73), _ast_parse_shift_73));

    } else if (k == TOKEN_GEQ) {

      (match(s, k, &_ast_match_74), _ast_match_74);

      val =
          (val >= (parse_shift(s, &_ast_parse_shift_75), _ast_parse_shift_75));

    } else if (k == TOKEN_LESS) {

      (match(s, k, &_ast_match_76), _ast_match_76);

      val = (val < (parse_shift(s, &_ast_parse_shift_77), _ast_parse_shift_77));

    } else if (k == TOKEN_GREATER) {

      (match(s, k, &_ast_match_78), _ast_match_78);

      val = (val > (parse_shift(s, &_ast_parse_shift_79), _ast_parse_shift_79));

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return 0;
  }
}

static int parse_equality(struct ExprState *s, long *_out_val) {
  long _ast_parse_relational_80;
  bool _ast_match_81;
  long _ast_parse_relational_82;
  bool _ast_match_83;
  long _ast_parse_relational_84;

  long val = (parse_relational(s, &_ast_parse_relational_80),
              _ast_parse_relational_80);

  while (s->pos < s->end && !s->error) {

    if ((match(s, TOKEN_EQ, &_ast_match_81), _ast_match_81)) {

      val = (val == (parse_relational(s, &_ast_parse_relational_82),
                     _ast_parse_relational_82));

    } else if ((match(s, TOKEN_NEQ, &_ast_match_83), _ast_match_83)) {

      val = (val != (parse_relational(s, &_ast_parse_relational_84),
                     _ast_parse_relational_84));

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return 0;
  }
}

static int parse_logic_and(struct ExprState *s, long *_out_val) {
  long _ast_parse_equality_85;
  bool _ast_match_86;
  long _ast_parse_equality_87;

  long val =
      (parse_equality(s, &_ast_parse_equality_85), _ast_parse_equality_85);

  while ((match(s, TOKEN_LOGICAL_AND, &_ast_match_86), _ast_match_86)) {

    long rhs =
        (parse_equality(s, &_ast_parse_equality_87), _ast_parse_equality_87);

    val = (val && rhs);
  }

  {
    *_out_val = val;
    return 0;
  }
}

static int parse_logic_or(struct ExprState *s, long *_out_val) {
  long _ast_parse_logic_and_88;
  bool _ast_match_89;
  long _ast_parse_logic_and_90;

  long val =
      (parse_logic_and(s, &_ast_parse_logic_and_88), _ast_parse_logic_and_88);

  while ((match(s, TOKEN_LOGICAL_OR, &_ast_match_89), _ast_match_89)) {

    long rhs =
        (parse_logic_and(s, &_ast_parse_logic_and_90), _ast_parse_logic_and_90);

    val = (val || rhs);
  }

  {
    *_out_val = val;
    return 0;
  }
}

static int parse_expr(struct ExprState *s, long *_out_val) {
  long _ast_parse_logic_or_91;
  {
    *_out_val =
        (parse_logic_or(s, &_ast_parse_logic_or_91), _ast_parse_logic_or_91);
    return 0;
  }
}

int pp_eval_expression(const struct TokenList *tokens, size_t start_idx,

                       size_t end_idx, const struct PreprocessorContext *ctx,

                       long *result) {
  long _ast_parse_expr_92;

  struct ExprState s;

  if (!tokens || !result)

    return EINVAL;

  s.tokens = tokens;

  s.pos = start_idx;

  s.end = end_idx;

  s.ctx = ctx;

  s.error = 0;

  *result = (parse_expr(&s, &_ast_parse_expr_92), _ast_parse_expr_92);

  if (s.error)

    return EINVAL;

  return 0;
}

/* --- Embed Directive Parsing --- */

static int parse_embed_params(const struct TokenList *tokens, size_t start,

                              size_t end, struct PreprocessorContext *ctx,

                              struct EmbedParams *out_params) {
  enum TokenKind _ast_identify_keyword_or_id_93;
  char *_ast_token_to_string_94;
  enum TokenKind _ast_identify_keyword_or_id_95;
  char *_ast_token_to_string_96;
  char *_ast_reconstruct_path_97;
  char *_ast_reconstruct_path_98;
  char *_ast_reconstruct_path_99;

  size_t i = start;

  while (i < end) {

    char *name = NULL;

    char *scope = NULL;

    /* Skip WS */

    while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)

      i++;

    if (i >= end)

      break; /* End of params */

    /* Expect Identifier */

    if (tokens->tokens[i].kind != TOKEN_IDENTIFIER &&

        (identify_keyword_or_id(tokens->tokens[i].start,

                                tokens->tokens[i].length,
                                &_ast_identify_keyword_or_id_93),
         _ast_identify_keyword_or_id_93) != TOKEN_IDENTIFIER) {

      /* Try identify and accept as ID if keyword-like */
    }

    name = (token_to_string(&tokens->tokens[i], &_ast_token_to_string_94),
            _ast_token_to_string_94);

    if (!name)

      return ENOMEM;

    i++;

    /* Check for scoped attribute :: */

    while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)

      i++;

    if (i + 1 < end && tokens->tokens[i].kind == TOKEN_COLON &&

        tokens->tokens[i + 1].kind == TOKEN_COLON) {

      /* Scoped */

      scope = name;

      name = NULL;

      i += 2; /* Skip :: */

      while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)

        i++;

      if (i < end && (tokens->tokens[i].kind == TOKEN_IDENTIFIER ||

                      (identify_keyword_or_id(tokens->tokens[i].start,

                                              tokens->tokens[i].length,
                                              &_ast_identify_keyword_or_id_95),
                       _ast_identify_keyword_or_id_95) !=

                          TOKEN_IDENTIFIER)) {

        name = (token_to_string(&tokens->tokens[i], &_ast_token_to_string_96),
                _ast_token_to_string_96);

        i++;

      } else {

        free(scope);

        return EINVAL; /* Expected identifier after :: */
      }
    }

    /* Expect '(' */

    while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)

      i++;

    if (i >= end || tokens->tokens[i].kind != TOKEN_LPAREN) {

      /* Embed params must have value clause? Standard params do. */

      free(name);

      if (scope)

        free(scope);

      return EINVAL;
    }

    {

      /* Find matching RPAREN */

      size_t open_idx = i;

      size_t close_idx = 0;

      int depth = 1;

      i++;

      while (i < end) {

        if (tokens->tokens[i].kind == TOKEN_LPAREN)

          depth++;

        else if (tokens->tokens[i].kind == TOKEN_RPAREN) {

          depth--;

          if (depth == 0) {

            close_idx = i;

            break;
          }
        }

        i++;
      }

      if (depth != 0) {

        free(name);

        if (scope)

          free(scope);

        return EINVAL; /* Unbalanced */
      }

      /* Store value */

      if (strcmp(name, "limit") == 0 && !scope) {

        long val = 0;

        /* Eval logic inside [open_idx + 1, close_idx) */

        pp_eval_expression(tokens, open_idx + 1, close_idx, ctx, &val);

        out_params->limit = val;

      } else if (strcmp(name, "prefix") == 0 && !scope) {

        out_params->prefix = (reconstruct_path(tokens, open_idx + 1, close_idx,
                                               &_ast_reconstruct_path_97),
                              _ast_reconstruct_path_97);

      } else if (strcmp(name, "suffix") == 0 && !scope) {

        out_params->suffix = (reconstruct_path(tokens, open_idx + 1, close_idx,
                                               &_ast_reconstruct_path_98),
                              _ast_reconstruct_path_98);

      } else if (strcmp(name, "if_empty") == 0 && !scope) {

        out_params->if_empty =

            (reconstruct_path(tokens, open_idx + 1, close_idx,
                              &_ast_reconstruct_path_99),
             _ast_reconstruct_path_99);

      } else {

        /* Ignore unknown parameters */
      }
    }

    free(name);

    if (scope)

      free(scope);

    i++; /* Skip RPAREN - loop continues to next param */
  }

  return 0;
}

/* --- Include Scanning & Conditional Logic --- */

/** @brief CondState definition */
enum CondState { COND_ACTIVE, COND_SKIPPING, COND_SATISFIED, COND_ELSE_SEEN };

/** @brief top */

struct ConditionalStack {

  /** @brief states[32] */
  enum CondState states[32];

  /** @brief top */
  int top;
};

static void stack_push(struct ConditionalStack *st, enum CondState s) {

  if (st->top < 31) {

    st->states[++st->top] = s;
  }
}

static void stack_pop(struct ConditionalStack *st) {

  if (st->top >= 0) {

    st->top--;
  }
}

static int stack_peek(const struct ConditionalStack *st,
                      enum CondState *_out_val) {

  if (st->top >= 0)

  {
    *_out_val = st->states[st->top];
    return 0;
  }

  {
    *_out_val = COND_ACTIVE;
    return 0;
  }
}

static int is_enabled(const struct ConditionalStack *st, bool *_out_val) {

  int i;

  for (i = 0; i <= st->top; ++i) {

    if (st->states[i] == COND_SKIPPING || st->states[i] == COND_SATISFIED)

    {
      *_out_val = false;
      return 0;
    }
  }

  {
    *_out_val = true;
    return 0;
  }
}

int pp_scan_includes(const char *const filename,

                     struct PreprocessorContext *ctx, pp_visitor_cb cb,

                     void *user_data) {
  bool _ast_token_matches_string_100;
  bool _ast_token_matches_string_101;
  bool _ast_token_matches_string_102;
  bool _ast_is_enabled_103;
  bool _ast_is_defined_macro_104;
  bool _ast_token_matches_string_105;
  bool _ast_is_enabled_106;
  bool _ast_token_matches_string_107;
  enum CondState _ast_stack_peek_108;
  bool _ast_is_enabled_109;
  bool _ast_token_matches_string_110;
  enum CondState _ast_stack_peek_111;
  bool _ast_is_enabled_112;
  bool _ast_token_matches_string_113;
  bool _ast_is_enabled_114;
  bool _ast_token_matches_string_115;
  bool _ast_token_matches_string_116;
  char *_ast_reconstruct_path_117;
  char *_ast_resolve_path_118;

  char *content = NULL;

  char *dir_name = NULL;

  size_t sz = 0;

  struct TokenList *tokens = NULL;

  struct ConditionalStack stack;

  size_t i;

  int rc = 0;

  if (!filename || !ctx)

    return EINVAL;

  /* Init stack */

  stack.top = -1;

  rc = read_to_file(filename, "r", &content, &sz);

  if (rc != 0)

    return rc;

  rc = tokenize(az_span_create_from_str(content), &tokens);

  if (rc != 0) {

    free(content);

    return rc;
  }

  rc = get_dirname(filename, &dir_name);

  if (rc != 0) {

    free_token_list(tokens);

    free(content);

    return rc;
  }

  ctx->current_file_dir = dir_name;

  for (i = 0; i < tokens->size; ++i) {

    if (tokens->tokens[i].kind == TOKEN_HASH) {

      size_t next = i + 1;

      while (next < tokens->size &&

             tokens->tokens[next].kind == TOKEN_WHITESPACE)

        next++;

      if (next < tokens->size) {

        const struct Token *cmd = &tokens->tokens[next];

        int directive_handled = 0;

        /* Find end of directive line */

        size_t eol = next + 1;

        while (eol < tokens->size) {

          size_t k = eol;

          for (; k < tokens->size; ++k) {

            if (tokens->tokens[k].kind == TOKEN_WHITESPACE) {

              const uint8_t *s = tokens->tokens[k].start;

              size_t l = tokens->tokens[k].length;

              if (memchr(s, '\n', l)) {

                eol = k;

                break;
              }
            }

            if (tokens->tokens[k].kind == TOKEN_HASH) {

              eol = k;

              break;
            }
          }

          if (k == tokens->size)

            eol = tokens->size;

          break;
        }

        /* --- Conditionals --- */

        if ((token_matches_string(cmd, "ifdef", &_ast_token_matches_string_100),
             _ast_token_matches_string_100) ||

            (token_matches_string(cmd, "ifndef",
                                  &_ast_token_matches_string_101),
             _ast_token_matches_string_101)) {

          bool inverse = (token_matches_string(cmd, "ifndef",
                                               &_ast_token_matches_string_102),
                          _ast_token_matches_string_102);

          bool enabled =
              (is_enabled(&stack, &_ast_is_enabled_103), _ast_is_enabled_103);

          bool condition_met = false;

          directive_handled = 1;

          {

            size_t id_idx = next + 1;

            while (id_idx < eol &&

                   tokens->tokens[id_idx].kind == TOKEN_WHITESPACE)

              id_idx++;

            if (id_idx < eol &&

                tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER) {

              bool defined = (is_defined_macro(ctx, &tokens->tokens[id_idx],
                                               &_ast_is_defined_macro_104),
                              _ast_is_defined_macro_104);

              condition_met = inverse ? !defined : defined;
            }
          }

          if (enabled && condition_met) {

            stack_push(&stack, COND_ACTIVE);

          } else if (enabled && !condition_met) {

            stack_push(&stack, COND_SKIPPING);

          } else {

            stack_push(&stack, COND_SATISFIED);
          }

        } else if ((token_matches_string(cmd, "if",
                                         &_ast_token_matches_string_105),
                    _ast_token_matches_string_105)) {

          bool enabled =
              (is_enabled(&stack, &_ast_is_enabled_106), _ast_is_enabled_106);

          bool condition_met = false;

          long val = 0;

          directive_handled = 1;

          if (enabled) {

            size_t start_expr = next + 1;

            pp_eval_expression(tokens, start_expr, eol, ctx, &val);

            condition_met = (val != 0);
          }

          if (enabled && condition_met) {

            stack_push(&stack, COND_ACTIVE);

          } else if (enabled && !condition_met) {

            stack_push(&stack, COND_SKIPPING);

          } else {

            stack_push(&stack, COND_SATISFIED);
          }

        } else if ((token_matches_string(cmd, "elif",
                                         &_ast_token_matches_string_107),
                    _ast_token_matches_string_107)) {

          enum CondState current =
              (stack_peek(&stack, &_ast_stack_peek_108), _ast_stack_peek_108);

          bool parent_enabled;

          stack_pop(&stack);

          parent_enabled =
              (is_enabled(&stack, &_ast_is_enabled_109), _ast_is_enabled_109);

          stack_push(&stack, current);

          directive_handled = 1;

          if (current == COND_ELSE_SEEN || current == COND_SATISFIED) {

            /* Keep skipping */

          } else if (current == COND_ACTIVE) {

            stack_pop(&stack);

            stack_push(&stack, COND_SATISFIED);

          } else if (current == COND_SKIPPING && parent_enabled) {

            long val = 0;

            size_t start_expr = next + 1;

            pp_eval_expression(tokens, start_expr, eol, ctx, &val);

            if (val != 0) {

              stack_pop(&stack);

              stack_push(&stack, COND_ACTIVE);
            }
          }

        } else if ((token_matches_string(cmd, "else",
                                         &_ast_token_matches_string_110),
                    _ast_token_matches_string_110)) {

          enum CondState current =
              (stack_peek(&stack, &_ast_stack_peek_111), _ast_stack_peek_111);

          bool parent_enabled;

          stack_pop(&stack);

          parent_enabled =
              (is_enabled(&stack, &_ast_is_enabled_112), _ast_is_enabled_112);

          stack_push(&stack, current);

          directive_handled = 1;

          if (current == COND_ACTIVE) {

            stack_pop(&stack);

            stack_push(&stack, COND_SATISFIED);

          } else if (current == COND_SKIPPING && parent_enabled) {

            stack_pop(&stack);

            stack_push(&stack, COND_ACTIVE);
          }

        } else if ((token_matches_string(cmd, "endif",
                                         &_ast_token_matches_string_113),
                    _ast_token_matches_string_113)) {

          directive_handled = 1;

          stack_pop(&stack);
        }

        /* --- Includes & Embeds --- */

        if (!directive_handled &&
            (is_enabled(&stack, &_ast_is_enabled_114), _ast_is_enabled_114)) {

          int input_is_embed =
              (token_matches_string(cmd, "embed",
                                    &_ast_token_matches_string_115),
               _ast_token_matches_string_115) ||

              tokens->tokens[next].kind == TOKEN_KEYWORD_EMBED;

          int input_is_include =
              (token_matches_string(cmd, "include",
                                    &_ast_token_matches_string_116),
               _ast_token_matches_string_116) != 0;

          if (input_is_embed || input_is_include) {

            size_t path_start = next + 1;

            size_t path_end = path_start;

            char *raw_path = NULL;

            char *resolved = NULL;

            int is_sys = 0;

            while (path_start < eol &&

                   tokens->tokens[path_start].kind == TOKEN_WHITESPACE)

              path_start++;

            if (path_start < eol) {

              /* Parse Header Name */

              if (tokens->tokens[path_start].kind == TOKEN_STRING_LITERAL) {

                const struct Token *t = &tokens->tokens[path_start];

                if (t->length >= 2) {

                  raw_path = (char *)malloc(t->length - 1);

                  if (raw_path) {

                    memcpy(raw_path, t->start + 1, t->length - 2);

                    raw_path[t->length - 2] = '\0';
                  }
                }

                path_end = path_start + 1;

              } else if (tokens->tokens[path_start].kind == TOKEN_LESS) {

                size_t end_arg = path_start + 1;

                while (end_arg < eol) {

                  if (tokens->tokens[end_arg].kind == TOKEN_GREATER)

                    break;

                  end_arg++;
                }

                if (end_arg < eol) {

                  raw_path = (reconstruct_path(tokens, path_start + 1, end_arg,
                                               &_ast_reconstruct_path_117),
                              _ast_reconstruct_path_117);

                  is_sys = 1;

                  path_end = end_arg + 1;
                }
              }

              if (raw_path) {

                resolved = (resolve_path(ctx, dir_name, raw_path, is_sys,
                                         &_ast_resolve_path_118),
                            _ast_resolve_path_118);

                if (resolved) {

                  if (cb) {

                    struct IncludeInfo info;

                    memset(&info, 0, sizeof(info));

                    info.kind = input_is_embed ? PP_DIR_EMBED : PP_DIR_INCLUDE;

                    info.resolved_path = resolved;

                    info.raw_path = raw_path;

                    info.is_system = is_sys;

                    info.params.limit = -1; /* Default -1 */

                    if (input_is_embed) {

                      /* Parse params starting at path_end up to eol */

                      if (path_end < eol) {

                        parse_embed_params(tokens, path_end, eol, ctx,

                                           &info.params);
                      }
                    }

                    if (cb(&info, user_data) != 0) {

                      i = tokens->size; /* Stop scanning */
                    }

                    if (input_is_embed) {

                      pp_embed_params_free(&info.params);
                    }
                  }

                  free(resolved);
                }

                free(raw_path);
              }
            }
          }
        }

        /* Advance outer loop to end of directive */

        i = eol - 1; /* i increments next loop */
      }
    }
  }

  ctx->current_file_dir = NULL;

  free(dir_name);

  free_token_list(tokens);

  free(content);

  return rc;
}
