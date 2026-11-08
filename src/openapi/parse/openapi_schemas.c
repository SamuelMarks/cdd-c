/**
 * @file openapi_schemas.c
 * @brief Schema types, constraints, and composition analysis.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses a schema `type` field (string or union array) and detects
 * `null`.
 *
 * @param[in] schema JSON object representing the schema.
 * @param[out] out_nullable Set to 1 if `"null"` or `nullable: true` is present.
 * @param[out] _out_val Primary non-null type string (borrowed from JSON).
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t parse_schema_type(const JSON_Object *schema, int *out_nullable,
                                const char **_out_val) {
  const char *type = NULL;
  JSON_Array *types = NULL;
  size_t i, count = 0;
  const char *chosen = NULL;

  if (out_nullable)
    *out_nullable = 0;
  if (!schema) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  cdd_json_object_get_string(schema, "type", &type);
  if (type) {
    *_out_val = type;
    return CDD_C_SUCCESS;
  }

  cdd_json_object_get_array(schema, "type", &types);
  if (!types) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  cdd_json_array_get_count(types, &count);
  for (i = 0; i < count; ++i) {
    const char *t = NULL;
    cdd_json_array_get_string(types, i, &t);
    if (!t)
      continue;
    if (strcmp(t, "null") == 0) {
      if (out_nullable)
        *out_nullable = 1;
      continue;
    }
    if (!chosen)
      chosen = t;
  }

  if (!chosen && out_nullable && *out_nullable) {
    *_out_val = "null";
    return CDD_C_SUCCESS;
  }

  {
    *_out_val = chosen;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses numeric, string, and array validation constraints from a schema
 * object into a target structure.
 *
 * @param[in] schema JSON object representing the schema.
 * @param[in,out] target Target pointers receiving parsed constraint values.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
cdd_c_error_t parse_schema_constraints(const JSON_Object *schema,
                                       struct SchemaConstraintTarget *target) {
  int has_val = 0;
  int b_val = 0;
  double num_val = 0.0;
  if (!schema || !target)
    return CDD_C_SUCCESS;

  if (target->example && target->example_set) {
    cdd_c_error_t rc = parse_any_field(schema, "example", target->example,
                                       target->example_set);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (target->has_min && target->min_val) {
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "minimum", JSONNumber, &has_val);
    if (has_val) {
      *target->has_min = 1;
      cdd_json_object_get_number(schema, "minimum", target->min_val);
    }
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "exclusiveMinimum", JSONNumber,
                                      &has_val);
    if (has_val) {
      *target->has_min = 1;
      cdd_json_object_get_number(schema, "exclusiveMinimum", target->min_val);
      if (target->exclusive_min)
        *target->exclusive_min = 1;
    } else {
      has_val = 0;
      b_val = 0;
      cdd_json_object_has_value_of_type(schema, "exclusiveMinimum", JSONBoolean,
                                        &has_val);
      if (has_val)
        cdd_json_object_get_boolean(schema, "exclusiveMinimum", &b_val);
      if (has_val && b_val == 1) {
        if (target->exclusive_min)
          *target->exclusive_min = 1;
      }
    }
  }

  if (target->has_max && target->max_val) {
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "maximum", JSONNumber, &has_val);
    if (has_val) {
      *target->has_max = 1;
      cdd_json_object_get_number(schema, "maximum", target->max_val);
    }
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "exclusiveMaximum", JSONNumber,
                                      &has_val);
    if (has_val) {
      *target->has_max = 1;
      cdd_json_object_get_number(schema, "exclusiveMaximum", target->max_val);
      if (target->exclusive_max)
        *target->exclusive_max = 1;
    } else {
      has_val = 0;
      b_val = 0;
      cdd_json_object_has_value_of_type(schema, "exclusiveMaximum", JSONBoolean,
                                        &has_val);
      if (has_val)
        cdd_json_object_get_boolean(schema, "exclusiveMaximum", &b_val);
      if (has_val && b_val == 1) {
        if (target->exclusive_max)
          *target->exclusive_max = 1;
      }
    }
  }

  if (target->has_min_len && target->min_len) {
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "minLength", JSONNumber,
                                      &has_val);
    if (has_val) {
      *target->has_min_len = 1;
      num_val = 0.0;
      cdd_json_object_get_number(schema, "minLength", &num_val);
      *target->min_len = (size_t)num_val;
    }
  }

  if (target->has_max_len && target->max_len) {
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "maxLength", JSONNumber,
                                      &has_val);
    if (has_val) {
      *target->has_max_len = 1;
      num_val = 0.0;
      cdd_json_object_get_number(schema, "maxLength", &num_val);
      *target->max_len = (size_t)num_val;
    }
  }

  if (target->pattern) {
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "pattern", JSONString, &has_val);
    if (has_val) {
      const char *pattern = NULL;
      cdd_json_object_get_string(schema, "pattern", &pattern);
      c_cdd_strdup(pattern, target->pattern);
      if (!*target->pattern) {
        if (target->example)
          free_any_value(target->example);
        return CDD_C_ERROR_MEMORY;
      }
    }
  }

  if (target->has_min_items && target->min_items) {
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "minItems", JSONNumber, &has_val);
    if (has_val) {
      *target->has_min_items = 1;
      num_val = 0.0;
      cdd_json_object_get_number(schema, "minItems", &num_val);
      *target->min_items = (size_t)num_val;
    }
  }

  if (target->has_max_items && target->max_items) {
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "maxItems", JSONNumber, &has_val);
    if (has_val) {
      *target->has_max_items = 1;
      num_val = 0.0;
      cdd_json_object_get_number(schema, "maxItems", &num_val);
      *target->max_items = (size_t)num_val;
    }
  }

  if (target->unique_items) {
    has_val = 0;
    cdd_json_object_has_value_of_type(schema, "uniqueItems", JSONBoolean,
                                      &has_val);
    if (has_val) {
      cdd_json_object_get_boolean(schema, "uniqueItems", target->unique_items);
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a JSON array of strings (e.g. `enum` or `type` array).
 *
 * @param[in] arr JSON array of strings.
 * @param[out] out Output array of allocated strings.
 * @param[out] out_count Output number of strings.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t parse_string_enum_array(const JSON_Array *arr, char ***out,
                                      size_t *out_count) {
  size_t i, count = 0;

  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return CDD_C_SUCCESS;

  cdd_json_array_get_count(arr, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < count; ++i) {
    const char *s = NULL;
    cdd_json_array_get_string(arr, i, &s);
    if (!s)
      return CDD_C_SUCCESS;
  }

  *out = (char **)C_CDD_CALLOC(count, sizeof(char *));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *val = NULL;
    cdd_json_array_get_string(arr, i, &val);
    c_cdd_strdup(val, &(*out)[i]);
    if (!(*out)[i]) {
      size_t j;
      for (j = 0; j < i; ++j)
        C_CDD_FREE((*out)[j]);
      C_CDD_FREE(*out);
      *out = NULL;
      *out_count = 0;
      return CDD_C_ERROR_MEMORY;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Deep-copies an array of strings.
 *
 * @param[out] dst Output allocated string array.
 * @param[out] dst_count Output number of strings.
 * @param[in] src Source string array.
 * @param[in] src_count Number of strings in source.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
cdd_c_error_t copy_string_array(char ***dst, size_t *dst_count, char **src,
                                size_t src_count) {
  size_t i;
  if (!dst || !dst_count)
    return CDD_C_SUCCESS;
  *dst = NULL;
  *dst_count = 0;
  if (!src || src_count == 0)
    return CDD_C_SUCCESS;
  *dst = (char **)C_CDD_CALLOC(src_count, sizeof(char *));
  if (!*dst)
    return CDD_C_ERROR_MEMORY;
  *dst_count = src_count;
  for (i = 0; i < src_count; ++i) {
    if (!src[i])
      continue;
    c_cdd_strdup(src[i], &(*dst)[i]);
    if (!(*dst)[i]) {
      size_t j;
      for (j = 0; j < i; ++j)
        C_CDD_FREE((*dst)[j]);
      C_CDD_FREE(*dst);
      *dst = NULL;
      *dst_count = 0;
      return CDD_C_ERROR_MEMORY;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a JSON schema object represents a pure string `enum`.
 *
 * @param[in] schema_obj JSON object representing the schema.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if string enum only, CDD_C_SUCCESS (0)
 * otherwise.
 */
cdd_c_error_t schema_is_string_enum_only(const JSON_Object *schema_obj) {
  JSON_Array *enum_arr = NULL;
  size_t i, count = 0;
  const char *type = NULL;

  if (!schema_obj)
    return CDD_C_SUCCESS;

  cdd_json_object_get_array(schema_obj, "enum", &enum_arr);
  if (!enum_arr)
    return CDD_C_SUCCESS;

  cdd_json_array_get_count(enum_arr, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  cdd_json_object_get_string(schema_obj, "type", &type);
  if (type && strcmp(type, "string") != 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < count; ++i) {
    const char *s = NULL;
    cdd_json_array_get_string(enum_arr, i, &s);
    if (!s)
      return CDD_C_SUCCESS;
  }

  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Checks if a JSON schema value/object is compatible with C struct
 * generation.
 *
 * @param[in] schema_val JSON value of the schema.
 * @param[in] schema_obj JSON object of the schema.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if struct-compatible, CDD_C_SUCCESS
 * (0) otherwise.
 */
cdd_c_error_t schema_is_struct_compatible(const JSON_Value *schema_val,
                                          const JSON_Object *schema_obj) {
  const char *type = NULL;
  JSON_Value_Type val_type = JSONError;
  JSON_Object *props_obj = NULL;
  JSON_Array *all_of = NULL, *any_of = NULL, *one_of = NULL;
  if (!schema_val || !schema_obj)
    return CDD_C_SUCCESS;
  cdd_json_value_get_type(schema_val, &val_type);
  if (val_type == JSONBoolean)
    return CDD_C_SUCCESS;
  if (schema_is_string_enum_only(schema_obj))
    return CDD_C_ERROR_UNKNOWN;
  cdd_json_object_get_string(schema_obj, "type", &type);
  if (type)
    return strcmp(type, "object") == 0;
  cdd_json_object_get_object(schema_obj, "properties", &props_obj);
  if (props_obj)
    return CDD_C_ERROR_UNKNOWN;
  cdd_json_object_get_array(schema_obj, "allOf", &all_of);
  cdd_json_object_get_array(schema_obj, "anyOf", &any_of);
  cdd_json_object_get_array(schema_obj, "oneOf", &one_of);
  if (all_of || any_of || one_of)
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a JSON schema object uses `allOf`, `anyOf`, or `oneOf`
 * composition.
 *
 * @param[in] schema_obj JSON object representing the schema.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if composition is present,
 * CDD_C_SUCCESS (0) otherwise.
 */
cdd_c_error_t schema_has_composition(const JSON_Object *schema_obj) {
  JSON_Array *all_of = NULL, *any_of = NULL, *one_of = NULL;
  if (!schema_obj)
    return CDD_C_SUCCESS;
  cdd_json_object_get_array(schema_obj, "allOf", &all_of);
  cdd_json_object_get_array(schema_obj, "anyOf", &any_of);
  cdd_json_object_get_array(schema_obj, "oneOf", &one_of);
  return (all_of || any_of || one_of) ? CDD_C_ERROR_UNKNOWN : CDD_C_SUCCESS;
}

/**
 * @brief Parses a JSON array of schemas into an array of OpenAPI_SchemaRef.
 *
 * @param[in] arr JSON array of schema objects.
 * @param[out] out Output array of parsed schema references.
 * @param[out] out_count Output number of parsed schema references.
 * @param[in] spec Parent OpenAPI specification context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t parse_schema_array_ref(const JSON_Array *arr,
                                     struct OpenAPI_SchemaRef **out,
                                     size_t *out_count,
                                     const struct OpenAPI_Spec *spec) {
  size_t i, count = 0;
  struct OpenAPI_SchemaRef *schemas;

  if (!arr || !out || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  cdd_json_array_get_count(arr, &count);
  if (count == 0) {
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  schemas = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_SchemaRef));
  if (!schemas)
    return CDD_C_ERROR_MEMORY;
  for (i = 0; i < count; ++i) {
    JSON_Object *elem_obj = NULL;
    cdd_c_error_t rc;
    cdd_json_array_get_object(arr, i, &elem_obj);
    rc = parse_schema_ref(elem_obj, &schemas[i], spec);
    if (rc != CDD_C_SUCCESS) {
      size_t j;
      for (j = 0; j < i; ++j) {
        free_schema_ref_content(&schemas[j]);
      }
      C_CDD_FREE(schemas);
      return rc;
    }
  }
  *out = schemas;
  *out_count = count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Allocates and parses a single heap-allocated OpenAPI_SchemaRef.
 *
 * @param[in] obj JSON object representing the schema.
 * @param[out] out Output pointer to the newly allocated OpenAPI_SchemaRef.
 * @param[in] spec Parent OpenAPI specification context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t parse_schema_ref_ptr(const JSON_Object *obj,
                                   struct OpenAPI_SchemaRef **out,
                                   const struct OpenAPI_Spec *spec) {
  struct OpenAPI_SchemaRef *schema;
  cdd_c_error_t rc;
  if (!obj || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  schema = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
      1, sizeof(struct OpenAPI_SchemaRef));
  if (!schema)
    return CDD_C_ERROR_MEMORY;
  rc = parse_schema_ref(obj, schema, spec);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(schema);
    return rc;
  }
  *out = schema;
  return CDD_C_SUCCESS;
}
