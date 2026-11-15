/**
 * @file cdd_ffi_emit_fortran.c
 * @brief Production ISO Fortran 2003/2008 (`ISO_C_BINDING`) FFI binding emitter
 * implementation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_fortran.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_ffi_emit_common.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief ISO Fortran maximum identifier length (Fortran 2003/2008). */
#define FORTRAN_MAX_IDENT_LEN 63

/** @brief Maximum column width before free-form `&` continuation wrapping. */
#define FORTRAN_WRAP_COL 120

/** @brief Fortran `! ` line comment style wrapped within 120 columns. */
static const cdd_ffi_comment_style_t k_fortran_comment_style = {
    "! ", NULL, NULL, NULL, NULL, NULL, NULL, FORTRAN_WRAP_COL};

/** @brief Complete table of ISO Fortran reserved keywords and intrinsic names.
 */
static const char *const k_fortran_keywords[] = {"module",
                                                 "program",
                                                 "subroutine",
                                                 "function",
                                                 "interface",
                                                 "type",
                                                 "end",
                                                 "use",
                                                 "import",
                                                 "value",
                                                 "intent",
                                                 "in",
                                                 "out",
                                                 "inout",
                                                 "result",
                                                 "integer",
                                                 "real",
                                                 "logical",
                                                 "character",
                                                 "complex",
                                                 "data",
                                                 "common",
                                                 "call",
                                                 "return",
                                                 "stop",
                                                 "do",
                                                 "if",
                                                 "then",
                                                 "else",
                                                 "select",
                                                 "case",
                                                 "where",
                                                 "forall",
                                                 "target",
                                                 "pointer",
                                                 "allocatable",
                                                 "dimension",
                                                 "parameter",
                                                 "bind",
                                                 "pure",
                                                 "elemental",
                                                 "recursive",
                                                 "abstract",
                                                 "contains",
                                                 "enum",
                                                 "enumerator",
                                                 "private",
                                                 "public",
                                                 "implicit",
                                                 "none",
                                                 "intrinsic",
                                                 "save",
                                                 "external",
                                                 "procedure",
                                                 "class",
                                                 "extends",
                                                 "generic",
                                                 "final",
                                                 "pass",
                                                 "nopass",
                                                 "deferred",
                                                 "non_overridable",
                                                 "volatile",
                                                 "asynchronous",
                                                 "contiguous",
                                                 "codimension",
                                                 "block",
                                                 "associate",
                                                 "critical",
                                                 "error",
                                                 "submodule",
                                                 "c_ptr",
                                                 "c_funptr",
                                                 "c_null_ptr",
                                                 "c_null_funptr",
                                                 "c_null_char",
                                                 "c_int",
                                                 "c_int8_t",
                                                 "c_int16_t",
                                                 "c_int32_t",
                                                 "c_int64_t",
                                                 "c_size_t",
                                                 "c_float",
                                                 "c_double",
                                                 "c_bool",
                                                 "c_char",
                                                 "c_sizeof",
                                                 "c_associated",
                                                 "c_f_pointer",
                                                 "c_f_procpointer",
                                                 "c_loc",
                                                 "c_funloc",
                                                 "transfer",
                                                 "trim",
                                                 "size",
                                                 "min",
                                                 "cdd_ret_"};

/**
 * @brief Case-insensitive scope symbol table to disambiguate C identifiers like
 * `Foo` vs `foo` in Fortran's case-insensitive namespace.
 */
typedef struct fortran_scope_t {
  char **names; /**< Array of lowercase Fortran names registered in scope */
  size_t count; /**< Number of registered names */
  size_t cap;   /**< Allocated capacity of `names` */
} fortran_scope_t;

/**
 * @brief Frees all entries in a `fortran_scope_t`.
 *
 * @param[in,out] scope Scope table to clean up.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t fortran_scope_free(fortran_scope_t *scope) {
  size_t i;
  for (i = 0; i < scope->count; i++) {
    C_CDD_FREE(scope->names[i]);
  }
  C_CDD_FREE(scope->names);
  scope->names = NULL;
  scope->count = 0;
  scope->cap = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Registers a sanitized lowercase identifier in `scope`, appending `_2`,
 * `_3`, ... on case-insensitive collision while staying `<= 63` characters.
 *
 * @param[in,out] scope Active Fortran scope table.
 * @param[in,out] io_ident Identifier buffer (`ident_cap >= 64`) to disambiguate
 * and register.
 * @param[in] ident_cap Capacity of `io_ident`.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static NO_DISCARD cdd_c_error_t fortran_scope_register(fortran_scope_t *scope,
                                                       char *io_ident,
                                                       size_t ident_cap) {
  char base[FORTRAN_MAX_IDENT_LEN + 1];
  size_t max_len = ident_cap - 1;
  size_t suffix_num = 2;
  cdd_c_error_t rc;

  rc = cdd_ffi_snprintf_checked(base, sizeof(base), "%s", io_ident);

  for (;;) {
    size_t i;
    int collision = 0;
    for (i = 0; i < scope->count; i++) {
      if (strcmp(scope->names[i], io_ident) == 0) {
        collision = 1;
        break;
      }
    }
    if (!collision)
      break;

    {
      char sfx[24];
      size_t sfx_len;
      size_t base_len;
      rc = cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(sfx, sizeof(sfx), "_%lu",
                                       (unsigned long)suffix_num++));
      sfx_len = strlen(sfx);
      base_len = strlen(base);
      if (base_len + sfx_len > max_len) {
        base_len = max_len - sfx_len;
      }
      memcpy(io_ident, base, base_len);
      memcpy(io_ident + base_len, sfx, sfx_len + 1);
    }
  }

  if (scope->count == scope->cap) {
    size_t new_cap = scope->cap == 0 ? 32 : scope->cap * 2;
    char **new_arr =
        (char **)C_CDD_REALLOC(scope->names, new_cap * sizeof(char *));
    if (!new_arr)
      return CDD_C_ERROR_MEMORY;
    scope->names = new_arr;
    scope->cap = new_cap;
  }
  scope->names[scope->count] = C_CDD_STRDUP(io_ident);
  if (!scope->names[scope->count])
    return CDD_C_ERROR_MEMORY;
  scope->count++;
  return rc;
}

/**
 * @brief Checks if `word` (already lowercase) is a Fortran keyword/intrinsic.
 *
 * @param[in] word Candidate lowercase identifier.
 * @param[out] out_reserved Receives 1 if reserved, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t fortran_is_reserved(const char *word,
                                                    int *out_reserved) {
  size_t i;
  size_t count;

  *out_reserved = 0;
  count = sizeof(k_fortran_keywords) / sizeof(k_fortran_keywords[0]);
  for (i = 0; i < count; i++) {
    if (strcmp(word, k_fortran_keywords[i]) == 0) {
      *out_reserved = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes a deterministic 16-bit hash for truncated identifier
 * disambiguation.
 *
 * @param[in] s Input string.
 * @param[out] out_hash Receives 16-bit FNV-style hash.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t fortran_hash16(const char *s,
                                               unsigned int *out_hash) {
  unsigned int h = 2166136261u;
  while (*s != '\0') {
    h ^= (unsigned int)(unsigned char)(*s++);
    h *= 16777619u;
  }
  *out_hash = (h ^ (h >> 16)) & 0xFFFFu;
  return CDD_C_SUCCESS;
}

/*
 * @brief Sanitizes a C identifier into a valid lowercase Fortran identifier
 * (`<= 63` characters), handling NULL/empty fallbacks and reserved words.
 *
 * @param[in] c_ident Source C identifier (may be NULL or empty).
 * @param[in] fallback_idx Fallback parameter/field index when `c_ident` is
 * NULL/empty.
 * @param[out] out_buf Destination buffer receiving the sanitized identifier.
 * @param[in] out_cap Capacity of `out_buf` in bytes (must be >= 8).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
NO_DISCARD cdd_c_error_t cdd_fortran_sanitize_ident(const char *c_ident,
                                                    size_t fallback_idx,
                                                    char *out_buf,
                                                    size_t out_cap) {
  char tmp[256];
  size_t pos;
  size_t i;
  size_t eff_max;
  int is_res = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!out_buf || out_cap < 8)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!c_ident || c_ident[0] == '\0') {
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "arg_%lu",
                                    (unsigned long)fallback_idx);
  }

  eff_max = out_cap - 1;
  if (eff_max > FORTRAN_MAX_IDENT_LEN)
    eff_max = FORTRAN_MAX_IDENT_LEN;

  pos = 0;
  for (i = 0; c_ident[i] != '\0'; i++) {
    unsigned char ch = (unsigned char)c_ident[i];
    char mapped;
    if (isupper(ch)) {
      mapped = (char)tolower(ch);
    } else if (isalnum(ch) || ch == '_') {
      mapped = (char)ch;
    } else {
      mapped = '_';
    }

    if (pos == 0) {
      if (!islower((unsigned char)mapped)) {
        tmp[pos++] = 'f';
        if (mapped != '_')
          tmp[pos++] = '_';
      }
    } else if (mapped == '_' && tmp[pos - 1] == '_') {
      continue;
    }
    if (pos + 1 < sizeof(tmp)) {
      tmp[pos++] = mapped;
    }
  }
  while (pos > 1 && tmp[pos - 1] == '_')
    pos--;
  tmp[pos] = '\0';

  if (pos > eff_max) {
    unsigned int h = 0;
    size_t keep;
    rc = cdd_c_keep_err(rc, fortran_hash16(c_ident, &h));
    keep = eff_max - 5;
    tmp[keep] = '\0';
    return cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(out_buf, out_cap, "%s_%04x", tmp, h));
  }

  rc = cdd_c_keep_err(rc, fortran_is_reserved(tmp, &is_res));
  if (is_res) {
    if (pos + 1 > eff_max)
      tmp[eff_max - 1] = '\0';
    return cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(out_buf, out_cap, "%s_", tmp));
  }

  return cdd_c_keep_err(rc,
                        cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", tmp));
}

/*
 * @brief Writes a single free-form Fortran code line to `fp`, wrapping lines
 * exceeding 120 columns using valid ISO Fortran free-form `&` continuations.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] line Single Fortran source line (with or without trailing `\n`).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * or CDD_C_ERROR_IO on stream write failure.
 */
NO_DISCARD cdd_c_error_t cdd_fortran_write_wrapped_line(FILE *fp,
                                                        const char *line) {
  size_t len;
  size_t pos;
  int in_quote;
  char quote_ch;
  cdd_c_error_t rc;

  if (!fp || !line)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  len = strlen(line);
  while (len > 0 && memchr("\r\n", (unsigned char)line[len - 1], 2) != NULL) {
    len--;
  }

  if (len <= FORTRAN_WRAP_COL) {
    if (len > 0) {
      rc = cdd_ffi_write_bytes(fp, line, len);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return cdd_ffi_write_str(fp, "\n");
  }

  /* Check if comment line */
  {
    size_t lead = 0;
    while (lead < len && memchr(" \t", (unsigned char)line[lead], 2) != NULL)
      lead++;
    if (lead < len && line[lead] == '!') {
      rc = cdd_ffi_write_bytes(fp, line, FORTRAN_WRAP_COL);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_ffi_write_str(fp, "\n");
      if (rc != CDD_C_SUCCESS)
        return rc;
      pos = FORTRAN_WRAP_COL;
      while (pos < len) {
        size_t chunk = (len - pos > FORTRAN_WRAP_COL - 4)
                           ? (FORTRAN_WRAP_COL - 4)
                           : (len - pos);
        rc = cdd_ffi_write_str(fp, "  ! ");
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = cdd_ffi_write_bytes(fp, line + pos, chunk);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = cdd_ffi_write_str(fp, "\n");
        if (rc != CDD_C_SUCCESS)
          return rc;
        pos += chunk;
      }
      return CDD_C_SUCCESS;
    }
  }

  pos = 0;
  in_quote = 0;
  quote_ch = '\0';

  for (;;) {
    size_t prefix_cols = (pos == 0) ? 0 : 7; /* "      &" */
    size_t rem = len - pos;
    size_t budget;
    size_t split_rel;
    size_t k;
    int cur_q;
    char cur_qch;
    int split_in_quote;
    int forced_split;

    if (prefix_cols + rem <= FORTRAN_WRAP_COL) {
      rc = cdd_ffi_write_str(fp, "      &");
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (rem > 0) {
        rc = cdd_ffi_write_bytes(fp, line + pos, rem);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      return cdd_ffi_write_str(fp, "\n");
    }

    budget = FORTRAN_WRAP_COL - prefix_cols - 2; /* leave room for " &" */
    split_rel = 0;
    cur_q = in_quote;
    cur_qch = quote_ch;

    for (k = 0; k < budget; k++) {
      char c = line[pos + k];
      if (cur_q) {
        if (c == cur_qch) {
          cur_q = 0;
          cur_qch = '\0';
        }
      } else {
        if (c == '"' || c == '\'') {
          cur_q = 1;
          cur_qch = c;
        } else if (memchr(", (", (unsigned char)c, 3) != NULL) {
          split_rel = k + 1;
        }
      }
    }

    forced_split = (split_rel == 0) ? 1 : 0;
    if (forced_split)
      split_rel = budget;

    cur_q = in_quote;
    cur_qch = quote_ch;
    for (k = 0; k < split_rel; k++) {
      char c = line[pos + k];
      if (cur_q && c == cur_qch) {
        cur_q = 0;
        cur_qch = '\0';
      } else if (!cur_q && (c == '"' || c == '\'')) {
        cur_q = 1;
        cur_qch = c;
      }
    }
    split_in_quote = forced_split ? cur_q : 0;

    if (pos > 0) {
      rc = cdd_ffi_write_str(fp, "      &");
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = cdd_ffi_write_bytes(fp, line + pos, split_rel);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_ffi_write_str(fp, split_in_quote ? "&\n" : " &\n");
    if (rc != CDD_C_SUCCESS)
      return rc;

    pos += split_rel;
    if (!cur_q) {
      while (pos < len && line[pos] == ' ')
        pos++;
    }
    in_quote = cur_q;
    quote_ch = cur_qch;
  }
}

/**
 * @brief Formats a `printf`-style line and writes it via
 * `cdd_fortran_write_wrapped_line`.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] fmt Printf-style format string.
 * @return CDD_C_SUCCESS on success, or error code on formatting/I/O failure.
 */
#if defined(__GNUC__) || defined(__clang__)
static NO_DISCARD cdd_c_error_t fortran_write_line_fmt(FILE *fp,
                                                       const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#endif
static NO_DISCARD cdd_c_error_t fortran_write_line_fmt(FILE *fp,
                                                       const char *fmt, ...) {
  char buf[4096];
  va_list ap;
  cdd_c_error_t rc;

  va_start(ap, fmt);
  rc = cdd_ffi_vsnprintf_checked(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  return cdd_c_keep_err(rc, cdd_fortran_write_wrapped_line(fp, buf));
}

/**
 * @brief Normalizes a C integer/enum literal (including C23 hex `0x`, octal
 * `0`, binary `0b`, and `'` digit separators) into an ISO Fortran 2008 constant
 * expression.
 *
 * @param[in] raw Raw C integer literal string.
 * @param[in] kind_name Fortran kind parameter (e.g. `"C_INT"` or
 * `"C_INT64_T"`).
 * @param[out] out_buf Destination buffer.
 * @param[in] out_cap Capacity of `out_buf`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t fortran_normalize_int_literal(
    const char *raw, const char *kind_name, char *out_buf, size_t out_cap) {
  const char *p;
  size_t len;
  size_t i;
  char digits[128];
  size_t dpos = 0;
  int neg = 0;

  if (raw[0] == '\0')
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "0_%s", kind_name);

  p = raw;
  while (memchr(" \t(", (unsigned char)*p, 3) != NULL)
    p++;
  if (*p == '-') {
    neg = 1;
    p++;
  } else if (*p == '+') {
    p++;
  }
  while (memchr(" \t", (unsigned char)*p, 2) != NULL)
    p++;

  len = strlen(p);
  while (len > 0 && memchr(" \t)uUlL", (unsigned char)p[len - 1], 7) != NULL) {
    len--;
  }
  if (len == 0)
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "0_%s", kind_name);

  if (len > 2 && p[0] == '0' && memchr("xX", (unsigned char)p[1], 2) != NULL) {
    for (i = 2; i < len && dpos + 1 < sizeof(digits); i++) {
      if (p[i] == '\'')
        continue;
      if (!isxdigit((unsigned char)p[i]))
        return cdd_ffi_snprintf_checked(out_buf, out_cap, "0_%s", kind_name);
      digits[dpos++] = (char)toupper((unsigned char)p[i]);
    }
    digits[dpos] = '\0';
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%sINT(Z'%s', %s)",
                                    neg ? "-" : "", digits, kind_name);
  }

  if (len > 2 && p[0] == '0' && memchr("bB", (unsigned char)p[1], 2) != NULL) {
    for (i = 2; i < len && dpos + 1 < sizeof(digits); i++) {
      if (p[i] == '\'')
        continue;
      if (p[i] != '0' && p[i] != '1')
        return cdd_ffi_snprintf_checked(out_buf, out_cap, "0_%s", kind_name);
      digits[dpos++] = p[i];
    }
    digits[dpos] = '\0';
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%sINT(B'%s', %s)",
                                    neg ? "-" : "", digits, kind_name);
  }

  if (len > 1 && p[0] == '0' && p[1] >= '0' && p[1] <= '7') {
    for (i = 1; i < len && dpos + 1 < sizeof(digits); i++) {
      if (p[i] == '\'')
        continue;
      if (p[i] < '0' || p[i] > '7')
        return cdd_ffi_snprintf_checked(out_buf, out_cap, "0_%s", kind_name);
      digits[dpos++] = p[i];
    }
    digits[dpos] = '\0';
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%sINT(O'%s', %s)",
                                    neg ? "-" : "", digits, kind_name);
  }

  for (i = 0; i < len && dpos + 1 < sizeof(digits); i++) {
    if (p[i] == '\'')
      continue;
    if (!isdigit((unsigned char)p[i]))
      return cdd_ffi_snprintf_checked(out_buf, out_cap, "0_%s", kind_name);
    digits[dpos++] = p[i];
  }
  digits[dpos] = '\0';
  if (dpos == 0)
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "0_%s", kind_name);
  return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s%s_%s", neg ? "-" : "",
                                  digits, kind_name);
}

/**
 * @brief Normalizes a C floating-point literal into a Fortran `REAL(C_DOUBLE)`
 * literal.
 *
 * @param[in] raw Raw C float literal.
 * @param[out] out_buf Destination buffer.
 * @param[in] out_cap Capacity of `out_buf`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t fortran_normalize_float_literal(
    const char *raw, char *out_buf, size_t out_cap) {
  char clean[128];
  size_t pos = 0;
  size_t i;
  int has_dot = 0;

  if (raw[0] == '\0')
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0.0_C_DOUBLE");

  for (i = 0; raw[i] != '\0' && pos + 4 < sizeof(clean); i++) {
    char c = raw[i];
    if (c == '\'')
      continue;
    if (isdigit((unsigned char)c) ||
        memchr(".eE+-", (unsigned char)c, 5) != NULL) {
      if (memchr(".eE", (unsigned char)c, 3) != NULL)
        has_dot = 1;
      clean[pos++] = c;
    }
  }
  if (pos == 0)
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0.0_C_DOUBLE");
  if (!has_dot) {
    clean[pos++] = '.';
    clean[pos++] = '0';
  }
  clean[pos] = '\0';
  return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s_C_DOUBLE", clean);
}

/**
 * @brief Looks up a struct/union/enum/typedef node in `ir` and obtains its
 * mapped Fortran identifier from `node_fnames`.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] node_fnames Parallel array of disambiguated Fortran node names.
 * @param[in] ref_name C reference name.
 * @param[out] out_node Receives matched IR node pointer or NULL.
 * @param[out] out_fname Receives disambiguated Fortran name or NULL.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t fortran_resolve_ref(
    const cdd_ffi_ir_t *ir, char **node_fnames, const char *ref_name,
    const cdd_ffi_ir_node_t **out_node, const char **out_fname) {
  size_t i;

  *out_node = NULL;
  *out_fname = NULL;
  if (!ref_name)
    return CDD_C_SUCCESS;

  for (i = 0; i < ir->nodes_count; i++) {
    if (ir->nodes[i].name && strcmp(ir->nodes[i].name, ref_name) == 0 &&
        ir->nodes[i].kind != CDD_FFI_NODE_FUNCTION &&
        ir->nodes[i].kind != CDD_FFI_NODE_MACRO) {
      *out_node = &ir->nodes[i];
      *out_fname = node_fnames[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends a symbol name to a comma-separated `IMPORT ::` list if not
 * already present.
 *
 * @param[in,out] io_list Comma-separated symbol list buffer.
 * @param[in] list_cap Capacity of `io_list`.
 * @param[in] sym Non-empty symbol identifier to add.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t fortran_import_add(char *io_list,
                                                   size_t list_cap,
                                                   const char *sym) {
  const char *p;
  size_t sym_len;
  char tmp[1024];
  cdd_c_error_t rc;

  sym_len = strlen(sym);
  p = io_list;
  while ((p = strstr(p, sym)) != NULL) {
    int left_ok = (p == io_list || p[-1] == ' ');
    int right_ok = (p[sym_len] == '\0' || p[sym_len] == ',');
    if (left_ok && right_ok)
      return CDD_C_SUCCESS;
    p += sym_len;
  }

  if (io_list[0] == '\0') {
    return cdd_ffi_snprintf_checked(io_list, list_cap, "%s", sym);
  }
  rc = cdd_ffi_snprintf_checked(tmp, sizeof(tmp), "%s, %s", io_list, sym);
  return cdd_c_keep_err(rc,
                        cdd_ffi_snprintf_checked(io_list, list_cap, "%s", tmp));
}

/**
 * @brief Maps an unindirected (`pointer_depth == 0`, `array_size <= 0`) FFI
 * base type to its Fortran `ISO_C_BINDING` specifier and records its required
 * `IMPORT ::` symbol.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] node_fnames Parallel array of disambiguated Fortran node names.
 * @param[in] type FFI type descriptor (base kind).
 * @param[out] out_spec Destination buffer for Fortran type specifier.
 * @param[in] spec_cap Capacity of `out_spec`.
 * @param[in,out] io_imports Optional `IMPORT ::` accumulator buffer (may be
 * NULL).
 * @param[in] imports_cap Capacity of `io_imports`.
 * @param[out] out_is_bindable Receives 1 if passable as typed pointee, 0 if
 * opaque/void.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t fortran_map_base_type(
    const cdd_ffi_ir_t *ir, char **node_fnames, const cdd_ffi_type_t *type,
    char *out_spec, size_t spec_cap, char *io_imports, size_t imports_cap,
    int *out_is_bindable) {
  const char *imp = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  *out_is_bindable = 1;

  switch (type->kind) {
  case CDD_FFI_KIND_BOOL:
    imp = "C_BOOL";
    rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "LOGICAL(C_BOOL)");
    break;
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
    imp = "C_INT8_T";
    rc =
        cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "INTEGER(C_INT8_T)");
    break;
  case CDD_FFI_KIND_INT16:
  case CDD_FFI_KIND_UINT16:
    imp = "C_INT16_T";
    rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s",
                                  "INTEGER(C_INT16_T)");
    break;
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_UINT32:
    imp = "C_INT32_T";
    rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s",
                                  "INTEGER(C_INT32_T)");
    break;
  case CDD_FFI_KIND_INT64:
  case CDD_FFI_KIND_UINT64:
    imp = "C_INT64_T";
    rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s",
                                  "INTEGER(C_INT64_T)");
    break;
  case CDD_FFI_KIND_FLOAT32:
    imp = "C_FLOAT";
    rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "REAL(C_FLOAT)");
    break;
  case CDD_FFI_KIND_FLOAT64:
    imp = "C_DOUBLE";
    rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "REAL(C_DOUBLE)");
    break;
  case CDD_FFI_KIND_ENUM_REF:
    imp = "C_INT";
    rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "INTEGER(C_INT)");
    break;
  case CDD_FFI_KIND_TYPEDEF_REF: {
    const cdd_ffi_ir_node_t *rn = NULL;
    const char *rfn = NULL;
    if (type->ref_name && (strcmp(type->ref_name, "size_t") == 0 ||
                           strcmp(type->ref_name, "uintptr_t") == 0)) {
      imp = "C_SIZE_T";
      rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s",
                                    "INTEGER(C_SIZE_T)");
      break;
    }
    rc = fortran_resolve_ref(ir, node_fnames, type->ref_name, &rn, &rfn);
    if (rn && rn->kind == CDD_FFI_NODE_TYPEDEF) {
      return cdd_c_keep_err(
          rc, fortran_map_base_type(ir, node_fnames, &rn->return_or_base_type,
                                    out_spec, spec_cap, io_imports, imports_cap,
                                    out_is_bindable));
    }
    *out_is_bindable = 0;
    imp = "C_PTR";
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "TYPE(C_PTR)"));
    break;
  }
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_TEMPLATE_STRUCT_REF: {
    const cdd_ffi_ir_node_t *rn = NULL;
    const char *rfn = NULL;
    rc = fortran_resolve_ref(ir, node_fnames, type->ref_name, &rn, &rfn);
    if (rn &&
        (rn->kind == CDD_FFI_NODE_STRUCT || rn->kind == CDD_FFI_NODE_UNION)) {
      if (rn->fields_count == 0) {
        *out_is_bindable = 0;
      }
      imp = rfn;
      rc = cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(out_spec, spec_cap, "TYPE(%s)", rfn));
    } else {
      *out_is_bindable = 0;
      imp = "C_PTR";
      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s",
                                                       "TYPE(C_PTR)"));
    }
    break;
  }
  case CDD_FFI_KIND_FUNCTION_PTR:
    *out_is_bindable = 0;
    imp = "C_FUNPTR";
    rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "TYPE(C_FUNPTR)");
    break;
  case CDD_FFI_KIND_VOID:
  case CDD_FFI_KIND_OPAQUE_PTR:
  default:
    *out_is_bindable = 0;
    imp = "C_PTR";
    rc = cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "TYPE(C_PTR)");
    break;
  }

  if (io_imports) {
    rc = cdd_c_keep_err(rc, fortran_import_add(io_imports, imports_cap, imp));
  }
  return rc;
}

/**
 * @brief Formats a dummy argument declaration inside a `BIND(C)` `INTERFACE`
 * block with exact ISO Fortran 2003/2008 `VALUE`, `INTENT`, and `DIMENSION`
 * attributes.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] node_fnames Parallel array of disambiguated Fortran node names.
 * @param[in] fld Parameter field descriptor.
 * @param[in] arg_name Sanitized Fortran dummy argument name.
 * @param[out] out_decl Destination buffer for full declaration line.
 * @param[in] decl_cap Capacity of `out_decl`.
 * @param[in,out] io_imports `IMPORT ::` symbol list accumulator.
 * @param[in] imports_cap Capacity of `io_imports`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t fortran_format_param_decl(
    const cdd_ffi_ir_t *ir, char **node_fnames, const cdd_ffi_field_t *fld,
    const char *arg_name, char *out_decl, size_t decl_cap, char *io_imports,
    size_t imports_cap) {
  char base_spec[256];
  int is_bindable = 0;
  cdd_c_error_t rc;

  /* Fixed-size C array parameter (`T arr[N]`): pass by reference, NO VALUE */
  if (fld->type.array_size > 0 && fld->type.pointer_depth == 0) {
    const char *intent_attr = "";
    if (fld->intent == CDD_FFI_INTENT_OUT)
      intent_attr = ", INTENT(OUT)";
    else if (fld->intent == CDD_FFI_INTENT_INOUT)
      intent_attr = ", INTENT(INOUT)";
    else if (fld->type.is_const || fld->intent == CDD_FFI_INTENT_IN)
      intent_attr = ", INTENT(IN)";

    if (fld->type.kind == CDD_FFI_KIND_INT8) {
      rc = fortran_import_add(io_imports, imports_cap, "C_CHAR");
      return cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(
                  out_decl, decl_cap,
                  "      CHARACTER(KIND=C_CHAR), DIMENSION(%ld)%s :: %s",
                  fld->type.array_size, intent_attr, arg_name));
    }
    rc = fortran_map_base_type(ir, node_fnames, &fld->type, base_spec,
                               sizeof(base_spec), io_imports, imports_cap,
                               &is_bindable);
    return cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                  out_decl, decl_cap,
                                  "      %s, DIMENSION(%ld)%s :: %s", base_spec,
                                  fld->type.array_size, intent_attr, arg_name));
  }

  /* Pass-by-value C scalars, function pointers, opaque handles, and structs */
  if (fld->type.pointer_depth == 0) {
    rc = fortran_map_base_type(ir, node_fnames, &fld->type, base_spec,
                               sizeof(base_spec), io_imports, imports_cap,
                               &is_bindable);
    return cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(out_decl, decl_cap,
                                                       "      %s, VALUE :: %s",
                                                       base_spec, arg_name));
  }

  /* Single-indirection C pointers (`pointer_depth == 1`) */
  if (fld->type.pointer_depth == 1) {
    /* `const char *` C string input */
    if ((fld->type.kind == CDD_FFI_KIND_INT8 ||
         fld->type.kind == CDD_FFI_KIND_UINT8) &&
        (fld->type.is_const || fld->intent == CDD_FFI_INTENT_IN) &&
        fld->intent != CDD_FFI_INTENT_OUT &&
        fld->intent != CDD_FFI_INTENT_INOUT) {
      rc = fortran_import_add(io_imports, imports_cap, "C_CHAR");
      return cdd_c_keep_err(
          rc,
          cdd_ffi_snprintf_checked(
              out_decl, decl_cap,
              "      CHARACTER(KIND=C_CHAR), DIMENSION(*), INTENT(IN) :: %s",
              arg_name));
    }

    if (fld->type.kind != CDD_FFI_KIND_VOID &&
        fld->type.kind != CDD_FFI_KIND_OPAQUE_PTR &&
        fld->type.kind != CDD_FFI_KIND_FUNCTION_PTR) {
      char trial_imports[1024];
      rc = cdd_ffi_snprintf_checked(trial_imports, sizeof(trial_imports), "%s",
                                    io_imports);
      rc = cdd_c_keep_err(
          rc, fortran_map_base_type(ir, node_fnames, &fld->type, base_spec,
                                    sizeof(base_spec), trial_imports,
                                    sizeof(trial_imports), &is_bindable));

      if (is_bindable) {
        const char *ptr_intent = NULL;
        if (fld->intent == CDD_FFI_INTENT_OUT) {
          ptr_intent = "OUT";
        } else if (fld->intent == CDD_FFI_INTENT_INOUT) {
          ptr_intent = "INOUT";
        } else if (fld->type.is_const || fld->intent == CDD_FFI_INTENT_IN) {
          ptr_intent = "IN";
        }
        if (ptr_intent) {
          rc = cdd_c_keep_err(rc,
                              cdd_ffi_snprintf_checked(io_imports, imports_cap,
                                                       "%s", trial_imports));
          return cdd_c_keep_err(
              rc, cdd_ffi_snprintf_checked(out_decl, decl_cap,
                                           "      %s, INTENT(%s) :: %s",
                                           base_spec, ptr_intent, arg_name));
        }
      }
    }
  }

  /* Generic `void *`, opaque struct pointers, unannotated pointers, `T **` */
  rc = fortran_import_add(io_imports, imports_cap, "C_PTR");
  return cdd_c_keep_err(
      rc, cdd_ffi_snprintf_checked(out_decl, decl_cap,
                                   "      TYPE(C_PTR), VALUE :: %s", arg_name));
}

/**
 * @brief Formats a function return type declaration inside a `BIND(C)`
 * `FUNCTION ... RESULT(cdd_ret_)` block.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] node_fnames Parallel array of disambiguated Fortran node names.
 * @param[in] ret_type Return FFI type descriptor.
 * @param[out] out_spec Destination buffer for return Fortran type specifier.
 * @param[in] spec_cap Capacity of `out_spec`.
 * @param[in,out] io_imports `IMPORT ::` symbol list accumulator.
 * @param[in] imports_cap Capacity of `io_imports`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t fortran_format_return_spec(
    const cdd_ffi_ir_t *ir, char **node_fnames, const cdd_ffi_type_t *ret_type,
    char *out_spec, size_t spec_cap, char *io_imports, size_t imports_cap) {
  int is_bindable = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (ret_type->pointer_depth > 0 ||
      ret_type->kind == CDD_FFI_KIND_OPAQUE_PTR) {
    if (io_imports) {
      rc = cdd_c_keep_err(rc,
                          fortran_import_add(io_imports, imports_cap, "C_PTR"));
    }
    return cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "TYPE(C_PTR)"));
  }
  if (ret_type->kind == CDD_FFI_KIND_FUNCTION_PTR) {
    if (io_imports) {
      rc = cdd_c_keep_err(
          rc, fortran_import_add(io_imports, imports_cap, "C_FUNPTR"));
    }
    return cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s",
                                                       "TYPE(C_FUNPTR)"));
  }
  rc = fortran_map_base_type(ir, node_fnames, ret_type, out_spec, spec_cap,
                             io_imports, imports_cap, &is_bindable);
  if (!is_bindable) {
    if (io_imports) {
      rc = cdd_c_keep_err(rc,
                          fortran_import_add(io_imports, imports_cap, "C_PTR"));
    }
    return cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(out_spec, spec_cap, "%s", "TYPE(C_PTR)"));
  }
  return rc;
}

/**
 * @brief Emits an `ABSTRACT INTERFACE` procedure block for a typed
 * `CDD_FFI_KIND_FUNCTION_PTR` signature (usable with `C_F_PROCPOINTER` and
 * `C_FUNLOC`).
 *
 * @param[in,out] fp Open `.f90` file stream.
 * @param[in] ir Enclosing FFI IR.
 * @param[in] node_fnames Parallel array of disambiguated Fortran node names.
 * @param[in] iface_name Sanitized abstract procedure name.
 * @param[in] fp_type Typed `CDD_FFI_KIND_FUNCTION_PTR` descriptor.
 * @return CDD_C_SUCCESS on success, or error code on I/O/OOM failure.
 */
static NO_DISCARD cdd_c_error_t fortran_emit_callback_abstract_iface(
    FILE *fp, const cdd_ffi_ir_t *ir, char **node_fnames,
    const char *iface_name, const cdd_ffi_type_t *fp_type) {
  fortran_scope_t arg_scope;
  char iface_reg[FORTRAN_MAX_IDENT_LEN + 1];
  char(*arg_names)[FORTRAN_MAX_IDENT_LEN + 1] = NULL;
  char imports[1024];
  char header_line[2048];
  char ret_spec[256];
  int is_void;
  size_t k;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  memset(&arg_scope, 0, sizeof(arg_scope));
  imports[0] = '\0';
  ret_spec[0] = '\0';

  rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(iface_reg, sizeof(iface_reg),
                                                   "%s", iface_name));
  rc = cdd_c_keep_err(
      rc, fortran_scope_register(&arg_scope, iface_reg, sizeof(iface_reg)));
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (fp_type->fn_params_count > 0) {
    arg_names = (char(*)[FORTRAN_MAX_IDENT_LEN + 1])
        C_CDD_CALLOC(fp_type->fn_params_count, sizeof(*arg_names));
    if (!arg_names) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup;
    }
  }

  for (k = 0; k < fp_type->fn_params_count; k++) {
    rc = cdd_c_keep_err(
        rc, cdd_fortran_sanitize_ident(fp_type->fn_params[k].name, k,
                                       arg_names[k], sizeof(arg_names[k])));
    rc = cdd_c_keep_err(rc, fortran_scope_register(&arg_scope, arg_names[k],
                                                   sizeof(arg_names[k])));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  is_void = (fp_type->fn_return_type->kind == CDD_FFI_KIND_VOID &&
             fp_type->fn_return_type->pointer_depth == 0);

  rc = cdd_fortran_write_wrapped_line(fp, "  ABSTRACT INTERFACE");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                              header_line, sizeof(header_line), "    %s %s(",
                              is_void ? "SUBROUTINE" : "FUNCTION", iface_name));

  for (k = 0; k < fp_type->fn_params_count; k++) {
    char next_hdr[2048];
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(
                next_hdr, sizeof(next_hdr), "%s%s%s", header_line, arg_names[k],
                (k + 1 < fp_type->fn_params_count) ? ", " : ""));
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(header_line,
                                                     sizeof(header_line), "%s",
                                                     next_hdr));
  }

  {
    char next_hdr[2048];
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(next_hdr, sizeof(next_hdr),
                                     "%s) BIND(C)%s", header_line,
                                     is_void ? "" : " RESULT(cdd_ret_)"));
    rc = cdd_c_keep_err(rc, cdd_fortran_write_wrapped_line(fp, next_hdr));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  /* Collect IMPORT symbols before emitting declarations */
  for (k = 0; k < fp_type->fn_params_count; k++) {
    char dummy_decl[512];
    rc = cdd_c_keep_err(rc, fortran_format_param_decl(
                                ir, node_fnames, &fp_type->fn_params[k],
                                arg_names[k], dummy_decl, sizeof(dummy_decl),
                                imports, sizeof(imports)));
  }
  if (!is_void) {
    rc = cdd_c_keep_err(
        rc, fortran_format_return_spec(ir, node_fnames, fp_type->fn_return_type,
                                       ret_spec, sizeof(ret_spec), imports,
                                       sizeof(imports)));
  }

  if (imports[0] != '\0') {
    rc = cdd_c_keep_err(
        rc, fortran_write_line_fmt(fp, "      IMPORT :: %s", imports));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  rc = cdd_fortran_write_wrapped_line(fp, "      IMPLICIT NONE");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (k = 0; k < fp_type->fn_params_count; k++) {
    char decl_line[512];
    rc = cdd_c_keep_err(rc, fortran_format_param_decl(
                                ir, node_fnames, &fp_type->fn_params[k],
                                arg_names[k], decl_line, sizeof(decl_line),
                                imports, sizeof(imports)));
    rc = cdd_c_keep_err(rc, cdd_fortran_write_wrapped_line(fp, decl_line));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  if (!is_void) {
    rc = fortran_write_line_fmt(fp, "      %s :: cdd_ret_", ret_spec);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = fortran_write_line_fmt(fp, "    END %s %s",
                              is_void ? "SUBROUTINE" : "FUNCTION", iface_name);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_fortran_write_wrapped_line(fp, "  END INTERFACE");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_ffi_write_str(fp, "\n");

cleanup:
  C_CDD_FREE(arg_names);
  return cdd_c_keep_err(rc, fortran_scope_free(&arg_scope));
}

/**
 * @brief Checks if a function parameter is mapped as a `const char *` C string
 * (`CHARACTER(KIND=C_CHAR), DIMENSION(*), INTENT(IN)`).
 *
 * @param[in] fld Parameter field descriptor.
 * @param[out] out_is_cstr Receives 1 if C string input, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t
fortran_is_const_cstr_param(const cdd_ffi_field_t *fld, int *out_is_cstr) {
  *out_is_cstr =
      (fld->type.pointer_depth == 1 && fld->type.array_size <= 0 &&
       (fld->type.kind == CDD_FFI_KIND_INT8 ||
        fld->type.kind == CDD_FFI_KIND_UINT8) &&
       (fld->type.is_const || fld->intent == CDD_FFI_INTENT_IN) &&
       fld->intent != CDD_FFI_INTENT_OUT && fld->intent != CDD_FFI_INTENT_INOUT)
          ? 1
          : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a function return type is `const char *` / `char *` eligible
 * for Fortran `CHARACTER(LEN=:), ALLOCATABLE` conversion in `CONTAINS`.
 *
 * @param[in] ret_type Return type descriptor.
 * @param[out] out_is_cstr_ret Receives 1 if `char *` return, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t
fortran_is_cstr_return(const cdd_ffi_type_t *ret_type, int *out_is_cstr_ret) {
  *out_is_cstr_ret =
      (ret_type->pointer_depth == 1 && ret_type->array_size <= 0 &&
       ret_type->kind == CDD_FFI_KIND_INT8)
          ? 1
          : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits `fpm.toml` (Fortran Package Manager manifest) and optional
 * `<mod_stem>_test.f90` layout/interface verification program.
 *
 * @param[in] ir FFI IR.
 * @param[in] config Generation configuration.
 * @param[in] lib_stem Sanitized library stem.
 * @param[in] mod_name Sanitized Fortran module name.
 * @param[in] node_fnames Parallel array of disambiguated Fortran node names.
 * @return CDD_C_SUCCESS on success, or error code on I/O failure.
 */
static NO_DISCARD cdd_c_error_t emit_fortran_fpm_and_tests(
    const cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config,
    const char *lib_stem, const char *mod_name, char **node_fnames) {
  FILE *fp = NULL;
  cdd_c_error_t rc;

  rc = cdd_ffi_open_output_file(config->output_dir, "fpm.toml", &fp);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_ffi_write_fmt(fp,
                         "name = \"%s\"\n"
                         "version = \"0.1.0\"\n\n"
                         "[library]\n"
                         "source-dir = \".\"\n\n"
                         "[build]\n"
                         "link = [\"%s\"]\n",
                         lib_stem, lib_stem);
  rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fp));
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (config->generate_tests) {
    char test_file[256];
    char test_prog[FORTRAN_MAX_IDENT_LEN + 1];
    size_t i;

    rc = cdd_ffi_snprintf_checked(test_file, sizeof(test_file), "%s_test.f90",
                                  mod_name);
    rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(test_file, 0, test_prog,
                                                       sizeof(test_prog)));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_open_output_file(config->output_dir, test_file, &fp));
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = fortran_write_line_fmt(
        fp, "! Auto-generated Fortran ISO_C_BINDING tests for %s", mod_name);
    if (rc != CDD_C_SUCCESS)
      goto test_cleanup;
    rc = fortran_write_line_fmt(fp, "PROGRAM %s", test_prog);
    if (rc != CDD_C_SUCCESS)
      goto test_cleanup;
    rc =
        cdd_fortran_write_wrapped_line(fp, "  USE, INTRINSIC :: ISO_C_BINDING");
    if (rc != CDD_C_SUCCESS)
      goto test_cleanup;
    rc = fortran_write_line_fmt(fp, "  USE %s", mod_name);
    if (rc != CDD_C_SUCCESS)
      goto test_cleanup;
    rc = cdd_fortran_write_wrapped_line(fp, "  IMPLICIT NONE");
    if (rc != CDD_C_SUCCESS)
      goto test_cleanup;

    for (i = 0; i < ir->nodes_count; i++) {
      const cdd_ffi_ir_node_t *node = &ir->nodes[i];
      if ((node->kind == CDD_FFI_NODE_STRUCT ||
           node->kind == CDD_FFI_NODE_UNION) &&
          node->name) {
        rc = fortran_write_line_fmt(fp, "  TYPE(%s) :: v_%lu", node_fnames[i],
                                    (unsigned long)i);
        if (rc != CDD_C_SUCCESS)
          goto test_cleanup;
      }
    }

    for (i = 0; i < ir->nodes_count; i++) {
      const cdd_ffi_ir_node_t *node = &ir->nodes[i];
      if ((node->kind == CDD_FFI_NODE_STRUCT ||
           node->kind == CDD_FFI_NODE_UNION) &&
          node->name) {
        if (node->kind == CDD_FFI_NODE_UNION && node->size_bytes > 0) {
          rc = fortran_write_line_fmt(
              fp, "  IF (C_SIZEOF(v_%lu) /= %lu_C_SIZE_T) STOP 1",
              (unsigned long)i, (unsigned long)node->size_bytes);
        } else {
          rc = fortran_write_line_fmt(
              fp, "  IF (C_SIZEOF(v_%lu) <= 0_C_SIZE_T) STOP 1",
              (unsigned long)i);
        }
        if (rc != CDD_C_SUCCESS)
          goto test_cleanup;
      }
    }

    rc = fortran_write_line_fmt(fp, "END PROGRAM %s", test_prog);

  test_cleanup:
    rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fp));
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits Fortran 2003/2008 `ISO_C_BINDING` `.f90` module bindings,
 * `fpm.toml`, and optional `<module>_test.f90` for the given FFI IR.
 *
 * @param[in,out] ir Extracted and topologically sorted FFI IR.
 * @param[in] config Bindings generation configuration.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * CDD_C_ERROR_MEMORY on OOM/truncation, or CDD_C_ERROR_IO on file I/O failure.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_emit_fortran(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  const char *raw_lib;
  const char *raw_mod;
  char lib_stem[FORTRAN_MAX_IDENT_LEN + 1];
  char mod_name[FORTRAN_MAX_IDENT_LEN + 1];
  char mod_file[256];
  fortran_scope_t mod_scope;
  char **node_fnames = NULL;
  size_t i;
  size_t j;
  int has_union_helpers = 0;
  int has_cstr_ret_wrapper = 0;
  int has_cstr_arg_wrapper = 0;
  cdd_c_error_t rc;

  if (!ir || !config || !config->output_dir)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(&mod_scope, 0, sizeof(mod_scope));

  /* Ensure struct and union ABI sizes are populated */
  rc = cdd_ffi_ir_compute_layout(ir, NULL);
  if (rc == CDD_C_ERROR_NOT_FOUND) {
    cdd_ffi_abi_profile_t relaxed_abi;
    rc = cdd_ffi_abi_profile_init(CDD_FFI_ABI_HOST, &relaxed_abi);
    relaxed_abi.allow_unresolved_refs = 1;
    rc = cdd_c_keep_err(rc, cdd_ffi_ir_compute_layout(ir, &relaxed_abi));
  }
  if (rc != CDD_C_SUCCESS)
    return rc;

  raw_lib = config->library_name
                ? config->library_name
                : (ir->module_name ? ir->module_name : "mylib");
  raw_mod = config->module_name ? config->module_name : "mylib_bindings";

  rc = cdd_c_keep_err(
      rc, cdd_fortran_sanitize_ident(raw_lib, 0, lib_stem, sizeof(lib_stem)));
  rc = cdd_c_keep_err(
      rc, cdd_fortran_sanitize_ident(raw_mod, 0, mod_name, sizeof(mod_name)));
  rc = cdd_c_keep_err(
      rc, fortran_scope_register(&mod_scope, mod_name, sizeof(mod_name)));
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (ir->nodes_count > 0) {
    node_fnames = (char **)C_CDD_CALLOC(ir->nodes_count, sizeof(char *));
    if (!node_fnames) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup;
    }
    for (i = 0; i < ir->nodes_count; i++) {
      char sbuf[FORTRAN_MAX_IDENT_LEN + 1];
      rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(ir->nodes[i].name, i,
                                                         sbuf, sizeof(sbuf)));
      rc = cdd_c_keep_err(
          rc, fortran_scope_register(&mod_scope, sbuf, sizeof(sbuf)));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      node_fnames[i] = C_CDD_STRDUP(sbuf);
      if (!node_fnames[i]) {
        rc = CDD_C_ERROR_MEMORY;
        goto cleanup;
      }
    }
  }

  rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(mod_file, sizeof(mod_file),
                                                   "%s.f90", mod_name));
  rc = cdd_c_keep_err(
      rc, cdd_ffi_open_output_file(config->output_dir, mod_file, &f));
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = fortran_write_line_fmt(
      f, "! Auto-generated Fortran 2003/2008 (ISO_C_BINDING) bindings for %s",
      raw_lib);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = fortran_write_line_fmt(f, "MODULE %s", mod_name);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_fortran_write_wrapped_line(f, "  USE, INTRINSIC :: ISO_C_BINDING");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_fortran_write_wrapped_line(f, "  IMPLICIT NONE");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_ffi_write_str(f, "\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  /* 1. Macros, Enums, Structs, Unions, Typedefs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    if (node->kind == CDD_FFI_NODE_FUNCTION)
      continue;

    rc = cdd_ffi_emit_translated_trivia(f, node->leading_trivia,
                                        &k_fortran_comment_style);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (node->doc) {
      rc = cdd_ffi_emit_translated_doc(f, node->doc, "  ",
                                       &k_fortran_comment_style);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    if (node->kind == CDD_FFI_NODE_MACRO) {
      const char *raw_val =
          node->evaluated_value
              ? node->evaluated_value
              : (node->variants_count > 0 ? node->variants[0].value : NULL);
      if (!raw_val)
        continue;

      if (node->inferred_type == CDD_FFI_MACRO_TYPE_INT) {
        char norm[160];
        rc = cdd_c_keep_err(rc, fortran_normalize_int_literal(
                                    raw_val, "C_INT64_T", norm, sizeof(norm)));
        rc = cdd_c_keep_err(rc,
                            fortran_write_line_fmt(
                                f, "  INTEGER(C_INT64_T), PARAMETER :: %s = %s",
                                node_fnames[i], norm));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else if (node->inferred_type == CDD_FFI_MACRO_TYPE_FLOAT) {
        char norm[160];
        rc = cdd_c_keep_err(
            rc, fortran_normalize_float_literal(raw_val, norm, sizeof(norm)));
        rc = cdd_c_keep_err(rc, fortran_write_line_fmt(
                                    f, "  REAL(C_DOUBLE), PARAMETER :: %s = %s",
                                    node_fnames[i], norm));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else if (node->inferred_type == CDD_FFI_MACRO_TYPE_STRING) {
        rc = fortran_write_line_fmt(
            f,
            raw_val[0] == '"' ? "  CHARACTER(LEN=*), PARAMETER :: %s = %s"
                              : "  CHARACTER(LEN=*), PARAMETER :: %s = \"%s\"",
            node_fnames[i], raw_val);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = cdd_ffi_write_str(f, "\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      if (node->variants_count == 0) {
        rc = fortran_write_line_fmt(
            f, "  INTEGER(C_INT), PARAMETER :: %s_kind = C_INT",
            node_fnames[i]);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else {
        rc = cdd_fortran_write_wrapped_line(f, "  ENUM, BIND(C)");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        for (j = 0; j < node->variants_count; j++) {
          cdd_ffi_enum_variant_t *var = &node->variants[j];
          char v_name[FORTRAN_MAX_IDENT_LEN + 1];
          char v_expr[160];
          const char *v_src;

          if (var->doc) {
            rc = cdd_ffi_emit_translated_doc(f, var->doc, "    ",
                                             &k_fortran_comment_style);
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
          rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(
                                      var->name, j, v_name, sizeof(v_name)));
          rc = cdd_c_keep_err(
              rc, fortran_scope_register(&mod_scope, v_name, sizeof(v_name)));
          if (rc != CDD_C_SUCCESS)
            goto cleanup;

          v_src = (var->number_details && var->number_details->raw_spelling)
                      ? var->number_details->raw_spelling
                      : var->value;
          if (v_src) {
            rc =
                cdd_c_keep_err(rc, fortran_normalize_int_literal(
                                       v_src, "C_INT", v_expr, sizeof(v_expr)));
          } else {
            rc = cdd_c_keep_err(
                rc, cdd_ffi_snprintf_checked(v_expr, sizeof(v_expr),
                                             "%lu_C_INT", (unsigned long)j));
          }
          rc = cdd_c_keep_err(
              rc, fortran_write_line_fmt(f, "    ENUMERATOR :: %s = %s", v_name,
                                         v_expr));
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = cdd_fortran_write_wrapped_line(f, "  END ENUM");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = cdd_ffi_write_str(f, "\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (node->kind == CDD_FFI_NODE_STRUCT) {
      rc = fortran_write_line_fmt(f, "  TYPE, BIND(C) :: %s", node_fnames[i]);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      if (node->fields_count == 0) {
        /* Opaque C struct handle wrapper (empty BIND(C) types are illegal) */
        rc = cdd_fortran_write_wrapped_line(
            f, "    TYPE(C_PTR) :: ptr = C_NULL_PTR");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else {
        fortran_scope_t field_scope;
        memset(&field_scope, 0, sizeof(field_scope));

        for (j = 0; j < node->fields_count; j++) {
          cdd_ffi_field_t *fld = &node->fields[j];
          char f_name[FORTRAN_MAX_IDENT_LEN + 1];
          char f_spec[256];

          if (fld->doc) {
            rc = cdd_ffi_emit_translated_doc(f, fld->doc, "    ",
                                             &k_fortran_comment_style);
            if (rc != CDD_C_SUCCESS)
              break;
          }
          rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(
                                      fld->name, j, f_name, sizeof(f_name)));
          rc = cdd_c_keep_err(
              rc, fortran_scope_register(&field_scope, f_name, sizeof(f_name)));
          if (rc != CDD_C_SUCCESS)
            break;

          if (fld->type.pointer_depth > 0 ||
              fld->type.kind == CDD_FFI_KIND_OPAQUE_PTR) {
            rc = cdd_c_keep_err(rc,
                                cdd_ffi_snprintf_checked(f_spec, sizeof(f_spec),
                                                         "%s", "TYPE(C_PTR)"));
          } else if (fld->type.kind == CDD_FFI_KIND_FUNCTION_PTR) {
            rc = cdd_c_keep_err(
                rc, cdd_ffi_snprintf_checked(f_spec, sizeof(f_spec), "%s",
                                             "TYPE(C_FUNPTR)"));
          } else if (fld->type.array_size > 0 &&
                     fld->type.kind == CDD_FFI_KIND_INT8) {
            rc = cdd_c_keep_err(
                rc, cdd_ffi_snprintf_checked(f_spec, sizeof(f_spec), "%s",
                                             "CHARACTER(KIND=C_CHAR)"));
          } else {
            int bind_ok = 0;
            rc = cdd_c_keep_err(
                rc, fortran_map_base_type(ir, node_fnames, &fld->type, f_spec,
                                          sizeof(f_spec), NULL, 0, &bind_ok));
          }

          if (fld->type.array_size > 0) {
            rc = cdd_c_keep_err(rc, fortran_write_line_fmt(
                                        f, "    %s, DIMENSION(%ld) :: %s",
                                        f_spec, fld->type.array_size, f_name));
          } else {
            rc = cdd_c_keep_err(
                rc, fortran_write_line_fmt(f, "    %s :: %s", f_spec, f_name));
          }
          if (rc != CDD_C_SUCCESS)
            break;
        }
        rc = cdd_c_keep_err(rc, fortran_scope_free(&field_scope));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }

      rc = fortran_write_line_fmt(f, "  END TYPE %s", node_fnames[i]);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_ffi_write_str(f, "\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (node->kind == CDD_FFI_NODE_UNION) {
      size_t u_bytes = node->size_bytes > 0 ? node->size_bytes : 8;
      if (node->fields_count > 0)
        has_union_helpers = 1;

      rc = fortran_write_line_fmt(f, "  TYPE, BIND(C) :: %s", node_fnames[i]);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = fortran_write_line_fmt(f, "    INTEGER(C_INT8_T) :: data(%lu)",
                                  (unsigned long)u_bytes);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = fortran_write_line_fmt(f, "  END TYPE %s", node_fnames[i]);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_ffi_write_str(f, "\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else {
      /* CDD_FFI_NODE_TYPEDEF */
      if (node->return_or_base_type.kind == CDD_FFI_KIND_FUNCTION_PTR &&
          node->return_or_base_type.fn_return_type &&
          !node->return_or_base_type.fn_is_variadic) {
        char cb_iface[FORTRAN_MAX_IDENT_LEN + 1];
        char raw_iface[128];
        rc = cdd_c_keep_err(
            rc, cdd_ffi_snprintf_checked(raw_iface, sizeof(raw_iface), "%s_cb",
                                         node_fnames[i]));
        rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(
                                    raw_iface, i, cb_iface, sizeof(cb_iface)));
        rc = cdd_c_keep_err(
            rc, fortran_scope_register(&mod_scope, cb_iface, sizeof(cb_iface)));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = fortran_emit_callback_abstract_iface(f, ir, node_fnames, cb_iface,
                                                  &node->return_or_base_type);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else {
        rc = fortran_write_line_fmt(f, "  ! Typedef alias: %s", node_fnames[i]);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
  }

  /* 2. ABSTRACT INTERFACE blocks for typed callback fields/parameters */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT ||
        node->kind == CDD_FFI_NODE_FUNCTION) {
      for (j = 0; j < node->fields_count; j++) {
        cdd_ffi_field_t *fld = &node->fields[j];
        if (fld->type.kind == CDD_FFI_KIND_FUNCTION_PTR &&
            fld->type.fn_return_type && !fld->type.fn_is_variadic) {
          char raw_cb[128];
          char cb_iface[FORTRAN_MAX_IDENT_LEN + 1];
          rc = cdd_c_keep_err(
              rc, cdd_ffi_snprintf_checked(raw_cb, sizeof(raw_cb), "%s_%s_cb",
                                           node_fnames[i],
                                           fld->name ? fld->name : "fn"));
          rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(
                                      raw_cb, j, cb_iface, sizeof(cb_iface)));
          rc = cdd_c_keep_err(rc, fortran_scope_register(&mod_scope, cb_iface,
                                                         sizeof(cb_iface)));
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
          rc = fortran_emit_callback_abstract_iface(f, ir, node_fnames,
                                                    cb_iface, &fld->type);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
    }
  }

  /* 3. BIND(C) INTERFACE block for exported C functions */
  rc = cdd_fortran_write_wrapped_line(f, "  INTERFACE");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    fortran_scope_t arg_scope;
    char fn_reg[FORTRAN_MAX_IDENT_LEN + 1];
    char(*arg_names)[FORTRAN_MAX_IDENT_LEN + 1] = NULL;
    char imports[1024];
    char sig_line[2048];
    char next_sig[2048];
    char ret_spec[256];
    int is_void;
    int fn_has_cstr_arg = 0;
    int fn_has_cstr_ret = 0;

    if (node->kind != CDD_FFI_NODE_FUNCTION)
      continue;

    rc = cdd_ffi_emit_translated_trivia(f, node->leading_trivia,
                                        &k_fortran_comment_style);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (node->doc) {
      rc = cdd_ffi_emit_translated_doc(f, node->doc, "    ",
                                       &k_fortran_comment_style);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    if (node->is_variadic) {
      rc = fortran_write_line_fmt(f,
                                  "    ! Skipped variadic C function (not "
                                  "ISO_C_BINDING compatible): %s",
                                  node->name ? node->name : "anon");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      continue;
    }

    memset(&arg_scope, 0, sizeof(arg_scope));
    imports[0] = '\0';
    ret_spec[0] = '\0';

    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(fn_reg, sizeof(fn_reg),
                                                     "%s", node_fnames[i]));
    rc = cdd_c_keep_err(
        rc, fortran_scope_register(&arg_scope, fn_reg, sizeof(fn_reg)));
    if (rc != CDD_C_SUCCESS) {
      rc = cdd_c_keep_err(rc, fortran_scope_free(&arg_scope));
      goto cleanup;
    }

    if (node->fields_count > 0) {
      arg_names = (char(*)[FORTRAN_MAX_IDENT_LEN + 1])
          C_CDD_CALLOC(node->fields_count, sizeof(*arg_names));
      if (!arg_names) {
        rc = cdd_c_keep_err(CDD_C_ERROR_MEMORY, fortran_scope_free(&arg_scope));
        goto cleanup;
      }
    }

    for (j = 0; j < node->fields_count; j++) {
      int is_cs = 0;
      rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(node->fields[j].name,
                                                         j, arg_names[j],
                                                         sizeof(arg_names[j])));
      rc = cdd_c_keep_err(rc, fortran_scope_register(&arg_scope, arg_names[j],
                                                     sizeof(arg_names[j])));
      if (rc != CDD_C_SUCCESS)
        break;
      rc = cdd_c_keep_err(
          rc, fortran_is_const_cstr_param(&node->fields[j], &is_cs));
      if (is_cs)
        fn_has_cstr_arg = 1;
    }
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(arg_names);
      rc = cdd_c_keep_err(rc, fortran_scope_free(&arg_scope));
      goto cleanup;
    }

    rc = cdd_c_keep_err(rc, fortran_is_cstr_return(&node->return_or_base_type,
                                                   &fn_has_cstr_ret));
    if (fn_has_cstr_arg)
      has_cstr_arg_wrapper = 1;
    if (fn_has_cstr_ret)
      has_cstr_ret_wrapper = 1;

    is_void = (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
               node->return_or_base_type.pointer_depth == 0);

    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(sig_line, sizeof(sig_line), "    %s %s(",
                                     is_void ? "SUBROUTINE" : "FUNCTION",
                                     node_fnames[i]));
    for (j = 0; j < node->fields_count; j++) {
      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                  next_sig, sizeof(next_sig), "%s%s%s",
                                  sig_line, arg_names[j],
                                  (j + 1 < node->fields_count) ? ", " : ""));
      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                  sig_line, sizeof(sig_line), "%s", next_sig));
    }
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(next_sig, sizeof(next_sig),
                                     "%s) BIND(C, NAME=\"%s\")%s", sig_line,
                                     node->name ? node->name : node_fnames[i],
                                     is_void ? "" : " RESULT(cdd_ret_)"));
    rc = cdd_c_keep_err(rc, cdd_fortran_write_wrapped_line(f, next_sig));
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(arg_names);
      rc = cdd_c_keep_err(rc, fortran_scope_free(&arg_scope));
      goto cleanup;
    }

    /* Pre-scan declarations to build selective IMPORT :: list */
    for (j = 0; j < node->fields_count; j++) {
      char dummy_decl[512];
      rc = cdd_c_keep_err(rc, fortran_format_param_decl(
                                  ir, node_fnames, &node->fields[j],
                                  arg_names[j], dummy_decl, sizeof(dummy_decl),
                                  imports, sizeof(imports)));
    }
    if (!is_void) {
      rc = cdd_c_keep_err(rc, fortran_format_return_spec(
                                  ir, node_fnames, &node->return_or_base_type,
                                  ret_spec, sizeof(ret_spec), imports,
                                  sizeof(imports)));
    }
    if (imports[0] != '\0') {
      rc = cdd_c_keep_err(
          rc, fortran_write_line_fmt(f, "      IMPORT :: %s", imports));
    }
    if (rc == CDD_C_SUCCESS) {
      rc = cdd_fortran_write_wrapped_line(f, "      IMPLICIT NONE");
    }
    if (rc == CDD_C_SUCCESS) {
      for (j = 0; j < node->fields_count; j++) {
        char decl_line[512];
        rc = cdd_c_keep_err(rc, fortran_format_param_decl(
                                    ir, node_fnames, &node->fields[j],
                                    arg_names[j], decl_line, sizeof(decl_line),
                                    imports, sizeof(imports)));
        rc = cdd_c_keep_err(rc, cdd_fortran_write_wrapped_line(f, decl_line));
        if (rc != CDD_C_SUCCESS)
          break;
      }
    }
    if (rc == CDD_C_SUCCESS && !is_void) {
      rc = fortran_write_line_fmt(f, "      %s :: cdd_ret_", ret_spec);
    }
    if (rc == CDD_C_SUCCESS) {
      rc = fortran_write_line_fmt(f, "    END %s %s",
                                  is_void ? "SUBROUTINE" : "FUNCTION",
                                  node_fnames[i]);
    }
    if (rc == CDD_C_SUCCESS) {
      rc = cdd_ffi_write_str(f, "\n");
    }

    C_CDD_FREE(arg_names);
    rc = cdd_c_keep_err(rc, fortran_scope_free(&arg_scope));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = cdd_fortran_write_wrapped_line(f, "  END INTERFACE");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_ffi_write_str(f, "\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  /* 4. CONTAINS section: Union TRANSFER accessors & Fortran string wrappers */
  if (has_union_helpers || has_cstr_ret_wrapper || has_cstr_arg_wrapper) {
    rc = cdd_fortran_write_wrapped_line(f, "CONTAINS");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_ffi_write_str(f, "\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    if (has_cstr_ret_wrapper) {
      rc = cdd_ffi_write_str(
          f, "  FUNCTION cdd_c_ptr_to_f_string(cstr_ptr) RESULT(cdd_ret_)\n"
             "    TYPE(C_PTR), VALUE :: cstr_ptr\n"
             "    CHARACTER(LEN=:), ALLOCATABLE :: cdd_ret_\n"
             "    INTERFACE\n"
             "      FUNCTION cdd_f_c_strlen(s) BIND(C, NAME=\"strlen\") "
             "RESULT(n)\n"
             "        IMPORT :: C_PTR, C_SIZE_T\n"
             "        IMPLICIT NONE\n"
             "        TYPE(C_PTR), VALUE :: s\n"
             "        INTEGER(C_SIZE_T) :: n\n"
             "      END FUNCTION cdd_f_c_strlen\n"
             "    END INTERFACE\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      rc = cdd_ffi_write_str(
          f, "    CHARACTER(KIND=C_CHAR), POINTER :: ch_arr(:)\n"
             "    INTEGER(C_SIZE_T) :: slen, idx\n"
             "    IF (.NOT. C_ASSOCIATED(cstr_ptr)) THEN\n"
             "      cdd_ret_ = \"\"\n"
             "      RETURN\n"
             "    END IF\n"
             "    slen = cdd_f_c_strlen(cstr_ptr)\n"
             "    CALL C_F_POINTER(cstr_ptr, ch_arr, [slen])\n"
             "    ALLOCATE(CHARACTER(LEN=slen) :: cdd_ret_)\n"
             "    DO idx = 1_C_SIZE_T, slen\n"
             "      cdd_ret_(idx:idx) = ch_arr(idx)\n"
             "    END DO\n"
             "  END FUNCTION cdd_c_ptr_to_f_string\n\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    /* Union TRANSFER getter/setter procedures */
    for (i = 0; i < ir->nodes_count; i++) {
      cdd_ffi_ir_node_t *node = &ir->nodes[i];
      if (node->kind != CDD_FFI_NODE_UNION)
        continue;
      for (j = 0; j < node->fields_count; j++) {
        cdd_ffi_field_t *fld = &node->fields[j];
        char f_name[FORTRAN_MAX_IDENT_LEN + 1];
        char raw_get[160];
        char raw_set[160];
        char get_name[FORTRAN_MAX_IDENT_LEN + 1];
        char set_name[FORTRAN_MAX_IDENT_LEN + 1];
        char f_spec[256];
        int bind_ok = 1;

        if (fld->type.array_size > 0)
          continue;

        rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(fld->name, j, f_name,
                                                           sizeof(f_name)));

        if (fld->type.pointer_depth > 0 ||
            fld->type.kind == CDD_FFI_KIND_OPAQUE_PTR) {
          rc = cdd_c_keep_err(rc,
                              cdd_ffi_snprintf_checked(f_spec, sizeof(f_spec),
                                                       "%s", "TYPE(C_PTR)"));
        } else if (fld->type.kind == CDD_FFI_KIND_FUNCTION_PTR) {
          rc = cdd_c_keep_err(rc,
                              cdd_ffi_snprintf_checked(f_spec, sizeof(f_spec),
                                                       "%s", "TYPE(C_FUNPTR)"));
        } else {
          rc = cdd_c_keep_err(
              rc, fortran_map_base_type(ir, node_fnames, &fld->type, f_spec,
                                        sizeof(f_spec), NULL, 0, &bind_ok));
          if (!bind_ok)
            continue;
        }

        rc = cdd_c_keep_err(
            rc, cdd_ffi_snprintf_checked(raw_get, sizeof(raw_get), "%s_get_%s",
                                         node_fnames[i], f_name));
        rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(raw_get, j, get_name,
                                                           sizeof(get_name)));
        rc = cdd_c_keep_err(
            rc, fortran_scope_register(&mod_scope, get_name, sizeof(get_name)));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        rc = cdd_c_keep_err(
            rc, cdd_ffi_snprintf_checked(raw_set, sizeof(raw_set), "%s_set_%s",
                                         node_fnames[i], f_name));
        rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(raw_set, j, set_name,
                                                           sizeof(set_name)));
        rc = cdd_c_keep_err(
            rc, fortran_scope_register(&mod_scope, set_name, sizeof(set_name)));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        rc = fortran_write_line_fmt(f, "  FUNCTION %s(u_val) RESULT(cdd_ret_)",
                                    get_name);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = fortran_write_line_fmt(f, "    TYPE(%s), INTENT(IN) :: u_val",
                                    node_fnames[i]);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = fortran_write_line_fmt(f, "    %s :: cdd_ret_", f_spec);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_fortran_write_wrapped_line(
            f, "    cdd_ret_ = TRANSFER(u_val%data, cdd_ret_)");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = fortran_write_line_fmt(f, "  END FUNCTION %s\n", get_name);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        rc = fortran_write_line_fmt(f, "  SUBROUTINE %s(u_val, f_val)",
                                    set_name);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = fortran_write_line_fmt(f, "    TYPE(%s), INTENT(INOUT) :: u_val",
                                    node_fnames[i]);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = fortran_write_line_fmt(f, "    %s, INTENT(IN) :: f_val", f_spec);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_fortran_write_wrapped_line(f,
                                            "    INTEGER(C_SIZE_T) :: n_copy");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_fortran_write_wrapped_line(f,
                                            "    n_copy = MIN(SIZE(u_val%data, "
                                            "KIND=C_SIZE_T), C_SIZEOF(f_val))");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_fortran_write_wrapped_line(
            f,
            "    u_val%data(1:n_copy) = TRANSFER(f_val, u_val%data(1:n_copy))");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = fortran_write_line_fmt(f, "  END SUBROUTINE %s\n", set_name);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }

    /* Native CHARACTER(LEN=*) string convenience wrappers */
    for (i = 0; i < ir->nodes_count; i++) {
      cdd_ffi_ir_node_t *node = &ir->nodes[i];
      int fn_cstr_arg = 0;
      int fn_cstr_ret = 0;
      int is_void;
      char raw_wrap[160];
      char wrap_name[FORTRAN_MAX_IDENT_LEN + 1];
      char wrap_reg[FORTRAN_MAX_IDENT_LEN + 1];
      fortran_scope_t arg_scope;
      char(*arg_names)[FORTRAN_MAX_IDENT_LEN + 1] = NULL;
      char hdr[2048];
      char next_hdr[2048];
      char call_args[2048];

      if (node->kind != CDD_FFI_NODE_FUNCTION || node->is_variadic)
        continue;

      for (j = 0; j < node->fields_count; j++) {
        int cs = 0;
        rc = cdd_c_keep_err(rc,
                            fortran_is_const_cstr_param(&node->fields[j], &cs));
        if (cs)
          fn_cstr_arg = 1;
      }
      rc = cdd_c_keep_err(
          rc, fortran_is_cstr_return(&node->return_or_base_type, &fn_cstr_ret));
      if (!fn_cstr_arg && !fn_cstr_ret)
        continue;

      rc = cdd_c_keep_err(rc,
                          cdd_ffi_snprintf_checked(raw_wrap, sizeof(raw_wrap),
                                                   "%s_str", node_fnames[i]));
      rc = cdd_c_keep_err(rc, cdd_fortran_sanitize_ident(raw_wrap, i, wrap_name,
                                                         sizeof(wrap_name)));
      rc = cdd_c_keep_err(
          rc, fortran_scope_register(&mod_scope, wrap_name, sizeof(wrap_name)));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      memset(&arg_scope, 0, sizeof(arg_scope));
      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                  wrap_reg, sizeof(wrap_reg), "%s", wrap_name));
      rc = cdd_c_keep_err(
          rc, fortran_scope_register(&arg_scope, wrap_reg, sizeof(wrap_reg)));
      if (rc != CDD_C_SUCCESS) {
        rc = cdd_c_keep_err(rc, fortran_scope_free(&arg_scope));
        goto cleanup;
      }

      if (node->fields_count > 0) {
        arg_names = (char(*)[FORTRAN_MAX_IDENT_LEN + 1])
            C_CDD_CALLOC(node->fields_count, sizeof(*arg_names));
        if (!arg_names) {
          rc = cdd_c_keep_err(CDD_C_ERROR_MEMORY,
                              fortran_scope_free(&arg_scope));
          goto cleanup;
        }
      }
      for (j = 0; j < node->fields_count; j++) {
        rc = cdd_c_keep_err(
            rc, cdd_fortran_sanitize_ident(node->fields[j].name, j,
                                           arg_names[j], sizeof(arg_names[j])));
        rc = cdd_c_keep_err(rc, fortran_scope_register(&arg_scope, arg_names[j],
                                                       sizeof(arg_names[j])));
        if (rc != CDD_C_SUCCESS)
          break;
      }
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(arg_names);
        rc = cdd_c_keep_err(rc, fortran_scope_free(&arg_scope));
        goto cleanup;
      }

      is_void = (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
                 node->return_or_base_type.pointer_depth == 0);

      rc = cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(hdr, sizeof(hdr), "  %s %s(",
                                       is_void ? "SUBROUTINE" : "FUNCTION",
                                       wrap_name));
      call_args[0] = '\0';
      for (j = 0; j < node->fields_count; j++) {
        int cs = 0;
        char next_call[2048];
        rc = cdd_c_keep_err(rc,
                            fortran_is_const_cstr_param(&node->fields[j], &cs));
        rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                    next_hdr, sizeof(next_hdr), "%s%s%s", hdr,
                                    arg_names[j],
                                    (j + 1 < node->fields_count) ? ", " : ""));
        rc = cdd_c_keep_err(
            rc, cdd_ffi_snprintf_checked(hdr, sizeof(hdr), "%s", next_hdr));
        rc = cdd_c_keep_err(
            rc, cdd_ffi_snprintf_checked(
                    next_call, sizeof(next_call),
                    cs ? "%sTRIM(%s) // C_NULL_CHAR%s" : "%s%s%s", call_args,
                    arg_names[j], (j + 1 < node->fields_count) ? ", " : ""));
        rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(call_args,
                                                         sizeof(call_args),
                                                         "%s", next_call));
      }
      rc = cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(next_hdr, sizeof(next_hdr), "%s)%s", hdr,
                                       is_void ? "" : " RESULT(cdd_ret_)"));
      rc = cdd_c_keep_err(rc, cdd_fortran_write_wrapped_line(f, next_hdr));

      if (rc == CDD_C_SUCCESS) {
        for (j = 0; j < node->fields_count; j++) {
          int cs = 0;
          rc = cdd_c_keep_err(
              rc, fortran_is_const_cstr_param(&node->fields[j], &cs));
          if (cs) {
            rc = cdd_c_keep_err(
                rc,
                fortran_write_line_fmt(
                    f, "    CHARACTER(LEN=*), INTENT(IN) :: %s", arg_names[j]));
          } else {
            char decl_line[512];
            char dummy_imp[128];
            dummy_imp[0] = '\0';
            rc = cdd_c_keep_err(
                rc, fortran_format_param_decl(ir, node_fnames, &node->fields[j],
                                              arg_names[j], decl_line,
                                              sizeof(decl_line), dummy_imp,
                                              sizeof(dummy_imp)));
            rc = cdd_c_keep_err(
                rc, cdd_fortran_write_wrapped_line(f, decl_line + 2));
          }
          if (rc != CDD_C_SUCCESS)
            break;
        }
      }

      if (rc == CDD_C_SUCCESS) {
        if (is_void) {
          rc = fortran_write_line_fmt(f, "    CALL %s(%s)", node_fnames[i],
                                      call_args);
        } else if (fn_cstr_ret) {
          rc = cdd_fortran_write_wrapped_line(
              f, "    CHARACTER(LEN=:), ALLOCATABLE :: cdd_ret_");
          if (rc == CDD_C_SUCCESS) {
            rc = fortran_write_line_fmt(
                f, "    cdd_ret_ = cdd_c_ptr_to_f_string(%s(%s))",
                node_fnames[i], call_args);
          }
        } else {
          char ret_spec[256];
          rc = cdd_c_keep_err(rc,
                              fortran_format_return_spec(
                                  ir, node_fnames, &node->return_or_base_type,
                                  ret_spec, sizeof(ret_spec), NULL, 0));
          rc = cdd_c_keep_err(
              rc, fortran_write_line_fmt(f, "    %s :: cdd_ret_", ret_spec));
          if (rc == CDD_C_SUCCESS) {
            rc = fortran_write_line_fmt(f, "    cdd_ret_ = %s(%s)",
                                        node_fnames[i], call_args);
          }
        }
      }
      if (rc == CDD_C_SUCCESS) {
        rc = fortran_write_line_fmt(
            f, "  END %s %s\n", is_void ? "SUBROUTINE" : "FUNCTION", wrap_name);
      }

      C_CDD_FREE(arg_names);
      rc = cdd_c_keep_err(rc, fortran_scope_free(&arg_scope));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  rc = fortran_write_line_fmt(f, "END MODULE %s", mod_name);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&f));
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = emit_fortran_fpm_and_tests(ir, config, lib_stem, mod_name, node_fnames);

cleanup:
  if (f) {
    rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&f));
  }
  if (node_fnames) {
    for (i = 0; i < ir->nodes_count; i++) {
      C_CDD_FREE(node_fnames[i]);
    }
    C_CDD_FREE(node_fnames);
  }
  return cdd_c_keep_err(rc, fortran_scope_free(&mod_scope));
}
