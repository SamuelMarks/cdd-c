/**
 * @file test_openapi_writer.h
 * @brief Unit tests for OpenAPI Writer module.
 */

#ifndef TEST_OPENAPI_WRITER_H
#define TEST_OPENAPI_WRITER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
#include "emit/test_openapi_writer_basic.h"
#include "emit/test_openapi_writer_schemas.h"
#include "emit/test_openapi_writer_params.h"
#include "emit/test_openapi_writer_headers.h"
#include "emit/test_openapi_writer_components.h"
#include "emit/test_openapi_writer_paths.h"
#include "emit/test_openapi_writer_types.h"
#include "emit/test_openapi_writer_extensions.h"
#include "emit/test_openapi_writer_branches.h"
#include "emit/test_openapi_writer_coverage.h"
#include "emit/test_openapi_writer_oom.h"
#include "emit/test_openapi_writer_reach.h"
#include "emit/test_openapi_writer_nail.h"
#include "emit/test_openapi_writer_finish.h"
/* clang-format on */
}

SUITE(openapi_writer_suite) {
  RUN_TEST(test_openapi_writer_close_100_percent);
  RUN_TEST(test_openapi_writer_nail_all_remaining);
  RUN_TEST(test_openapi_writer_null_and_defensive);
  RUN_TEST(test_openapi_writer_schema_ref_branches);
  RUN_TEST(test_openapi_writer_schema_and_types_coverage);
  RUN_TEST(test_openapi_writer_any_and_examples_coverage);
  RUN_TEST(test_openapi_writer_objects_and_maps_coverage);
  RUN_TEST(test_openapi_writer_full_spec_edges);
  RUN_TEST(test_openapi_writer_extras_and_edge_branches);
  RUN_TEST(test_openapi_writer_direct_oom);
  RUN_TEST(test_openapi_writer_close_all_remaining_branches);
  RUN_TEST(test_openapi_writer_component_returns_and_root_oom);
  RUN_TEST(test_openapi_writer_strike_last_19);
  RUN_TEST(test_openapi_writer_nail_last_11);
  RUN_TEST(test_openapi_writer_branch_sweep);
  RUN_TEST(test_openapi_writer_branch_sweep);
  RUN_TEST(test_writer_extended_coverage);
  RUN_TEST(test_openapi_utils);
  RUN_TEST(test_writer_empty_spec);
  RUN_TEST(test_writer_basic_operation);
  RUN_TEST(test_writer_schema_document);
  RUN_TEST(test_writer_root_metadata_and_tags);
  RUN_TEST(test_writer_path_ref_and_servers);
  RUN_TEST(test_writer_webhooks);
  RUN_TEST(test_writer_params_responses);
  RUN_TEST(test_writer_parameter_metadata);
  RUN_TEST(test_writer_allow_empty_value);
  RUN_TEST(test_writer_request_body_metadata_and_response_description);
  RUN_TEST(test_writer_info_metadata);
  RUN_TEST(test_writer_info_license_identifier_and_url_rejected);
  RUN_TEST(test_writer_server_url_query_rejected);
  RUN_TEST(test_writer_operation_metadata);
  RUN_TEST(test_writer_response_content_type);
  RUN_TEST(test_writer_inline_response_schema_primitive);
  RUN_TEST(test_writer_inline_response_schema_array);
  RUN_TEST(test_writer_inline_schema_format_and_content);
  RUN_TEST(test_writer_inline_schema_array_item_format_and_content);
  RUN_TEST(test_writer_schema_external_docs_discriminator_xml);
  RUN_TEST(test_writer_inline_schema_const_examples_annotations);
  RUN_TEST(test_writer_preserves_composed_component_schema);
  RUN_TEST(test_writer_preserves_inline_composed_schema);
  RUN_TEST(test_writer_schema_ref_summary_description);
  RUN_TEST(test_writer_info_license_missing_name_rejected);
  RUN_TEST(test_writer_options_trace_verbs);
  RUN_TEST(test_writer_query_and_external_docs);
  RUN_TEST(test_writer_parameter_styles);
  RUN_TEST(test_writer_parameter_style_matrix);
  RUN_TEST(test_writer_servers);
  RUN_TEST(test_writer_querystring_param);
  RUN_TEST(test_writer_ignores_reserved_header_params);
  RUN_TEST(test_writer_ignores_content_type_response_header);
  RUN_TEST(test_writer_parameter_content_any);
  RUN_TEST(test_writer_parameter_and_header_content_media_type);
  RUN_TEST(test_writer_parameter_examples_object);
  RUN_TEST(test_writer_parameter_examples_media);
  RUN_TEST(test_writer_component_examples);
  RUN_TEST(test_writer_oauth2_flows);
  RUN_TEST(test_writer_path_level_parameters);
  RUN_TEST(test_writer_server_variables);
  RUN_TEST(test_writer_components_and_response_headers);
  RUN_TEST(test_writer_components_request_bodies);
  RUN_TEST(test_writer_security_schemes);
  RUN_TEST(test_writer_security_requirements);
  RUN_TEST(test_writer_multipart_schema);
  RUN_TEST(test_writer_components_schemas);
  RUN_TEST(test_writer_components_schemas_raw);
  RUN_TEST(test_writer_schema_ref_external);
  RUN_TEST(test_writer_schema_dynamic_ref_external);
  RUN_TEST(test_writer_schema_items_ref_external);
  RUN_TEST(test_writer_schema_items_dynamic_ref_external);
  RUN_TEST(test_writer_additional_operations);
  RUN_TEST(test_writer_component_media_types_and_content_ref);
  RUN_TEST(test_writer_response_multiple_content);
  RUN_TEST(test_writer_request_body_multiple_content_and_encoding);
  RUN_TEST(test_writer_media_type_prefix_item_encoding);
  RUN_TEST(test_writer_component_path_items);
  RUN_TEST(test_writer_response_links);
  RUN_TEST(test_writer_callbacks);
  RUN_TEST(test_writer_parameter_and_header_schema_ref);
  RUN_TEST(test_writer_parameter_schema_format_and_content);
  RUN_TEST(test_writer_request_body_ref_with_description);
  RUN_TEST(test_writer_security_scheme_deprecated);
  RUN_TEST(test_writer_schema_enum_default_nullable);
  RUN_TEST(test_writer_schema_type_union);
  RUN_TEST(test_writer_schema_array_items_enum_nullable);
  RUN_TEST(test_writer_schema_items_type_union);
  RUN_TEST(test_writer_schema_example_and_numeric_constraints);
  RUN_TEST(test_writer_schema_array_constraints_and_items_example);
  RUN_TEST(test_writer_inline_schema_items_const_default_and_extras);
  RUN_TEST(test_writer_input_validation);
  RUN_TEST(test_writer_extensions_non_schema);
  RUN_TEST(test_writer_paths_webhooks_components_extensions);
  RUN_TEST(test_writer_methods_and_styles);
  RUN_TEST(test_writer_xml_and_oauth);
  RUN_TEST(test_writer_xml_types);
  RUN_TEST(test_writer_parameter_explode_false);
  RUN_TEST(test_writer_schema_boolean);
  RUN_TEST(test_writer_schema_items_boolean);
  RUN_TEST(test_writer_schema_items_examples);
  RUN_TEST(test_writer_schema_numeric_enum);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_H */
