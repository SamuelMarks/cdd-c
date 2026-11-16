/**
 * @file cdd_ffi_emit_ocaml.c
 * @brief Production OCaml (`Ctypes`, `.mli`, `dune`, and C89 stubs) FFI binding
 * emitter implementation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_ocaml.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_ffi_emit_common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Maximum length of sanitized OCaml file basename to stay well within
 * Windows MAX_PATH. */
#define OCAML_MAX_FILE_STEM 64

/** @brief Standard OCaml block comment style `(* ... *)`. */
static const cdd_ffi_comment_style_t k_ocaml_comment_style = {
    NULL, "(* ", " *)", "(*", "( *", "*)", "* )", 0};

/** @brief OCamldoc docstring comment style `(** ... *)`. */
static const cdd_ffi_comment_style_t k_ocamldoc_style = {
    NULL, "(** ", " *)", "(*", "( *", "*)", "* )", 0};

/** @brief Complete table of OCaml reserved keywords and shadowed Ctypes names.
 */
static const char *const k_ocaml_keywords[] = {
    "and",       "as",         "assert",    "asr",         "begin",
    "class",     "constraint", "do",        "done",        "downto",
    "else",      "end",        "exception", "external",    "false",
    "for",       "fun",        "function",  "functor",     "if",
    "in",        "include",    "inherit",   "initializer", "land",
    "lazy",      "let",        "lor",       "lsl",         "lsr",
    "lxor",      "match",      "method",    "mod",         "module",
    "mutable",   "new",        "nonrec",    "object",      "of",
    "open",      "or",         "private",   "rec",         "sig",
    "struct",    "then",       "to",        "true",        "try",
    "type",      "val",        "virtual",   "when",        "while",
    "with",      "unit",       "int",       "int32",       "int64",
    "nativeint", "char",       "string",    "float",       "double",
    "bool",      "void",       "ptr",       "array",       "field",
    "seal",      "view",       "structure", "union",       "foreign",
    "returning", "int8_t",     "uint8_t",   "int16_t",     "uint16_t",
    "int32_t",   "uint32_t",   "int64_t",   "uint64_t",    "static_funptr"};

/**
 * @brief Checks whether `word` matches an OCaml keyword or Ctypes combinator.
 *
 * @param[in] word Candidate lowercase identifier.
 * @param[out] out_is_kw Receives 1 if reserved, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t ocaml_is_reserved(const char *word,
                                                  int *out_is_kw) {
  size_t i;
  size_t count;

  *out_is_kw = 0;
  count = sizeof(k_ocaml_keywords) / sizeof(k_ocaml_keywords[0]);
  for (i = 0; i < count; i++) {
    if (strcmp(word, k_ocaml_keywords[i]) == 0) {
      *out_is_kw = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Sanitizes an arbitrary C identifier into a valid lowercase OCaml
 * identifier (`[a-z_][a-zA-Z0-9_']*`), escaping reserved words by appending
 * `_`.
 *
 * @param[in] c_ident Source C identifier (may be NULL or empty).
 * @param[out] out_buf Destination buffer.
 * @param[in] out_cap Capacity of `out_buf` in bytes (must be >= 4).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
NO_DISCARD cdd_c_error_t cdd_ocaml_sanitize_lid(const char *c_ident,
                                                char *out_buf, size_t out_cap) {
  size_t pos;
  size_t i;
  size_t max_len;
  int is_kw;
  cdd_c_error_t rc;

  if (!out_buf || out_cap < 4)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out_buf[0] = '\0';
  if (!c_ident || c_ident[0] == '\0') {
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "unnamed_");
  }

  max_len = out_cap - 2;
  if (max_len > 96)
    max_len = 96;

  pos = 0;
  for (i = 0; c_ident[i] != '\0' && pos < max_len; i++) {
    unsigned char ch = (unsigned char)c_ident[i];
    if (isupper(ch)) {
      if (i > 0 && (islower((unsigned char)c_ident[i - 1]) ||
                    isdigit((unsigned char)c_ident[i - 1]) ||
                    (isupper((unsigned char)c_ident[i - 1]) &&
                     islower((unsigned char)c_ident[i + 1])))) {
        if (pos + 1 < max_len) {
          out_buf[pos++] = '_';
        }
      }
      out_buf[pos++] = (char)tolower(ch);
    } else if (isalnum(ch) || memchr("_'", ch, 2) != NULL) {
      if (pos == 0 && (isdigit(ch) || ch == '\'')) {
        out_buf[pos++] = 'v';
        out_buf[pos++] = '_';
      }
      if (pos < max_len) {
        out_buf[pos++] = (char)ch;
      }
    } else {
      if (pos == 0 || out_buf[pos - 1] != '_') {
        out_buf[pos++] = '_';
      }
    }
  }
  out_buf[pos] = '\0';

  if (pos == 1 && out_buf[0] == '_') {
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "unnamed_");
  }

  rc = ocaml_is_reserved(out_buf, &is_kw);
  if (is_kw) {
    out_buf[pos++] = '_';
    out_buf[pos] = '\0';
  }

  return rc;
}

/*
 * @brief Sanitizes an arbitrary C identifier into a valid capitalized OCaml
 * module or variant constructor identifier (`[A-Z][a-zA-Z0-9_']*`).
 *
 * @param[in] c_ident Source C identifier (may be NULL or empty).
 * @param[out] out_buf Destination buffer.
 * @param[in] out_cap Capacity of `out_buf` in bytes (must be >= 4).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
NO_DISCARD cdd_c_error_t cdd_ocaml_sanitize_uid(const char *c_ident,
                                                char *out_buf, size_t out_cap) {
  size_t pos;
  size_t i;
  size_t max_len;
  const char *p;

  if (!out_buf || out_cap < 4)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out_buf[0] = '\0';
  if (!c_ident)
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "Unnamed");

  p = c_ident;
  while (*p != '\0' && !isalnum((unsigned char)*p))
    p++;
  if (*p == '\0')
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "Unnamed");

  max_len = out_cap - 1;
  if (max_len > 96)
    max_len = 96;

  pos = 0;
  if (isdigit((unsigned char)*p)) {
    out_buf[pos++] = 'M';
    out_buf[pos++] = '_';
  } else if (islower((unsigned char)*p)) {
    out_buf[pos++] = (char)toupper((unsigned char)*p);
    p++;
  } else {
    out_buf[pos++] = *p;
    p++;
  }

  for (i = 0; p[i] != '\0' && pos < max_len; i++) {
    unsigned char ch = (unsigned char)p[i];
    if (isalnum(ch) || memchr("_'", ch, 2) != NULL) {
      out_buf[pos++] = (char)ch;
    } else if (out_buf[pos - 1] != '_') {
      out_buf[pos++] = '_';
    }
  }
  out_buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Normalizes a C integer/enum literal (including C23 hex, binary, octal,
 * and `'` digit separators) into a valid OCaml `int` literal expression.
 *
 * @param[in] raw Raw C integer literal string.
 * @param[out] out_buf Destination buffer for OCaml literal (`out_cap >= 16`).
 * @param[in] out_cap Capacity of `out_buf`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t ocaml_normalize_int_literal(const char *raw,
                                                            char *out_buf,
                                                            size_t out_cap) {
  const char *p;
  size_t len;
  size_t pos;
  size_t i;
  int neg;

  if (raw[0] == '\0')
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");

  p = raw;
  while (memchr(" \t(", (unsigned char)*p, 3) != NULL)
    p++;

  neg = 0;
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
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");

  pos = 0;
  if (neg) {
    out_buf[pos++] = '(';
    out_buf[pos++] = '-';
  }

  if (len > 2 && p[0] == '0' &&
      memchr("xXbB", (unsigned char)p[1], 4) != NULL) {
    out_buf[pos++] = '0';
    out_buf[pos++] = (char)tolower((unsigned char)p[1]);
    i = 2;
  } else if (len > 1 && p[0] == '0' && p[1] >= '0' && p[1] <= '7') {
    out_buf[pos++] = '0';
    out_buf[pos++] = 'o';
    i = 1;
  } else {
    i = 0;
  }

  for (; i < len && pos + 2 < out_cap; i++) {
    char c = p[i];
    if (c == '\'') {
      out_buf[pos++] = '_';
    } else if (isalnum((unsigned char)c) || c == '_') {
      out_buf[pos++] = c;
    } else {
      return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");
    }
  }

  if (neg) {
    out_buf[pos++] = ')';
  }
  out_buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Normalizes a C floating-point literal into a valid OCaml `float`
 * literal.
 *
 * @param[in] raw Raw C float literal string.
 * @param[out] out_buf Destination buffer (`out_cap >= 16`).
 * @param[in] out_cap Capacity of `out_buf`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t ocaml_normalize_float_literal(const char *raw,
                                                              char *out_buf,
                                                              size_t out_cap) {
  size_t i;
  size_t pos;
  int has_dot_or_exp;

  if (raw[0] == '\0')
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0.0");

  pos = 0;
  has_dot_or_exp = 0;
  for (i = 0; raw[i] != '\0' && pos + 3 < out_cap; i++) {
    char c = raw[i];
    if (c == '\'') {
      out_buf[pos++] = '_';
    } else if (isdigit((unsigned char)c) ||
               memchr(".eE+-", (unsigned char)c, 5) != NULL) {
      if (memchr(".eE", (unsigned char)c, 3) != NULL)
        has_dot_or_exp = 1;
      out_buf[pos++] = c;
    }
  }
  if (pos == 0)
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0.0");
  if (!has_dot_or_exp) {
    out_buf[pos++] = '.';
    out_buf[pos++] = '0';
  }
  out_buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds a node by exact C name and kind mask in `ir`.
 *
 * @param[in] ir FFI IR to search (non-NULL).
 * @param[in] name Target C symbol name.
 * @param[out] out_node Receives pointer to matching node, or NULL if not found.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t
ocaml_find_ir_node(const cdd_ffi_ir_t *ir, const char *name,
                   const cdd_ffi_ir_node_t **out_node) {
  size_t i;

  *out_node = NULL;
  if (!name)
    return CDD_C_SUCCESS;

  for (i = 0; i < ir->nodes_count; i++) {
    if (ir->nodes[i].name && strcmp(ir->nodes[i].name, name) == 0 &&
        ir->nodes[i].kind != CDD_FFI_NODE_FUNCTION &&
        ir->nodes[i].kind != CDD_FFI_NODE_MACRO) {
      *out_node = &ir->nodes[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Formats both the OCaml `Ctypes` combinator expression (`ctyp_buf`) and
 * the corresponding `.mli` OCaml type expression (`ml_buf`) for an FFI type.
 *
 * @param[in] ir Enclosing FFI IR (used to resolve struct/union/enum/typedef
 * refs).
 * @param[in] type FFI type descriptor.
 * @param[in] for_out_param Non-zero if parameter is `[out]`/`[inout]` (avoid
 * mapping `char *` to immutable `string`).
 * @param[out] ctyp_buf Output buffer for `Ctypes` value combinator.
 * @param[in] ctyp_cap Capacity of `ctyp_buf`.
 * @param[out] ml_buf Output buffer for OCaml `.mli` type signature.
 * @param[in] ml_cap Capacity of `ml_buf`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t ocaml_format_type_pair(
    const cdd_ffi_ir_t *ir, const cdd_ffi_type_t *type, int for_out_param,
    char *ctyp_buf, size_t ctyp_cap, char *ml_buf, size_t ml_cap) {
  char base_ctyp[512];
  char base_ml[512];
  int eff_ptr_depth;
  int p;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  eff_ptr_depth = type->pointer_depth;

  if (type->pointer_depth == 1 && !for_out_param && type->array_size <= 0 &&
      (type->kind == CDD_FFI_KIND_INT8 || type->kind == CDD_FFI_KIND_UINT8) &&
      type->is_const) {
    rc = cdd_ffi_snprintf_checked(ctyp_buf, ctyp_cap, "%s", "string");
    return cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(ml_buf, ml_cap, "%s", "string"));
  }

  switch (type->kind) {
  case CDD_FFI_KIND_VOID:
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", "void"));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", "unit"));
    break;
  case CDD_FFI_KIND_BOOL:
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", "bool"));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", "bool"));
    break;
  case CDD_FFI_KIND_INT8:
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", "char"));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", "char"));
    break;
  case CDD_FFI_KIND_UINT8:
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", "uint8_t"));
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                     "%s", "Unsigned.uint8"));
    break;
  case CDD_FFI_KIND_INT16:
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", "int16_t"));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", "int"));
    break;
  case CDD_FFI_KIND_UINT16:
    rc =
        cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                               base_ctyp, sizeof(base_ctyp), "%s", "uint16_t"));
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                     "%s", "Unsigned.uint16"));
    break;
  case CDD_FFI_KIND_INT32:
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", "int32_t"));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", "int32"));
    break;
  case CDD_FFI_KIND_UINT32:
    rc =
        cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                               base_ctyp, sizeof(base_ctyp), "%s", "uint32_t"));
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                     "%s", "Unsigned.uint32"));
    break;
  case CDD_FFI_KIND_INT64:
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", "int64_t"));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", "int64"));
    break;
  case CDD_FFI_KIND_UINT64:
    rc =
        cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                               base_ctyp, sizeof(base_ctyp), "%s", "uint64_t"));
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                     "%s", "Unsigned.uint64"));
    break;
  case CDD_FFI_KIND_FLOAT32:
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", "float"));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", "float"));
    break;
  case CDD_FFI_KIND_FLOAT64:
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", "double"));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", "float"));
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_TEMPLATE_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF: {
    const cdd_ffi_ir_node_t *ref_node = NULL;
    if (type->ref_name && strcmp(type->ref_name, "cdd_ffi_error_t") == 0) {
      rc = cdd_c_keep_err(rc,
                          cdd_ffi_snprintf_checked(base_ctyp, sizeof(base_ctyp),
                                                   "%s", "cdd_ffi_error_t"));
      rc = cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s",
                                       "cdd_ffi_error_t structure"));
      break;
    }
    if (type->ref_name && (strcmp(type->ref_name, "size_t") == 0 ||
                           strcmp(type->ref_name, "uintptr_t") == 0)) {
      rc =
          cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                 base_ctyp, sizeof(base_ctyp), "%s", "size_t"));
      rc =
          cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                      "%s", "Unsigned.size_t"));
      break;
    }
    rc = cdd_c_keep_err(rc, ocaml_find_ir_node(ir, type->ref_name, &ref_node));
    if (ref_node) {
      char ref_lid[128];
      rc = cdd_c_keep_err(
          rc, cdd_ocaml_sanitize_lid(ref_node->name, ref_lid, sizeof(ref_lid)));
      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                  base_ctyp, sizeof(base_ctyp), "%s", ref_lid));
      if (ref_node->kind == CDD_FFI_NODE_STRUCT) {
        rc = cdd_c_keep_err(rc,
                            cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                     "%s structure", ref_lid));
      } else if (ref_node->kind == CDD_FFI_NODE_UNION) {
        rc = cdd_c_keep_err(rc,
                            cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                     "%s union", ref_lid));
      } else {
        rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                    base_ml, sizeof(base_ml), "%s", ref_lid));
      }
    } else if (type->kind == CDD_FFI_KIND_ENUM_REF) {
      rc = cdd_c_keep_err(rc,
                          cdd_ffi_snprintf_checked(base_ctyp, sizeof(base_ctyp),
                                                   "%s", "int32_t"));
      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                       "%s", "int32"));
    } else {
      rc = cdd_c_keep_err(rc,
                          cdd_ffi_snprintf_checked(base_ctyp, sizeof(base_ctyp),
                                                   "%s", "(ptr void)"));
      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                       "%s", "unit ptr"));
      if (eff_ptr_depth > 0)
        eff_ptr_depth--;
    }
    break;
  }
  case CDD_FFI_KIND_FUNCTION_PTR: {
    if (type->fn_return_type && !type->fn_is_variadic) {
      char sig_ctyp[384];
      char sig_ml[384];
      char ret_c[128];
      char ret_m[128];
      size_t k;

      sig_ctyp[0] = '\0';
      sig_ml[0] = '\0';
      if (type->fn_params_count == 0) {
        rc = cdd_c_keep_err(rc,
                            cdd_ffi_snprintf_checked(sig_ctyp, sizeof(sig_ctyp),
                                                     "%s", "void @-> "));
        rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(sig_ml, sizeof(sig_ml),
                                                         "%s", "unit -> "));
      } else {
        for (k = 0; k < type->fn_params_count; k++) {
          char pc[128];
          char pm[128];
          char tmp_c[384];
          char tmp_m[384];
          rc = cdd_c_keep_err(
              rc, ocaml_format_type_pair(ir, &type->fn_params[k].type, 0, pc,
                                         sizeof(pc), pm, sizeof(pm)));
          rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(tmp_c, sizeof(tmp_c),
                                                           "%s%s @-> ",
                                                           sig_ctyp, pc));
          rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                      sig_ctyp, sizeof(sig_ctyp), "%s", tmp_c));
          rc = cdd_c_keep_err(rc,
                              cdd_ffi_snprintf_checked(tmp_m, sizeof(tmp_m),
                                                       "%s%s -> ", sig_ml, pm));
          rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                      sig_ml, sizeof(sig_ml), "%s", tmp_m));
        }
      }
      rc = cdd_c_keep_err(rc, ocaml_format_type_pair(ir, type->fn_return_type,
                                                     0, ret_c, sizeof(ret_c),
                                                     ret_m, sizeof(ret_m)));
      rc = cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(base_ctyp, sizeof(base_ctyp),
                                       "(Foreign.funptr (%sreturning %s))",
                                       sig_ctyp, ret_c));
      rc =
          cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                      "(%s%s)", sig_ml, ret_m));
    } else {
      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                  base_ctyp, sizeof(base_ctyp), "%s",
                                  "(static_funptr (void @-> returning void))"));
      rc = cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s",
                                       "(unit -> unit) static_funptr"));
    }
    break;
  }
  case CDD_FFI_KIND_OPAQUE_PTR:
  default:
    rc = cdd_c_keep_err(rc,
                        cdd_ffi_snprintf_checked(base_ctyp, sizeof(base_ctyp),
                                                 "%s", "(ptr void)"));
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml),
                                                     "%s", "unit ptr"));
    break;
  }

  for (p = 0; p < eff_ptr_depth; p++) {
    char next_c[512];
    char next_m[512];
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(next_c, sizeof(next_c),
                                                     "(ptr %s)", base_ctyp));
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", next_c));
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(next_m, sizeof(next_m),
                                                     "%s ptr", base_ml));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", next_m));
  }

  if (type->array_size > 0) {
    char next_c[512];
    char next_m[512];
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                next_c, sizeof(next_c), "(Ctypes.array %ld %s)",
                                type->array_size, base_ctyp));
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                base_ctyp, sizeof(base_ctyp), "%s", next_c));
    rc = cdd_c_keep_err(rc,
                        cdd_ffi_snprintf_checked(next_m, sizeof(next_m),
                                                 "%s Ctypes.carray", base_ml));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(base_ml, sizeof(base_ml), "%s", next_m));
  }

  rc = cdd_c_keep_err(
      rc, cdd_ffi_snprintf_checked(ctyp_buf, ctyp_cap, "%s", base_ctyp));
  return cdd_c_keep_err(
      rc, cdd_ffi_snprintf_checked(ml_buf, ml_cap, "%s", base_ml));
}

/**
 * @brief Determines the default zero-value OCaml expression for allocating a
 * pointee `Ctypes` storage slot via `Ctypes.allocate`.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] pointee Pointee FFI type (pointer_depth decremented by 1).
 * @param[out] zero_buf Output buffer receiving the zero initializer expression.
 * @param[in] zero_cap Capacity of `zero_buf`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t
ocaml_pointee_zero_expr(const cdd_ffi_ir_t *ir, const cdd_ffi_type_t *pointee,
                        char *zero_buf, size_t zero_cap) {
  if (pointee->pointer_depth > 0 || pointee->kind == CDD_FFI_KIND_OPAQUE_PTR)
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s", "null");

  switch (pointee->kind) {
  case CDD_FFI_KIND_BOOL:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s", "false");
  case CDD_FFI_KIND_INT8:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s", "'\\000'");
  case CDD_FFI_KIND_UINT8:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s",
                                    "Unsigned.UInt8.zero");
  case CDD_FFI_KIND_INT16:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s", "0");
  case CDD_FFI_KIND_UINT16:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s",
                                    "Unsigned.UInt16.zero");
  case CDD_FFI_KIND_INT32:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s", "0l");
  case CDD_FFI_KIND_UINT32:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s",
                                    "Unsigned.UInt32.zero");
  case CDD_FFI_KIND_INT64:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s", "0L");
  case CDD_FFI_KIND_UINT64:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s",
                                    "Unsigned.UInt64.zero");
  case CDD_FFI_KIND_FLOAT32:
  case CDD_FFI_KIND_FLOAT64:
    return cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s", "0.0");
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
  default: {
    const cdd_ffi_ir_node_t *rn = NULL;
    cdd_c_error_t rc = ocaml_find_ir_node(ir, pointee->ref_name, &rn);
    if (pointee->ref_name &&
        strcmp(pointee->ref_name, "cdd_ffi_error_t") == 0) {
      return cdd_c_keep_err(rc,
                            cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s",
                                                     "(make cdd_ffi_error_t)"));
    }
    if (rn &&
        (rn->kind == CDD_FFI_NODE_STRUCT || rn->kind == CDD_FFI_NODE_UNION)) {
      char rlid[128];
      rc = cdd_c_keep_err(rc,
                          cdd_ocaml_sanitize_lid(rn->name, rlid, sizeof(rlid)));
      return cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(zero_buf, zero_cap, "(make %s)", rlid));
    }
    if (rn && rn->kind == CDD_FFI_NODE_ENUM) {
      char ruid[128];
      rc = cdd_c_keep_err(rc,
                          cdd_ocaml_sanitize_uid(rn->name, ruid, sizeof(ruid)));
      return cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                    zero_buf, zero_cap, "(%s.of_int 0)", ruid));
    }
    return cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(zero_buf, zero_cap, "%s", "0l"));
  }
  }
}

/**
 * @brief Checks if an FFI parameter is a marshaled `[out]` or `[inout]` typed
 * single-indirection pointer eligible for `Ctypes.allocate` wrapping.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] f Function parameter field.
 * @param[out] out_is_marshaled Receives 1 if eligible, 0 otherwise.
 * @param[out] out_pointee Receives dereferenced pointee type when eligible.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t
ocaml_check_out_param(const cdd_ffi_ir_t *ir, const cdd_ffi_field_t *f,
                      int *out_is_marshaled, cdd_ffi_type_t *out_pointee) {
  const cdd_ffi_ir_node_t *rn = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  *out_is_marshaled = 0;
  memset(out_pointee, 0, sizeof(*out_pointee));

  if (f->intent != CDD_FFI_INTENT_OUT && f->intent != CDD_FFI_INTENT_INOUT)
    return CDD_C_SUCCESS;
  if (f->type.pointer_depth < 1 || f->type.array_size > 0 ||
      f->array_length_ref != NULL)
    return CDD_C_SUCCESS;
  if (f->type.pointer_depth == 1 && f->type.kind == CDD_FFI_KIND_VOID)
    return CDD_C_SUCCESS;

  if (f->type.pointer_depth == 1 &&
      (f->type.kind == CDD_FFI_KIND_STRUCT_REF ||
       f->type.kind == CDD_FFI_KIND_TYPEDEF_REF)) {
    if (!f->type.ref_name)
      return CDD_C_SUCCESS;
    if (strcmp(f->type.ref_name, "cdd_ffi_error_t") != 0) {
      rc = ocaml_find_ir_node(ir, f->type.ref_name, &rn);
      if (!rn)
        return rc;
      if ((rn->kind == CDD_FFI_NODE_STRUCT || rn->kind == CDD_FFI_NODE_UNION) &&
          rn->fields_count == 0) {
        return rc;
      }
    }
  }

  *out_pointee = f->type;
  out_pointee->pointer_depth = f->type.pointer_depth - 1;
  *out_is_marshaled = 1;
  return rc;
}

/**
 * @brief Checks if `type` is `cdd_ffi_error_t` (by value or single pointer).
 *
 * @param[in] type FFI type descriptor.
 * @param[out] out_is_err Receives 1 if `cdd_ffi_error_t` by value, 2 if
 * pointer, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t
ocaml_is_ffi_error_type(const cdd_ffi_type_t *type, int *out_is_err) {
  *out_is_err = 0;
  if (type->kind == CDD_FFI_KIND_STRUCT_REF && type->ref_name &&
      strcmp(type->ref_name, "cdd_ffi_error_t") == 0) {
    if (type->pointer_depth == 0)
      *out_is_err = 1;
    else if (type->pointer_depth == 1)
      *out_is_err = 2;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits the `<lib>.ml` implementation file and `<lib>.mli` signature
 * file simultaneously to guarantee 100% signature synchronization.
 *
 * @param[in] ir Topologically sorted FFI IR.
 * @param[in] config Generation configuration.
 * @param[in] lib_stem Sanitized lowercase library stem.
 * @return CDD_C_SUCCESS on success, or error code on I/O or formatting failure.
 */
static NO_DISCARD cdd_c_error_t emit_ocaml_ml_and_mli(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config,
    const char *lib_stem) {
  char ml_file[256];
  char mli_file[256];
  FILE *fml = NULL;
  FILE *fmli = NULL;
  size_t i;
  size_t j;
  int has_builtin_err_struct = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(ml_file, sizeof(ml_file),
                                                   "%s.ml", lib_stem));
  rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(mli_file, sizeof(mli_file),
                                                   "%s.mli", lib_stem));
  rc = cdd_c_keep_err(
      rc, cdd_ffi_open_output_file(config->output_dir, ml_file, &fml));
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_ffi_open_output_file(config->output_dir, mli_file, &fmli);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fml));
  }

  rc = cdd_ffi_write_fmt(fml,
                         "(* Auto-generated OCaml Ctypes bindings for %s *)\n\n"
                         "open Ctypes\n"
                         "open Foreign\n\n"
                         "let _open_lib () =\n"
                         "  let candidates = [\n"
                         "    \"lib%s.so\";\n"
                         "    \"lib%s.dylib\";\n"
                         "    \"%s.dll\";\n"
                         "    \"lib%s.dll\";\n"
                         "  ] in\n",
                         lib_stem, lib_stem, lib_stem, lib_stem, lib_stem);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_ffi_write_str(
      fml, "  let rec try_load = function\n"
           "    | [] -> Dl.dlopen ~flags:[Dl.RTLD_NOW; Dl.RTLD_GLOBAL] ()\n"
           "    | filename :: rest ->\n"
           "        (try Dl.dlopen ~filename ~flags:[Dl.RTLD_NOW]\n"
           "         with _ -> try_load rest)\n"
           "  in\n"
           "  try_load candidates\n\n"
           "let _lib = _open_lib ()\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_ffi_write_fmt(
      fmli,
      "(** Auto-generated OCaml Ctypes interface for %s *)\n\n"
      "open Ctypes\n\n",
      lib_stem);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < ir->nodes_count; i++) {
    if (ir->nodes[i].name &&
        strcmp(ir->nodes[i].name, "cdd_ffi_error_t") == 0 &&
        ir->nodes[i].kind == CDD_FFI_NODE_STRUCT) {
      has_builtin_err_struct = 1;
      break;
    }
  }

  if (!has_builtin_err_struct) {
    rc = cdd_ffi_write_str(
        fml, "type cdd_ffi_error_t\n"
             "let cdd_ffi_error_t : cdd_ffi_error_t structure typ = "
             "structure \"cdd_ffi_error_t\"\n"
             "let cdd_ffi_error_t_code = field cdd_ffi_error_t \"code\" int\n"
             "let cdd_ffi_error_t_message = field cdd_ffi_error_t \"message\" "
             "string_opt\n"
             "let () = seal cdd_ffi_error_t\n\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    rc = cdd_ffi_write_str(
        fmli,
        "type cdd_ffi_error_t\n"
        "val cdd_ffi_error_t : cdd_ffi_error_t structure typ\n"
        "val cdd_ffi_error_t_code : (int, cdd_ffi_error_t structure) field\n"
        "val cdd_ffi_error_t_message : "
        "(string option, cdd_ffi_error_t structure) field\n\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  /* Emit nodes in IR topological order (macros, enums, structs, unions,
   * typedefs, functions) */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    rc = cdd_ffi_emit_translated_trivia(fml, node->leading_trivia,
                                        &k_ocaml_comment_style);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    if (node->doc) {
      rc = cdd_ffi_emit_translated_doc(fml, node->doc, "", &k_ocamldoc_style);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_ffi_emit_translated_doc(fmli, node->doc, "", &k_ocamldoc_style);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    if (node->kind == CDD_FFI_NODE_MACRO) {
      char m_lid[128];
      const char *raw_val;

      raw_val =
          node->evaluated_value
              ? node->evaluated_value
              : (node->variants_count > 0 ? node->variants[0].value : NULL);
      if (!raw_val)
        continue;

      rc = cdd_c_keep_err(
          rc, cdd_ocaml_sanitize_lid(node->name, m_lid, sizeof(m_lid)));

      if (node->inferred_type == CDD_FFI_MACRO_TYPE_INT) {
        char norm[128];
        rc = cdd_c_keep_err(
            rc, ocaml_normalize_int_literal(raw_val, norm, sizeof(norm)));
        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fml, "let %s = %s\n\n", m_lid, norm));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_ffi_write_fmt(fmli, "val %s : int\n\n", m_lid);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else if (node->inferred_type == CDD_FFI_MACRO_TYPE_FLOAT) {
        char norm[128];
        rc = cdd_c_keep_err(
            rc, ocaml_normalize_float_literal(raw_val, norm, sizeof(norm)));
        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fml, "let %s = %s\n\n", m_lid, norm));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_ffi_write_fmt(fmli, "val %s : float\n\n", m_lid);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else if (node->inferred_type == CDD_FFI_MACRO_TYPE_STRING) {
        if (raw_val[0] == '"') {
          rc = cdd_c_keep_err(
              rc, cdd_ffi_write_fmt(fml, "let %s = %s\n\n", m_lid, raw_val));
        } else {
          rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fml, "let %s = \"%s\"\n\n",
                                                    m_lid, raw_val));
        }
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_ffi_write_fmt(fmli, "val %s : string\n\n", m_lid);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      char e_uid[128];
      char e_lid[128];

      rc = cdd_c_keep_err(
          rc, cdd_ocaml_sanitize_uid(node->name, e_uid, sizeof(e_uid)));
      rc = cdd_c_keep_err(
          rc, cdd_ocaml_sanitize_lid(node->name, e_lid, sizeof(e_lid)));

      rc = cdd_c_keep_err(
          rc, cdd_ffi_write_fmt(fml, "module %s = struct\n", e_uid));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_ffi_write_fmt(fmli, "module %s : sig\n", e_uid);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        char v_lid[128];
        char v_norm[128];
        const char *v_src;

        if (var->doc) {
          rc = cdd_ffi_emit_translated_doc(fml, var->doc, "  ",
                                           &k_ocamldoc_style);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = cdd_c_keep_err(
            rc, cdd_ocaml_sanitize_lid(var->name, v_lid, sizeof(v_lid)));

        v_src = (var->number_details && var->number_details->raw_spelling)
                    ? var->number_details->raw_spelling
                    : var->value;
        if (v_src) {
          rc = cdd_c_keep_err(
              rc, ocaml_normalize_int_literal(v_src, v_norm, sizeof(v_norm)));
        } else {
          rc = cdd_c_keep_err(
              rc, cdd_ffi_snprintf_checked(v_norm, sizeof(v_norm), "%lu",
                                           (unsigned long)j));
        }

        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fml, "  let %s = %s\n", v_lid, v_norm));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_ffi_write_fmt(fmli, "  val %s : int\n", v_lid);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }

      rc = cdd_ffi_write_str(fml, "  type t = [\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_ffi_write_str(fmli, "  type t = [\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      for (j = 0; j < node->variants_count; j++) {
        char v_uid[128];
        rc = cdd_c_keep_err(rc, cdd_ocaml_sanitize_uid(node->variants[j].name,
                                                       v_uid, sizeof(v_uid)));
        rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fml, "    | `%s_%lu\n", v_uid,
                                                  (unsigned long)j));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc =
            cdd_ffi_write_fmt(fmli, "    | `%s_%lu\n", v_uid, (unsigned long)j);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = cdd_ffi_write_str(fml, "    | `Unknown of int\n  ]\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_ffi_write_str(fmli, "    | `Unknown of int\n  ]\n"
                                   "  val of_int : int -> t\n"
                                   "  val to_int : t -> int\n"
                                   "  val t_view : t typ\n"
                                   "end\n\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      rc = cdd_ffi_write_str(fml, "  let of_int v =\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      for (j = 0; j < node->variants_count; j++) {
        char v_lid[128];
        char v_uid[128];
        rc = cdd_c_keep_err(rc, cdd_ocaml_sanitize_lid(node->variants[j].name,
                                                       v_lid, sizeof(v_lid)));
        rc = cdd_c_keep_err(rc, cdd_ocaml_sanitize_uid(node->variants[j].name,
                                                       v_uid, sizeof(v_uid)));
        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fml, "    %sif v = %s then `%s_%lu\n",
                                  j == 0 ? "" : "else ", v_lid, v_uid,
                                  (unsigned long)j));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = cdd_ffi_write_fmt(fml, "    %s`Unknown v\n",
                             node->variants_count > 0 ? "else " : "");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      rc = cdd_ffi_write_str(fml, "  let to_int = function\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      for (j = 0; j < node->variants_count; j++) {
        char v_lid[128];
        char v_uid[128];
        rc = cdd_c_keep_err(rc, cdd_ocaml_sanitize_lid(node->variants[j].name,
                                                       v_lid, sizeof(v_lid)));
        rc = cdd_c_keep_err(rc, cdd_ocaml_sanitize_uid(node->variants[j].name,
                                                       v_uid, sizeof(v_uid)));
        rc = cdd_c_keep_err(rc,
                            cdd_ffi_write_fmt(fml, "    | `%s_%lu -> %s\n",
                                              v_uid, (unsigned long)j, v_lid));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = cdd_ffi_write_str(
          fml, "    | `Unknown v -> v\n"
               "  let t_view : t typ = view ~read:of_int ~write:to_int int\n"
               "end\n\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      rc = cdd_ffi_write_fmt(fml,
                             "type %s = %s.t\n"
                             "let %s : %s typ = %s.t_view\n\n",
                             e_lid, e_uid, e_lid, e_lid, e_uid);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_ffi_write_fmt(fmli,
                             "type %s = %s.t\n"
                             "val %s : %s typ\n\n",
                             e_lid, e_uid, e_lid, e_lid);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (node->kind == CDD_FFI_NODE_STRUCT ||
               node->kind == CDD_FFI_NODE_UNION) {
      char s_lid[128];
      const char *kind_kw;
      const char *raw_tag;

      kind_kw = (node->kind == CDD_FFI_NODE_UNION) ? "union" : "structure";
      raw_tag = node->name ? node->name : "anon";
      rc = cdd_c_keep_err(
          rc, cdd_ocaml_sanitize_lid(raw_tag, s_lid, sizeof(s_lid)));

      rc = cdd_c_keep_err(rc,
                          cdd_ffi_write_fmt(fml,
                                            "type %s\n"
                                            "let %s : %s %s typ = %s \"%s\"\n",
                                            s_lid, s_lid, s_lid, kind_kw,
                                            kind_kw, raw_tag));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_ffi_write_fmt(fmli,
                             "type %s\n"
                             "val %s : %s %s typ\n",
                             s_lid, s_lid, s_lid, kind_kw);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      for (j = 0; j < node->fields_count; j++) {
        cdd_ffi_field_t *fld = &node->fields[j];
        char f_lid[128];
        char f_ctyp[512];
        char f_ml[512];
        char fallback_fname[64];
        const char *raw_fname;

        if (fld->doc) {
          rc =
              cdd_ffi_emit_translated_doc(fml, fld->doc, "", &k_ocamldoc_style);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        if (fld->name && fld->name[0] != '\0') {
          raw_fname = fld->name;
        } else {
          rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                      fallback_fname, sizeof(fallback_fname),
                                      "field_%lu", (unsigned long)j));
          raw_fname = fallback_fname;
        }
        rc = cdd_c_keep_err(
            rc, cdd_ocaml_sanitize_lid(raw_fname, f_lid, sizeof(f_lid)));
        rc = cdd_c_keep_err(rc, ocaml_format_type_pair(ir, &fld->type, 0,
                                                       f_ctyp, sizeof(f_ctyp),
                                                       f_ml, sizeof(f_ml)));

        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fml, "let %s_%s = field %s \"%s\" %s\n",
                                  s_lid, f_lid, s_lid, raw_fname, f_ctyp));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_ffi_write_fmt(fmli, "val %s_%s : (%s, %s %s) field\n", s_lid,
                               f_lid, f_ml, s_lid, kind_kw);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }

      if (node->fields_count > 0) {
        rc = cdd_ffi_write_fmt(fml, "let () = seal %s\n\n", s_lid);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else {
        rc = cdd_ffi_write_str(fml, "\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = cdd_ffi_write_str(fmli, "\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (node->kind == CDD_FFI_NODE_TYPEDEF) {
      char td_lid[128];
      char td_ctyp[512];
      char td_ml[512];

      rc = cdd_c_keep_err(
          rc, cdd_ocaml_sanitize_lid(node->name, td_lid, sizeof(td_lid)));
      rc = cdd_c_keep_err(
          rc, ocaml_format_type_pair(ir, &node->return_or_base_type, 0, td_ctyp,
                                     sizeof(td_ctyp), td_ml, sizeof(td_ml)));

      rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fml,
                                                "type %s = %s\n"
                                                "let %s : %s typ = %s\n\n",
                                                td_lid, td_ml, td_lid, td_lid,
                                                td_ctyp));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_ffi_write_fmt(fmli,
                             "type %s = %s\n"
                             "val %s : %s typ\n\n",
                             td_lid, td_ml, td_lid, td_lid);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else {
      /* CDD_FFI_NODE_FUNCTION */
      char fn_lid[128];
      char ret_ctyp[512];
      char ret_ml[512];
      const char *c_sym;
      const char *lock_flag;
      size_t out_count = 0;
      int ret_err_kind = 0;
      int err_param_idx = -1;
      int needs_wrapper = 0;

      c_sym = node->name ? node->name : "anon_fn";
      rc = cdd_c_keep_err(
          rc, cdd_ocaml_sanitize_lid(c_sym, fn_lid, sizeof(fn_lid)));
      rc = cdd_c_keep_err(rc, ocaml_format_type_pair(
                                  ir, &node->return_or_base_type, 0, ret_ctyp,
                                  sizeof(ret_ctyp), ret_ml, sizeof(ret_ml)));
      rc = cdd_c_keep_err(rc, ocaml_is_ffi_error_type(
                                  &node->return_or_base_type, &ret_err_kind));

      for (j = 0; j < node->fields_count; j++) {
        int is_m = 0;
        int p_err = 0;
        cdd_ffi_type_t pt;
        rc = cdd_c_keep_err(
            rc, ocaml_check_out_param(ir, &node->fields[j], &is_m, &pt));
        rc = cdd_c_keep_err(
            rc, ocaml_is_ffi_error_type(&node->fields[j].type, &p_err));
        if (p_err == 2 && node->fields[j].intent != CDD_FFI_INTENT_IN) {
          err_param_idx = (int)j;
          needs_wrapper = 1;
        } else if (is_m) {
          out_count++;
          needs_wrapper = 1;
        }
      }
      if (ret_err_kind == 1)
        needs_wrapper = 1;

      lock_flag =
          node->requires_gil_release ? "~release_runtime_lock:true " : "";

      rc = cdd_c_keep_err(
          rc, cdd_ffi_write_fmt(fml, "let %s%s = foreign ~from:_lib %s\"%s\" (",
                                needs_wrapper ? "_raw_" : "", fn_lid, lock_flag,
                                c_sym));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      if (node->fields_count == 0 && !node->is_variadic) {
        rc = cdd_ffi_write_str(fml, "void @-> ");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else {
        for (j = 0; j < node->fields_count; j++) {
          char p_ctyp[512];
          char p_ml[512];
          int for_out = (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
                         node->fields[j].intent == CDD_FFI_INTENT_INOUT ||
                         (int)j == err_param_idx)
                            ? 1
                            : 0;
          rc = cdd_c_keep_err(
              rc,
              ocaml_format_type_pair(ir, &node->fields[j].type, for_out, p_ctyp,
                                     sizeof(p_ctyp), p_ml, sizeof(p_ml)));
          rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fml, "%s @-> ", p_ctyp));
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        if (node->is_variadic) {
          /* Safe fixed-arity string tail for C variadics */
          rc = cdd_ffi_write_str(fml, "string @-> ");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      rc = cdd_ffi_write_fmt(fml, "returning %s)\n", ret_ctyp);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      if (!needs_wrapper) {
        rc = cdd_ffi_write_str(fml, "\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        rc = cdd_ffi_write_fmt(fmli, "val %s : ", fn_lid);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        if (node->fields_count == 0 && !node->is_variadic) {
          rc = cdd_ffi_write_str(fmli, "unit -> ");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        } else {
          for (j = 0; j < node->fields_count; j++) {
            char p_ctyp[512];
            char p_ml[512];
            rc = cdd_c_keep_err(
                rc, ocaml_format_type_pair(ir, &node->fields[j].type, 0, p_ctyp,
                                           sizeof(p_ctyp), p_ml, sizeof(p_ml)));
            rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fmli, "%s -> ", p_ml));
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
          if (node->is_variadic) {
            rc = cdd_ffi_write_str(fmli, "string -> ");
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
        }
        rc = cdd_ffi_write_fmt(fmli, "%s\n\n", ret_ml);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else {
        size_t exposed_inputs = 0;
        int ret_is_void =
            (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
             node->return_or_base_type.pointer_depth == 0) ||
            (ret_err_kind == 1);

        rc = cdd_ffi_write_fmt(fml, "let %s", fn_lid);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_ffi_write_fmt(fmli, "val %s : ", fn_lid);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        for (j = 0; j < node->fields_count; j++) {
          int is_m = 0;
          cdd_ffi_type_t pt;
          char pt_ctyp[512];
          char pt_ml[512];
          if ((int)j == err_param_idx)
            continue;
          rc = cdd_c_keep_err(
              rc, ocaml_check_out_param(ir, &node->fields[j], &is_m, &pt));
          if (is_m && node->fields[j].intent == CDD_FFI_INTENT_OUT)
            continue;

          exposed_inputs++;
          rc = cdd_c_keep_err(
              rc, ocaml_format_type_pair(ir, is_m ? &pt : &node->fields[j].type,
                                         0, pt_ctyp, sizeof(pt_ctyp), pt_ml,
                                         sizeof(pt_ml)));
          rc = cdd_c_keep_err(
              rc, cdd_ffi_write_fmt(fml, " _arg_%lu", (unsigned long)j));
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
          rc = cdd_ffi_write_fmt(fmli, "%s -> ", pt_ml);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        if (node->is_variadic) {
          exposed_inputs++;
          rc = cdd_ffi_write_str(fml, " _var_tail");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
          rc = cdd_ffi_write_str(fmli, "string -> ");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        if (exposed_inputs == 0) {
          rc = cdd_ffi_write_str(fml, " ()");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
          rc = cdd_ffi_write_str(fmli, "unit -> ");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = cdd_ffi_write_str(fml, " =\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        /* Allocate temporary Ctypes slots */
        for (j = 0; j < node->fields_count; j++) {
          int is_m = 0;
          cdd_ffi_type_t pt;
          if ((int)j == err_param_idx) {
            rc = cdd_ffi_write_fmt(
                fml,
                "  let _err_%lu = Ctypes.allocate cdd_ffi_error_t "
                "(make cdd_ffi_error_t) in\n",
                (unsigned long)j);
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
            continue;
          }
          rc = cdd_c_keep_err(
              rc, ocaml_check_out_param(ir, &node->fields[j], &is_m, &pt));
          if (is_m) {
            char pt_ctyp[512];
            char pt_ml[512];
            rc = cdd_c_keep_err(
                rc, ocaml_format_type_pair(ir, &pt, 0, pt_ctyp, sizeof(pt_ctyp),
                                           pt_ml, sizeof(pt_ml)));
            if (node->fields[j].intent == CDD_FFI_INTENT_INOUT) {
              rc = cdd_c_keep_err(
                  rc,
                  cdd_ffi_write_fmt(
                      fml, "  let _out_%lu = Ctypes.allocate %s _arg_%lu in\n",
                      (unsigned long)j, pt_ctyp, (unsigned long)j));
            } else {
              char zero_expr[128];
              rc = cdd_c_keep_err(rc,
                                  ocaml_pointee_zero_expr(ir, &pt, zero_expr,
                                                          sizeof(zero_expr)));
              rc = cdd_c_keep_err(
                  rc, cdd_ffi_write_fmt(
                          fml, "  let _out_%lu = Ctypes.allocate %s %s in\n",
                          (unsigned long)j, pt_ctyp, zero_expr));
            }
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
        }

        /* Invoke _raw_<fn> */
        rc = cdd_ffi_write_fmt(fml, "  let _ret = _raw_%s", fn_lid);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        if (node->fields_count == 0 && !node->is_variadic) {
          rc = cdd_ffi_write_str(fml, " ()");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        } else {
          for (j = 0; j < node->fields_count; j++) {
            int is_m = 0;
            cdd_ffi_type_t pt;
            if ((int)j == err_param_idx) {
              rc = cdd_ffi_write_fmt(fml, " _err_%lu", (unsigned long)j);
              if (rc != CDD_C_SUCCESS)
                goto cleanup;
              continue;
            }
            rc = cdd_c_keep_err(
                rc, ocaml_check_out_param(ir, &node->fields[j], &is_m, &pt));
            rc = cdd_c_keep_err(
                rc, cdd_ffi_write_fmt(fml, is_m ? " _out_%lu" : " _arg_%lu",
                                      (unsigned long)j));
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
          if (node->is_variadic) {
            rc = cdd_ffi_write_str(fml, " _var_tail");
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
        }
        rc = cdd_ffi_write_str(fml, " in\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        /* Build value tuple & optional Result wrapping */
        {
          char val_expr[1024];
          char val_ml[1024];
          size_t n_elems = (ret_is_void ? 0 : 1) + out_count;

          val_expr[0] = '\0';
          val_ml[0] = '\0';

          if (n_elems == 0) {
            rc =
                cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                       val_expr, sizeof(val_expr), "%s", "()"));
            rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                        val_ml, sizeof(val_ml), "%s", "unit"));
          } else {
            size_t emitted_e = 0;
            if (n_elems > 1) {
              rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(val_expr,
                                                               sizeof(val_expr),
                                                               "%s", "("));
              rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                          val_ml, sizeof(val_ml), "%s", "("));
            }
            if (!ret_is_void) {
              char te[1024];
              char tm[1024];
              rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                          te, sizeof(te), "%s_ret", val_expr));
              rc =
                  cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                         val_expr, sizeof(val_expr), "%s", te));
              rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(tm, sizeof(tm),
                                                               "%s%s", val_ml,
                                                               ret_ml));
              rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                          val_ml, sizeof(val_ml), "%s", tm));
              emitted_e++;
            }
            for (j = 0; j < node->fields_count; j++) {
              int is_m = 0;
              cdd_ffi_type_t pt;
              if ((int)j == err_param_idx)
                continue;
              rc = cdd_c_keep_err(
                  rc, ocaml_check_out_param(ir, &node->fields[j], &is_m, &pt));
              if (is_m) {
                char pt_ctyp[512];
                char pt_ml[512];
                char te[1024];
                char tm[1024];
                rc = cdd_c_keep_err(
                    rc,
                    ocaml_format_type_pair(ir, &pt, 0, pt_ctyp, sizeof(pt_ctyp),
                                           pt_ml, sizeof(pt_ml)));
                rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                            te, sizeof(te), "%s%s(!@ _out_%lu)",
                                            val_expr, emitted_e > 0 ? ", " : "",
                                            (unsigned long)j));
                rc = cdd_c_keep_err(
                    rc, cdd_ffi_snprintf_checked(val_expr, sizeof(val_expr),
                                                 "%s", te));
                rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                            tm, sizeof(tm), "%s%s%s", val_ml,
                                            emitted_e > 0 ? " * " : "", pt_ml));
                rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                            val_ml, sizeof(val_ml), "%s", tm));
                emitted_e++;
              }
            }
            if (n_elems > 1) {
              char te[1024];
              char tm[1024];
              rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                          te, sizeof(te), "%s)", val_expr));
              rc =
                  cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                         val_expr, sizeof(val_expr), "%s", te));
              rc = cdd_c_keep_err(
                  rc, cdd_ffi_snprintf_checked(tm, sizeof(tm), "%s)", val_ml));
              rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                          val_ml, sizeof(val_ml), "%s", tm));
            }
          }

          if (ret_err_kind == 1) {
            rc = cdd_c_keep_err(
                rc,
                cdd_ffi_write_fmt(
                    fml,
                    "  let _code = getf _ret cdd_ffi_error_t_code in\n"
                    "  if _code <> 0 then\n"
                    "    let _msg = match getf _ret cdd_ffi_error_t_message "
                    "with Some s -> s | None -> \"FFI error\" in\n"
                    "    Error _msg\n"
                    "  else\n"
                    "    Ok %s\n\n",
                    val_expr));
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
            rc = cdd_ffi_write_fmt(fmli, "(%s, string) result\n\n", val_ml);
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          } else if (err_param_idx >= 0) {
            rc = cdd_c_keep_err(
                rc, cdd_ffi_write_fmt(
                        fml,
                        "  let _err_val = !@ _err_%d in\n"
                        "  let _code = getf _err_val cdd_ffi_error_t_code in\n"
                        "  if _code <> 0 then\n"
                        "    let _msg = match getf _err_val "
                        "cdd_ffi_error_t_message with Some s -> s | None -> "
                        "\"FFI error\" in\n"
                        "    Error _msg\n"
                        "  else\n"
                        "    Ok %s\n\n",
                        err_param_idx, val_expr));
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
            rc = cdd_ffi_write_fmt(fmli, "(%s, string) result\n\n", val_ml);
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          } else {
            rc = cdd_c_keep_err(rc,
                                cdd_ffi_write_fmt(fml, "  %s\n\n", val_expr));
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
            rc = cdd_ffi_write_fmt(fmli, "%s\n\n", val_ml);
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          }
        }
      }
    }
  }

  /* Static C-stub external declarations (`Stubs` submodule) */
  rc = cdd_ffi_write_fmt(fml,
                         "module Stubs = struct\n"
                         "  external stub_abi_version : unit -> int = "
                         "\"cdd_ocaml_%s_abi_version\"\n",
                         lib_stem);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_ffi_write_str(fmli, "module Stubs : sig\n"
                               "  val stub_abi_version : unit -> int\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < ir->nodes_count; i++) {
    if (ir->nodes[i].kind == CDD_FFI_NODE_FUNCTION && ir->nodes[i].name) {
      char fn_lid[128];
      rc = cdd_c_keep_err(rc, cdd_ocaml_sanitize_lid(ir->nodes[i].name, fn_lid,
                                                     sizeof(fn_lid)));
      rc = cdd_c_keep_err(
          rc,
          cdd_ffi_write_fmt(
              fml,
              "  external stub_ping_%s : unit -> int = \"cdd_ocaml_stub_%s\"\n",
              fn_lid, fn_lid));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc =
          cdd_ffi_write_fmt(fmli, "  val stub_ping_%s : unit -> int\n", fn_lid);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  rc = cdd_ffi_write_str(fml, "end\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_ffi_write_str(fmli, "end\n");

cleanup:
  rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fml));
  rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fmli));
  return rc;
}

/**
 * @brief Emits strict ISO C89 OCaml C-stub companion file `<lib>_stubs.c`.
 *
 * @param[in] ir FFI IR.
 * @param[in] config Generation configuration.
 * @param[in] lib_stem Sanitized library stem.
 * @return CDD_C_SUCCESS on success, or error code on I/O failure.
 */
static NO_DISCARD cdd_c_error_t emit_ocaml_c_stubs(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config,
    const char *lib_stem) {
  char stub_file[256];
  FILE *fp = NULL;
  size_t i;
  cdd_c_error_t rc;

  rc = cdd_ffi_snprintf_checked(stub_file, sizeof(stub_file), "%s_stubs.c",
                                lib_stem);
  rc = cdd_c_keep_err(
      rc, cdd_ffi_open_output_file(config->output_dir, stub_file, &fp));
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_ffi_write_fmt(
      fp,
      "/* Auto-generated strict ISO C89 OCaml C stubs for %s */\n"
      "/* "
      "clang-format off */\n"
      "#if !defined(CDD_OCAML_STUB_STANDALONE)\n"
      "#include <caml/mlvalues.h>\n"
      "#include <caml/memory.h>\n"
      "#include <caml/alloc.h>\n"
      "#include <caml/threads.h>\n"
      "#else\n",
      lib_stem);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_ffi_write_str(
      fp, "typedef long value;\n"
          "#define Val_int(x) ((value)(((x) << 1) + 1))\n"
          "#define CAMLparam1(x) value _caml_keep_##x = (x)\n"
          "#define CAMLlocal1(x) value x = 0\n"
          "#define CAMLreturn(x) do { if (_caml_keep_unit) {} return (x); } "
          "while (0)\n"
          "#define caml_release_runtime_system() do {} while (0)\n"
          "#define caml_acquire_runtime_system() do {} while (0)\n"
          "#endif\n"
          "/* "
          "clang-format on */\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_ffi_write_fmt(fp,
                         "value cdd_ocaml_%s_abi_version(value unit);\n"
                         "value cdd_ocaml_%s_abi_version(value unit) {\n"
                         "  CAMLparam1(unit);\n"
                         "  CAMLlocal1(res);\n"
                         "  res = Val_int(1);\n"
                         "  CAMLreturn(res);\n"
                         "}\n\n",
                         lib_stem, lib_stem);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < ir->nodes_count; i++) {
    if (ir->nodes[i].kind == CDD_FFI_NODE_FUNCTION && ir->nodes[i].name) {
      char fn_lid[128];
      rc = cdd_c_keep_err(rc, cdd_ocaml_sanitize_lid(ir->nodes[i].name, fn_lid,
                                                     sizeof(fn_lid)));
      rc = cdd_c_keep_err(
          rc, cdd_ffi_write_fmt(fp,
                                "value cdd_ocaml_stub_%s(value unit);\n"
                                "value cdd_ocaml_stub_%s(value unit) {\n"
                                "  CAMLparam1(unit);\n"
                                "  CAMLlocal1(res);\n",
                                fn_lid, fn_lid));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      if (ir->nodes[i].requires_gil_release) {
        rc = cdd_ffi_write_str(fp, "  caml_release_runtime_system();\n"
                                   "  caml_acquire_runtime_system();\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }

      rc = cdd_ffi_write_str(fp, "  res = Val_int(0);\n"
                                 "  CAMLreturn(res);\n"
                                 "}\n\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

cleanup:
  return cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fp));
}

/**
 * @brief Emits `dune-project`, `dune`, and (when `config->generate_tests != 0`)
 * `<lib>_test.ml`.
 *
 * @param[in] ir FFI IR.
 * @param[in] config Generation configuration.
 * @param[in] lib_stem Sanitized library stem.
 * @return CDD_C_SUCCESS on success, or error code on I/O failure.
 */
static NO_DISCARD cdd_c_error_t emit_ocaml_dune_and_tests(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config,
    const char *lib_stem) {
  FILE *fp = NULL;
  cdd_c_error_t rc;

  rc = cdd_ffi_open_output_file(config->output_dir, "dune-project", &fp);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_ffi_write_fmt(fp,
                         "(lang dune 3.0)\n"
                         "(name %s)\n",
                         lib_stem);
  rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fp));
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_ffi_open_output_file(config->output_dir, "dune", &fp);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_ffi_write_fmt(fp,
                         "(library\n"
                         " (name %s)\n"
                         " (modules %s)\n"
                         " (libraries ctypes ctypes.foreign)\n"
                         " (foreign_stubs\n"
                         "  (language c)\n"
                         "  (names %s_stubs)))\n",
                         lib_stem, lib_stem, lib_stem);
  if (rc == CDD_C_SUCCESS && config->generate_tests) {
    rc = cdd_ffi_write_fmt(fp,
                           "\n(test\n"
                           " (name %s_test)\n"
                           " (modules %s_test)\n"
                           " (libraries %s ctypes ctypes.foreign))\n",
                           lib_stem, lib_stem, lib_stem);
  }
  rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fp));
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (config->generate_tests) {
    char test_file[256];
    char mod_uid[128];
    size_t i;

    rc = cdd_ffi_snprintf_checked(test_file, sizeof(test_file), "%s_test.ml",
                                  lib_stem);
    rc = cdd_c_keep_err(
        rc, cdd_ocaml_sanitize_uid(lib_stem, mod_uid, sizeof(mod_uid)));
    rc = cdd_c_keep_err(
        rc, cdd_ffi_open_output_file(config->output_dir, test_file, &fp));
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = cdd_ffi_write_fmt(fp,
                           "(* Auto-generated OCaml Ctypes layout and binding "
                           "tests for %s *)\n\n"
                           "open Ctypes\n\n"
                           "let () =\n",
                           lib_stem);
    if (rc != CDD_C_SUCCESS)
      goto test_cleanup;

    for (i = 0; i < ir->nodes_count; i++) {
      cdd_ffi_ir_node_t *node = &ir->nodes[i];
      if ((node->kind == CDD_FFI_NODE_STRUCT ||
           node->kind == CDD_FFI_NODE_UNION) &&
          node->fields_count > 0 && node->name) {
        char s_lid[128];
        rc = cdd_c_keep_err(
            rc, cdd_ocaml_sanitize_lid(node->name, s_lid, sizeof(s_lid)));
        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fp, "  assert (sizeof %s.%s > 0);\n", mod_uid,
                                  s_lid));
        if (rc != CDD_C_SUCCESS)
          goto test_cleanup;
      } else if (node->kind == CDD_FFI_NODE_ENUM && node->variants_count > 0 &&
                 node->name) {
        char e_uid[128];
        char v_lid[128];
        rc = cdd_c_keep_err(
            rc, cdd_ocaml_sanitize_uid(node->name, e_uid, sizeof(e_uid)));
        rc = cdd_c_keep_err(rc, cdd_ocaml_sanitize_lid(node->variants[0].name,
                                                       v_lid, sizeof(v_lid)));
        rc = cdd_c_keep_err(
            rc,
            cdd_ffi_write_fmt(
                fp,
                "  assert (%s.%s.to_int (%s.%s.of_int %s.%s.%s) = %s.%s.%s);\n",
                mod_uid, e_uid, mod_uid, e_uid, mod_uid, e_uid, v_lid, mod_uid,
                e_uid, v_lid));
        if (rc != CDD_C_SUCCESS)
          goto test_cleanup;
      }
    }

    rc = cdd_ffi_write_str(fp, "  ()\n");

  test_cleanup:
    rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fp));
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits OCaml (`Ctypes` `.ml`, `.mli`, `dune`, `dune-project`, C89
 * `_stubs.c`, and optional `_test.ml`) bindings for the given FFI IR.
 *
 * @param[in,out] ir Extracted and topologically sorted FFI IR.
 * @param[in] config Bindings generation configuration.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * CDD_C_ERROR_MEMORY on OOM/truncation, or CDD_C_ERROR_IO on file I/O failure.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_emit_ocaml(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  const char *raw_lib;
  char lib_stem[OCAML_MAX_FILE_STEM];
  cdd_c_error_t rc;

  if (!ir || !config || !config->output_dir)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  raw_lib = config->library_name
                ? config->library_name
                : (ir->module_name ? ir->module_name : "mylib");
  rc = cdd_ocaml_sanitize_lid(raw_lib, lib_stem, sizeof(lib_stem));
  rc = cdd_c_keep_err(rc, emit_ocaml_ml_and_mli(ir, config, lib_stem));
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = emit_ocaml_c_stubs(ir, config, lib_stem);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return emit_ocaml_dune_and_tests(ir, config, lib_stem);
}
