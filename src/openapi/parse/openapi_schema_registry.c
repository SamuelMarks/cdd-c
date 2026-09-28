/**
 * @file openapi_schema_registry.c
 * @brief Inline schema registration and naming utilities.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Applies schema ref to param.
 */
cdd_c_error_t
apply_schema_ref_to_param(struct OpenAPI_Parameter *out_param,
                          const struct OpenAPI_SchemaRef *schema_ref) {
  char *_ast_strdup_192 = NULL;
  char *_ast_strdup_193 = NULL;
  char *_ast_strdup_194 = NULL;
  char *_ast_strdup_195 = NULL;
  char *_ast_strdup_196 = NULL;
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
    out_param->type =
        (c_cdd_strdup("array", &_ast_strdup_192), _ast_strdup_192);
    if (!out_param->type)
      return CDD_C_ERROR_MEMORY;
    if (schema_ref->inline_type) {
      out_param->items_type =
          (c_cdd_strdup(schema_ref->inline_type, &_ast_strdup_193),
           _ast_strdup_193);
      if (!out_param->items_type) {
        free(out_param->type);
        out_param->type = NULL;
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (schema_ref->ref_name) {
      out_param->items_type =
          (c_cdd_strdup(schema_ref->ref_name, &_ast_strdup_194),
           _ast_strdup_194);
      if (!out_param->items_type)
        return CDD_C_ERROR_MEMORY;
    }
    return CDD_C_SUCCESS;
  }

  out_param->is_array = 0;
  if (schema_ref->inline_type) {
    out_param->type = (c_cdd_strdup(schema_ref->inline_type, &_ast_strdup_195),
                       _ast_strdup_195);
  } else if (schema_ref->ref_name) {
    out_param->type =
        (c_cdd_strdup(schema_ref->ref_name, &_ast_strdup_196), _ast_strdup_196);
  }
  if (!out_param->type)
    return CDD_C_ERROR_MEMORY;

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies schema ref to header.
 */
cdd_c_error_t
apply_schema_ref_to_header(struct OpenAPI_Header *out_hdr,
                           const struct OpenAPI_SchemaRef *schema_ref) {
  char *_ast_strdup_197 = NULL;
  char *_ast_strdup_198 = NULL;
  char *_ast_strdup_199 = NULL;
  char *_ast_strdup_200 = NULL;
  char *_ast_strdup_201 = NULL;
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
    out_hdr->type = (c_cdd_strdup("array", &_ast_strdup_197), _ast_strdup_197);
    if (!out_hdr->type)
      return CDD_C_ERROR_MEMORY;
    if (schema_ref->inline_type) {
      out_hdr->items_type =
          (c_cdd_strdup(schema_ref->inline_type, &_ast_strdup_198),
           _ast_strdup_198);
      if (!out_hdr->items_type) {
        free(out_hdr->type);
        out_hdr->type = NULL;
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (schema_ref->ref_name) {
      out_hdr->items_type =
          (c_cdd_strdup(schema_ref->ref_name, &_ast_strdup_199),
           _ast_strdup_199);
      if (!out_hdr->items_type)
        return CDD_C_ERROR_MEMORY;
    }
    return CDD_C_SUCCESS;
  }

  out_hdr->is_array = 0;
  if (schema_ref->inline_type) {
    out_hdr->type = (c_cdd_strdup(schema_ref->inline_type, &_ast_strdup_200),
                     _ast_strdup_200);
  } else if (schema_ref->ref_name) {
    out_hdr->type =
        (c_cdd_strdup(schema_ref->ref_name, &_ast_strdup_201), _ast_strdup_201);
  }
  if (!out_hdr->type)
    return CDD_C_ERROR_MEMORY;

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
  char *_ast_strdup_202 = NULL;
  char *_ast_strdup_203 = NULL;
  size_t i, len;
  char *out;
  if (!name || !*name) {
    *_out_val =
        (c_cdd_strdup("InlineSchema", &_ast_strdup_202), _ast_strdup_202);
    return CDD_C_SUCCESS;
  }
  len = strlen(name);
  out = (char *)(size_t)calloc(len + 1, sizeof(char));
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
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
  if (!out[0]) {
    free(out);
    {
      *_out_val =
          (c_cdd_strdup("InlineSchema", &_ast_strdup_203), _ast_strdup_203);
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the make unique schema name operation.
 */
cdd_c_error_t make_unique_schema_name(const struct OpenAPI_Spec *spec,
                                      const char *base, char **_out_val) {
  char *_ast_strdup_204 = NULL;
  char *_ast_strdup_205 = NULL;
  size_t attempt = 0;
  if (!base) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (!schema_name_in_use(spec, base)) {
    *_out_val = (c_cdd_strdup(base, &_ast_strdup_204), _ast_strdup_204);
    return CDD_C_SUCCESS;
  }
  for (attempt = 1; attempt < 10000; ++attempt) {
    char buf[256];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, sizeof(buf), "%s_%lu", base, (unsigned long)attempt);
#else
    sprintf(buf, "%s_%lu", base, (unsigned long)attempt);
#endif
    if (!schema_name_in_use(spec, buf)) {
      *_out_val = (c_cdd_strdup(buf, &_ast_strdup_205), _ast_strdup_205);
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
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
  if (schema_type_array_includes(type_arr, "object"))
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
  new_names = (char **)calloc(new_count, sizeof(char *));
  new_ids = (char **)calloc(new_count, sizeof(char *));
  new_anchors = (char **)calloc(new_count, sizeof(char *));
  new_dyn_anchors = (char **)calloc(new_count, sizeof(char *));
  new_schemas =
      (struct StructFields *)calloc(new_count, sizeof(struct StructFields));
  if (!new_names || !new_ids || !new_anchors || !new_dyn_anchors ||
      !new_schemas) {
    free(new_names);
    free(new_ids);
    free(new_anchors);
    free(new_dyn_anchors);
    free(new_schemas);
    return CDD_C_ERROR_MEMORY;
  }

  for (i = 0; i < spec->n_defined_schemas; ++i) {
    new_names[i] =
        spec->defined_schema_names ? spec->defined_schema_names[i] : NULL;
    new_ids[i] = spec->defined_schema_ids ? spec->defined_schema_ids[i] : NULL;
    new_anchors[i] =
        spec->defined_schema_anchors ? spec->defined_schema_anchors[i] : NULL;
    new_dyn_anchors[i] = spec->defined_schema_dynamic_anchors
                             ? spec->defined_schema_dynamic_anchors[i]
                             : NULL;
    new_schemas[i] =
        spec->defined_schemas ? spec->defined_schemas[i] : new_schemas[i];
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
  char *_ast_strdup_206 = NULL;
  char *_ast_strdup_207 = NULL;
  size_t i;
  size_t new_count;
  char **new_names = NULL;
  char **new_json = NULL;
  char *dup_name = NULL;
  char *dup_json = NULL;
  char *raw_json = NULL;

  if (!spec || !name || !schema_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (raw_schema_name_exists(spec, name))
    return CDD_C_SUCCESS;

  raw_json = json_serialize_to_string(schema_val);
  if (!raw_json)
    return CDD_C_ERROR_MEMORY;
  dup_json = (c_cdd_strdup(raw_json, &_ast_strdup_206), _ast_strdup_206);
  json_free_serialized_string(raw_json);
  if (!dup_json)
    return CDD_C_ERROR_MEMORY;

  dup_name = (c_cdd_strdup(name, &_ast_strdup_207), _ast_strdup_207);
  if (!dup_name) {
    free(dup_json);
    return CDD_C_ERROR_MEMORY;
  }

  new_count = spec->n_raw_schemas + 1;
  new_names = (char **)calloc(new_count, sizeof(char *));
  new_json = (char **)calloc(new_count, sizeof(char *));
  if (!new_names || !new_json) {
    free(new_names);
    free(new_json);
    free(dup_name);
    free(dup_json);
    return CDD_C_ERROR_MEMORY;
  }

  for (i = 0; i < spec->n_raw_schemas; ++i) {
    new_names[i] = spec->raw_schema_names ? spec->raw_schema_names[i] : NULL;
    new_json[i] = spec->raw_schema_json ? spec->raw_schema_json[i] : NULL;
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
  char *_ast_sanitize_component_name_49 = NULL;
  char *_ast_make_unique_schema_name_50 = NULL;
  struct StructFields tmp;
  char *sanitized = NULL;
  char *unique = NULL;
  cdd_c_error_t rc;

  if (!spec || !schema_obj || !out_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    cdd_c_error_t _rc = struct_fields_init(&tmp);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  rc = json_object_to_struct_fields_ex(schema_obj, &tmp, NULL, base_name);
  if (rc != CDD_C_SUCCESS) {
    struct_fields_free(&tmp);
    return rc;
  }

  sanitized =
      (sanitize_component_name(base_name, &_ast_sanitize_component_name_49),
       _ast_sanitize_component_name_49);
  if (!sanitized) {
    struct_fields_free(&tmp);
    return CDD_C_ERROR_MEMORY;
  }

  unique = (make_unique_schema_name(spec, sanitized,
                                    &_ast_make_unique_schema_name_50),
            _ast_make_unique_schema_name_50);
  free(sanitized);
  if (!unique) {
    struct_fields_free(&tmp);
    return CDD_C_ERROR_MEMORY;
  }

  rc = append_defined_schema(spec, unique, &tmp);
  if (rc != CDD_C_SUCCESS) {
    struct_fields_free(&tmp);
    free(unique);
    return rc;
  }

  if (schema_has_composition(schema_obj) && schema_val) {
    cdd_c_error_t raw_rc = append_raw_schema(spec, unique, schema_val);
    if (raw_rc != CDD_C_SUCCESS) {
      struct_fields_free(&tmp);
      return raw_rc;
    }
  }

  *out_name = unique;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the assign schema ref name operation.
 */
cdd_c_error_t assign_schema_ref_name(struct OpenAPI_SchemaRef *schema_ref,
                                     const char *name) {
  char *_ast_strdup_208 = NULL;
  char *dup;
  if (!schema_ref || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  dup = (c_cdd_strdup(name, &_ast_strdup_208), _ast_strdup_208);
  if (!dup)
    return CDD_C_ERROR_MEMORY;
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
  const char *op = (op_id && *op_id) ? op_id : "unnamed";
  const char *suffix = is_item ? "Request_Item" : "Request";
  size_t len = strlen("Inline_") + strlen(op) + 1 + strlen(suffix) + 1;
  char *out = (char *)(size_t)malloc(len);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(out, len, "Inline_%s_%s", op, suffix);
#else
  sprintf(out, "Inline_%s_%s", op, suffix);
#endif
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the build inline response name operation.
 */
cdd_c_error_t build_inline_response_name(const char *op_id, const char *code,
                                         int is_item, char **_out_val) {
  const char *op = (op_id && *op_id) ? op_id : "unnamed";
  const char *resp = (code && *code) ? code : "default";
  const char *suffix = is_item ? "Item" : "";
  size_t len = strlen("Inline_") + strlen(op) + strlen("_Response_") +
               strlen(resp) + (suffix[0] ? 1 + strlen(suffix) : 0) + 1;
  char *out = (char *)(size_t)malloc(len);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (suffix[0]) {
    sprintf_s(out, len, "Inline_%s_Response_%s_%s", op, resp, suffix);
  } else {
    sprintf_s(out, len, "Inline_%s_Response_%s", op, resp);
  }
#else
  if (suffix[0]) {
    sprintf(out, "Inline_%s_Response_%s_%s", op, resp, suffix);
  } else {
    sprintf(out, "Inline_%s_Response_%s", op, resp);
  }
#endif
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the build inline param name operation.
 */
cdd_c_error_t build_inline_param_name(const char *param_name, char **_out_val) {
  const char *p = (param_name && *param_name) ? param_name : "param";
  size_t len = strlen("Inline_Querystring_") + strlen(p) + 1;
  char *out = (char *)(size_t)malloc(len);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(out, len, "Inline_Querystring_%s", p);
#else
  sprintf(out, "Inline_Querystring_%s", p);
#endif
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}
