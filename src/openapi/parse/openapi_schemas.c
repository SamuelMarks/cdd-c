/**
 * @file openapi_schemas.c
 * @brief Schema types, constraints, and composition analysis.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Frees an `OpenAPI_Any` item.
 *
 * @param[in,out] val Any value to release.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t oa_schema_any_free(struct OpenAPI_Any *val) {
  return free_any_value(val);
}

/**
 * @brief Frees an `OpenAPI_SchemaRef` content item.
 *
 * @param[in,out] sr SchemaRef item to release.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t oa_schema_ref_item_free(struct OpenAPI_SchemaRef *sr) {
  return free_schema_ref_content(sr);
}

/*
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
  cdd_c_error_t rc;

  if (out_nullable)
    *out_nullable = 0;
  if (!schema) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  rc = oa_json_opt_string(schema, "type", &type);
  if (rc != CDD_C_SUCCESS) {
    *_out_val = NULL;
    return rc;
  }
  if (type) {
    *_out_val = type;
    return CDD_C_SUCCESS;
  }

  rc = oa_json_opt_array(schema, "type", &types);
  if (rc != CDD_C_SUCCESS) {
    *_out_val = NULL;
    return rc;
  }
  if (!types) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  rc = cdd_json_array_get_count(types, &count);
  if (rc != CDD_C_SUCCESS) {
    *_out_val = NULL;
    return rc;
  }
  for (i = 0; i < count; ++i) {
    const char *t = NULL;
    rc = oa_json_array_opt_string(types, i, &t);
    if (rc != CDD_C_SUCCESS) {
      *_out_val = NULL;
      return rc;
    }
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

  *_out_val = chosen;
  return CDD_C_SUCCESS;
}

/*
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
  cdd_c_error_t rc;
  if (!schema || !target)
    return CDD_C_SUCCESS;

  if (target->example && target->example_set) {
    rc = parse_any_field(schema, "example", target->example,
                         target->example_set);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (target->has_min && target->min_val) {
    rc = oa_json_opt_number(schema, "minimum", &has_val, &num_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_val) {
      *target->has_min = 1;
      *target->min_val = num_val;
    }
    rc = oa_json_opt_number(schema, "exclusiveMinimum", &has_val, &num_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_val) {
      *target->has_min = 1;
      *target->min_val = num_val;
      if (target->exclusive_min)
        *target->exclusive_min = 1;
    } else {
      rc = oa_json_opt_bool(schema, "exclusiveMinimum", &has_val, &b_val);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      if (has_val && b_val == 1) {
        if (target->exclusive_min)
          *target->exclusive_min = 1;
      }
    }
  }

  if (target->has_max && target->max_val) {
    rc = oa_json_opt_number(schema, "maximum", &has_val, &num_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_val) {
      *target->has_max = 1;
      *target->max_val = num_val;
    }
    rc = oa_json_opt_number(schema, "exclusiveMaximum", &has_val, &num_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_val) {
      *target->has_max = 1;
      *target->max_val = num_val;
      if (target->exclusive_max)
        *target->exclusive_max = 1;
    } else {
      rc = oa_json_opt_bool(schema, "exclusiveMaximum", &has_val, &b_val);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      if (has_val && b_val == 1) {
        if (target->exclusive_max)
          *target->exclusive_max = 1;
      }
    }
  }

  if (target->has_min_len && target->min_len) {
    rc = oa_json_opt_number(schema, "minLength", &has_val, &num_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_val) {
      *target->has_min_len = 1;
      *target->min_len = (size_t)num_val;
    }
  }

  if (target->has_max_len && target->max_len) {
    rc = oa_json_opt_number(schema, "maxLength", &has_val, &num_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_val) {
      *target->has_max_len = 1;
      *target->max_len = (size_t)num_val;
    }
  }

  if (target->pattern) {
    const char *pattern = NULL;
    rc = oa_json_opt_string(schema, "pattern", &pattern);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (pattern) {
      rc = c_cdd_strdup(pattern, target->pattern);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  if (target->has_min_items && target->min_items) {
    rc = oa_json_opt_number(schema, "minItems", &has_val, &num_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_val) {
      *target->has_min_items = 1;
      *target->min_items = (size_t)num_val;
    }
  }

  if (target->has_max_items && target->max_items) {
    rc = oa_json_opt_number(schema, "maxItems", &has_val, &num_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_val) {
      *target->has_max_items = 1;
      *target->max_items = (size_t)num_val;
    }
  }

  if (target->unique_items) {
    rc = oa_json_opt_bool(schema, "uniqueItems", &has_val, &b_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_val) {
      *target->unique_items = b_val;
    }
  }

  return CDD_C_SUCCESS;

cleanup:
  if (target->example) {
    cdd_c_error_t rc_f = oa_schema_any_free(target->example);
    *target->example_set = 0;
    return (cdd_c_error_t)((int)rc | (int)rc_f);
  }
  return rc;
}

/*
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
  cdd_c_error_t rc;

  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return CDD_C_SUCCESS;

  rc = cdd_json_array_get_count(arr, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < count; ++i) {
    const char *s = NULL;
    rc = oa_json_array_opt_string(arr, i, &s);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!s)
      return CDD_C_SUCCESS;
  }

  *out = (char **)C_CDD_CALLOC(count, sizeof(char *));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *val = NULL;
    rc = cdd_json_array_get_string(arr, i, &val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = c_cdd_strdup(val, &(*out)[i]);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  return CDD_C_SUCCESS;

cleanup: {
  size_t j;
  for (j = 0; j < i; ++j)
    C_CDD_FREE((*out)[j]);
  C_CDD_FREE(*out);
  *out = NULL;
  *out_count = 0;
}
  return rc;
}

/*
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
  cdd_c_error_t rc;
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
    rc = c_cdd_strdup(src[i], &(*dst)[i]);
    if (rc != CDD_C_SUCCESS) {
      size_t j;
      for (j = 0; j < i; ++j)
        C_CDD_FREE((*dst)[j]);
      C_CDD_FREE(*dst);
      *dst = NULL;
      *dst_count = 0;
      return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Checks if a JSON schema object represents a pure string `enum`.
 *
 * @param[in] schema_obj JSON object representing the schema.
 * @param[out] out_is_enum Set to 1 if string enum only, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t schema_is_string_enum_only(const JSON_Object *schema_obj,
                                         int *out_is_enum) {
  JSON_Array *enum_arr = NULL;
  size_t i, count = 0;
  const char *type = NULL;
  cdd_c_error_t rc;

  if (!out_is_enum)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_enum = 0;

  if (!schema_obj)
    return CDD_C_SUCCESS;

  rc = oa_json_opt_array(schema_obj, "enum", &enum_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!enum_arr)
    return CDD_C_SUCCESS;

  rc = cdd_json_array_get_count(enum_arr, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_SUCCESS;

  rc = oa_json_opt_string(schema_obj, "type", &type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (type && strcmp(type, "string") != 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < count; ++i) {
    const char *s = NULL;
    rc = oa_json_array_opt_string(enum_arr, i, &s);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!s)
      return CDD_C_SUCCESS;
  }

  *out_is_enum = 1;
  return CDD_C_SUCCESS;
}

/*
 * @brief Checks if a JSON schema value/object is compatible with C struct
 * generation.
 *
 * @param[in] schema_val JSON value of the schema.
 * @param[in] schema_obj JSON object of the schema.
 * @param[out] out_compat Set to 1 if struct-compatible, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t schema_is_struct_compatible(const JSON_Value *schema_val,
                                          const JSON_Object *schema_obj,
                                          int *out_compat) {
  const char *type = NULL;
  JSON_Value_Type val_type = JSONError;
  JSON_Object *props_obj = NULL;
  int has_comp = 0;
  int is_enum = 0;
  cdd_c_error_t rc;

  if (!out_compat)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_compat = 0;

  if (!schema_val || !schema_obj)
    return CDD_C_SUCCESS;
  rc = cdd_json_value_get_type(schema_val, &val_type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (val_type == JSONBoolean)
    return CDD_C_SUCCESS;
  rc = schema_is_string_enum_only(schema_obj, &is_enum);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_enum) {
    *out_compat = 1;
    return CDD_C_SUCCESS;
  }
  rc = oa_json_opt_string(schema_obj, "type", &type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (type) {
    *out_compat = (strcmp(type, "object") == 0) ? 1 : 0;
    return CDD_C_SUCCESS;
  }
  rc = oa_json_opt_object(schema_obj, "properties", &props_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (props_obj) {
    *out_compat = 1;
    return CDD_C_SUCCESS;
  }
  rc = schema_has_composition(schema_obj, &has_comp);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_comp) {
    *out_compat = 1;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Checks if a JSON schema object uses `allOf`, `anyOf`, or `oneOf`
 * composition.
 *
 * @param[in] schema_obj JSON object representing the schema.
 * @param[out] out_has_comp Set to 1 if composition is present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t schema_has_composition(const JSON_Object *schema_obj,
                                     int *out_has_comp) {
  JSON_Array *all_of = NULL, *any_of = NULL, *one_of = NULL;
  cdd_c_error_t rc;
  if (!out_has_comp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has_comp = 0;
  if (!schema_obj)
    return CDD_C_SUCCESS;
  rc = oa_json_opt_array(schema_obj, "allOf", &all_of);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_opt_array(schema_obj, "anyOf", &any_of);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_opt_array(schema_obj, "oneOf", &one_of);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_has_comp = (all_of || any_of || one_of) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/*
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
  cdd_c_error_t rc;

  if (!arr || !out || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_json_array_get_count(arr, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
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
    rc = oa_json_array_opt_object(arr, i, &elem_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = parse_schema_ref(elem_obj, &schemas[i], spec);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  *out = schemas;
  *out_count = count;
  return CDD_C_SUCCESS;

cleanup: {
  size_t j;
  for (j = 0; j < i; ++j) {
    rc = (cdd_c_error_t)((int)rc | (int)oa_schema_ref_item_free(&schemas[j]));
  }
  C_CDD_FREE(schemas);
}
  return rc;
}

/*
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
