/**
 * @file code2schema_names.c
 * @brief Identifier sanitization and inline schema naming.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/** @brief Sorted list of C89, C99, C11, C23, and C++ reserved keywords */
static const char *const k_c_reserved_keywords[] = {"_Alignas",
                                                    "_Alignof",
                                                    "_Atomic",
                                                    "_BitInt",
                                                    "_Bool",
                                                    "_Complex",
                                                    "_Decimal128",
                                                    "_Decimal32",
                                                    "_Decimal64",
                                                    "_Generic",
                                                    "_Imaginary",
                                                    "_Noreturn",
                                                    "_Static_assert",
                                                    "_Thread_local",
                                                    "alignas",
                                                    "alignof",
                                                    "and",
                                                    "and_eq",
                                                    "asm",
                                                    "atomic",
                                                    "auto",
                                                    "bitand",
                                                    "bitor",
                                                    "bool",
                                                    "break",
                                                    "case",
                                                    "catch",
                                                    "char",
                                                    "char16_t",
                                                    "char32_t",
                                                    "char8_t",
                                                    "class",
                                                    "co_await",
                                                    "co_return",
                                                    "co_yield",
                                                    "compl",
                                                    "complex",
                                                    "concept",
                                                    "const",
                                                    "const_cast",
                                                    "consteval",
                                                    "constexpr",
                                                    "constinit",
                                                    "continue",
                                                    "decltype",
                                                    "default",
                                                    "delete",
                                                    "do",
                                                    "double",
                                                    "dynamic_cast",
                                                    "else",
                                                    "enum",
                                                    "explicit",
                                                    "export",
                                                    "extern",
                                                    "false",
                                                    "float",
                                                    "for",
                                                    "friend",
                                                    "goto",
                                                    "if",
                                                    "imaginary",
                                                    "inline",
                                                    "int",
                                                    "long",
                                                    "mutable",
                                                    "namespace",
                                                    "new",
                                                    "noexcept",
                                                    "not",
                                                    "not_eq",
                                                    "nullptr",
                                                    "operator",
                                                    "or",
                                                    "or_eq",
                                                    "private",
                                                    "protected",
                                                    "public",
                                                    "register",
                                                    "reinterpret_cast",
                                                    "requires",
                                                    "restrict",
                                                    "return",
                                                    "short",
                                                    "signed",
                                                    "sizeof",
                                                    "static",
                                                    "static_assert",
                                                    "static_cast",
                                                    "struct",
                                                    "switch",
                                                    "template",
                                                    "this",
                                                    "thread_local",
                                                    "throw",
                                                    "true",
                                                    "try",
                                                    "typedef",
                                                    "typeid",
                                                    "typename",
                                                    "typeof",
                                                    "typeof_unqual",
                                                    "union",
                                                    "unsigned",
                                                    "using",
                                                    "virtual",
                                                    "void",
                                                    "volatile",
                                                    "wchar_t",
                                                    "while",
                                                    "xor",
                                                    "xor_eq"};

/**
 * @brief Checks if an identifier matches a C/C++ reserved keyword.
 *
 * @param[in] ident Candidate identifier string.
 * @param[out] out_is_reserved Set to 1 if reserved keyword, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_is_reserved_keyword(const char *ident,
                                             int *out_is_reserved) {
  size_t k;
  const size_t n_kw =
      sizeof(k_c_reserved_keywords) / sizeof(k_c_reserved_keywords[0]);
  *out_is_reserved = 0;
  for (k = 0; k < n_kw; ++k) {
    if (strcmp(ident, k_c_reserved_keywords[k]) == 0) {
      *out_is_reserved = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Checks if a string requires sanitization to be a valid, unreserved C
 * identifier.
 *
 * @param[in] in Candidate identifier string.
 * @param[out] out_needs Set to 1 if sanitization is required, 0 if `in` is
 * already a clean C identifier.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_needs_identifier_sanitization(const char *in,
                                                int *out_needs) {
  size_t i;
  if (!out_needs)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!in || !in[0]) {
    *out_needs = 1;
    return CDD_C_SUCCESS;
  }
  if (in[0] >= '0' && in[0] <= '9') {
    *out_needs = 1;
    return CDD_C_SUCCESS;
  }
  if (in[0] == '_' && (in[1] == '_' || (in[1] >= 'A' && in[1] <= 'Z'))) {
    *out_needs = 1;
    return CDD_C_SUCCESS;
  }
  for (i = 0; in[i] != '\0'; ++i) {
    unsigned char uc = (unsigned char)in[i];
    if (!((uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z') ||
          (uc >= '0' && uc <= '9') || uc == '_')) {
      *out_needs = 1;
      return CDD_C_SUCCESS;
    }
    if (uc == '_' && in[i + 1] == '_') {
      *out_needs = 1;
      return CDD_C_SUCCESS;
    }
  }
  if (i > 59) {
    *out_needs = 1;
    return CDD_C_SUCCESS;
  }
  return c2s_is_reserved_keyword(in, out_needs);
}

/*
 * @brief Sanitizes a string to be used as a valid C identifier.
 *
 * @param[in] in Input string to sanitize.
 * @param[out] _out_val Output pointer receiving the newly allocated C
 * identifier string.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t sanitize_identifier(const char *in, char **_out_val) {
  size_t i, len, pos = 0, cap;
  char *out;
  int has_alnum = 0;
  int only_ws = 1;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!in || !*in) {
    {
      cdd_c_error_t rc_c2s = c_cdd_strdup("Variant", _out_val);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    return CDD_C_SUCCESS;
  }
  len = strlen(in);
  for (i = 0; i < len; ++i) {
    unsigned char uc = (unsigned char)in[i];
    if (!isspace(uc))
      only_ws = 0;
    if ((uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z') ||
        (uc >= '0' && uc <= '9')) {
      has_alnum = 1;
    }
  }
  if (only_ws) {
    return c_cdd_strdup("field_", _out_val);
  }
  cap = len * 5 + 32;
  out = (char *)C_CDD_CALLOC(cap, sizeof(char));
  if (!out) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  for (i = 0; i < len; ++i) {
    const unsigned char uc = (unsigned char)in[i];
    if ((uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z') ||
        (uc >= '0' && uc <= '9')) {
      out[pos++] = (char)uc;
    } else if (uc == '_') {
      if (pos == 0 || out[pos - 1] != '_')
        out[pos++] = '_';
    } else if (uc >= 0x80 || !has_alnum) {
      static const char hex_digits[] = "0123456789abcdef";
      out[pos++] = '_';
      out[pos++] = 'x';
      out[pos++] = hex_digits[(uc >> 4) & 0x0F];
      out[pos++] = hex_digits[uc & 0x0F];
    } else {
      if (pos == 0 || out[pos - 1] != '_')
        out[pos++] = '_';
    }
  }
  out[pos] = '\0';

  /* Prepend '_' if first character is a digit */
  if (isdigit((unsigned char)out[0])) {
    memmove(out + 1, out, pos + 1);
    out[0] = '_';
    pos++;
  }

  /* Avoid ISO C90 7.1.3 reserved leading _[A-Z] */
  if (out[0] == '_' && isupper((unsigned char)out[1])) {
    memmove(out + 1, out, pos + 1);
    out[0] = 'f';
    pos++;
  }

  /* Escape C89/C99/C11/C23/C++ reserved keywords */
  {
    int is_res = 0;
    c2s_is_reserved_keyword(out, &is_res);
    if (is_res) {
      out[pos++] = '_';
      out[pos] = '\0';
    }
  }

  /* Truncate safely to fit within sizeof(field->name) - 4 (59 chars) */
  if (pos > 59) {
    out[59] = '\0';
  }

  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/*
 * @brief Generates a unique variant name for a union type.
 *
 * @param[in] dest Existing StructFields container to check for collisions.
 * @param[in] base Base candidate variant name.
 * @param[in] index Variant index for numeric suffix disambiguation.
 * @param[out] _out_val Output pointer receiving the newly allocated unique
 * variant name.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t make_unique_variant_name(const struct StructFields *dest,
                                       const char *base, size_t index,
                                       char **_out_val) {
  char buf[128];
  char *sanitized;
  char *out;
  if (!dest) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  {
    struct StructField *tmp4 = NULL;
    cdd_c_error_t rc_c2s = sanitize_identifier(base, &sanitized);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;

    struct_fields_get(dest, sanitized, &tmp4);
    if (!tmp4) {
      *_out_val = sanitized;
      return CDD_C_SUCCESS;
    }
  }
  CDD_SNPRINTF(buf, sizeof(buf), "%s_%lu", sanitized,
               (unsigned long)(index + 1));
  C_CDD_FREE(sanitized);
  {
    cdd_c_error_t rc_c2s = c_cdd_strdup(buf, &out);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }
  {
    struct StructField *tmp5 = NULL;
    struct_fields_get(dest, out, &tmp5);
    if (!tmp5) {
      *_out_val = out;
      return CDD_C_SUCCESS;
    }
  }
  C_CDD_FREE(out);
  CDD_SNPRINTF(buf, sizeof(buf), "Variant_%lu", (unsigned long)(index + 1));
  {
    {
      cdd_c_error_t rc_c2s = c_cdd_strdup(buf, _out_val);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    return CDD_C_SUCCESS;
  }
}

/*
 * @brief Generates a schema name for an inline anonymous struct/union.
 *
 * @param[in] schema_name Enclosing parent schema name.
 * @param[in] variant_name Union variant or field name.
 * @param[in] suffix Optional additional suffix (e.g., `"Item"`).
 * @param[out] _out_val Output pointer receiving the newly allocated schema
 * name.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t make_inline_schema_name(const char *schema_name,
                                      const char *variant_name,
                                      const char *suffix, char **_out_val) {
  char buf[256];
  const char *base_schema =
      (schema_name && *schema_name) ? schema_name : "Union";
  const char *base_variant =
      (variant_name && *variant_name) ? variant_name : "Variant";
  if (suffix && *suffix)
    CDD_SNPRINTF(buf, sizeof(buf), "%s_%s_%s", base_schema, base_variant,
                 suffix);
  else
    CDD_SNPRINTF(buf, sizeof(buf), "%s_%s", base_schema, base_variant);
  {
    {
      cdd_c_error_t rc_c2s = sanitize_identifier(buf, _out_val);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    return CDD_C_SUCCESS;
  }
}

/*
 * @brief Registers an inline schema within the root OpenAPI components.
 *
 * @param[in,out] root Mutable root `components.schemas` JSON_Object.
 * @param[in] schema_name Enclosing parent schema name.
 * @param[in] variant_name Union variant or field name.
 * @param[in] suffix Optional additional suffix.
 * @param[in] schema_val JSON_Value of the inline schema to register.
 * @param[out] out_name Output pointer receiving the registered schema name.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t
register_inline_schema_c2s(JSON_Object *root, const char *schema_name,
                           const char *variant_name, const char *suffix,
                           const JSON_Value *schema_val, char **out_name) {
  char *name;
  int has_val = 0;

  if (!root || !schema_val || !out_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  name = NULL;
  {
    cdd_c_error_t rc_c2s =
        make_inline_schema_name(schema_name, variant_name, suffix, &name);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }

  cdd_json_object_has_value(root, name, &has_val);
  if (!has_val) {
    JSON_Value *copy = NULL;
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(schema_val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        C_CDD_FREE(name);
        return rc_c2s;
      }
    }
    if (cdd_json_object_set_value(root, name, copy) != CDD_C_SUCCESS) {
      cdd_json_value_free(copy);
      C_CDD_FREE(name);
      return CDD_C_ERROR_MEMORY;
    }
  }

  *out_name = name;

  return CDD_C_SUCCESS;
}

/*
 * @brief Retrieves the discriminator mapping value for a given schema
 * variant.
 *
 * @param[in] disc_obj Optional `discriminator` JSON_Object.
 * @param[in] schema_name Schema or variant name.
 * @param[in] ref Optional `$ref` string for the variant.
 * @param[out] _out_val Output pointer receiving the newly allocated
 * discriminator value string.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t discriminator_value_for_variant(const JSON_Object *disc_obj,
                                              const char *schema_name,
                                              const char *ref,
                                              char **_out_val) {
  JSON_Object *mapping = NULL;
  size_t i, count = 0;
  const char *ref_name;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!schema_name && !ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  if (ref) {
    cdd_c_error_t rc_c2s = c_cdd_str_after_last(ref, '/', &ref_name);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  } else
    ref_name = NULL;

  if (!disc_obj)
    goto fallback;

  cdd_json_object_get_object(disc_obj, "mapping", &mapping);
  if (!mapping)
    goto fallback;

  cdd_json_object_get_count(mapping, &count);
  for (i = 0; i < count; ++i) {
    const char *key = NULL;
    const char *val = NULL;
    cdd_json_object_get_name(mapping, i, &key);
    cdd_json_object_get_string(mapping, key, &val);
    if (!val)
      continue;
    if (ref && strcmp(val, ref) == 0) {
      {
        cdd_c_error_t rc_c2s = c_cdd_strdup(key, _out_val);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      return CDD_C_SUCCESS;
    }
    if (ref_name && strcmp(val, ref_name) == 0) {
      {
        cdd_c_error_t rc_c2s = c_cdd_strdup(key, _out_val);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      return CDD_C_SUCCESS;
    }
    if (schema_name && strcmp(val, schema_name) == 0) {
      {
        cdd_c_error_t rc_c2s = c_cdd_strdup(key, _out_val);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      return CDD_C_SUCCESS;
    }
  }

fallback:
  if (schema_name) {
    cdd_c_error_t rc_c2s = c_cdd_strdup(schema_name, _out_val);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
    return CDD_C_SUCCESS;
  }
  return c_cdd_strdup(ref_name, _out_val);
}
