/**
 * @file strategy_safety.c
 * @brief Implementation of safety injection strategies.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse_str.h" /* For string duplication utilities if specific needed */
#include "functions/parse_strategy.h"

/* Common constants */
/* Note: In a larger system these might be configurable via context struct */
static const char *const DEFAULT_ERROR_CODE = "ENOMEM";

/* Internal helpers */

/**
 * @brief Find explicit token indices for range extractions.
 */
static size_t find_next_token_idx(const struct TokenList *tokens, size_t start,
                                  enum TokenKind kind) {
  size_t i;
  for (i = start; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == kind)
      return i;
  }
  return tokens->size;
}

static char *range_to_string(const struct TokenList *tokens, size_t start,
                             size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  if (start >= end) {
    char *empty = (char *)malloc(1);
    if (empty)
      *empty = '\0';
    return empty;
  }

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

/* --- Realloc Strategy --- */

int strategy_rewrite_realloc(const struct TokenList *const tokens,
                             const struct AllocationSite *const site,
                             const size_t semi_idx,
                             struct PatchList *const patches) {
  size_t call_idx = site->token_index;
  size_t lparen_idx;
  size_t assign_op_idx = 0;
  size_t stmt_start = 0;
  int is_self_assign = 0;

  if (!site->var_name)
    return 0;

  /* 1. Locate assignment `=` backwards from call */
  {
    size_t i = call_idx;
    while (i > 0) {
      i--;
      if (tokens->tokens[i].kind == TOKEN_ASSIGN) {
        assign_op_idx = i;
        break;
      }
      /* Boundary check to stop search */
      if (tokens->tokens[i].kind == TOKEN_SEMICOLON ||
          tokens->tokens[i].kind == TOKEN_LBRACE ||
          tokens->tokens[i].kind == TOKEN_RBRACE) {
        break;
      }
    }
  }

  if (assign_op_idx == 0)
    return 0; /* Not an assignment */

  /* 2. Find start of statement */
  stmt_start = assign_op_idx;
  while (stmt_start > 0) {
    size_t prev = stmt_start - 1;
    if (tokens->tokens[prev].kind == TOKEN_SEMICOLON ||
        tokens->tokens[prev].kind == TOKEN_LBRACE ||
        tokens->tokens[prev].kind == TOKEN_RBRACE) {
      break;
    }
    stmt_start--;
  }
  /* Skip leading whitespace */
  while (stmt_start < assign_op_idx &&
         tokens->tokens[stmt_start].kind == TOKEN_WHITESPACE)
    stmt_start++;

  /* 3. Check arguments: realloc(ptr, size) */
  lparen_idx = find_next_token_idx(tokens, call_idx, TOKEN_LPAREN);
  if (lparen_idx >= tokens->size)
    return 0;

  {
    /* Extract first arg name crudely */
    size_t start_arg = lparen_idx + 1;
    while (start_arg < tokens->size &&
           tokens->tokens[start_arg].kind == TOKEN_WHITESPACE)
      start_arg++;

    if (start_arg < tokens->size &&
        tokens->tokens[start_arg].kind == TOKEN_IDENTIFIER) {
      /* Compare against var_name */
      if (token_matches_string(&tokens->tokens[start_arg], site->var_name)) {
        is_self_assign = 1;
      }
    }
  }

  if (is_self_assign) {
    char *call_expr = NULL;
    char *replacement = NULL;

    /* Extract "realloc(p, n)" text */
    call_expr = range_to_string(tokens, call_idx, semi_idx);
    if (!call_expr)
      return ENOMEM;

      /* Build safe block */
      /* { void *_safe_tmp = call_expr; if (!_safe_tmp) return ENOMEM; var =
       * _safe_tmp; } */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      /* Use explicit size calc + sprintf_s or malloc+sprintf in C89 portable
         way? The codebase uses asprintf extension usually. Assuming
         asprintf/compatibility macro exists from str_utils context. */
      /* Wait, asprintf is provided by project macros. */
#endif
      /* Using asprintf for consistency as per previous project standards */
      /* However, strict C89 doesn't have it. We assume project provides it via
       * c89stringutils. */
#ifdef HAVE_ASPRINTF
    if (asprintf(&replacement,
                 "{ void *_safe_tmp = %s; if (!_safe_tmp) return %s; %s = "
                 "_safe_tmp; }",
                 call_expr, DEFAULT_ERROR_CODE, site->var_name) == -1) {
      free(call_expr);
      return ENOMEM;
    }
#else
    /* Fallback logic or assume HAVE_ASPRINTF is defined by build system */
    /* This sample assumes HAVE_ASPRINTF */
    replacement = malloc(strlen(call_expr) + strlen(site->var_name) + 128);
    if (replacement) {
      sprintf(replacement,
              "{ void *_safe_tmp = %s; if (!_safe_tmp) return %s; %s = "
              "_safe_tmp; }",
              call_expr, DEFAULT_ERROR_CODE, site->var_name);
    } else {
      free(call_expr);
      return ENOMEM;
    }
#endif
    free(call_expr);

    /* Replace the entire statement "p = realloc(...);" with the block */
    /* Range: [stmt_start, semi_idx + 1) (to include semicolon) */
    return patch_list_add(patches, stmt_start, semi_idx + 1, replacement);
  }

  return 0;
}

/* --- General Safety Injection --- */

int strategy_inject_safety_checks(const struct TokenList *const tokens,
                                  const struct AllocationSiteList *const allocs,
                                  struct PatchList *const patches) {
  size_t i;
  if (!tokens || !allocs || !patches)
    return EINVAL;

  for (i = 0; i < allocs->size; ++i) {
    size_t semi_idx;
    char *injection = NULL;
    const struct AllocationSite *site = &allocs->sites[i];

    if (site->is_checked)
      continue;

    semi_idx = find_next_token_idx(tokens, site->token_index, TOKEN_SEMICOLON);
    if (semi_idx >= tokens->size)
      continue;

    /* Dispatch realloc strategy */
    if (strcmp(site->spec->name, "realloc") == 0) {
      size_t old_size = patches->size;
      int rc = strategy_rewrite_realloc(tokens, site, semi_idx, patches);
      if (rc != 0)
        return rc;
      /* If matched and patched, skip standard injection logic */
      if (patches->size > old_size)
        continue;
    }

    if (!site->var_name)
      continue;

    /* Build check string */
    /* We inject AFTER the semicolon */
    if (site->spec->check_style == CHECK_PTR_NULL) {
      /* " if (!var) { return ENOMEM; }" */
      size_t len = strlen(site->var_name) + 40;
      injection = (char *)malloc(len);
      if (!injection)
        return ENOMEM;
      sprintf(injection, " if (!%s) { return %s; }", site->var_name,
              DEFAULT_ERROR_CODE);

    } else if (site->spec->check_style == CHECK_INT_NEGATIVE) {
      size_t len = strlen(site->var_name) + 40;
      injection = (char *)malloc(len);
      if (!injection)
        return ENOMEM;
      sprintf(injection, " if (%s < 0) { return %s; }", site->var_name,
              DEFAULT_ERROR_CODE);

    } else if (site->spec->check_style == CHECK_INT_NONZERO) {
      size_t len = strlen(site->var_name) + 40;
      injection = (char *)malloc(len);
      if (!injection)
        return ENOMEM;
      sprintf(injection, " if (%s != 0) { return %s; }", site->var_name,
              DEFAULT_ERROR_CODE);
    } else {
      /* Unknown -> skip */
      continue;
    }

    /* Add patch: Insert at semi_idx + 1 (Start==End for insertion) */
    if (patch_list_add(patches, semi_idx + 1, semi_idx + 1, injection) != 0) {
      /* patch_list_add frees injection on failure, but we rely on its behavior.
         Check implementation: Yes, it handles free on failure. */
      return ENOMEM;
    }
  }

  return 0;
}
