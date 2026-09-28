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

#include <parson.h>

#include "c_cdd/log.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
/* clang-format on */

/**
 * @brief Generates C code for write callback object.
 */
C_CDD_EXPORT void write_callback_object(JSON_Object *cb_obj,
                                        const struct OpenAPI_Callback *cb) {
  size_t i;
  cdd_c_error_t rc;

  if (!cb_obj || !cb)
    return;

  if (cb->ref) {
    json_object_set_string(cb_obj, "$ref", cb->ref);
    if (cb->summary)
      json_object_set_string(cb_obj, "summary", cb->summary);
    if (cb->description)
      json_object_set_string(cb_obj, "description", cb->description);
    return;
  }

  if (cb->paths && cb->n_paths > 0) {
    for (i = 0; i < cb->n_paths; ++i) {
      const struct OpenAPI_Path *p = &cb->paths[i];
      const char *route = p->route ? p->route : "callback";
      JSON_Value *item_val = json_value_init_object();
      JSON_Object *item_obj = json_value_get_object(item_val);

      rc = write_path_item_object(item_obj, p);
      if (rc != CDD_C_SUCCESS) {
        json_value_free(item_val);
        continue;
      }

      json_object_set_value(cb_obj, route, item_val);
    }
  }

  if (cb->extensions_json)
    merge_schema_extras_object_openapi(cb_obj, cb->extensions_json);
}

/**
 * @brief Generates C code for write callbacks.
 */
C_CDD_EXPORT cdd_c_error_t write_callbacks(JSON_Object *op_obj,
                                           const struct OpenAPI_Operation *op) {
  JSON_Value *cbs_val;
  JSON_Object *cbs_obj;
  size_t i;

  if (!op_obj || !op || op->n_callbacks == 0 || !op->callbacks)
    return CDD_C_SUCCESS;

  cbs_val = json_value_init_object();
  if (!cbs_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  cbs_obj = json_value_get_object(cbs_val);

  for (i = 0; i < op->n_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &op->callbacks[i];
    const char *name = cb->name ? cb->name : "callback";
    JSON_Value *cb_val = json_value_init_object();
    JSON_Object *cb_obj = json_value_get_object(cb_val);

    write_callback_object(cb_obj, cb);
    json_object_set_value(cbs_obj, name, cb_val);
  }

  json_object_set_value(op_obj, "callbacks", cbs_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write responses.
 */
C_CDD_EXPORT cdd_c_error_t write_responses(JSON_Object *op_obj,
                                           const struct OpenAPI_Operation *op) {
  JSON_Value *resps_val;
  JSON_Object *resps_obj;
  size_t i;

  if (!op_obj || !op)
    return CDD_C_SUCCESS;

  resps_val = json_value_init_object();
  if (!resps_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  resps_obj = json_value_get_object(resps_val);

  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *r = &op->responses[i];
    JSON_Value *r_val = json_value_init_object();
    JSON_Object *r_obj = json_value_get_object(r_val);
    if (!r_val) {
      json_value_free(resps_val);
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }

    write_response_object(r_obj, r);

    json_object_set_value(resps_obj, r->code ? r->code : "default", r_val);
  }
  if (op->responses_extensions_json)
    merge_schema_extras_object_openapi(resps_obj,
                                       op->responses_extensions_json);

  json_object_set_value(op_obj, "responses", resps_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write operation object.
 */
C_CDD_EXPORT cdd_c_error_t write_operation_object(
    JSON_Object *op_obj, const struct OpenAPI_Operation *op) {
  cdd_c_error_t rc;

  if (!op_obj || !op)
    return CDD_C_SUCCESS;

  if (op->operation_id) {
    json_object_set_string(op_obj, "operationId", op->operation_id);
  }
  if (op->summary) {
    json_object_set_string(op_obj, "summary", op->summary);
  }
  if (op->description) {
    json_object_set_string(op_obj, "description", op->description);
  }
  if (op->external_docs.url) {
    write_external_docs(op_obj, "externalDocs", &op->external_docs);
  }
  if (op->deprecated) {
    json_object_set_boolean(op_obj, "deprecated", 1);
  }
  rc = write_security_requirements(op_obj, "security", op->security,
                                   op->n_security, op->security_set);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  if (op->n_tags > 0) {
    JSON_Value *tags_val = json_value_init_array();
    JSON_Array *tags_arr = json_value_get_array(tags_val);
    size_t k;
    for (k = 0; k < op->n_tags; ++k) {
      json_array_append_string(tags_arr, op->tags[k]);
    }
    json_object_set_value(op_obj, "tags", tags_val);
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
    merge_schema_extras_object_openapi(op_obj, op->extensions_json);

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write operations.
 */
C_CDD_EXPORT cdd_c_error_t write_operations(JSON_Object *path_item,
                                            const struct OpenAPI_Path *path) {
  char *_ast_verb_to_str_20 = NULL;
  size_t i;
  cdd_c_error_t rc;

  for (i = 0; i < path->n_operations; ++i) {
    const struct OpenAPI_Operation *op = &path->operations[i];
    const char *verb = (verb_to_str_openapi(op->verb, &_ast_verb_to_str_20),
                        _ast_verb_to_str_20);
    JSON_Value *op_val;
    JSON_Object *op_obj;

    if (!verb)
      continue;

    op_val = json_value_init_object();
    op_obj = json_value_get_object(op_val);

    rc = write_operation_object(op_obj, op);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(op_val);
      return rc;
    }

    json_object_set_value(path_item, verb, op_val);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write additional operations.
 */
C_CDD_EXPORT cdd_c_error_t write_additional_operations(
    JSON_Object *path_item, const struct OpenAPI_Path *path) {
  char *_ast_verb_to_str_21 = NULL;
  JSON_Value *add_val;
  JSON_Object *add_obj;
  size_t i;
  cdd_c_error_t rc;

  if (!path_item || !path || path->n_additional_operations == 0 ||
      !path->additional_operations)
    return CDD_C_SUCCESS;

  add_val = json_value_init_object();
  if (!add_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  add_obj = json_value_get_object(add_val);

  for (i = 0; i < path->n_additional_operations; ++i) {
    const struct OpenAPI_Operation *op = &path->additional_operations[i];
    const char *method =
        op->method ? op->method
                   : (verb_to_str_openapi(op->verb, &_ast_verb_to_str_21),
                      _ast_verb_to_str_21);
    JSON_Value *op_val;
    JSON_Object *op_obj;

    if (!method)
      continue;

    op_val = json_value_init_object();
    op_obj = json_value_get_object(op_val);

    rc = write_operation_object(op_obj, op);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(op_val);
      json_value_free(add_val);
      return rc;
    }

    json_object_set_value(add_obj, method, op_val);
  }

  json_object_set_value(path_item, "additionalOperations", add_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write path item object.
 */
C_CDD_EXPORT cdd_c_error_t
write_path_item_object(JSON_Object *item_obj, const struct OpenAPI_Path *path) {
  cdd_c_error_t rc;

  if (!item_obj || !path)
    return CDD_C_SUCCESS;

  if (path->summary)
    json_object_set_string(item_obj, "summary", path->summary);
  if (path->description)
    json_object_set_string(item_obj, "description", path->description);
  if (path->ref)
    json_object_set_string(item_obj, "$ref", path->ref);
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
    merge_schema_extras_object_openapi(item_obj, path->extensions_json);

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write paths.
 */
C_CDD_EXPORT cdd_c_error_t write_paths(JSON_Object *root_obj,
                                       const struct OpenAPI_Spec *spec) {
  JSON_Value *paths_val = json_value_init_object();
  JSON_Object *paths_obj = json_value_get_object(paths_val);
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!spec) {
    json_object_set_value(root_obj, "paths", paths_val);
    return rc;
  }

  if (spec->paths_extensions_json) {
    merge_schema_extras_object_openapi(paths_obj, spec->paths_extensions_json);
  }

  for (i = 0; i < spec->n_paths; ++i) {
    const struct OpenAPI_Path *p = &spec->paths[i];
    const char *route = p->route ? p->route : "/";
    JSON_Value *item_val;
    JSON_Object *item_obj;

    if (json_object_has_value(paths_obj, route)) {
      item_obj = json_object_get_object(paths_obj, route);
    } else {
      item_val = json_value_init_object();
      item_obj = json_value_get_object(item_val);
      json_object_set_value(paths_obj, route, item_val);
    }

    rc = write_path_item_object(item_obj, p);
    if (rc != CDD_C_SUCCESS)
      break;
  }

  if (rc == CDD_C_SUCCESS) {
    json_object_set_value(root_obj, "paths", paths_val);
  } else {
    json_value_free(paths_val);
  }
  return rc;
}

/**
 * @brief Generates C code for write webhooks.
 */
C_CDD_EXPORT cdd_c_error_t write_webhooks(JSON_Object *root_obj,
                                          const struct OpenAPI_Spec *spec) {
  JSON_Value *hooks_val;
  JSON_Object *hooks_obj;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!spec)
    return CDD_C_SUCCESS;
  if ((spec->n_webhooks == 0 || !spec->webhooks) &&
      !spec->webhooks_extensions_json)
    return CDD_C_SUCCESS;

  hooks_val = json_value_init_object();
  if (!hooks_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  hooks_obj = json_value_get_object(hooks_val);

  if (spec->webhooks_extensions_json) {
    merge_schema_extras_object_openapi(hooks_obj,
                                       spec->webhooks_extensions_json);
  }

  for (i = 0; i < spec->n_webhooks; ++i) {
    const struct OpenAPI_Path *p = &spec->webhooks[i];
    const char *route = p->route ? p->route : "webhook";
    JSON_Value *item_val = json_value_init_object();
    JSON_Object *item_obj = json_value_get_object(item_val);

    rc = write_path_item_object(item_obj, p);
    if (rc != CDD_C_SUCCESS)
      break;

    json_object_set_value(hooks_obj, route, item_val);
  }

  if (rc == CDD_C_SUCCESS) {
    json_object_set_value(root_obj, "webhooks", hooks_val);
  } else {
    json_value_free(hooks_val);
  }
  return rc;
}

/**
 * @brief Generates C code for write security requirements.
 */
C_CDD_EXPORT cdd_c_error_t
write_security_requirements(JSON_Object *parent, const char *key,
                            const struct OpenAPI_SecurityRequirementSet *sets,
                            size_t count, int set_flag) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (!parent || !key || !set_flag)
    return CDD_C_SUCCESS;

  arr_val = json_value_init_array();
  if (!arr_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  arr = json_value_get_array(arr_val);

  if (count == 0) {
    json_object_set_value(parent, key, arr_val);
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < count; ++i) {
    const struct OpenAPI_SecurityRequirementSet *set = &sets[i];
    JSON_Value *set_val = json_value_init_object();
    JSON_Object *set_obj = json_value_get_object(set_val);
    size_t j;

    if (!set_val) {
      json_value_free(arr_val);
      return CDD_C_ERROR_MEMORY;
    }

    for (j = 0; j < set->n_requirements; ++j) {
      const struct OpenAPI_SecurityRequirement *req = &set->requirements[j];
      JSON_Value *scopes_val = json_value_init_array();
      JSON_Array *scopes_arr = json_value_get_array(scopes_val);
      size_t k;

      if (!scopes_val) {
        json_value_free(set_val);
        json_value_free(arr_val);
        return CDD_C_ERROR_MEMORY;
      }

      for (k = 0; k < req->n_scopes; ++k) {
        json_array_append_string(scopes_arr, req->scopes[k]);
      }

      json_object_set_value(set_obj, req->scheme ? req->scheme : "",
                            scopes_val);
    }

    if (set->extensions_json)
      merge_schema_extras_object_openapi(set_obj, set->extensions_json);

    json_array_append_value(arr, set_val);
  }

  json_object_set_value(parent, key, arr_val);
  return CDD_C_SUCCESS;
}
