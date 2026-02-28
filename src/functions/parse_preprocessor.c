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

#include "functions/parse_fs.h"

#include "functions/parse_preprocessor.h"

#include "functions/parse_str.h"

#include "functions/parse_tokenizer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

#define strdup _strdup

#endif

#define PATH_SEP_CHAR '\\'

#else

#define PATH_SEP_CHAR '/'

#include <sys/errno.h>

#endif

/* Standard IO / FS helpers */

static char *join_path(const char *dir, const char *file) {

  char *out;

  size_t len;

  if (!dir || !file)

    return NULL;

  len = strlen(dir) + strlen(file) + 2;

  out = (char *)malloc(len);

  if (!out)

    return NULL;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

  sprintf_s(out, len, "%s%c%s", dir, PATH_SEP_CHAR, file);

#else

  sprintf(out, "%s%c%s", dir, PATH_SEP_CHAR, file);

#endif

  return out;
}

static int file_exists(const char *path) {

  FILE *f;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

  if (fopen_s(&f, path, "r") == 0 && f) {

    fclose(f);

    return 1;
  }

#else

  f = fopen(path, "r");

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

static char *token_to_string(const struct Token *t) {

  char *s = malloc(t->length + 1);

  if (!s)

    return NULL;

  memcpy(s, t->start, t->length);

  s[t->length] = '\0';

  return s;
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

static char *resolve_path(const struct PreprocessorContext *ctx,

                          const char *current_dir, const char *include_path,

                          int is_system) {

  size_t i;

  char *candidate;

  if (!is_system && current_dir) {

    candidate = join_path(current_dir, include_path);

    if (candidate) {

      if (file_exists(candidate)) {

        return candidate;
      }

      free(candidate);
    }
  }

  if (ctx) {

    for (i = 0; i < ctx->size; ++i) {

      candidate = join_path(ctx->search_paths[i], include_path);

      if (candidate) {

        if (file_exists(candidate)) {

          return candidate;
        }

        free(candidate);
      }
    }
  }

  return NULL;
}

static char *reconstruct_path(const struct TokenList *tokens, size_t start,

                              size_t end) {

  size_t len = 0;

  size_t i;

  char *buf, *p;

  if (start >= end)

    return c_cdd_strdup("");

  for (i = start; i < end; ++i) {

    len += tokens->tokens[i].length;
  }

  buf = (char *)malloc(len + 1);

  if (!buf)

    return NULL;

  p = buf;

  for (i = start; i < end; ++i) {

    const struct Token *t = &tokens->tokens[i];

    memcpy(p, t->start, t->length);

    p += t->length;
  }

  *p = '\0';

  return buf;
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

  char *copy;

  char **new_paths;

  if (!ctx || !path)

    return EINVAL;

  copy = c_cdd_strdup(path);

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

  struct MacroDef def;

  if (!ctx || !name)

    return EINVAL;

  memset(&def, 0, sizeof(def));

  def.name = c_cdd_strdup(name);

  if (!def.name)

    return ENOMEM;

  if (value) {

    def.value = c_cdd_strdup(value);

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

          token_matches_string(&tokens->tokens[next], "define")) {

        size_t name_idx = next + 1;

        while (name_idx < tokens->size &&

               tokens->tokens[name_idx].kind == TOKEN_WHITESPACE)

          name_idx++;

        if (name_idx < tokens->size &&

            tokens->tokens[name_idx].kind == TOKEN_IDENTIFIER) {

          struct MacroDef def;

          memset(&def, 0, sizeof(def));

          def.name = token_to_string(&tokens->tokens[name_idx]);

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

                  char *argName = token_to_string(&tokens->tokens[curr]);

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

struct ExprState {

  const struct TokenList *tokens;

  size_t pos;

  size_t end;

  const struct PreprocessorContext *ctx;

  int error;
};

/* Forward Declarations for Recursive Descent */

static long parse_expr(struct ExprState *s);

static long parse_logic_or(struct ExprState *s);

static long parse_logic_and(struct ExprState *s);

static long parse_equality(struct ExprState *s);

static long parse_relational(struct ExprState *s);

static long parse_shift(struct ExprState *s);

static long parse_additive(struct ExprState *s);

static long parse_multiplicative(struct ExprState *s);

static long parse_unary(struct ExprState *s);

static long parse_primary(struct ExprState *s);

/* Helper: skip whitespace */

static void skip_ws(struct ExprState *s) {

  while (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_WHITESPACE)

    s->pos++;
}

/* Helper: Check current token kind */

static bool match(struct ExprState *s, enum TokenKind kind) {

  skip_ws(s);

  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == kind) {

    s->pos++;

    return true;
  }

  return false;
}

/* Helper: Peek current token kind */

static enum TokenKind peek(struct ExprState *s) {

  size_t p = s->pos;

  while (p < s->end && s->tokens->tokens[p].kind == TOKEN_WHITESPACE)

    p++;

  if (p >= s->end)

    return TOKEN_UNKNOWN;

  return s->tokens->tokens[p].kind;
}

static bool is_defined_macro(const struct PreprocessorContext *ctx,

                             const struct Token *tok) {

  size_t i;

  if (!ctx)

    return false;

  for (i = 0; i < ctx->macro_count; ++i) {

    if (token_matches_string(tok, ctx->macros[i].name))

      return true;
  }

  return false;
}

/**
 * @brief Handle __has_include and __has_embed logic.
 *
 * Syntax: `__has_include ( header-name )` or `__has_include ( "header" )` etc.
 * Spec: The operands are processed as in phase 3. The result is 1 if found, 0
 * otherwise.
 */

static long handle_has_include_embed(struct ExprState *s) {

  int is_header = false;

  char *path = NULL;

  char *resolved = NULL;

  long result = 0;

  /* Expect '(' */

  skip_ws(s);

  if (!match(s, TOKEN_LPAREN)) {

    s->error = 1;

    return 0;
  }

  skip_ws(s);

  if (s->pos >= s->end) {

    s->error = 1;

    return 0;
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

      path = reconstruct_path(s->tokens, start_p, end_p);

      s->pos = end_p + 1; /* Skip closing '>' */

      is_header = 1;

    } else {

      s->error = 1;

      return 0;
    }

  } else {

    s->error = 1;

    return 0;
  }

  /* Consume remaining parameters until RPAREN */

  while (s->pos < s->end) {

    if (s->tokens->tokens[s->pos].kind == TOKEN_RPAREN) {

      break;
    }

    s->pos++;
  }

  skip_ws(s);

  if (!match(s, TOKEN_RPAREN)) {

    s->error = 1;

    if (path)

      free(path);

    return 0;
  }

  if (path) {

    resolved = resolve_path(s->ctx, s->ctx ? s->ctx->current_file_dir : NULL,

                            path, is_header);

    result = (resolved != NULL);

    free(resolved);

    free(path);
  }

  return result;
}

/**
 * @brief Handle __has_c_attribute logic.
 */

static long handle_has_c_attribute(struct ExprState *s) {

  long result = 0;

  char *attr_name = NULL;

  skip_ws(s);

  if (!match(s, TOKEN_LPAREN)) {

    s->error = 1;

    return 0;
  }

  skip_ws(s);

  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {

    attr_name = token_to_string(&s->tokens->tokens[s->pos]);

    s->pos++;

  } else if (s->pos < s->end &&

             identify_keyword_or_id(s->tokens->tokens[s->pos].start,

                                    s->tokens->tokens[s->pos].length) !=

                 TOKEN_IDENTIFIER) {

    /* Handle keywords treated as attributes */

    attr_name = token_to_string(&s->tokens->tokens[s->pos]);

    s->pos++;
  }

  /* Support scoping `::` */

  skip_ws(s);

  if (s->pos + 1 < s->end && s->tokens->tokens[s->pos].kind == TOKEN_COLON &&

      s->tokens->tokens[s->pos + 1].kind == TOKEN_COLON) {

    char *scope = attr_name;

    char *name = NULL;

    s->pos += 2; /* Skip :: */

    skip_ws(s);

    if (s->pos < s->end &&

        (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER)) {

      name = token_to_string(&s->tokens->tokens[s->pos]);

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

  skip_ws(s);

  if (!match(s, TOKEN_RPAREN)) {

    s->error = 1;
  }

  return result;
}

static long parse_primary(struct ExprState *s) {

  skip_ws(s);

  if (s->pos >= s->end) {

    s->error = 1;

    return 0;
  }

  if (match(s, TOKEN_LPAREN)) {

    long val = parse_expr(s);

    if (!match(s, TOKEN_RPAREN))

      s->error = 1;

    return val;
  }

  if (s->tokens->tokens[s->pos].kind == TOKEN_NUMBER_LITERAL) {

    char *txt = token_to_string(&s->tokens->tokens[s->pos]);

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

    return val;
  }

  if (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER ||

      s->tokens->tokens[s->pos].kind >= TOKEN_KEYWORD_AUTO) {

    const struct Token *tok = &s->tokens->tokens[s->pos];

    if (token_matches_string(tok, "__has_include")) {

      s->pos++;

      return handle_has_include_embed(s);
    }

    if (token_matches_string(tok, "__has_embed")) {

      s->pos++;

      return handle_has_include_embed(s);
    }

    if (token_matches_string(tok, "__has_c_attribute")) {

      s->pos++;

      return handle_has_c_attribute(s);
    }

    /* Macro value lookup */

    {

      size_t i;

      long val = 0;

      if (s->ctx) {

        for (i = 0; i < s->ctx->macro_count; ++i) {

          if (!s->ctx->macros[i].is_function_like && s->ctx->macros[i].value &&

              token_matches_string(tok, s->ctx->macros[i].name)) {

            char *endptr;

            val = strtol(s->ctx->macros[i].value, &endptr, 0);

            break;
          }
        }
      }

      s->pos++;

      return val;
    }
  }

  s->pos++;

  return 0;
}

static long parse_unary(struct ExprState *s) {

  skip_ws(s);

  if (match(s, TOKEN_BANG)) {

    return !parse_unary(s);
  }

  if (match(s, TOKEN_TILDE)) {

    return ~parse_unary(s);
  }

  if (match(s, TOKEN_MINUS)) {

    return -parse_unary(s);
  }

  if (match(s, TOKEN_PLUS)) {

    return +parse_unary(s);
  }

  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER &&

      token_matches_string(&s->tokens->tokens[s->pos], "defined")) {

    bool has_paren = false;

    long result = 0;

    s->pos++;

    skip_ws(s);

    if (match(s, TOKEN_LPAREN))

      has_paren = true;

    skip_ws(s);

    if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {

      if (is_defined_macro(s->ctx, &s->tokens->tokens[s->pos])) {

        result = 1;
      }

      s->pos++;

    } else {

      s->error = 1;
    }

    if (has_paren) {

      if (!match(s, TOKEN_RPAREN))

        s->error = 1;
    }

    return result;
  }

  return parse_primary(s);
}

static long parse_multiplicative(struct ExprState *s) {

  long val = parse_unary(s);

  while (s->pos < s->end && !s->error) {

    if (match(s, TOKEN_STAR)) {

      val *= parse_unary(s);

    } else if (match(s, TOKEN_SLASH)) {

      long divisor = parse_unary(s);

      if (divisor == 0)

        val = 0;

      else

        val /= divisor;

    } else if (match(s, TOKEN_PERCENT)) {

      long divisor = parse_unary(s);

      if (divisor == 0)

        val = 0;

      else

        val %= divisor;

    } else {

      break;
    }
  }

  return val;
}

static long parse_additive(struct ExprState *s) {

  long val = parse_multiplicative(s);

  while (s->pos < s->end && !s->error) {

    if (match(s, TOKEN_PLUS)) {

      val += parse_multiplicative(s);

    } else if (match(s, TOKEN_MINUS)) {

      val -= parse_multiplicative(s);

    } else {

      break;
    }
  }

  return val;
}

static long parse_shift(struct ExprState *s) {

  long val = parse_additive(s);

  while (s->pos < s->end && !s->error) {

    if (match(s, TOKEN_LSHIFT)) {

      val <<= parse_additive(s);

    } else if (match(s, TOKEN_RSHIFT)) {

      val >>= parse_additive(s);

    } else {

      break;
    }
  }

  return val;
}

static long parse_relational(struct ExprState *s) {

  long val = parse_shift(s);

  while (s->pos < s->end && !s->error) {

    enum TokenKind k = peek(s);

    if (k == TOKEN_LEQ) {

      match(s, k);

      val = (val <= parse_shift(s));

    } else if (k == TOKEN_GEQ) {

      match(s, k);

      val = (val >= parse_shift(s));

    } else if (k == TOKEN_LESS) {

      match(s, k);

      val = (val < parse_shift(s));

    } else if (k == TOKEN_GREATER) {

      match(s, k);

      val = (val > parse_shift(s));

    } else {

      break;
    }
  }

  return val;
}

static long parse_equality(struct ExprState *s) {

  long val = parse_relational(s);

  while (s->pos < s->end && !s->error) {

    if (match(s, TOKEN_EQ)) {

      val = (val == parse_relational(s));

    } else if (match(s, TOKEN_NEQ)) {

      val = (val != parse_relational(s));

    } else {

      break;
    }
  }

  return val;
}

static long parse_logic_and(struct ExprState *s) {

  long val = parse_equality(s);

  while (match(s, TOKEN_LOGICAL_AND)) {

    long rhs = parse_equality(s);

    val = (val && rhs);
  }

  return val;
}

static long parse_logic_or(struct ExprState *s) {

  long val = parse_logic_and(s);

  while (match(s, TOKEN_LOGICAL_OR)) {

    long rhs = parse_logic_and(s);

    val = (val || rhs);
  }

  return val;
}

static long parse_expr(struct ExprState *s) { return parse_logic_or(s); }

int pp_eval_expression(const struct TokenList *tokens, size_t start_idx,

                       size_t end_idx, const struct PreprocessorContext *ctx,

                       long *result) {

  struct ExprState s;

  if (!tokens || !result)

    return EINVAL;

  s.tokens = tokens;

  s.pos = start_idx;

  s.end = end_idx;

  s.ctx = ctx;

  s.error = 0;

  *result = parse_expr(&s);

  if (s.error)

    return EINVAL;

  return 0;
}

/* --- Embed Directive Parsing --- */

static int parse_embed_params(const struct TokenList *tokens, size_t start,

                              size_t end, struct PreprocessorContext *ctx,

                              struct EmbedParams *out_params) {

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

        identify_keyword_or_id(tokens->tokens[i].start,

                               tokens->tokens[i].length) != TOKEN_IDENTIFIER) {

      /* Try identify and accept as ID if keyword-like */
    }

    name = token_to_string(&tokens->tokens[i]);

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

                      identify_keyword_or_id(tokens->tokens[i].start,

                                             tokens->tokens[i].length) !=

                          TOKEN_IDENTIFIER)) {

        name = token_to_string(&tokens->tokens[i]);

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

        out_params->prefix = reconstruct_path(tokens, open_idx + 1, close_idx);

      } else if (strcmp(name, "suffix") == 0 && !scope) {

        out_params->suffix = reconstruct_path(tokens, open_idx + 1, close_idx);

      } else if (strcmp(name, "if_empty") == 0 && !scope) {

        out_params->if_empty =

            reconstruct_path(tokens, open_idx + 1, close_idx);

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

enum CondState { COND_ACTIVE, COND_SKIPPING, COND_SATISFIED, COND_ELSE_SEEN };

struct ConditionalStack {

  enum CondState states[32];

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

static enum CondState stack_peek(const struct ConditionalStack *st) {

  if (st->top >= 0)

    return st->states[st->top];

  return COND_ACTIVE;
}

static bool is_enabled(const struct ConditionalStack *st) {

  int i;

  for (i = 0; i <= st->top; ++i) {

    if (st->states[i] == COND_SKIPPING || st->states[i] == COND_SATISFIED)

      return false;
  }

  return true;
}

int pp_scan_includes(const char *const filename,

                     struct PreprocessorContext *ctx, pp_visitor_cb cb,

                     void *user_data) {

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

        if (token_matches_string(cmd, "ifdef") ||

            token_matches_string(cmd, "ifndef")) {

          bool inverse = token_matches_string(cmd, "ifndef");

          bool enabled = is_enabled(&stack);

          bool condition_met = false;

          directive_handled = 1;

          {

            size_t id_idx = next + 1;

            while (id_idx < eol &&

                   tokens->tokens[id_idx].kind == TOKEN_WHITESPACE)

              id_idx++;

            if (id_idx < eol &&

                tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER) {

              bool defined = is_defined_macro(ctx, &tokens->tokens[id_idx]);

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

        } else if (token_matches_string(cmd, "if")) {

          bool enabled = is_enabled(&stack);

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

        } else if (token_matches_string(cmd, "elif")) {

          enum CondState current = stack_peek(&stack);

          bool parent_enabled;

          stack_pop(&stack);

          parent_enabled = is_enabled(&stack);

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

        } else if (token_matches_string(cmd, "else")) {

          enum CondState current = stack_peek(&stack);

          bool parent_enabled;

          stack_pop(&stack);

          parent_enabled = is_enabled(&stack);

          stack_push(&stack, current);

          directive_handled = 1;

          if (current == COND_ACTIVE) {

            stack_pop(&stack);

            stack_push(&stack, COND_SATISFIED);

          } else if (current == COND_SKIPPING && parent_enabled) {

            stack_pop(&stack);

            stack_push(&stack, COND_ACTIVE);
          }

        } else if (token_matches_string(cmd, "endif")) {

          directive_handled = 1;

          stack_pop(&stack);
        }

        /* --- Includes & Embeds --- */

        if (!directive_handled && is_enabled(&stack)) {

          int input_is_embed = token_matches_string(cmd, "embed") ||

                               tokens->tokens[next].kind == TOKEN_KEYWORD_EMBED;

          int input_is_include = token_matches_string(cmd, "include") != 0;

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

                  raw_path = reconstruct_path(tokens, path_start + 1, end_arg);

                  is_sys = 1;

                  path_end = end_arg + 1;
                }
              }

              if (raw_path) {

                resolved = resolve_path(ctx, dir_name, raw_path, is_sys);

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
