/**
 * @file code2schema_defaults.c
 * @brief Default value parsing and quote stripping.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Strips enclosing quotes from a string.
 *
 * @param[in] in Input string possibly enclosed in double quotes.
 * @param[out] buf Scratch buffer used when quotes are stripped.
 * @param[in] bufsz Size of `buf` in bytes.
 * @param[out] _out_val Output pointer receiving the unquoted string.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t c2s_strip_quotes(const char *in, char *buf, size_t bufsz,
                               const char **_out_val) {
  size_t len;
  if (!in || !buf || bufsz == 0) {
    *_out_val = in;
    return CDD_C_SUCCESS;
  }
  len = strlen(in);
  if (len >= 2 && in[0] == '"' && in[len - 1] == '"') {
    size_t inner = len - 2;
    if (inner >= bufsz)
      inner = bufsz - 1;
    memcpy(buf, in + 1, inner);
    buf[inner] = '\0';
    {
      *_out_val = buf;
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = in;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses a boolean default value from a string.
 *
 * @param[in] in Input default literal string (`"1"`, `"0"`, `"true"`,
 * `"false"`).
 * @param[out] out Output pointer receiving 1 for true or 0 for false.
 * @param[out] out_has_val Output pointer receiving 1 if parsed, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_has_val` is NULL.
 */
cdd_c_error_t c2s_parse_bool_default(const char *in, int *out,
                                     int *out_has_val) {
  if (!out_has_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has_val = 0;
  if (!in || !out)
    return CDD_C_SUCCESS;
  if (strcmp(in, "1") == 0 || strcmp(in, "true") == 0) {
    *out = 1;
    *out_has_val = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "0") == 0 || strcmp(in, "false") == 0) {
    *out = 0;
    *out_has_val = 1;
    return CDD_C_SUCCESS;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a numeric default value from a string.
 *
 * @param[in] in Input numeric literal string.
 * @param[out] out Output pointer receiving the parsed numeric value as
 * `double`.
 * @param[out] out_has_val Output pointer receiving 1 if parsed, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_has_val` is NULL.
 */
cdd_c_error_t c2s_parse_number_default(const char *in, double *out,
                                       int *out_has_val) {
  struct NumericValue nv;
  if (!out_has_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has_val = 0;
  if (!in || !out)
    return CDD_C_SUCCESS;
  if (parse_numeric_literal(in, &nv) == 0) {
    if (nv.kind == NUMERIC_INTEGER) {
      *out = (double)nv.data.integer.value;
      *out_has_val = 1;
      return CDD_C_SUCCESS;
    } else {
      *out = nv.data.floating.value;
      *out_has_val = 1;
      return CDD_C_SUCCESS;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Detects the underlying JSON type of a union schema object.
 *
 * @param[in] schema_obj JSON Schema object to inspect.
 * @param[out] _out_val Output pointer receiving the detected union variant JSON
 * type.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `_out_val` is NULL.
 */
cdd_c_error_t c2s_detect_union_json_type(const JSON_Object *schema_obj,
                                         enum UnionVariantJsonType *_out_val) {
  const char *type;
  const JSON_Object *props;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!schema_obj) {
    *_out_val = UNION_JSON_UNKNOWN;
    return CDD_C_SUCCESS;
  }
  {
    int is_enum = 0;
    schema_object_is_string_enum(schema_obj, NULL, &is_enum);
    if (is_enum) {
      *_out_val = UNION_JSON_STRING;
      return CDD_C_SUCCESS;
    }
  }
  type = json_object_get_string(schema_obj, "type");
  if (type) {
    if (strcmp(type, "object") == 0) {
      *_out_val = UNION_JSON_OBJECT;
      return CDD_C_SUCCESS;
    }
    if (strcmp(type, "string") == 0) {
      *_out_val = UNION_JSON_STRING;
      return CDD_C_SUCCESS;
    }
    if (strcmp(type, "integer") == 0) {
      *_out_val = UNION_JSON_INTEGER;
      return CDD_C_SUCCESS;
    }
    if (strcmp(type, "number") == 0) {
      *_out_val = UNION_JSON_NUMBER;
      return CDD_C_SUCCESS;
    }
    if (strcmp(type, "boolean") == 0) {
      *_out_val = UNION_JSON_BOOLEAN;
      return CDD_C_SUCCESS;
    }
    if (strcmp(type, "array") == 0) {
      *_out_val = UNION_JSON_ARRAY;
      return CDD_C_SUCCESS;
    }
    if (strcmp(type, "null") == 0) {
      *_out_val = UNION_JSON_NULL;
      return CDD_C_SUCCESS;
    }
  }
  props = json_object_get_object(schema_obj, "properties");
  if (props) {
    *_out_val = UNION_JSON_OBJECT;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = UNION_JSON_UNKNOWN;
    return CDD_C_SUCCESS;
  }
}
