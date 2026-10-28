/**
 * @file openapi_validation.c
 * @brief Querystring usage and unique operation ID validation.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Scans a parameter array for `in: querystring` and `in: query` usage.
 *
 * @param[in] params Array of parameters.
 * @param[in] n_params Number of parameters.
 * @param[in,out] qs_count Incremented for each `in: querystring` parameter.
 * @param[in,out] has_query Set to 1 if any `in: query` parameter is present.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
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
 * @brief Validates `in: querystring` parameter constraints across `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
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
 * @brief Validates `in: querystring` usage inside callback objects.
 *
 * @param[in] callbacks Array of callback objects.
 * @param[in] n_callbacks Number of callback objects.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
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
 * @brief Validates `in: querystring` usage inside operations' callbacks.
 *
 * @param[in] ops Array of operations.
 * @param[in] n_ops Number of operations.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
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
 * @brief Validates `in: querystring` usage in callbacks nested under `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
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
 * @brief Validates `in: querystring` usage in `spec->component_callbacks`.
 *
 * @param[in] spec OpenAPI specification.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
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
 * @brief Adds an `operationId` into a seen list, checking for duplicates.
 *
 * @param[in,out] ids Pointer to array of seen operation ID strings.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @param[in] op_id Operation ID string.
 * @return CDD_C_SUCCESS if unique, CDD_C_ERROR_INVALID_ARGUMENT if duplicate,
 * or CDD_C_ERROR_MEMORY on OOM.
 */
cdd_c_error_t add_unique_operation_id(char ***ids, size_t *count, size_t *cap,
                                      const char *op_id) {
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
  c_cdd_strdup(op_id, &(*ids)[*count]);
  if (!(*ids)[*count])
    return CDD_C_ERROR_MEMORY;
  (*count)++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Collects and validates uniqueness of `operationId`s across `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @param[in,out] ids Pointer to array of seen operation IDs.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @return CDD_C_SUCCESS on success, or error code on duplicate/OOM.
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
 * @brief Checks if a `$ref` matches a specific component path item name.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] ref Reference string.
 * @param[in] name Component path item name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if matches, CDD_C_SUCCESS (0)
 * otherwise.
 */
cdd_c_error_t path_item_ref_matches_component(const struct OpenAPI_Spec *spec,
                                              const char *ref,
                                              const char *name) {
  const char *name_enc = NULL;
  char *name_dec = NULL;
  int match = 0;
  if (!ref || !name)
    return CDD_C_SUCCESS;
  ref_name_from_prefix(spec, ref, "#/components/pathItems/", &name_enc);
  if (!name_enc)
    return CDD_C_SUCCESS;
  json_pointer_unescape(name_enc, &name_dec);
  if (!name_dec)
    return CDD_C_SUCCESS;
  match = (strcmp(name_dec, name) == 0);
  C_CDD_FREE(name_dec);
  return (cdd_c_error_t)match;
}

/**
 * @brief Checks if a component path item is referenced anywhere in `spec`.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] name Component path item name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if referenced, CDD_C_SUCCESS (0)
 * otherwise.
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
 * @brief Checks if a `$ref` matches a specific component callback name.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] ref Reference string.
 * @param[in] name Component callback name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if matches, CDD_C_SUCCESS (0)
 * otherwise.
 */
cdd_c_error_t callback_ref_matches_component(const struct OpenAPI_Spec *spec,
                                             const char *ref,
                                             const char *name) {
  const char *name_enc = NULL;
  char *name_dec = NULL;
  int match = 0;
  if (!ref || !name)
    return CDD_C_SUCCESS;
  ref_name_from_prefix(spec, ref, "#/components/callbacks/", &name_enc);
  if (!name_enc)
    return CDD_C_SUCCESS;
  json_pointer_unescape(name_enc, &name_dec);
  if (!name_dec)
    return CDD_C_SUCCESS;
  match = (strcmp(name_dec, name) == 0);
  C_CDD_FREE(name_dec);
  return (cdd_c_error_t)match;
}

/**
 * @brief Checks if a component callback is referenced in an array of
 * operations.
 *
 * @param[in] ops Array of operations.
 * @param[in] n_ops Number of operations.
 * @param[in] spec OpenAPI specification context.
 * @param[in] name Component callback name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if referenced, CDD_C_SUCCESS (0)
 * otherwise.
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
 * @brief Checks if a component callback is referenced anywhere in `spec`.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] name Component callback name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if referenced, CDD_C_SUCCESS (0)
 * otherwise.
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
 * @brief Collects and validates uniqueness of `operationId`s across callbacks.
 *
 * @param[in] callbacks Array of callback objects.
 * @param[in] n_callbacks Number of callback objects.
 * @param[in,out] ids Pointer to array of seen operation IDs.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @return CDD_C_SUCCESS on success, or error code on duplicate/OOM.
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
 * @brief Collects and validates uniqueness of `operationId`s across operations'
 * callbacks.
 *
 * @param[in] ops Array of operations.
 * @param[in] n_ops Number of operations.
 * @param[in,out] ids Pointer to array of seen operation IDs.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @return CDD_C_SUCCESS on success, or error code on duplicate/OOM.
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
 * @brief Collects and validates uniqueness of `operationId`s across callbacks
 * nested under `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @param[in,out] ids Pointer to array of seen operation IDs.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @return CDD_C_SUCCESS on success, or error code on duplicate/OOM.
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
 * @brief Validates that all `operationId` values across the entire `spec` are
 * unique.
 *
 * @param[in] spec OpenAPI specification.
 * @return CDD_C_SUCCESS if all operationIds are unique, or error code.
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
    C_CDD_FREE(ids[i]);
  }
  C_CDD_FREE(ids);
  return rc;
}
