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
#include "routes/parse/cli.h"
/* clang-format on */

/* Global OOM injection state and mock allocator */
/** @brief Test definition g_parson_oom_fail_at. */
static int g_parson_oom_fail_at = -1;
/**
 * @brief Helper function mock_parson_oom_malloc.
 * @param[in] sz Parameter sz.
 * @return Result status or value.
 */
static void *mock_parson_oom_malloc(size_t sz) {
  if (g_parson_oom_fail_at == 0) {
    g_parson_oom_fail_at = -1;
    return NULL;
  }
  if (g_parson_oom_fail_at > 0)
    g_parson_oom_fail_at--;
  return malloc(sz);
}
/**
 * @brief Helper function mock_parson_oom_free.
 * @param[in,out] ptr Parameter ptr.
 */
static void mock_parson_oom_free(void *ptr) { free(ptr); }

/**
 * @brief Helper function set_both_parson_allocators.
 * @param[in,out] malloc_fun Parameter malloc_fun.
 * @param[in,out] free_fun Parameter free_fun.
 */
static void set_both_parson_allocators(void *(*malloc_fun)(size_t),
                                       void (*free_fun)(void *)) {
  (json_set_allocation_functions)(malloc_fun, free_fun);
  c2openapi_set_json_allocators(malloc_fun, free_fun);
}
#ifdef json_set_allocation_functions
#undef json_set_allocation_functions
#endif
/**
 * @brief Macro json_set_allocation_functions.
 */
#define json_set_allocation_functions set_both_parson_allocators

/**
 * @brief Macro ASSERT_WRITER_RC.
 * @param[in] expr Macro argument expr.
 */
#define ASSERT_WRITER_RC(expr)                                                 \
  do {                                                                         \
    cdd_c_error_t _w_rc = (expr);                                              \
    ASSERT(_w_rc == CDD_C_SUCCESS || _w_rc == CDD_C_ERROR_MEMORY ||            \
           _w_rc == CDD_C_ERROR_UNKNOWN ||                                     \
           _w_rc == CDD_C_ERROR_INVALID_ARGUMENT);                             \
  } while (0)

/* --- Helpers --- */

/**
 * @brief Helper function load_spec_str2.
 * @param[in] json_str Parameter json_str.
 * @param[in,out] spec Parameter spec.
 * @return Result status or value.
 */
static cdd_c_error_t load_spec_str2(const char *json_str,
                                    struct OpenAPI_Spec *spec) {
  JSON_Value *dyn;
  cdd_c_error_t rc = 0;
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

/**
 * @brief Helper function setup_test_spec.
 * @param[in,out] spec Parameter spec.
 * @param[in,out] path Parameter path.
 * @param[in,out] op Parameter op.
 * @param[in,out] param Parameter param.
 * @param[out] response Parameter response.
 */
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
    path->route = cdd_test_mut_str("/test/route");
    if (op) {
      path->operations = op;
      path->n_operations = 1;
      op->verb = OA_VERB_GET;
      op->operation_id = cdd_test_mut_str("testOp");
      if (param) {
        op->parameters = param;
        op->n_parameters = 1;
        param->name = cdd_test_mut_str("p1");
        param->in = OA_PARAM_IN_QUERY;
        param->type = cdd_test_mut_str("string");
      }
      if (response) {
        op->responses = response;
        op->n_responses = 1;
        response->code = cdd_test_mut_str("200");
        response->schema.ref_name = cdd_test_mut_str("TestModel");
      }
    }
  }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_COMMON_H */
