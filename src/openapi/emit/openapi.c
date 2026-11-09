/**
 * @file openapi.c
 * @brief Implementation of OpenAPI generation entry point.
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
 * @brief Safely sets a JSON object property and frees the value on failure.
 *
 * @param[in,out] obj The target JSON object.
 * @param[in] name The property key name.
 * @param[in] val The JSON value to set (transferred or freed).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on failure.
 */
cdd_c_error_t cdd_json_object_set_value_safe(JSON_Object *obj, const char *name,
                                             JSON_Value *val) {
  cdd_c_error_t rc = cdd_json_object_set_value(obj, name, val);
  if (rc != CDD_C_SUCCESS) {
    cdd_c_error_t rc_free = cdd_json_value_free(val);
    return (cdd_c_error_t)((int)rc | (int)rc_free);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Safely appends a JSON array element and frees the value on failure.
 *
 * @param[in,out] arr The target JSON array.
 * @param[in] val The JSON value to append (transferred or freed).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_json_array_append_value_safe(JSON_Array *arr,
                                               JSON_Value *val) {
  cdd_c_error_t rc = cdd_json_array_append_value(arr, val);
  if (rc != CDD_C_SUCCESS) {
    cdd_c_error_t rc_free = cdd_json_value_free(val);
    return (cdd_c_error_t)((int)rc | (int)rc_free);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Serialize an OpenAPI Spec structure to a JSON string.
 *
 * Converts the full specification tree (Paths, Operations, Parameters,
 * Responses, and Components) into a formatted JSON string.
 *
 * The caller is responsible for freeing the output string.
 *
 * @param[in] spec The specification structure to serialize.
 * @param[out] json_out Pointer to a char* where the allocated JSON string will
 * be stored.
 * @return 0 on success, EINVAL if inputs are invalid, ENOMEM on allocation
 * failure.
 */
cdd_c_error_t openapi_write_spec_to_json(const struct OpenAPI_Spec *spec,
                                         char **json_out) {
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t rc_free;

  if (!spec || !json_out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (spec->is_schema_document) {
    if (!spec->schema_root_json)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = c_cdd_strdup(spec->schema_root_json, json_out);
    return (rc == CDD_C_SUCCESS) ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }
  {
    int lic_invalid = 0;
    g_cdd_last_alloc_err =
        license_fields_invalid(&spec->info.license, &lic_invalid);
    if (lic_invalid)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_json_value_init_object(&root_val);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(root_val, &root_obj);

  if (spec->swagger_version) {
    rc = cdd_json_object_set_string(root_obj, "swagger", spec->swagger_version);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_json_value_free(root_val);
      return rc;
    }
    if (spec->host) {
      rc = cdd_json_object_set_string(root_obj, "host", spec->host);
      if (rc != CDD_C_SUCCESS) {
        rc_free = cdd_json_value_free(root_val);
        return rc;
      }
    }
    if (spec->basePath) {
      rc = cdd_json_object_set_string(root_obj, "basePath", spec->basePath);
      if (rc != CDD_C_SUCCESS) {
        rc_free = cdd_json_value_free(root_val);
        return rc;
      }
    }
  } else {
    rc = cdd_json_object_set_string(
        root_obj, "openapi",
        spec->openapi_version ? spec->openapi_version : "3.2.0");
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_json_value_free(root_val);
      return rc;
    }
  }
  if (spec->self_uri) {
    rc = cdd_json_object_set_string(root_obj, "$self", spec->self_uri);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_json_value_free(root_val);
      return rc;
    }
  }
  if (spec->json_schema_dialect) {
    rc = cdd_json_object_set_string(root_obj, "jsonSchemaDialect",
                                    spec->json_schema_dialect);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_json_value_free(root_val);
      return rc;
    }
  }
  if (spec->extensions_json) {
    rc = merge_schema_extras_object_openapi(root_obj, spec->extensions_json);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_json_value_free(root_val);
      return rc;
    }
  }

  rc = write_info(root_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    rc_free = cdd_json_value_free(root_val);
    return rc;
  }
  if (spec->external_docs.url) {
    rc = write_external_docs(root_obj, "externalDocs", &spec->external_docs);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_json_value_free(root_val);
      return rc;
    }
  }

  rc = write_tags(root_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    rc_free = cdd_json_value_free(root_val);
    return rc;
  }

  rc = write_security_requirements(root_obj, "security", spec->security,
                                   spec->n_security, spec->security_set);
  if (rc != CDD_C_SUCCESS) {
    rc_free = cdd_json_value_free(root_val);
    return rc;
  }

  rc = write_servers(root_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    rc_free = cdd_json_value_free(root_val);
    return rc;
  }

  rc = write_components(root_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    rc_free = cdd_json_value_free(root_val);
    return rc;
  }

  rc = write_webhooks(root_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    rc_free = cdd_json_value_free(root_val);
    return rc;
  }

  if (spec->n_paths > 0 || spec->paths_extensions_json) {
    rc = write_paths(root_obj, spec);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_json_value_free(root_val);
      return rc;
    }
  } else {
    JSON_Value *empty = NULL;
    rc = cdd_json_value_init_object(&empty);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_json_value_free(root_val);
      return rc;
    }
    rc = cdd_json_object_set_value_safe(root_obj, "paths", empty);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_json_value_free(root_val);
      return rc;
    }
  }

  *json_out = NULL;
  rc = cdd_json_serialize_to_string_pretty(root_val, json_out);
  rc_free = cdd_json_value_free(root_val);

  return (rc == CDD_C_SUCCESS) ? rc_free : CDD_C_ERROR_MEMORY;
}
