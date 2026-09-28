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

#include <parson.h>

#include "c_cdd/log.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
/* clang-format on */

/**
 * @brief Executes the openapi write spec to json operation.
 */
cdd_c_error_t openapi_write_spec_to_json(const struct OpenAPI_Spec *spec,
                                         char **json_out) {
  char *_ast_strdup_4 = NULL;
  JSON_Value *root_val;
  JSON_Object *root_obj;
  cdd_c_error_t rc;

  if (!spec || !json_out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (spec->is_schema_document) {
    if (!spec->schema_root_json)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    *json_out =
        (c_cdd_strdup(spec->schema_root_json, &_ast_strdup_4), _ast_strdup_4);
    return *json_out ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }
  if (license_fields_invalid(&spec->info.license))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  root_val = json_value_init_object();
  if (!root_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  root_obj = json_value_get_object(root_val);

  json_object_set_string(root_obj, "openapi",
                         spec->openapi_version ? spec->openapi_version
                                               : "3.2.0");
  if (spec->self_uri)
    json_object_set_string(root_obj, "$self", spec->self_uri);
  if (spec->json_schema_dialect)
    json_object_set_string(root_obj, "jsonSchemaDialect",
                           spec->json_schema_dialect);
  if (spec->extensions_json)
    merge_schema_extras_object_openapi(root_obj, spec->extensions_json);
  write_info(root_obj, spec);
  if (spec->external_docs.url)
    write_external_docs(root_obj, "externalDocs", &spec->external_docs);
  rc = write_tags(root_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(root_val);
    return rc;
  }

  rc = write_security_requirements(root_obj, "security", spec->security,
                                   spec->n_security, spec->security_set);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(root_val);
    return rc;
  }

  rc = write_servers(root_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(root_val);
    return rc;
  }

  rc = write_components(root_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(root_val);
    return rc;
  }

  rc = write_webhooks(root_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(root_val);
    return rc;
  }

  if (spec->n_paths > 0 || spec->paths_extensions_json) {
    rc = write_paths(root_obj, spec);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(root_val);
      return rc;
    }
  } else {
    JSON_Value *empty = json_value_init_object();
    json_object_set_value(root_obj, "paths", empty);
  }

  *json_out = json_serialize_to_string_pretty(root_val);
  json_value_free(root_val);

  return *json_out ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}
