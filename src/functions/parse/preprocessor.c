/**
 * @file preprocessor.c
 * @brief Implementation of C Preprocessor include resolution and macro
 * indexing.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"
#include "c_cdd/log.h"
#include "c_cdd_stdbool.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/fs.h"
#include "functions/parse/preprocessor.h"
#include "functions/parse/preprocessor_internal.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "c_cdd/safe_crt.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#define PATH_SEP_CHAR '\\'
#else
/** @brief PATH_SEP_CHAR definition */
#define PATH_SEP_CHAR '/'
#endif

/* Standard IO / FS helpers */

cdd_c_error_t pp_join_path(const char *dir, const char *file, char **out_val) {
  char *out;
  size_t len;

  if (!dir || !file || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = NULL;
  len = strlen(dir) + strlen(file) + 2;
  out = (char *)C_CDD_MALLOC(len);
  if (!out)
    return CDD_C_ERROR_MEMORY;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(out, len, "%s%c%s", dir, PATH_SEP_CHAR, file);
#else
  sprintf(out, "%s%c%s", dir, PATH_SEP_CHAR, file);
#endif

  *out_val = out;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_file_exists(const char *path, int *out_exists) {
  FILE *f;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_file_exists_fail && --g_cdd_pp_file_exists_fail == 0)
    return CDD_C_ERROR_IO;
#endif

  if (!path || !out_exists)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_exists = 0;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (fopen_s(&f, path, "r") == 0 && f) {
    fclose(f);
    *out_exists = 1;
    return CDD_C_SUCCESS;
  }
#else
  f = fopen(path, "r");
  if (f) {
    fclose(f);
    *out_exists = 1;
    return CDD_C_SUCCESS;
  }
#endif

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the pp free macro def operation.
 */
void pp_free_macro_def(struct MacroDef *def) {
  size_t i;

  if (!def)
    return;

  if (def->name)
    C_CDD_FREE(def->name);

  if (def->value)
    C_CDD_FREE(def->value);

  if (def->args) {
    for (i = 0; i < def->arg_count; i++) {
      if (def->args[i])
        C_CDD_FREE(def->args[i]);
    }
    C_CDD_FREE(def->args);
  }
}

cdd_c_error_t pp_token_to_string(const struct Token *t, char **out_val) {
  char *s;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_token_to_string_fail && --g_cdd_pp_token_to_string_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif

  if (!t || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = NULL;
  s = (char *)C_CDD_MALLOC(t->length + 1);
  if (!s)
    return CDD_C_ERROR_MEMORY;

  memcpy(s, t->start, t->length);
  s[t->length] = '\0';

  *out_val = s;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the pp add macro internal operation.
 */
cdd_c_error_t pp_add_macro_internal(struct PreprocessorContext *ctx,
                                    const struct MacroDef *def) {
  if (!ctx || !def)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (ctx->macro_count >= ctx->macro_capacity) {
    size_t new_cap = (ctx->macro_capacity == 0) ? 16 : ctx->macro_capacity * 2;
    struct MacroDef *new_arr = (struct MacroDef *)C_CDD_REALLOC(
        ctx->macros, new_cap * sizeof(struct MacroDef));

    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }

    ctx->macros = new_arr;
    ctx->macro_capacity = new_cap;
  }

  ctx->macros[ctx->macro_count++] = *def;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_resolve_path(const struct PreprocessorContext *ctx,
                              const char *current_dir, const char *include_path,
                              int is_system, char **out_val) {
  size_t i;
  char *candidate = NULL;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_resolve_path_fail && --g_cdd_pp_resolve_path_fail == 0)
    return CDD_C_ERROR_IO;
#endif

  if (!include_path || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = NULL;

  if (!is_system && current_dir) {
    rc = pp_join_path(current_dir, include_path, &candidate);
    if (rc != CDD_C_SUCCESS)
      return rc;

    {
      int exists = 0;
      rc = pp_file_exists(candidate, &exists);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(candidate);
        return rc;
      }
      if (exists) {
        *out_val = candidate;
        return CDD_C_SUCCESS;
      }
      C_CDD_FREE(candidate);
      candidate = NULL;
    }
  }

  if (ctx) {
    for (i = 0; i < ctx->size; ++i) {
      rc = pp_join_path(ctx->search_paths[i], include_path, &candidate);
      if (rc != CDD_C_SUCCESS)
        return rc;

      {
        int exists = 0;
        rc = pp_file_exists(candidate, &exists);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(candidate);
          return rc;
        }
        if (exists) {
          *out_val = candidate;
          return CDD_C_SUCCESS;
        }
        C_CDD_FREE(candidate);
        candidate = NULL;
      }
    }
  }

  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_reconstruct_path(const struct TokenList *tokens, size_t start,
                                  size_t end, char **out_val) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  if (!tokens || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = NULL;

  if (start >= end)
    return c_cdd_strdup("", out_val);

  for (i = start; i < end; ++i)
    len += tokens->tokens[i].length;

  buf = (char *)C_CDD_MALLOC(len + 1);
  if (!buf)
    return CDD_C_ERROR_MEMORY;

  p = buf;
  for (i = start; i < end; ++i) {
    const struct Token *t = &tokens->tokens[i];
    memcpy(p, t->start, t->length);
    p += t->length;
  }
  *p = '\0';

  *out_val = buf;
  return CDD_C_SUCCESS;
}

/* --- Public API Implementation --- */

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_pp_context_init_fail = 0;
C_CDD_EXPORT int g_cdd_pp_scan_defines_fail = 0;
#endif

cdd_c_error_t pp_context_init(struct PreprocessorContext *ctx) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_context_init_fail && --g_cdd_pp_context_init_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!ctx)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(ctx, 0, sizeof(*ctx));
  return CDD_C_SUCCESS;
}

void pp_context_free(struct PreprocessorContext *ctx) {
  size_t i;

  if (!ctx)
    return;

  if (ctx->search_paths) {
    for (i = 0; i < ctx->size; ++i) {
      if (ctx->search_paths[i])
        C_CDD_FREE(ctx->search_paths[i]);
    }
    C_CDD_FREE(ctx->search_paths);
  }

  if (ctx->macros) {
    for (i = 0; i < ctx->macro_count; ++i)
      free_macro_def(&ctx->macros[i]);
    C_CDD_FREE(ctx->macros);
  }

  memset(ctx, 0, sizeof(*ctx));
}

cdd_c_error_t pp_add_search_path(struct PreprocessorContext *ctx,
                                 const char *path) {
  char *copy = NULL;
  char **new_paths;
  cdd_c_error_t rc;

  if (!ctx || !path)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = c_cdd_strdup(path, &copy);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  if (ctx->size >= ctx->capacity) {
    size_t new_cap = (ctx->capacity == 0) ? 8 : ctx->capacity * 2;
    new_paths =
        (char **)C_CDD_REALLOC(ctx->search_paths, new_cap * sizeof(char *));
    if (!new_paths) {
      C_CDD_FREE(copy);
      return CDD_C_ERROR_MEMORY;
    }
    ctx->search_paths = new_paths;
    ctx->capacity = new_cap;
  }

  ctx->search_paths[ctx->size++] = copy;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_add_macro(struct PreprocessorContext *ctx, const char *name,
                           const char *value) {
  struct MacroDef def;
  cdd_c_error_t rc;

  if (!ctx || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(&def, 0, sizeof(def));

  rc = c_cdd_strdup(name, &def.name);
  if (rc != CDD_C_SUCCESS)
    return CDD_C_ERROR_MEMORY;

  if (value) {
    rc = c_cdd_strdup(value, &def.value);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(def.name);
      return CDD_C_ERROR_MEMORY;
    }
  }

  def.is_function_like = 0;
  rc = add_macro_internal(ctx, &def);
  if (rc != CDD_C_SUCCESS) {
    free_macro_def(&def);
    return rc;
  }

  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_scan_defines(struct PreprocessorContext *ctx,
                              const char *filename) {
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  cdd_c_error_t rc;
  size_t i;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_scan_defines_fail && --g_cdd_pp_scan_defines_fail == 0)
    return CDD_C_ERROR_IO;
#endif

  if (!ctx || !filename)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = read_to_file(filename, "r", &content, &sz);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = tokenize(az_span_create_from_str(content), &tokens);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(content);
    return rc;
  }

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_HASH) {
      size_t next = i + 1;
      int is_define = 0;

      while (next < tokens->size &&
             tokens->tokens[next].kind == TOKEN_WHITESPACE)
        next++;

      if (next < tokens->size) {
        rc = token_matches_string(&tokens->tokens[next], "define", &is_define);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_define) {
          size_t name_idx = next + 1;

          while (name_idx < tokens->size &&
                 tokens->tokens[name_idx].kind == TOKEN_WHITESPACE)
            name_idx++;

          if (name_idx < tokens->size) {
            if (tokens->tokens[name_idx].kind == TOKEN_IDENTIFIER) {
              struct MacroDef def;
              size_t val_start_idx = name_idx + 1;

              memset(&def, 0, sizeof(def));
              rc = pp_token_to_string(&tokens->tokens[name_idx], &def.name);
              if (rc != CDD_C_SUCCESS) {
                free_token_list(tokens);
                C_CDD_FREE(content);
                return rc;
              }

              if (name_idx + 1 < tokens->size) {
                if (tokens->tokens[name_idx + 1].kind == TOKEN_LPAREN) {
                  size_t curr = name_idx + 2;
                  int done_args = 0;

                  def.is_function_like = 1;
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
                      char *arg_name = NULL;
                      char **new_args = NULL;

                      rc = pp_token_to_string(&tokens->tokens[curr], &arg_name);
                      if (rc != CDD_C_SUCCESS) {
                        free_macro_def(&def);
                        free_token_list(tokens);
                        C_CDD_FREE(content);
                        return rc;
                      }

                      new_args = (char **)C_CDD_REALLOC(
                          def.args, (def.arg_count + 1) * sizeof(char *));
                      if (new_args) {
                        def.args = new_args;
                        def.args[def.arg_count++] = arg_name;
                      } else {
                        C_CDD_FREE(arg_name);
                        pp_free_macro_def(&def);
                        free_token_list(tokens);
                        C_CDD_FREE(content);
                        return CDD_C_ERROR_MEMORY;
                      }
                      curr++;

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
                      curr++;
                    }
                  }
                  val_start_idx = curr;
                } else {
                  def.is_function_like = 0;
                }
              }

              {
                size_t val_end_idx = val_start_idx;
                while (val_end_idx < tokens->size) {
                  if (tokens->tokens[val_end_idx].kind == TOKEN_WHITESPACE) {
                    size_t idx;
                    int has_nl = 0;
                    for (idx = 0; idx < tokens->tokens[val_end_idx].length;
                         idx++) {
                      if (tokens->tokens[val_end_idx].start[idx] == '\n') {
                        has_nl = 1;
                        break;
                      }
                    }
                    if (has_nl)
                      break;
                  }
                  val_end_idx++;
                }

                while (val_start_idx < val_end_idx &&
                       tokens->tokens[val_start_idx].kind == TOKEN_WHITESPACE)
                  val_start_idx++;

                if (val_end_idx > val_start_idx) {
                  size_t val_start_byte =
                      (size_t)(tokens->tokens[val_start_idx].start -
                               (const uint8_t *)content);
                  size_t val_end_byte =
                      (size_t)(tokens->tokens[val_end_idx - 1].start -
                               (const uint8_t *)content) +
                      tokens->tokens[val_end_idx - 1].length;
                  size_t val_len = val_end_byte - val_start_byte;
                  char *v = (char *)C_CDD_MALLOC(val_len + 1);
                  if (v) {
                    memcpy(v, content + val_start_byte, val_len);
                    v[val_len] = '\0';
                    def.value = v;
                  }
                }
              }

              rc = add_macro_internal(ctx, &def);
              if (rc != CDD_C_SUCCESS) {
                free_macro_def(&def);
                free_token_list(tokens);
                C_CDD_FREE(content);
                return rc;
              }
              i = name_idx;
            }
          }
        }
      }
    }
  }

  free_token_list(tokens);
  C_CDD_FREE(content);
  return CDD_C_SUCCESS;
}

void pp_embed_params_free(struct EmbedParams *params) {
  if (!params)
    return;

  if (params->prefix)
    C_CDD_FREE(params->prefix);

  if (params->suffix)
    C_CDD_FREE(params->suffix);

  if (params->if_empty)
    C_CDD_FREE(params->if_empty);

  params->prefix = NULL;
  params->suffix = NULL;
  params->if_empty = NULL;
}

cdd_c_error_t pp_scan_includes(const char *filename,
                               struct PreprocessorContext *ctx,
                               pp_visitor_cb cb, void *user_data) {
  char *content = NULL;
  char *dir_name = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  struct ConditionalStack stack;
  size_t i;
  cdd_c_error_t rc;

  if (!filename || !ctx)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  stack.top = -1;

  rc = read_to_file(filename, "r", &content, &sz);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = tokenize(az_span_create_from_str(content), &tokens);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(content);
    return rc;
  }

  rc = get_dirname(filename, &dir_name);
  if (rc != CDD_C_SUCCESS) {
    free_token_list(tokens);
    C_CDD_FREE(content);
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
        {
          int is_ifdef = 0;
          int is_ifndef = 0;
          rc = token_matches_string(cmd, "ifdef", &is_ifdef);
          rc = token_matches_string(cmd, "ifndef", &is_ifndef);

          if (is_ifdef || is_ifndef) {
            int inverse = is_ifndef;
            int enabled = 0;
            int condition_met = 0;

            rc = pp_is_enabled(&stack, &enabled);
            directive_handled = 1;

            {
              size_t id_idx = next + 1;
              while (id_idx < eol &&
                     tokens->tokens[id_idx].kind == TOKEN_WHITESPACE)
                id_idx++;

              if (id_idx < eol) {
                if (tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER) {
                  int defined = 0;
                  rc = pp_is_defined_macro(ctx, &tokens->tokens[id_idx],
                                           &defined);
                  if (rc != CDD_C_SUCCESS)
                    goto cleanup_and_exit;
                  condition_met = inverse ? !defined : defined;
                }
              }
            }

            if (enabled)
              rc = pp_stack_push(&stack,
                                 condition_met ? COND_ACTIVE : COND_SKIPPING);
            else
              rc = pp_stack_push(&stack, COND_SATISFIED);
          }
        }

        if (!directive_handled) {
          int is_if = 0;
          rc = token_matches_string(cmd, "if", &is_if);
          if (is_if) {
            int enabled = 0;
            int condition_met = 0;
            long val = 0;

            rc = pp_is_enabled(&stack, &enabled);
            directive_handled = 1;

            if (enabled) {
              size_t start_expr = next + 1;
              rc = pp_eval_expression(tokens, start_expr, eol, ctx, &val);
              if (rc != CDD_C_SUCCESS)
                goto cleanup_and_exit;
              condition_met = (val != 0);
              rc = pp_stack_push(&stack,
                                 condition_met ? COND_ACTIVE : COND_SKIPPING);
            } else {
              rc = pp_stack_push(&stack, COND_SATISFIED);
            }
          }
        }

        if (!directive_handled) {
          int is_elif = 0;
          rc = token_matches_string(cmd, "elif", &is_elif);
          if (is_elif) {
            enum CondState current = COND_ACTIVE;
            int parent_enabled = 0;

            rc = pp_stack_peek(&stack, &current);
            rc = pp_stack_pop(&stack);
            rc = pp_is_enabled(&stack, &parent_enabled);
            rc = pp_stack_push(&stack, current);
            directive_handled = 1;

            if (current == COND_SATISFIED) {
              /* Keep skipping */
            } else if (current == COND_ACTIVE) {
              rc = pp_stack_pop(&stack);
              rc = pp_stack_push(&stack, COND_SATISFIED);
            } else {
              long val = 0;
              size_t start_expr = next + 1;
              rc = pp_eval_expression(tokens, start_expr, eol, ctx, &val);
              if (rc != CDD_C_SUCCESS)
                goto cleanup_and_exit;
              if (val != 0) {
                rc = pp_stack_pop(&stack);
                rc = pp_stack_push(&stack, COND_ACTIVE);
              }
            }
          }
        }

        if (!directive_handled) {
          int is_else = 0;
          rc = token_matches_string(cmd, "else", &is_else);
          if (is_else) {
            enum CondState current = COND_ACTIVE;
            int parent_enabled = 0;

            rc = pp_stack_peek(&stack, &current);
            rc = pp_stack_pop(&stack);
            rc = pp_is_enabled(&stack, &parent_enabled);
            rc = pp_stack_push(&stack, current);
            directive_handled = 1;

            if (current == COND_ACTIVE) {
              rc = pp_stack_pop(&stack);
              rc = pp_stack_push(&stack, COND_SATISFIED);
            } else if (current == COND_SKIPPING) {
              rc = pp_stack_pop(&stack);
              rc = pp_stack_push(&stack, COND_ACTIVE);
            }
          }
        }

        if (!directive_handled) {
          int is_endif = 0;
          rc = token_matches_string(cmd, "endif", &is_endif);
          if (is_endif) {
            directive_handled = 1;
            rc = pp_stack_pop(&stack);
          }
        }

        /* --- Includes & Embeds --- */
        if (!directive_handled) {
          int enabled = 0;
          rc = pp_is_enabled(&stack, &enabled);
          if (enabled) {
            int is_embed = 0;
            int is_include = 0;
            int is_include_next = 0;

            is_embed = (tokens->tokens[next].kind == TOKEN_KEYWORD_EMBED);
            if (!is_embed)
              rc = token_matches_string(cmd, "embed", &is_embed);

            rc = token_matches_string(cmd, "include", &is_include);
            rc = token_matches_string(cmd, "include_next", &is_include_next);

            if (is_embed || is_include || is_include_next) {
              size_t path_start = next + 1;
              size_t path_end = path_start;
              char *raw_path = NULL;
              char *resolved = NULL;
              int is_sys = 0;

              while (path_start < eol &&
                     tokens->tokens[path_start].kind == TOKEN_WHITESPACE)
                path_start++;

              if (path_start < eol) {
                if (tokens->tokens[path_start].kind == TOKEN_STRING_LITERAL) {
                  const struct Token *t = &tokens->tokens[path_start];
                  raw_path = (char *)C_CDD_MALLOC(t->length - 1);
                  if (!raw_path)
                    goto cleanup_and_exit;
                  memcpy(raw_path, t->start + 1, t->length - 2);
                  raw_path[t->length - 2] = '\0';
                  path_end = path_start + 1;
                } else if (tokens->tokens[path_start].kind == TOKEN_LESS) {
                  size_t end_arg = path_start + 1;
                  while (end_arg < eol) {
                    if (tokens->tokens[end_arg].kind == TOKEN_GREATER)
                      break;
                    end_arg++;
                  }
                  if (end_arg < eol) {
                    rc = pp_reconstruct_path(tokens, path_start + 1, end_arg,
                                             &raw_path);
                    is_sys = 1;
                    path_end = end_arg + 1;
                  }
                }

                if (raw_path) {
                  rc = pp_resolve_path(ctx, dir_name, raw_path, is_sys,
                                       &resolved);
                  if (rc != CDD_C_SUCCESS) {
                    C_CDD_FREE(raw_path);
                    goto cleanup_and_exit;
                  }
                  if (resolved) {
                    if (cb) {
                      struct IncludeInfo info;
                      cdd_c_error_t cb_rc;
                      memset(&info, 0, sizeof(info));
                      info.kind = is_embed ? PP_DIR_EMBED : PP_DIR_INCLUDE;
                      info.resolved_path = resolved;
                      info.raw_path = raw_path;
                      info.is_system = is_sys;
                      info.is_next = is_include_next;
                      info.params.limit = -1;

                      if (is_embed) {
                        if (path_end < eol) {
                          rc = pp_parse_embed_params(tokens, path_end, eol, ctx,
                                                     &info.params);
                          if (rc != CDD_C_SUCCESS) {
                            pp_embed_params_free(&info.params);
                            C_CDD_FREE(resolved);
                            C_CDD_FREE(raw_path);
                            goto cleanup_and_exit;
                          }
                        }
                      }

                      cb_rc = cb(&info, user_data);
                      if (cb_rc != CDD_C_SUCCESS) {
                        if (is_embed)
                          pp_embed_params_free(&info.params);
                        C_CDD_FREE(resolved);
                        C_CDD_FREE(raw_path);
                        goto cleanup_and_exit;
                      }

                      if (is_embed)
                        pp_embed_params_free(&info.params);
                    }
                    C_CDD_FREE(resolved);
                  }
                  C_CDD_FREE(raw_path);
                }
              }
            }
          }
        }

        i = eol - 1;
      }
    }
  }

cleanup_and_exit:
  ctx->current_file_dir = NULL;
  C_CDD_FREE(dir_name);
  free_token_list(tokens);
  C_CDD_FREE(content);
  return rc;
}
