/**
 * @file openapi_validation.c
 * @brief Querystring usage and unique operation ID validation.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Executes the scan querystring usage operation.
 */
cdd_c_error_t scan_querystring_usage(const struct OpenAPI_Parameter *params,
                                     size_t n_params, size_t *qs_count,
                                     int *has_query) {
  size_t i;
  if (!qs_count || !has_query || (!params && n_params > 0))
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < n_params; ++i) {
    const struct OpenAPI_Parameter *p = &params[i];
    if (p->in == OA_PARAM_IN_QUERYSTRING) {
      (*qs_count)++;
    } else if (p->in == OA_PARAM_IN_QUERY) {
      *has_query = 1;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate querystring usage operation.
 */
cdd_c_error_t validate_querystring_usage(const struct OpenAPI_Path *paths,
                                         size_t n_paths) {
  size_t i;
  cdd_c_error_t rc;
  for (i = 0; i < n_paths; ++i) {
    const struct OpenAPI_Path *path = &paths[i];
    size_t path_qs = 0;
    int path_has_query = 0;
    size_t op_idx;

    if (!path->route)
      continue;

    rc = scan_querystring_usage(path->parameters, path->n_parameters, &path_qs,
                                &path_has_query);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (path_qs > 1)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (path_qs > 0 && path_has_query)
      return CDD_C_ERROR_INVALID_ARGUMENT;

    for (op_idx = 0; op_idx < path->n_operations; ++op_idx) {
      size_t op_qs = 0;
      int op_has_query = 0;
      size_t total_qs;
      int has_query;
      rc = scan_querystring_usage(path->operations[op_idx].parameters,
                                  path->operations[op_idx].n_parameters, &op_qs,
                                  &op_has_query);
      if (rc != CDD_C_SUCCESS)
        return rc;
      total_qs = path_qs + op_qs;
      has_query = path_has_query || op_has_query;
      if (total_qs > 1)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      if (total_qs > 0 && has_query)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    for (op_idx = 0; op_idx < path->n_additional_operations; ++op_idx) {
      size_t op_qs = 0;
      int op_has_query = 0;
      size_t total_qs;
      int has_query;
      rc = scan_querystring_usage(
          path->additional_operations[op_idx].parameters,
          path->additional_operations[op_idx].n_parameters, &op_qs,
          &op_has_query);
      if (rc != CDD_C_SUCCESS)
        return rc;
      total_qs = path_qs + op_qs;
      has_query = path_has_query || op_has_query;
      if (total_qs > 1)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      if (total_qs > 0 && has_query)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate querystring usage in callbacks operation.
 */
cdd_c_error_t validate_querystring_usage_in_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks) {
  size_t i;
  if (!callbacks)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &callbacks[i];
    if (cb->paths && cb->n_paths > 0) {
      cdd_c_error_t rc = validate_querystring_usage(cb->paths, cb->n_paths);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate querystring usage in operations
 * operation.
 */
cdd_c_error_t
validate_querystring_usage_in_operations(const struct OpenAPI_Operation *ops,
                                         size_t n_ops) {
  size_t i;
  if (!ops)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_ops; ++i) {
    cdd_c_error_t rc = validate_querystring_usage_in_callbacks(
        ops[i].callbacks, ops[i].n_callbacks);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate querystring usage in paths callbacks
 * operation.
 */
cdd_c_error_t
validate_querystring_usage_in_paths_callbacks(const struct OpenAPI_Path *paths,
                                              size_t n_paths) {
  size_t i;
  if (!paths)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_paths; ++i) {
    cdd_c_error_t rc = validate_querystring_usage_in_operations(
        paths[i].operations, paths[i].n_operations);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = validate_querystring_usage_in_operations(
        paths[i].additional_operations, paths[i].n_additional_operations);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate querystring usage in component callbacks
 * operation.
 */
cdd_c_error_t validate_querystring_usage_in_component_callbacks(
    const struct OpenAPI_Spec *spec) {
  size_t i;
  if (!spec || !spec->component_callbacks)
    return CDD_C_SUCCESS;
  for (i = 0; i < spec->n_component_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &spec->component_callbacks[i];
    if (cb->paths && cb->n_paths > 0) {
      cdd_c_error_t rc = validate_querystring_usage(cb->paths, cb->n_paths);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets unique operation id.
 */
cdd_c_error_t add_unique_operation_id(char ***ids, size_t *count, size_t *cap,
                                      const char *op_id) {
  char *_ast_strdup_292 = NULL;
  size_t i;
  char **tmp;
  if (!op_id || !*op_id)
    return CDD_C_SUCCESS;
  for (i = 0; i < *count; ++i) {
    if ((*ids)[i] && strcmp((*ids)[i], op_id) == 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (*count == *cap) {
    size_t new_cap = *cap ? (*cap * 2) : 8;
    tmp = (char **)C_CDD_REALLOC(*ids, new_cap * sizeof(char *));
    if (!tmp)
      return CDD_C_ERROR_MEMORY;
    memset(tmp + *cap, 0, (new_cap - *cap) * sizeof(char *));
    *ids = tmp;
    *cap = new_cap;
  }
  (*ids)[*count] = (c_cdd_strdup(op_id, &_ast_strdup_292), _ast_strdup_292);
  if (!(*ids)[*count])
    return CDD_C_ERROR_MEMORY;
  (*count)++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Collects operation ids.
 */
cdd_c_error_t collect_operation_ids(const struct OpenAPI_Path *paths,
                                    size_t n_paths, char ***ids, size_t *count,
                                    size_t *cap) {
  size_t i, j = 0;
  cdd_c_error_t rc;
  (void)j;
  if (!paths)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_paths; ++i) {
    const struct OpenAPI_Path *p = &paths[i];
    for (j = 0; j < p->n_operations; ++j) {
      rc = add_unique_operation_id(ids, count, cap,
                                   p->operations[j].operation_id);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    for (j = 0; j < p->n_additional_operations; ++j) {
      rc = add_unique_operation_id(ids, count, cap,
                                   p->additional_operations[j].operation_id);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the path item ref matches component operation.
 */
cdd_c_error_t path_item_ref_matches_component(const struct OpenAPI_Spec *spec,
                                              const char *ref,
                                              const char *name) {
  char *_ast_ref_name_from_prefix_85 = NULL;
  char *_ast_json_pointer_unescape_86 = NULL;
  const char *name_enc;
  char *name_dec;
  int match = 0;
  if (!ref || !name)
    return CDD_C_SUCCESS;
  name_enc = (ref_name_from_prefix(spec, ref, "#/components/pathItems/",
                                   &_ast_ref_name_from_prefix_85),
              _ast_ref_name_from_prefix_85);
  if (!name_enc)
    return CDD_C_SUCCESS;
  name_dec = (json_pointer_unescape(name_enc, &_ast_json_pointer_unescape_86),
              _ast_json_pointer_unescape_86);
  if (!name_dec)
    return CDD_C_SUCCESS;
  match = (strcmp(name_dec, name) == 0);
  free(name_dec);
  return (cdd_c_error_t)match;
}

/**
 * @brief Executes the component path item is referenced operation.
 */
cdd_c_error_t component_path_item_is_referenced(const struct OpenAPI_Spec *spec,
                                                const char *name) {
  size_t i;
  if (!spec || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < spec->n_paths; ++i) {
    if (spec->paths[i].ref &&
        path_item_ref_matches_component(spec, spec->paths[i].ref, name))
      return CDD_C_ERROR_UNKNOWN;
  }
  for (i = 0; i < spec->n_webhooks; ++i) {
    if (spec->webhooks[i].ref &&
        path_item_ref_matches_component(spec, spec->webhooks[i].ref, name))
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the callback ref matches component operation.
 */
cdd_c_error_t callback_ref_matches_component(const struct OpenAPI_Spec *spec,
                                             const char *ref,
                                             const char *name) {
  char *_ast_ref_name_from_prefix_87 = NULL;
  char *_ast_json_pointer_unescape_88 = NULL;
  const char *name_enc;
  char *name_dec;
  int match = 0;
  if (!ref || !name)
    return CDD_C_SUCCESS;
  name_enc = (ref_name_from_prefix(spec, ref, "#/components/callbacks/",
                                   &_ast_ref_name_from_prefix_87),
              _ast_ref_name_from_prefix_87);
  if (!name_enc)
    return CDD_C_SUCCESS;
  name_dec = (json_pointer_unescape(name_enc, &_ast_json_pointer_unescape_88),
              _ast_json_pointer_unescape_88);
  if (!name_dec)
    return CDD_C_SUCCESS;
  match = (strcmp(name_dec, name) == 0);
  free(name_dec);
  return (cdd_c_error_t)match;
}

/**
 * @brief Executes the component callback is referenced in ops operation.
 */
cdd_c_error_t component_callback_is_referenced_in_ops(
    const struct OpenAPI_Operation *ops, size_t n_ops,
    const struct OpenAPI_Spec *spec, const char *name) {
  size_t i, j;
  if (!ops || !spec || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_ops; ++i) {
    const struct OpenAPI_Operation *op = &ops[i];
    for (j = 0; j < op->n_callbacks; ++j) {
      const struct OpenAPI_Callback *cb = &op->callbacks[j];
      if (cb->ref && callback_ref_matches_component(spec, cb->ref, name))
        return CDD_C_ERROR_UNKNOWN;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the component callback is referenced operation.
 */
cdd_c_error_t component_callback_is_referenced(const struct OpenAPI_Spec *spec,
                                               const char *name) {
  size_t i;
  if (!spec || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < spec->n_paths; ++i) {
    if (component_callback_is_referenced_in_ops(spec->paths[i].operations,
                                                spec->paths[i].n_operations,
                                                spec, name) ||
        component_callback_is_referenced_in_ops(
            spec->paths[i].additional_operations,
            spec->paths[i].n_additional_operations, spec, name)) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
  for (i = 0; i < spec->n_webhooks; ++i) {
    if (component_callback_is_referenced_in_ops(spec->webhooks[i].operations,
                                                spec->webhooks[i].n_operations,
                                                spec, name) ||
        component_callback_is_referenced_in_ops(
            spec->webhooks[i].additional_operations,
            spec->webhooks[i].n_additional_operations, spec, name)) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
  if (spec->component_path_items && spec->n_component_path_items > 0) {
    for (i = 0; i < spec->n_component_path_items; ++i) {
      const char *item_name = NULL;
      if (spec->component_path_item_names)
        item_name = spec->component_path_item_names[i];
      if (item_name && component_path_item_is_referenced(spec, item_name))
        continue;
      if (component_callback_is_referenced_in_ops(
              spec->component_path_items[i].operations,
              spec->component_path_items[i].n_operations, spec, name) ||
          component_callback_is_referenced_in_ops(
              spec->component_path_items[i].additional_operations,
              spec->component_path_items[i].n_additional_operations, spec,
              name)) {
        return CDD_C_ERROR_UNKNOWN;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Collects callback operation ids from callbacks.
 */
cdd_c_error_t collect_callback_operation_ids_from_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks, char ***ids,
    size_t *count, size_t *cap) {
  size_t i;
  if (!callbacks)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &callbacks[i];
    if (cb->paths && cb->n_paths > 0) {
      cdd_c_error_t rc =
          collect_operation_ids(cb->paths, cb->n_paths, ids, count, cap);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Collects callback operation ids from operations.
 */
cdd_c_error_t collect_callback_operation_ids_from_operations(
    const struct OpenAPI_Operation *ops, size_t n_ops, char ***ids,
    size_t *count, size_t *cap) {
  size_t i;
  if (!ops)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_ops; ++i) {
    const struct OpenAPI_Operation *op = &ops[i];
    cdd_c_error_t rc = collect_callback_operation_ids_from_callbacks(
        op->callbacks, op->n_callbacks, ids, count, cap);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Collects callback operation ids from paths.
 */
cdd_c_error_t
collect_callback_operation_ids_from_paths(const struct OpenAPI_Path *paths,
                                          size_t n_paths, char ***ids,
                                          size_t *count, size_t *cap) {
  size_t i;
  if (!paths)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_paths; ++i) {
    cdd_c_error_t rc = collect_callback_operation_ids_from_operations(
        paths[i].operations, paths[i].n_operations, ids, count, cap);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = collect_callback_operation_ids_from_operations(
        paths[i].additional_operations, paths[i].n_additional_operations, ids,
        count, cap);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate unique operation ids operation.
 */
cdd_c_error_t validate_unique_operation_ids(const struct OpenAPI_Spec *spec) {
  char **ids = NULL;
  size_t count = 0;
  size_t cap = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t i;

  if (!spec)
    return CDD_C_SUCCESS;

  rc = collect_operation_ids(spec->paths, spec->n_paths, &ids, &count, &cap);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = collect_operation_ids(spec->webhooks, spec->n_webhooks, &ids, &count,
                             &cap);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = collect_callback_operation_ids_from_paths(spec->paths, spec->n_paths,
                                                 &ids, &count, &cap);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = collect_callback_operation_ids_from_paths(
      spec->webhooks, spec->n_webhooks, &ids, &count, &cap);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (spec->component_path_items && spec->n_component_path_items > 0) {
    for (i = 0; i < spec->n_component_path_items; ++i) {
      const char *name = NULL;
      if (spec->component_path_item_names)
        name = spec->component_path_item_names[i];
      if (name && component_path_item_is_referenced(spec, name))
        continue;
      rc = collect_operation_ids(&spec->component_path_items[i], 1, &ids,
                                 &count, &cap);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = collect_callback_operation_ids_from_paths(
          &spec->component_path_items[i], 1, &ids, &count, &cap);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  if (spec->component_callbacks && spec->n_component_callbacks > 0) {
    for (i = 0; i < spec->n_component_callbacks; ++i) {
      const struct OpenAPI_Callback *cb = &spec->component_callbacks[i];
      const char *name = cb->name;
      if (name && component_callback_is_referenced(spec, name))
        continue;
      if (cb->paths && cb->n_paths > 0) {
        rc = collect_operation_ids(cb->paths, cb->n_paths, &ids, &count, &cap);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
  }

cleanup:
  for (i = 0; i < count; ++i) {
    free(ids[i]);
  }
  free(ids);
  return rc;
}
