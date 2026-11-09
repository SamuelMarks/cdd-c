/**
 * @file openapi_components.c
 * @brief Components object and component collection parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses component parameters from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_component_parameters(
    const JSON_Object *components, struct OpenAPI_Spec *out) {
  JSON_Object *params = NULL;
  size_t count = 0, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(components, "parameters", &params);
  if (!params)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(params) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(params, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->component_parameters = (struct OpenAPI_Parameter *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Parameter));
  out->component_parameter_names = (char **)C_CDD_CALLOC(count, sizeof(char *));
  out->n_component_parameters = count;
  if (!out->component_parameters || !out->component_parameter_names)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *p_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_name(params, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(params, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &p_obj);
    g_cdd_last_alloc_err =
        c_cdd_strdup(name, &out->component_parameter_names[i]);
    if (!out->component_parameter_names[i])
      return CDD_C_ERROR_MEMORY;
    if (p_obj) {
      cdd_c_error_t rc =
          parse_parameter_object(p_obj, &out->component_parameters[i], out, 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses component responses from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_component_responses(
    const JSON_Object *components, struct OpenAPI_Spec *out) {
  JSON_Object *responses = NULL;
  size_t count = 0, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(components, "responses", &responses);
  if (!responses)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(responses) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(responses, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->component_responses = (struct OpenAPI_Response *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Response));
  out->component_response_names = (char **)C_CDD_CALLOC(count, sizeof(char *));
  out->n_component_responses = count;
  if (!out->component_responses || !out->component_response_names)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *r_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_name(responses, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(responses, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &r_obj);
    g_cdd_last_alloc_err =
        c_cdd_strdup(name, &out->component_response_names[i]);
    if (!out->component_response_names[i])
      return CDD_C_ERROR_MEMORY;
    if (r_obj) {
      cdd_c_error_t rc = parse_response_object(
          r_obj, &out->component_responses[i], out, 0, NULL, NULL);
      if (rc != CDD_C_SUCCESS) {
        out->n_component_responses = i + 1;
        return rc;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses component headers from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_component_headers(
    const JSON_Object *components, struct OpenAPI_Spec *out) {
  JSON_Object *headers = NULL;
  size_t count = 0, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(components, "headers", &headers);
  if (!headers)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(headers) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(headers, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->component_headers = (struct OpenAPI_Header *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Header));
  out->component_header_names = (char **)C_CDD_CALLOC(count, sizeof(char *));
  out->n_component_headers = count;
  if (!out->component_headers || !out->component_header_names)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *h_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_name(headers, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(headers, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &h_obj);
    g_cdd_last_alloc_err = c_cdd_strdup(name, &out->component_header_names[i]);
    if (!out->component_header_names[i])
      return CDD_C_ERROR_MEMORY;
    if (h_obj) {
      cdd_c_error_t rc =
          parse_header_object(h_obj, &out->component_headers[i], out, 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses component request bodies from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_component_request_bodies(
    const JSON_Object *components, struct OpenAPI_Spec *out) {
  JSON_Object *bodies = NULL;
  size_t count = 0, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(components, "requestBodies", &bodies);
  if (!bodies)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(bodies) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(bodies, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->component_request_bodies = (struct OpenAPI_RequestBody *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_RequestBody));
  out->component_request_body_names =
      (char **)C_CDD_CALLOC(count, sizeof(char *));
  out->n_component_request_bodies = count;
  if (!out->component_request_bodies || !out->component_request_body_names)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *rb_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_name(bodies, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(bodies, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &rb_obj);
    g_cdd_last_alloc_err =
        c_cdd_strdup(name, &out->component_request_body_names[i]);
    if (!out->component_request_body_names[i])
      return CDD_C_ERROR_MEMORY;
    if (rb_obj) {
      cdd_c_error_t rc = parse_request_body_object(
          rb_obj, &out->component_request_bodies[i], out, 0, NULL);
      if (rc != CDD_C_SUCCESS) {
        out->n_component_request_bodies = i + 1;
        return rc;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses component media types from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_component_media_types(
    const JSON_Object *components, struct OpenAPI_Spec *out) {
  JSON_Object *media_types = NULL;
  size_t count = 0, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(components, "mediaTypes", &media_types);
  if (!media_types)
    return CDD_C_SUCCESS;

  if (validate_media_type_key_map(media_types) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(media_types, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->component_media_types = (struct OpenAPI_MediaType *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_MediaType));
  out->component_media_type_names =
      (char **)C_CDD_CALLOC(count, sizeof(char *));
  out->n_component_media_types = count;
  if (!out->component_media_types || !out->component_media_type_names)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *mt_obj = NULL;
    struct OpenAPI_MediaType *curr = &out->component_media_types[i];
    g_cdd_last_alloc_err = cdd_json_object_get_name(media_types, i, &name);
    g_cdd_last_alloc_err =
        cdd_json_object_get_value_at(media_types, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &mt_obj);

    g_cdd_last_alloc_err =
        c_cdd_strdup(name, &out->component_media_type_names[i]);
    if (!out->component_media_type_names[i])
      return CDD_C_ERROR_MEMORY;
    g_cdd_last_alloc_err = c_cdd_strdup(name, &curr->name);
    if (!curr->name)
      return CDD_C_ERROR_MEMORY;

    if (mt_obj) {
      cdd_c_error_t rc = parse_media_type_object(mt_obj, curr, out, 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses component examples from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_component_examples(
    const JSON_Object *components, struct OpenAPI_Spec *out) {
  JSON_Object *examples = NULL;
  size_t count = 0, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(components, "examples", &examples);
  if (!examples)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(examples) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(examples, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->component_examples = (struct OpenAPI_Example *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Example));
  out->component_example_names = (char **)C_CDD_CALLOC(count, sizeof(char *));
  out->n_component_examples = count;
  if (!out->component_examples || !out->component_example_names)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *ex_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_name(examples, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(examples, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &ex_obj);
    g_cdd_last_alloc_err = c_cdd_strdup(name, &out->component_example_names[i]);
    if (!out->component_example_names[i])
      return CDD_C_ERROR_MEMORY;
    if (ex_obj) {
      cdd_c_error_t rc = parse_example_object(
          ex_obj, name, &out->component_examples[i], out, 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses component links from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_component_links(const JSON_Object *components,
                                                 struct OpenAPI_Spec *out) {
  JSON_Object *links = NULL;
  size_t count = 0, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(components, "links", &links);
  if (!links)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(links) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(links, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->component_links =
      (struct OpenAPI_Link *)C_CDD_CALLOC(count, sizeof(struct OpenAPI_Link));
  if (!out->component_links)
    return CDD_C_ERROR_MEMORY;
  out->n_component_links = count;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *link_obj = NULL;
    struct OpenAPI_Link *curr = &out->component_links[i];
    g_cdd_last_alloc_err = cdd_json_object_get_name(links, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(links, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &link_obj);
    g_cdd_last_alloc_err = c_cdd_strdup(name, &curr->name);
    if (!curr->name)
      return CDD_C_ERROR_MEMORY;
    if (link_obj) {
      cdd_c_error_t rc = parse_link_object(link_obj, curr, out, 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses component callbacks from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_component_callbacks(
    const JSON_Object *components, struct OpenAPI_Spec *out) {
  JSON_Object *callbacks = NULL;
  size_t count = 0, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(components, "callbacks", &callbacks);
  if (!callbacks)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(callbacks) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(callbacks, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->component_callbacks = (struct OpenAPI_Callback *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Callback));
  if (!out->component_callbacks)
    return CDD_C_ERROR_MEMORY;
  out->n_component_callbacks = count;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *cb_obj = NULL;
    struct OpenAPI_Callback *curr = &out->component_callbacks[i];
    g_cdd_last_alloc_err = cdd_json_object_get_name(callbacks, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(callbacks, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &cb_obj);
    g_cdd_last_alloc_err = c_cdd_strdup(name, &curr->name);
    if (!curr->name)
      return CDD_C_ERROR_MEMORY;
    if (cb_obj) {
      cdd_c_error_t rc = parse_callback_object(cb_obj, curr, out, 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses component path items from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_component_path_items(
    const JSON_Object *components, struct OpenAPI_Spec *out) {
  JSON_Object *path_items = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(components, "pathItems", &path_items);
  if (!path_items)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(path_items) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = parse_paths_object(path_items, &out->component_path_items,
                          &out->n_component_path_items, out, 0, 0);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (out->n_component_path_items == 0)
    return CDD_C_SUCCESS;

  out->component_path_item_names =
      (char **)C_CDD_CALLOC(out->n_component_path_items, sizeof(char *));
  if (!out->component_path_item_names)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < out->n_component_path_items; ++i) {
    const char *name = out->component_path_items[i].route;
    g_cdd_last_alloc_err =
        c_cdd_strdup(name, &out->component_path_item_names[i]);
    if (!out->component_path_item_names[i])
      return CDD_C_ERROR_MEMORY;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses components from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_components(const JSON_Object *components,
                                            struct OpenAPI_Spec *out) {
  JSON_Object *schemas = NULL;
  size_t i, count = 0;

  if (!components || !out)
    return CDD_C_SUCCESS;

  {
    cdd_c_error_t rc = parse_security_schemes(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_component_parameters(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_component_responses(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_component_headers(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_component_request_bodies(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_component_media_types(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_component_examples(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_component_links(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_component_callbacks(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_component_path_items(components, out);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  g_cdd_last_alloc_err = cdd_json_object_get_object(
      components, out->swagger_version ? "definitions" : "schemas", &schemas);
  if (!schemas)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(schemas) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(schemas, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  {
    size_t struct_count = 0;
    size_t raw_count = 0;
    for (i = 0; i < count; ++i) {
      JSON_Value *schema_val = NULL;
      JSON_Object *schema_obj = NULL;
      int is_struct = 0;
      int has_comp = 0;
      int needs_raw;
      g_cdd_last_alloc_err =
          cdd_json_object_get_value_at(schemas, i, &schema_val);
      g_cdd_last_alloc_err = cdd_json_value_get_object(schema_val, &schema_obj);
      g_cdd_last_alloc_err =
          schema_is_struct_compatible(schema_val, schema_obj, &is_struct);
      g_cdd_last_alloc_err = schema_has_composition(schema_obj, &has_comp);
      needs_raw = (!is_struct) || has_comp;
      if (is_struct)
        struct_count++;
      if (needs_raw)
        raw_count++;
    }

    if (struct_count > 0) {
      out->defined_schemas = (struct StructFields *)C_CDD_CALLOC(
          struct_count, sizeof(struct StructFields));
      out->defined_schema_names =
          (char **)C_CDD_CALLOC(struct_count, sizeof(char *));
      out->defined_schema_ids =
          (char **)C_CDD_CALLOC(struct_count, sizeof(char *));
      out->defined_schema_anchors =
          (char **)C_CDD_CALLOC(struct_count, sizeof(char *));
      out->defined_schema_dynamic_anchors =
          (char **)C_CDD_CALLOC(struct_count, sizeof(char *));
      out->n_defined_schemas = struct_count;
      if (!out->defined_schemas || !out->defined_schema_names ||
          !out->defined_schema_ids || !out->defined_schema_anchors ||
          !out->defined_schema_dynamic_anchors)
        return CDD_C_ERROR_MEMORY;
    }

    if (raw_count > 0) {
      out->raw_schema_names = (char **)C_CDD_CALLOC(raw_count, sizeof(char *));
      out->raw_schema_json = (char **)C_CDD_CALLOC(raw_count, sizeof(char *));
      out->n_raw_schemas = raw_count;
      if (!out->raw_schema_names || !out->raw_schema_json)
        return CDD_C_ERROR_MEMORY;
    }
  }

  {
    size_t struct_idx = 0;
    size_t raw_idx = 0;
    for (i = 0; i < count; ++i) {
      const char *name = NULL;
      JSON_Value *schema_val = NULL;
      JSON_Object *schema_obj = NULL;
      int is_struct = 0;
      int has_comp = 0;
      g_cdd_last_alloc_err = cdd_json_object_get_name(schemas, i, &name);
      g_cdd_last_alloc_err =
          cdd_json_object_get_value_at(schemas, i, &schema_val);
      g_cdd_last_alloc_err = cdd_json_value_get_object(schema_val, &schema_obj);
      g_cdd_last_alloc_err =
          schema_is_struct_compatible(schema_val, schema_obj, &is_struct);
      g_cdd_last_alloc_err = schema_has_composition(schema_obj, &has_comp);

      if (is_struct) {
        const char *schema_id = NULL;
        const char *schema_anchor = NULL;
        const char *schema_dynamic_anchor = NULL;
        g_cdd_last_alloc_err =
            c_cdd_strdup(name, &out->defined_schema_names[struct_idx]);
        if (!out->defined_schema_names[struct_idx])
          return CDD_C_ERROR_MEMORY;
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(schema_obj, "$id", &schema_id);
        if (schema_id) {
          g_cdd_last_alloc_err =
              c_cdd_strdup(schema_id, &out->defined_schema_ids[struct_idx]);
          if (!out->defined_schema_ids[struct_idx])
            return CDD_C_ERROR_MEMORY;
        }
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(schema_obj, "$anchor", &schema_anchor);
        g_cdd_last_alloc_err = cdd_json_object_get_string(
            schema_obj, "$dynamicAnchor", &schema_dynamic_anchor);
        if (schema_anchor) {
          g_cdd_last_alloc_err = c_cdd_strdup(
              schema_anchor, &out->defined_schema_anchors[struct_idx]);
          if (!out->defined_schema_anchors[struct_idx])
            return CDD_C_ERROR_MEMORY;
        }
        if (schema_dynamic_anchor) {
          g_cdd_last_alloc_err =
              c_cdd_strdup(schema_dynamic_anchor,
                           &out->defined_schema_dynamic_anchors[struct_idx]);
          if (!out->defined_schema_dynamic_anchors[struct_idx])
            return CDD_C_ERROR_MEMORY;
        }
        if (struct_fields_init(&out->defined_schemas[struct_idx]) !=
            CDD_C_SUCCESS) {
          return CDD_C_ERROR_MEMORY;
        }
        if (json_object_to_struct_fields_ex(schema_obj,
                                            &out->defined_schemas[struct_idx],
                                            schemas, name) != 0) {
          return CDD_C_ERROR_MEMORY;
        }
        struct_idx++;
      }

      if (!is_struct || has_comp) {
        char *raw_json = NULL;
        char *dup_json = NULL;
        g_cdd_last_alloc_err =
            c_cdd_strdup(name, &out->raw_schema_names[raw_idx]);
        if (!out->raw_schema_names[raw_idx])
          return CDD_C_ERROR_MEMORY;
        g_cdd_last_alloc_err =
            cdd_json_serialize_to_string(schema_val, &raw_json);
        if (!raw_json)
          return CDD_C_ERROR_MEMORY;
        g_cdd_last_alloc_err = c_cdd_strdup(raw_json, &dup_json);
        g_cdd_last_alloc_err = cdd_json_free_serialized_string(raw_json);
        if (!dup_json)
          return CDD_C_ERROR_MEMORY;
        out->raw_schema_json[raw_idx] = dup_json;
        raw_idx++;
      }
    }
  }

  return CDD_C_SUCCESS;
}
