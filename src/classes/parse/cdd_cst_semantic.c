/**
 * @file cdd_cst_semantic.c
 * @brief Concrete Syntax Tree (CST) semantic implementation and utilities.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_semantic.h"
#include "cdd_cst_parser.h"
#include "c_cdd_export.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_str_span.h"
#include "functions/parse/fs.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
/** @brief Deterministic fault-injection counter for CST semantic helpers. */
C_CDD_EXPORT int g_cdd_sem_fail_step = 0;
#endif

/** @brief Maximum header include recursion depth during pre-scanning */
#define CDD_SEMANTIC_MAX_INCLUDE_DEPTH 8
/** @brief Maximum unique headers pre-scanned per translation unit */
#define CDD_SEMANTIC_MAX_VISITED_HEADERS 64

/**
 * @brief State tracked while populating semantic symbols and scanning headers.
 */
struct cdd_semantic_scan_ctx_t {
  const char *const *include_dirs; /**< Header search directories */
  size_t num_include_dirs;         /**< Number of search directories */
  char *visited[CDD_SEMANTIC_MAX_VISITED_HEADERS]; /**< Visited header paths */
  size_t num_visited; /**< Number of visited header paths */
  int depth;          /**< Current header recursion depth */
  int is_external;    /**< 1 when scanning an external header tree */
  int anon_tag_seq;   /**< Sequence counter for unnamed struct/union tags */
};

static cdd_c_error_t analyze_node_ctx(cdd_cst_scope_env_t *env,
                                      cdd_cst_node_t *node,
                                      struct cdd_semantic_scan_ctx_t *ctx,
                                      int bind_ast);

/**
 * @brief Non-fatal scope symbol lookup helper that sets `*out_sym = NULL` when
 * absent.
 *
 * @param[in] env Scope environment.
 * @param[in] name Symbol identifier.
 * @param[in] kind Symbol namespace/kind.
 * @param[out] out_sym Receives found symbol pointer, or NULL if absent.
 * @return CDD_C_SUCCESS on lookup completion, or error code on fault injection.
 */
static cdd_c_error_t sem_try_lookup_symbol(cdd_cst_scope_env_t *env,
                                           const char *name,
                                           enum cdd_cst_symbol_kind_t kind,
                                           cdd_cst_symbol_t **out_sym) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_sem_fail_step > 0 && --g_cdd_sem_fail_step == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_sym = NULL;
  rc = cdd_cst_scope_lookup_symbol(env, name, kind, out_sym);
  if (rc == CDD_C_SUCCESS)
    return rc;
  *out_sym = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Non-fatal header file reader that leaves `*out_src = NULL` when a
 * candidate include path does not exist.
 *
 * @param[in] path Candidate filesystem path.
 * @param[out] out_src Receives heap buffer if file exists, or NULL if absent.
 * @param[out] out_sz Receives file byte length.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t sem_try_read_header(const char *path, char **out_src,
                                         size_t *out_sz) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_sem_fail_step > 0 && --g_cdd_sem_fail_step == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_src = NULL;
  *out_sz = 0;
  rc = read_to_file(path, "rb", out_src, out_sz);
  if (rc == CDD_C_SUCCESS)
    return rc;
  *out_src = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `tok` matches any string in `table[0..count)`.
 *
 * @param[in] tok Token to inspect.
 * @param[in] table Array of null-terminated strings.
 * @param[in] count Number of entries in `table`.
 * @param[out] out_match Receives 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t sem_tok_in_table(const cdd_token_t *tok,
                                      const char *const *table, size_t count,
                                      int *out_match) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_sem_fail_step > 0 && --g_cdd_sem_fail_step == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_match = 0;
  for (i = 0; i < count; i++) {
    size_t len = strlen(table[i]);
    if (tok->length == len && memcmp(tok->start, table[i], len) == 0) {
      *out_match = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether a token starts a C control-flow statement rather than a
 * declaration or function definition.
 *
 * @param[in] tok Token to inspect.
 * @param[out] out_is_ctrl Receives 1 if control keyword, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t sem_is_control_kw(const cdd_token_t *tok,
                                       int *out_is_ctrl) {
  static const char *const k_ctrl[] = {
      "if",       "else",   "for",     "while", "do",
      "switch",   "case",   "default", "goto",  "break",
      "continue", "return", "__asm__", "__asm", "asm"};
#ifdef CDD_BUILD_TESTS
  if (g_cdd_sem_fail_step > 0 && --g_cdd_sem_fail_step == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (tok->kind == CDD_TOKEN_KEYWORD_RETURN ||
      tok->kind == CDD_TOKEN_KEYWORD_GOTO ||
      tok->kind == CDD_TOKEN_KEYWORD_CASE ||
      tok->kind == CDD_TOKEN_KEYWORD_DEFAULT) {
    *out_is_ctrl = 1;
    return CDD_C_SUCCESS;
  }
  return sem_tok_in_table(tok, k_ctrl, sizeof(k_ctrl) / sizeof(k_ctrl[0]),
                          out_is_ctrl);
}

/**
 * @brief Appends a space-separated token spelling into a fixed buffer.
 *
 * @param[in,out] buf Destination string buffer.
 * @param[in] cap Buffer byte capacity.
 * @param[in,out] io_len Current string length in `buf`.
 * @param[in] str Token text pointer.
 * @param[in] len Length of `str`.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t sem_buf_append_tok(char *buf, size_t cap, size_t *io_len,
                                        const char *str, size_t len) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_sem_fail_step > 0 && --g_cdd_sem_fail_step == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (*io_len + len + 2 < cap) {
    if (*io_len > 0 && buf[*io_len - 1] != '*' && str[0] != '[') {
      buf[(*io_len)++] = ' ';
    }
    memcpy(buf + *io_len, str, len);
    *io_len += len;
    buf[*io_len] = '\0';
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Skips `__attribute__((...))` or `__declspec(...)` at index `*io_idx`.
 *
 * @param[in] toks Array of token pointers.
 * @param[in] n Number of tokens.
 * @param[in,out] io_idx Current index; advanced past attribute if matched.
 * @param[out] out_skipped Receives 1 if an attribute was skipped, else 0.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t sem_skip_attr(const cdd_token_t *const *toks, size_t n,
                                   size_t *io_idx, int *out_skipped) {
  static const char *const k_attrs[] = {"__attribute__", "__attribute",
                                        "__declspec"};
  size_t i = *io_idx;
  int m = 0;
  cdd_c_error_t rc;
  *out_skipped = 0;
  rc = sem_tok_in_table(toks[i], k_attrs, sizeof(k_attrs) / sizeof(k_attrs[0]),
                        &m);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (m) {
    i++;
    if (i < n && toks[i]->kind == CDD_TOKEN_LPAREN) {
      int depth = 0;
      for (; i < n; i++) {
        if (toks[i]->kind == CDD_TOKEN_LPAREN)
          depth++;
        else if (toks[i]->kind == CDD_TOKEN_RPAREN) {
          depth--;
          if (depth == 0) {
            i++;
            break;
          }
        }
      }
    }
    *io_idx = i;
    *out_skipped = 1;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a constant integer/char/enum enumerator value expression.
 *
 * @param[in] env Active scope environment for prior enum constant lookup.
 * @param[in] toks Token pointer slice of the enumerator value expression.
 * @param[in] n Number of tokens in slice.
 * @param[out] out_val Evaluated integer value.
 * @param[out] out_ok Receives 1 if evaluated, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t sem_eval_enum_expr(cdd_cst_scope_env_t *env,
                                        const cdd_token_t *const *toks,
                                        size_t n, long *out_val, int *out_ok) {
  size_t i = 0;
  int neg = 0;
  const cdd_token_t *t;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_sem_fail_step > 0 && --g_cdd_sem_fail_step == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_val = 0;
  *out_ok = 0;
  while (i < n && toks[i]->kind == CDD_TOKEN_LPAREN)
    i++;
  if (i < n && toks[i]->kind == CDD_TOKEN_MINUS) {
    neg = 1;
    i++;
  } else if (i < n && toks[i]->kind == CDD_TOKEN_PLUS) {
    i++;
  }
  if (i >= n)
    return CDD_C_SUCCESS;
  t = toks[i];
  if (t->kind == CDD_TOKEN_NUMBER && t->length < 60) {
    char buf[64];
    long v = 0;
    memcpy(buf, t->start, t->length);
    buf[t->length] = '\0';
    if (t->length > 2 && buf[0] == '0' && (buf[1] == 'b' || buf[1] == 'B')) {
      size_t k;
      for (k = 2; k < t->length && (buf[k] == '0' || buf[k] == '1'); k++) {
        v = (v << 1) | (long)(buf[k] - '0');
      }
    } else {
      v = strtol(buf, NULL, 0);
    }
    *out_val = neg ? -v : v;
    *out_ok = 1;
  } else if (t->kind == CDD_TOKEN_CHAR && t->length >= 3) {
    long v = (unsigned char)t->start[1];
    if (t->start[1] == 92 && t->length >= 4) {
      char esc = (char)t->start[2];
      v = (esc == 'n') ? 10 : (esc == '0' ? 0 : (unsigned char)esc);
    }
    *out_val = neg ? -v : v;
    *out_ok = 1;
  } else if (t->kind == CDD_TOKEN_IDENTIFIER && t->length < 120) {
    char id_buf[128];
    cdd_cst_symbol_t *sym = NULL;
    memcpy(id_buf, t->start, t->length);
    id_buf[t->length] = '\0';
    rc = sem_try_lookup_symbol(env, id_buf, CDD_CST_SYMBOL_ENUM_CONSTANT, &sym);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (sym) {
      *out_val = neg ? -sym->enum_value : sym->enum_value;
      *out_ok = 1;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds a symbol with an associated canonical C type string.
 *
 * @param[in,out] env Scope environment.
 * @param[in] name Non-empty symbol identifier.
 * @param[in] kind Symbol namespace/kind.
 * @param[in] type_str Non-empty canonical C type string.
 * @param[in] decl_node Optional CST declaration node.
 * @param[out] out_sym Optional pointer receiving the created symbol.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t sem_register_typed_symbol(cdd_cst_scope_env_t *env,
                                               const char *name,
                                               enum cdd_cst_symbol_kind_t kind,
                                               const char *type_str,
                                               cdd_cst_node_t *decl_node,
                                               cdd_cst_symbol_t **out_sym) {
  cdd_c_error_t rc;
  cdd_cst_symbol_t *sym;
  rc = cdd_cst_scope_add_symbol(env, name, kind, decl_node);
  if (rc != CDD_C_SUCCESS)
    return rc;
  sym = env->current_scope->symbols;
  rc = cdd_cst_symbol_set_type(sym, type_str);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (out_sym)
    *out_sym = sym;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses declarators following a base type string (e.g. `int *a, b[4];`)
 * and registers either struct/union fields or scope variables/typedefs.
 *
 * @param[in,out] env Active scope environment.
 * @param[in] base_type Non-empty base declaration-specifier type string.
 * @param[in] is_typedef Non-zero if `typedef` storage class was present.
 * @param[in,out] tag_owner Optional struct/union tag symbol when parsing
 * members.
 * @param[in] toks Declarator token array (up to `;`).
 * @param[in] n Number of tokens in `toks`.
 * @param[in] decl_node Optional CST declaration node.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t
sem_parse_declarator_list(cdd_cst_scope_env_t *env, const char *base_type,
                          int is_typedef, cdd_cst_symbol_t *tag_owner,
                          const cdd_token_t *const *toks, size_t n,
                          cdd_cst_node_t *decl_node) {
  static const char *const k_quals[] = {"const", "volatile", "restrict",
                                        "__restrict"};
  size_t pos = 0;
  cdd_c_error_t rc;

  while (pos < n) {
    size_t seg_start = pos;
    size_t seg_end = n;
    size_t eq_pos = n;
    size_t colon_pos = n;
    int depth = 0;
    size_t i;

    if (toks[pos]->kind == CDD_TOKEN_SEMICOLON)
      break;

    for (i = pos; i < n; i++) {
      enum cdd_token_kind_t k = toks[i]->kind;
      if (k == CDD_TOKEN_LPAREN || k == CDD_TOKEN_LBRACKET)
        depth++;
      else if ((k == CDD_TOKEN_RPAREN || k == CDD_TOKEN_RBRACKET) && depth > 0)
        depth--;
      else if (depth == 0) {
        if (k == CDD_TOKEN_COMMA || k == CDD_TOKEN_SEMICOLON) {
          seg_end = i;
          break;
        }
        if (k == CDD_TOKEN_ASSIGN && eq_pos == n) {
          eq_pos = i;
        } else if (k == CDD_TOKEN_COLON && colon_pos == n && tag_owner) {
          colon_pos = i;
        }
      }
    }

    {
      size_t decl_end = seg_end;
      int ptr_count = 0;
      int is_func_decl = 0;
      int is_fn_ptr = 0;
      size_t arr_len = 0;
      int has_arr = 0;
      char arr_suffix[64];
      char id_name[128];
      char full_type[256];
      size_t k;

      if (eq_pos < decl_end)
        decl_end = eq_pos;
      if (colon_pos < decl_end)
        decl_end = colon_pos;

      arr_suffix[0] = '\0';
      id_name[0] = '\0';

      for (k = seg_start; k < decl_end; k++) {
        int skipped = 0;
        rc = sem_skip_attr(toks, decl_end, &k, &skipped);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (skipped) {
          k--;
          continue;
        }
        if (toks[k]->kind == CDD_TOKEN_STAR) {
          ptr_count++;
        } else if (toks[k]->kind == CDD_TOKEN_LPAREN) {
          if (k + 2 < decl_end && toks[k + 1]->kind == CDD_TOKEN_STAR &&
              toks[k + 2]->kind == CDD_TOKEN_IDENTIFIER) {
            size_t nl = toks[k + 2]->length < 127 ? toks[k + 2]->length : 127;
            memcpy(id_name, toks[k + 2]->start, nl);
            id_name[nl] = '\0';
            is_fn_ptr = 1;
            break;
          } else if (id_name[0] != '\0') {
            is_func_decl = 1;
            break;
          }
        } else if (toks[k]->kind == CDD_TOKEN_LBRACKET) {
          has_arr = 1;
          if (k + 2 < decl_end && toks[k + 1]->kind == CDD_TOKEN_NUMBER &&
              toks[k + 2]->kind == CDD_TOKEN_RBRACKET &&
              toks[k + 1]->length < 24) {
            char nbuf[32];
            memcpy(nbuf, toks[k + 1]->start, toks[k + 1]->length);
            nbuf[toks[k + 1]->length] = '\0';
            arr_len = (size_t)strtoul(nbuf, NULL, 0);
            if (arr_suffix[0] == '\0') {
              CDD_SNPRINTF(arr_suffix, sizeof(arr_suffix), "[%lu]",
                           (unsigned long)arr_len);
            }
          } else if (arr_suffix[0] == '\0') {
            CDD_SNPRINTF(arr_suffix, sizeof(arr_suffix), " *");
          }
          while (k < decl_end && toks[k]->kind != CDD_TOKEN_RBRACKET)
            k++;
        } else if (toks[k]->kind == CDD_TOKEN_IDENTIFIER) {
          int is_qual = 0;
          rc = sem_tok_in_table(toks[k], k_quals,
                                sizeof(k_quals) / sizeof(k_quals[0]), &is_qual);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (!is_qual) {
            size_t nl = toks[k]->length < 127 ? toks[k]->length : 127;
            memcpy(id_name, toks[k]->start, nl);
            id_name[nl] = '\0';
          }
        }
      }

      if (id_name[0] != '\0') {
        size_t fl = 0;
        int p;
        full_type[0] = '\0';
        rc = sem_buf_append_tok(full_type, sizeof(full_type), &fl, base_type,
                                strlen(base_type));
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_fn_ptr) {
          rc =
              sem_buf_append_tok(full_type, sizeof(full_type), &fl, "(*)()", 5);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else {
          for (p = 0; p < ptr_count; p++) {
            rc = sem_buf_append_tok(full_type, sizeof(full_type), &fl, "*", 1);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (has_arr) {
            rc = sem_buf_append_tok(full_type, sizeof(full_type), &fl,
                                    arr_suffix, strlen(arr_suffix));
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }

        if (tag_owner) {
          rc = cdd_cst_symbol_add_field(tag_owner, id_name, full_type, arr_len,
                                        colon_pos < seg_end ? 1 : 0);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else {
          enum cdd_cst_symbol_kind_t sk =
              is_typedef ? CDD_CST_SYMBOL_TYPEDEF
                         : (is_func_decl ? CDD_CST_SYMBOL_FUNCTION
                                         : CDD_CST_SYMBOL_VARIABLE);
          rc = sem_register_typed_symbol(env, id_name, sk, full_type, decl_node,
                                         NULL);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }

    if (seg_end >= n || toks[seg_end]->kind == CDD_TOKEN_SEMICOLON)
      break;
    pos = seg_end + 1;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Collects direct token children of a CST node into a small stack array.
 *
 * @param[in] node CST node whose direct token children to collect.
 * @param[out] out_toks Destination array of token pointers.
 * @param[in] cap Maximum capacity of `out_toks`.
 * @param[out] out_count Number of tokens collected.
 * @param[out] out_block First direct `CDD_CST_BLOCK` child if any.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t sem_collect_direct_tokens(cdd_cst_node_t *node,
                                               const cdd_token_t **out_toks,
                                               size_t cap, size_t *out_count,
                                               cdd_cst_node_t **out_block) {
  size_t i;
  size_t cnt = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_sem_fail_step > 0 && --g_cdd_sem_fail_step == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_block = NULL;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *t = node->children[i].val.token;
      if (t->length > 0 && cnt < cap) {
        out_toks[cnt++] = t;
      }
    } else if (!*out_block &&
               node->children[i].val.node->kind == CDD_CST_BLOCK) {
      *out_block = node->children[i].val.node;
    }
  }
  *out_count = cnt;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses enum constant definitions inside an `enum { ... }` block.
 *
 * @param[in,out] env Active scope environment.
 * @param[in] enum_block CST block node `{ A = 1, B, ... }`.
 * @param[in] decl_node Optional CST declaration node.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t sem_parse_enum_block(cdd_cst_scope_env_t *env,
                                          cdd_cst_node_t *enum_block,
                                          cdd_cst_node_t *decl_node) {
  const cdd_token_t *toks[256];
  size_t n = 0;
  size_t i;
  long next_val = 0;
  cdd_c_error_t rc;

  for (i = 0; i < enum_block->num_children; i++) {
    if (enum_block->children[i].kind == CDD_CST_CHILD_TOKEN) {
      const cdd_token_t *t = enum_block->children[i].val.token;
      if (t->kind != CDD_TOKEN_LBRACE && t->kind != CDD_TOKEN_RBRACE &&
          n < 256) {
        toks[n++] = t;
      }
    } else {
      cdd_cst_node_t *sub = enum_block->children[i].val.node;
      cdd_cst_node_t *dummy = NULL;
      size_t sub_n = 0;
      rc = sem_collect_direct_tokens(sub, &toks[n], 256 - n, &sub_n, &dummy);
      if (rc != CDD_C_SUCCESS)
        return rc;
      n += sub_n;
    }
  }

  i = 0;
  while (i < n) {
    size_t comma = n;
    size_t eq = n;
    int depth = 0;
    size_t k;
    for (k = i; k < n; k++) {
      if (toks[k]->kind == CDD_TOKEN_LPAREN)
        depth++;
      else if (toks[k]->kind == CDD_TOKEN_RPAREN && depth > 0)
        depth--;
      else if (depth == 0) {
        if (toks[k]->kind == CDD_TOKEN_COMMA) {
          comma = k;
          break;
        }
        if (toks[k]->kind == CDD_TOKEN_ASSIGN && eq == n)
          eq = k;
      }
    }
    if (toks[i]->kind == CDD_TOKEN_IDENTIFIER && toks[i]->length < 120) {
      char ename[128];
      cdd_cst_symbol_t *esym = NULL;
      memcpy(ename, toks[i]->start, toks[i]->length);
      ename[toks[i]->length] = '\0';
      if (eq < comma && eq + 1 < comma) {
        long eval_v = 0;
        int ok = 0;
        rc = sem_eval_enum_expr(env, &toks[eq + 1], comma - (eq + 1), &eval_v,
                                &ok);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (ok)
          next_val = eval_v;
      }
      rc = sem_register_typed_symbol(env, ename, CDD_CST_SYMBOL_ENUM_CONSTANT,
                                     "int", decl_node, &esym);
      if (rc != CDD_C_SUCCESS)
        return rc;
      esym->enum_value = next_val;
      esym->has_enum_value = 1;
      next_val++;
    }
    if (comma >= n)
      break;
    i = comma + 1;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts base type specifiers from `toks[0..n)` and returns the index
 * where the declarator begins.
 *
 * @param[in] env Scope environment for typedef disambiguation.
 * @param[in] toks Array of statement tokens.
 * @param[in] n Number of tokens.
 * @param[out] out_is_typedef Receives 1 if `typedef` was present.
 * @param[out] out_base_type Buffer receiving canonical base type string.
 * @param[in] base_cap Capacity of `out_base_type`.
 * @param[out] out_decl_start Index of first declarator token.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t
sem_extract_decl_specs(cdd_cst_scope_env_t *env, const cdd_token_t *const *toks,
                       size_t n, int *out_is_typedef, char *out_base_type,
                       size_t base_cap, size_t *out_decl_start) {
  static const char *const k_storage[] = {
      "extern",    "static",        "auto",          "register",
      "inline",    "__inline",      "__inline__",    "__forceinline",
      "_Noreturn", "__extension__", "_Thread_local", "thread_local",
      "__thread",  "constexpr",     "_Atomic"};
  static const char *const k_aggr[] = {"union", "enum"};
  static const char *const k_quals[] = {"const", "volatile", "restrict",
                                        "__restrict"};
  static const char *const k_multi_nouns[] = {"unsigned", "signed", "long",
                                              "short",    "char",   "double"};
  static const char *const k_single_nouns[] = {
      "void",     "float",     "_Bool",        "bool",         "size_t",
      "ssize_t",  "ptrdiff_t", "intptr_t",     "uintptr_t",    "int8_t",
      "int16_t",  "int32_t",   "int64_t",      "uint8_t",      "uint16_t",
      "uint32_t", "uint64_t",  "cdd_int128_t", "cdd_uint128_t"};
  size_t i = 0;
  size_t blen = 0;
  int have_type_noun = 0;
  cdd_c_error_t rc;

  *out_is_typedef = 0;
  out_base_type[0] = '\0';
  *out_decl_start = 0;

  while (i < n) {
    const cdd_token_t *t = toks[i];
    int skipped = 0;
    int m = 0;

    rc = sem_skip_attr(toks, n, &i, &skipped);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (skipped)
      continue;

    if (t->length == 7 && memcmp(t->start, "typedef", 7) == 0) {
      *out_is_typedef = 1;
      i++;
      continue;
    }
    rc = sem_tok_in_table(t, k_storage,
                          sizeof(k_storage) / sizeof(k_storage[0]), &m);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (m) {
      i++;
      continue;
    }

    rc = sem_tok_in_table(t, k_aggr, sizeof(k_aggr) / sizeof(k_aggr[0]), &m);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (t->kind == CDD_TOKEN_KEYWORD_STRUCT || m) {
      rc = sem_buf_append_tok(out_base_type, base_cap, &blen,
                              (const char *)t->start, t->length);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i++;
      if (i < n && toks[i]->kind == CDD_TOKEN_IDENTIFIER) {
        rc = sem_buf_append_tok(out_base_type, base_cap, &blen,
                                (const char *)toks[i]->start, toks[i]->length);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i++;
      }
      have_type_noun = 1;
      continue;
    }

    if (t->kind == CDD_TOKEN_KEYWORD_INT ||
        t->kind == CDD_TOKEN_KEYWORD___INT128 ||
        t->kind == CDD_TOKEN_KEYWORD__DECIMAL32 ||
        t->kind == CDD_TOKEN_KEYWORD__DECIMAL64 ||
        t->kind == CDD_TOKEN_KEYWORD__DECIMAL128 ||
        t->kind == CDD_TOKEN_KEYWORD___FP16 ||
        t->kind == CDD_TOKEN_KEYWORD__FLOAT16 ||
        t->kind == CDD_TOKEN_KEYWORD___BF16) {
      rc = sem_buf_append_tok(out_base_type, base_cap, &blen,
                              (const char *)t->start, t->length);
      if (rc != CDD_C_SUCCESS)
        return rc;
      have_type_noun = 1;
      i++;
      continue;
    }

    if (t->kind == CDD_TOKEN_IDENTIFIER) {
      rc = sem_tok_in_table(t, k_quals, sizeof(k_quals) / sizeof(k_quals[0]),
                            &m);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (m) {
        rc = sem_buf_append_tok(out_base_type, base_cap, &blen,
                                (const char *)t->start, t->length);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i++;
        continue;
      }
      rc = sem_tok_in_table(t, k_multi_nouns,
                            sizeof(k_multi_nouns) / sizeof(k_multi_nouns[0]),
                            &m);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (!m && !have_type_noun) {
        rc = sem_tok_in_table(
            t, k_single_nouns,
            sizeof(k_single_nouns) / sizeof(k_single_nouns[0]), &m);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (m) {
        rc = sem_buf_append_tok(out_base_type, base_cap, &blen,
                                (const char *)t->start, t->length);
        if (rc != CDD_C_SUCCESS)
          return rc;
        have_type_noun = 1;
        i++;
        continue;
      }
      if (!have_type_noun && t->length < 120) {
        char tname[128];
        cdd_cst_symbol_t *tsym = NULL;
        memcpy(tname, t->start, t->length);
        tname[t->length] = '\0';
        rc = sem_try_lookup_symbol(env, tname, CDD_CST_SYMBOL_TYPEDEF, &tsym);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (tsym && tsym->kind == CDD_CST_SYMBOL_TYPEDEF) {
          rc = sem_buf_append_tok(out_base_type, base_cap, &blen, tname,
                                  t->length);
          if (rc != CDD_C_SUCCESS)
            return rc;
          have_type_noun = 1;
          i++;
          continue;
        }
      }
    }
    break;
  }

  if (!have_type_noun) {
    out_base_type[0] = '\0';
  }
  *out_decl_start = i;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses all member declarations inside a `struct` or `union` block into
 * `tag_sym->fields`.
 *
 * @param[in,out] env Active scope environment.
 * @param[in,out] tag_sym Struct or union tag symbol.
 * @param[in] body_block CST block node `{ ... }` of the struct/union.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t sem_parse_record_fields(cdd_cst_scope_env_t *env,
                                             cdd_cst_symbol_t *tag_sym,
                                             cdd_cst_node_t *body_block) {
  size_t i;
  cdd_c_error_t rc;
  for (i = 0; i < body_block->num_children; i++) {
    if (body_block->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *mem_node = body_block->children[i].val.node;
      const cdd_token_t *mtoks[64];
      size_t mn = 0;
      cdd_cst_node_t *sub_blk = NULL;
      int is_td = 0;
      char btype[128];
      size_t dstart = 0;

      rc = sem_collect_direct_tokens(mem_node, mtoks, 64, &mn, &sub_blk);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = sem_extract_decl_specs(env, mtoks, mn, &is_td, btype, sizeof(btype),
                                  &dstart);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (btype[0] != '\0' && dstart < mn) {
        rc = sem_parse_declarator_list(env, btype, 0, tag_sym, &mtoks[dstart],
                                       mn - dstart, NULL);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Analyzes a `struct`/`union`/`enum` aggregate declaration (optionally
 * split across `node` and `trailing_node` by the CST parser).
 *
 * @param[in,out] env Active scope environment.
 * @param[in] node CST node holding the aggregate specifier and `{ ... }` block.
 * @param[in] trailing_node Optional sibling CST node holding post-`}` tokens.
 * @param[in,out] ctx Semantic scan state.
 * @param[out] out_consumed_next Receives 1 if `trailing_node` was consumed.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t sem_analyze_aggregate(cdd_cst_scope_env_t *env,
                                           cdd_cst_node_t *node,
                                           cdd_cst_node_t *trailing_node,
                                           struct cdd_semantic_scan_ctx_t *ctx,
                                           int *out_consumed_next) {
  const cdd_token_t *pre_toks[32];
  const cdd_token_t *post_toks[64];
  size_t n_pre = 0;
  size_t n_post = 0;
  cdd_cst_node_t *body_block = NULL;
  size_t i;
  int seen_block = 0;
  int is_typedef = 0;
  int is_union = 0;
  int is_enum = 0;
  int is_struct = 0;
  char tag_name[128];
  char full_tag_type[160];
  cdd_cst_symbol_t *tag_sym = NULL;
  cdd_cst_node_t *rec_decl = ctx->is_external ? NULL : node;
  cdd_c_error_t rc;

  *out_consumed_next = 0;
  tag_name[0] = '\0';

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      if (node->children[i].val.node->kind == CDD_CST_BLOCK && !seen_block) {
        body_block = node->children[i].val.node;
        seen_block = 1;
      }
    } else {
      cdd_token_t *t = node->children[i].val.token;
      if (t->length == 0)
        continue;
      if (!seen_block) {
        if (n_pre < 32)
          pre_toks[n_pre++] = t;
      } else if (n_post < 64) {
        post_toks[n_post++] = t;
      }
    }
  }

  for (i = 0; i < n_pre; i++) {
    int skipped = 0;
    rc = sem_skip_attr(pre_toks, n_pre, &i, &skipped);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (skipped) {
      i--;
      continue;
    }
    if (pre_toks[i]->length == 7 &&
        memcmp(pre_toks[i]->start, "typedef", 7) == 0) {
      is_typedef = 1;
    } else if (pre_toks[i]->kind == CDD_TOKEN_KEYWORD_STRUCT ||
               (pre_toks[i]->length == 5 &&
                memcmp(pre_toks[i]->start, "union", 5) == 0) ||
               (pre_toks[i]->length == 4 &&
                memcmp(pre_toks[i]->start, "enum", 4) == 0)) {
      if (pre_toks[i]->kind == CDD_TOKEN_KEYWORD_STRUCT)
        is_struct = 1;
      else if (pre_toks[i]->length == 5)
        is_union = 1;
      else
        is_enum = 1;
      if (i + 1 < n_pre && pre_toks[i + 1]->kind == CDD_TOKEN_IDENTIFIER) {
        size_t nl =
            pre_toks[i + 1]->length < 120 ? pre_toks[i + 1]->length : 120;
        memcpy(tag_name, pre_toks[i + 1]->start, nl);
        tag_name[nl] = '\0';
        i++;
      }
    }
  }

  if (!is_struct && !is_union && !is_enum)
    return CDD_C_SUCCESS;

  if (n_post == 0 && trailing_node && trailing_node->kind == CDD_CST_UNKNOWN) {
    cdd_cst_node_t *dummy_blk = NULL;
    rc = sem_collect_direct_tokens(trailing_node, post_toks, 64, &n_post,
                                   &dummy_blk);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_consumed_next = 1;
  }

  if (tag_name[0] == '\0') {
    CDD_SNPRINTF(tag_name, sizeof(tag_name), "__cdd_anon_tag_%d",
                 ctx->anon_tag_seq++);
  }

  if (is_enum) {
    rc = sem_register_typed_symbol(env, tag_name, CDD_CST_SYMBOL_ENUM_TAG,
                                   "int", rec_decl, &tag_sym);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = sem_parse_enum_block(env, body_block, rec_decl);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (memcmp(tag_name, "__cdd_anon_tag_", 15) == 0) {
      CDD_SNPRINTF(full_tag_type, sizeof(full_tag_type), "int");
    } else {
      CDD_SNPRINTF(full_tag_type, sizeof(full_tag_type), "enum %s", tag_name);
    }
  } else {
    enum cdd_cst_symbol_kind_t tkind =
        is_union ? CDD_CST_SYMBOL_UNION_TAG : CDD_CST_SYMBOL_STRUCT_TAG;
    CDD_SNPRINTF(full_tag_type, sizeof(full_tag_type), "%s %s",
                 is_union ? "union" : "struct", tag_name);
    rc = sem_register_typed_symbol(env, tag_name, tkind, full_tag_type,
                                   rec_decl, &tag_sym);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = sem_parse_record_fields(env, tag_sym, body_block);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (n_post > 0) {
    rc = sem_parse_declarator_list(env, full_tag_type, is_typedef, NULL,
                                   post_toks, n_post, rec_decl);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses parameter declarations inside `( ... )` of a function
 * definition into the active function scope.
 *
 * @param[in,out] env Active scope environment (inside function scope).
 * @param[in] param_toks Tokens inside outer parentheses of function signature.
 * @param[in] n Number of parameter tokens.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t sem_parse_fn_params(cdd_cst_scope_env_t *env,
                                         const cdd_token_t *const *param_toks,
                                         size_t n) {
  size_t pos = 0;
  size_t comma = 0;
  cdd_c_error_t rc;

  if (n == 1 && param_toks[0]->length == 4 &&
      memcmp(param_toks[0]->start, "void", 4) == 0) {
    return CDD_C_SUCCESS;
  }

  for (pos = 0; pos < n; pos = comma + 1) {
    size_t comma_pos = n;
    int depth = 0;
    size_t k;
    int is_td = 0;
    char btype[128];
    size_t dstart = 0;

    for (k = pos; k < n; k++) {
      if (param_toks[k]->kind == CDD_TOKEN_LPAREN ||
          param_toks[k]->kind == CDD_TOKEN_LBRACKET)
        depth++;
      else if ((param_toks[k]->kind == CDD_TOKEN_RPAREN ||
                param_toks[k]->kind == CDD_TOKEN_RBRACKET) &&
               depth > 0)
        depth--;
      else if (depth == 0 && param_toks[k]->kind == CDD_TOKEN_COMMA) {
        comma_pos = k;
        break;
      }
    }
    comma = comma_pos;

    rc = sem_extract_decl_specs(env, &param_toks[pos], comma - pos, &is_td,
                                btype, sizeof(btype), &dstart);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (btype[0] != '\0' && pos + dstart < comma) {
      rc = sem_parse_declarator_list(env, btype, 0, NULL,
                                     &param_toks[pos + dstart],
                                     comma - (pos + dstart), NULL);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Pre-scans a `#include` directive against `ctx->include_dirs` if not
 * yet visited.
 *
 * @param[in,out] env Active scope environment.
 * @param[in] inc_tok Preprocessor include token.
 * @param[in,out] ctx Semantic scan state.
 * @return CDD_C_SUCCESS on success (missing headers are ignored gracefully), or
 * error code on failure.
 */
static cdd_c_error_t
sem_scan_include_token(cdd_cst_scope_env_t *env, const cdd_token_t *inc_tok,
                       struct cdd_semantic_scan_ctx_t *ctx) {
  const char *raw;
  size_t len;
  size_t s = 0;
  size_t e = 0;
  char rel_hdr[256];
  size_t d;
  cdd_c_error_t rc;

  if (!ctx->include_dirs || ctx->num_include_dirs == 0 ||
      ctx->depth >= CDD_SEMANTIC_MAX_INCLUDE_DEPTH ||
      ctx->num_visited >= CDD_SEMANTIC_MAX_VISITED_HEADERS) {
    return CDD_C_SUCCESS;
  }

  raw = (const char *)inc_tok->start;
  len = inc_tok->length;
  for (s = 0; s < len; s++) {
    if (raw[s] == '"' || raw[s] == '<') {
      char close_ch = (raw[s] == '"') ? '"' : '>';
      s++;
      for (e = s; e < len; e++) {
        if (raw[e] == close_ch)
          break;
      }
      break;
    }
  }
  if (e <= s || e - s >= sizeof(rel_hdr))
    return CDD_C_SUCCESS;

  memcpy(rel_hdr, raw + s, e - s);
  rel_hdr[e - s] = '\0';

  for (d = 0; d < ctx->num_include_dirs; d++) {
    char *joined = NULL;
    char *hdr_src = NULL;
    size_t hdr_sz = 0;
    size_t v;
    int already = 0;

    if (!ctx->include_dirs[d])
      continue;
    rc = fs_path_join(ctx->include_dirs[d], rel_hdr, &joined);
    if (rc != CDD_C_SUCCESS)
      return rc;

    for (v = 0; v < ctx->num_visited; v++) {
      if (strcmp(ctx->visited[v], joined) == 0) {
        already = 1;
        break;
      }
    }
    if (already) {
      C_CDD_FREE(joined);
      return CDD_C_SUCCESS;
    }

    rc = sem_try_read_header(joined, &hdr_src, &hdr_sz);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(joined);
      return rc;
    }
    if (hdr_src) {
      cdd_cst_tree_t *hdr_tree = NULL;
      int prev_ext;
      cdd_cst_scope_t *saved_scope;
      ctx->visited[ctx->num_visited++] = joined;
      rc = cdd_cst_parse(az_span_create_from_str(hdr_src), &hdr_tree);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(hdr_src);
        return rc;
      }
      prev_ext = ctx->is_external;
      saved_scope = env->current_scope;
      ctx->depth++;
      ctx->is_external = 1;
      env->current_scope = env->global_scope;
      rc = analyze_node_ctx(env, hdr_tree->root, ctx, 0);
      if (rc != CDD_C_SUCCESS) {
        env->current_scope = saved_scope;
        ctx->is_external = prev_ext;
        ctx->depth--;
        C_CDD_FREE(hdr_src);
        return cdd_c_keep_err(rc, cdd_cst_tree_free(hdr_tree));
      }
      env->current_scope = saved_scope;
      ctx->is_external = prev_ext;
      ctx->depth--;
      C_CDD_FREE(hdr_src);
      return cdd_cst_tree_free(hdr_tree);
    }
    C_CDD_FREE(joined);
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Recursively extracts the first identifier token name from a CST
 * subtree, setting `*out_name = NULL` if absent.
 *
 * @param[in] node Root CST node to search.
 * @param[out] out_name Pointer receiving newly allocated identifier string, or
 * NULL if absent.
 * @return CDD_C_SUCCESS on completion, or error code on OOM/fault injection.
 */
static cdd_c_error_t extract_identifier(cdd_cst_node_t *node, char **out_name) {
  size_t i;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_sem_fail_step > 0 && --g_cdd_sem_fail_step == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_name = NULL;
  if (node->kind == CDD_CST_IDENTIFIER) {
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->kind == CDD_TOKEN_IDENTIFIER) {
          char *name = (char *)C_CDD_MALLOC(tok->length + 1);
          if (!name) {
            return CDD_C_ERROR_MEMORY;
          }
          memcpy(name, tok->start, tok->length);
          name[tok->length] = '\0';
          *out_name = name;
          return CDD_C_SUCCESS;
        }
      }
    }
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      rc = extract_identifier(node->children[i].val.node, out_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (*out_name)
        return CDD_C_SUCCESS;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Analyzes a CST node and its children to populate scopes, tags,
 * typedefs, functions, parameters, variables, and included headers.
 *
 * @param[in,out] env Scope environment to populate.
 * @param[in] node Current CST node being visited.
 * @param[in,out] ctx Optional semantic scan state (NULL for synthetic trees).
 * @param[in] bind_ast Non-zero to bind `scope->ast_node` pointers.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t analyze_node_ctx(cdd_cst_scope_env_t *env,
                                      cdd_cst_node_t *node,
                                      struct cdd_semantic_scan_ctx_t *ctx,
                                      int bind_ast) {
  cdd_c_error_t rc;
  size_t i;
  int entered_scope = 0;

  if (!node)
    return CDD_C_SUCCESS;

  switch (node->kind) {
  case CDD_CST_BLOCK: {
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (bind_ast)
      env->current_scope->ast_node = node;
    entered_scope = 1;
    break;
  }
  case CDD_CST_FUNCTION_DEFINITION: {
    const cdd_token_t *ftoks[128] = {0};
    size_t fn = 0;
    cdd_cst_node_t *fblk = NULL;
    int is_real_fn = 0;
    size_t lparen_idx = 0;
    size_t rparen_idx = 0;

    if (ctx) {
      rc = sem_collect_direct_tokens(node, ftoks, 128, &fn, &fblk);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (fn >= 3) {
        int is_ctrl = 0;
        int has_eq = 0;
        size_t k;
        rc = sem_is_control_kw(ftoks[0], &is_ctrl);
        if (rc != CDD_C_SUCCESS)
          return rc;
        for (k = 0; k < fn; k++) {
          if (ftoks[k]->kind == CDD_TOKEN_ASSIGN) {
            has_eq = 1;
            break;
          }
          if (ftoks[k]->kind == CDD_TOKEN_LPAREN && lparen_idx == 0 && k > 0) {
            lparen_idx = k;
          }
        }
        if (!is_ctrl && !has_eq && lparen_idx > 0 &&
            ftoks[lparen_idx - 1]->kind == CDD_TOKEN_IDENTIFIER) {
          int depth = 0;
          for (k = lparen_idx; k < fn; k++) {
            if (ftoks[k]->kind == CDD_TOKEN_LPAREN)
              depth++;
            else if (ftoks[k]->kind == CDD_TOKEN_RPAREN) {
              depth--;
              if (depth == 0) {
                rparen_idx = k;
                is_real_fn = 1;
                break;
              }
            }
          }
        }
      }
      if (is_real_fn) {
        int is_td = 0;
        char ret_type[128];
        size_t dstart = 0;
        size_t k;
        size_t rlen;
        char fname[128];
        size_t fnl;

        rc = sem_extract_decl_specs(env, ftoks, lparen_idx - 1, &is_td,
                                    ret_type, sizeof(ret_type), &dstart);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (ret_type[0] == '\0') {
          CDD_SNPRINTF(ret_type, sizeof(ret_type), "int");
        }
        rlen = strlen(ret_type);
        for (k = dstart; k < lparen_idx - 1; k++) {
          if (ftoks[k]->kind == CDD_TOKEN_STAR) {
            rc = sem_buf_append_tok(ret_type, sizeof(ret_type), &rlen, "*", 1);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
        fnl = ftoks[lparen_idx - 1]->length < 127
                  ? ftoks[lparen_idx - 1]->length
                  : 127;
        memcpy(fname, ftoks[lparen_idx - 1]->start, fnl);
        fname[fnl] = '\0';
        rc = sem_register_typed_symbol(env, fname, CDD_CST_SYMBOL_FUNCTION,
                                       ret_type, ctx->is_external ? NULL : node,
                                       NULL);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (fn > 3 && ftoks[0]->length == 3 &&
                 memcmp(ftoks[0]->start, "for", 3) == 0 &&
                 ftoks[1]->kind == CDD_TOKEN_LPAREN) {
        size_t semi = 2;
        while (semi < fn && ftoks[semi]->kind != CDD_TOKEN_SEMICOLON)
          semi++;
        if (semi > 2) {
          int is_td = 0;
          char btype[128];
          size_t dstart = 0;
          rc = sem_extract_decl_specs(env, &ftoks[2], semi - 2, &is_td, btype,
                                      sizeof(btype), &dstart);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (btype[0] != '\0' && 2 + dstart < semi) {
            rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (bind_ast)
              env->current_scope->ast_node = node;
            entered_scope = 1;
            rc = sem_parse_declarator_list(env, btype, 0, NULL,
                                           &ftoks[2 + dstart],
                                           semi - (2 + dstart), NULL);
            if (rc != CDD_C_SUCCESS) {
              return cdd_c_keep_err(rc, cdd_cst_scope_leave(env));
            }
          }
        }
      }
    }

    if (!entered_scope) {
      rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_FUNCTION);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (bind_ast)
        env->current_scope->ast_node = node;
      entered_scope = 1;
    }

    if (ctx && is_real_fn && rparen_idx > lparen_idx + 1) {
      rc = sem_parse_fn_params(env, &ftoks[lparen_idx + 1],
                               rparen_idx - lparen_idx - 1);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_cst_scope_leave(env));
      }
    }
    break;
  }
  case CDD_CST_NAMESPACE_DECLARATION: {
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_NAMESPACE);
    if (rc != CDD_C_SUCCESS)
      return rc;
    entered_scope = 1;
    break;
  }
  case CDD_CST_DECLARATION: {
    char *name = NULL;
    rc = extract_identifier(node, &name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (name) {
      rc = cdd_cst_scope_add_symbol(env, name, CDD_CST_SYMBOL_VARIABLE, node);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(name);
        return rc;
      }
      C_CDD_FREE(name);
    }
    break;
  }
  case CDD_CST_TYPE_SPECIFIER: {
    char *name = NULL;
    rc = extract_identifier(node, &name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (name) {
      rc = cdd_cst_scope_add_symbol(env, name, CDD_CST_SYMBOL_STRUCT_TAG, node);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(name);
        return rc;
      }
      C_CDD_FREE(name);
    }
    break;
  }
  case CDD_CST_PREPROC_DIRECTIVE:
    if (ctx && node->num_children > 0 &&
        node->children[0].kind == CDD_CST_CHILD_TOKEN &&
        node->children[0].val.token->kind == CDD_TOKEN_PREPROC_INCLUDE) {
      rc = sem_scan_include_token(env, node->children[0].val.token, ctx);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    break;
  case CDD_CST_UNKNOWN:
    if (ctx) {
      const cdd_token_t *utoks[128];
      size_t un = 0;
      cdd_cst_node_t *ublk = NULL;
      int is_ctrl = 0;
      rc = sem_collect_direct_tokens(node, utoks, 128, &un, &ublk);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (un > 0) {
        rc = sem_is_control_kw(utoks[0], &is_ctrl);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (!is_ctrl) {
          int is_td = 0;
          char btype[128];
          size_t dstart = 0;
          rc = sem_extract_decl_specs(env, utoks, un, &is_td, btype,
                                      sizeof(btype), &dstart);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (btype[0] != '\0' && dstart < un) {
            rc = sem_parse_declarator_list(env, btype, is_td, NULL,
                                           &utoks[dstart], un - dstart,
                                           ctx->is_external ? NULL : node);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      }
    }
    break;
  default:
    break;
  }

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *ch = node->children[i].val.node;
      if (ctx && (ch->kind == CDD_CST_CLASS_DECLARATION ||
                  ch->kind == CDD_CST_FUNCTION_DEFINITION)) {
        static const char *const k_agg_lead[] = {"typedef", "union", "enum"};
        const cdd_token_t *ctoks[16] = {0};
        size_t cn = 0;
        cdd_cst_node_t *cblk = NULL;
        int m_lead = 0;
        int has_assign = 0;
        rc = sem_collect_direct_tokens(ch, ctoks, 16, &cn, &cblk);
        if (rc != CDD_C_SUCCESS) {
          if (entered_scope) {
            return cdd_c_keep_err(rc, cdd_cst_scope_leave(env));
          }
          return rc;
        }
        {
          size_t ti;
          int pdepth = 0;
          for (ti = 0; ti < cn; ti++) {
            if (ctoks[ti]->kind == CDD_TOKEN_LPAREN)
              pdepth++;
            else if (ctoks[ti]->kind == CDD_TOKEN_RPAREN && pdepth > 0)
              pdepth--;
            else if (pdepth == 0) {
              if (ctoks[ti]->kind == CDD_TOKEN_ASSIGN) {
                has_assign = 1;
              } else {
                int m_cur = 0;
                rc = sem_tok_in_table(
                    ctoks[ti], k_agg_lead,
                    sizeof(k_agg_lead) / sizeof(k_agg_lead[0]), &m_cur);
                if (rc != CDD_C_SUCCESS) {
                  if (entered_scope) {
                    return cdd_c_keep_err(rc, cdd_cst_scope_leave(env));
                  }
                  return rc;
                }
                if (ctoks[ti]->kind == CDD_TOKEN_KEYWORD_STRUCT || m_cur) {
                  m_lead = 1;
                }
              }
            }
          }
        }
        if (cblk && !has_assign &&
            (ch->kind == CDD_CST_CLASS_DECLARATION || m_lead)) {
          cdd_cst_node_t *next_sib = NULL;
          int consumed_next = 0;
          if (i + 1 < node->num_children &&
              node->children[i + 1].kind == CDD_CST_CHILD_NODE) {
            next_sib = node->children[i + 1].val.node;
          }
          rc = sem_analyze_aggregate(env, ch, next_sib, ctx, &consumed_next);
          if (rc != CDD_C_SUCCESS) {
            if (entered_scope) {
              return cdd_c_keep_err(rc, cdd_cst_scope_leave(env));
            }
            return rc;
          }
          if (consumed_next)
            i++;
          continue;
        }
      }

      rc = analyze_node_ctx(env, ch, ctx, bind_ast);
      if (rc != CDD_C_SUCCESS) {
        if (entered_scope) {
          return cdd_c_keep_err(rc, cdd_cst_scope_leave(env));
        }
        return rc;
      }
    }
  }

  if (entered_scope) {
    return cdd_cst_scope_leave(env);
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Build semantic scope and symbol information from a CST with optional
 * `-I` include directory header pre-scanning.
 *
 * @param[in,out] tree The CST tree to analyze.
 * @param[in] include_dirs Optional array of header search directory paths.
 * @param[in] num_include_dirs Number of entries in `include_dirs`.
 * @param[out] out_env Output pointer receiving the populated scope environment.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_build_semantic_info_ex(cdd_cst_tree_t *tree,
                                             const char *const *include_dirs,
                                             size_t num_include_dirs,
                                             cdd_cst_scope_env_t **out_env) {
  cdd_cst_scope_env_t *env = NULL;
  struct cdd_semantic_scan_ctx_t ctx;
  struct cdd_semantic_scan_ctx_t *ctx_ptr = NULL;
  cdd_c_error_t rc;
  size_t v;

  if (!tree || !out_env)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_cst_scope_env_init(&env);
  if (rc != CDD_C_SUCCESS)
    return rc;

  memset(&ctx, 0, sizeof(ctx));
  ctx.include_dirs = include_dirs;
  ctx.num_include_dirs = num_include_dirs;

  if (tree->base_tokens != NULL) {
    ctx_ptr = &ctx;
    if (tree->root)
      env->global_scope->ast_node = tree->root;
  }

  rc = analyze_node_ctx(env, tree->root, ctx_ptr, tree->base_tokens != NULL);
  if (rc != CDD_C_SUCCESS) {
    for (v = 0; v < ctx.num_visited; v++) {
      C_CDD_FREE(ctx.visited[v]);
    }
    return cdd_c_keep_err(rc, cdd_cst_scope_env_free(env));
  }
  for (v = 0; v < ctx.num_visited; v++) {
    C_CDD_FREE(ctx.visited[v]);
  }

  env->bound_tree = tree;
  tree->active_sem_env = env;
  *out_env = env;
  return CDD_C_SUCCESS;
}

/*
 * @brief Builds semantic scope and symbol information for an entire CST tree.
 *
 * @param[in] tree CST tree to analyze.
 * @param[out] out_env Output pointer receiving the populated scope environment.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_build_semantic_info(cdd_cst_tree_t *tree,
                                          cdd_cst_scope_env_t **out_env) {
  return cdd_cst_build_semantic_info_ex(tree, NULL, 0, out_env);
}
