/**
 * @file openapi_copy_operations.c
 * @brief Deep copy routines for operations, paths, servers, and links.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Creates a deep copy of server object.
 *
 * @param[out] dst Destination OpenAPI_Server structure.
 * @param[in] src Source OpenAPI_Server structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_server_object(struct OpenAPI_Server *dst,
                                 const struct OpenAPI_Server *src) {
  size_t v, e;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->url) {
    c_cdd_strdup(src->url, &dst->url);
    if (!dst->url)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description) {
    c_cdd_strdup(src->description, &dst->description);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->name) {
    c_cdd_strdup(src->name, &dst->name);
    if (!dst->name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->extensions_json) {
    c_cdd_strdup(src->extensions_json, &dst->extensions_json);
    if (!dst->extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->n_variables > 0 && src->variables) {
    dst->variables = (struct OpenAPI_ServerVariable *)C_CDD_CALLOC(
        src->n_variables, sizeof(struct OpenAPI_ServerVariable));
    if (!dst->variables)
      return CDD_C_ERROR_MEMORY;
    dst->n_variables = src->n_variables;
    for (v = 0; v < src->n_variables; ++v) {
      const struct OpenAPI_ServerVariable *src_var = &src->variables[v];
      struct OpenAPI_ServerVariable *dst_var = &dst->variables[v];
      if (src_var->name) {
        c_cdd_strdup(src_var->name, &dst_var->name);
        if (!dst_var->name)
          return CDD_C_ERROR_MEMORY;
      }
      if (src_var->default_value) {
        c_cdd_strdup(src_var->default_value, &dst_var->default_value);
        if (!dst_var->default_value)
          return CDD_C_ERROR_MEMORY;
      }
      if (src_var->description) {
        c_cdd_strdup(src_var->description, &dst_var->description);
        if (!dst_var->description)
          return CDD_C_ERROR_MEMORY;
      }
      if (src_var->extensions_json) {
        c_cdd_strdup(src_var->extensions_json, &dst_var->extensions_json);
        if (!dst_var->extensions_json)
          return CDD_C_ERROR_MEMORY;
      }
      if (src_var->n_enum_values > 0 && src_var->enum_values) {
        dst_var->enum_values =
            (char **)C_CDD_CALLOC(src_var->n_enum_values, sizeof(char *));
        if (!dst_var->enum_values)
          return CDD_C_ERROR_MEMORY;
        dst_var->n_enum_values = src_var->n_enum_values;
        for (e = 0; e < src_var->n_enum_values; ++e) {
          if (src_var->enum_values[e]) {
            c_cdd_strdup(src_var->enum_values[e], &dst_var->enum_values[e]);
            if (!dst_var->enum_values[e])
              return CDD_C_ERROR_MEMORY;
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of link fields.
 *
 * @param[out] dst Destination OpenAPI_Link structure.
 * @param[in] src Source OpenAPI_Link structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_link_fields(struct OpenAPI_Link *dst,
                               const struct OpenAPI_Link *src) {
  size_t i;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->summary) {
    c_cdd_strdup(src->summary, &dst->summary);
    if (!dst->summary)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description) {
    c_cdd_strdup(src->description, &dst->description);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->extensions_json) {
    c_cdd_strdup(src->extensions_json, &dst->extensions_json);
    if (!dst->extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->operation_ref) {
    c_cdd_strdup(src->operation_ref, &dst->operation_ref);
    if (!dst->operation_ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->operation_id) {
    c_cdd_strdup(src->operation_id, &dst->operation_id);
    if (!dst->operation_id)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->n_parameters > 0 && src->parameters) {
    dst->parameters = (struct OpenAPI_LinkParam *)C_CDD_CALLOC(
        src->n_parameters, sizeof(struct OpenAPI_LinkParam));
    if (!dst->parameters)
      return CDD_C_ERROR_MEMORY;
    dst->n_parameters = src->n_parameters;
    for (i = 0; i < src->n_parameters; ++i) {
      cdd_c_error_t rc;
      if (src->parameters[i].name) {
        c_cdd_strdup(src->parameters[i].name, &dst->parameters[i].name);
        if (!dst->parameters[i].name)
          return CDD_C_ERROR_MEMORY;
      }
      rc = copy_any_value(&dst->parameters[i].value, &src->parameters[i].value);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->request_body_set) {
    cdd_c_error_t rc;
    dst->request_body_set = 1;
    rc = copy_any_value(&dst->request_body, &src->request_body);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->server_set && src->server) {
    cdd_c_error_t rc;
    dst->server =
        (struct OpenAPI_Server *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Server));
    if (!dst->server)
      return CDD_C_ERROR_MEMORY;
    dst->server_set = 1;
    rc = copy_server_object(dst->server, src->server);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of security requirement sets.
 *
 * @param[out] dst Output array of OpenAPI_SecurityRequirementSet structures.
 * @param[out] dst_count Output count of security requirement sets.
 * @param[in] src Source array of OpenAPI_SecurityRequirementSet structures.
 * @param[in] src_count Number of elements in src.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_security_requirement_sets(
    struct OpenAPI_SecurityRequirementSet **dst, size_t *dst_count,
    const struct OpenAPI_SecurityRequirementSet *src, size_t src_count) {
  size_t i, j, k;
  if (!dst || !dst_count)
    return CDD_C_SUCCESS;
  *dst = NULL;
  *dst_count = 0;
  if (!src || src_count == 0)
    return CDD_C_SUCCESS;
  *dst = (struct OpenAPI_SecurityRequirementSet *)C_CDD_CALLOC(
      src_count, sizeof(struct OpenAPI_SecurityRequirementSet));
  if (!*dst)
    return CDD_C_ERROR_MEMORY;
  *dst_count = src_count;
  for (i = 0; i < src_count; ++i) {
    struct OpenAPI_SecurityRequirementSet *dst_set = &(*dst)[i];
    const struct OpenAPI_SecurityRequirementSet *src_set = &src[i];
    if (src_set->extensions_json) {
      c_cdd_strdup(src_set->extensions_json, &dst_set->extensions_json);
      if (!dst_set->extensions_json)
        return CDD_C_ERROR_MEMORY;
    }
    if (src_set->n_requirements > 0 && src_set->requirements) {
      dst_set->requirements =
          (struct OpenAPI_SecurityRequirement *)C_CDD_CALLOC(
              src_set->n_requirements,
              sizeof(struct OpenAPI_SecurityRequirement));
      if (!dst_set->requirements)
        return CDD_C_ERROR_MEMORY;
      dst_set->n_requirements = src_set->n_requirements;
      for (j = 0; j < src_set->n_requirements; ++j) {
        struct OpenAPI_SecurityRequirement *dst_req = &dst_set->requirements[j];
        const struct OpenAPI_SecurityRequirement *src_req =
            &src_set->requirements[j];
        if (src_req->scheme) {
          c_cdd_strdup(src_req->scheme, &dst_req->scheme);
          if (!dst_req->scheme)
            return CDD_C_ERROR_MEMORY;
        }
        if (src_req->n_scopes > 0 && src_req->scopes) {
          dst_req->scopes =
              (char **)C_CDD_CALLOC(src_req->n_scopes, sizeof(char *));
          if (!dst_req->scopes)
            return CDD_C_ERROR_MEMORY;
          dst_req->n_scopes = src_req->n_scopes;
          for (k = 0; k < src_req->n_scopes; ++k) {
            if (src_req->scopes[k]) {
              c_cdd_strdup(src_req->scopes[k], &dst_req->scopes[k]);
              if (!dst_req->scopes[k])
                return CDD_C_ERROR_MEMORY;
            }
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of callback fields.
 *
 * @param[out] dst Destination OpenAPI_Callback structure.
 * @param[in] src Source OpenAPI_Callback structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_callback_fields(struct OpenAPI_Callback *dst,
                                   const struct OpenAPI_Callback *src) {
  size_t i;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (!dst->name && src->name) {
    c_cdd_strdup(src->name, &dst->name);
    if (!dst->name)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dst->ref && src->ref) {
    c_cdd_strdup(src->ref, &dst->ref);
    if (!dst->ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dst->summary && src->summary) {
    c_cdd_strdup(src->summary, &dst->summary);
    if (!dst->summary)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dst->description && src->description) {
    c_cdd_strdup(src->description, &dst->description);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dst->extensions_json && src->extensions_json) {
    c_cdd_strdup(src->extensions_json, &dst->extensions_json);
    if (!dst->extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->n_paths > 0 && src->paths && !dst->paths) {
    dst->paths = (struct OpenAPI_Path *)C_CDD_CALLOC(
        src->n_paths, sizeof(struct OpenAPI_Path));
    if (!dst->paths)
      return CDD_C_ERROR_MEMORY;
    dst->n_paths = src->n_paths;
    for (i = 0; i < src->n_paths; ++i) {
      cdd_c_error_t rc = copy_path_fields(&dst->paths[i], &src->paths[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of operation fields.
 *
 * @param[out] dst Destination OpenAPI_Operation structure.
 * @param[in] src Source OpenAPI_Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_operation_fields(struct OpenAPI_Operation *dst,
                                    const struct OpenAPI_Operation *src) {
  size_t i;
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  dst->verb = src->verb;
  dst->is_additional = src->is_additional;
  dst->deprecated = src->deprecated;
  dst->security_set = src->security_set;
  if (src->method) {
    c_cdd_strdup(src->method, &dst->method);
    if (!dst->method)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->operation_id) {
    c_cdd_strdup(src->operation_id, &dst->operation_id);
    if (!dst->operation_id)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->summary) {
    c_cdd_strdup(src->summary, &dst->summary);
    if (!dst->summary)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description) {
    c_cdd_strdup(src->description, &dst->description);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->extensions_json) {
    c_cdd_strdup(src->extensions_json, &dst->extensions_json);
    if (!dst->extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->n_security > 0 && src->security) {
    rc = copy_security_requirement_sets(&dst->security, &dst->n_security,
                                        src->security, src->n_security);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->n_parameters > 0 && src->parameters) {
    dst->parameters = (struct OpenAPI_Parameter *)C_CDD_CALLOC(
        src->n_parameters, sizeof(struct OpenAPI_Parameter));
    if (!dst->parameters)
      return CDD_C_ERROR_MEMORY;
    dst->n_parameters = src->n_parameters;
    for (i = 0; i < src->n_parameters; ++i) {
      rc = copy_parameter_fields(&dst->parameters[i], &src->parameters[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->n_tags > 0 && src->tags) {
    dst->tags = (char **)C_CDD_CALLOC(src->n_tags, sizeof(char *));
    if (!dst->tags)
      return CDD_C_ERROR_MEMORY;
    dst->n_tags = src->n_tags;
    for (i = 0; i < src->n_tags; ++i) {
      if (src->tags[i]) {
        c_cdd_strdup(src->tags[i], &dst->tags[i]);
        if (!dst->tags[i])
          return CDD_C_ERROR_MEMORY;
      }
    }
  }
  rc = copy_schema_ref(&dst->req_body, &src->req_body);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (src->n_req_body_media_types > 0 && src->req_body_media_types) {
    rc = copy_media_type_array(
        &dst->req_body_media_types, &dst->n_req_body_media_types,
        src->req_body_media_types, src->n_req_body_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  dst->req_body_required = src->req_body_required;
  dst->req_body_required_set = src->req_body_required_set;
  if (src->req_body_description) {
    c_cdd_strdup(src->req_body_description, &dst->req_body_description);
    if (!dst->req_body_description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->req_body_extensions_json) {
    c_cdd_strdup(src->req_body_extensions_json, &dst->req_body_extensions_json);
    if (!dst->req_body_extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->req_body_ref) {
    c_cdd_strdup(src->req_body_ref, &dst->req_body_ref);
    if (!dst->req_body_ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->external_docs.description) {
    c_cdd_strdup(src->external_docs.description,
                 &dst->external_docs.description);
    if (!dst->external_docs.description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->external_docs.url) {
    c_cdd_strdup(src->external_docs.url, &dst->external_docs.url);
    if (!dst->external_docs.url)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->external_docs.extensions_json) {
    c_cdd_strdup(src->external_docs.extensions_json,
                 &dst->external_docs.extensions_json);
    if (!dst->external_docs.extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->n_servers > 0 && src->servers) {
    dst->servers = (struct OpenAPI_Server *)C_CDD_CALLOC(
        src->n_servers, sizeof(struct OpenAPI_Server));
    if (!dst->servers)
      return CDD_C_ERROR_MEMORY;
    dst->n_servers = src->n_servers;
    for (i = 0; i < src->n_servers; ++i) {
      rc = copy_server_object(&dst->servers[i], &src->servers[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->n_responses > 0 && src->responses) {
    dst->responses = (struct OpenAPI_Response *)C_CDD_CALLOC(
        src->n_responses, sizeof(struct OpenAPI_Response));
    if (!dst->responses)
      return CDD_C_ERROR_MEMORY;
    dst->n_responses = src->n_responses;
    for (i = 0; i < src->n_responses; ++i) {
      rc = copy_response_fields(&dst->responses[i], &src->responses[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->n_callbacks > 0 && src->callbacks) {
    dst->callbacks = (struct OpenAPI_Callback *)C_CDD_CALLOC(
        src->n_callbacks, sizeof(struct OpenAPI_Callback));
    if (!dst->callbacks)
      return CDD_C_ERROR_MEMORY;
    dst->n_callbacks = src->n_callbacks;
    for (i = 0; i < src->n_callbacks; ++i) {
      rc = copy_callback_fields(&dst->callbacks[i], &src->callbacks[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of path fields.
 *
 * @param[out] dst Destination OpenAPI_Path structure.
 * @param[in] src Source OpenAPI_Path structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_path_fields(struct OpenAPI_Path *dst,
                               const struct OpenAPI_Path *src) {
  size_t i;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (!dst->route && src->route) {
    c_cdd_strdup(src->route, &dst->route);
    if (!dst->route)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dst->ref && src->ref) {
    c_cdd_strdup(src->ref, &dst->ref);
    if (!dst->ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dst->summary && src->summary) {
    c_cdd_strdup(src->summary, &dst->summary);
    if (!dst->summary)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dst->description && src->description) {
    c_cdd_strdup(src->description, &dst->description);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dst->extensions_json && src->extensions_json) {
    c_cdd_strdup(src->extensions_json, &dst->extensions_json);
    if (!dst->extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->n_parameters > 0 && src->parameters && !dst->parameters) {
    dst->parameters = (struct OpenAPI_Parameter *)C_CDD_CALLOC(
        src->n_parameters, sizeof(struct OpenAPI_Parameter));
    if (!dst->parameters)
      return CDD_C_ERROR_MEMORY;
    dst->n_parameters = src->n_parameters;
    for (i = 0; i < src->n_parameters; ++i) {
      cdd_c_error_t rc =
          copy_parameter_fields(&dst->parameters[i], &src->parameters[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->n_servers > 0 && src->servers && !dst->servers) {
    dst->servers = (struct OpenAPI_Server *)C_CDD_CALLOC(
        src->n_servers, sizeof(struct OpenAPI_Server));
    if (!dst->servers)
      return CDD_C_ERROR_MEMORY;
    dst->n_servers = src->n_servers;
    for (i = 0; i < src->n_servers; ++i) {
      cdd_c_error_t rc = copy_server_object(&dst->servers[i], &src->servers[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->n_operations > 0 && src->operations && !dst->operations) {
    dst->operations = (struct OpenAPI_Operation *)C_CDD_CALLOC(
        src->n_operations, sizeof(struct OpenAPI_Operation));
    if (!dst->operations)
      return CDD_C_ERROR_MEMORY;
    dst->n_operations = src->n_operations;
    for (i = 0; i < src->n_operations; ++i) {
      cdd_c_error_t rc =
          copy_operation_fields(&dst->operations[i], &src->operations[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->n_additional_operations > 0 && src->additional_operations &&
      !dst->additional_operations) {
    dst->additional_operations = (struct OpenAPI_Operation *)C_CDD_CALLOC(
        src->n_additional_operations, sizeof(struct OpenAPI_Operation));
    if (!dst->additional_operations)
      return CDD_C_ERROR_MEMORY;
    dst->n_additional_operations = src->n_additional_operations;
    for (i = 0; i < src->n_additional_operations; ++i) {
      cdd_c_error_t rc = copy_operation_fields(&dst->additional_operations[i],
                                               &src->additional_operations[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of request body fields.
 *
 * @param[out] dst Destination OpenAPI_RequestBody structure.
 * @param[in] src Source OpenAPI_RequestBody structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_request_body_fields(struct OpenAPI_RequestBody *dst,
                                       const struct OpenAPI_RequestBody *src) {
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->ref && !dst->ref) {
    c_cdd_strdup(src->ref, &dst->ref);
    if (!dst->ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description) {
    if (dst->description) {
      C_CDD_FREE(dst->description);
      dst->description = NULL;
    }
    c_cdd_strdup(src->description, &dst->description);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_ref) {
    c_cdd_strdup(src->content_ref, &dst->content_ref);
    if (!dst->content_ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->extensions_json) {
    c_cdd_strdup(src->extensions_json, &dst->extensions_json);
    if (!dst->extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    rc = copy_media_type_array(
        &dst->content_media_types, &dst->n_content_media_types,
        src->content_media_types, src->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->example_set) {
    rc = copy_any_value(&dst->example, &src->example);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    size_t i;
    dst->examples = (struct OpenAPI_Example *)C_CDD_CALLOC(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      rc = copy_example_fields(&dst->examples[i], &src->examples[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  dst->required = src->required;
  dst->required_set = src->required_set;
  rc = copy_schema_ref(&dst->schema, &src->schema);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}
