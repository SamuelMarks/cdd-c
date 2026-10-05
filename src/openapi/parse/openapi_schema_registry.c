/**
 * @file openapi_schema_registry.c
 * @brief Inline schema registration and naming utilities.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/memory.h"
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Applies schema ref to param.
 */
cdd_c_error_t
apply_schema_ref_to_param(struct OpenAPI_Parameter *out_param,
                          const struct OpenAPI_SchemaRef *schema_ref) {
  cdd_c_error_t rc;
  if (!out_param || !schema_ref)
    return CDD_C_SUCCESS;
  if (!schema_ref->ref_name && !schema_ref->inline_type &&
      !schema_ref->is_array)
    return CDD_C_SUCCESS;

  if (out_param->type) {
    free(out_param->type);
    out_param->type = NULL;
  }
  if (out_param->items_type) {
    free(out_param->items_type);
    out_param->items_type = NULL;
  }

  if (schema_ref->is_array) {
    out_param->is_array = 1;
    rc = c_cdd_strdup("array", &out_param->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (schema_ref->inline_type) {
      rc = c_cdd_strdup(schema_ref->inline_type, &out_param->items_type);
      if (rc != CDD_C_SUCCESS) {
        free(out_param->type);
        out_param->type = NULL;
        return rc;
      }
    }
    if (schema_ref->ref_name) {
      if (out_param->items_type) {
        free(out_param->items_type);
        out_param->items_type = NULL;
      }
      rc = c_cdd_strdup(schema_ref->ref_name, &out_param->items_type);
      if (rc != CDD_C_SUCCESS) {
        free(out_param->type);
        out_param->type = NULL;
        return rc;
      }
    }
    return CDD_C_SUCCESS;
  }

  out_param->is_array = 0;
  if (schema_ref->inline_type) {
    rc = c_cdd_strdup(schema_ref->inline_type, &out_param->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    rc = c_cdd_strdup(schema_ref->ref_name, &out_param->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies schema ref to header.
 */
cdd_c_error_t
apply_schema_ref_to_header(struct OpenAPI_Header *out_hdr,
                           const struct OpenAPI_SchemaRef *schema_ref) {
  cdd_c_error_t rc;
  if (!out_hdr || !schema_ref)
    return CDD_C_SUCCESS;
  if (!schema_ref->ref_name && !schema_ref->inline_type &&
      !schema_ref->is_array)
    return CDD_C_SUCCESS;

  if (out_hdr->type) {
    free(out_hdr->type);
    out_hdr->type = NULL;
  }
  if (out_hdr->items_type) {
    free(out_hdr->items_type);
    out_hdr->items_type = NULL;
  }

  if (schema_ref->is_array) {
    out_hdr->is_array = 1;
    rc = c_cdd_strdup("array", &out_hdr->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (schema_ref->inline_type) {
      rc = c_cdd_strdup(schema_ref->inline_type, &out_hdr->items_type);
      if (rc != CDD_C_SUCCESS) {
        free(out_hdr->type);
        out_hdr->type = NULL;
        return rc;
      }
    }
    if (schema_ref->ref_name) {
      if (out_hdr->items_type) {
        free(out_hdr->items_type);
        out_hdr->items_type = NULL;
      }
      rc = c_cdd_strdup(schema_ref->ref_name, &out_hdr->items_type);
      if (rc != CDD_C_SUCCESS) {
        free(out_hdr->type);
        out_hdr->type = NULL;
        return rc;
      }
    }
    return CDD_C_SUCCESS;
  }

  out_hdr->is_array = 0;
  if (schema_ref->inline_type) {
    rc = c_cdd_strdup(schema_ref->inline_type, &out_hdr->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    rc = c_cdd_strdup(schema_ref->ref_name, &out_hdr->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the schema name in use operation.
 */
cdd_c_error_t schema_name_in_use(const struct OpenAPI_Spec *spec,
                                 const char *name) {
  size_t i;
  if (!spec || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < spec->n_defined_schemas; ++i) {
    if (spec->defined_schema_names && spec->defined_schema_names[i] &&
        strcmp(spec->defined_schema_names[i], name) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  for (i = 0; i < spec->n_raw_schemas; ++i) {
    if (spec->raw_schema_names && spec->raw_schema_names[i] &&
        strcmp(spec->raw_schema_names[i], name) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the sanitize component name operation.
 */
cdd_c_error_t sanitize_component_name(const char *name, char **_out_val) {
  size_t i, len;
  char *out;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!name || !*name) {
    rc = c_cdd_strdup("InlineSchema", _out_val);
    return rc;
  }
  len = strlen(name);
  out = (char *)C_CDD_CALLOC(len + 1, sizeof(char));
  if (!out) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  for (i = 0; i < len; ++i) {
    const char c = name[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-') {
      out[i] = c;
    } else {
      out[i] = '_';
    }
  }
  *_out_val = out;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the make unique schema name operation.
 */
cdd_c_error_t make_unique_schema_name(const struct OpenAPI_Spec *spec,
                                      const char *base, char **_out_val) {
  size_t attempt = 0;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!base) {
    *_out_val = NULL;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (schema_name_in_use(spec, base) == CDD_C_SUCCESS) {
    rc = c_cdd_strdup(base, _out_val);
    return rc;
  }
  for (attempt = 1; attempt < 5; ++attempt) {
    char buf[256];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, sizeof(buf), "%s_%lu", base, (unsigned long)attempt);
#else
    CDD_SNPRINTF(buf, sizeof(buf), "%s_%lu", base, (unsigned long)attempt);
#endif
    if (schema_name_in_use(spec, buf) == CDD_C_SUCCESS) {
      rc = c_cdd_strdup(buf, _out_val);
      return rc;
    }
  }
  *_out_val = NULL;
  return CDD_C_ERROR_NOT_FOUND;
}

/**
 * @brief Executes the schema type array includes operation.
 */
cdd_c_error_t schema_type_array_includes(const JSON_Array *arr,
                                         const char *type) {
  size_t i, count;
  if (!arr || !type)
    return CDD_C_SUCCESS;
  count = json_array_get_count(arr);
  for (i = 0; i < count; ++i) {
    const char *val = json_array_get_string(arr, i);
    if (val && strcmp(val, type) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the schema object is object like operation.
 */
cdd_c_error_t schema_object_is_object_like(const JSON_Object *schema_obj) {
  const char *type;
  const JSON_Array *type_arr;
  if (!schema_obj)
    return CDD_C_SUCCESS;
  type = json_object_get_string(schema_obj, "type");
  if (type && strcmp(type, "object") == 0)
    return CDD_C_ERROR_UNKNOWN;
  type_arr = json_object_get_array(schema_obj, "type");
  if (schema_type_array_includes(type_arr, "object") != CDD_C_SUCCESS)
    return CDD_C_ERROR_UNKNOWN;
  if (json_object_get_object(schema_obj, "properties"))
    return CDD_C_ERROR_UNKNOWN;
  if (json_object_get_array(schema_obj, "allOf") ||
      json_object_get_array(schema_obj, "anyOf") ||
      json_object_get_array(schema_obj, "oneOf"))
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the append defined schema operation.
 */
cdd_c_error_t append_defined_schema(struct OpenAPI_Spec *spec,
                                    char *schema_name,
                                    struct StructFields *schema_fields) {
  size_t i;
  size_t new_count;
  char **new_names = NULL;
  char **new_ids = NULL;
  char **new_anchors = NULL;
  char **new_dyn_anchors = NULL;
  struct StructFields *new_schemas = NULL;

  if (!spec || !schema_name || !schema_fields)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  new_count = spec->n_defined_schemas + 1;
  new_names = (char **)C_CDD_CALLOC(new_count, sizeof(char *));
  new_ids = (char **)C_CDD_CALLOC(new_count, sizeof(char *));
  new_anchors = (char **)C_CDD_CALLOC(new_count, sizeof(char *));
  new_dyn_anchors = (char **)C_CDD_CALLOC(new_count, sizeof(char *));
  new_schemas = (struct StructFields *)C_CDD_CALLOC(
      new_count, sizeof(struct StructFields));
  if (!new_names || !new_ids || !new_anchors || !new_dyn_anchors ||
      !new_schemas) {
    free(new_names);
    free(new_ids);
    free(new_anchors);
    free(new_dyn_anchors);
    free(new_schemas);
    return CDD_C_ERROR_MEMORY;
  }

  if (spec->defined_schema_names) {
    for (i = 0; i < spec->n_defined_schemas; ++i)
      new_names[i] = spec->defined_schema_names[i];
  }
  if (spec->defined_schema_ids) {
    for (i = 0; i < spec->n_defined_schemas; ++i)
      new_ids[i] = spec->defined_schema_ids[i];
  }
  if (spec->defined_schema_anchors) {
    for (i = 0; i < spec->n_defined_schemas; ++i)
      new_anchors[i] = spec->defined_schema_anchors[i];
  }
  if (spec->defined_schema_dynamic_anchors) {
    for (i = 0; i < spec->n_defined_schemas; ++i)
      new_dyn_anchors[i] = spec->defined_schema_dynamic_anchors[i];
  }
  if (spec->defined_schemas) {
    for (i = 0; i < spec->n_defined_schemas; ++i)
      new_schemas[i] = spec->defined_schemas[i];
  }

  new_names[new_count - 1] = schema_name;
  new_ids[new_count - 1] = NULL;
  new_anchors[new_count - 1] = NULL;
  new_dyn_anchors[new_count - 1] = NULL;
  new_schemas[new_count - 1] = *schema_fields;

  free(spec->defined_schema_names);
  free(spec->defined_schema_ids);
  free(spec->defined_schema_anchors);
  free(spec->defined_schema_dynamic_anchors);
  free(spec->defined_schemas);
  spec->defined_schema_names = new_names;
  spec->defined_schema_ids = new_ids;
  spec->defined_schema_anchors = new_anchors;
  spec->defined_schema_dynamic_anchors = new_dyn_anchors;
  spec->defined_schemas = new_schemas;
  spec->n_defined_schemas = new_count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the raw schema name exists operation.
 */
cdd_c_error_t raw_schema_name_exists(const struct OpenAPI_Spec *spec,
                                     const char *name) {
  size_t i;
  if (!spec || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < spec->n_raw_schemas; ++i) {
    if (spec->raw_schema_names && spec->raw_schema_names[i] &&
        strcmp(spec->raw_schema_names[i], name) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the append raw schema operation.
 */
cdd_c_error_t append_raw_schema(struct OpenAPI_Spec *spec, const char *name,
                                const JSON_Value *schema_val) {
  size_t i;
  size_t new_count;
  char **new_names = NULL;
  char **new_json = NULL;
  char *dup_name = NULL;
  char *dup_json = NULL;
  char *raw_json = NULL;
  cdd_c_error_t rc;

  if (!spec || !name || !schema_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (raw_schema_name_exists(spec, name) != CDD_C_SUCCESS)
    return CDD_C_SUCCESS;

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_raw_schema_serialize;
    if (g_cdd_fail_raw_schema_serialize &&
        --g_cdd_fail_raw_schema_serialize == 0)
      raw_json = NULL;
    else
      raw_json = json_serialize_to_string(schema_val);
  }
#else
  raw_json = json_serialize_to_string(schema_val);
#endif
  if (!raw_json)
    return CDD_C_ERROR_MEMORY;
  rc = c_cdd_strdup(raw_json, &dup_json);
  json_free_serialized_string(raw_json);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = c_cdd_strdup(name, &dup_name);
  if (rc != CDD_C_SUCCESS) {
    free(dup_json);
    return rc;
  }

  new_count = spec->n_raw_schemas + 1;
  new_names = (char **)C_CDD_CALLOC(new_count, sizeof(char *));
  new_json = (char **)C_CDD_CALLOC(new_count, sizeof(char *));
  if (!new_names || !new_json) {
    free(new_names);
    free(new_json);
    free(dup_name);
    free(dup_json);
    return CDD_C_ERROR_MEMORY;
  }

  if (spec->raw_schema_names) {
    for (i = 0; i < spec->n_raw_schemas; ++i)
      new_names[i] = spec->raw_schema_names[i];
  }
  if (spec->raw_schema_json) {
    for (i = 0; i < spec->n_raw_schemas; ++i)
      new_json[i] = spec->raw_schema_json[i];
  }
  new_names[new_count - 1] = dup_name;
  new_json[new_count - 1] = dup_json;

  free(spec->raw_schema_names);
  free(spec->raw_schema_json);
  spec->raw_schema_names = new_names;
  spec->raw_schema_json = new_json;
  spec->n_raw_schemas = new_count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the register inline schema operation.
 */
cdd_c_error_t register_inline_schema(struct OpenAPI_Spec *spec,
                                     const char *base_name,
                                     const JSON_Object *schema_obj,
                                     const JSON_Value *schema_val,
                                     char **out_name) {
  struct StructFields tmp;
  char *sanitized = NULL;
  char *unique = NULL;
  cdd_c_error_t rc;

  if (!spec || !schema_obj || !out_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = struct_fields_init(&tmp);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = json_object_to_struct_fields_ex(schema_obj, &tmp, NULL, base_name);
  if (rc != CDD_C_SUCCESS) {
    struct_fields_free(&tmp);
    return rc;
  }

  rc = sanitize_component_name(base_name, &sanitized);
  if (rc != CDD_C_SUCCESS) {
    struct_fields_free(&tmp);
    return rc;
  }

  rc = make_unique_schema_name(spec, sanitized, &unique);
  free(sanitized);
  if (rc != CDD_C_SUCCESS) {
    struct_fields_free(&tmp);
    return rc;
  }

  if (schema_has_composition(schema_obj) && schema_val) {
    cdd_c_error_t raw_rc = append_raw_schema(spec, unique, schema_val);
    if (raw_rc != CDD_C_SUCCESS) {
      struct_fields_free(&tmp);
      free(unique);
      return raw_rc;
    }
  }

  rc = append_defined_schema(spec, unique, &tmp);
  if (rc != CDD_C_SUCCESS) {
    struct_fields_free(&tmp);
    free(unique);
    return rc;
  }

  *out_name = unique;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the assign schema ref name operation.
 */
cdd_c_error_t assign_schema_ref_name(struct OpenAPI_SchemaRef *schema_ref,
                                     const char *name) {
  char *dup;
  cdd_c_error_t rc;

  if (!schema_ref || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_assign_ref_name;
    if (g_cdd_fail_assign_ref_name && --g_cdd_fail_assign_ref_name == 0)
      return CDD_C_ERROR_MEMORY;
  }
#endif
  rc = c_cdd_strdup(name, &dup);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (schema_ref->ref_name)
    free(schema_ref->ref_name);
  schema_ref->ref_name = dup;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the build inline request name operation.
 */
cdd_c_error_t build_inline_request_name(const char *op_id, int is_item,
                                        char **_out_val) {
  const char *op;
  const char *suffix;
  size_t len;
  char *out;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  op = (op_id && *op_id) ? op_id : "unnamed";
  suffix = is_item ? "Request_Item" : "Request";
  len = strlen("Inline_") + strlen(op) + 1 + strlen(suffix) + 1;
  out = (char *)C_CDD_MALLOC(len);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(out, len, "Inline_%s_%s", op, suffix);
#else
  CDD_SNPRINTF(out, len, "Inline_%s_%s", op, suffix);
#endif
  *_out_val = out;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the build inline response name operation.
 */
cdd_c_error_t build_inline_response_name(const char *op_id, const char *code,
                                         int is_item, char **_out_val) {
  const char *op;
  const char *resp;
  const char *suffix;
  size_t len;
  char *out;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  op = (op_id && *op_id) ? op_id : "unnamed";
  resp = (code && *code) ? code : "default";
  suffix = is_item ? "Item" : "";
  len = strlen("Inline_") + strlen(op) + strlen("_Response_") + strlen(resp) +
        (suffix[0] ? 1 + strlen(suffix) : 0) + 1;
  out = (char *)C_CDD_MALLOC(len);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (suffix[0]) {
    sprintf_s(out, len, "Inline_%s_Response_%s_%s", op, resp, suffix);
  } else {
    sprintf_s(out, len, "Inline_%s_Response_%s", op, resp);
  }
#else
  if (suffix[0]) {
    CDD_SNPRINTF(out, len, "Inline_%s_Response_%s_%s", op, resp, suffix);
  } else {
    CDD_SNPRINTF(out, len, "Inline_%s_Response_%s", op, resp);
  }
#endif
  *_out_val = out;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the build inline param name operation.
 */
cdd_c_error_t build_inline_param_name(const char *param_name, char **_out_val) {
  const char *p;
  size_t len;
  char *out;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  p = (param_name && *param_name) ? param_name : "param";
  len = strlen("Inline_Querystring_") + strlen(p) + 1;
  out = (char *)C_CDD_MALLOC(len);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(out, len, "Inline_Querystring_%s", p);
#else
  CDD_SNPRINTF(out, len, "Inline_Querystring_%s", p);
#endif
  *_out_val = out;
  return CDD_C_SUCCESS;
}
