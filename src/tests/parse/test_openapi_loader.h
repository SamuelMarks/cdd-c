/**
 * @file test_openapi_loader.h
 * @brief Unit tests for OpenAPI loading.
 */

#ifndef TEST_OPENAPI_LOADER_H
#define TEST_OPENAPI_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "parse/test_openapi_loader_validation.h"
#include "parse/test_openapi_loader_params.h"
#include "parse/test_openapi_loader_security.h"
#include "parse/test_openapi_loader_media.h"
#include "parse/test_openapi_loader_schemas.h"
#include "parse/test_openapi_loader_refs.h"
#include "parse/test_openapi_loader_paths.h"
#include "parse/test_openapi_loader_advanced.h"
#include "parse/test_openapi_loader_coverage.h"
#include "parse/test_openapi_loader_copy_free.h"
#include "parse/test_openapi_loader_branches.h"
#include "parse/test_openapi_find_coverage.h"
#include "parse/test_openapi_schema_ref_coverage.h"
#include "parse/test_openapi_schema_registry_coverage.h"
#include "parse/test_openapi_utils_coverage.h"
#include "parse/test_openapi_paths_coverage.h"
#include "parse/test_openapi_top_level_coverage.h"
#include "parse/test_openapi_components_coverage.h"
#include "parse/test_openapi_metadata_coverage.h"
#include "parse/test_openapi_uri_coverage.h"
#include "parse/test_openapi_headers_links_coverage.h"
/* clang-format on */

SUITE(openapi_loader_suite) {
  RUN_TEST(test_openapi_loader_100_percent_final);
  RUN_TEST(test_load_link_full_fields);
  RUN_TEST(test_load_schema_content_schema);
  RUN_TEST(test_load_nested_encoding_fields);
  RUN_TEST(test_load_parameter_and_header_content_media_types);
  RUN_TEST(test_load_parameter_array);
  RUN_TEST(test_openapi_parameters_full_coverage);
  RUN_TEST(test_load_parameter_metadata);
  RUN_TEST(test_load_allow_empty_value);
  RUN_TEST(test_load_allow_empty_value_non_query_rejected);
  RUN_TEST(test_load_querystring_parameter);
  RUN_TEST(test_load_querystring_json_inline_promoted);
  RUN_TEST(test_ignore_reserved_header_parameters);
  RUN_TEST(test_ignore_content_type_response_header);
  RUN_TEST(test_param_schema_and_content_conflict);
  RUN_TEST(test_header_schema_and_content_conflict);
  RUN_TEST(test_load_parameter_content_any);
  RUN_TEST(test_load_parameter_content_media_type_encoding);
  RUN_TEST(test_load_header_content_media_type);
  RUN_TEST(test_load_parameter_schema_ref);
  RUN_TEST(test_load_header_schema_ref);
  RUN_TEST(test_load_path_level_parameters);
  RUN_TEST(test_load_server_variables);
  RUN_TEST(test_server_variable_default_required);
  RUN_TEST(test_load_schema_parsing);
  RUN_TEST(test_load_schema_external_docs_discriminator_xml);
  RUN_TEST(test_load_form_content_type);
  RUN_TEST(test_request_body_content_required);
  RUN_TEST(test_load_request_body_item_schema_array);
  RUN_TEST(test_param_content_multiple_entries_rejected);
  RUN_TEST(test_header_content_multiple_entries_rejected);
  RUN_TEST(test_response_description_required);
  RUN_TEST(test_paths_require_leading_slash);
  RUN_TEST(test_paths_ambiguous_templates_rejected);
  RUN_TEST(test_component_key_regex_rejected);
  RUN_TEST(test_tag_duplicate_rejected);
  RUN_TEST(test_tag_name_required);
  RUN_TEST(test_tag_parent_missing_rejected);
  RUN_TEST(test_tag_parent_cycle_rejected);
  RUN_TEST(test_external_docs_url_required);
  RUN_TEST(test_operation_id_duplicate_rejected);
  RUN_TEST(test_operation_id_duplicate_in_callback_rejected);
  RUN_TEST(test_parameter_duplicates_rejected);
  RUN_TEST(test_querystring_with_query_rejected);
  RUN_TEST(test_querystring_duplicate_rejected);
  RUN_TEST(test_querystring_path_and_operation_mixed_rejected);
  RUN_TEST(test_querystring_with_query_in_callback_rejected);
  RUN_TEST(test_parameter_missing_name_or_in_rejected);
  RUN_TEST(test_header_style_non_simple_rejected);
  RUN_TEST(test_media_type_encoding_conflict_rejected);
  RUN_TEST(test_encoding_object_conflict_rejected);
  RUN_TEST(test_load_operation_tags);
  RUN_TEST(test_load_openapi_version_and_servers);
  RUN_TEST(test_load_server_duplicate_name_rejected);
  RUN_TEST(test_load_missing_openapi_and_swagger_rejected);
  RUN_TEST(test_load_schema_root_document_with_id);
  RUN_TEST(test_load_schema_root_boolean);
  RUN_TEST(test_load_swagger_root_allowed);
  RUN_TEST(test_load_openapi_version_unsupported_rejected);
  RUN_TEST(test_load_server_url_query_rejected);
  RUN_TEST(test_load_security_requirements);
  RUN_TEST(test_load_security_schemes);
  RUN_TEST(test_load_security_scheme_deprecated);
  RUN_TEST(test_load_oauth2_flows);
  RUN_TEST(test_load_security_scheme_http_missing_scheme_rejected);
  RUN_TEST(test_load_security_scheme_apikey_missing_name_rejected);
  RUN_TEST(test_load_security_scheme_apikey_missing_in_rejected);
  RUN_TEST(test_load_security_scheme_openid_missing_url_rejected);
  RUN_TEST(test_load_oauth2_missing_flows_rejected);
  RUN_TEST(test_load_oauth2_flow_missing_scopes_rejected);
  RUN_TEST(test_load_oauth2_flow_missing_required_urls_rejected);
  RUN_TEST(test_load_oauth2_flow_unknown_rejected);
  RUN_TEST(test_load_parameter_examples_object);
  RUN_TEST(test_load_parameter_examples_media);
  RUN_TEST(test_load_parameter_example_and_examples_rejected);
  RUN_TEST(test_load_header_example_and_examples_rejected);
  RUN_TEST(test_load_media_example_and_examples_rejected);
  RUN_TEST(test_load_example_data_value_and_value_rejected);
  RUN_TEST(test_load_example_serialized_and_external_rejected);
  RUN_TEST(test_load_response_examples_media);
  RUN_TEST(test_load_component_examples);
  RUN_TEST(test_load_example_component_ref_strict);
  RUN_TEST(test_load_request_body_metadata_and_response_description);
  RUN_TEST(test_load_request_body_component_ref);
  RUN_TEST(test_load_response_multiple_content);
  RUN_TEST(test_openapi_responses_full_coverage);
  RUN_TEST(test_load_request_body_multiple_content_with_ref);
  RUN_TEST(test_load_media_type_encoding);
  RUN_TEST(test_load_media_type_prefix_item_encoding);
  RUN_TEST(test_load_info_metadata);
  RUN_TEST(test_load_info_missing_title_rejected);
  RUN_TEST(test_load_info_missing_version_rejected);
  RUN_TEST(test_load_license_identifier_and_url_rejected);
  RUN_TEST(test_load_license_missing_name_rejected);
  RUN_TEST(test_load_operation_metadata);
  RUN_TEST(test_load_root_metadata_and_tags);
  RUN_TEST(test_self_qualified_component_refs);
  RUN_TEST(test_relative_self_component_refs);
  RUN_TEST(test_schema_id_ref_resolution);
  RUN_TEST(test_schema_anchor_ref_resolution);
  RUN_TEST(test_schema_dynamic_ref_resolution);
  RUN_TEST(test_external_component_ref_registry_absolute);
  RUN_TEST(test_external_component_ref_registry_relative);
  RUN_TEST(test_load_response_content_type);
  RUN_TEST(test_load_response_content_type_specificity);
  RUN_TEST(test_load_response_content_type_params_json);
  RUN_TEST(test_load_inline_response_schema_primitive);
  RUN_TEST(test_load_inline_response_schema_array);
  RUN_TEST(test_load_inline_schema_format_and_content);
  RUN_TEST(test_load_inline_schema_array_item_format_and_content);
  RUN_TEST(test_load_inline_schema_const_examples_annotations);
  RUN_TEST(test_load_schema_ref_summary_description);
  RUN_TEST(test_load_parameter_schema_format_and_content);
  RUN_TEST(test_load_inline_schema_enum_default_nullable);
  RUN_TEST(test_load_inline_schema_type_union);
  RUN_TEST(test_load_inline_schema_array_items_enum_nullable);
  RUN_TEST(test_load_inline_schema_items_type_union);
  RUN_TEST(test_load_inline_schema_example_and_numeric_constraints);
  RUN_TEST(test_load_inline_schema_array_constraints_and_items_example);
  RUN_TEST(test_load_inline_schema_items_const_default_and_extras);
  RUN_TEST(test_load_inline_request_body_object_promoted);
  RUN_TEST(test_load_inline_response_schema_object_item_promoted);
  RUN_TEST(test_load_inline_response_item_schema_object_promoted);
  RUN_TEST(test_load_request_body_ref_description_override);
  RUN_TEST(test_load_options_trace_verbs);
  RUN_TEST(test_load_query_verb_and_external_docs);
  RUN_TEST(test_load_path_and_operation_servers);
  RUN_TEST(test_load_webhooks);
  RUN_TEST(test_load_path_ref);
  RUN_TEST(test_load_component_parameter_ref);
  RUN_TEST(test_load_component_response_and_headers);
  RUN_TEST(test_load_additional_operations);
  RUN_TEST(test_load_component_media_type_ref);
  RUN_TEST(test_load_schema_conditional_keywords);
  RUN_TEST(test_load_path_item_ref_with_operation_security);
  RUN_TEST(test_load_component_path_items);
  RUN_TEST(test_load_response_links_and_component_links);
  RUN_TEST(test_load_callbacks_and_component_callbacks);
  RUN_TEST(test_load_path_item_ref_resolves_component);
  RUN_TEST(test_load_callback_ref_resolves_component);
  RUN_TEST(test_load_extensions_non_schema);
  RUN_TEST(test_load_paths_webhooks_components_extensions);
  RUN_TEST(test_webhook_path_template_not_validated);
  RUN_TEST(test_load_component_schema_raw);
  RUN_TEST(test_load_schema_external_ref);
  RUN_TEST(test_load_schema_external_items_ref);
  RUN_TEST(test_load_path_template_missing_param);
  RUN_TEST(test_load_path_template_param_not_in_route);
  RUN_TEST(test_load_path_template_param_not_required);
  RUN_TEST(test_load_root_missing_paths_components_webhooks_rejected);
  RUN_TEST(test_param_style_invalid_for_in_rejected);
  RUN_TEST(test_param_style_deep_object_scalar_rejected);
  RUN_TEST(test_server_url_variable_missing_definition_rejected);
  RUN_TEST(test_server_url_variable_duplicate_rejected);
  RUN_TEST(test_load_server_missing_url_rejected);
  RUN_TEST(test_load_additional_operations_standard_method_rejected);
  RUN_TEST(test_load_link_missing_operation_ref_or_id_rejected);
  RUN_TEST(test_load_link_operation_ref_and_id_both_rejected);
  RUN_TEST(test_load_parameter_explode_false);
  RUN_TEST(test_load_schema_boolean_and_numeric_enum);
  RUN_TEST(test_load_schema_items_examples_and_boolean_items);
  RUN_TEST(test_load_schema_ref_with_pointer_is_not_component);
  RUN_TEST(test_operation_responses_required);
  RUN_TEST(test_response_code_key_invalid_rejected);
  RUN_TEST(test_response_code_range_valid);
  RUN_TEST(test_load_comprehensive_path_item_and_schema_copying);
  RUN_TEST(test_load_components_ref_overrides_and_deep_copying);
  RUN_TEST(test_load_all_security_scheme_types_and_flows);
  RUN_TEST(test_security_scheme_validation_errors);
  RUN_TEST(test_load_header_schema_array_and_ref_propagation);
  RUN_TEST(test_load_uri_resolution_and_self_matching);
  RUN_TEST(test_header_and_parameter_style_and_content_variations);
  RUN_TEST(test_header_and_parameter_validation_errors);
  RUN_TEST(test_swagger2_parameter_and_response_type_fallbacks);
  RUN_TEST(test_load_webhooks_and_component_callbacks);
  RUN_TEST(test_path_template_validation_and_collision_errors);
  RUN_TEST(test_openapi_loader_helper_fallbacks);
  RUN_TEST(test_openapi_loader_copy_and_free_helpers);
  RUN_TEST(test_openapi_loader_copy_oom_branches);
  RUN_TEST(test_openapi_loader_copy_and_free_advanced);
  RUN_TEST(test_openapi_loader_uri_and_schema_ref_branches);
  RUN_TEST(test_openapi_loader_media_type_and_response_examples);
  RUN_TEST(test_openapi_loader_swagger2_and_schema_ref_branches);
  RUN_TEST(test_openapi_find_all_components);
  RUN_TEST(test_openapi_loader_security_branches);
  RUN_TEST(test_openapi_media_all_branches);
  RUN_TEST(test_openapi_schemas_all_branches);
  RUN_TEST(test_openapi_examples_all_branches);
  RUN_TEST(test_openapi_validation_all_branches);
  RUN_TEST(test_openapi_schema_ref_all_branches);
  RUN_TEST(test_openapi_paths_name_list_and_template_names);
  RUN_TEST(test_openapi_paths_normalize_and_collisions);
  RUN_TEST(test_openapi_paths_validation_branches);
  RUN_TEST(test_openapi_paths_parsing_branches);
  OPENAPI_SCHEMA_REGISTRY_TESTS();
  OPENAPI_UTILS_COVERAGE_TESTS();
  OPENAPI_TOP_LEVEL_COVERAGE_TESTS();
  OPENAPI_COMPONENTS_COVERAGE_TESTS();
  OPENAPI_HEADERS_LINKS_COVERAGE_TESTS();
  OPENAPI_METADATA_COVERAGE_TESTS();
  OPENAPI_URI_COVERAGE_TESTS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_H */
