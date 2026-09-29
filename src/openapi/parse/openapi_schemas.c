/**
 * @file openapi_schemas.c
 * @brief Schema types, constraints, and composition analysis.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses schema type from the given input.
 */
cdd_c_error_t parse_schema_type(const JSON_Object *schema, int *out_nullable,
                                char **_out_val) {
  const char *type;
  const JSON_Array *types;
  size_t i, count;
  const char *chosen = NULL;

  if (out_nullable)
    *out_nullable = 0;
  if (!schema) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  type = json_object_get_string(schema, "type");
  if (type) {
    *_out_val = (char *)(size_t)(type);
    return CDD_C_SUCCESS;
  }

  types = json_object_get_array(schema, "type");
  if (!types) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  count = json_array_get_count(types);
  for (i = 0; i < count; ++i) {
    const char *t = json_array_get_string(types, i);
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
    *_out_val = (char *)(size_t) "null";
    return CDD_C_SUCCESS;
  }

  {
    *_out_val = (char *)(size_t)(chosen);
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses schema constraints from the given input.
 */
cdd_c_error_t parse_schema_constraints(const JSON_Object *schema,
                                       struct SchemaConstraintTarget *target) {
  char *_ast_strdup_17 = NULL;
  if (!schema || !target)
    return CDD_C_SUCCESS;

  if (target->example && target->example_set) {
    {
      cdd_c_error_t _rc = parse_any_field(schema, "example", target->example,
                                          target->example_set);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  if (target->has_min && target->min_val) {
    if (json_object_has_value_of_type(schema, "minimum", JSONNumber)) {
      *target->has_min = 1;
      *target->min_val = json_object_get_number(schema, "minimum");
    }
    if (json_object_has_value_of_type(schema, "exclusiveMinimum", JSONNumber)) {
      *target->has_min = 1;
      *target->min_val = json_object_get_number(schema, "exclusiveMinimum");
      if (target->exclusive_min)
        *target->exclusive_min = 1;
    } else if (json_object_has_value_of_type(schema, "exclusiveMinimum",
                                             JSONBoolean) &&
               json_object_get_boolean(schema, "exclusiveMinimum")) {
      if (target->exclusive_min)
        *target->exclusive_min = 1;
    }
  }

  if (target->has_max && target->max_val) {
    if (json_object_has_value_of_type(schema, "maximum", JSONNumber)) {
      *target->has_max = 1;
      *target->max_val = json_object_get_number(schema, "maximum");
    }
    if (json_object_has_value_of_type(schema, "exclusiveMaximum", JSONNumber)) {
      *target->has_max = 1;
      *target->max_val = json_object_get_number(schema, "exclusiveMaximum");
      if (target->exclusive_max)
        *target->exclusive_max = 1;
    } else if (json_object_has_value_of_type(schema, "exclusiveMaximum",
                                             JSONBoolean) &&
               json_object_get_boolean(schema, "exclusiveMaximum")) {
      if (target->exclusive_max)
        *target->exclusive_max = 1;
    }
  }

  if (target->has_min_len && target->min_len) {
    if (json_object_has_value_of_type(schema, "minLength", JSONNumber)) {
      *target->has_min_len = 1;
      *target->min_len = (size_t)json_object_get_number(schema, "minLength");
    }
  }

  if (target->has_max_len && target->max_len) {
    if (json_object_has_value_of_type(schema, "maxLength", JSONNumber)) {
      *target->has_max_len = 1;
      *target->max_len = (size_t)json_object_get_number(schema, "maxLength");
    }
  }

  if (target->pattern) {
    if (json_object_has_value_of_type(schema, "pattern", JSONString)) {
      const char *pattern = json_object_get_string(schema, "pattern");
      *target->pattern =
          (c_cdd_strdup(pattern, &_ast_strdup_17), _ast_strdup_17);
      if (!*target->pattern)
        return CDD_C_ERROR_MEMORY;
    }
  }

  if (target->has_min_items && target->min_items) {
    if (json_object_has_value_of_type(schema, "minItems", JSONNumber)) {
      *target->has_min_items = 1;
      *target->min_items = (size_t)json_object_get_number(schema, "minItems");
    }
  }

  if (target->has_max_items && target->max_items) {
    if (json_object_has_value_of_type(schema, "maxItems", JSONNumber)) {
      *target->has_max_items = 1;
      *target->max_items = (size_t)json_object_get_number(schema, "maxItems");
    }
  }

  if (target->unique_items) {
    if (json_object_has_value_of_type(schema, "uniqueItems", JSONBoolean)) {
      *target->unique_items = json_object_get_boolean(schema, "uniqueItems");
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses string enum array from the given input.
 */
cdd_c_error_t parse_string_enum_array(const JSON_Array *arr, char ***out,
                                      size_t *out_count) {
  char *_ast_strdup_18 = NULL;
  size_t i, count;

  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < count; ++i) {
    if (!json_array_get_string(arr, i))
      return CDD_C_SUCCESS;
  }

  *out = (char **)C_CDD_CALLOC(count, sizeof(char *));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *val = json_array_get_string(arr, i);
    (*out)[i] = (c_cdd_strdup(val, &_ast_strdup_18), _ast_strdup_18);
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
 * @brief Creates a deep copy of string array.
 */
cdd_c_error_t copy_string_array(char ***dst, size_t *dst_count, char **src,
                                size_t src_count) {
  char *_ast_strdup_19 = NULL;
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
    (*dst)[i] = (c_cdd_strdup(src[i], &_ast_strdup_19), _ast_strdup_19);
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
 * @brief Executes the schema is string enum only operation.
 */
cdd_c_error_t schema_is_string_enum_only(const JSON_Object *schema_obj) {
  const JSON_Array *enum_arr;
  size_t i, count;
  const char *type;

  if (!schema_obj)
    return CDD_C_SUCCESS;

  enum_arr = json_object_get_array(schema_obj, "enum");
  if (!enum_arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(enum_arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  type = json_object_get_string(schema_obj, "type");
  if (type && strcmp(type, "string") != 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < count; ++i) {
    if (!json_array_get_string(enum_arr, i))
      return CDD_C_SUCCESS;
  }

  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Executes the schema is struct compatible operation.
 */
cdd_c_error_t schema_is_struct_compatible(const JSON_Value *schema_val,
                                          const JSON_Object *schema_obj) {
  const char *type;
  if (!schema_val || !schema_obj)
    return CDD_C_SUCCESS;
  if (json_value_get_type(schema_val) == JSONBoolean)
    return CDD_C_SUCCESS;
  if (schema_is_string_enum_only(schema_obj))
    return CDD_C_ERROR_UNKNOWN;
  type = json_object_get_string(schema_obj, "type");
  if (type)
    return strcmp(type, "object") == 0;
  if (json_object_get_object(schema_obj, "properties"))
    return CDD_C_ERROR_UNKNOWN;
  if (json_object_get_array(schema_obj, "allOf") ||
      json_object_get_array(schema_obj, "anyOf") ||
      json_object_get_array(schema_obj, "oneOf"))
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the schema has composition operation.
 */
cdd_c_error_t schema_has_composition(const JSON_Object *schema_obj) {
  if (!schema_obj)
    return CDD_C_SUCCESS;
  return json_object_get_array(schema_obj, "allOf") ||
         json_object_get_array(schema_obj, "anyOf") ||
         json_object_get_array(schema_obj, "oneOf");
}

/**
 * @brief Parses schema array ref from the given input.
 */
cdd_c_error_t parse_schema_array_ref(const JSON_Array *arr,
                                     struct OpenAPI_SchemaRef **out,
                                     size_t *out_count,
                                     const struct OpenAPI_Spec *spec) {
  size_t i, count;
  struct OpenAPI_SchemaRef *schemas;

  if (!arr || !out || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  count = json_array_get_count(arr);
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
    cdd_c_error_t rc =
        parse_schema_ref(json_array_get_object(arr, i), &schemas[i], spec);
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
 * @brief Parses schema ref ptr from the given input.
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
