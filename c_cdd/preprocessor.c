/**
 * @file preprocessor.c
 * @brief Implementation of preprocessor logic including expression evaluation
 * and conditional scanning.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "fs.h"
#include "preprocessor.h"
#include "str_utils.h"
#include "tokenizer.h"

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

static long parse_primary(struct ExprState *s) {
  skip_ws(s);
  if (s->pos >= s->end) {
    s->error = 1;
    return 0;
  }

  /* Parenthesized expression */
  if (match(s, TOKEN_LPAREN)) {
    long val = parse_expr(s);
    if (!match(s, TOKEN_RPAREN))
      s->error = 1;
    return val;
  }

  /* Number */
  if (s->tokens->tokens[s->pos].kind == TOKEN_NUMBER_LITERAL) {
    char *txt = token_to_string(&s->tokens->tokens[s->pos]);
    long val = 0;
    if (txt) {
      val = strtol(txt, NULL, 0); /* Handles hex/oct/dec */
      free(txt);
    }
    s->pos++;
    return val;
  }

  /* Identifier -> 0 unless it's a known object-like macro with numeric value */
  if (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER ||
      s->tokens->tokens[s->pos].kind >= TOKEN_KEYWORD_AUTO) { /* All Keywords */
    /* Check if it is a macro that expands to a number.
       Strict expansion is complex.
       Simplified logic: If macro found and has value, try parse value.
       Otherwise 0. */
    long val = 0;
    const struct Token *tok = &s->tokens->tokens[s->pos];
    if (s->ctx) {
      size_t i;
      for (i = 0; i < s->ctx->macro_count; ++i) {
        if (!s->ctx->macros[i].is_function_like && s->ctx->macros[i].value &&
            token_matches_string(tok, s->ctx->macros[i].name)) {
          /* Simple recursion check (depth 1) */
          char *endptr;
          val = strtol(s->ctx->macros[i].value, &endptr, 0);
          /* If definition is not a number, it expands to 0 or another ID -> 0
           */
          break;
        }
      }
    }
    s->pos++;
    return val;
  }

  /* defined() operator handled in Unary, so getting here usually means 0 or
   * error */
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

  /* Check 'defined' */
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
        val = 0; /* Avoid DBZ, spec behavior undefined but 0 safe */
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

/* --- Include Scanning & Conditional Logic --- */

static char *resolve_path(const struct PreprocessorContext *ctx,
                          const char *current_dir, const char *include_path,
                          int is_system) {
  size_t i;
  char *candidate;

  if (!is_system) {
    candidate = join_path(current_dir, include_path);
    if (candidate) {
      if (file_exists(candidate)) {
        return candidate;
      }
      free(candidate);
    }
  }

  for (i = 0; i < ctx->size; ++i) {
    candidate = join_path(ctx->search_paths[i], include_path);
    if (candidate) {
      if (file_exists(candidate)) {
        return candidate;
      }
      free(candidate);
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

enum CondState {
  COND_ACTIVE,    /* Currently processing, parent also active */
  COND_SKIPPING,  /* Ignoring tokens (condition false or taken elsewhere) */
  COND_SATISFIED, /* Current block skipping, but a branch WAS taken */
  COND_ELSE_SEEN  /* Flag that #else was seen */
};

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
  return COND_ACTIVE; /* Default if stack empty */
}

/* Check if we are currently enabled (all nesting levels ACTIVE) */
static bool is_enabled(const struct ConditionalStack *st) {
  int i;
  for (i = 0; i <= st->top; ++i) {
    if (st->states[i] == COND_SKIPPING || st->states[i] == COND_SATISFIED)
      return false;
  }
  return true;
}

int pp_scan_includes(const char *const filename,
                     const struct PreprocessorContext *const ctx,
                     pp_visitor_cb cb, void *user_data) {
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
  /* Push base state */
  /* stack_push(&stack, COND_ACTIVE); Not strictly needed if loop uses helper */

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

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_HASH) {
      size_t next = i + 1;
      while (next < tokens->size &&
             tokens->tokens[next].kind == TOKEN_WHITESPACE)
        next++;

      if (next < tokens->size) {
        const struct Token *cmd = &tokens->tokens[next];
        int directive_handled = 0;

        /* Find end of line for arguments */
        size_t eol = next + 1;
        while (eol < tokens->size) {
          /* Heuristic: simplistic EOL finding. */
          size_t k = eol;
          for (; k < tokens->size; ++k) {
            if (tokens->tokens[k].kind == TOKEN_WHITESPACE) {
              /* Check for newline */
              const uint8_t *s = tokens->tokens[k].start;
              size_t l = tokens->tokens[k].length;
              if (memchr(s, '\n', l)) {
                eol = k;
                break;
              }
            }
            if (tokens->tokens[k].kind == TOKEN_HASH) {
              eol = k; /* Start of next directive */
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

          /* Get identifier */
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
            /* Parent disabled -> disabled + satisfied (to prevent else) */
            stack_push(&stack, COND_SATISFIED);
          }

        } else if (token_matches_string(cmd, "if")) {
          bool enabled = is_enabled(&stack);
          bool condition_met = false;
          long val = 0;

          directive_handled = 1;

          /* Evaluate expression */
          if (enabled) {
            /* Find expression start */
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
          /* Temporarily pop to check parent state */
          stack_pop(&stack);
          bool parent_enabled = is_enabled(&stack);
          stack_push(&stack, current);

          directive_handled = 1;

          if (current == COND_ELSE_SEEN) {
            /* Error: elif after else */
          } else if (current == COND_SATISFIED) {
            /* Already taken a branch, keep skipping */
            /* State remains COND_SATISFIED (equiv to skipping but blocks else)
             */
          } else if (current == COND_ACTIVE) {
            /* Was active, so this branch is done. Transition to SATISFIED
             * (skip) */
            stack_pop(&stack);
            stack_push(&stack, COND_SATISFIED);
          } else if (current == COND_SKIPPING && parent_enabled) {
            /* Was skipping, check if we should take this one */
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
          stack_pop(&stack);
          bool parent_enabled = is_enabled(&stack);
          stack_push(&stack, current);

          directive_handled = 1;

          if (current == COND_ACTIVE) {
            /* If we were active, else block is skipped */
            stack_pop(&stack);
            stack_push(&stack, COND_SATISFIED);
          } else if (current == COND_SKIPPING && parent_enabled) {
            /* If we were skipping (and parent enabled), take else */
            stack_pop(&stack);
            stack_push(&stack, COND_ACTIVE);
          } else {
            /* Else remains skipped/satisfied */
          }
          /* Mark that we hit else, preventing further elifs (simplistic model)
           */
          /* In this simple stack model, switching to Active/Satisfied is
           * enough. */

        } else if (token_matches_string(cmd, "endif")) {
          directive_handled = 1;
          stack_pop(&stack);
        }

        /* --- Includes --- */

        if (!directive_handled && is_enabled(&stack)) {
          if (token_matches_string(cmd, "include") ||
              token_matches_string(cmd, "embed") ||
              tokens->tokens[next].kind == TOKEN_KEYWORD_EMBED) {

            size_t path_start = next + 1;
            /* Skip WS */
            while (path_start < eol &&
                   tokens->tokens[path_start].kind == TOKEN_WHITESPACE)
              path_start++;

            if (path_start < eol) {
              char *raw_path = NULL;
              char *resolved = NULL;
              int is_sys = 0;

              if (tokens->tokens[path_start].kind == TOKEN_STRING_LITERAL) {
                const struct Token *t = &tokens->tokens[path_start];
                if (t->length >= 2) {
                  raw_path = (char *)malloc(t->length - 1);
                  if (raw_path) {
                    memcpy(raw_path, t->start + 1, t->length - 2);
                    raw_path[t->length - 2] = '\0';
                  }
                }
                is_sys = 0;
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
                }
              }

              if (raw_path) {
                resolved = resolve_path(ctx, dir_name, raw_path, is_sys);
                if (resolved) {
                  if (cb) {
                    if (cb(resolved, user_data) != 0) {
                      free(resolved);
                      free(raw_path);
                      /* Break main loop logic? For now breaks loop over tokens
                       */
                      i = tokens->size;
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

  free(dir_name);
  free_token_list(tokens);
  free(content);

  return rc;
}

int pp_context_init(struct PreprocessorContext *const ctx) {
  if (!ctx)
    return EINVAL;
  ctx->search_paths = NULL;
  ctx->size = 0;
  ctx->capacity = 0;
  ctx->macros = NULL;
  ctx->macro_count = 0;
  ctx->macro_capacity = 0;
  return 0;
}

void pp_context_free(struct PreprocessorContext *const ctx) {
  size_t i;
  if (!ctx)
    return;
  if (ctx->search_paths) {
    for (i = 0; i < ctx->size; ++i) {
      free(ctx->search_paths[i]);
    }
    free(ctx->search_paths);
    ctx->search_paths = NULL;
  }
  if (ctx->macros) {
    for (i = 0; i < ctx->macro_count; ++i) {
      free_macro_def(&ctx->macros[i]);
    }
    free(ctx->macros);
    ctx->macros = NULL;
  }
  ctx->size = 0;
  ctx->capacity = 0;
  ctx->macro_count = 0;
  ctx->macro_capacity = 0;
}

int pp_add_search_path(struct PreprocessorContext *const ctx,
                       const char *const path) {
  if (!ctx || !path)
    return EINVAL;

  if (ctx->size >= ctx->capacity) {
    size_t new_cap = (ctx->capacity == 0) ? 4 : ctx->capacity * 2;
    char **new_arr =
        (char **)realloc(ctx->search_paths, new_cap * sizeof(char *));
    if (!new_arr)
      return ENOMEM;
    ctx->search_paths = new_arr;
    ctx->capacity = new_cap;
  }

  ctx->search_paths[ctx->size] = c_cdd_strdup(path);
  if (!ctx->search_paths[ctx->size])
    return ENOMEM;

  ctx->size++;
  return 0;
}

int pp_add_macro(struct PreprocessorContext *const ctx, const char *const name,
                 const char *const value) {
  struct MacroDef def;
  if (!ctx || !name)
    return EINVAL;

  memset(&def, 0, sizeof(def));
  def.name = c_cdd_strdup(name);
  if (value)
    def.value = c_cdd_strdup(value);

  if (!def.name || (value && !def.value)) {
    free_macro_def(&def);
    return ENOMEM;
  }

  return add_macro_internal(ctx, &def);
}

static int add_macro_arg(struct MacroDef *def, const char *arg_name) {
  char **new_args = realloc(def->args, (def->arg_count + 1) * sizeof(char *));
  if (!new_args)
    return ENOMEM;
  def->args = new_args;
  def->args[def->arg_count] = c_cdd_strdup(arg_name);
  if (!def->args[def->arg_count])
    return ENOMEM;
  def->arg_count++;
  return 0;
}

int pp_scan_defines(struct PreprocessorContext *const ctx,
                    const char *const filename) {
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  size_t i;
  int rc = 0;

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
      size_t k = i + 1;
      /* Skip whitespace */
      while (k < tokens->size && tokens->tokens[k].kind == TOKEN_WHITESPACE)
        k++;

      if (k < tokens->size && tokens->tokens[k].kind == TOKEN_IDENTIFIER &&
          token_matches_string(&tokens->tokens[k], "define")) {

        k++;
        while (k < tokens->size && tokens->tokens[k].kind == TOKEN_WHITESPACE)
          k++;

        if (k < tokens->size && tokens->tokens[k].kind == TOKEN_IDENTIFIER) {
          struct MacroDef def;
          struct Token *id_tok = &tokens->tokens[k];

          memset(&def, 0, sizeof(def));
          def.name = token_to_string(id_tok);

          /* Check function-like (LPAREN immediately after name) */
          if (k + 1 < tokens->size) {
            const uint8_t *end_of_name = id_tok->start + id_tok->length;
            const struct Token *next_tok = &tokens->tokens[k + 1];

            if (next_tok->kind == TOKEN_LPAREN &&
                next_tok->start == end_of_name) {
              def.is_function_like = 1;

              {
                size_t curr = k + 2;
                while (curr < tokens->size) {
                  /* Skip WS */
                  while (curr < tokens->size &&
                         tokens->tokens[curr].kind == TOKEN_WHITESPACE)
                    curr++;

                  if (curr >= tokens->size)
                    break;

                  if (tokens->tokens[curr].kind == TOKEN_RPAREN) {
                    curr++;
                    k = curr - 1; /* Update k to point to RPAREN so value
                                     scanning picks up afterwards */
                    break;        /* End of args */
                  }

                  if (tokens->tokens[curr].kind == TOKEN_ELLIPSIS) {
                    def.is_variadic = 1;
                    curr++;
                    /* Expect RPAREN next (ignoring WS) */
                    while (curr < tokens->size &&
                           tokens->tokens[curr].kind == TOKEN_WHITESPACE)
                      curr++;
                    if (curr < tokens->size &&
                        tokens->tokens[curr].kind == TOKEN_RPAREN) {
                      curr++;
                      k = curr - 1;
                      break;
                    }
                  } else if (tokens->tokens[curr].kind == TOKEN_IDENTIFIER) {
                    /* Check if next token is ellipsis (GCC style args...) */
                    size_t next_t = curr + 1;
                    while (next_t < tokens->size &&
                           tokens->tokens[next_t].kind == TOKEN_WHITESPACE)
                      next_t++;

                    if (next_t < tokens->size &&
                        tokens->tokens[next_t].kind == TOKEN_ELLIPSIS) {
                      /* Named variadic */
                      char *arg_name = token_to_string(&tokens->tokens[curr]);
                      add_macro_arg(&def, arg_name);
                      free(arg_name);
                      def.is_variadic = 1;

                      curr = next_t + 1; /* Skip ID and ... */
                    } else {
                      /* Normal arg */
                      char *arg_name = token_to_string(&tokens->tokens[curr]);
                      add_macro_arg(&def, arg_name);
                      free(arg_name);
                      curr++;
                    }
                  } else {
                    /* Malformed? consume until comma or rparen */
                    curr++;
                  }

                  /* Consume comma if present */
                  while (curr < tokens->size &&
                         tokens->tokens[curr].kind == TOKEN_WHITESPACE)
                    curr++;
                  if (curr < tokens->size &&
                      tokens->tokens[curr].kind == TOKEN_COMMA) {
                    curr++;
                  }
                }

                /* After arg loop, k needs to be right before value start. */
                /* Arg loop finished at RPAREN. curr is after RPAREN. */
                k = curr - 1;
              }
            } else {
              /* Object-like: no parens or space before parens */
            }
          }

          /* Extract Value */
          {
            size_t start_val = k + 1;
            /* Skip WS to finds value start */
            while (start_val < tokens->size &&
                   tokens->tokens[start_val].kind == TOKEN_WHITESPACE) {
              if (memchr(tokens->tokens[start_val].start, '\n',
                         tokens->tokens[start_val].length))
                break;
              start_val++;
            }

            /* Capture value until newline */
            size_t end_val = start_val;
            while (end_val < tokens->size) {
              if (memchr(tokens->tokens[end_val].start, '\n',
                         tokens->tokens[end_val].length))
                break;
              if (tokens->tokens[end_val].kind == TOKEN_HASH)
                break;
              end_val++;
            }

            if (end_val > start_val) {
              char *val_str = reconstruct_path(tokens, start_val, end_val);
              def.value = val_str;
            }
          }

          if (add_macro_internal(ctx, &def) != 0) {
            rc = ENOMEM;
            free_macro_def(&def);
            goto cleanup;
          }
        }
      }
    }
  }

cleanup:
  free_token_list(tokens);
  free(content);
  return rc;
}
