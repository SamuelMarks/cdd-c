/**
 * @file rewriter_body.c
 * @brief Implementation of body rewriting and injection logic.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c89stringutils_string_extras.h>

#include "rewriter_body.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <sys/errno.h>
#endif

/* --- Internal Structures --- */

/**
 * @brief Represents a text replacement at a specific token index.
 */
struct Replacement {
  size_t token_start; /**< Start index of tokens to replace (inclusive) */
  size_t token_end;   /**< End index of tokens to replace (exclusive) */
  char *text;         /**< Null-terminated replacement text */
};

/**
 * @brief List of pending replacements.
 */
struct PatchList {
  struct Replacement *items;
  size_t size;
  size_t capacity;
};

/* --- Helpers --- */

static int patch_list_init(struct PatchList *pl) {
  pl->size = 0;
  pl->capacity = 8;
  pl->items =
      (struct Replacement *)malloc(sizeof(struct Replacement) * pl->capacity);
  if (!pl->items)
    return ENOMEM;
  return 0;
}

static void patch_list_free(struct PatchList *pl) {
  size_t i;
  if (!pl)
    return;
  if (pl->items) {
    for (i = 0; i < pl->size; ++i) {
      free(pl->items[i].text);
    }
    free(pl->items);
  }
}

/**
 * @brief Add a replacement request.
 * Takes ownership of `text`.
 */
static int add_replacement(struct PatchList *pl, size_t start, size_t end,
                           char *text) {
  if (pl->size >= pl->capacity) {
    size_t new_cap = pl->capacity * 2;
    struct Replacement *new_items = (struct Replacement *)realloc(
        pl->items, new_cap * sizeof(struct Replacement));
    if (!new_items) {
      free(text);
      return ENOMEM;
    }
    pl->items = new_items;
    pl->capacity = new_cap;
  }
  pl->items[pl->size].token_start = start;
  pl->items[pl->size].token_end = end;
  pl->items[pl->size].text = text;
  pl->size++;
  return 0;
}

/**
 * @brief Compare function for qsort. Sorts by start index ascending.
 */
static int cmp_replacements(const void *a, const void *b) {
  const struct Replacement *ra = (const struct Replacement *)a;
  const struct Replacement *rb = (const struct Replacement *)b;
  if (ra->token_start < rb->token_start)
    return -1;
  if (ra->token_start > rb->token_start)
    return 1;
  return 0;
}

/**
 * @brief Helper to find token matching string.
 */
static int token_eq(const struct Token *tok, const char *s) {
  size_t len = strlen(s);
  return (tok->length == len && strncmp((const char *)tok->start, s, len) == 0);
}

/**
 * @brief Find index of next occurrence of a specific token kind.
 */
static size_t find_next_token(const struct TokenList *tokens, size_t start,
                              enum TokenKind kind) {
  size_t i;
  for (i = start; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == kind)
      return i;
  }
  return tokens->size;
}

/**
 * @brief Convert a range of tokens back to a string (joins them).
 */
static char *tokens_to_string(const struct TokenList *tokens, size_t start,
                              size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  for (i = start; i < end; ++i)
    len += tokens->tokens[i].length;

  buf = (char *)malloc(len + 1);
  if (!buf)
    return NULL;

  p = buf;
  for (i = start; i < end; ++i) {
    memcpy(p, tokens->tokens[i].start, tokens->tokens[i].length);
    p += tokens->tokens[i].length;
  }
  *p = '\0';
  return buf;
}

/* --- Logic --- */

/**
 * @brief Generate check injections for unchecked allocations.
 */
static int process_allocations(const struct TokenList *tokens,
                               const struct AllocationSiteList *allocs,
                               struct PatchList *patches) {
  size_t i;
  for (i = 0; i < allocs->size; ++i) {
    size_t semi_idx;
    char *injection = NULL;

    if (allocs->sites[i].is_checked)
      continue;

    /* Find the semicolon ending the malloc statement */
    /* allocs->sites[i].token_index points to 'malloc' */
    semi_idx =
        find_next_token(tokens, allocs->sites[i].token_index, TOKEN_SEMICOLON);

    if (semi_idx >= tokens->size)
      continue; /* Should not happen in valid C */

    /* After semicolon, inject check */
    if (asprintf(&injection, " if (!%s) { return ENOMEM; }",
                 allocs->sites[i].var_name) == -1) {
      return ENOMEM;
    }

    /* Insert range [semi_idx + 1, semi_idx + 1] means insert without delete */
    if (add_replacement(patches, semi_idx + 1, semi_idx + 1, injection) != 0) {
      return ENOMEM;
    }
  }
  return 0;
}

/**
 * @brief Identify and rewrite calls to refactored functions.
 */
static int process_function_calls(const struct TokenList *tokens,
                                  const struct RefactoredFunction *funcs,
                                  size_t func_count,
                                  struct PatchList *patches) {
  size_t i;

  for (i = 0; i < tokens->size; ++i) {
    size_t f_idx;
    const struct RefactoredFunction *target = NULL;

    if (tokens->tokens[i].kind != TOKEN_IDENTIFIER)
      continue;

    /* Check if this identifier matches a known refactored function */
    for (f_idx = 0; f_idx < func_count; ++f_idx) {
      if (token_eq(&tokens->tokens[i], funcs[f_idx].name)) {
        target = &funcs[f_idx];
        break;
      }
    }

    if (!target)
      continue;

    /* Check if it looks like a call: Identifier followed by LPAREN */
    /* Only whitespace allowed between */
    {
      size_t next = i + 1;
      while (next < tokens->size &&
             tokens->tokens[next].kind == TOKEN_WHITESPACE)
        next++;

      if (next >= tokens->size || tokens->tokens[next].kind != TOKEN_LPAREN) {
        continue; /* Not a call (maybe address taking or pointer) */
      }

      /* Find matching RPAREN */
      {
        size_t lparen = next;
        size_t rparen = lparen + 1;
        int depth = 1;

        while (rparen < tokens->size && depth > 0) {
          if (tokens->tokens[rparen].kind == TOKEN_LPAREN)
            depth++;
          else if (tokens->tokens[rparen].kind == TOKEN_RPAREN)
            depth--;
          rparen++;
        }
        rparen--; /* Points to ')' */

        if (depth != 0)
          continue; /* Malformed/Unterminated */

        /* Find Statement Terminator (Scanner should handle boundaries but we do
         * naive scan) */
        {
          size_t semi = find_next_token(tokens, rparen, TOKEN_SEMICOLON);
          if (semi >= tokens->size)
            continue;

          if (target->type == REF_VOID_TO_INT) {
            /* Transform: func(a,b); -> if (func(a,b) != 0) return EIO; */
            /* We replace range [i, semi] inclusive? */
            /* Scan backwards to ensure we consume the whole statement if
               possible? Wait, void calls are usually standalone statements:
               func(a);

               We assume for simplicity it is a statement expression.
            */
            /* Construct replacement string */
            char *original_call =
                tokens_to_string(tokens, i, rparen + 1); /* func... ) */
            char *replacement = NULL;
            if (!original_call)
              return ENOMEM;

            if (asprintf(&replacement, "if (%s != 0) return EIO;",
                         original_call) == -1) {
              free(original_call);
              return ENOMEM;
            }
            free(original_call);

            /* Replace from 'func' to ';' inclusive (assuming stand-alone) */
            /* Checking if preceded by assign logic */
            /* If void-to-int, implies it wasn't assigned before. */
            if (add_replacement(patches, i, semi + 1, replacement) != 0)
              return ENOMEM;

            /* Advance i */
            i = semi;
          } else if (target->type == REF_PTR_TO_INT_OUT) {
            /* Transform: var = func(a); -> if (func(a, &var) != 0) return EIO;
             */
            /* We need to find the variable assignment preceding the call */
            /* Look backwards from 'i' (func name) for '=' */
            size_t eq_idx = i;
            int found_eq = 0;
            while (eq_idx > 0) {
              eq_idx--;
              if (tokens->tokens[eq_idx].kind == TOKEN_WHITESPACE)
                continue;

              if (tokens->tokens[eq_idx].kind == TOKEN_OTHER &&
                  tokens->tokens[eq_idx].length == 1 &&
                  *tokens->tokens[eq_idx].start == '=') {
                found_eq = 1;
                break;
              }
              /* Hit statement boundary */
              if (tokens->tokens[eq_idx].kind == TOKEN_SEMICOLON ||
                  tokens->tokens[eq_idx].kind == TOKEN_LBRACE ||
                  tokens->tokens[eq_idx].kind == TOKEN_RBRACE) {
                break;
              }
            }

            if (found_eq) {
              /* Found 'var = func(...)'. */
              /* Find 'var' identifier before '=' */
              size_t var_idx = eq_idx;
              int found_var = 0;
              while (var_idx > 0) {
                var_idx--;
                if (tokens->tokens[var_idx].kind == TOKEN_WHITESPACE)
                  continue;
                if (tokens->tokens[var_idx].kind == TOKEN_IDENTIFIER) {
                  found_var = 1;
                  break;
                }
                /* Ignore type keywords if declaration 'Type var =' */
                /* Just assume the token before = (skipping ws) is valid lvalue
                 * var */
                found_var = 1; /* Assume checking done by compiler */
                break;
              }

              if (found_var) {
                /* Construct new call.
                   Original args: from lparen+1 to rparen. */
                char *var_name = tokens_to_string(tokens, var_idx, var_idx + 1);
                char *args = tokens_to_string(tokens, lparen + 1, rparen);
                char *replacement = NULL;
                char *comma = "";

                /* Are there existing args? */
                int has_args = 0;
                size_t k;
                for (k = lparen + 1; k < rparen; k++) {
                  if (tokens->tokens[k].kind != TOKEN_WHITESPACE) {
                    has_args = 1;
                    break;
                  }
                }
                if (has_args)
                  comma = ", ";

                if (!var_name || !args) {
                  free(var_name);
                  free(args);
                  return ENOMEM;
                }

                /* Prepend "; " to terminate potential declaration (converting
                 * assignment to statement) */
                if (asprintf(&replacement,
                             "; if (%s(%s%s&%s) != 0) return EIO;",
                             target->name, args, comma, var_name) == -1) {
                  free(var_name);
                  free(args);
                  return ENOMEM;
                }
                free(var_name);
                free(args);

                /* Replace from '=' to ';' inclusive (the assignment and
                 * original statement terminator) */
                /* Range: [eq_idx, semi + 1] will replace '= func(...);'
                   Leaving 'var' or 'Type var' intact. */
                if (add_replacement(patches, eq_idx, semi + 1, replacement) !=
                    0)
                  return ENOMEM;

                i = semi;
              }
            }
          }
        }
      }
    }
  }
  return 0;
}

int rewrite_body(const struct TokenList *tokens,
                 const struct AllocationSiteList *allocs,
                 const struct RefactoredFunction *funcs, size_t func_count,
                 char **out_code) {
  struct PatchList patches;
  int rc;
  char *output = NULL;
  size_t out_len = 0;
  size_t current_token = 0;
  size_t p_idx = 0;

  if (!tokens || !out_code)
    return EINVAL;

  if (patch_list_init(&patches) != 0)
    return ENOMEM;

  /* 1. Generate Allocation Checks */
  if (allocs) {
    rc = process_allocations(tokens, allocs, &patches);
    if (rc != 0)
      goto cleanup;
  }

  /* 2. Generate Function Rewrites */
  if (funcs && func_count > 0) {
    rc = process_function_calls(tokens, funcs, func_count, &patches);
    if (rc != 0)
      goto cleanup;
  }

  /* 3. Sort Patches */
  qsort(patches.items, patches.size, sizeof(struct Replacement),
        cmp_replacements);

  /* 4. Apply Patches / Reconstruct String */
  /* Estimate size? Alloc a reasonable buffer and grow. */
  /* Safe simple buffer logic */
  {
    size_t cap = 1024;
    output = (char *)malloc(cap);
    if (!output) {
      rc = ENOMEM;
      goto cleanup;
    }
    output[0] = '\0';

    while (current_token < tokens->size) {
      /* Check for pending patch */
      if (p_idx < patches.size &&
          patches.items[p_idx].token_start == current_token) {
        struct Replacement *rep = &patches.items[p_idx];
        size_t rep_len = strlen(rep->text);

        while (out_len + rep_len + 1 > cap) {
          char *tmp;
          cap *= 2;
          tmp = (char *)realloc(output, cap);
          if (!tmp) {
            rc = ENOMEM;
            goto cleanup;
          }
          output = tmp;
        }
        memcpy(output + out_len, rep->text, rep_len);
        out_len += rep_len;
        output[out_len] = '\0';

        /* Advance past replaced tokens */
        current_token = rep->token_end;
        p_idx++;
      } else {
        /* Append current token text */
        /* Note: This simplistic reconstruction assumes tokens cover the file
           densely or we accept whitespace loss if tokens don't include it. (Our
           CST tokenizer includes WHITESPACE tokens, so exact reconstruction is
           possible). */
        const struct Token *tok = &tokens->tokens[current_token];
        size_t tok_len = tok->length;

        while (out_len + tok_len + 1 > cap) {
          char *tmp;
          cap *= 2;
          tmp = (char *)realloc(output, cap);
          if (!tmp) {
            rc = ENOMEM;
            goto cleanup;
          }
          output = tmp;
        }
        memcpy(output + out_len, tok->start, tok_len);
        out_len += tok_len;
        output[out_len] = '\0';

        current_token++;
      }
    }
  }

  *out_code = output;
  output = NULL; /* Ownership transferred */
  rc = 0;

cleanup:
  patch_list_free(&patches);
  if (output)
    free(output);
  return rc;
}
