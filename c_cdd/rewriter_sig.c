/**
 * @file rewriter_sig.c
 * @brief Implementation of function signature rewriting logic.
 * Parses token streams to extract storage specifiers, return types, and
 * arguments, then reconstructs them into the standard error-code pattern.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c89stringutils_string_extras.h>

#include "rewriter_sig.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <sys/errno.h>
#endif

/**
 * @brief Join tokens into a string, preserving original whitespace if possible.
 *
 * Iterates through the given range [start, end) of the token list.
 * Effectively creates a substring of the original source code covered by these
 * tokens.
 *
 * @param[in] tokens The full token list.
 * @param[in] start Start index (inclusive).
 * @param[in] end End index (exclusive).
 * @param[out] out Pointer to char* for result.
 * @return 0 on success, ENOMEM on allocation failure.
 */
static int join_tokens(const struct TokenList *tokens, size_t start, size_t end,
                       char **out) {
  size_t len = 0;
  size_t i;
  char *buf;
  char *p;

  if (start >= end) {
    *out = strdup("");
    return *out ? 0 : ENOMEM;
  }

  for (i = start; i < end; ++i) {
    len += tokens->tokens[i].length;
  }

  buf = (char *)malloc(len + 1);
  if (!buf)
    return ENOMEM;

  p = buf;
  for (i = start; i < end; ++i) {
    memcpy(p, tokens->tokens[i].start, tokens->tokens[i].length);
    p += tokens->tokens[i].length;
  }
  *p = '\0';
  *out = buf;
  return 0;
}

/**
 * @brief Check if a token is a C storage class specifier.
 *
 * @param[in] tok The token to check.
 * @return 1 if static/extern/typedef/inline, 0 otherwise.
 */
static int is_storage_specifier(const struct Token *tok) {
  if (tok->kind == TOKEN_IDENTIFIER || tok->kind == TOKEN_OTHER) {
    /* Inline is technically a function specifier but treated as storage prefix
     * here */
    const char *s = (const char *)tok->start;
    size_t l = tok->length;
    if (l == 6 && strncmp(s, "static", 6) == 0)
      return 1;
    if (l == 6 && strncmp(s, "extern", 6) == 0)
      return 1;
    if (l == 7 && strncmp(s, "typedef", 7) == 0)
      return 1;
    if (l == 6 && strncmp(s, "inline", 6) == 0)
      return 1;
    if (l == 8 && strncmp(s, "__inline", 8) == 0)
      return 1; /* MSVC/GNU extension */
  }
  return 0;
}

/**
 * @brief Find the boundary between storage specifiers and the return type.
 *
 * @param[in] tokens Token list.
 * @param[in] end_idx Index of the function name (upper bound).
 * @return The index where the return type begins.
 */
static size_t find_type_start(const struct TokenList *tokens, size_t end_idx) {
  size_t i = 0;
  while (i < end_idx) {
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
        tokens->tokens[i].kind == TOKEN_COMMENT) {
      i++;
      continue;
    }
    if (is_storage_specifier(&tokens->tokens[i])) {
      i++;
      continue;
    }
    return i;
  }
  return i;
}

/**
 * @brief Trim trailing whitespace from a C string in place.
 *
 * @param[in,out] s The string to trim.
 */
static void trim_trailing_ws(char *s) {
  size_t len;
  if (!s)
    return;
  len = strlen(s);
  while (len > 0 && isspace((unsigned char)s[len - 1])) {
    s[--len] = '\0';
  }
}

/**
 * @brief Check if the range of tokens represents the specific type "void".
 * Ignores comments/whitespace.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @return 1 if strictly "void", 0 otherwise (e.g. "void *").
 */
static int is_pure_void(const struct TokenList *tokens, size_t start,
                        size_t end) {
  size_t i;
  int saw_void = 0;
  for (i = start; i < end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
        tokens->tokens[i].kind == TOKEN_COMMENT)
      continue;

    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER &&
        tokens->tokens[i].length == 4 &&
        strncmp((const char *)tokens->tokens[i].start, "void", 4) == 0) {
      if (saw_void)
        return 0; /* void void? */
      saw_void = 1;
    } else {
      return 0; /* Any other token means not pure void */
    }
  }
  return saw_void;
}

/**
 * @brief Check if the range of tokens represents the specific type "int".
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @return 1 if strictly "int" (or "signed int"), 0 otherwise.
 */
static int is_pure_int(const struct TokenList *tokens, size_t start,
                       size_t end) {
  size_t i;
  int saw_int = 0;
  for (i = start; i < end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
        tokens->tokens[i].kind == TOKEN_COMMENT)
      continue;

    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      /* Allow 'signed' or 'int'. reject 'unsigned' (u32), 'short' etc unless we
         decide standard int covers them. Standard says preserve 'int' return.
         'long' is not 'int'. */
      const char *s = (const char *)tokens->tokens[i].start;
      size_t l = tokens->tokens[i].length;
      if (l == 3 && strncmp(s, "int", 3) == 0) {
        saw_int = 1;
      } else if (l == 6 && strncmp(s, "signed", 6) == 0) {
        /* continue */
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }
  return saw_int;
}

int rewrite_signature(const struct TokenList *tokens, char **out_code) {
  size_t i;
  size_t lparen_idx = 0;
  size_t name_idx = 0;
  size_t type_start_idx = 0;
  size_t rparen_idx = 0;
  int found_lparen = 0;
  int found_rparen = 0;

  char *storage_str = NULL;
  char *name_str = NULL;
  char *type_str = NULL;
  char *args_str = NULL;
  int rc = 0;

  if (!tokens || !out_code)
    return EINVAL;

  /* 1. Find the opening parenthesis of the argument list */
  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      lparen_idx = i;
      found_lparen = 1;
      break;
    }
  }

  if (!found_lparen || lparen_idx == 0) {
    return EINVAL; /* Not a function definition/declaration */
  }

  /* 2. Find the Function Name (last identifier before lparen) */
  name_idx = lparen_idx;
  do {
    name_idx--;
  } while (name_idx > 0 && (tokens->tokens[name_idx].kind == TOKEN_WHITESPACE ||
                            tokens->tokens[name_idx].kind == TOKEN_COMMENT));

  if (tokens->tokens[name_idx].kind != TOKEN_IDENTIFIER) {
    return EINVAL; /* e.g. "(*f)(...)" function pointer decl or weird logic, not
                      supported yet */
  }

  /* 3. Determine extent of return type vs storage specifiers */
  type_start_idx = find_type_start(tokens, name_idx);

  /* 4. Find matching R-Paren to get args */
  {
    int depth = 0;
    for (i = lparen_idx; i < tokens->size; ++i) {
      if (tokens->tokens[i].kind == TOKEN_LPAREN)
        depth++;
      else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
        depth--;
        if (depth == 0) {
          rparen_idx = i;
          found_rparen = 1;
          break;
        }
      }
    }
  }
  if (!found_rparen)
    return EINVAL;

  /* 5. Extract Components */

  /* Storage (e.g. "static inline ") */
  if (type_start_idx > 0) {
    rc = join_tokens(tokens, 0, type_start_idx, &storage_str);
    if (rc != 0)
      goto cleanup;
  } else {
    storage_str = strdup("");
    if (!storage_str) {
      rc = ENOMEM;
      goto cleanup;
    }
  }

  /* Name (e.g. "my_func") */
  rc = join_tokens(tokens, name_idx, name_idx + 1, &name_str);
  if (rc != 0)
    goto cleanup;

  /* Type (e.g. "char *") */
  rc = join_tokens(tokens, type_start_idx, name_idx, &type_str);
  if (rc != 0)
    goto cleanup;

  /* Args (e.g. "int a, float b") - inside variable parens */
  if (lparen_idx + 1 < rparen_idx) {
    rc = join_tokens(tokens, lparen_idx + 1, rparen_idx, &args_str);
  } else {
    args_str = strdup("");
  }
  if (rc != 0 || !args_str) {
    if (!rc)
      rc = ENOMEM;
    goto cleanup;
  }

  /* 6. Construct New Signature */

  if (is_pure_int(tokens, type_start_idx, name_idx)) {
    /* Already returns int, assume error code pattern or leave as is.
       Reconstruct: [Storage] [Type] [Name]([Args]) */
    if (asprintf(out_code, "%s%s%s(%s)", storage_str, type_str, name_str,
                 args_str) == -1) {
      rc = ENOMEM;
    }
  } else if (is_pure_void(tokens, type_start_idx, name_idx)) {
    /* void -> int */
    /* Reconstruct: [Storage] int [Name]([Args]) */
    /* Check for trailing space on storage_str */
    if (asprintf(out_code, "%sint %s(%s)", storage_str, name_str, args_str) ==
        -1) {
      rc = ENOMEM;
    }
  } else {
    /* Complex type -> int, Type moved to args */
    int args_is_void = 0;
    int args_is_empty =
        (args_str[0] == '\0'); /* Derived from loop range previously */

    if (!args_is_empty) {
      /* Check if args is just "void" */
      /* use existing parser logic or simple string compare since we joined it?
         String compare is risky due to whitespace (" void ").
         Iterate tokens again for robustness. */
      size_t k;
      int has_ident = 0;
      for (k = lparen_idx + 1; k < rparen_idx; ++k) {
        if (tokens->tokens[k].kind == TOKEN_WHITESPACE ||
            tokens->tokens[k].kind == TOKEN_COMMENT)
          continue;
        if (tokens->tokens[k].kind == TOKEN_IDENTIFIER &&
            tokens->tokens[k].length == 4 &&
            strncmp((const char *)tokens->tokens[k].start, "void", 4) == 0)
          has_ident = 1;
        else {
          has_ident = 0;
          break; /* found something else */
        }
      }
      if (has_ident)
        args_is_void = 1;
    }

    trim_trailing_ws(type_str);

    /* If return type has no trailing pointer *, and is not struct, we add one *
       to make it output ptr. If it is a pointer `char *`, we add `*` to make
       `char **`. Logic: Just append `*out`. Example: `int` (if complex logic
       used) -> `int *out` `char *` -> `char * *out` (spacing ok) `struct S` ->
       `struct S *out`
    */

    if (args_is_empty || args_is_void) {
      /* func() or func(void) -> func(Type *out) */
      if (asprintf(out_code, "%sint %s(%s *out)", storage_str, name_str,
                   type_str) == -1) {
        rc = ENOMEM;
      }
    } else {
      /* func(args) -> func(args, Type *out) */
      if (asprintf(out_code, "%sint %s(%s, %s *out)", storage_str, name_str,
                   args_str, type_str) == -1) {
        rc = ENOMEM;
      }
    }
  }

cleanup:
  free(storage_str);
  free(name_str);
  free(type_str);
  free(args_str);
  return rc;
}
