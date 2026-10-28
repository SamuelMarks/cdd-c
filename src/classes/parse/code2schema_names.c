/**
 * @file code2schema_names.c
 * @brief Identifier sanitization and inline schema naming.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Sanitizes a string to be used as a valid C identifier.
 *
 * @param[in] in Input string to sanitize.
 * @param[out] _out_val Output pointer receiving the newly allocated C
 * identifier string.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t sanitize_identifier(const char *in, char **_out_val) {
  size_t i, len;
  char *out;
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
  out = (char *)C_CDD_CALLOC(len + 1, sizeof(char));
  if (!out) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  for (i = 0; i < len; ++i) {
    const char c = in[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_') {
      out[i] = c;
    } else {
      out[i] = '_';
    }
  }

  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
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

/**
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

/**
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

  if (!root || !schema_val || !out_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  name = NULL;
  {
    cdd_c_error_t rc_c2s =
        make_inline_schema_name(schema_name, variant_name, suffix, &name);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }

  if (!json_object_has_value(root, name)) {
    JSON_Value *copy = NULL;
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(schema_val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        C_CDD_FREE(name);
        return rc_c2s;
      }
    }
    if (json_object_set_value(root, name, copy) != JSONSuccess) {
      json_value_free(copy);
      C_CDD_FREE(name);
      return CDD_C_ERROR_MEMORY;
    }
  }

  *out_name = name;

  return CDD_C_SUCCESS;
}

/**
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
  const JSON_Object *mapping;
  size_t i, count;
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

  mapping = json_object_get_object(disc_obj, "mapping");
  if (!mapping)
    goto fallback;

  count = json_object_get_count(mapping);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(mapping, i);
    const char *val = json_object_get_string(mapping, key);
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
