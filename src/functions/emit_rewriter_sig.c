/**
 * @file rewriter_sig.c
 * @brief Implementation of function signature rewriting logic.
 *
 * Implements a recursive descent-style scanner to robustly identify logical
 * blocks of a C function signature (Attributes, Storage, Type, Name, Args)
 * while respecting nested parentheses and brackets.
 *
 * Now supports K&R style definitions by capturing trailing declarations.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c89stringutils_string_extras.h>

#include "functions/emit_rewriter_sig.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <sys/errno.h>
#endif

/**
 * @brief Internal component structure for a parsed signature.
 */
struct ParsedSig {
  char *attributes; /**< Leading attributes aka [[nodiscard]] */
  char *storage;    /**< static, extern, inline */
  char *ret_type;   /**< int, char *, struct S */
  char *name;       /**< func_name */
  char *args;       /**< int a, int b */
  char *k_r_decls;  /**< K&R Declarations (e.g. "int a;") */
  int is_void_ret;  /**< 1 if return type is visually 'void' (no pointers) */
};

/**
 * @brief Initialize ParsedSig to NULLs.
 */
static void parsed_sig_init(struct ParsedSig *sig) {
  sig->attributes = NULL;
  sig->storage = NULL;
  sig->ret_type = NULL;
  sig->name = NULL;
  sig->args = NULL;
  sig->k_r_decls = NULL;
  sig->is_void_ret = 0;
}

/**
 * @brief Free ParsedSig resources.
 */
static void parsed_sig_free(struct ParsedSig *sig) {
  if (sig->attributes)
    free(sig->attributes);
  if (sig->storage)
    free(sig->storage);
  if (sig->ret_type)
    free(sig->ret_type);
  if (sig->name)
    free(sig->name);
  if (sig->args)
    free(sig->args);
  if (sig->k_r_decls)
    free(sig->k_r_decls);
}

/**
 * @brief Join tokens into a string.
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
 * @brief Check if token is a storage class specifier.
 */
static int is_storage_specifier(const struct Token *tok) {
  switch (tok->kind) {
  case TOKEN_KEYWORD_STATIC:
  case TOKEN_KEYWORD_EXTERN:
  case TOKEN_KEYWORD_INLINE:
  case TOKEN_KEYWORD_NORETURN:
  case TOKEN_KEYWORD_THREAD_LOCAL:
    return 1;
  case TOKEN_IDENTIFIER: /* Check extensions like __inline */
    if (tok->length == 8 &&
        strncmp((const char *)tok->start, "__inline", 8) == 0)
      return 1;
    return 0;
  default:
    return 0;
  }
}

/**
 * @brief Find the end of a balanced group (parens or brackets).
 *
 * @param tokens Token list.
 * @param start Index of the opening token (e.g. LPAREN).
 * @param open Kind of opening token.
 * @param close Kind of closing token.
 * @return Index of the matching closing token, or tokens->size if not found.
 */
static size_t find_balanced_end(const struct TokenList *tokens, size_t start,
                                enum TokenKind open, enum TokenKind close) {
  size_t i = start + 1;
  int depth = 1;

  while (i < tokens->size && depth > 0) {
    if (tokens->tokens[i].kind == open) {
      depth++;
    } else if (tokens->tokens[i].kind == close) {
      depth--;
    }
    i++;
  }

  if (depth == 0)
    return i - 1; /* Return index of the closer */
  return tokens->size;
}

/**
 * @brief Identify if return type is logically 'void' (no pointers).
 */
static int check_is_void(const struct TokenList *tokens, size_t start,
                         size_t end) {
  size_t i;
  int saw_void = 0;

  for (i = start; i < end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
        tokens->tokens[i].kind == TOKEN_COMMENT)
      continue;

    /* If we see a pointer start ('*') or brackets, it's not void */
    if (tokens->tokens[i].kind == TOKEN_STAR) {
      return 0;
    }

    if (tokens->tokens[i].kind == TOKEN_KEYWORD_VOID) {
      saw_void = 1;
    } else if (tokens->tokens[i].kind != TOKEN_WHITESPACE) {
      /* Any token other than void or whitespace implies not simple void */
      return 0;
    }
  }
  return saw_void;
}

/**
 * @brief Convert 'void' args string to empty string, or detect if empty.
 */
static int args_represent_void(const char *args) {
  const char *p = args;
  while (*p && isspace((unsigned char)*p))
    p++;
  if (*p == '\0')
    return 1;
  /* Check for 'void' */
  if (strncmp(p, "void", 4) == 0) {
    p += 4;
    while (*p && isspace((unsigned char)*p))
      p++;
    if (*p == '\0')
      return 1;
  }
  return 0;
}

/**
 * @brief Check if a range contains meaningful tokens (not just
 * whitespace/comments).
 */
static int has_meaningful_tokens(const struct TokenList *tokens, size_t start,
                                 size_t end) {
  size_t i;
  for (i = start; i < end; ++i) {
    if (tokens->tokens[i].kind != TOKEN_WHITESPACE &&
        tokens->tokens[i].kind != TOKEN_COMMENT) {
      return 1;
    }
  }
  return 0;
}

int rewrite_signature(const struct TokenList *tokens, char **out_code) {
  struct ParsedSig sig;
  size_t i = 0;
  size_t lparen_idx = 0;
  size_t rparen_idx = 0;
  size_t name_idx = 0;
  size_t type_end_idx = 0;
  size_t attr_end_idx = 0;
  size_t storage_end_idx = 0;
  int rc = 0;

  if (!tokens || !out_code)
    return EINVAL;
  parsed_sig_init(&sig);

  /* 1. Attributes (C23 [[...]]) */
  /* Skip whitespace/comments */
  while (i < tokens->size && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                              tokens->tokens[i].kind == TOKEN_COMMENT))
    i++;

  if (i < tokens->size && tokens->tokens[i].kind == TOKEN_LBRACKET) {
    if (i + 1 < tokens->size && tokens->tokens[i + 1].kind == TOKEN_LBRACKET) {
      /* Found [[ */
      size_t end_attr =
          find_balanced_end(tokens, i, TOKEN_LBRACKET, TOKEN_RBRACKET);

      if (end_attr < tokens->size && end_attr > i) {
        attr_end_idx = end_attr + 1; /* Past the closing ] */
      }

      if (attr_end_idx > 0) {
        rc = join_tokens(tokens, i, attr_end_idx, &sig.attributes);
        if (rc)
          goto cleanup;
        i = attr_end_idx;
      }
    }
  }

  /* 2. Storage Specifiers */
  /* Consumes static, extern, inline... */
  {
    size_t start_storage = i;
    int found_storage = 0;
    while (i < tokens->size) {
      if (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
          tokens->tokens[i].kind == TOKEN_COMMENT) {
        i++;
        continue;
      }
      if (is_storage_specifier(&tokens->tokens[i])) {
        found_storage = 1;
        i++;
      } else {
        break; /* Found type or name */
      }
    }
    storage_end_idx = i;
    if (found_storage) {
      rc = join_tokens(tokens, start_storage, storage_end_idx, &sig.storage);
      if (rc)
        goto cleanup;
    } else {
      if (start_storage < storage_end_idx) {
        rc = join_tokens(tokens, start_storage, storage_end_idx, &sig.storage);
        if (rc)
          goto cleanup;
      } else {
        sig.storage = strdup("");
      }
    }
  }

  /* 3. Scan for Name and Arguments (The Anchor) */
  {
    size_t k = storage_end_idx;
    size_t candidate_name = 0;
    int found = 0;

    while (k < tokens->size) {
      if (tokens->tokens[k].kind == TOKEN_LPAREN) {
        /* Check if preceding non-ws token was identifier */
        size_t prev = k;
        int is_ident = 0;
        while (prev > storage_end_idx) {
          prev--;
          if (tokens->tokens[prev].kind == TOKEN_WHITESPACE ||
              tokens->tokens[prev].kind == TOKEN_COMMENT)
            continue;

          if (tokens->tokens[prev].kind == TOKEN_IDENTIFIER) {
            is_ident = 1;
            candidate_name = prev;
          }
          break; /* Found closest non-ws token */
        }

        if (is_ident) {
          /* Found `Name (` pattern */
          lparen_idx = k;
          name_idx = candidate_name;
          found = 1;
          break;
        }
      }
      k++;
    }

    if (!found) {
      rc = EINVAL;
      goto cleanup;
    }
  }

  /* 4. Extract Return Type & Name */
  type_end_idx = name_idx;

  rc = join_tokens(tokens, name_idx, lparen_idx, &sig.name);
  if (rc)
    goto cleanup;

  if (type_end_idx > storage_end_idx) {
    rc = join_tokens(tokens, storage_end_idx, type_end_idx, &sig.ret_type);
    if (rc)
      goto cleanup;
  } else {
    sig.ret_type = strdup("int ");
  }

  sig.is_void_ret = check_is_void(tokens, storage_end_idx, type_end_idx);

  /* 5. Extract Arguments */
  {
    size_t rparen =
        find_balanced_end(tokens, lparen_idx, TOKEN_LPAREN, TOKEN_RPAREN);
    if (rparen >= tokens->size) {
      rc = EINVAL;
      goto cleanup;
    }
    rparen_idx = rparen;

    /* Content between parens */
    if (lparen_idx + 1 < rparen) {
      rc = join_tokens(tokens, lparen_idx + 1, rparen, &sig.args);
    } else {
      sig.args = strdup("");
    }
    if (rc)
      goto cleanup;
  }

  /* 6. Extract K&R Declarations (if any) */
  if (rparen_idx + 1 < tokens->size) {
    if (has_meaningful_tokens(tokens, rparen_idx + 1, tokens->size)) {
      rc = join_tokens(tokens, rparen_idx + 1, tokens->size, &sig.k_r_decls);
      if (rc)
        goto cleanup;
    }
  }

  /* 7. Construct new signature */
  {
    int args_is_empty = args_represent_void(sig.args);
    char *new_args = NULL;
    char *prefix = sig.attributes ? sig.attributes : "";
    char *k_r_suffix = sig.k_r_decls ? sig.k_r_decls : "";

    if (sig.is_void_ret) {
      /* Case: void f(...) -> int f(...) ... */
#ifdef HAVE_ASPRINTF
      asprintf(out_code, "%s%sint %s(%s)%s", prefix, sig.storage, sig.name,
               sig.args, k_r_suffix);
#else
      int len = strlen(prefix) + strlen(sig.storage) + strlen(sig.name) +
                strlen(sig.args) + strlen(k_r_suffix) + 20;
      *out_code = malloc(len);
      if (!*out_code) {
        rc = ENOMEM;
        goto cleanup;
      }
      sprintf(*out_code, "%s%sint %s(%s)%s", prefix, sig.storage, sig.name,
              sig.args, k_r_suffix);
#endif
    } else {
      /* Case: Type f(...) -> int f(..., Type *out) */
      /* Trim trailing whitespace from ret_type for cleaner output */
      size_t rtl = strlen(sig.ret_type);
      while (rtl > 0 && isspace((unsigned char)sig.ret_type[rtl - 1]))
        sig.ret_type[--rtl] = '\0';

      if (sig.k_r_decls) {
        /* K&R Refactor Logic */
        /* f(a) int a; -> f(a, out) int a; Type *out; */
        if (args_is_empty) {
          /* f() -> f(out) */
#ifdef HAVE_ASPRINTF
          asprintf(&new_args, "out");
#else
          new_args = strdup("out");
#endif
        } else {
          /* f(a) -> f(a, out) */
#ifdef HAVE_ASPRINTF
          asprintf(&new_args, "%s, out", sig.args);
#else
          new_args = malloc(strlen(sig.args) + 10);
          sprintf(new_args, "%s, out", sig.args);
#endif
        }
      } else {
        /* Standard Prototype Logic */
        if (args_is_empty) {
#ifdef HAVE_ASPRINTF
          asprintf(&new_args, "%s *out", sig.ret_type);
#else
          new_args = malloc(strlen(sig.ret_type) + 10);
          sprintf(new_args, "%s *out", sig.ret_type);
#endif
        } else {
#ifdef HAVE_ASPRINTF
          asprintf(&new_args, "%s, %s *out", sig.args, sig.ret_type);
#else
          new_args = malloc(strlen(sig.args) + strlen(sig.ret_type) + 10);
          sprintf(new_args, "%s, %s *out", sig.args, sig.ret_type);
#endif
        }
      }

      if (!new_args) {
        rc = ENOMEM;
        goto cleanup;
      }

      /* Assemble final string */
#ifdef HAVE_ASPRINTF
      if (sig.k_r_decls) {
        asprintf(out_code, "%s%sint %s(%s)%s %s *out;", prefix, sig.storage,
                 sig.name, new_args, k_r_suffix, sig.ret_type);
      } else {
        asprintf(out_code, "%s%sint %s(%s)", prefix, sig.storage, sig.name,
                 new_args);
      }
#else
      {
        int len = strlen(prefix) + strlen(sig.storage) + strlen(sig.name) +
                  strlen(new_args) + strlen(k_r_suffix) + strlen(sig.ret_type) +
                  30;
        *out_code = malloc(len);
        if (sig.k_r_decls) {
          sprintf(*out_code, "%s%sint %s(%s)%s %s *out;", prefix, sig.storage,
                  sig.name, new_args, k_r_suffix, sig.ret_type);
        } else {
          sprintf(*out_code, "%s%sint %s(%s)", prefix, sig.storage, sig.name,
                  new_args);
        }
      }
#endif
      free(new_args);
    }
  }

cleanup:
  parsed_sig_free(&sig);
  return rc;
}
