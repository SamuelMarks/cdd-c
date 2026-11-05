/**
 * @file test_openapi_validation_querystring.h
 * @brief Unit tests for OpenAPI querystring parameter validation
 * (openapi_validation.c).
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_VALIDATION_QUERYSTRING_H
#define TEST_OPENAPI_VALIDATION_QUERYSTRING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test for scan_querystring_usage and validate_querystring_usage*
 * routines.
 * @return Greatest test result.
 */
TEST test_openapi_validation_querystring_branches(void) {
  size_t qs_count = 0;
  int has_query = 0;
  struct OpenAPI_Parameter params[3];
  struct OpenAPI_Path paths[2];
  struct OpenAPI_Operation ops[2];
  struct OpenAPI_Callback cbs[2];
  struct OpenAPI_Spec spec;

  memset(params, 0, sizeof(params));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  memset(&spec, 0, sizeof(spec));

  /* 1. scan_querystring_usage */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            scan_querystring_usage(params, 1, NULL, &has_query));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            scan_querystring_usage(params, 1, &qs_count, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            scan_querystring_usage(params, 1, NULL, NULL));

  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[1].in = OA_PARAM_IN_QUERY;
  params[2].in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(CDD_C_SUCCESS,
            scan_querystring_usage(params, 3, &qs_count, &has_query));
  ASSERT_EQ(1, qs_count);
  ASSERT_EQ(1, has_query);

  /* 2. validate_querystring_usage */
  /* Path without route is skipped */
  paths[0].route = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage(paths, 1));

  paths[0].route = cdd_test_mut_str("/test");
  paths[0].parameters = params;
  paths[0].n_parameters = 3; /* has qs_count=1 and has_query=1 -> invalid! */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_querystring_usage(paths, 1));

  /* path_qs > 1 */
  params[1].in = OA_PARAM_IN_QUERYSTRING;
  paths[0].n_parameters = 2; /* 2 querystring params -> invalid! */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_querystring_usage(paths, 1));

  /* path has 1 querystring param, op has query param -> invalid */
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  paths[0].n_parameters = 1;
  ops[0].parameters = &params[1];
  params[1].in = OA_PARAM_IN_QUERY;
  ops[0].n_parameters = 1;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_querystring_usage(paths, 1));

  /* path has 1 querystring, op has 1 querystring -> total_qs > 1 */
  params[1].in = OA_PARAM_IN_QUERYSTRING;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_querystring_usage(paths, 1));

  /* path has 0 querystring, additional_operations has total_qs > 1 */
  paths[0].n_parameters = 0;
  paths[0].n_operations = 0;
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[1].in = OA_PARAM_IN_QUERYSTRING;
  ops[0].parameters = params;
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_querystring_usage(paths, 1));

  /* additional_operations total_qs > 0 && has_query */
  params[1].in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_querystring_usage(paths, 1));

  /* scan_querystring_usage error branches */
  paths[0].parameters = NULL;
  paths[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_querystring_usage(paths, 1));
  paths[0].parameters = params;
  paths[0].n_parameters = 0;

  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ops[0].parameters = NULL;
  ops[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_querystring_usage(paths, 1));
  ops[0].parameters = params;
  ops[0].n_parameters = 0;
  paths[0].n_operations = 0;

  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ops[0].parameters = NULL;
  ops[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_querystring_usage(paths, 1));
  ops[0].parameters = params;
  ops[0].n_parameters = 0;
  paths[0].n_additional_operations = 0;

  /* operations with total_qs == 1 && has_query == 0 */
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  ops[0].parameters = params;
  ops[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage(paths, 1));
  paths[0].n_operations = 0;

  /* additional_operations with total_qs == 1 && has_query == 0 */
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage(paths, 1));
  paths[0].n_additional_operations = 0;

  /* path with query param (path_has_query == 1) */
  params[0].in = OA_PARAM_IN_QUERY;
  paths[0].parameters = params;
  paths[0].n_parameters = 1;
  paths[0].operations = ops;
  ops[0].n_parameters = 0;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage(paths, 1));
  paths[0].n_operations = 0;
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage(paths, 1));
  paths[0].n_additional_operations = 0;
  paths[0].n_parameters = 0;

  /* Valid querystring usage */
  ops[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage(paths, 1));

  /* 3. validate_querystring_usage_in_callbacks */
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage_in_callbacks(NULL, 0));
  cbs[0].paths = NULL;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage_in_callbacks(cbs, 1));
  cbs[0].paths = paths;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage_in_callbacks(cbs, 1));
  cbs[0].paths = paths;
  cbs[0].n_paths = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage_in_callbacks(cbs, 1));
  /* callback error */
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[1].in = OA_PARAM_IN_QUERYSTRING;
  ops[0].parameters = params;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            validate_querystring_usage_in_callbacks(cbs, 1));
  ops[0].n_parameters = 1;

  /* 4. validate_querystring_usage_in_operations */
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage_in_operations(NULL, 0));
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_querystring_usage_in_operations(ops, 1));
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            validate_querystring_usage_in_operations(ops, 1));
  ops[0].n_parameters = 1;

  /* 5. validate_querystring_usage_in_paths_callbacks */
  ASSERT_EQ(CDD_C_SUCCESS,
            validate_querystring_usage_in_paths_callbacks(NULL, 0));
  paths[0].n_additional_operations = 0;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            validate_querystring_usage_in_paths_callbacks(paths, 1));
  paths[0].n_operations = 0;
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            validate_querystring_usage_in_paths_callbacks(paths, 1));
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            validate_querystring_usage_in_paths_callbacks(paths, 1));
  paths[0].n_operations = 1;
  paths[0].n_additional_operations = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            validate_querystring_usage_in_paths_callbacks(paths, 1));
  ops[0].n_parameters = 1;

  /* 6. validate_querystring_usage_in_component_callbacks */
  ASSERT_EQ(CDD_C_SUCCESS,
            validate_querystring_usage_in_component_callbacks(NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            validate_querystring_usage_in_component_callbacks(&spec));
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 1;
  cbs[0].paths = paths;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            validate_querystring_usage_in_component_callbacks(&spec));
  cbs[0].paths = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            validate_querystring_usage_in_component_callbacks(&spec));
  cbs[0].paths = paths;
  cbs[0].n_paths = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            validate_querystring_usage_in_component_callbacks(&spec));
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            validate_querystring_usage_in_component_callbacks(&spec));
  ops[0].n_parameters = 1;
  memset(&spec, 0, sizeof(spec));

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_VALIDATION_QUERYSTRING_H */
