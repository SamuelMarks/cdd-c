/**
 * @file test_openapi_writer_common.h
 * @brief Common test fixtures and helpers for OpenAPI Writer unit tests.
 */

#ifndef TEST_OPENAPI_WRITER_COMMON_H
#define TEST_OPENAPI_WRITER_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <parson.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

/* Global OOM injection state and mock allocator */
static int g_parson_oom_fail_at = -1;
static void *mock_parson_oom_malloc(size_t sz) {
  if (g_parson_oom_fail_at == 0) {
    g_parson_oom_fail_at = -1;
    return NULL;
  }
  if (g_parson_oom_fail_at > 0)
    g_parson_oom_fail_at--;
  return malloc(sz);
}
static void mock_parson_oom_free(void *ptr) { free(ptr); }

/* --- Helpers --- */

static cdd_c_error_t load_spec_str2(const char *json_str,
                                    struct OpenAPI_Spec *spec) {
  JSON_Value *dyn;
  cdd_c_error_t rc;
  dyn = json_parse_string(json_str);
  if (!dyn)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = openapi_spec_init(spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(dyn);
    return rc;
  }
  rc = openapi_load_from_json(dyn, spec);
  json_value_free(dyn);
  return rc;
}

static void setup_test_spec(struct OpenAPI_Spec *spec,
                            struct OpenAPI_Path *path,
                            struct OpenAPI_Operation *op,
                            struct OpenAPI_Parameter *param,
                            struct OpenAPI_Response *response) {
  /* Zero out stack structs */
  memset(spec, 0, sizeof(*spec));
  if (path)
    memset(path, 0, sizeof(*path));
  if (op)
    memset(op, 0, sizeof(*op));
  if (param)
    memset(param, 0, sizeof(*param));
  if (response)
    memset(response, 0, sizeof(*response));

  if (path) {
    spec->paths = path;
    spec->n_paths = 1;
    path->route = (char *)(size_t)(size_t) "/test/route";
    if (op) {
      path->operations = op;
      path->n_operations = 1;
      op->verb = OA_VERB_GET;
      op->operation_id = (char *)(size_t)(size_t) "testOp";
      if (param) {
        op->parameters = param;
        op->n_parameters = 1;
        param->name = (char *)(size_t)(size_t) "p1";
        param->in = OA_PARAM_IN_QUERY;
        param->type = (char *)(size_t)(size_t) "string";
      }
      if (response) {
        op->responses = response;
        op->n_responses = 1;
        response->code = (char *)(size_t)(size_t) "200";
        response->schema.ref_name = (char *)(size_t)(size_t) "TestModel";
      }
    }
  }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_COMMON_H */
