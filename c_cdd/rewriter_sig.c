/**
 * @file rewriter_sig.c
 * @brief Implementation of function signature rewriting logic.
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

/* Helper: Trim trailing whitespace from a mutable string */
static void trim_trailing_ws(char *s) {
  size_t len;
  if (!s)
    return;
  len = strlen(s);
  while (len > 0 && isspace((unsigned char)s[len - 1])) {
    s[--len] = '\0';
  }
}

/* Helper: Concatenate token contents in range [start, end) into a single
 * string. */
static int join_tokens(const struct TokenList *tokens, size_t start, size_t end,
                       char **out) {
  size_t len = 0;
  size_t i;
  char *buf;
  char *p;

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

static int is_void(const struct Token *tok) {
  return tok->kind == TOKEN_IDENTIFIER && tok->length == 4 &&
         strncmp((const char *)tok->start, "void", 4) == 0;
}

static int is_int(const struct Token *tok) {
  return tok->kind == TOKEN_IDENTIFIER && tok->length == 3 &&
         strncmp((const char *)tok->start, "int", 3) == 0;
}

static int is_storage_specifier(const struct Token *tok) {
  if (tok->kind == TOKEN_IDENTIFIER) {
    const char *s = (const char *)tok->start;
    size_t l = tok->length;
    if (l == 6 && strncmp(s, "static", 6) == 0)
      return 1;
    if (l == 6 && strncmp(s, "extern", 6) == 0)
      return 1;
    if (l == 6 && strncmp(s, "inline", 6) == 0)
      return 1;
  }
  return 0;
}

static size_t find_type_start(const struct TokenList *tokens, size_t end_idx) {
  size_t i = 0;
  while (i < end_idx) {
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE) {
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

static int is_pure_void(const struct TokenList *tokens, size_t start,
                        size_t end) {
  size_t i;
  int saw_void = 0;
  for (i = start; i < end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE)
      continue;
    if (is_void(&tokens->tokens[i])) {
      saw_void = 1;
    } else {
      return 0;
    }
  }
  return saw_void;
}

static int is_pure_int(const struct TokenList *tokens, size_t start,
                       size_t end) {
  size_t i;
  int saw_int = 0;
  for (i = start; i < end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE)
      continue;
    if (is_int(&tokens->tokens[i])) {
      saw_int = 1;
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

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      lparen_idx = i;
      found_lparen = 1;
      break;
    }
  }

  if (!found_lparen || lparen_idx == 0) {
    return EINVAL;
  }

  name_idx = lparen_idx;
  do {
    name_idx--;
  } while (name_idx > 0 && tokens->tokens[name_idx].kind == TOKEN_WHITESPACE);

  if (tokens->tokens[name_idx].kind != TOKEN_IDENTIFIER) {
    return EINVAL;
  }

  type_start_idx = find_type_start(tokens, name_idx);

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

  rc = join_tokens(tokens, name_idx, name_idx + 1, &name_str);
  if (rc != 0)
    goto cleanup;

  rc = join_tokens(tokens, type_start_idx, name_idx, &type_str);
  if (rc != 0)
    goto cleanup;

  rc = join_tokens(tokens, lparen_idx + 1, rparen_idx, &args_str);
  if (rc != 0)
    goto cleanup;

  if (is_pure_int(tokens, type_start_idx, name_idx)) {
    if (asprintf(out_code, "%s%s%s(%s)", storage_str, type_str, name_str,
                 args_str) == -1) {
      rc = ENOMEM;
    }
  } else if (is_pure_void(tokens, type_start_idx, name_idx)) {
    if (asprintf(out_code, "%sint %s(%s)", storage_str, name_str, args_str) ==
        -1) {
      rc = ENOMEM;
    }
  } else {
    int args_is_void = 0;
    int args_is_empty = (lparen_idx + 1 == rparen_idx);

    if (!args_is_empty) {
      size_t k;
      int has_ident = 0;
      for (k = lparen_idx + 1; k < rparen_idx; ++k) {
        if (tokens->tokens[k].kind == TOKEN_WHITESPACE)
          continue;
        if (is_void(&tokens->tokens[k]))
          has_ident = 1;
        else {
          has_ident = 0;
          break;
        }
      }
      if (has_ident)
        args_is_void = 1;
    }

    trim_trailing_ws(type_str);

    if (args_is_empty || args_is_void) {
      if (asprintf(out_code, "%sint %s(%s *out)", storage_str, name_str,
                   type_str) == -1) {
        rc = ENOMEM;
      }
    } else {
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
