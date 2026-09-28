/**
 * @file test_operation.h
 * @brief Unit tests for operation generator orchestrator.
 */

#ifndef TEST_OPERATION_H
#define TEST_OPERATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_operation_common.h"
#include "emit/test_operation_headers.h"
#include "emit/test_operation_links.h"
#include "emit/test_operation_params.h"
#include "emit/test_operation_builders.h"
#include "emit/test_operation_coverage.h"
#include "emit/test_operation_coverage_part2.h"
#include "emit/test_operation_coverage_part3.h"
#include "emit/test_operation_reach.h"
/* clang-format on */

SUITE(operation_suite) {
  RUN_TEST(test_operation_reset_with_link_details);
  RUN_TEST(test_operation_reach_100_percent);
  RUN_TEST(test_operation_100_percent_coverage);
  RUN_TEST(test_operation_100_percent_coverage_part2);
  RUN_TEST(test_operation_100_percent_coverage_part3);
  RUN_TEST(test_operation_all_null_and_boundary_checks);
  RUN_TEST(test_operation_final_gaps);
  RUN_TEST(test_operation_is_reserved_header_name);
  RUN_TEST(test_operation_parse_example_any);
  RUN_TEST(test_operation_parse_example_any_oom);
  RUN_TEST(test_operation_any_from_json_value);
  RUN_TEST(test_operation_any_from_json_value_oom_and_types);
  RUN_TEST(test_operation_parse_link_params_json);
  RUN_TEST(test_operation_parse_link_params_json_oom);
  RUN_TEST(test_operation_parse_link_params_json_cleanup_branches);
  RUN_TEST(test_operation_copy_and_free_any_value_local);
  RUN_TEST(test_operation_copy_any_value_local_nulls);
  RUN_TEST(test_operation_apply_example_to_response_branches);
  RUN_TEST(test_operation_free_openapi_server_variables_op);
  RUN_TEST(test_operation_copy_doc_server_variables_op);
  RUN_TEST(test_operation_copy_doc_server_variables_op_oom);
  RUN_TEST(test_operation_doc_server_variables_enum_calloc_oom);
  RUN_TEST(test_operation_find_doc_param);
  RUN_TEST(test_operation_find_response_by_code);
  RUN_TEST(test_operation_find_media_type_op);
  RUN_TEST(test_operation_ensure_response_for_code);
  RUN_TEST(test_operation_ensure_response_for_code_oom);
  RUN_TEST(test_operation_apply_example);
  RUN_TEST(test_operation_apply_example_to_media_type_oom);
  RUN_TEST(test_operation_add_header_to_response);
  RUN_TEST(test_operation_add_header_to_response_advanced);
  RUN_TEST(test_operation_add_header_to_response_existing_oom);
  RUN_TEST(test_operation_header_example_failure);
  RUN_TEST(test_operation_add_link_to_response);
  RUN_TEST(test_operation_add_link_to_response_advanced);
  RUN_TEST(test_operation_add_link_to_response_oom_sweep);
  RUN_TEST(test_operation_link_cleanup_specific);
  RUN_TEST(test_operation_add_param_to_op);
  RUN_TEST(test_operation_schema_ref_has_data_basic);
  RUN_TEST(test_operation_copy_schema_ref_basic);
  RUN_TEST(test_operation_response_has_media_type);
  RUN_TEST(test_operation_is_struct_pointer);
  RUN_TEST(test_operation_doc_style_to_openapi);
  RUN_TEST(test_operation_oa_type_is_primitive);
  RUN_TEST(test_operation_apply_format_to_schema_ref);
  RUN_TEST(test_operation_set_querystring_schema_from_type_map);
  RUN_TEST(test_operation_init_and_add_response_media_type);
  RUN_TEST(test_operation_request_body_media_types);
  RUN_TEST(test_operation_media_types_item_schema_oom);
  RUN_TEST(test_operation_c2openapi_build_operation_nulls);
  RUN_TEST(test_operation_c2openapi_build_operation_verbs);
  RUN_TEST(test_operation_c2openapi_build_operation_metadata);
  RUN_TEST(test_operation_c2openapi_build_operation_arguments);
  RUN_TEST(test_operation_c2openapi_build_operation_bodies_returns_links);
  RUN_TEST(test_operation_c2openapi_build_operation_coverage_extensions);
  RUN_TEST(test_operation_c2openapi_build_operation_mega_oom);
  RUN_TEST(test_operation_c2openapi_build_operation_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_H */
