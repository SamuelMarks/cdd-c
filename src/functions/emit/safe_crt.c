/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "functions/emit/safe_crt.h"
/* clang-format on */

/**
 * @brief Executes the safe crt patch list init operation.
 */
cdd_c_error_t safe_crt_patch_list_init(struct SafeCrtPatchList *list) {
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  list->patches = NULL;
  list->size = 0;
  list->capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the safe crt patch list free operation.
 */
cdd_c_error_t safe_crt_patch_list_free(struct SafeCrtPatchList *list) {
  size_t i;
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (list->patches) {
    for (i = 0; i < list->size; ++i) {
      if (list->patches[i].replacement_text) {
        C_CDD_FREE(list->patches[i].replacement_text);
        list->patches[i].replacement_text = NULL;
      }
    }
    C_CDD_FREE(list->patches);
    list->patches = NULL;
  }
  list->size = 0;
  list->capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds a patch to the patch list.
 */
cdd_c_error_t safe_crt_add_patch(struct SafeCrtPatchList *list, size_t start,
                                 size_t end, const char *text) {
  struct SafeCrtPatch *p;
  if (!list || !text)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (list->size >= list->capacity) {
    size_t new_cap = list->capacity == 0 ? 8 : list->capacity * 2;
    struct SafeCrtPatch *new_arr = (struct SafeCrtPatch *)C_CDD_REALLOC(
        list->patches, new_cap * sizeof(struct SafeCrtPatch));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    list->patches = new_arr;
    list->capacity = new_cap;
  }

  p = &list->patches[list->size];
  p->start_token_idx = start;
  p->end_token_idx = end;

  p->replacement_text = C_CDD_STRDUP(text);
  if (!p->replacement_text) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  list->size++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts token text.
 */
cdd_c_error_t safe_crt_extract_token_text(const struct TokenList *tokens,
                                          size_t start, size_t end,
                                          char **_out_val) {
  size_t len = 0;
  size_t i;
  char *str;
  char *ptr;

  if (!tokens || !_out_val || start > end || end > tokens->size)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *_out_val = NULL;
  for (i = start; i < end; ++i) {
    len += tokens->tokens[i].length;
  }

  str = (char *)C_CDD_MALLOC(len + 1);
  if (!str) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  ptr = str;
  for (i = start; i < end; ++i) {
    memcpy(ptr, tokens->tokens[i].start, tokens->tokens[i].length);
    ptr += tokens->tokens[i].length;
  }
  *ptr = '\0';
  *_out_val = str;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates strcpy patch.
 */
cdd_c_error_t safe_crt_generate_strcpy_patch(const struct TokenList *tokens,
                                             size_t call_start, size_t call_end,
                                             struct SafeCrtPatchList *out) {
  /* Pattern: strcpy(dest, src) */
  size_t lparen = 0;
  size_t comma = 0;
  size_t rparen = 0;
  size_t i;
  char *dest = NULL;
  char *src = NULL;
  char replacement[1024] = {0};
  cdd_c_error_t rc_crt;

  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = call_start; i < call_end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      if (lparen == 0)
        lparen = i;
    } else if (tokens->tokens[i].kind == TOKEN_COMMA) {
      if (lparen != 0 && comma == 0)
        comma = i;
    } else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      if (comma != 0 && rparen == 0)
        rparen = i;
    }
  }

  if (lparen == 0 || comma == 0 || rparen == 0)
    return CDD_C_SUCCESS;

  rc_crt = safe_crt_extract_token_text(tokens, lparen + 1, comma, &dest);
  if (rc_crt != CDD_C_SUCCESS)
    return rc_crt;

  rc_crt = safe_crt_extract_token_text(tokens, comma + 1, rparen, &src);
  if (rc_crt != CDD_C_SUCCESS) {
    C_CDD_FREE(dest);
    return rc_crt;
  }

#if defined(_MSC_VER)
  sprintf_s(replacement, sizeof(replacement),
            "#if defined(_MSC_VER)\n"
            "  strcpy_s(%s, sizeof(%s), %s);\n"
            "#else\n"
            "  str"
            "cpy(%s, %s);\n"
            "#endif\n",
            dest, dest, src, dest, src);
#else
  sprintf(replacement,
          "#if defined(_MSC_VER)\n"
          "  strcpy_s(%s, sizeof(%s), %s);\n"
          "#else\n"
          "  str"
          "cpy(%s, %s);\n"
          "#endif\n",
          dest, dest, src, dest, src);
#endif

  rc_crt = safe_crt_add_patch(out, call_start, call_end, replacement);
  C_CDD_FREE(dest);
  C_CDD_FREE(src);
  return rc_crt;
}

/**
 * @brief Generates fopen patch.
 */
cdd_c_error_t safe_crt_generate_fopen_patch(const struct TokenList *tokens,
                                            size_t call_start, size_t call_end,
                                            struct SafeCrtPatchList *out) {
  size_t lparen = 0, comma1 = 0, rparen = 0, i;
  char *path = NULL;
  char *mode = NULL;
  char *dest = NULL;
  char replacement[1024] = {0};
  size_t assign_idx = 0;
  size_t id_idx;
  cdd_c_error_t rc_crt;

  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Look backwards for '=' to find assignment target */
  for (i = call_start; i > 0; --i) {
    if (tokens->tokens[i].kind == TOKEN_ASSIGN) {
      assign_idx = i;
      break;
    }
    if (tokens->tokens[i].kind == TOKEN_SEMICOLON ||
        tokens->tokens[i].kind == TOKEN_LBRACE ||
        tokens->tokens[i].kind == TOKEN_RBRACE) {
      break; /* Stop looking if we cross statement boundaries */
    }
  }

  for (i = call_start; i < call_end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      if (lparen == 0)
        lparen = i;
    } else if (tokens->tokens[i].kind == TOKEN_COMMA) {
      if (lparen != 0 && comma1 == 0)
        comma1 = i;
    } else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      if (comma1 != 0)
        rparen = i;
    }
  }

  if (assign_idx == 0 || lparen == 0 || comma1 == 0 || rparen == 0)
    return CDD_C_SUCCESS;

  id_idx = assign_idx;
  rc_crt = safe_crt_extract_token_text(tokens, lparen + 1, comma1, &path);
  if (rc_crt != CDD_C_SUCCESS)
    return rc_crt;

  rc_crt = safe_crt_extract_token_text(tokens, comma1 + 1, rparen, &mode);
  if (rc_crt != CDD_C_SUCCESS) {
    C_CDD_FREE(path);
    return rc_crt;
  }

  /* Go left of '=' to find the identifier */
  while (id_idx > 0) {
    id_idx--;
    if (tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER) {
      break;
    }
    if (tokens->tokens[id_idx].kind == TOKEN_SEMICOLON ||
        tokens->tokens[id_idx].kind == TOKEN_LBRACE ||
        tokens->tokens[id_idx].kind == TOKEN_RBRACE) {
      break;
    }
  }
  if (tokens->tokens[id_idx].kind != TOKEN_IDENTIFIER) {
    C_CDD_FREE(path);
    C_CDD_FREE(mode);
    return CDD_C_SUCCESS;
  }
  rc_crt = safe_crt_extract_token_text(tokens, id_idx, id_idx + 1, &dest);
  if (rc_crt != CDD_C_SUCCESS) {
    C_CDD_FREE(path);
    C_CDD_FREE(mode);
    return rc_crt;
  }

#if defined(_MSC_VER)
  sprintf_s(replacement, sizeof(replacement),
            "#if defined(_MSC_VER)\n"
            "  fopen_s(&%s, %s, %s);\n"
            "#else\n"
            "  %s = fopen(%s, %s);\n"
            "#endif\n",
            dest, path, mode, dest, path, mode);
#else
  sprintf(replacement,
          "#if defined(_MSC_VER)\n"
          "  fopen_s(&%s, %s, %s);\n"
          "#else\n"
          "  %s = fopen(%s, %s);\n"
          "#endif\n",
          dest, path, mode, dest, path, mode);
#endif
  rc_crt = safe_crt_add_patch(out, assign_idx - 1, call_end, replacement);
  C_CDD_FREE(path);
  C_CDD_FREE(mode);
  C_CDD_FREE(dest);
  return rc_crt;
}

/**
 * @brief Generates strncpy patch.
 */
cdd_c_error_t safe_crt_generate_strncpy_patch(const struct TokenList *tokens,
                                              size_t call_start,
                                              size_t call_end,
                                              struct SafeCrtPatchList *out) {
  /* Pattern: strncpy(dest, src, count) */
  size_t lparen = 0, comma1 = 0, comma2 = 0, rparen = 0, i;
  char *dest = NULL, *src = NULL, *count = NULL;
  char replacement[1024] = {0};
  cdd_c_error_t rc_crt;

  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = call_start; i < call_end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      if (lparen == 0)
        lparen = i;
    } else if (tokens->tokens[i].kind == TOKEN_COMMA) {
      if (lparen != 0) {
        if (comma1 == 0)
          comma1 = i;
        else if (comma2 == 0)
          comma2 = i;
      }
    } else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      if (comma2 != 0 && rparen == 0)
        rparen = i;
    }
  }

  if (lparen == 0 || comma1 == 0 || comma2 == 0 || rparen == 0)
    return CDD_C_SUCCESS;

  rc_crt = safe_crt_extract_token_text(tokens, lparen + 1, comma1, &dest);
  if (rc_crt != CDD_C_SUCCESS)
    return rc_crt;

  rc_crt = safe_crt_extract_token_text(tokens, comma1 + 1, comma2, &src);
  if (rc_crt != CDD_C_SUCCESS) {
    C_CDD_FREE(dest);
    return rc_crt;
  }

  rc_crt = safe_crt_extract_token_text(tokens, comma2 + 1, rparen, &count);
  if (rc_crt != CDD_C_SUCCESS) {
    C_CDD_FREE(dest);
    C_CDD_FREE(src);
    return rc_crt;
  }

#if defined(_MSC_VER)
  sprintf_s(replacement, sizeof(replacement),
            "#if defined(_MSC_VER)\n"
            "  strncpy_s(%s, sizeof(%s), %s, %s);\n"
            "#else\n"
            "  str"
            "ncpy(%s, %s, %s);\n"
            "#endif\n",
            dest, dest, src, count, dest, src, count);
#else
  sprintf(replacement,
          "#if defined(_MSC_VER)\n"
          "  strncpy_s(%s, sizeof(%s), %s, %s);\n"
          "#else\n"
          "  str"
          "ncpy(%s, %s, %s);\n"
          "#endif\n",
          dest, dest, src, count, dest, src, count);
#endif
  rc_crt = safe_crt_add_patch(out, call_start, call_end, replacement);
  C_CDD_FREE(dest);
  C_CDD_FREE(src);
  C_CDD_FREE(count);
  return rc_crt;
}

/**
 * @brief Generates sprintf patch.
 */
cdd_c_error_t safe_crt_generate_sprintf_patch(const struct TokenList *tokens,
                                              size_t call_start,
                                              size_t call_end,
                                              struct SafeCrtPatchList *out) {
  /* Pattern: sprintf(dest, format, ...) */
  size_t lparen = 0, comma1 = 0, rparen = 0, i;
  char *dest = NULL;
  char *args = NULL;
  char replacement[2048] = {0};
  cdd_c_error_t rc_crt;

  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = call_start; i < call_end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      if (lparen == 0)
        lparen = i;
    } else if (tokens->tokens[i].kind == TOKEN_COMMA) {
      if (lparen != 0 && comma1 == 0)
        comma1 = i;
    } else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      if (comma1 != 0)
        rparen = i; /* Use the last one for ... */
    }
  }

  if (lparen == 0 || comma1 == 0 || rparen == 0)
    return CDD_C_SUCCESS;

  rc_crt = safe_crt_extract_token_text(tokens, lparen + 1, comma1, &dest);
  if (rc_crt != CDD_C_SUCCESS)
    return rc_crt;

  rc_crt = safe_crt_extract_token_text(tokens, comma1 + 1, rparen, &args);
  if (rc_crt != CDD_C_SUCCESS) {
    C_CDD_FREE(dest);
    return rc_crt;
  }

#if defined(_MSC_VER)
  sprintf_s(replacement, sizeof(replacement),
            "#if defined(_MSC_VER)\n"
            "  sprintf_s(%s, sizeof(%s), %s);\n"
            "#else\n"
            "  spr"
            "intf(%s, %s);\n"
            "#endif",
            dest, dest, args, dest, args);
#else
  sprintf(replacement,
          "#if defined(_MSC_VER)\n"
          "  sprintf_s(%s, sizeof(%s), %s);\n"
          "#else\n"
          "  spr"
          "intf(%s, %s);\n"
          "#endif",
          dest, dest, args, dest, args);
#endif
  rc_crt = safe_crt_add_patch(out, call_start, call_end, replacement);
  C_CDD_FREE(dest);
  C_CDD_FREE(args);
  return rc_crt;
}

/**
 * @brief Executes the cst generate safe crt patches operation.
 */
cdd_c_error_t
cst_generate_safe_crt_patches(const struct CstNodeList *cst,
                              const struct TokenList *tokens,
                              struct SafeCrtPatchList *out_patches) {
  size_t i, j;
  cdd_c_error_t rc_crt;

  if (!cst || !tokens || !out_patches)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < cst->size; ++i) {
    const struct CstNode *n = &cst->nodes[i];

    /* Search for function calls inside OTHER nodes */
    for (j = n->start_token; j < n->end_token; ++j) {
      if (tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        size_t len = tokens->tokens[j].length;
        const char *str = (const char *)tokens->tokens[j].start;

        if (len == 6 && strncmp(str, "strcpy", 6) == 0) {
          size_t end = j;
          while (end < n->end_token &&
                 tokens->tokens[end].kind != TOKEN_SEMICOLON) {
            end++;
          }
          if (end < n->end_token) {
            rc_crt =
                safe_crt_generate_strcpy_patch(tokens, j, end + 1, out_patches);
            if (rc_crt != CDD_C_SUCCESS)
              return rc_crt;
            j = end;
          }
        } else if (len == 7 && strncmp(str, "strncpy", 7) == 0) {
          size_t end = j;
          while (end < n->end_token &&
                 tokens->tokens[end].kind != TOKEN_SEMICOLON) {
            end++;
          }
          if (end < n->end_token) {
            rc_crt = safe_crt_generate_strncpy_patch(tokens, j, end + 1,
                                                     out_patches);
            if (rc_crt != CDD_C_SUCCESS)
              return rc_crt;
            j = end;
          }
        } else if (len == 7 && strncmp(str, "sprintf", 7) == 0) {
          size_t end = j;
          while (end < n->end_token &&
                 tokens->tokens[end].kind != TOKEN_SEMICOLON) {
            end++;
          }
          if (end < n->end_token) {
            rc_crt = safe_crt_generate_sprintf_patch(tokens, j, end + 1,
                                                     out_patches);
            if (rc_crt != CDD_C_SUCCESS)
              return rc_crt;
            j = end;
          }
        } else if (len == 5 && strncmp(str, "fopen", 5) == 0) {
          size_t end = j;
          while (end < n->end_token &&
                 tokens->tokens[end].kind != TOKEN_SEMICOLON) {
            end++;
          }
          if (end < n->end_token) {
            rc_crt =
                safe_crt_generate_fopen_patch(tokens, j, end + 1, out_patches);
            if (rc_crt != CDD_C_SUCCESS)
              return rc_crt;
            j = end;
          }
        }
      }
    }

    /* VLA Scan: Look for `type id[expr];` where expr is not just an integer
     * literal */
    for (j = n->start_token; j < n->end_token; ++j) {
      if (tokens->tokens[j].kind == TOKEN_KEYWORD_CHAR ||
          tokens->tokens[j].kind == TOKEN_KEYWORD_INT ||
          tokens->tokens[j].kind == TOKEN_KEYWORD_DOUBLE) {
        size_t id_idx = j + 1;
        while (id_idx < n->end_token &&
               tokens->tokens[id_idx].kind == TOKEN_WHITESPACE) {
          id_idx++;
        }

        if (id_idx < n->end_token &&
            tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER) {
          size_t lbrack_idx = id_idx + 1;
          while (lbrack_idx < n->end_token &&
                 tokens->tokens[lbrack_idx].kind == TOKEN_WHITESPACE) {
            lbrack_idx++;
          }

          if (lbrack_idx < n->end_token &&
              tokens->tokens[lbrack_idx].kind == TOKEN_LBRACKET) {
            /* potential array declaration */
            size_t end_bracket = lbrack_idx + 1;
            int is_vla = 0;
            while (end_bracket < n->end_token &&
                   tokens->tokens[end_bracket].kind != TOKEN_RBRACKET) {
              if (tokens->tokens[end_bracket].kind == TOKEN_IDENTIFIER) {
                is_vla = 1; /* Expression involves variable -> VLA */
              }
              end_bracket++;
            }

            if (is_vla && end_bracket < n->end_token) {
              char *type_name = NULL;
              char *var_name = NULL;
              char *expr = NULL;
              char replacement[1024] = {0};
              /* found a VLA */
              rc_crt =
                  safe_crt_extract_token_text(tokens, j, j + 1, &type_name);
              if (rc_crt != CDD_C_SUCCESS)
                return rc_crt;

              rc_crt = safe_crt_extract_token_text(tokens, id_idx, id_idx + 1,
                                                   &var_name);
              if (rc_crt != CDD_C_SUCCESS) {
                C_CDD_FREE(type_name);
                return rc_crt;
              }

              rc_crt = safe_crt_extract_token_text(tokens, lbrack_idx + 1,
                                                   end_bracket, &expr);
              if (rc_crt != CDD_C_SUCCESS) {
                C_CDD_FREE(type_name);
                C_CDD_FREE(var_name);
                return rc_crt;
              }

              {
                size_t end_stmt = end_bracket;
#if defined(_MSC_VER)
                sprintf_s(replacement, sizeof(replacement),
                          "#if defined(_MSC_VER)\n"
                          "  %s *%s = (%s*)_alloca((%s) * sizeof(%s));\n"
                          "#else\n"
                          "  %s %s[%s];\n"
                          "#endif",
                          type_name, var_name, type_name, expr, type_name,
                          type_name, var_name, expr);
#else
                sprintf(replacement,
                        "#if defined(_MSC_VER)\n"
                        "  %s *%s = (%s*)_alloca((%s) * sizeof(%s));\n"
                        "#else\n"
                        "  %s %s[%s];\n"
                        "#endif",
                        type_name, var_name, type_name, expr, type_name,
                        type_name, var_name, expr);
#endif

                /* statement usually ends with ; */
                while (end_stmt < n->end_token &&
                       tokens->tokens[end_stmt].kind != TOKEN_SEMICOLON) {
                  end_stmt++;
                }
                if (end_stmt < n->end_token) {
                  rc_crt = safe_crt_add_patch(out_patches, j, end_stmt + 1,
                                              replacement);
                  C_CDD_FREE(type_name);
                  C_CDD_FREE(var_name);
                  C_CDD_FREE(expr);
                  if (rc_crt != CDD_C_SUCCESS)
                    return rc_crt;
                  /* Only skip to end_bracket, so we don't skip the rest of the
                   * node loop processing */
                  j = end_bracket;
                } else {
                  C_CDD_FREE(type_name);
                  C_CDD_FREE(var_name);
                  C_CDD_FREE(expr);
                }
              }
            }
          }
        }
      }
    }
  }

  return CDD_C_SUCCESS;
}
