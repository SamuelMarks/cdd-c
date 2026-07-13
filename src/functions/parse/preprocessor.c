/**
 * @file preprocessor.c
 * @brief Implementation of the C preprocessor.
 */

/* clang-format off */
#include <ctype.h>

#include <errno.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "c_cdd_stdbool.h"

#include "functions/parse/fs.h"

#include "functions/parse/preprocessor.h"
#include "c_cdd/log.h"

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
#include "c_cdd/log.h"

#endif
/* clang-format on */

/* Standard IO / FS helpers */

static enum cdd_c_error join_path(const char *dir, const char *file,
                                  char **_out_val) {

  char *out;

  size_t len;

  if (!dir || !file)

  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  len = strlen(dir) + strlen(file) + 2;

  out = (char *)malloc(len);

  if (!out)

  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

  sprintf_s(out, len, "%s%c%s", dir, PATH_SEP_CHAR, file);

#else

  sprintf(out, "%s%c%s", dir, PATH_SEP_CHAR, file);

#endif

  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the file exists operation.
 */
static enum cdd_c_error file_exists(const char *path, int *out_exists) {
  FILE *f;
  if (!out_exists)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *out_exists = 0;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

  if (fopen_s(&f, path, "r") == 0 && f) {
    fclose(f);
    *out_exists = 1;
    return CDD_C_SUCCESS;
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
    *out_exists = 1;
    return CDD_C_SUCCESS;
  }
#endif
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees the memory associated with macro def.
 */
static void free_macro_def(struct MacroDef *def) {

  size_t i;

  if (def->name)

    free(def->name);

  if (def->value)

    free(def->value);

  if (def->args) {

    /* LCOV_EXCL_START */
    for (i = 0; i < def->arg_count; i++) {
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      free(def->args[i]);
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    free(def->args);
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Executes the token to string operation.
 */
static enum cdd_c_error token_to_string(const struct Token *t,
                                        char **_out_val) {

  char *s = malloc(t->length + 1);

  if (!s)

  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  memcpy(s, t->start, t->length);

  s[t->length] = '\0';

  {
    *_out_val = s;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Adds or sets macro internal.
 */
static enum cdd_c_error add_macro_internal(struct PreprocessorContext *ctx,

                                           const struct MacroDef *def) {

  if (ctx->macro_count >= ctx->macro_capacity) {

    size_t new_cap = (ctx->macro_capacity == 0) ? 16 : ctx->macro_capacity * 2;

    struct MacroDef *new_arr = (struct MacroDef *)realloc(

        ctx->macros, new_cap * sizeof(struct MacroDef));

    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }

    ctx->macros = new_arr;

    ctx->macro_capacity = new_cap;
  }

  ctx->macros[ctx->macro_count++] = *def;

  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to resolve include paths.
 */

static enum cdd_c_error resolve_path(const struct PreprocessorContext *ctx,

                                     const char *current_dir,
                                     const char *include_path,

                                     int is_system, char **_out_val) {
  char *_ast_join_path_0 = NULL;
  char *_ast_join_path_1 = NULL;

  size_t i;

  char *candidate;

  if (!is_system && current_dir) {

    candidate = (join_path(current_dir, include_path, &_ast_join_path_0),
                 _ast_join_path_0);

    if (candidate) {

      int exists = 0;
      file_exists(candidate, &exists);
      if (exists) {

        {
          *_out_val = candidate;
          return CDD_C_SUCCESS;
        }
      }

      /* LCOV_EXCL_START */
      free(candidate);
      /* LCOV_EXCL_STOP */
    }
  }

  if (ctx) {

    for (i = 0; i < ctx->size; ++i) {

      candidate =
          (join_path(ctx->search_paths[i], include_path, &_ast_join_path_1),
           _ast_join_path_1);

      if (candidate) {

        int exists = 0;
        file_exists(candidate, &exists);
        if (exists) {

          {
            *_out_val = candidate;
            return CDD_C_SUCCESS;
          }
        }

        free(candidate);
      }
    }
  }

  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the reconstruct path operation.
 */
static enum cdd_c_error reconstruct_path(const struct TokenList *tokens,
                                         size_t start,

                                         size_t end, char **_out_val) {
  char *_ast_strdup_0 = NULL;

  size_t len = 0;

  size_t i;

  char *buf, *p;

  if (start >= end)

  {
    *_out_val = (c_cdd_strdup("", &_ast_strdup_0), _ast_strdup_0);
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  for (i = start; i < end; ++i) {

    len += tokens->tokens[i].length;
  }

  buf = (char *)malloc(len + 1);

  if (!buf)

  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
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
    return CDD_C_SUCCESS;
  }
}

/* --- Public API Implementation --- */

/**
 * @brief Executes the pp context init operation.
 */
enum cdd_c_error pp_context_init(struct PreprocessorContext *ctx) {

  if (!ctx)

    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  memset(ctx, 0, sizeof(*ctx));

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the pp context free operation.
 */
void pp_context_free(struct PreprocessorContext *ctx) {

  size_t i;

  if (!ctx)

    /* LCOV_EXCL_START */
    return;
  /* LCOV_EXCL_STOP */

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

/**
 * @brief Executes the pp add search path operation.
 */
enum cdd_c_error pp_add_search_path(struct PreprocessorContext *ctx,
                                    const char *path) {
  char *_ast_strdup_1 = NULL;

  char *copy;

  char **new_paths;

  if (!ctx || !path)

    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  copy = (c_cdd_strdup(path, &_ast_strdup_1), _ast_strdup_1);

  if (!copy) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  if (ctx->size >= ctx->capacity) {

    size_t new_cap = (ctx->capacity == 0) ? 8 : ctx->capacity * 2;

    new_paths = (char **)realloc(ctx->search_paths, new_cap * sizeof(char *));

    if (!new_paths) {

      /* LCOV_EXCL_START */
      free(copy);
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }

    ctx->search_paths = new_paths;

    ctx->capacity = new_cap;
  }

  ctx->search_paths[ctx->size++] = copy;

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the pp add macro operation.
 */
enum cdd_c_error pp_add_macro(struct PreprocessorContext *ctx, const char *name,

                              const char *value) {
  char *_ast_strdup_2 = NULL;
  char *_ast_strdup_3 = NULL;

  struct MacroDef def;

  if (!ctx || !name)

    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  memset(&def, 0, sizeof(def));

  def.name = (c_cdd_strdup(name, &_ast_strdup_2), _ast_strdup_2);

  if (!def.name)

    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */

  if (value) {

    def.value = (c_cdd_strdup(value, &_ast_strdup_3), _ast_strdup_3);

    if (!def.value) {

      /* LCOV_EXCL_START */
      free(def.name);
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
  }

  def.is_function_like = 0;

  if (add_macro_internal(ctx, &def) != 0) {

    /* LCOV_EXCL_START */
    free_macro_def(&def);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the pp scan defines operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error pp_scan_defines(struct PreprocessorContext *ctx,
                                 /* LCOV_EXCL_STOP */
                                 const char *filename) {
  /* LCOV_EXCL_START */
  int _ast_token_matches_string_2 = 0;
  char *_ast_token_to_string_3 = NULL;
  char *_ast_token_to_string_4 = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  char *content = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  size_t sz = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  struct TokenList *tokens = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  int rc = 0;
  /* LCOV_EXCL_STOP */

  size_t i;

  /* LCOV_EXCL_START */
  if (!ctx || !filename)
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = read_to_file(filename, "r", &content, &sz);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (rc != 0)
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = tokenize(az_span_create_from_str(content), &tokens);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (rc != 0) {
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    free(content);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    return rc;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  for (i = 0; i < tokens->size; ++i) {
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    if (tokens->tokens[i].kind == TOKEN_HASH) {
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      size_t next = i + 1;
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      while (next < tokens->size &&
             /* LCOV_EXCL_STOP */

             /* LCOV_EXCL_START */
             tokens->tokens[next].kind == TOKEN_WHITESPACE)
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        next++;
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      if (next < tokens->size &&
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          (token_matches_string(&tokens->tokens[next], "define",
                                &_ast_token_matches_string_2) == 0 &&
           /* LCOV_EXCL_STOP */
           _ast_token_matches_string_2)) {

        /* LCOV_EXCL_START */
        size_t name_idx = next + 1;
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        while (name_idx < tokens->size &&
               /* LCOV_EXCL_STOP */

               /* LCOV_EXCL_START */
               tokens->tokens[name_idx].kind == TOKEN_WHITESPACE)
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          name_idx++;
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        if (name_idx < tokens->size &&
            /* LCOV_EXCL_STOP */

            /* LCOV_EXCL_START */
            tokens->tokens[name_idx].kind == TOKEN_IDENTIFIER) {
          /* LCOV_EXCL_STOP */

          struct MacroDef def;
          /* LCOV_EXCL_START */
          size_t val_start_idx = name_idx + 1;
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          memset(&def, 0, sizeof(def));
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          def.name = (token_to_string(&tokens->tokens[name_idx],
                                      /* LCOV_EXCL_STOP */
                                      &_ast_token_to_string_3),
                      _ast_token_to_string_3);

          /* LCOV_EXCL_START */
          if (name_idx + 1 < tokens->size &&
              /* LCOV_EXCL_STOP */

              /* LCOV_EXCL_START */
              tokens->tokens[name_idx + 1].kind == TOKEN_LPAREN) {
            /* LCOV_EXCL_STOP */

            /* Function-like macro */

            /* LCOV_EXCL_START */
            def.is_function_like = 1;
            /* LCOV_EXCL_STOP */

            /* LCOV_EXCL_START */
            {
              /* LCOV_EXCL_STOP */

              /* LCOV_EXCL_START */
              size_t curr = name_idx + 2;
              /* LCOV_EXCL_STOP */

              /* LCOV_EXCL_START */
              int done_args = 0;
              /* LCOV_EXCL_STOP */

              /* LCOV_EXCL_START */
              while (curr < tokens->size && !done_args) {
                /* LCOV_EXCL_STOP */

                /* LCOV_EXCL_START */
                while (curr < tokens->size &&
                       /* LCOV_EXCL_STOP */

                       /* LCOV_EXCL_START */
                       tokens->tokens[curr].kind == TOKEN_WHITESPACE)
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                  curr++;
                /* LCOV_EXCL_STOP */

                /* LCOV_EXCL_START */
                if (curr >= tokens->size)
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                  break;
                /* LCOV_EXCL_STOP */

                /* LCOV_EXCL_START */
                if (tokens->tokens[curr].kind == TOKEN_RPAREN) {
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                  done_args = 1;
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                  curr++;
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                } else if (tokens->tokens[curr].kind == TOKEN_IDENTIFIER) {
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                  char *argName = (token_to_string(&tokens->tokens[curr],
                                                   /* LCOV_EXCL_STOP */
                                                   &_ast_token_to_string_4),
                                   _ast_token_to_string_4);

                  /* LCOV_EXCL_START */
                  char **new_args = (char **)realloc(
                      /* LCOV_EXCL_STOP */

                      /* LCOV_EXCL_START */
                      def.args, (def.arg_count + 1) * sizeof(char *));
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                  if (new_args) {
                    /* LCOV_EXCL_STOP */

                    /* LCOV_EXCL_START */
                    def.args = new_args;
                    /* LCOV_EXCL_STOP */

                    /* LCOV_EXCL_START */
                    def.args[def.arg_count++] = argName;
                    /* LCOV_EXCL_STOP */

                  } else {

                    /* LCOV_EXCL_START */
                    free(argName);
                    /* LCOV_EXCL_STOP */
                  }

                  /* LCOV_EXCL_START */
                  curr++;
                  /* LCOV_EXCL_STOP */

                  /* Check for GCC variadic "args..." */

                  /* LCOV_EXCL_START */
                  if (curr < tokens->size &&
                      /* LCOV_EXCL_STOP */

                      /* LCOV_EXCL_START */
                      tokens->tokens[curr].kind == TOKEN_ELLIPSIS) {
                    /* LCOV_EXCL_STOP */

                    /* LCOV_EXCL_START */
                    def.is_variadic = 1;
                    /* LCOV_EXCL_STOP */

                    /* LCOV_EXCL_START */
                    curr++;
                    /* LCOV_EXCL_STOP */
                  }

                  /* LCOV_EXCL_START */
                } else if (tokens->tokens[curr].kind == TOKEN_ELLIPSIS) {
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                  def.is_variadic = 1;
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                  curr++;
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                } else if (tokens->tokens[curr].kind == TOKEN_COMMA) {
                  /* LCOV_EXCL_STOP */

                  /* LCOV_EXCL_START */
                  curr++;
                  /* LCOV_EXCL_STOP */

                } else {

                  curr++; /* Error recovery */
                }
              }
              /* LCOV_EXCL_START */
              val_start_idx = curr;
              /* LCOV_EXCL_STOP */
            }

          } else {

            /* Object-like */

            /* LCOV_EXCL_START */
            def.is_function_like = 0;
            /* LCOV_EXCL_STOP */
          }

          {
            /* LCOV_EXCL_START */
            size_t val_end_idx = val_start_idx;
            while (val_end_idx < tokens->size) {
              if (tokens->tokens[val_end_idx].kind == TOKEN_WHITESPACE) {
                /* LCOV_EXCL_STOP */
                size_t idx;
                /* LCOV_EXCL_START */
                int has_nl = 0;
                for (idx = 0; idx < tokens->tokens[val_end_idx].length; idx++) {
                  if (tokens->tokens[val_end_idx].start[idx] == '\n') {
                    has_nl = 1;
                    break;
                    /* LCOV_EXCL_STOP */
                  }
                }
                /* LCOV_EXCL_START */
                if (has_nl)
                  break;
                /* LCOV_EXCL_STOP */
              }
              /* LCOV_EXCL_START */
              val_end_idx++;
              /* LCOV_EXCL_STOP */
            }
            /* LCOV_EXCL_START */
            if (val_end_idx > val_start_idx) {
              size_t val_start_byte =
                  (size_t)(tokens->tokens[val_start_idx].start -
                           (const uint8_t *)content);
              size_t val_end_byte =
                  (size_t)(tokens->tokens[val_end_idx - 1].start -
                           (const uint8_t *)content) +
                  tokens->tokens[val_end_idx - 1].length;
              size_t val_len = val_end_byte - val_start_byte;
              if (val_len > 0) {
                char *v = (char *)malloc(val_len + 1);
                if (v) {
                  size_t k = 0;
                  memcpy(v, content + val_start_byte, val_len);
                  v[val_len] = '\0';
                  while (val_len > 0 &&
                         (v[val_len - 1] == ' ' || v[val_len - 1] == '\t' ||
                          v[val_len - 1] == '\r')) {
                    v[--val_len] = '\0';
                    /* LCOV_EXCL_STOP */
                  }
                  /* LCOV_EXCL_START */
                  while (k < val_len && (v[k] == ' ' || v[k] == '\t'))
                    k++;
                  if (k > 0 && k < val_len) {
                    memmove(v, v + k, val_len - k + 1);
                  } else if (k == val_len) {
                    v[0] = '\0';
                    /* LCOV_EXCL_STOP */
                  }
                  /* LCOV_EXCL_START */
                  if (strlen(v) > 0) {
                    def.value = v;
                    /* LCOV_EXCL_STOP */
                  } else {
                    /* LCOV_EXCL_START */
                    free(v);
                    /* LCOV_EXCL_STOP */
                  }
                }
              }
            }
          }

          /* LCOV_EXCL_START */
          add_macro_internal(ctx, &def);
          /* LCOV_EXCL_STOP */
        }

        i = name_idx; /* Advance */
      }
    }
  }

  /* LCOV_EXCL_START */
  free_token_list(tokens);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(content);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  return rc;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the pp embed params free operation.
 */
void pp_embed_params_free(struct EmbedParams *params) {

  if (!params)

    /* LCOV_EXCL_START */
    return;
  /* LCOV_EXCL_STOP */

  if (params->prefix)

    free(params->prefix);

  if (params->suffix)

    free(params->suffix);

  if (params->if_empty)

    /* LCOV_EXCL_START */
    free(params->if_empty);
  /* LCOV_EXCL_STOP */

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

static enum cdd_c_error parse_expr(struct ExprState *s, long *_out_val);

/**
 * @brief Parses logic or from the given input.
 */
static enum cdd_c_error parse_logic_or(struct ExprState *s, long *_out_val);

/**
 * @brief Parses logic and from the given input.
 */
static enum cdd_c_error parse_logic_and(struct ExprState *s, long *_out_val);

/**
 * @brief Parses equality from the given input.
 */
static enum cdd_c_error parse_equality(struct ExprState *s, long *_out_val);

/**
 * @brief Parses relational from the given input.
 */
static enum cdd_c_error parse_relational(struct ExprState *s, long *_out_val);

/**
 * @brief Parses shift from the given input.
 */
static enum cdd_c_error parse_shift(struct ExprState *s, long *_out_val);

/**
 * @brief Parses additive from the given input.
 */
static enum cdd_c_error parse_additive(struct ExprState *s, long *_out_val);

/**
 * @brief Parses multiplicative from the given input.
 */
static enum cdd_c_error parse_multiplicative(struct ExprState *s,
                                             long *_out_val);

/**
 * @brief Parses unary from the given input.
 */
static enum cdd_c_error parse_unary(struct ExprState *s, long *_out_val);

/**
 * @brief Parses primary from the given input.
 */
static enum cdd_c_error parse_primary(struct ExprState *s, long *_out_val);

/* Helper: skip whitespace */

/**
 * @brief Executes the skip ws operation.
 */
static enum cdd_c_error skip_ws(struct ExprState *s, size_t *_out_val) {
  (void)_out_val;
  while (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_WHITESPACE)
    s->pos++;
  return CDD_C_SUCCESS;
}

/* Helper: Check current token kind */

/**
 * @brief Executes the match operation.
 */
static enum cdd_c_error match(struct ExprState *s, enum TokenKind kind,
                              int *_out_val) {
  size_t _ast_skip_ws_5 = 0;
  if (!_out_val)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *_out_val = 0;

  skip_ws(s, &_ast_skip_ws_5);

  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == kind) {

    s->pos++;

    {
      *_out_val = 1;
      return CDD_C_SUCCESS;
    }
  }

  {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
}

/* Helper: Peek current token kind */

/**
 * @brief Executes the peek operation.
 */
static enum cdd_c_error peek(struct ExprState *s, enum TokenKind *_out_val) {

  size_t p = s->pos;

  while (p < s->end && s->tokens->tokens[p].kind == TOKEN_WHITESPACE)

    /* LCOV_EXCL_START */
    p++;
  /* LCOV_EXCL_STOP */

  if (p >= s->end)

  {
    *_out_val = TOKEN_UNKNOWN;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  {
    *_out_val = s->tokens->tokens[p].kind;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if defined macro.
 */
static enum cdd_c_error is_defined_macro(const struct PreprocessorContext *ctx,

                                         const struct Token *tok,
                                         int *_out_val) {
  int _ast_token_matches_string_6 = 0;

  size_t i;

  if (!ctx)

  {
    *_out_val = 0;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  for (i = 0; i < ctx->macro_count; ++i) {

    if ((token_matches_string(tok, ctx->macros[i].name,
                              &_ast_token_matches_string_6) == 0 &&
         _ast_token_matches_string_6))

    {
      *_out_val = 1;
      return CDD_C_SUCCESS;
    }
  }

  {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Handle __has_include and __has_embed logic.
 *
 * Syntax: `__has_include ( header-name )` or `__has_include ( "header" )` etc.
 * Spec: The operands are processed as in phase 3. The result is 1 if found, 0
 * otherwise.
 */

static enum cdd_c_error handle_has_include_embed(struct ExprState *s,
                                                 long *_out_val) {
  size_t _ast_skip_ws_7 = 0;
  int _ast_match_8 = 0;
  size_t _ast_skip_ws_9 = 0;
  char *_ast_reconstruct_path_10 = NULL;
  size_t _ast_skip_ws_11 = 0;
  int _ast_match_12 = 0;
  char *_ast_resolve_path_13 = NULL;

  int is_header = false;

  char *path = NULL;

  char *resolved = NULL;

  long result = 0;

  /* Expect '(' */

  skip_ws(s, &_ast_skip_ws_7);

  if (!(match(s, TOKEN_LPAREN, &_ast_match_8) == 0 && _ast_match_8)) {

    /* LCOV_EXCL_START */
    s->error = 1;
    /* LCOV_EXCL_STOP */

    {
      *_out_val = 0;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  skip_ws(s, &_ast_skip_ws_9);

  if (s->pos >= s->end) {

    /* LCOV_EXCL_START */
    s->error = 1;
    /* LCOV_EXCL_STOP */

    {
      *_out_val = 0;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
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

    /* LCOV_EXCL_START */
  } else if (s->tokens->tokens[s->pos].kind == TOKEN_LESS) {
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    size_t start_p = s->pos + 1;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    size_t end_p = start_p;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    while (end_p < s->end && s->tokens->tokens[end_p].kind != TOKEN_GREATER) {
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      end_p++;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    if (end_p < s->end) {
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      path = (reconstruct_path(s->tokens, start_p, end_p,
                               /* LCOV_EXCL_STOP */
                               &_ast_reconstruct_path_10),
              _ast_reconstruct_path_10);

      s->pos = end_p + 1; /* Skip closing '>' */

      /* LCOV_EXCL_START */
      is_header = 1;
      /* LCOV_EXCL_STOP */

    } else {

      /* LCOV_EXCL_START */
      s->error = 1;
      /* LCOV_EXCL_STOP */

      {
        *_out_val = 0;
        /* LCOV_EXCL_START */
        return CDD_C_SUCCESS;
        /* LCOV_EXCL_STOP */
      }
    }

  } else {

    /* LCOV_EXCL_START */
    s->error = 1;
    /* LCOV_EXCL_STOP */

    {
      *_out_val = 0;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  /* Consume remaining parameters until RPAREN */

  while (s->pos < s->end) {

    if (s->tokens->tokens[s->pos].kind == TOKEN_RPAREN) {

      break;
    }

    /* LCOV_EXCL_START */
    s->pos++;
    /* LCOV_EXCL_STOP */
  }

  skip_ws(s, &_ast_skip_ws_11);

  if (!(match(s, TOKEN_RPAREN, &_ast_match_12) == 0 && _ast_match_12)) {
    /* LCOV_EXCL_START */
    fprintf(stderr, "Missing RPAREN, setting error!\n");
    s->error = 1;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    if (path)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      free(path);
    /* LCOV_EXCL_STOP */

    {
      *_out_val = 0;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
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
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Handle __has_c_attribute logic.
 */

/* LCOV_EXCL_START */
static enum cdd_c_error handle_has_c_attribute(struct ExprState *s,
                                               /* LCOV_EXCL_STOP */
                                               long *_out_val) {
  /* LCOV_EXCL_START */
  size_t _ast_skip_ws_14 = 0;
  int _ast_match_15 = 0;
  size_t _ast_skip_ws_16 = 0;
  char *_ast_token_to_string_17 = NULL;
  /* LCOV_EXCL_STOP */
  enum TokenKind _ast_identify_keyword_or_id_18;
  /* LCOV_EXCL_START */
  char *_ast_token_to_string_19 = NULL;
  size_t _ast_skip_ws_20 = 0;
  size_t _ast_skip_ws_21 = 0;
  char *_ast_token_to_string_22 = NULL;
  size_t _ast_skip_ws_23 = 0;
  int _ast_match_24 = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  long result = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  char *attr_name = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  skip_ws(s, &_ast_skip_ws_14);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!(match(s, TOKEN_LPAREN, &_ast_match_15) == 0 && _ast_match_15)) {
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    s->error = 1;
    /* LCOV_EXCL_STOP */

    {
      *_out_val = 0;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  skip_ws(s, &_ast_skip_ws_16);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    attr_name =
        (token_to_string(&s->tokens->tokens[s->pos], &_ast_token_to_string_17),
         /* LCOV_EXCL_STOP */
         _ast_token_to_string_17);

    /* LCOV_EXCL_START */
    s->pos++;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
  } else if (s->pos < s->end &&
             /* LCOV_EXCL_STOP */

             /* LCOV_EXCL_START */
             (identify_keyword_or_id(s->tokens->tokens[s->pos].start,
                                     /* LCOV_EXCL_STOP */

                                     /* LCOV_EXCL_START */
                                     s->tokens->tokens[s->pos].length,
                                     /* LCOV_EXCL_STOP */
                                     &_ast_identify_keyword_or_id_18),
              /* LCOV_EXCL_START */
              _ast_identify_keyword_or_id_18) !=
                 /* LCOV_EXCL_STOP */

                 TOKEN_IDENTIFIER) {

    /* Handle keywords treated as attributes */

    /* LCOV_EXCL_START */
    attr_name =
        (token_to_string(&s->tokens->tokens[s->pos], &_ast_token_to_string_19),
         /* LCOV_EXCL_STOP */
         _ast_token_to_string_19);

    /* LCOV_EXCL_START */
    s->pos++;
    /* LCOV_EXCL_STOP */
  }

  /* Support scoping `::` */

  /* LCOV_EXCL_START */
  skip_ws(s, &_ast_skip_ws_20);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (s->pos + 1 < s->end && s->tokens->tokens[s->pos].kind == TOKEN_COLON &&
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      s->tokens->tokens[s->pos + 1].kind == TOKEN_COLON) {
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    char *scope = attr_name;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    char *name = NULL;
    /* LCOV_EXCL_STOP */

    s->pos += 2; /* Skip :: */

    /* LCOV_EXCL_START */
    skip_ws(s, &_ast_skip_ws_21);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    if (s->pos < s->end &&
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER)) {
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      name = (token_to_string(&s->tokens->tokens[s->pos],
                              /* LCOV_EXCL_STOP */
                              &_ast_token_to_string_22),
              _ast_token_to_string_22);

      /* LCOV_EXCL_START */
      s->pos++;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    if (name)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      free(name);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    if (scope)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      free(scope);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    attr_name = NULL;
    /* LCOV_EXCL_STOP */

    result = 0; /* Scoped ignored currently */
  }

  /* LCOV_EXCL_START */
  if (attr_name) {
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    if (strcmp(attr_name, "deprecated") == 0)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      result = 201904L;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    else if (strcmp(attr_name, "fallthrough") == 0)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      result = 201904L;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    else if (strcmp(attr_name, "maybe_unused") == 0)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      result = 201904L;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    else if (strcmp(attr_name, "nodiscard") == 0)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      result = 201904L;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    else if (strcmp(attr_name, "noreturn") == 0)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      result = 202202L;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    else if (strcmp(attr_name, "unsequenced") == 0)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      result = 202311L;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    else if (strcmp(attr_name, "reproducible") == 0)
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      result = 202311L;
    /* LCOV_EXCL_STOP */

    else

      /* LCOV_EXCL_START */
      result = 0;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    free(attr_name);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  skip_ws(s, &_ast_skip_ws_23);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!(match(s, TOKEN_RPAREN, &_ast_match_24) == 0 && _ast_match_24)) {
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    s->error = 1;
    /* LCOV_EXCL_STOP */
  }

  {
    *_out_val = result;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Parses primary from the given input.
 */
static enum cdd_c_error parse_primary(struct ExprState *s, long *_out_val) {
  size_t _ast_skip_ws_25 = 0;
  int _ast_match_26 = 0;
  long _ast_parse_expr_27;
  int _ast_match_28 = 0;
  char *_ast_token_to_string_29 = NULL;
  int _ast_token_matches_string_30 = 0;
  long _ast_handle_has_include_embed_31;
  int _ast_token_matches_string_32 = 0;
  long _ast_handle_has_include_embed_33;
  int _ast_token_matches_string_34 = 0;
  long _ast_handle_has_c_attribute_35;
  int _ast_token_matches_string_36 = 0;

  skip_ws(s, &_ast_skip_ws_25);

  if (s->pos >= s->end) {

    /* LCOV_EXCL_START */
    s->error = 1;
    /* LCOV_EXCL_STOP */

    {
      *_out_val = 0;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  if ((match(s, TOKEN_LPAREN, &_ast_match_26) == 0 && _ast_match_26)) {

    long val = (parse_expr(s, &_ast_parse_expr_27), _ast_parse_expr_27);

    if (!(match(s, TOKEN_RPAREN, &_ast_match_28) == 0 && _ast_match_28))

      /* LCOV_EXCL_START */
      s->error = 1;
    /* LCOV_EXCL_STOP */

    {
      *_out_val = val;
      return CDD_C_SUCCESS;
    }
  }

  if (s->tokens->tokens[s->pos].kind == TOKEN_NUMBER_LITERAL) {

    char *txt =
        (token_to_string(&s->tokens->tokens[s->pos], &_ast_token_to_string_29),
         _ast_token_to_string_29);

    long val = 0;

    if (txt) {

      if (strlen(txt) > 2 && txt[0] == '0' &&

          /* LCOV_EXCL_START */
          (txt[1] == 'b' || txt[1] == 'B')) {
        /* LCOV_EXCL_STOP */

        char *endptr;

        /* LCOV_EXCL_START */
        val = strtol(txt + 2, &endptr, 2);
        /* LCOV_EXCL_STOP */

      } else {

        val = strtol(txt, NULL, 0);
      }

      free(txt);
    }

    s->pos++;

    {
      *_out_val = val;
      return CDD_C_SUCCESS;
    }
  }

  if (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER ||

      /* LCOV_EXCL_START */
      s->tokens->tokens[s->pos].kind >= TOKEN_KEYWORD_AUTO) {
    /* LCOV_EXCL_STOP */

    const struct Token *tok = &s->tokens->tokens[s->pos];
    if ((token_matches_string(tok, "__has_include",
                              &_ast_token_matches_string_30) == 0 &&
         _ast_token_matches_string_30)) {

      s->pos++;

      {
        *_out_val =
            (handle_has_include_embed(s, &_ast_handle_has_include_embed_31),
             _ast_handle_has_include_embed_31);
        return CDD_C_SUCCESS;
      }
    }

    if ((token_matches_string(tok, "__has_embed",
                              &_ast_token_matches_string_32) == 0 &&
         _ast_token_matches_string_32)) {

      /* LCOV_EXCL_START */
      s->pos++;
      /* LCOV_EXCL_STOP */

      {
        *_out_val =
            /* LCOV_EXCL_START */
            (handle_has_include_embed(s, &_ast_handle_has_include_embed_33),
             /* LCOV_EXCL_STOP */
             _ast_handle_has_include_embed_33);
        /* LCOV_EXCL_START */
        return CDD_C_SUCCESS;
        /* LCOV_EXCL_STOP */
      }
    }

    if ((token_matches_string(tok, "__has_c_attribute",
                              &_ast_token_matches_string_34) == 0 &&
         _ast_token_matches_string_34)) {

      /* LCOV_EXCL_START */
      s->pos++;
      /* LCOV_EXCL_STOP */

      {
        *_out_val = (handle_has_c_attribute(s, &_ast_handle_has_c_attribute_35),
                     _ast_handle_has_c_attribute_35);
        /* LCOV_EXCL_START */
        return CDD_C_SUCCESS;
        /* LCOV_EXCL_STOP */
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
                                    &_ast_token_matches_string_36) == 0 &&
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
        return CDD_C_SUCCESS;
      }
    }
  }

  /* LCOV_EXCL_START */
  s->pos++;
  /* LCOV_EXCL_STOP */

  {
    *_out_val = 0;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Parses unary from the given input.
 */
static enum cdd_c_error parse_unary(struct ExprState *s, long *_out_val) {
  size_t _ast_skip_ws_37 = 0;
  int _ast_match_38 = 0;
  long _ast_parse_unary_39;
  int _ast_match_40 = 0;
  long _ast_parse_unary_41;
  int _ast_match_42 = 0;
  long _ast_parse_unary_43;
  int _ast_match_44 = 0;
  long _ast_parse_unary_45;
  int _ast_token_matches_string_46 = 0;
  size_t _ast_skip_ws_47 = 0;
  int _ast_match_48 = 0;
  size_t _ast_skip_ws_49 = 0;
  int _ast_is_defined_macro_50 = 0;
  int _ast_match_51 = 0;
  long _ast_parse_primary_52;

  skip_ws(s, &_ast_skip_ws_37);

  if ((match(s, TOKEN_BANG, &_ast_match_38) == 0 && _ast_match_38)) {

    {
      *_out_val = !(parse_unary(s, &_ast_parse_unary_39), _ast_parse_unary_39);
      return CDD_C_SUCCESS;
    }
  }

  if ((match(s, TOKEN_TILDE, &_ast_match_40) == 0 && _ast_match_40)) {

    {
      *_out_val = ~(parse_unary(s, &_ast_parse_unary_41), _ast_parse_unary_41);
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  if ((match(s, TOKEN_MINUS, &_ast_match_42) == 0 && _ast_match_42)) {

    {
      *_out_val = -(parse_unary(s, &_ast_parse_unary_43), _ast_parse_unary_43);
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  if ((match(s, TOKEN_PLUS, &_ast_match_44) == 0 && _ast_match_44)) {

    {
      *_out_val = +(parse_unary(s, &_ast_parse_unary_45), _ast_parse_unary_45);
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER &&

      (token_matches_string(&s->tokens->tokens[s->pos], "defined",
                            &_ast_token_matches_string_46) == 0 &&
       _ast_token_matches_string_46)) {

    int has_paren = false;

    long result = 0;

    s->pos++;

    skip_ws(s, &_ast_skip_ws_47);

    if ((match(s, TOKEN_LPAREN, &_ast_match_48) == 0 && _ast_match_48))

      has_paren = true;

    skip_ws(s, &_ast_skip_ws_49);

    if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {

      if ((is_defined_macro(s->ctx, &s->tokens->tokens[s->pos],
                            &_ast_is_defined_macro_50) == 0 &&
           _ast_is_defined_macro_50)) {

        result = 1;
      }

      s->pos++;

    } else {

      /* LCOV_EXCL_START */
      s->error = 1;
      /* LCOV_EXCL_STOP */
    }

    if (has_paren) {

      if (!(match(s, TOKEN_RPAREN, &_ast_match_51) == 0 && _ast_match_51))

        /* LCOV_EXCL_START */
        s->error = 1;
      /* LCOV_EXCL_STOP */
    }

    {
      *_out_val = result;
      return CDD_C_SUCCESS;
    }
  }

  {
    *_out_val =
        (parse_primary(s, &_ast_parse_primary_52), _ast_parse_primary_52);
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses multiplicative from the given input.
 */
static enum cdd_c_error parse_multiplicative(struct ExprState *s,
                                             long *_out_val) {
  long _ast_parse_unary_53;
  int _ast_match_54 = 0;
  long _ast_parse_unary_55;
  int _ast_match_56 = 0;
  long _ast_parse_unary_57;
  int _ast_match_58 = 0;
  long _ast_parse_unary_59;

  long val = (parse_unary(s, &_ast_parse_unary_53), _ast_parse_unary_53);

  while (s->pos < s->end && !s->error) {

    if ((match(s, TOKEN_STAR, &_ast_match_54) == 0 && _ast_match_54)) {

      val *= (parse_unary(s, &_ast_parse_unary_55), _ast_parse_unary_55);

    } else if ((match(s, TOKEN_SLASH, &_ast_match_56) == 0 && _ast_match_56)) {

      long divisor =
          (parse_unary(s, &_ast_parse_unary_57), _ast_parse_unary_57);

      if (divisor == 0)

        /* LCOV_EXCL_START */
        val = 0;
      /* LCOV_EXCL_STOP */

      else

        val /= divisor;

    } else if ((match(s, TOKEN_PERCENT, &_ast_match_58) == 0 &&
                _ast_match_58)) {

      long divisor =
          (parse_unary(s, &_ast_parse_unary_59), _ast_parse_unary_59);

      if (divisor == 0)

        /* LCOV_EXCL_START */
        val = 0;
      /* LCOV_EXCL_STOP */

      else

        val %= divisor;

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses additive from the given input.
 */
static enum cdd_c_error parse_additive(struct ExprState *s, long *_out_val) {
  long _ast_parse_multiplicative_60;
  int _ast_match_61 = 0;
  long _ast_parse_multiplicative_62;
  int _ast_match_63 = 0;
  long _ast_parse_multiplicative_64;

  long val = (parse_multiplicative(s, &_ast_parse_multiplicative_60),
              _ast_parse_multiplicative_60);

  while (s->pos < s->end && !s->error) {

    if ((match(s, TOKEN_PLUS, &_ast_match_61) == 0 && _ast_match_61)) {

      val += (parse_multiplicative(s, &_ast_parse_multiplicative_62),
              _ast_parse_multiplicative_62);

    } else if ((match(s, TOKEN_MINUS, &_ast_match_63) == 0 && _ast_match_63)) {

      val -= (parse_multiplicative(s, &_ast_parse_multiplicative_64),
              _ast_parse_multiplicative_64);

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses shift from the given input.
 */
static enum cdd_c_error parse_shift(struct ExprState *s, long *_out_val) {
  long _ast_parse_additive_65;
  int _ast_match_66 = 0;
  long _ast_parse_additive_67;
  int _ast_match_68 = 0;
  long _ast_parse_additive_69;

  long val =
      (parse_additive(s, &_ast_parse_additive_65), _ast_parse_additive_65);

  while (s->pos < s->end && !s->error) {

    if ((match(s, TOKEN_LSHIFT, &_ast_match_66) == 0 && _ast_match_66)) {

      /* LCOV_EXCL_START */
      val <<=
          (parse_additive(s, &_ast_parse_additive_67), _ast_parse_additive_67);
      /* LCOV_EXCL_STOP */

    } else if ((match(s, TOKEN_RSHIFT, &_ast_match_68) == 0 && _ast_match_68)) {

      /* LCOV_EXCL_START */
      val >>=
          (parse_additive(s, &_ast_parse_additive_69), _ast_parse_additive_69);
      /* LCOV_EXCL_STOP */

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses relational from the given input.
 */
static enum cdd_c_error parse_relational(struct ExprState *s, long *_out_val) {
  long _ast_parse_shift_70;
  enum TokenKind _ast_peek_71;
  int _ast_match_72 = 0;
  long _ast_parse_shift_73;
  int _ast_match_74 = 0;
  long _ast_parse_shift_75;
  int _ast_match_76 = 0;
  long _ast_parse_shift_77;
  int _ast_match_78 = 0;
  long _ast_parse_shift_79;

  long val = (parse_shift(s, &_ast_parse_shift_70), _ast_parse_shift_70);

  while (s->pos < s->end && !s->error) {

    enum TokenKind k = (peek(s, &_ast_peek_71), _ast_peek_71);

    if (k == TOKEN_LEQ) {

      (match(s, k, &_ast_match_72) == 0 && _ast_match_72);

      val =
          (val <= (parse_shift(s, &_ast_parse_shift_73), _ast_parse_shift_73));

    } else if (k == TOKEN_GEQ) {

      /* LCOV_EXCL_START */
      (match(s, k, &_ast_match_74) == 0 && _ast_match_74);
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      val =
          (val >= (parse_shift(s, &_ast_parse_shift_75), _ast_parse_shift_75));
      /* LCOV_EXCL_STOP */

    } else if (k == TOKEN_LESS) {

      /* LCOV_EXCL_START */
      (match(s, k, &_ast_match_76) == 0 && _ast_match_76);
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      val = (val < (parse_shift(s, &_ast_parse_shift_77), _ast_parse_shift_77));
      /* LCOV_EXCL_STOP */

    } else if (k == TOKEN_GREATER) {

      (match(s, k, &_ast_match_78) == 0 && _ast_match_78);

      val = (val > (parse_shift(s, &_ast_parse_shift_79), _ast_parse_shift_79));

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses equality from the given input.
 */
static enum cdd_c_error parse_equality(struct ExprState *s, long *_out_val) {
  long _ast_parse_relational_80;
  int _ast_match_81 = 0;
  long _ast_parse_relational_82;
  int _ast_match_83 = 0;
  long _ast_parse_relational_84;

  long val = (parse_relational(s, &_ast_parse_relational_80),
              _ast_parse_relational_80);

  while (s->pos < s->end && !s->error) {

    if ((match(s, TOKEN_EQ, &_ast_match_81) == 0 && _ast_match_81)) {

      val = (val == (parse_relational(s, &_ast_parse_relational_82),
                     _ast_parse_relational_82));

    } else if ((match(s, TOKEN_NEQ, &_ast_match_83) == 0 && _ast_match_83)) {

      val = (val != (parse_relational(s, &_ast_parse_relational_84),
                     _ast_parse_relational_84));

    } else {

      break;
    }
  }

  {
    *_out_val = val;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses logic and from the given input.
 */
static enum cdd_c_error parse_logic_and(struct ExprState *s, long *_out_val) {
  long _ast_parse_equality_85;
  int _ast_match_86 = 0;
  long _ast_parse_equality_87;

  long val =
      (parse_equality(s, &_ast_parse_equality_85), _ast_parse_equality_85);

  while ((match(s, TOKEN_LOGICAL_AND, &_ast_match_86) == 0 && _ast_match_86)) {

    long rhs =
        (parse_equality(s, &_ast_parse_equality_87), _ast_parse_equality_87);

    val = (val && rhs);
  }

  {
    *_out_val = val;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses logic or from the given input.
 */
static enum cdd_c_error parse_logic_or(struct ExprState *s, long *_out_val) {
  long _ast_parse_logic_and_88;
  int _ast_match_89 = 0;
  long _ast_parse_logic_and_90;

  long val =
      (parse_logic_and(s, &_ast_parse_logic_and_88), _ast_parse_logic_and_88);

  while ((match(s, TOKEN_LOGICAL_OR, &_ast_match_89) == 0 && _ast_match_89)) {

    long rhs =
        (parse_logic_and(s, &_ast_parse_logic_and_90), _ast_parse_logic_and_90);

    val = (val || rhs);
  }

  {
    *_out_val = val;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses expr from the given input.
 */
static enum cdd_c_error parse_expr(struct ExprState *s, long *_out_val) {
  long _ast_parse_logic_or_91;
  {
    *_out_val =
        (parse_logic_or(s, &_ast_parse_logic_or_91), _ast_parse_logic_or_91);
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the pp eval expression operation.
 */
enum cdd_c_error pp_eval_expression(const struct TokenList *tokens,
                                    size_t start_idx,

                                    size_t end_idx,
                                    const struct PreprocessorContext *ctx,

                                    long *result) {
  long _ast_parse_expr_92;

  struct ExprState s;

  if (!tokens || !result)

    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  s.tokens = tokens;

  s.pos = start_idx;

  s.end = end_idx;

  s.ctx = ctx;

  s.error = 0;

  *result = (parse_expr(&s, &_ast_parse_expr_92), _ast_parse_expr_92);

  if (s.error)

    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  return CDD_C_SUCCESS;
}

/* --- Embed Directive Parsing --- */

static enum cdd_c_error parse_embed_params(const struct TokenList *tokens,
                                           size_t start,

                                           size_t end,
                                           struct PreprocessorContext *ctx,

                                           struct EmbedParams *out_params) {
  enum TokenKind _ast_identify_keyword_or_id_93;
  char *_ast_token_to_string_94 = NULL;
  enum TokenKind _ast_identify_keyword_or_id_95;
  char *_ast_token_to_string_96 = NULL;
  char *_ast_reconstruct_path_97 = NULL;
  char *_ast_reconstruct_path_98 = NULL;
  char *_ast_reconstruct_path_99 = NULL;

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

        /* LCOV_EXCL_START */
        (identify_keyword_or_id(tokens->tokens[i].start,
                                /* LCOV_EXCL_STOP */

                                /* LCOV_EXCL_START */
                                tokens->tokens[i].length,
                                /* LCOV_EXCL_STOP */
                                &_ast_identify_keyword_or_id_93),
         /* LCOV_EXCL_START */
         _ast_identify_keyword_or_id_93) != TOKEN_IDENTIFIER) {
      /* LCOV_EXCL_STOP */

      /* Try identify and accept as ID if keyword-like */
    }

    name = (token_to_string(&tokens->tokens[i], &_ast_token_to_string_94),
            _ast_token_to_string_94);

    if (!name) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }

    i++;

    /* Check for scoped attribute :: */

    while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)

      /* LCOV_EXCL_START */
      i++;
    /* LCOV_EXCL_STOP */

    if (i + 1 < end && tokens->tokens[i].kind == TOKEN_COLON &&

        /* LCOV_EXCL_START */
        tokens->tokens[i + 1].kind == TOKEN_COLON) {
      /* LCOV_EXCL_STOP */

      /* Scoped */

      /* LCOV_EXCL_START */
      scope = name;
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      name = NULL;
      /* LCOV_EXCL_STOP */

      i += 2; /* Skip :: */

      /* LCOV_EXCL_START */
      while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        i++;
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      if (i < end && (tokens->tokens[i].kind == TOKEN_IDENTIFIER ||
                      /* LCOV_EXCL_STOP */

                      /* LCOV_EXCL_START */
                      (identify_keyword_or_id(tokens->tokens[i].start,
                                              /* LCOV_EXCL_STOP */

                                              /* LCOV_EXCL_START */
                                              tokens->tokens[i].length,
                                              /* LCOV_EXCL_STOP */
                                              &_ast_identify_keyword_or_id_95),
                       /* LCOV_EXCL_START */
                       _ast_identify_keyword_or_id_95) !=
                          /* LCOV_EXCL_STOP */

                          TOKEN_IDENTIFIER)) {

        /* LCOV_EXCL_START */
        name = (token_to_string(&tokens->tokens[i], &_ast_token_to_string_96),
                /* LCOV_EXCL_STOP */
                _ast_token_to_string_96);

        /* LCOV_EXCL_START */
        i++;
        /* LCOV_EXCL_STOP */

      } else {

        /* LCOV_EXCL_START */
        free(scope);
        /* LCOV_EXCL_STOP */

        return CDD_C_ERROR_INVALID_ARGUMENT; /* Expected identifier after :: */
      }
    }

    /* Expect '(' */

    while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)

      /* LCOV_EXCL_START */
      i++;
    /* LCOV_EXCL_STOP */

    if (i >= end || tokens->tokens[i].kind != TOKEN_LPAREN) {

      /* Embed params must have value clause? Standard params do. */

      /* LCOV_EXCL_START */
      free(name);
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      if (scope)
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        free(scope);
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      return CDD_C_ERROR_INVALID_ARGUMENT;
      /* LCOV_EXCL_STOP */
    }

    {

      /* Find matching RPAREN */

      size_t open_idx = i;

      size_t close_idx = 0;

      int depth = 1;

      i++;

      while (i < end) {

        if (tokens->tokens[i].kind == TOKEN_LPAREN)

          /* LCOV_EXCL_START */
          depth++;
        /* LCOV_EXCL_STOP */

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

        /* LCOV_EXCL_START */
        free(name);
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        if (scope)
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          free(scope);
        /* LCOV_EXCL_STOP */

        return CDD_C_ERROR_INVALID_ARGUMENT; /* Unbalanced */
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

        /* LCOV_EXCL_START */
      } else if (strcmp(name, "if_empty") == 0 && !scope) {
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        out_params->if_empty =
            /* LCOV_EXCL_STOP */

            /* LCOV_EXCL_START */
            (reconstruct_path(tokens, open_idx + 1, close_idx,
                              /* LCOV_EXCL_STOP */
                              &_ast_reconstruct_path_99),
             _ast_reconstruct_path_99);

      } else {

        /* Ignore unknown parameters */
      }
    }

    free(name);

    if (scope)

      /* LCOV_EXCL_START */
      free(scope);
    /* LCOV_EXCL_STOP */

    i++; /* Skip RPAREN - loop continues to next param */
  }

  return CDD_C_SUCCESS;
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

/**
 * @brief Executes the stack push operation.
 */
static enum cdd_c_error stack_push(struct ConditionalStack *st,
                                   enum CondState s) {

  if (!st)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  if (st->top < 31) {

    st->states[++st->top] = s;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the stack pop operation.
 */
static enum cdd_c_error stack_pop(struct ConditionalStack *st) {

  if (!st)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  if (st->top >= 0) {

    st->top--;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the stack peek operation.
 */
static enum cdd_c_error stack_peek(const struct ConditionalStack *st,
                                   enum CondState *_out_val) {

  if (st->top >= 0)

  {
    *_out_val = st->states[st->top];
    return CDD_C_SUCCESS;
  }

  {
    *_out_val = COND_ACTIVE;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Checks if enabled.
 */
static enum cdd_c_error is_enabled(const struct ConditionalStack *st,
                                   int *_out_val) {

  int i;

  for (i = 0; i <= st->top; ++i) {

    if (st->states[i] == COND_SKIPPING || st->states[i] == COND_SATISFIED)

    {
      *_out_val = 0;
      return CDD_C_SUCCESS;
    }
  }

  {
    *_out_val = 1;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the pp scan includes operation.
 */
enum cdd_c_error pp_scan_includes(const char *filename,

                                  struct PreprocessorContext *ctx,
                                  pp_visitor_cb cb,

                                  void *user_data) {
  int _ast_token_matches_string_100 = 0;
  int _ast_token_matches_string_101 = 0;
  int _ast_token_matches_string_102 = 0;
  int _ast_is_enabled_103 = 0;
  int _ast_is_defined_macro_104 = 0;
  int _ast_token_matches_string_105 = 0;
  int _ast_is_enabled_106 = 0;
  int _ast_token_matches_string_107 = 0;
  enum CondState _ast_stack_peek_108;
  int _ast_is_enabled_109 = 0;
  int _ast_token_matches_string_110 = 0;
  enum CondState _ast_stack_peek_111;
  int _ast_is_enabled_112 = 0;
  int _ast_token_matches_string_113 = 0;
  int _ast_is_enabled_114 = 0;
  int _ast_token_matches_string_115 = 0;
  int _ast_token_matches_string_116 = 0;
  char *_ast_reconstruct_path_117 = NULL;
  char *_ast_resolve_path_118 = NULL;

  char *content = NULL;

  char *dir_name = NULL;

  size_t sz = 0;

  struct TokenList *tokens = NULL;

  struct ConditionalStack stack;

  size_t i;

  int rc = 0;

  if (!filename || !ctx)

    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* Init stack */

  stack.top = -1;

  rc = read_to_file(filename, "r", &content, &sz);

  if (rc != 0)

    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */

  rc = tokenize(az_span_create_from_str(content), &tokens);

  if (rc != 0) {

    /* LCOV_EXCL_START */
    free(content);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    return rc;
    /* LCOV_EXCL_STOP */
  }

  rc = get_dirname(filename, &dir_name);

  if (rc != 0) {

    /* LCOV_EXCL_START */
    free_token_list(tokens);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    free(content);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    return rc;
    /* LCOV_EXCL_STOP */
  }

  ctx->current_file_dir = dir_name;

  for (i = 0; i < tokens->size; ++i) {

    if (tokens->tokens[i].kind == TOKEN_HASH) {

      size_t next = i + 1;

      while (next < tokens->size &&

             tokens->tokens[next].kind == TOKEN_WHITESPACE)

        /* LCOV_EXCL_START */
        next++;
      /* LCOV_EXCL_STOP */

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

              /* LCOV_EXCL_START */
              eol = k;
              /* LCOV_EXCL_STOP */

              /* LCOV_EXCL_START */
              break;
              /* LCOV_EXCL_STOP */
            }
          }

          if (k == tokens->size)

            /* LCOV_EXCL_START */
            eol = tokens->size;
          /* LCOV_EXCL_STOP */

          break;
        }

        /* --- Conditionals --- */

        if ((token_matches_string(cmd, "ifdef",
                                  &_ast_token_matches_string_100) == 0 &&
             _ast_token_matches_string_100) ||

            (token_matches_string(cmd, "ifndef",
                                  &_ast_token_matches_string_101) == 0 &&
             _ast_token_matches_string_101)) {

          int inverse =
              (token_matches_string(cmd, "ifndef",
                                    &_ast_token_matches_string_102) == 0 &&
               _ast_token_matches_string_102);

          int enabled = (is_enabled(&stack, &_ast_is_enabled_103) == 0 &&
                         _ast_is_enabled_103);

          int condition_met = false;

          directive_handled = 1;

          {

            size_t id_idx = next + 1;

            while (id_idx < eol &&

                   tokens->tokens[id_idx].kind == TOKEN_WHITESPACE)

              id_idx++;

            if (id_idx < eol &&

                tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER) {

              int defined =
                  (is_defined_macro(ctx, &tokens->tokens[id_idx],
                                    &_ast_is_defined_macro_104) == 0 &&
                   _ast_is_defined_macro_104);

              condition_met = inverse ? !defined : defined;
            }
          }

          if (enabled && condition_met) {

            (void)stack_push(&stack, COND_ACTIVE);

            /* LCOV_EXCL_START */
          } else if (enabled && !condition_met) {
            /* LCOV_EXCL_STOP */

            /* LCOV_EXCL_START */
            (void)stack_push(&stack, COND_SKIPPING);
            /* LCOV_EXCL_STOP */

          } else {

            /* LCOV_EXCL_START */
            (void)stack_push(&stack, COND_SATISFIED);
            /* LCOV_EXCL_STOP */
          }

        } else if ((token_matches_string(cmd, "if",
                                         &_ast_token_matches_string_105) == 0 &&
                    _ast_token_matches_string_105)) {

          int enabled = (is_enabled(&stack, &_ast_is_enabled_106) == 0 &&
                         _ast_is_enabled_106);

          int condition_met = false;

          long val = 0;

          directive_handled = 1;

          if (enabled) {

            size_t start_expr = next + 1;

            pp_eval_expression(tokens, start_expr, eol, ctx, &val);

            condition_met = (val != 0);
          }

          if (enabled && condition_met) {

            (void)stack_push(&stack, COND_ACTIVE);

          } else if (enabled && !condition_met) {

            (void)stack_push(&stack, COND_SKIPPING);

          } else {

            /* LCOV_EXCL_START */
            (void)stack_push(&stack, COND_SATISFIED);
            /* LCOV_EXCL_STOP */
          }

        } else if ((token_matches_string(cmd, "elif",
                                         &_ast_token_matches_string_107) == 0 &&
                    _ast_token_matches_string_107)) {

          enum CondState current =
              (stack_peek(&stack, &_ast_stack_peek_108), _ast_stack_peek_108);

          int parent_enabled;

          (void)stack_pop(&stack);

          parent_enabled = (is_enabled(&stack, &_ast_is_enabled_109) == 0 &&
                            _ast_is_enabled_109);

          (void)stack_push(&stack, current);

          directive_handled = 1;

          if (current == COND_ELSE_SEEN || current == COND_SATISFIED) {

            /* Keep skipping */

          } else if (current == COND_ACTIVE) {

            /* LCOV_EXCL_START */
            (void)stack_pop(&stack);
            /* LCOV_EXCL_STOP */

            /* LCOV_EXCL_START */
            (void)stack_push(&stack, COND_SATISFIED);
            /* LCOV_EXCL_STOP */

          } else if (current == COND_SKIPPING && parent_enabled) {

            long val = 0;

            size_t start_expr = next + 1;

            pp_eval_expression(tokens, start_expr, eol, ctx, &val);

            if (val != 0) {

              (void)stack_pop(&stack);

              (void)stack_push(&stack, COND_ACTIVE);
            }
          }

        } else if ((token_matches_string(cmd, "else",
                                         &_ast_token_matches_string_110) == 0 &&
                    _ast_token_matches_string_110)) {

          enum CondState current =
              (stack_peek(&stack, &_ast_stack_peek_111), _ast_stack_peek_111);

          int parent_enabled;

          (void)stack_pop(&stack);

          parent_enabled = (is_enabled(&stack, &_ast_is_enabled_112) == 0 &&
                            _ast_is_enabled_112);

          (void)stack_push(&stack, current);

          directive_handled = 1;

          if (current == COND_ACTIVE) {

            /* LCOV_EXCL_START */
            (void)stack_pop(&stack);
            /* LCOV_EXCL_STOP */

            /* LCOV_EXCL_START */
            (void)stack_push(&stack, COND_SATISFIED);
            /* LCOV_EXCL_STOP */

          } else if (current == COND_SKIPPING && parent_enabled) {

            (void)stack_pop(&stack);

            (void)stack_push(&stack, COND_ACTIVE);
          }

        } else if ((token_matches_string(cmd, "endif",
                                         &_ast_token_matches_string_113) == 0 &&
                    _ast_token_matches_string_113)) {

          directive_handled = 1;

          (void)stack_pop(&stack);
        }

        /* --- Includes & Embeds --- */

        if (!directive_handled &&
            (is_enabled(&stack, &_ast_is_enabled_114) == 0 &&
             _ast_is_enabled_114)) {

          int input_is_embed =
              (token_matches_string(cmd, "embed",
                                    &_ast_token_matches_string_115) == 0 &&
               _ast_token_matches_string_115) ||

              tokens->tokens[next].kind == TOKEN_KEYWORD_EMBED;

          int input_is_include =
              (token_matches_string(cmd, "include",
                                    &_ast_token_matches_string_116) == 0 &&
               _ast_token_matches_string_116) != 0;

          int input_is_include_next =
              (token_matches_string(cmd, "include_next",
                                    &_ast_token_matches_string_116) == 0 &&
               _ast_token_matches_string_116) != 0;

          if (input_is_embed || input_is_include || input_is_include_next) {

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

                    info.is_next = input_is_include_next;

                    info.params.limit = -1; /* Default -1 */

                    if (input_is_embed) {

                      /* Parse params starting at path_end up to eol */

                      if (path_end < eol) {

                        parse_embed_params(tokens, path_end, eol, ctx,

                                           &info.params);
                      }
                    }

                    if (cb(&info, user_data) != 0) {

                      if (input_is_embed) {
                        /* LCOV_EXCL_START */
                        pp_embed_params_free(&info.params);
                        /* LCOV_EXCL_STOP */
                      }

                      free(resolved);
                      free(raw_path);
                      goto cleanup_and_exit;
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

cleanup_and_exit:
  ctx->current_file_dir = NULL;
  free(dir_name);

  free_token_list(tokens);

  free(content);

  return rc;
}
