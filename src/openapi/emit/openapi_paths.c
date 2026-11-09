/**
 * @file openapi_paths.c
 * @brief OpenAPI emitter operations and paths generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/json_dom.h"
#include "c_cdd/log.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
/* clang-format on */

/**
 * @brief Writes an OpenAPI callback object.
 *
 * @param[in,out] cb_obj The callback JSON object.
 * @param[in] cb The callback structure.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t
write_callback_object(JSON_Object *cb_obj, const struct OpenAPI_Callback *cb) {
  size_t i;
  cdd_c_error_t rc;

  if (!cb_obj || !cb)
    return CDD_C_SUCCESS;

  if (cb->ref) {
    g_cdd_last_alloc_err = cdd_json_object_set_string(cb_obj, "$ref", cb->ref);
    if (cb->summary)
      g_cdd_last_alloc_err =
          cdd_json_object_set_string(cb_obj, "summary", cb->summary);
    if (cb->description)
      g_cdd_last_alloc_err =
          cdd_json_object_set_string(cb_obj, "description", cb->description);
    return CDD_C_SUCCESS;
  }

  if (cb->paths && cb->n_paths > 0) {
    for (i = 0; i < cb->n_paths; ++i) {
      const struct OpenAPI_Path *p = &cb->paths[i];
      const char *route = p->route ? p->route : "callback";
      JSON_Value *item_val = NULL;
      JSON_Object *item_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&item_val);
      g_cdd_last_alloc_err = cdd_json_value_get_object(item_val, &item_obj);

      rc = write_path_item_object(item_obj, p);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(item_val);
        continue;
      }

      g_cdd_last_alloc_err =
          cdd_json_object_set_value_safe(cb_obj, route, item_val);
    }
  }

  if (cb->extensions_json)
    g_cdd_last_alloc_err =
        merge_schema_extras_object_openapi(cb_obj, cb->extensions_json);
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes callbacks map to operation JSON object.
 *
 * @param[in,out] op_obj Operation JSON object.
 * @param[in] op Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_callbacks(JSON_Object *op_obj,
                                           const struct OpenAPI_Operation *op) {
  JSON_Value *cbs_val = NULL;
  JSON_Object *cbs_obj = NULL;
  size_t i;

  if (!op_obj || !op || op->n_callbacks == 0 || !op->callbacks)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&cbs_val);
  if (!cbs_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  g_cdd_last_alloc_err = cdd_json_value_get_object(cbs_val, &cbs_obj);

  for (i = 0; i < op->n_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &op->callbacks[i];
    const char *name = cb->name ? cb->name : "callback";
    JSON_Value *cb_val = NULL;
    JSON_Object *cb_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&cb_val);
    g_cdd_last_alloc_err = cdd_json_value_get_object(cb_val, &cb_obj);

    g_cdd_last_alloc_err = write_callback_object(cb_obj, cb);
    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(cbs_obj, name, cb_val);
  }

  g_cdd_last_alloc_err =
      cdd_json_object_set_value_safe(op_obj, "callbacks", cbs_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes responses map to operation JSON object.
 *
 * @param[in,out] op_obj Operation JSON object.
 * @param[in] op Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_responses(JSON_Object *op_obj,
                                           const struct OpenAPI_Operation *op) {
  JSON_Value *resps_val = NULL;
  JSON_Object *resps_obj = NULL;
  size_t i;

  if (!op_obj || !op)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&resps_val);
  if (!resps_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  g_cdd_last_alloc_err = cdd_json_value_get_object(resps_val, &resps_obj);

  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *r = &op->responses[i];
    JSON_Value *r_val = NULL;
    JSON_Object *r_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&r_val);
    if (!r_val) {
      g_cdd_last_alloc_err = cdd_json_value_free(resps_val);
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    g_cdd_last_alloc_err = cdd_json_value_get_object(r_val, &r_obj);

    g_cdd_last_alloc_err = write_response_object(r_obj, r);

    g_cdd_last_alloc_err = cdd_json_object_set_value_safe(
        resps_obj, r->code ? r->code : "default", r_val);
  }
  if (op->responses_extensions_json)
    g_cdd_last_alloc_err = merge_schema_extras_object_openapi(
        resps_obj, op->responses_extensions_json);

  g_cdd_last_alloc_err =
      cdd_json_object_set_value_safe(op_obj, "responses", resps_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes operation object fields.
 *
 * @param[in,out] op_obj Operation JSON object.
 * @param[in] op Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_operation_object(
    JSON_Object *op_obj, const struct OpenAPI_Operation *op) {
  cdd_c_error_t rc;

  if (!op_obj || !op)
    return CDD_C_SUCCESS;

  if (op->operation_id) {
    g_cdd_last_alloc_err =
        cdd_json_object_set_string(op_obj, "operationId", op->operation_id);
  }
  if (op->summary) {
    g_cdd_last_alloc_err =
        cdd_json_object_set_string(op_obj, "summary", op->summary);
  }
  if (op->description) {
    g_cdd_last_alloc_err =
        cdd_json_object_set_string(op_obj, "description", op->description);
  }
  if (op->external_docs.url) {
    g_cdd_last_alloc_err =
        write_external_docs(op_obj, "externalDocs", &op->external_docs);
  }
  if (op->deprecated) {
    g_cdd_last_alloc_err = cdd_json_object_set_boolean(op_obj, "deprecated", 1);
  }
  rc = write_security_requirements(op_obj, "security", op->security,
                                   op->n_security, op->security_set);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  if (op->n_tags > 0) {
    JSON_Value *tags_val = NULL;
    JSON_Array *tags_arr = NULL;
    size_t k;
    g_cdd_last_alloc_err = cdd_json_value_init_array(&tags_val);
    g_cdd_last_alloc_err = cdd_json_value_get_array(tags_val, &tags_arr);
    for (k = 0; k < op->n_tags; ++k) {
      g_cdd_last_alloc_err =
          cdd_json_array_append_string(tags_arr, op->tags[k]);
    }
    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(op_obj, "tags", tags_val);
  }

  rc = write_parameters(op_obj, op->parameters, op->n_parameters);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  rc = write_request_body(op_obj, op);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  rc = write_responses(op_obj, op);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  rc = write_callbacks(op_obj, op);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  if (op->n_servers > 0 && op->servers) {
    rc = write_server_array(op_obj, "servers", op->servers, op->n_servers);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  }

  if (op->extensions_json)
    g_cdd_last_alloc_err =
        merge_schema_extras_object_openapi(op_obj, op->extensions_json);

  return CDD_C_SUCCESS;
}

/**
 * @brief Writes operations to path item JSON object.
 *
 * @param[in,out] path_item Path item JSON object.
 * @param[in] path Path structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_operations(JSON_Object *path_item,
                                            const struct OpenAPI_Path *path) {
  size_t i;
  cdd_c_error_t rc;

  for (i = 0; i < path->n_operations; ++i) {
    const struct OpenAPI_Operation *op = &path->operations[i];
    char *verb = NULL;
    JSON_Value *op_val = NULL;
    JSON_Object *op_obj = NULL;

    g_cdd_last_alloc_err = verb_to_str_openapi(op->verb, &verb);
    if (!verb)
      continue;

    g_cdd_last_alloc_err = cdd_json_value_init_object(&op_val);
    g_cdd_last_alloc_err = cdd_json_value_get_object(op_val, &op_obj);

    rc = write_operation_object(op_obj, op);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(op_val);
      return rc;
    }

    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(path_item, verb, op_val);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes additional operations to path item JSON object.
 *
 * @param[in,out] path_item Path item JSON object.
 * @param[in] path Path structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_additional_operations(
    JSON_Object *path_item, const struct OpenAPI_Path *path) {
  JSON_Value *add_val = NULL;
  JSON_Object *add_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!path_item || !path || path->n_additional_operations == 0 ||
      !path->additional_operations)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&add_val);
  if (!add_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  g_cdd_last_alloc_err = cdd_json_value_get_object(add_val, &add_obj);

  for (i = 0; i < path->n_additional_operations; ++i) {
    const struct OpenAPI_Operation *op = &path->additional_operations[i];
    char *verb_fallback = NULL;
    const char *method;
    JSON_Value *op_val = NULL;
    JSON_Object *op_obj = NULL;

    if (!op->method)
      g_cdd_last_alloc_err = verb_to_str_openapi(op->verb, &verb_fallback);
    method = op->method ? op->method : verb_fallback;

    if (!method)
      continue;

    g_cdd_last_alloc_err = cdd_json_value_init_object(&op_val);
    g_cdd_last_alloc_err = cdd_json_value_get_object(op_val, &op_obj);

    rc = write_operation_object(op_obj, op);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(op_val);
      g_cdd_last_alloc_err = cdd_json_value_free(add_val);
      return rc;
    }

    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(add_obj, method, op_val);
  }

  g_cdd_last_alloc_err = cdd_json_object_set_value_safe(
      path_item, "additionalOperations", add_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes path item object.
 *
 * @param[in,out] item_obj Path item JSON object.
 * @param[in] path Path structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
write_path_item_object(JSON_Object *item_obj, const struct OpenAPI_Path *path) {
  cdd_c_error_t rc;

  if (!item_obj || !path)
    return CDD_C_SUCCESS;

  if (path->summary)
    g_cdd_last_alloc_err =
        cdd_json_object_set_string(item_obj, "summary", path->summary);
  if (path->description)
    g_cdd_last_alloc_err =
        cdd_json_object_set_string(item_obj, "description", path->description);
  if (path->ref)
    g_cdd_last_alloc_err =
        cdd_json_object_set_string(item_obj, "$ref", path->ref);
  if (path->n_parameters > 0) {
    rc = write_parameters(item_obj, path->parameters, path->n_parameters);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (path->n_servers > 0 && path->servers) {
    rc =
        write_server_array(item_obj, "servers", path->servers, path->n_servers);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = write_operations(item_obj, path);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = write_additional_operations(item_obj, path);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (path->extensions_json)
    g_cdd_last_alloc_err =
        merge_schema_extras_object_openapi(item_obj, path->extensions_json);

  return CDD_C_SUCCESS;
}

/**
 * @brief Writes paths object to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_paths(JSON_Object *root_obj,
                                       const struct OpenAPI_Spec *spec) {
  JSON_Value *paths_val = NULL;
  JSON_Object *paths_obj = NULL;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&paths_val);
  g_cdd_last_alloc_err = cdd_json_value_get_object(paths_val, &paths_obj);

  if (!spec) {
    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(root_obj, "paths", paths_val);
    return rc;
  }

  if (spec->paths_extensions_json) {
    g_cdd_last_alloc_err = merge_schema_extras_object_openapi(
        paths_obj, spec->paths_extensions_json);
  }

  for (i = 0; i < spec->n_paths; ++i) {
    const struct OpenAPI_Path *p = &spec->paths[i];
    const char *route = p->route ? p->route : "/";
    JSON_Value *item_val = NULL;
    JSON_Object *item_obj = NULL;
    int is_new = 0;
    int has_val = 0;

    g_cdd_last_alloc_err =
        cdd_json_object_has_value(paths_obj, route, &has_val);
    if (has_val) {
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(paths_obj, route, &item_obj);
    } else {
      g_cdd_last_alloc_err = cdd_json_value_init_object(&item_val);
      g_cdd_last_alloc_err = cdd_json_value_get_object(item_val, &item_obj);
      is_new = 1;
    }

    rc = write_path_item_object(item_obj, p);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(item_val);
      break;
    }

    if (is_new) {
      g_cdd_last_alloc_err =
          cdd_json_object_set_value_safe(paths_obj, route, item_val);
    }
  }

  if (rc == CDD_C_SUCCESS) {
    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(root_obj, "paths", paths_val);
  } else {
    g_cdd_last_alloc_err = cdd_json_value_free(paths_val);
  }
  return rc;
}

/**
 * @brief Writes webhooks object to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_webhooks(JSON_Object *root_obj,
                                          const struct OpenAPI_Spec *spec) {
  JSON_Value *hooks_val = NULL;
  JSON_Object *hooks_obj = NULL;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!spec)
    return CDD_C_SUCCESS;
  if ((spec->n_webhooks == 0 || !spec->webhooks) &&
      !spec->webhooks_extensions_json)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&hooks_val);
  if (!hooks_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  g_cdd_last_alloc_err = cdd_json_value_get_object(hooks_val, &hooks_obj);

  if (spec->webhooks_extensions_json) {
    g_cdd_last_alloc_err = merge_schema_extras_object_openapi(
        hooks_obj, spec->webhooks_extensions_json);
  }

  for (i = 0; i < spec->n_webhooks; ++i) {
    const struct OpenAPI_Path *p = &spec->webhooks[i];
    const char *route = p->route ? p->route : "webhook";
    JSON_Value *item_val = NULL;
    JSON_Object *item_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&item_val);
    g_cdd_last_alloc_err = cdd_json_value_get_object(item_val, &item_obj);

    rc = write_path_item_object(item_obj, p);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(item_val);
      break;
    }

    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(hooks_obj, route, item_val);
  }

  if (rc == CDD_C_SUCCESS) {
    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(root_obj, "webhooks", hooks_val);
  } else {
    g_cdd_last_alloc_err = cdd_json_value_free(hooks_val);
  }
  return rc;
}

/**
 * @brief Writes security requirements array to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] sets Array of security requirement sets.
 * @param[in] count Number of sets.
 * @param[in] set_flag Flag indicating if security requirements are set.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
write_security_requirements(JSON_Object *parent, const char *key,
                            const struct OpenAPI_SecurityRequirementSet *sets,
                            size_t count, int set_flag) {
  JSON_Value *arr_val = NULL;
  JSON_Array *arr = NULL;
  size_t i;

  if (!parent || !key || !set_flag)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_array(&arr_val);
  if (!arr_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  g_cdd_last_alloc_err = cdd_json_value_get_array(arr_val, &arr);

  if (count == 0) {
    g_cdd_last_alloc_err = cdd_json_object_set_value_safe(parent, key, arr_val);
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < count; ++i) {
    const struct OpenAPI_SecurityRequirementSet *set = &sets[i];
    JSON_Value *set_val = NULL;
    JSON_Object *set_obj = NULL;
    size_t j;

    g_cdd_last_alloc_err = cdd_json_value_init_object(&set_val);
    if (!set_val) {
      g_cdd_last_alloc_err = cdd_json_value_free(arr_val);
      return CDD_C_ERROR_MEMORY;
    }
    g_cdd_last_alloc_err = cdd_json_value_get_object(set_val, &set_obj);

    for (j = 0; j < set->n_requirements; ++j) {
      const struct OpenAPI_SecurityRequirement *req = &set->requirements[j];
      JSON_Value *scopes_val = NULL;
      JSON_Array *scopes_arr = NULL;
      size_t k;

      g_cdd_last_alloc_err = cdd_json_value_init_array(&scopes_val);
      if (!scopes_val) {
        g_cdd_last_alloc_err = cdd_json_value_free(set_val);
        g_cdd_last_alloc_err = cdd_json_value_free(arr_val);
        return CDD_C_ERROR_MEMORY;
      }
      g_cdd_last_alloc_err = cdd_json_value_get_array(scopes_val, &scopes_arr);

      for (k = 0; k < req->n_scopes; ++k) {
        g_cdd_last_alloc_err =
            cdd_json_array_append_string(scopes_arr, req->scopes[k]);
      }

      g_cdd_last_alloc_err = cdd_json_object_set_value_safe(
          set_obj, req->scheme ? req->scheme : "", scopes_val);
    }

    if (set->extensions_json)
      g_cdd_last_alloc_err =
          merge_schema_extras_object_openapi(set_obj, set->extensions_json);

    g_cdd_last_alloc_err = cdd_json_array_append_value_safe(arr, set_val);
  }

  g_cdd_last_alloc_err = cdd_json_object_set_value_safe(parent, key, arr_val);
  return CDD_C_SUCCESS;
}
