/**
 * @file struct_reflect_emit.c
 * @brief Code generation for struct/function reflection tables and C11 _Generic
 * dispatch macros.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/emit/struct_internal.h"
#include "c_cdd/code_writer.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "classes/parse/cdd_serde.h"
#include "classes/parse/code2schema.h"
#include "classes/parse/inspector.h"
#include "win_compat_sym.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

static cdd_c_error_t bw_parse_expr(const char **p, int *saw_digit,
                                   long *out_val);

/**
 * @brief Skips leading ASCII spaces and tabs in a bit-width expression cursor.
 *
 * @param[in,out] p Pointer to current character cursor.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t bw_skip_ws(const char **p) {
  while (**p == ' ' || **p == '\t')
    (*p)++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a primary bit-width factor: parenthesized sub-expression, unary
 * `+`/`-`/`~`, or decimal/hexadecimal/binary integer literal.
 *
 * @param[in,out] p Pointer to current character cursor.
 * @param[in,out] saw_digit Set to 1 when at least one numeric literal is
 * parsed.
 * @param[out] out_val Evaluated factor value.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t bw_parse_primary(const char **p, int *saw_digit,
                                      long *out_val) {
  long v = 0;
  cdd_c_error_t rc = bw_skip_ws(p);
  if (**p == '(') {
    (*p)++;
    rc = bw_parse_expr(p, saw_digit, &v);
    rc = bw_skip_ws(p);
    if (**p == ')')
      (*p)++;
    *out_val = v;
    return rc;
  }
  if (**p == '+') {
    (*p)++;
    return bw_parse_primary(p, saw_digit, out_val);
  }
  if (**p == '-') {
    (*p)++;
    rc = bw_parse_primary(p, saw_digit, &v);
    *out_val = -v;
    return rc;
  }
  if (**p == '~') {
    (*p)++;
    rc = bw_parse_primary(p, saw_digit, &v);
    *out_val = ~v;
    return rc;
  }
  if (**p == '\'') {
    (*p)++;
    if (**p != '\0') {
      *saw_digit = 1;
      v = (long)((unsigned char)**p);
      (*p)++;
      if (**p == '\'')
        (*p)++;
    }
    *out_val = v;
    return rc;
  }
  if ((*p)[0] == '0' && ((*p)[1] == 'x' || (*p)[1] == 'X')) {
    (*p) += 2;
    *saw_digit = 1;
    while ((**p >= '0' && **p <= '9') || (**p >= 'a' && **p <= 'f') ||
           (**p >= 'A' && **p <= 'F')) {
      long d = (**p <= '9') ? (long)(**p - '0')
                            : ((**p >= 'a') ? (long)(**p - 'a' + 10)
                                            : (long)(**p - 'A' + 10));
      v = v * 16 + d;
      (*p)++;
    }
  } else if ((*p)[0] == '0' && ((*p)[1] == 'b' || (*p)[1] == 'B')) {
    (*p) += 2;
    *saw_digit = 1;
    while (**p == '0' || **p == '1') {
      v = v * 2 + (long)(**p - '0');
      (*p)++;
    }
  } else if (**p >= '0' && **p <= '9') {
    *saw_digit = 1;
    while (**p >= '0' && **p <= '9') {
      v = v * 10 + (long)(**p - '0');
      (*p)++;
    }
  }
  while (**p == 'u' || **p == 'U' || **p == 'l' || **p == 'L')
    (*p)++;
  *out_val = v;
  return rc;
}

/**
 * @brief Parses multiplicative (`*`, `/`, `%`) bit-width sub-expressions.
 *
 * @param[in,out] p Pointer to current character cursor.
 * @param[in,out] saw_digit Set to 1 when at least one numeric literal is
 * parsed.
 * @param[out] out_val Evaluated term value.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t bw_parse_mul(const char **p, int *saw_digit,
                                  long *out_val) {
  long lhs = 0;
  cdd_c_error_t rc = bw_parse_primary(p, saw_digit, &lhs);
  for (;;) {
    long rhs = 0;
    char op;
    rc = bw_skip_ws(p);
    op = **p;
    if (op != '*' && op != '/' && op != '%')
      break;
    (*p)++;
    rc = bw_parse_primary(p, saw_digit, &rhs);
    if (op == '*')
      lhs *= rhs;
    else if (op == '/' && rhs != 0)
      lhs /= rhs;
    else if (op == '%' && rhs != 0)
      lhs %= rhs;
  }
  *out_val = lhs;
  return rc;
}

/**
 * @brief Parses additive (`+`, `-`) bit-width sub-expressions.
 *
 * @param[in,out] p Pointer to current character cursor.
 * @param[in,out] saw_digit Set to 1 when at least one numeric literal is
 * parsed.
 * @param[out] out_val Evaluated additive value.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t bw_parse_add(const char **p, int *saw_digit,
                                  long *out_val) {
  long lhs = 0;
  cdd_c_error_t rc = bw_parse_mul(p, saw_digit, &lhs);
  for (;;) {
    long rhs = 0;
    char op;
    rc = bw_skip_ws(p);
    op = **p;
    if (op != '+' && op != '-')
      break;
    (*p)++;
    rc = bw_parse_mul(p, saw_digit, &rhs);
    if (op == '+')
      lhs += rhs;
    else
      lhs -= rhs;
  }
  *out_val = lhs;
  return rc;
}

/**
 * @brief Parses shift (`<<`, `>>`) bit-width sub-expressions.
 *
 * @param[in,out] p Pointer to current character cursor.
 * @param[in,out] saw_digit Set to 1 when at least one numeric literal is
 * parsed.
 * @param[out] out_val Evaluated shift value.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t bw_parse_shift(const char **p, int *saw_digit,
                                    long *out_val) {
  long lhs = 0;
  cdd_c_error_t rc = bw_parse_add(p, saw_digit, &lhs);
  for (;;) {
    long rhs = 0;
    int is_shl = 0;
    rc = bw_skip_ws(p);
    if ((*p)[0] == '<' && (*p)[1] == '<') {
      is_shl = 1;
      (*p) += 2;
    } else if ((*p)[0] == '>' && (*p)[1] == '>') {
      is_shl = 0;
      (*p) += 2;
    } else {
      break;
    }
    rc = bw_parse_add(p, saw_digit, &rhs);
    if (is_shl)
      lhs = lhs << rhs;
    else
      lhs = lhs >> rhs;
  }
  *out_val = lhs;
  return rc;
}

/**
 * @brief Parses bitwise (`&`, `^`, `|`) bit-width expressions.
 *
 * @param[in,out] p Pointer to current character cursor.
 * @param[in,out] saw_digit Set to 1 when at least one numeric literal is
 * parsed.
 * @param[out] out_val Evaluated expression value.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t bw_parse_expr(const char **p, int *saw_digit,
                                   long *out_val) {
  long lhs = 0;
  cdd_c_error_t rc = bw_parse_shift(p, saw_digit, &lhs);
  for (;;) {
    long rhs = 0;
    char op;
    rc = bw_skip_ws(p);
    op = **p;
    if (op != '&' && op != '^' && op != '|')
      break;
    (*p)++;
    rc = bw_parse_shift(p, saw_digit, &rhs);
    if (op == '&')
      lhs &= rhs;
    else if (op == '^')
      lhs ^= rhs;
    else
      lhs |= rhs;
  }
  *out_val = lhs;
  return rc;
}

/**
 * @brief Evaluates a bit-field width string (such as `"3"`, `"(8)"`,
 * `"(1 + 2 * 3)"`, `"1 << 2"`, `"0x4"`, or a symbolic constant `"MAX_BITS"`)
 * into a non-zero unsigned integer width with full C operator precedence.
 *
 * @param[in] bw_str Bit-width expression string.
 * @param[out] out_bw Output evaluated bit-width (`0` if empty or `"0"`).
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t cdd_struct_eval_bitwidth_str(const char *bw_str,
                                           unsigned int *out_bw) {
  const char *p = bw_str;
  long acc = 0;
  int saw_digit = 0;
  cdd_c_error_t rc;

  *out_bw = 0U;
  if (!bw_str || bw_str[0] == '\0')
    return CDD_C_SUCCESS;

  rc = bw_parse_expr(&p, &saw_digit, &acc);

  if (saw_digit) {
    *out_bw = (acc > 0) ? (unsigned int)acc : 0U;
  } else {
    *out_bw = 1U;
  }
  return rc;
}

/*
 * @brief Emits runtime reflection descriptor tables (`struct cdd_field_info[]`
 * and `struct cdd_type_info`) and `<struct_name>_reflect` accessor for a C
 * struct.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Optional configuration options.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t
write_struct_reflect_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config) {
  size_t i;
  const char *member_prefix;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  member_prefix = sf->is_union ? "data." : "";

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  /* Emit bit-field getter/setter trampolines and alias tables */
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    unsigned int bw = 0U;
    rc = cdd_struct_eval_bitwidth_str(f->bit_width, &bw);
    if (bw > 0 && f->name[0] != '\0') {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "static cdd_c_error_t %s_get_bf_%s(const void *obj, int *out_val) {\n"
          "  if (!obj || !out_val) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  *out_val = (int)((const struct %s *)obj)->%s%s;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"
          "static cdd_c_error_t %s_set_bf_%s(void *obj, int val) {\n"
          "  if (!obj) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  ((struct %s *)obj)->%s%s = val;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n",
          struct_name, f->name, struct_name, member_prefix, f->name,
          struct_name, f->name, struct_name, member_prefix, f->name));
    }
    if (f->n_serde_aliases > 0 && f->serde_aliases) {
      size_t a_idx;
      CHECK_IO(FPRINTF_HOOK(fp, "static const char *const %s_%s_aliases[] = { ",
                            struct_name, f->name));
      for (a_idx = 0; a_idx < f->n_serde_aliases; ++a_idx) {
        char esc_al[256];
        rc = cdd_escape_json_c_str(f->serde_aliases[a_idx], esc_al,
                                   sizeof(esc_al));
        CHECK_IO(
            FPRINTF_HOOK(fp, "\"%s\"%s", esc_al,
                         (a_idx + 1 < f->n_serde_aliases) ? ", " : " };\n"));
      }
    }
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "static const struct cdd_field_info %s_fields[] = {\n", struct_name));

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    const char *kind_str = "CDD_FIELD_KIND_UNKNOWN";
    const char *elem_kind_str = "CDD_FIELD_KIND_UNKNOWN";
    const char *ref_out = "";
    char wire_name[128];
    char de_wire_name[128];
    char elem_size_expr[192];
    char alias_ref[160];
    char esc_doc[512];
    char esc_fmt[128];
    char esc_pat[256];
    char esc_def[256];
    unsigned int bw = 0U;
    int is_ptr;
    int eff_has_min_len;
    size_t eff_min_len;
    int eff_has_max_len;
    size_t eff_max_len;
    rc = cdd_struct_eval_bitwidth_str(f->bit_width, &bw);
    is_ptr = ((strcmp(f->type, "string") == 0 && f->fixed_array_len == 0 &&
               !f->is_flexible_array) ||
              (strcmp(f->type, "object") == 0 && f->c_type[0] == '\0') ||
              (strcmp(f->type, "array") == 0 && f->fixed_array_len == 0 &&
               !f->is_flexible_array) ||
              f->is_nullable_ptr)
                 ? 1
                 : 0;
    eff_has_min_len = (strcmp(f->type, "array") == 0 && f->has_min_items)
                          ? 1
                          : f->has_min_len;
    eff_min_len = (strcmp(f->type, "array") == 0 && f->has_min_items)
                      ? f->min_items
                      : f->min_len;
    eff_has_max_len = (strcmp(f->type, "array") == 0 && f->has_max_items)
                          ? 1
                          : f->has_max_len;
    eff_max_len = (strcmp(f->type, "array") == 0 && f->has_max_items)
                      ? f->max_items
                      : f->max_len;

    if (cdd_serde_resolve_field_names(sf, i, wire_name, sizeof(wire_name),
                                      de_wire_name,
                                      sizeof(de_wire_name)) != CDD_C_SUCCESS) {
      CDD_STRCPY(wire_name, sizeof(wire_name), f->name);
      CDD_STRCPY(de_wire_name, sizeof(de_wire_name), f->name);
    }
    if (f->ref[0] != '\0') {
      rc = get_type_from_ref(f->ref, &ref_out);
    }
    if (strcmp(f->type, "integer") == 0)
      kind_str = "CDD_FIELD_KIND_INTEGER";
    else if (strcmp(f->type, "number") == 0)
      kind_str = "CDD_FIELD_KIND_NUMBER";
    else if (strcmp(f->type, "boolean") == 0)
      kind_str = "CDD_FIELD_KIND_BOOLEAN";
    else if (strcmp(f->type, "string") == 0)
      kind_str = "CDD_FIELD_KIND_STRING";
    else if (strcmp(f->type, "enum") == 0)
      kind_str = "CDD_FIELD_KIND_ENUM";
    else if (strcmp(f->type, "object") == 0)
      kind_str = "CDD_FIELD_KIND_OBJECT";
    else if (strcmp(f->type, "array") == 0) {
      kind_str = "CDD_FIELD_KIND_ARRAY";
      if (strcmp(f->ref, "integer") == 0)
        elem_kind_str = "CDD_FIELD_KIND_INTEGER";
      else if (strcmp(f->ref, "number") == 0)
        elem_kind_str = "CDD_FIELD_KIND_NUMBER";
      else if (strcmp(f->ref, "boolean") == 0)
        elem_kind_str = "CDD_FIELD_KIND_BOOLEAN";
      else if (strcmp(f->ref, "string") == 0)
        elem_kind_str = "CDD_FIELD_KIND_STRING";
      else if (f->is_enum_array)
        elem_kind_str = "CDD_FIELD_KIND_ENUM";
      else
        elem_kind_str = "CDD_FIELD_KIND_OBJECT";
    }

    if (strcmp(f->type, "array") == 0 && !f->is_flexible_array) {
      CDD_SNPRINTF(elem_size_expr, sizeof(elem_size_expr),
                   "sizeof(*((struct %s *)0)->%s%s)", struct_name,
                   member_prefix, f->name);
    } else {
      CDD_STRCPY(elem_size_expr, sizeof(elem_size_expr), "0");
    }

    if (f->n_serde_aliases > 0 && f->serde_aliases) {
      CDD_SNPRINTF(alias_ref, sizeof(alias_ref), "%s_%s_aliases", struct_name,
                   f->name);
    } else {
      CDD_STRCPY(alias_ref, sizeof(alias_ref), "NULL");
    }

    rc = cdd_escape_json_c_str(f->description, esc_doc, sizeof(esc_doc));
    rc = cdd_escape_json_c_str(f->format, esc_fmt, sizeof(esc_fmt));
    rc = cdd_escape_json_c_str(f->pattern, esc_pat, sizeof(esc_pat));
    rc = cdd_escape_json_c_str(f->default_val, esc_def, sizeof(esc_def));

    if (bw > 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { \"%s\", \"%s\", \"%s\", \"%s\", %s, 0, 0, 0, %u, %d, %d, "
          "\"%s\", 0, %d, %d, %lu, %d, %d, %s_get_bf_%s, %s_set_bf_%s, "
          "%s, %s, %s, %lu, \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, "
          "%d, %f, %d, %f, %d, %lu, %d, %lu, %d, %d, %d },\n",
          f->name, wire_name, f->c_type, ref_out, kind_str, bw, f->required,
          f->serde_skip, de_wire_name, f->is_nullable_ptr, f->is_flexible_array,
          (unsigned long)f->fixed_array_len, f->serde_skip_serializing,
          f->serde_skip_deserializing, struct_name, f->name, struct_name,
          f->name, elem_size_expr, elem_kind_str, alias_ref,
          (unsigned long)f->n_serde_aliases, esc_doc, esc_fmt, esc_pat, esc_def,
          f->deprecated, f->read_only, f->write_only, f->serde_flatten,
          f->has_min, f->min_val, f->has_max, f->max_val, eff_has_min_len,
          (unsigned long)eff_min_len, eff_has_max_len,
          (unsigned long)eff_max_len, f->exclusive_min, f->exclusive_max,
          f->unique_items));
    } else if (strcmp(f->type, "array") == 0 && f->fixed_array_len == 0 &&
               !f->is_flexible_array) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { \"%s\", \"%s\", \"%s\", \"%s\", %s, offsetof(struct %s, %s%s), "
          "sizeof(((struct %s *)0)->%s%s), offsetof(struct %s, n_%s), 0, %d, "
          "%d, \"%s\", %d, %d, %d, %lu, %d, %d, NULL, NULL, "
          "%s, %s, %s, %lu, \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, "
          "%d, %f, %d, %f, %d, %lu, %d, %lu, %d, %d, %d },\n",
          f->name, wire_name, f->c_type, ref_out, kind_str, struct_name,
          member_prefix, f->name, struct_name, member_prefix, f->name,
          struct_name, f->name, f->required, f->serde_skip, de_wire_name,
          is_ptr, f->is_nullable_ptr, f->is_flexible_array,
          (unsigned long)f->fixed_array_len, f->serde_skip_serializing,
          f->serde_skip_deserializing, elem_size_expr, elem_kind_str, alias_ref,
          (unsigned long)f->n_serde_aliases, esc_doc, esc_fmt, esc_pat, esc_def,
          f->deprecated, f->read_only, f->write_only, f->serde_flatten,
          f->has_min, f->min_val, f->has_max, f->max_val, eff_has_min_len,
          (unsigned long)eff_min_len, eff_has_max_len,
          (unsigned long)eff_max_len, f->exclusive_min, f->exclusive_max,
          f->unique_items));
    } else if (f->is_flexible_array) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { \"%s\", \"%s\", \"%s\", \"%s\", %s, offsetof(struct %s, %s%s), "
          "0, 0, 0, %d, %d, \"%s\", %d, %d, %d, %lu, %d, %d, NULL, NULL, "
          "%s, %s, %s, %lu, \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, "
          "%d, %f, %d, %f, %d, %lu, %d, %lu, %d, %d, %d },\n",
          f->name, wire_name, f->c_type, ref_out, kind_str, struct_name,
          member_prefix, f->name, f->required, f->serde_skip, de_wire_name,
          is_ptr, f->is_nullable_ptr, f->is_flexible_array,
          (unsigned long)f->fixed_array_len, f->serde_skip_serializing,
          f->serde_skip_deserializing, elem_size_expr, elem_kind_str, alias_ref,
          (unsigned long)f->n_serde_aliases, esc_doc, esc_fmt, esc_pat, esc_def,
          f->deprecated, f->read_only, f->write_only, f->serde_flatten,
          f->has_min, f->min_val, f->has_max, f->max_val, eff_has_min_len,
          (unsigned long)eff_min_len, eff_has_max_len,
          (unsigned long)eff_max_len, f->exclusive_min, f->exclusive_max,
          f->unique_items));
    } else {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { \"%s\", \"%s\", \"%s\", \"%s\", %s, offsetof(struct %s, %s%s), "
          "sizeof(((struct %s *)0)->%s%s), 0, 0, %d, %d, \"%s\", %d, %d, %d, "
          "%lu, %d, %d, NULL, NULL, "
          "%s, %s, %s, %lu, \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, "
          "%d, %f, %d, %f, %d, %lu, %d, %lu, %d, %d, %d },\n",
          f->name, wire_name, f->c_type, ref_out, kind_str, struct_name,
          member_prefix, f->name, struct_name, member_prefix, f->name,
          f->required, f->serde_skip, de_wire_name, is_ptr, f->is_nullable_ptr,
          f->is_flexible_array, (unsigned long)f->fixed_array_len,
          f->serde_skip_serializing, f->serde_skip_deserializing,
          elem_size_expr, elem_kind_str, alias_ref,
          (unsigned long)f->n_serde_aliases, esc_doc, esc_fmt, esc_pat, esc_def,
          f->deprecated, f->read_only, f->write_only, f->serde_flatten,
          f->has_min, f->min_val, f->has_max, f->max_val, eff_has_min_len,
          (unsigned long)eff_min_len, eff_has_max_len,
          (unsigned long)eff_max_len, f->exclusive_min, f->exclusive_max,
          f->unique_items));
    }
  }
  if (sf->is_union) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "  { NULL, NULL, NULL, NULL, CDD_FIELD_KIND_UNKNOWN, 0, 0, 0, 0, 0, 0, "
        "NULL, 0, 0, 0, 0, 0, 0, NULL, NULL, 0, CDD_FIELD_KIND_UNKNOWN, NULL, "
        "0, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0.0, 0, 0.0, 0, 0, 0, 0, 0, "
        "0, 0 }\n"
        "};\n"
        "static const struct cdd_type_info %s_type_info_table = {\n"
        "  \"%s\", sizeof(struct %s), %d, %s_fields, %lu, offsetof(struct %s, "
        "tag)\n"
        "};\n",
        struct_name, struct_name, struct_name, sf->is_union, struct_name,
        (unsigned long)sf->size, struct_name));
  } else {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "  { NULL, NULL, NULL, NULL, CDD_FIELD_KIND_UNKNOWN, 0, 0, 0, 0, 0, 0, "
        "NULL, 0, 0, 0, 0, 0, 0, NULL, NULL, 0, CDD_FIELD_KIND_UNKNOWN, NULL, "
        "0, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0.0, 0, 0.0, 0, 0, 0, 0, 0, "
        "0, 0 }\n"
        "};\n"
        "static const struct cdd_type_info %s_type_info_table = {\n"
        "  \"%s\", sizeof(struct %s), %d, %s_fields, %lu, 0\n"
        "};\n",
        struct_name, struct_name, struct_name, sf->is_union, struct_name,
        (unsigned long)sf->size));
  }
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_reflect(const struct cdd_type_info **out) {\n"
      "  if (!out) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "  *out = &%s_type_info_table;\n"
      "  return CDD_C_SUCCESS;\n"
      "}\n",
      struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "#ifdef CDD_REFLECT_AUTO_REGISTER\n"
      "#if defined(__GNUC__) || defined(__clang__)\n"
      "__attribute__((constructor)) static void %s_auto_register(void) {\n"
      "  cdd_reflect_register_type(&%s_type_info_table);\n"
      "}\n"
      "#elif defined(_MSC_VER)\n"
      "#pragma section(\".CRT$XCU\", read)\n"
      "static void __cdecl %s_auto_register(void) {\n"
      "  cdd_reflect_register_type(&%s_type_info_table);\n"
      "}\n"
      "__declspec(allocate(\".CRT$XCU\")) static void (__cdecl "
      "*%s_auto_reg_ptr)(void) = %s_auto_register;\n"
      "#endif\n"
      "#endif /* CDD_REFLECT_AUTO_REGISTER */\n",
      struct_name, struct_name, struct_name, struct_name, struct_name,
      struct_name));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return rc;
}

/**
 * @brief Classifies a raw C type string into a `CddFieldTypeKind` constant
 * name.
 *
 * @param[in] c_type Raw C type string.
 * @param[out] out_kind_str Output string literal for the `CddFieldTypeKind`.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t classify_c_type_kind_str(const char *c_type,
                                              const char **out_kind_str) {
  if (c_type[0] == '\0' || strcmp(c_type, "void") == 0) {
    *out_kind_str = "CDD_FIELD_KIND_UNKNOWN";
  } else if (strstr(c_type, "char") && strstr(c_type, "**")) {
    *out_kind_str = "CDD_FIELD_KIND_ARRAY";
  } else if (strstr(c_type, "char") && strchr(c_type, '*')) {
    *out_kind_str = "CDD_FIELD_KIND_STRING";
  } else if (strstr(c_type, "double") || strstr(c_type, "float")) {
    *out_kind_str = "CDD_FIELD_KIND_NUMBER";
  } else if (strstr(c_type, "bool") || strstr(c_type, "_Bool")) {
    *out_kind_str = "CDD_FIELD_KIND_BOOLEAN";
  } else if (strstr(c_type, "enum ")) {
    *out_kind_str = "CDD_FIELD_KIND_ENUM";
  } else if (strstr(c_type, "struct ") || strstr(c_type, "union ")) {
    *out_kind_str = "CDD_FIELD_KIND_OBJECT";
  } else if (strstr(c_type, "int") || strstr(c_type, "long") ||
             strstr(c_type, "short") || strstr(c_type, "size_t") ||
             strstr(c_type, "char")) {
    *out_kind_str = "CDD_FIELD_KIND_INTEGER";
  } else {
    *out_kind_str = "CDD_FIELD_KIND_UNKNOWN";
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Emits runtime function reflection descriptor tables
 * (`struct cdd_param_info[]` and `struct cdd_func_info`) and
 * `<func_name>_reflect` accessor for a C function signature.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] sig Extracted function signature descriptor.
 * @param[in] config Optional configuration options.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t
write_func_reflect_func(FILE *fp, const struct FuncSignature *sig,
                        const struct CodegenStructConfig *config) {
  size_t i;
  const char *ret_kind = "CDD_FIELD_KIND_UNKNOWN";
  char esc_doc[512];
  char route_method_lit[128];
  char route_path_lit[256];
  char summary_lit[256];
  char invoke_ref[160];
  int can_emit_trampoline = 1;
  cdd_c_error_t rc;

  if (!fp || !sig || !sig->name || sig->name[0] == '\0')
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  rc = classify_c_type_kind_str(sig->return_type, &ret_kind);
  rc =
      cdd_escape_json_c_str(sig->doc ? sig->doc : "", esc_doc, sizeof(esc_doc));

  if (sig->route_method) {
    char esc_rm[96];
    rc = cdd_escape_json_c_str(sig->route_method, esc_rm, sizeof(esc_rm));
    CDD_SNPRINTF(route_method_lit, sizeof(route_method_lit), "\"%s\"", esc_rm);
  } else {
    CDD_STRCPY(route_method_lit, sizeof(route_method_lit), "NULL");
  }

  if (sig->route_path) {
    char esc_rp[224];
    rc = cdd_escape_json_c_str(sig->route_path, esc_rp, sizeof(esc_rp));
    CDD_SNPRINTF(route_path_lit, sizeof(route_path_lit), "\"%s\"", esc_rp);
  } else {
    CDD_STRCPY(route_path_lit, sizeof(route_path_lit), "NULL");
  }

  if (sig->summary) {
    char esc_sum[224];
    rc = cdd_escape_json_c_str(sig->summary, esc_sum, sizeof(esc_sum));
    CDD_SNPRINTF(summary_lit, sizeof(summary_lit), "\"%s\"", esc_sum);
  } else {
    CDD_STRCPY(summary_lit, sizeof(summary_lit), "NULL");
  }

  if (sig->is_variadic || sig->return_type[0] == '\0') {
    can_emit_trampoline = 0;
  } else {
    for (i = 0; i < sig->n_params; ++i) {
      if (sig->params[i].type[0] == '\0') {
        can_emit_trampoline = 0;
        break;
      }
    }
  }

  if (can_emit_trampoline) {
    int is_void_ret = (strcmp(sig->return_type, "void") == 0) ? 1 : 0;
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "static cdd_c_error_t %s_invoke_trampoline(const void *const *args, "
        "void *ret_out) {\n",
        sig->name));
    if (sig->n_params > 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  if (!args) return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
    } else {
      CHECK_IO(FPRINTF_HOOK(fp, "  if (!args && 0) return CDD_C_SUCCESS;\n"));
    }
    if (is_void_ret) {
      CHECK_IO(FPRINTF_HOOK(fp, "  if (ret_out) { }\n  %s(", sig->name));
    } else {
      CHECK_IO(FPRINTF_HOOK(fp, "  if (ret_out) {\n    *(%s *)ret_out = %s(",
                            sig->return_type, sig->name));
    }
    for (i = 0; i < sig->n_params; ++i) {
      CHECK_IO(FPRINTF_HOOK(fp, "%s*(const %s *)args[%lu]", (i > 0) ? ", " : "",
                            sig->params[i].type, (unsigned long)i));
    }
    if (is_void_ret) {
      CHECK_IO(FPRINTF_HOOK(fp, ");\n  return CDD_C_SUCCESS;\n}\n"));
    } else {
      CHECK_IO(FPRINTF_HOOK(fp, ");\n  } else {\n    %s(", sig->name));
      for (i = 0; i < sig->n_params; ++i) {
        CHECK_IO(FPRINTF_HOOK(fp, "%s*(const %s *)args[%lu]",
                              (i > 0) ? ", " : "", sig->params[i].type,
                              (unsigned long)i));
      }
      CHECK_IO(FPRINTF_HOOK(fp, ");\n  }\n  return CDD_C_SUCCESS;\n}\n"));
    }
    CDD_SNPRINTF(invoke_ref, sizeof(invoke_ref), "%s_invoke_trampoline",
                 sig->name);
  } else {
    CDD_STRCPY(invoke_ref, sizeof(invoke_ref), "NULL");
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "static const struct cdd_param_info %s_params[] = {\n", sig->name));

  for (i = 0; i < sig->n_params; ++i) {
    const struct FuncParamReflection *p = &sig->params[i];
    const char *p_kind = "CDD_FIELD_KIND_UNKNOWN";
    char esc_pdoc[256];
    int is_ptr = (strchr(p->type, '*') != NULL) ? 1 : 0;
    int is_const = (strstr(p->type, "const") != NULL) ? 1 : 0;
    int is_out = (is_ptr && (!is_const || strstr(p->type, "**") != NULL ||
                             strncmp(p->name, "out", 3) == 0))
                     ? 1
                     : 0;
    int is_nullable = p->has_nullable_flag ? p->is_nullable : is_ptr;
    int req = p->has_required_flag ? p->required : 1;
    rc = classify_c_type_kind_str(p->type, &p_kind);
    rc =
        cdd_escape_json_c_str(p->doc ? p->doc : "", esc_pdoc, sizeof(esc_pdoc));
    CHECK_IO(FPRINTF_HOOK(
        fp, "  { \"%s\", \"%s\", %s, %d, %d, %d, %d, %d, \"%s\" },\n", p->name,
        p->type, p_kind, is_ptr, is_const, is_nullable, is_out, req, esc_pdoc));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "  { NULL, NULL, CDD_FIELD_KIND_UNKNOWN, 0, 0, 0, 0, 0, NULL }\n"
      "};\n"
      "static const struct cdd_func_info %s_func_info_table = {\n"
      "  \"%s\", \"%s\", %s, %s_params, %lu, \"%s\", %s, %s, %s, %d, %d, %s\n"
      "};\n"
      "cdd_c_error_t %s_reflect(const struct cdd_func_info **out) {\n"
      "  if (!out) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "  *out = &%s_func_info_table;\n"
      "  return CDD_C_SUCCESS;\n"
      "}\n",
      sig->name, sig->name, sig->return_type, ret_kind, sig->name,
      (unsigned long)sig->n_params, esc_doc, route_method_lit, route_path_lit,
      summary_lit, sig->is_variadic, sig->is_deprecated, invoke_ref, sig->name,
      sig->name));
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "#ifdef CDD_REFLECT_AUTO_REGISTER\n"
      "#if defined(__GNUC__) || defined(__clang__)\n"
      "__attribute__((constructor)) static void %s_auto_register(void) {\n"
      "  cdd_reflect_register_func(&%s_func_info_table);\n"
      "}\n"
      "#elif defined(_MSC_VER)\n"
      "#pragma section(\".CRT$XCU\", read)\n"
      "static void __cdecl %s_auto_register(void) {\n"
      "  cdd_reflect_register_func(&%s_func_info_table);\n"
      "}\n"
      "__declspec(allocate(\".CRT$XCU\")) static void (__cdecl "
      "*%s_auto_reg_ptr)(void) = %s_auto_register;\n"
      "#endif\n"
      "#endif /* CDD_REFLECT_AUTO_REGISTER */\n",
      sig->name, sig->name, sig->name, sig->name, sig->name, sig->name));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return rc;
}

/*
 * @brief Emits C11/C23 `_Generic` polymorphic reflection dispatch macros
 * (`CDD_REFLECT`, `CDD_TO_JSON`, `CDD_FROM_JSON`, `CDD_FROM_CLI`,
 * `CDD_TO_CLI_ARGV`, `CDD_TO_FORM`, `CDD_FROM_FORM`, `CDD_EQ`,
 * `CDD_DEEPCOPY`, `CDD_DEFAULT`, `CDD_CLEANUP`, `CDD_DEBUG`,
 * `CDD_VALIDATE`, `CDD_MERGE_PATCH`, `CDD_APPLY_MERGE_PATCH`) for a set of
 * struct names.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_names Array of struct names.
 * @param[in] n_structs Number of struct names in the array.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t
write_struct_generic_dispatch_macros(FILE *fp, const char *const *struct_names,
                                     size_t n_structs) {
  size_t i;
  if (!fp || !struct_names || n_structs == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  CHECK_IO(FPRINTF_HOOK(
      fp, "#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L\n"
          "#define CDD_REFLECT(ptr, out) _Generic((ptr), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    if (struct_names[i][0] == '\0')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    CHECK_IO(FPRINTF_HOOK(fp, "  const struct %s *: %s_reflect, \\\n",
                          struct_names[i], struct_names[i]));
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: %s_reflect%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(out)"));
  }
  CHECK_IO(
      FPRINTF_HOOK(fp, "#define CDD_TO_JSON(ptr, out) _Generic((ptr), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  const struct %s *: %s_to_json, \\\n",
                          struct_names[i], struct_names[i]));
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: %s_to_json%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(ptr, out)"));
  }
  CHECK_IO(
      FPRINTF_HOOK(fp, "#define CDD_FROM_JSON(str, out) _Generic((out), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s **: %s_from_json%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(str, out)"));
  }
  CHECK_IO(FPRINTF_HOOK(
      fp, "#define CDD_FROM_CLI(argc, argv, out) _Generic((out), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s **: %s_from_cli%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(argc, argv, out)"));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "#define CDD_TO_CLI_ARGV(ptr, out_argc, out_argv) "
                            "_Generic((ptr), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  const struct %s *: %s_to_cli_argv, \\\n",
                          struct_names[i], struct_names[i]));
    CHECK_IO(
        FPRINTF_HOOK(fp, "  struct %s *: %s_to_cli_argv%s \\\n",
                     struct_names[i], struct_names[i],
                     (i + 1 < n_structs) ? "," : ")(ptr, out_argc, out_argv)"));
  }
  CHECK_IO(
      FPRINTF_HOOK(fp, "#define CDD_TO_FORM(ptr, out) _Generic((ptr), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  const struct %s *: %s_to_form_urlencoded, \\\n",
                          struct_names[i], struct_names[i]));
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: %s_to_form_urlencoded%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(ptr, out)"));
  }
  CHECK_IO(
      FPRINTF_HOOK(fp, "#define CDD_FROM_FORM(str, out) _Generic((out), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s **: %s_from_form_urlencoded%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(str, out)"));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "#define CDD_EQ(a, b, out_eq) _Generic((a), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  const struct %s *: %s_eq, \\\n",
                          struct_names[i], struct_names[i]));
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: %s_eq%s \\\n", struct_names[i],
                          struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(a, b, out_eq)"));
  }
  CHECK_IO(
      FPRINTF_HOOK(fp, "#define CDD_DEEPCOPY(src, dest) _Generic((src), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  const struct %s *: %s_deepcopy, \\\n",
                          struct_names[i], struct_names[i]));
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: %s_deepcopy%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(src, dest)"));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "#define CDD_DEFAULT(out) _Generic((out), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s **: %s_default%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(out)"));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "#define CDD_CLEANUP(ptr) _Generic((ptr), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: %s_cleanup%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(ptr)"));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "#define CDD_DEBUG(ptr, fp) _Generic((ptr), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  const struct %s *: %s_debug, \\\n",
                          struct_names[i], struct_names[i]));
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: %s_debug%s \\\n", struct_names[i],
                          struct_names[i],
                          (i + 1 < n_structs) ? "," : ")(ptr, fp)"));
  }
  CHECK_IO(FPRINTF_HOOK(fp,
                        "#define CDD_VALIDATE(ptr) cdd_reflect_validate((ptr), "
                        "_Generic((ptr), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  const struct %s *: &%s_type_info_table, \\\n",
                          struct_names[i], struct_names[i]));
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: &%s_type_info_table%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : "))"));
  }
  CHECK_IO(FPRINTF_HOOK(
      fp, "#define CDD_MERGE_PATCH(src, dst, out) "
          "cdd_reflect_merge_patch((src), (dst), _Generic((src), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  const struct %s *: &%s_type_info_table, \\\n",
                          struct_names[i], struct_names[i]));
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: &%s_type_info_table%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : "), (out))"));
  }
  CHECK_IO(FPRINTF_HOOK(
      fp, "#define CDD_APPLY_MERGE_PATCH(target, patch) "
          "cdd_reflect_apply_merge_patch((target), _Generic((target), \\\n"));
  for (i = 0; i < n_structs; ++i) {
    CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *: &%s_type_info_table%s \\\n",
                          struct_names[i], struct_names[i],
                          (i + 1 < n_structs) ? "," : "), (patch))"));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "#endif /* __STDC_VERSION__ >= 201112L */\n"));
  return CDD_C_SUCCESS;
}
