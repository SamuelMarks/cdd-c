/**
 * @file cdd_cst_type_eval.c
 * @brief Concrete Syntax Tree (CST) type eval implementation and utilities.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_type_eval.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/*
 * @brief Evaluates the byte size and alignment of a C primitive type under a
 * target ABI model.
 *
 * @param[in] type_name Canonical C primitive type name string.
 * @param[in] abi Target ABI data model (ILP32, LLP64, LP64).
 * @param[out] out_info Output structure receiving size and alignment in bytes.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if unknown, or
 * CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_cst_eval_primitive_type(const char *type_name,
                                          enum cdd_cst_abi_model_t abi,
                                          cdd_cst_type_info_t *out_info) {
  C_CDD_LOG_DEBUG("eval_primitive_type called with: %s", type_name);
  if (!type_name || !out_info)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (strcmp(type_name, "char") == 0 || strcmp(type_name, "signed char") == 0 ||
      strcmp(type_name, "unsigned char") == 0) {
    out_info->size = 1;
    out_info->alignment = 1;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "short") == 0 ||
      strcmp(type_name, "unsigned short") == 0) {
    out_info->size = 2;
    out_info->alignment = 2;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "int") == 0 || strcmp(type_name, "unsigned int") == 0) {
    out_info->size = 4;
    out_info->alignment = 4;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "long") == 0 ||
      strcmp(type_name, "unsigned long") == 0) {
    if (abi == CDD_CST_ABI_ILP32 || abi == CDD_CST_ABI_LLP64) {
      out_info->size = 4;
      out_info->alignment = 4;
    } else {
      out_info->size = 8;
      out_info->alignment = 8;
    }
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "long long") == 0 ||
      strcmp(type_name, "unsigned long long") == 0) {
    out_info->size = 8;
    out_info->alignment = 8;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "float") == 0) {
    out_info->size = 4;
    out_info->alignment = 4;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "double") == 0) {
    out_info->size = 8;
    out_info->alignment = 8;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "long double") == 0) {
    if (abi == CDD_CST_ABI_ILP32) {
      out_info->size = 8;
      out_info->alignment = 4;
    } else if (abi == CDD_CST_ABI_LLP64) {
      out_info->size = 8;
      out_info->alignment = 8;
    } else {
      out_info->size = 16;
      out_info->alignment = 16;
    }
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "void *") == 0 || strcmp(type_name, "ptr") == 0) {
    if (abi == CDD_CST_ABI_ILP32) {
      out_info->size = 4;
      out_info->alignment = 4;
    } else {
      out_info->size = 8;
      out_info->alignment = 8;
    }
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "__int128") == 0 ||
      strcmp(type_name, "unsigned __int128") == 0 ||
      strcmp(type_name, "cdd_int128_t") == 0 ||
      strcmp(type_name, "cdd_uint128_t") == 0) {
    out_info->size = 16;
    out_info->alignment = 16;
    return CDD_C_SUCCESS;
  }

  return CDD_C_ERROR_NOT_FOUND;
}

/**
 * @brief Resolves a type name through `CDD_CST_SYMBOL_TYPEDEF` chains in `env`
 * and evaluates primitive/pointer ABI info.
 *
 * @param[in] env Non-null active scope environment.
 * @param[in] type_name Non-null type name to evaluate.
 * @param[in] is_pointer Non-zero if explicit pointer declarator was present.
 * @param[in] abi Target ABI model.
 * @param[out] out_info Evaluated size and alignment.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t eval_named_or_typedef(cdd_cst_scope_env_t *env,
                                           const char *type_name,
                                           int is_pointer,
                                           enum cdd_cst_abi_model_t abi,
                                           cdd_cst_type_info_t *out_info) {
  const char *cur = type_name;
  int hops = 0;

  if (is_pointer) {
    return cdd_cst_eval_primitive_type("ptr", abi, out_info);
  }
  while (hops < 8) {
    size_t len = strlen(cur);
    cdd_cst_symbol_t *sym = NULL;
    if (cur[len - 1] == '*') {
      return cdd_cst_eval_primitive_type("ptr", abi, out_info);
    }
    if (cdd_cst_eval_primitive_type(cur, abi, out_info) == CDD_C_SUCCESS) {
      return CDD_C_SUCCESS;
    }
    if (cdd_cst_scope_lookup_symbol(env, cur, CDD_CST_SYMBOL_TYPEDEF, &sym) ==
            CDD_C_SUCCESS &&
        sym->kind == CDD_CST_SYMBOL_TYPEDEF && sym->type_str) {
      cur = sym->type_str;
      hops++;
      continue;
    }
    break;
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/**
 * @brief Extracts the normalized type name and pointer indicator from a CST
 * type node.
 *
 * @param[in] node CST node representing the type specifier.
 * @param[out] out_name Pointer to receive the allocated type name string.
 * @param[out] is_pointer Pointer to receive 1 if pointer declarator `*` is
 * present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t extract_type_name(cdd_cst_node_t *node, char **out_name,
                                       int *is_pointer) {
  size_t i;
  char buf[256];
  size_t buf_len = 0;
  int ptr_count = 0;

  buf[0] = '\0';
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = node->children[i].val.token;
      if (tok->kind == CDD_TOKEN_STAR) {
        ptr_count++;
      } else if (tok->kind == CDD_TOKEN_IDENTIFIER ||
                 tok->kind == CDD_TOKEN_KEYWORD_INT ||
                 tok->kind == CDD_TOKEN_KEYWORD___INT128) {
        if (buf_len > 0 && buf_len + 1 < sizeof(buf)) {
          buf[buf_len++] = ' ';
          buf[buf_len] = '\0';
        }
        if (buf_len + tok->length < sizeof(buf)) {
          memcpy(buf + buf_len, tok->start, tok->length);
          buf_len += tok->length;
          buf[buf_len] = '\0';
        }
      }
    }
  }

  *is_pointer = (ptr_count > 0) ? 1 : 0;

  if (buf_len > 0) {
    char *ret = (char *)C_CDD_MALLOC(buf_len + 1);
    if (!ret) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM");
      return CDD_C_ERROR_MEMORY;
    }
    memcpy(ret, buf, buf_len + 1);
    *out_name = ret;
    return CDD_C_SUCCESS;
  }

  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Evaluates `sizeof` for a CST type node under the specified ABI model.
 *
 * @param[in] env Active lexical scope environment.
 * @param[in] type_node CST node representing the type specifier.
 * @param[in] abi Target ABI data model.
 * @param[out] out_size Output pointer receiving the evaluated byte size.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_eval_sizeof(cdd_cst_scope_env_t *env,
                                  cdd_cst_node_t *type_node,
                                  enum cdd_cst_abi_model_t abi,
                                  size_t *out_size) {
  char *name = NULL;
  int is_pointer = 0;
  cdd_c_error_t rc;
  cdd_cst_type_info_t info;

  if (!env || !type_node || !out_size)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = extract_type_name(type_node, &name, &is_pointer);
  if (rc != CDD_C_SUCCESS) {
    *out_size = 0;
    return rc;
  }

  rc = eval_named_or_typedef(env, name, is_pointer, abi, &info);
  C_CDD_FREE(name);
  if (rc != CDD_C_SUCCESS) {
    *out_size = 0;
    return rc;
  }

  *out_size = info.size;
  return CDD_C_SUCCESS;
}

/*
 * @brief Evaluates `alignof` for a CST type node under the specified ABI model.
 *
 * @param[in] env Active lexical scope environment.
 * @param[in] type_node CST node representing the type specifier.
 * @param[in] abi Target ABI data model.
 * @param[out] out_align Output pointer receiving the evaluated byte alignment.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_eval_alignof(cdd_cst_scope_env_t *env,
                                   cdd_cst_node_t *type_node,
                                   enum cdd_cst_abi_model_t abi,
                                   size_t *out_align) {
  char *name = NULL;
  int is_pointer = 0;
  cdd_c_error_t rc;
  cdd_cst_type_info_t info;

  if (!env || !type_node || !out_align)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = extract_type_name(type_node, &name, &is_pointer);
  if (rc != CDD_C_SUCCESS) {
    *out_align = 0;
    return rc;
  }

  rc = eval_named_or_typedef(env, name, is_pointer, abi, &info);
  C_CDD_FREE(name);
  if (rc != CDD_C_SUCCESS) {
    *out_align = 0;
    return rc;
  }

  *out_align = info.alignment;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `tok` matches any string in `table[0..count)`.
 *
 * @param[in] tok Token to compare.
 * @param[in] table Array of null-terminated strings.
 * @param[in] count Number of entries in `table`.
 * @param[out] out_eq Receives 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t te_tok_in_table(const cdd_token_t *tok,
                                     const char *const *table, size_t count,
                                     int *out_eq) {
  size_t i;
  *out_eq = 0;
  for (i = 0; i < count; i++) {
    size_t l = strlen(table[i]);
    if (tok->length == l && memcmp(tok->start, table[i], l) == 0) {
      *out_eq = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Copies a null-terminated string safely into `dst[0..cap)`.
 *
 * @param[out] dst Non-null destination buffer (`cap > 0`).
 * @param[in] cap Buffer capacity.
 * @param[in] src Non-null source string.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t te_copy_str(char *dst, size_t cap, const char *src) {
  size_t len = strlen(src);
  if (len >= cap)
    len = cap - 1;
  memcpy(dst, src, len);
  dst[len] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Normalizes a C type string according to `mode` (stripping top-level
 * qualifiers and/or decaying array `[N]` bounds to `*`).
 *
 * @param[in,out] type_buf Buffer containing the C type string to normalize.
 * @param[in] cap Capacity of `type_buf`.
 * @param[in] mode Evaluation mode (`cdd_cst_type_eval_mode_t`).
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t te_normalize_type_mode(char *type_buf, size_t cap,
                                            int mode) {
  static const char *const k_cvr_pre[] = {"const ", "volatile ", "restrict ",
                                          "__restrict "};
  size_t len;
  if (mode == CDD_CST_TYPE_MODE_EXACT)
    return CDD_C_SUCCESS;

  len = strlen(type_buf);
  if (mode == CDD_CST_TYPE_MODE_RVALUE && len > 2 && type_buf[len - 1] == ']') {
    char *lb = strchr(type_buf, '[');
    if (lb && (size_t)(lb - type_buf) + 3 < cap) {
      size_t p_len = (size_t)(lb - type_buf);
      type_buf[p_len] = ' ';
      type_buf[p_len + 1] = '*';
      type_buf[p_len + 2] = '\0';
    }
  }

  /* Strip top-level cvr qualifiers only when not qualifying a pointee */
  if (strchr(type_buf, '*') == NULL) {
    int changed = 1;
    while (changed) {
      size_t q;
      changed = 0;
      for (q = 0; q < sizeof(k_cvr_pre) / sizeof(k_cvr_pre[0]); q++) {
        size_t ql = strlen(k_cvr_pre[q]);
        if (strncmp(type_buf, k_cvr_pre[q], ql) == 0) {
          memmove(type_buf, type_buf + ql, strlen(type_buf + ql) + 1);
          changed = 1;
          break;
        }
      }
    }
  } else if (strlen(type_buf) > 6 &&
             strcmp(type_buf + strlen(type_buf) - 6, " const") == 0) {
    type_buf[strlen(type_buf) - 6] = '\0';
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `tokens[0..n)` forms a valid C type-name (`int`, `char *`,
 * `struct S`, `my_typedef_t`, `int[4]`) rather than a value expression.
 *
 * @param[in] scope Active lexical scope for typedef lookup.
 * @param[in] tokens Token pointer slice (`n > 0`).
 * @param[in] n Number of tokens.
 * @param[out] out_type Buffer receiving the formatted type if matched.
 * @param[in] cap Buffer capacity.
 * @param[out] out_matched Receives 1 if `tokens` is a type name, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t te_try_parse_type_name(const cdd_cst_scope_t *scope,
                                            const cdd_token_t *const *tokens,
                                            size_t n, char *out_type,
                                            size_t cap, int *out_matched) {
  static const char *const k_type_words[] = {
      "void",         "char",         "short",    "int",       "long",
      "float",        "double",       "signed",   "unsigned",  "_Bool",
      "bool",         "size_t",       "ssize_t",  "ptrdiff_t", "intptr_t",
      "uintptr_t",    "int8_t",       "int16_t",  "int32_t",   "int64_t",
      "uint8_t",      "uint16_t",     "uint32_t", "uint64_t",  "__int128",
      "cdd_int128_t", "cdd_uint128_t"};
  static const char *const k_quals[] = {"const", "volatile", "restrict",
                                        "__restrict", "__const"};
  static const char *const k_aggr[] = {"union", "enum"};
  size_t i = 0;
  int has_noun = 0;
  size_t blen = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  *out_matched = 0;
  out_type[0] = '\0';

  while (i < n) {
    const cdd_token_t *t = tokens[i];
    int m = 0;

    rc = (cdd_c_error_t)((int)rc | (int)te_tok_in_table(
                                       t, k_aggr,
                                       sizeof(k_aggr) / sizeof(k_aggr[0]), &m));
    if (t->kind == CDD_TOKEN_KEYWORD_STRUCT || m) {
      if (i + 1 >= n || tokens[i + 1]->kind != CDD_TOKEN_IDENTIFIER)
        return rc;
      if (blen > 0 && blen + 1 < cap)
        out_type[blen++] = ' ';
      if (blen + t->length + 1 + tokens[i + 1]->length < cap) {
        memcpy(out_type + blen, t->start, t->length);
        blen += t->length;
        out_type[blen++] = ' ';
        memcpy(out_type + blen, tokens[i + 1]->start, tokens[i + 1]->length);
        blen += tokens[i + 1]->length;
        out_type[blen] = '\0';
      }
      has_noun = 1;
      i += 2;
      continue;
    }

    if (t->kind == CDD_TOKEN_KEYWORD_INT ||
        t->kind == CDD_TOKEN_KEYWORD___INT128 ||
        t->kind == CDD_TOKEN_IDENTIFIER) {
      int matched_word = 0;
      rc = (cdd_c_error_t)((int)rc |
                           (int)te_tok_in_table(
                               t, k_quals, sizeof(k_quals) / sizeof(k_quals[0]),
                               &matched_word));
      if (!matched_word) {
        rc = (cdd_c_error_t)((int)rc |
                             (int)te_tok_in_table(t, k_type_words,
                                                  sizeof(k_type_words) /
                                                      sizeof(k_type_words[0]),
                                                  &matched_word));
        if (matched_word)
          has_noun = 1;
      }
      if (!matched_word && !has_noun && scope && t->length < 120) {
        char idbuf[128];
        cdd_cst_symbol_t *tsym = NULL;
        memcpy(idbuf, t->start, t->length);
        idbuf[t->length] = '\0';
        if (cdd_cst_scope_lookup_in_scope(scope, idbuf, CDD_CST_SYMBOL_TYPEDEF,
                                          &tsym) == CDD_C_SUCCESS &&
            tsym->kind == CDD_CST_SYMBOL_TYPEDEF) {
          matched_word = 1;
          has_noun = 1;
        }
      }
      if (matched_word) {
        if (blen > 0 && blen + 1 < cap)
          out_type[blen++] = ' ';
        if (blen + t->length < cap) {
          memcpy(out_type + blen, t->start, t->length);
          blen += t->length;
          out_type[blen] = '\0';
        }
        i++;
        continue;
      }
    }

    if (has_noun && t->kind == CDD_TOKEN_STAR) {
      if (blen > 0 && out_type[blen - 1] != '*' && blen + 1 < cap)
        out_type[blen++] = ' ';
      if (blen + 1 < cap) {
        out_type[blen++] = '*';
        out_type[blen] = '\0';
      }
      i++;
      continue;
    }

    if (has_noun && t->kind == CDD_TOKEN_LBRACKET) {
      while (i < n) {
        if (blen + tokens[i]->length < cap) {
          memcpy(out_type + blen, tokens[i]->start, tokens[i]->length);
          blen += tokens[i]->length;
          out_type[blen] = '\0';
        }
        if (tokens[i]->kind == CDD_TOKEN_RBRACKET) {
          i++;
          break;
        }
        i++;
      }
      continue;
    }

    return rc;
  }

  if (has_noun)
    *out_matched = 1;
  return rc;
}

/**
 * @brief Evaluates the type of a single numeric literal token.
 *
 * @param[in] tok Numeric literal token.
 * @param[out] out_type Destination buffer.
 * @param[in] cap Buffer capacity.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t te_eval_number_literal(const cdd_token_t *tok,
                                            char *out_type, size_t cap) {
  const char *s = (const char *)tok->start;
  size_t len = tok->length;
  size_t j;
  int is_hex = (len > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'));
  int is_flt = 0;
  int has_u = 0;
  int l_cnt = 0;

  for (j = 0; j < len; j++) {
    char c = s[j];
    if (c == '.' || (is_hex && (c == 'p' || c == 'P')) ||
        (!is_hex && (c == 'e' || c == 'E'))) {
      is_flt = 1;
    }
  }
  if (is_flt) {
    char last = s[len - 1];
    if (last == 'f' || last == 'F')
      return te_copy_str(out_type, cap, "float");
    if (last == 'l' || last == 'L')
      return te_copy_str(out_type, cap, "long double");
    return te_copy_str(out_type, cap, "double");
  }
  for (j = len; j > 0; j--) {
    char c = s[j - 1];
    if (c == 'u' || c == 'U')
      has_u = 1;
    else if (c == 'l' || c == 'L')
      l_cnt++;
    else
      break;
  }
  if (l_cnt >= 2)
    return te_copy_str(out_type, cap,
                       has_u ? "unsigned long long" : "long long");
  if (l_cnt == 1)
    return te_copy_str(out_type, cap, has_u ? "unsigned long" : "long");
  return te_copy_str(out_type, cap, has_u ? "unsigned int" : "int");
}

/**
 * @brief Ranks arithmetic types for C usual arithmetic conversions.
 *
 * @param[in] t Canonical C type string.
 * @param[out] out_rank Numeric conversion rank.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t te_arith_rank(const char *t, int *out_rank) {
  static const struct {
    const char *name;
    int rank;
  } k_ranks[] = {
      {"long double", 100},  {"double", 90},        {"float", 80},
      {"cdd_uint128_t", 75}, {"cdd_int128_t", 70},  {"unsigned long long", 65},
      {"uint64_t", 65},      {"long long", 60},     {"int64_t", 60},
      {"size_t", 55},        {"unsigned long", 55}, {"long", 50},
      {"unsigned int", 45}};
  size_t i;
  *out_rank = 40;
  for (i = 0; i < sizeof(k_ranks) / sizeof(k_ranks[0]); i++) {
    if (strcmp(t, k_ranks[i].name) == 0) {
      *out_rank = k_ranks[i].rank;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves member access (`cur_type.field` or `cur_type->field`) using
 * struct/union layouts in `env` / `scope`.
 *
 * @param[in] env Scope environment.
 * @param[in] scope Active scope.
 * @param[in,out] cur_type Buffer holding current base type; updated to member
 * type on match.
 * @param[in] cap Buffer capacity.
 * @param[in] field_tok Identifier token of member field.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t te_resolve_member(const cdd_cst_scope_env_t *env,
                                       const cdd_cst_scope_t *scope,
                                       char *cur_type, size_t cap,
                                       const cdd_token_t *field_tok) {
  char work[160];
  char fname[128];
  size_t flen;
  int hops = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!env || field_tok->length >= 127)
    return te_copy_str(cur_type, cap, "int");

  flen = field_tok->length;
  memcpy(fname, field_tok->start, flen);
  fname[flen] = '\0';
  rc =
      (cdd_c_error_t)((int)rc | (int)te_copy_str(work, sizeof(work), cur_type));

  while (hops < 8) {
    size_t wl = strlen(work);
    while (wl > 0 && (work[wl - 1] == '*' || work[wl - 1] == ' '))
      work[--wl] = '\0';
    if (strncmp(work, "const ", 6) == 0)
      memmove(work, work + 6, strlen(work + 6) + 1);

    if (strncmp(work, "struct ", 7) == 0 || strncmp(work, "union ", 6) == 0) {
      int is_un = (strncmp(work, "union ", 6) == 0);
      const char *tag = work + (is_un ? 6 : 7);
      enum cdd_cst_symbol_kind_t tk =
          is_un ? CDD_CST_SYMBOL_UNION_TAG : CDD_CST_SYMBOL_STRUCT_TAG;
      cdd_cst_symbol_t *tsym = NULL;
      if (cdd_cst_scope_lookup_in_scope(scope, tag, tk, &tsym) ==
          CDD_C_SUCCESS) {
        cdd_cst_field_info_t *fi = tsym->fields;
        while (fi) {
          if (strcmp(fi->name, fname) == 0) {
            return te_copy_str(cur_type, cap, fi->type_str);
          }
          fi = fi->next;
        }
      }
      break;
    } else {
      cdd_cst_symbol_t *td = NULL;
      if (cdd_cst_scope_lookup_in_scope(scope, work, CDD_CST_SYMBOL_TYPEDEF,
                                        &td) == CDD_C_SUCCESS &&
          td->kind == CDD_CST_SYMBOL_TYPEDEF && td->type_str) {
        rc = (cdd_c_error_t)((int)rc | (int)te_copy_str(work, sizeof(work),
                                                        td->type_str));
        hops++;
        continue;
      }
      break;
    }
  }
  return te_copy_str(cur_type, cap, "int");
}

/**
 * @brief Strips one pointer/array indirection level from `type_buf`.
 *
 * @param[in,out] type_buf Canonical C type buffer.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t te_deref_type(char *type_buf) {
  size_t len = strlen(type_buf);
  if (len > 2 && type_buf[len - 1] == ']') {
    char *lb = strrchr(type_buf, '[');
    if (lb)
      *lb = '\0';
    return CDD_C_SUCCESS;
  }
  if (len > 0 && type_buf[len - 1] == '*') {
    type_buf[--len] = '\0';
    while (len > 0 && type_buf[len - 1] == ' ')
      type_buf[--len] = '\0';
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursive expression type evaluator on `tokens[0..n)`.
 *
 * @param[in] env Optional semantic scope environment.
 * @param[in] scope Active lexical scope.
 * @param[in] tokens Slice of token pointers.
 * @param[in] n Number of tokens in `tokens`.
 * @param[out] out_type Buffer receiving evaluated C type.
 * @param[in] cap Buffer capacity.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t te_eval_expr_rec(const cdd_cst_scope_env_t *env,
                                      const cdd_cst_scope_t *scope,
                                      const cdd_token_t *const *tokens,
                                      size_t n, char *out_type, size_t cap) {
  static const char *const k_sizeof_ops[] = {"sizeof", "_Alignof",
                                             "__alignof__", "__alignof"};
  static const char *const k_voidp_fns[] = {
      "malloc",           "calloc", "realloc", "alloca", "_alloca",
      "__builtin_alloca", "memcpy", "memmove", "memset"};
  static const char *const k_charp_fns[] = {"strdup",  "_strdup", "strchr",
                                            "strrchr", "strstr",  "getenv"};
  size_t i;
  int depth;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (n == 0)
    return te_copy_str(out_type, cap, "void");

  /* 1. Unwrap balanced outer parentheses `( expr )` or C cast `(Type)expr` */
  if (tokens[0]->kind == CDD_TOKEN_LPAREN) {
    int d = 0;
    size_t match_rp = n;
    for (i = 0; i < n; i++) {
      if (tokens[i]->kind == CDD_TOKEN_LPAREN)
        d++;
      else if (tokens[i]->kind == CDD_TOKEN_RPAREN) {
        d--;
        if (d == 0) {
          match_rp = i;
          break;
        }
      }
    }
    if (match_rp == n - 1) {
      return te_eval_expr_rec(env, scope, tokens + 1, n - 2, out_type, cap);
    }
    if (match_rp < n - 1 && match_rp > 1) {
      char cast_t[160];
      int is_tn = 0;
      rc = (cdd_c_error_t)((int)rc | (int)te_try_parse_type_name(
                                         scope, tokens + 1, match_rp - 1,
                                         cast_t, sizeof(cast_t), &is_tn));
      if (is_tn)
        return te_copy_str(out_type, cap, cast_t);
    }
  }

  /* 2. Top-level comma, assignment, ternary `?:`, logical, and binary ops */
  depth = 0;
  {
    size_t last_comma = n;
    size_t q_idx = n;
    size_t colon_idx = n;
    size_t assign_idx = n;
    size_t logic_idx = n;
    size_t bin_idx = n;
    enum cdd_token_kind_t bin_op = CDD_TOKEN_EOF;

    for (i = 0; i < n; i++) {
      const cdd_token_t *ti = tokens[i];
      enum cdd_token_kind_t k = ti->kind;
      char ch = ti->length == 1 ? (char)ti->start[0] : '\0';
      char nxt_ch = (i + 1 < n && tokens[i + 1]->length == 1)
                        ? (char)tokens[i + 1]->start[0]
                        : '\0';

      if (k == CDD_TOKEN_LPAREN || k == CDD_TOKEN_LBRACKET ||
          k == CDD_TOKEN_LBRACE)
        depth++;
      else if ((k == CDD_TOKEN_RPAREN || k == CDD_TOKEN_RBRACKET ||
                k == CDD_TOKEN_RBRACE) &&
               depth > 0)
        depth--;
      else if (depth == 0) {
        if (k == CDD_TOKEN_COMMA) {
          last_comma = i;
        } else if (ch == '?' && q_idx == n) {
          q_idx = i;
        } else if (k == CDD_TOKEN_COLON && q_idx < n && colon_idx == n) {
          colon_idx = i;
        } else if (k == CDD_TOKEN_EQ || k == CDD_TOKEN_NEQ ||
                   (ch == '|' && nxt_ch == '|') ||
                   (ch == '&' && nxt_ch == '&')) {
          logic_idx = i;
          if (nxt_ch == '|' || nxt_ch == '&')
            i++;
        } else if ((k == CDD_TOKEN_LT && nxt_ch == '<') ||
                   (k == CDD_TOKEN_GT && nxt_ch == '>')) {
          if (i + 2 < n && tokens[i + 2]->kind == CDD_TOKEN_ASSIGN) {
            assign_idx = i;
            i += 2;
          } else {
            bin_idx = i;
            bin_op = (k == CDD_TOKEN_LT) ? CDD_TOKEN_LSHIFT : CDD_TOKEN_RSHIFT;
            i++;
          }
        } else if (k == CDD_TOKEN_LT || k == CDD_TOKEN_GT) {
          logic_idx = i;
          if (nxt_ch == '=')
            i++;
        } else if ((ch == '+' && nxt_ch == '+') ||
                   (ch == '-' && nxt_ch == '-')) {
          i++;
        } else if ((ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
                    ch == '%' || ch == '&' || ch == '|' || ch == '^') &&
                   nxt_ch == '=') {
          assign_idx = i;
          i++;
        } else if (k == CDD_TOKEN_ASSIGN && assign_idx == n) {
          assign_idx = i;
        } else if ((ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
                    ch == '%' || ch == '&' || ch == '|' || ch == '^') &&
                   i > 0) {
          enum cdd_token_kind_t pk = tokens[i - 1]->kind;
          if (pk == CDD_TOKEN_IDENTIFIER || pk == CDD_TOKEN_NUMBER ||
              pk == CDD_TOKEN_CHAR || pk == CDD_TOKEN_STRING ||
              pk == CDD_TOKEN_RPAREN || pk == CDD_TOKEN_RBRACKET) {
            bin_idx = i;
            bin_op = k;
          }
        }
      }
    }

    if (last_comma < n && last_comma + 1 < n) {
      return te_eval_expr_rec(env, scope, tokens + last_comma + 1,
                              n - last_comma - 1, out_type, cap);
    }
    if (assign_idx < n && assign_idx > 0) {
      return te_eval_expr_rec(env, scope, tokens, assign_idx, out_type, cap);
    }
    if (q_idx < n && colon_idx < n && colon_idx + 1 < n) {
      char lt[160];
      char rt[160];
      int rl = 0, rr = 0;
      size_t l_start = (colon_idx > q_idx + 1) ? (q_idx + 1) : 0;
      size_t l_len = (colon_idx > q_idx + 1) ? (colon_idx - q_idx - 1) : q_idx;
      rc = (cdd_c_error_t)((int)rc |
                           (int)te_eval_expr_rec(env, scope, tokens + l_start,
                                                 l_len, lt, sizeof(lt)));
      rc = (cdd_c_error_t)((int)rc | (int)te_eval_expr_rec(
                                         env, scope, tokens + colon_idx + 1,
                                         n - colon_idx - 1, rt, sizeof(rt)));
      rc = (cdd_c_error_t)((int)rc |
                           (int)te_normalize_type_mode(
                               lt, sizeof(lt), CDD_CST_TYPE_MODE_RVALUE));
      rc = (cdd_c_error_t)((int)rc |
                           (int)te_normalize_type_mode(
                               rt, sizeof(rt), CDD_CST_TYPE_MODE_RVALUE));
      if (strchr(lt, '*') != NULL && strcmp(lt, "void *") != 0)
        return te_copy_str(out_type, cap, lt);
      if (strchr(rt, '*') != NULL)
        return te_copy_str(out_type, cap, rt);
      rc = (cdd_c_error_t)((int)rc | (int)te_arith_rank(lt, &rl));
      rc = (cdd_c_error_t)((int)rc | (int)te_arith_rank(rt, &rr));
      return te_copy_str(out_type, cap, rl >= rr ? lt : rt);
    }
    if (logic_idx < n) {
      return te_copy_str(out_type, cap, "int");
    }
    if (bin_idx < n) {
      size_t rhs_off =
          (bin_op == CDD_TOKEN_LSHIFT || bin_op == CDD_TOKEN_RSHIFT) ? 2 : 1;
      if (bin_idx + rhs_off < n) {
        char lt[160];
        char rt[160];
        int l_ptr, r_ptr, rl = 0, rr = 0;
        rc = (cdd_c_error_t)((int)rc | (int)te_eval_expr_rec(env, scope, tokens,
                                                             bin_idx, lt,
                                                             sizeof(lt)));
        rc = (cdd_c_error_t)((int)rc |
                             (int)te_eval_expr_rec(
                                 env, scope, tokens + bin_idx + rhs_off,
                                 n - bin_idx - rhs_off, rt, sizeof(rt)));
        rc = (cdd_c_error_t)((int)rc |
                             (int)te_normalize_type_mode(
                                 lt, sizeof(lt), CDD_CST_TYPE_MODE_RVALUE));
        rc = (cdd_c_error_t)((int)rc |
                             (int)te_normalize_type_mode(
                                 rt, sizeof(rt), CDD_CST_TYPE_MODE_RVALUE));
        l_ptr = (strchr(lt, '*') != NULL);
        r_ptr = (strchr(rt, '*') != NULL);
        if (l_ptr && r_ptr && bin_op == CDD_TOKEN_MINUS)
          return te_copy_str(out_type, cap, "long");
        if (l_ptr)
          return te_copy_str(out_type, cap, lt);
        if (r_ptr && bin_op == CDD_TOKEN_PLUS)
          return te_copy_str(out_type, cap, rt);
        if (bin_op == CDD_TOKEN_LSHIFT || bin_op == CDD_TOKEN_RSHIFT)
          return te_copy_str(out_type, cap, lt);
        rc = (cdd_c_error_t)((int)rc | (int)te_arith_rank(lt, &rl));
        rc = (cdd_c_error_t)((int)rc | (int)te_arith_rank(rt, &rr));
        return te_copy_str(out_type, cap, rl >= rr ? lt : rt);
      }
    }
  }

  /* 3. Prefix unary operators */
  if (tokens[0]->length == 1 && tokens[0]->start[0] == '!') {
    return te_copy_str(out_type, cap, "int");
  }
  {
    int is_sz = 0;
    rc = (cdd_c_error_t)((int)rc |
                         (int)te_tok_in_table(tokens[0], k_sizeof_ops,
                                              sizeof(k_sizeof_ops) /
                                                  sizeof(k_sizeof_ops[0]),
                                              &is_sz));
    if (is_sz) {
      cdd_cst_symbol_t *sz_sym = NULL;
      if (scope &&
          cdd_cst_scope_lookup_in_scope(scope, "size_t", CDD_CST_SYMBOL_TYPEDEF,
                                        &sz_sym) == CDD_C_SUCCESS) {
        return te_copy_str(out_type, cap, "size_t");
      }
      return te_copy_str(out_type, cap, "unsigned long");
    }
  }
  if (n >= 2 && tokens[0]->kind == CDD_TOKEN_STAR) {
    char sub[160];
    rc = (cdd_c_error_t)((int)rc | (int)te_eval_expr_rec(env, scope, tokens + 1,
                                                         n - 1, sub,
                                                         sizeof(sub)));
    rc = (cdd_c_error_t)((int)rc | (int)te_deref_type(sub));
    return te_copy_str(out_type, cap, sub);
  }
  if (n >= 2 && tokens[0]->length == 1 && tokens[0]->start[0] == '&') {
    char sub[168];
    size_t sl;
    rc = (cdd_c_error_t)((int)rc | (int)te_eval_expr_rec(env, scope, tokens + 1,
                                                         n - 1, sub,
                                                         sizeof(sub) - 4));
    rc = (cdd_c_error_t)((int)rc |
                         (int)te_normalize_type_mode(sub, sizeof(sub) - 4,
                                                     CDD_CST_TYPE_MODE_UNQUAL));
    sl = strlen(sub);
    if (sl > 2 && sub[sl - 1] == ']') {
      char *lb = strchr(sub, '[');
      if (lb)
        *lb = '\0';
      sl = strlen(sub);
    }
    if (sl > 0 && sub[sl - 1] != '*')
      sub[sl++] = ' ';
    sub[sl++] = '*';
    sub[sl] = '\0';
    return te_copy_str(out_type, cap, sub);
  }
  if (n >= 3 && ((tokens[0]->kind == CDD_TOKEN_PLUS &&
                  tokens[1]->kind == CDD_TOKEN_PLUS) ||
                 (tokens[0]->kind == CDD_TOKEN_MINUS &&
                  tokens[1]->kind == CDD_TOKEN_MINUS))) {
    return te_eval_expr_rec(env, scope, tokens + 2, n - 2, out_type, cap);
  }
  if (n >= 2 && (tokens[0]->kind == CDD_TOKEN_PLUS ||
                 tokens[0]->kind == CDD_TOKEN_MINUS ||
                 tokens[0]->kind == CDD_TOKEN_TILDE)) {
    return te_eval_expr_rec(env, scope, tokens + 1, n - 1, out_type, cap);
  }

  /* 4. Primary expression + Postfix chain */
  {
    char cur[160];
    size_t pos = 0;

    if (tokens[0]->kind == CDD_TOKEN_LPAREN) {
      int d = 0;
      size_t rp = n;
      for (pos = 0; pos < n; pos++) {
        if (tokens[pos]->kind == CDD_TOKEN_LPAREN)
          d++;
        else if (tokens[pos]->kind == CDD_TOKEN_RPAREN) {
          d--;
          if (d == 0) {
            rp = pos;
            break;
          }
        }
      }
      if (rp < n && rp > 1) {
        rc = (cdd_c_error_t)((int)rc | (int)te_eval_expr_rec(env, scope,
                                                             tokens + 1, rp - 1,
                                                             cur, sizeof(cur)));
        pos = rp + 1;
      } else {
        return te_copy_str(out_type, cap, "int");
      }
    } else if (tokens[0]->kind == CDD_TOKEN_STRING) {
      rc = (cdd_c_error_t)((int)rc |
                           (int)te_copy_str(cur, sizeof(cur), "const char *"));
      pos = 1;
      while (pos < n && tokens[pos]->kind == CDD_TOKEN_STRING)
        pos++;
    } else if (tokens[0]->kind == CDD_TOKEN_CHAR) {
      rc = (cdd_c_error_t)((int)rc | (int)te_copy_str(cur, sizeof(cur), "int"));
      pos = 1;
    } else if (tokens[0]->kind == CDD_TOKEN_NUMBER) {
      rc = (cdd_c_error_t)((int)rc | (int)te_eval_number_literal(tokens[0], cur,
                                                                 sizeof(cur)));
      pos = 1;
    } else if (tokens[0]->kind == CDD_TOKEN_IDENTIFIER &&
               tokens[0]->length < 127) {
      char idbuf[128];
      memcpy(idbuf, tokens[0]->start, tokens[0]->length);
      idbuf[tokens[0]->length] = '\0';

      if (n >= 2 && tokens[1]->kind == CDD_TOKEN_LPAREN) {
        cdd_cst_symbol_t *fsym = NULL;
        int d = 0;
        int m_vp = 0, m_cp = 0;
        for (pos = 1; pos < n; pos++) {
          if (tokens[pos]->kind == CDD_TOKEN_LPAREN)
            d++;
          else if (tokens[pos]->kind == CDD_TOKEN_RPAREN) {
            d--;
            if (d == 0) {
              pos++;
              break;
            }
          }
        }
        rc = (cdd_c_error_t)((int)rc |
                             (int)te_tok_in_table(tokens[0], k_voidp_fns,
                                                  sizeof(k_voidp_fns) /
                                                      sizeof(k_voidp_fns[0]),
                                                  &m_vp));
        rc = (cdd_c_error_t)((int)rc |
                             (int)te_tok_in_table(tokens[0], k_charp_fns,
                                                  sizeof(k_charp_fns) /
                                                      sizeof(k_charp_fns[0]),
                                                  &m_cp));
        if (scope &&
            cdd_cst_scope_lookup_in_scope(scope, idbuf, CDD_CST_SYMBOL_FUNCTION,
                                          &fsym) == CDD_C_SUCCESS &&
            fsym->type_str) {
          rc = (cdd_c_error_t)((int)rc | (int)te_copy_str(cur, sizeof(cur),
                                                          fsym->type_str));
        } else if (m_vp) {
          rc = (cdd_c_error_t)((int)rc |
                               (int)te_copy_str(cur, sizeof(cur), "void *"));
        } else if (m_cp) {
          rc = (cdd_c_error_t)((int)rc |
                               (int)te_copy_str(cur, sizeof(cur), "char *"));
        } else {
          rc = (cdd_c_error_t)((int)rc |
                               (int)te_copy_str(cur, sizeof(cur), "int"));
        }
      } else {
        cdd_cst_symbol_t *vsym = NULL;
        pos = 1;
        if (scope &&
            cdd_cst_scope_lookup_in_scope(scope, idbuf, CDD_CST_SYMBOL_VARIABLE,
                                          &vsym) == CDD_C_SUCCESS &&
            vsym->type_str) {
          rc = (cdd_c_error_t)((int)rc | (int)te_copy_str(cur, sizeof(cur),
                                                          vsym->type_str));
        } else if (strcmp(idbuf, "NULL") == 0 ||
                   strcmp(idbuf, "nullptr") == 0) {
          rc = (cdd_c_error_t)((int)rc |
                               (int)te_copy_str(cur, sizeof(cur), "void *"));
        } else {
          rc = (cdd_c_error_t)((int)rc |
                               (int)te_copy_str(cur, sizeof(cur), "int"));
        }
      }
    } else {
      return te_copy_str(out_type, cap, "int");
    }

    /* Walk postfix chain `[...]`, `.field`, `->field`, `++`, `--` */
    while (pos < n) {
      if (tokens[pos]->kind == CDD_TOKEN_LBRACKET) {
        int d = 0;
        for (; pos < n; pos++) {
          if (tokens[pos]->kind == CDD_TOKEN_LBRACKET)
            d++;
          else if (tokens[pos]->kind == CDD_TOKEN_RBRACKET) {
            d--;
            if (d == 0) {
              pos++;
              break;
            }
          }
        }
        rc = (cdd_c_error_t)((int)rc | (int)te_deref_type(cur));
      } else if ((tokens[pos]->kind == CDD_TOKEN_DOT ||
                  tokens[pos]->kind == CDD_TOKEN_ARROW) &&
                 pos + 1 < n && tokens[pos + 1]->kind == CDD_TOKEN_IDENTIFIER) {
        rc = (cdd_c_error_t)((int)rc | (int)te_resolve_member(env, scope, cur,
                                                              sizeof(cur),
                                                              tokens[pos + 1]));
        pos += 2;
      } else if (pos + 1 < n && ((tokens[pos]->kind == CDD_TOKEN_PLUS &&
                                  tokens[pos + 1]->kind == CDD_TOKEN_PLUS) ||
                                 (tokens[pos]->kind == CDD_TOKEN_MINUS &&
                                  tokens[pos + 1]->kind == CDD_TOKEN_MINUS))) {
        rc = (cdd_c_error_t)((int)rc |
                             (int)te_normalize_type_mode(
                                 cur, sizeof(cur), CDD_CST_TYPE_MODE_RVALUE));
        pos += 2;
      } else {
        break;
      }
    }

    return te_copy_str(out_type, cap, cur);
  }
}

/*
 * @brief Evaluates the canonical C type of a token-pointer expression or type
 * name within a lexical scope.
 *
 * @param[in] env Optional semantic scope environment.
 * @param[in] scope Optional active lexical scope.
 * @param[in] tokens Array of token pointers forming the expression or type.
 * @param[in] num_tokens Number of entries in `tokens`.
 * @param[in] mode Qualifier/decay mode (`cdd_cst_type_eval_mode_t`).
 * @param[out] out_type Destination buffer receiving canonical C type string.
 * @param[in] out_cap Byte capacity of `out_type`.
 * @param[out] out_is_type_expr Optional flag set to 1 if `tokens` is a type
 * name, 0 if value expression.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t cdd_cst_eval_token_ptrs_type(const cdd_cst_scope_env_t *env,
                                           const cdd_cst_scope_t *scope,
                                           const cdd_token_t *const *tokens,
                                           size_t num_tokens, int mode,
                                           char *out_type, size_t out_cap,
                                           int *out_is_type_expr) {
  const cdd_cst_scope_t *eff_scope;
  int is_type = 0;
  cdd_c_error_t rc;

  if (!tokens || num_tokens == 0 || !out_type || out_cap == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (out_is_type_expr)
    *out_is_type_expr = 0;

  eff_scope = scope ? scope : (env ? env->global_scope : NULL);

  rc = te_try_parse_type_name(eff_scope, tokens, num_tokens, out_type, out_cap,
                              &is_type);
  if (is_type) {
    if (out_is_type_expr)
      *out_is_type_expr = 1;
    return (cdd_c_error_t)((int)rc | (int)te_normalize_type_mode(
                                         out_type, out_cap, mode));
  }

  rc = (cdd_c_error_t)((int)rc | (int)te_eval_expr_rec(env, eff_scope, tokens,
                                                       num_tokens, out_type,
                                                       out_cap));
  return (cdd_c_error_t)((int)rc |
                         (int)te_normalize_type_mode(out_type, out_cap, mode));
}

/*
 * @brief Evaluates the canonical C type of a contiguous token slice within a
 * lexical scope.
 *
 * @param[in] env Optional semantic scope environment.
 * @param[in] scope Optional active lexical scope.
 * @param[in] tokens Contiguous array of tokens.
 * @param[in] num_tokens Number of tokens in `tokens`.
 * @param[in] mode Qualifier/decay mode (`cdd_cst_type_eval_mode_t`).
 * @param[out] out_type Destination buffer receiving canonical C type string.
 * @param[in] out_cap Byte capacity of `out_type`.
 * @param[out] out_is_type_expr Optional flag set to 1 if `tokens` is a type
 * name, 0 if value expression.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t cdd_cst_eval_tokens_type(const cdd_cst_scope_env_t *env,
                                       const cdd_cst_scope_t *scope,
                                       const cdd_token_t *tokens,
                                       size_t num_tokens, int mode,
                                       char *out_type, size_t out_cap,
                                       int *out_is_type_expr) {
  const cdd_token_t *stack_ptrs[64];
  const cdd_token_t **ptrs = stack_ptrs;
  size_t i;
  cdd_c_error_t rc;

  if (!tokens || num_tokens == 0 || !out_type || out_cap == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (num_tokens > 64) {
    ptrs = (const cdd_token_t **)C_CDD_MALLOC(num_tokens *
                                              sizeof(const cdd_token_t *));
    if (!ptrs)
      return CDD_C_ERROR_MEMORY;
  }
  for (i = 0; i < num_tokens; i++) {
    ptrs[i] = &tokens[i];
  }
  rc = cdd_cst_eval_token_ptrs_type(env, scope, ptrs, num_tokens, mode,
                                    out_type, out_cap, out_is_type_expr);
  if (ptrs != stack_ptrs) {
    C_CDD_FREE((void *)ptrs);
  }
  return rc;
}
