/**
 * @file g_fail_io_after.c
 * @brief Unit tests and mocks in g_fail_io_after.c.
 */
/* clang-format off */
#include "c_cdd_export.h"
/* clang-format on */
/** @brief Test definition g_fail_io_after. */
C_CDD_EXPORT int g_fail_io_after = -1;
/** @brief Test definition g_io_calls. */
C_CDD_EXPORT int g_io_calls = 0;

/** @brief Test definition g_crypto_fail_digestfinal. */
C_CDD_EXPORT int g_crypto_fail_digestfinal = 0;
/** @brief Test definition g_crypto_fail_digestfinal_len. */
C_CDD_EXPORT int g_crypto_fail_digestfinal_len = 0;
/** @brief Test definition g_crypto_fail_digestinit. */
C_CDD_EXPORT int g_crypto_fail_digestinit = 0;
/** @brief Test definition g_crypto_fail_digestupdate. */
C_CDD_EXPORT int g_crypto_fail_digestupdate = 0;
/** @brief Test definition g_crypto_fail_hmac. */
C_CDD_EXPORT int g_crypto_fail_hmac = 0;
/** @brief Test definition g_crypto_fail_hmac_len. */
C_CDD_EXPORT int g_crypto_fail_hmac_len = 0;
/** @brief Test definition g_crypto_fail_mdctx_new. */
C_CDD_EXPORT int g_crypto_fail_mdctx_new = 0;
/** @brief Test definition g_crypto_fail_sha256. */
C_CDD_EXPORT int g_crypto_fail_sha256 = 0;

/** @brief Test definition g_cdd_mock_dlopen_success. */
C_CDD_EXPORT int g_cdd_mock_dlopen_success = 0;
/** @brief Test definition g_cdd_fail_raw_schema_serialize. */
C_CDD_EXPORT int g_cdd_fail_raw_schema_serialize = 0;
/** @brief Test definition g_cdd_fail_assign_ref_name. */
C_CDD_EXPORT int g_cdd_fail_assign_ref_name = 0;
/** @brief Test definition g_cdd_fail_any_serialize. */
C_CDD_EXPORT int g_cdd_fail_any_serialize = 0;
/** @brief Test definition g_cdd_fail_str_iequal. */
C_CDD_EXPORT int g_cdd_fail_str_iequal = 0;
/** @brief Test definition g_cdd_fail_clone_json_serialize. */
C_CDD_EXPORT int g_cdd_fail_clone_json_serialize = 0;
/** @brief Test definition g_cdd_fail_clone_json_parse. */
C_CDD_EXPORT int g_cdd_fail_clone_json_parse = 0;
/** @brief Test definition g_cdd_fail_extras_init. */
C_CDD_EXPORT int g_cdd_fail_extras_init = 0;
/** @brief Test definition g_cdd_fail_extras_serialize. */
C_CDD_EXPORT int g_cdd_fail_extras_serialize = 0;
/** @brief Test definition g_cdd_fail_extensions_init. */
C_CDD_EXPORT int g_cdd_fail_extensions_init = 0;
/** @brief Test definition g_cdd_fail_extensions_serialize. */
C_CDD_EXPORT int g_cdd_fail_extensions_serialize = 0;
C_CDD_EXPORT int g_cdd_fprintf_fail = 0;
/** @brief Test definition g_cdd_fail_alloc. */
C_CDD_EXPORT int g_cdd_fail_alloc = 0;
/** @brief Test definition g_str_unquote_malloc_fail. */
C_CDD_EXPORT int g_str_unquote_malloc_fail = 0;
/** @brief Test definition g_parse_binary_str_fail. */
C_CDD_EXPORT int g_parse_binary_str_fail = 0;
/** @brief Test definition g_is_basic_type_keyword_fail. */
C_CDD_EXPORT int g_is_basic_type_keyword_fail = 0;
/** @brief Test definition g_cdd_fail_stricmp. */
C_CDD_EXPORT int g_cdd_fail_stricmp = 0;
/** @brief Test definition g_cdd_fail_count_returning_allocs. */
C_CDD_EXPORT int g_cdd_fail_count_returning_allocs = 0;
/** @brief Test definition g_cdd_fail_find_balanced_end. */
C_CDD_EXPORT int g_cdd_fail_find_balanced_end = 0;
/** @brief Test definition g_cdd_fail_parsed_sig_init. */
C_CDD_EXPORT int g_cdd_fail_parsed_sig_init = 0;
/** @brief Test definition g_cdd_fail_args_represent_void. */
C_CDD_EXPORT int g_cdd_fail_args_represent_void = 0;
/** @brief Test definition g_cdd_fail_is_inside_condition. */
C_CDD_EXPORT int g_cdd_fail_is_inside_condition = 0;
/** @brief Test definition g_cdd_fail_is_dereference_use. */
C_CDD_EXPORT int g_cdd_fail_is_dereference_use = 0;
/** @brief Test definition g_cdd_fail_is_checked. */
C_CDD_EXPORT int g_cdd_fail_is_checked = 0;
/** @brief Test definition g_cdd_fail_vla_basic_type. */
C_CDD_EXPORT int g_cdd_fail_vla_basic_type = 0;
/** @brief Test definition g_cdd_fail_str_after_last. */
C_CDD_EXPORT int g_cdd_fail_str_after_last = 0;
/** @brief Test definition g_cdd_fail_find_next_token_idx. */
C_CDD_EXPORT int g_cdd_fail_find_next_token_idx = 0;
/** @brief Test definition g_cdd_fail_patch_list_sort. */
C_CDD_EXPORT int g_cdd_fail_patch_list_sort = 0;
/** @brief Test definition g_cdd_fail_str_starts_with. */
C_CDD_EXPORT int g_cdd_fail_str_starts_with = 0;
/** @brief Test definition g_cdd_fail_skip_qualifiers. */
C_CDD_EXPORT int g_cdd_fail_skip_qualifiers = 0;
/** @brief Test definition g_cdd_fail_get_basename. */
C_CDD_EXPORT int g_cdd_fail_get_basename = 0;
/** @brief Test definition g_cdd_fail_sync_matches. */
C_CDD_EXPORT int g_cdd_fail_sync_matches = 0;
/** @brief Test definition g_cdd_fail_sync_patch_add. */
C_CDD_EXPORT int g_cdd_fail_sync_patch_add = 0;
/** @brief Test definition g_cdd_fail_find_line_for_token. */
C_CDD_EXPORT int g_cdd_fail_find_line_for_token = 0;
/** @brief Test definition g_cdd_fail_append_to_diff. */
C_CDD_EXPORT int g_cdd_fail_append_to_diff = 0;
/** @brief Test definition g_cdd_fail_find_semicolon. */
C_CDD_EXPORT int g_cdd_fail_find_semicolon = 0;
/** @brief Test definition g_cdd_fail_find_stmt_start. */
C_CDD_EXPORT int g_cdd_fail_find_stmt_start = 0;
/** @brief Test definition g_cdd_fail_find_refactored_func. */
C_CDD_EXPORT int g_cdd_fail_find_refactored_func = 0;
/** @brief Test definition g_cdd_fail_patch_list_add. */
C_CDD_EXPORT int g_cdd_fail_patch_list_add = 0;
/** @brief Test definition g_cdd_fail_skip_ws. */
C_CDD_EXPORT int g_cdd_fail_skip_ws = 0;
/** @brief Test definition g_cdd_fail_skip_ws_back. */
C_CDD_EXPORT int g_cdd_fail_skip_ws_back = 0;
/** @brief Test definition g_cdd_fail_skip_group. */
C_CDD_EXPORT int g_cdd_fail_skip_group = 0;
/** @brief Test definition g_cdd_fail_add_type_node. */
C_CDD_EXPORT int g_cdd_fail_add_type_node = 0;
/** @brief Test definition g_cdd_fail_create_node. */
C_CDD_EXPORT int g_cdd_fail_create_node = 0;
/** @brief Test definition g_cdd_fail_find_pivot. */
C_CDD_EXPORT int g_cdd_fail_find_pivot = 0;
/** @brief Test definition g_cdd_fail_is_grouping_paren. */
C_CDD_EXPORT int g_cdd_fail_is_grouping_paren = 0;
/** @brief Test definition g_cdd_fail_is_type_start. */
C_CDD_EXPORT int g_cdd_fail_is_type_start = 0;
/** @brief Test definition g_cdd_fail_consume_balanced_parens. */
C_CDD_EXPORT int g_cdd_fail_consume_balanced_parens = 0;
/** @brief Test definition g_cdd_fail_consume_attributes. */
C_CDD_EXPORT int g_cdd_fail_consume_attributes = 0;
/** @brief Test definition g_cdd_fail_consume_static_assert. */
C_CDD_EXPORT int g_cdd_fail_consume_static_assert = 0;
/** @brief Test definition g_cdd_fail_consume_generic_selection. */
C_CDD_EXPORT int g_cdd_fail_consume_generic_selection = 0;
/** @brief Test definition g_cdd_fail_is_expression_brace. */
C_CDD_EXPORT int g_cdd_fail_is_expression_brace = 0;
/** @brief Test definition g_cdd_fail_consume_balanced_braces. */
C_CDD_EXPORT int g_cdd_fail_consume_balanced_braces = 0;
/** @brief Test definition g_cdd_fail_match_function_definition. */
C_CDD_EXPORT int g_cdd_fail_match_function_definition = 0;
/** @brief Test definition g_cdd_fail_make_tmpfile. */
C_CDD_EXPORT int g_cdd_fail_make_tmpfile = 0;
/** @brief Test definition g_cdd_fail_find_function_node. */
C_CDD_EXPORT int g_cdd_fail_find_function_node = 0;
/** @brief Test definition g_cdd_fail_generate_expected_sig. */
C_CDD_EXPORT int g_cdd_fail_generate_expected_sig = 0;
/** @brief Test definition g_cdd_fail_extract_current_sig. */
C_CDD_EXPORT int g_cdd_fail_extract_current_sig = 0;
/** @brief Test definition g_cdd_fail_apply_query_sync. */
C_CDD_EXPORT int g_cdd_fail_apply_query_sync = 0;
/** @brief Test definition g_cdd_fail_apply_header_sync. */
C_CDD_EXPORT int g_cdd_fail_apply_header_sync = 0;
/** @brief Test definition g_cdd_fail_generate_expected_url. */
C_CDD_EXPORT int g_cdd_fail_generate_expected_url = 0;
/** @brief Test definition g_cdd_fail_codegen_write. */
C_CDD_EXPORT int g_cdd_fail_codegen_write = 0;
/** @brief Test definition g_cdd_sync_sig_no_brace. */
C_CDD_EXPORT int g_cdd_sync_sig_no_brace = 0;
/** @brief Test definition g_build_system_fail_io_after. */
C_CDD_EXPORT int g_build_system_fail_io_after = -1;
/** @brief Test definition g_cdd_fail_media_type_base_len. */
C_CDD_EXPORT int g_cdd_fail_media_type_base_len = 0;
/** @brief Test definition g_cdd_fail_media_type_has_suffix. */
C_CDD_EXPORT int g_cdd_fail_media_type_has_suffix = 0;
/** @brief Test definition g_cdd_fail_media_type_ieq. */
C_CDD_EXPORT int g_cdd_fail_media_type_ieq = 0;
/** @brief Test definition g_cdd_fail_media_type_is_json. */
C_CDD_EXPORT int g_cdd_fail_media_type_is_json = 0;
/** @brief Test definition g_cdd_fail_media_type_is_text_plain. */
C_CDD_EXPORT int g_cdd_fail_media_type_is_text_plain = 0;
/** @brief Test definition g_cdd_fail_media_type_has_prefix. */
C_CDD_EXPORT int g_cdd_fail_media_type_has_prefix = 0;
/** @brief Test definition g_cdd_fail_media_type_is_form. */
C_CDD_EXPORT int g_cdd_fail_media_type_is_form = 0;
/** @brief Test definition g_cdd_fail_media_type_is_multipart. */
C_CDD_EXPORT int g_cdd_fail_media_type_is_multipart = 0;
/** @brief Test definition g_cdd_fail_media_type_is_textual. */
C_CDD_EXPORT int g_cdd_fail_media_type_is_textual = 0;
/** @brief Test definition g_cdd_fail_media_type_is_binary. */
C_CDD_EXPORT int g_cdd_fail_media_type_is_binary = 0;
/** @brief Test definition g_cdd_fail_media_type_is_multipart_form. */
C_CDD_EXPORT int g_cdd_fail_media_type_is_multipart_form = 0;
/** @brief Test definition g_cdd_fail_qs_json_array_item_type. */
C_CDD_EXPORT int g_cdd_fail_qs_json_array_item_type = 0;
/** @brief Test definition g_cdd_fail_qs_json_array_item_ref. */
C_CDD_EXPORT int g_cdd_fail_qs_json_array_item_ref = 0;
/** @brief Test definition g_cdd_fail_qs_json_prim. */
C_CDD_EXPORT int g_cdd_fail_qs_json_prim = 0;
/** @brief Test definition g_cdd_fail_qs_raw. */
C_CDD_EXPORT int g_cdd_fail_qs_raw = 0;
/** @brief Test definition g_cdd_fail_qs_form_obj. */
C_CDD_EXPORT int g_cdd_fail_qs_form_obj = 0;
/** @brief Test definition g_cdd_fail_qs_json_ref. */
C_CDD_EXPORT int g_cdd_fail_qs_json_ref = 0;
/** @brief Test definition g_cdd_fail_map_array_item_type. */
C_CDD_EXPORT int g_cdd_fail_map_array_item_type = 0;
/** @brief Test definition g_cdd_fail_sanitize_ident. */
C_CDD_EXPORT int g_cdd_fail_sanitize_ident = 0;
/** @brief Test definition g_cdd_fail_map_type_to_c_arg. */
C_CDD_EXPORT int g_cdd_fail_map_type_to_c_arg = 0;
/** @brief Test definition g_cdd_fail_is_primitive_type. */
C_CDD_EXPORT int g_cdd_fail_is_primitive_type = 0;
/** @brief Test definition g_cdd_fail_param_is_object_kv. */
C_CDD_EXPORT int g_cdd_fail_param_is_object_kv = 0;
/** @brief Test definition g_cdd_fail_find_media_type. */
C_CDD_EXPORT int g_cdd_fail_find_media_type = 0;
/** @brief Test definition g_cdd_fail_header_name_is_content_type. */
C_CDD_EXPORT int g_cdd_fail_header_name_is_content_type = 0;
/** @brief Test definition g_cdd_fail_c_cdd_str_iequal. */
C_CDD_EXPORT int g_cdd_fail_c_cdd_str_iequal = 0;
/** @brief Test definition g_cdd_fail_multipart_header_param_name. */
C_CDD_EXPORT int g_cdd_fail_multipart_header_param_name = 0;
/** @brief Test definition g_cdd_fail_response_is_binary_success. */
C_CDD_EXPORT int g_cdd_fail_response_is_binary_success = 0;
/** @brief Test definition g_cdd_fail_get_success_schema. */
C_CDD_EXPORT int g_cdd_fail_get_success_schema = 0;
/** @brief Test definition g_cdd_fail_get_success_response. */
C_CDD_EXPORT int g_cdd_fail_get_success_response = 0;
/** @brief Test definition g_cdd_fail_schema_has_inline. */
C_CDD_EXPORT int g_cdd_fail_schema_has_inline = 0;
/** @brief Test definition g_cdd_fail_map_type_to_c_out. */
C_CDD_EXPORT int g_cdd_fail_map_type_to_c_out = 0;
/** @brief Test definition g_cdd_fail_map_array_item_type_out. */
C_CDD_EXPORT int g_cdd_fail_map_array_item_type_out = 0;
/** @brief Test definition g_force_find_allocations_fail. */
C_CDD_EXPORT int g_force_find_allocations_fail = 0;
/** @brief Test definition g_force_parse_tokens_fail. */
C_CDD_EXPORT int g_force_parse_tokens_fail = 0;
/** @brief Test definition g_force_tokenize_fail. */
C_CDD_EXPORT int g_force_tokenize_fail = 0;
/** @brief Test definition g_cdd_fail_token_eq_str. */
C_CDD_EXPORT int g_cdd_fail_token_eq_str = 0;
/** @brief Test definition g_cdd_fail_propagate. */
C_CDD_EXPORT int g_cdd_fail_propagate = 0;
/** @brief Test definition g_cdd_fail_extract_func_name. */
C_CDD_EXPORT int g_cdd_fail_extract_func_name = 0;
/** @brief Test definition g_cdd_fail_concat_strings. */
C_CDD_EXPORT int g_cdd_fail_concat_strings = 0;
/** @brief Test definition g_cdd_fail_local_alloc_init. */
C_CDD_EXPORT int g_cdd_fail_local_alloc_init = 0;
/** @brief Test definition g_cdd_fail_get_token_slice. */
C_CDD_EXPORT int g_cdd_fail_get_token_slice = 0;
/** @brief Test definition g_cdd_fail_json_serialize. */
C_CDD_EXPORT int g_cdd_fail_json_serialize = 0;
/** @brief Test definition g_cdd_fail_type_def_list_init. */
C_CDD_EXPORT int g_cdd_fail_type_def_list_init = 0;
/** @brief Test definition g_cdd_fail_check_is_void. */
C_CDD_EXPORT int g_cdd_fail_check_is_void = 0;
/** @brief Test definition g_cdd_fail_audit_stats_init. */
C_CDD_EXPORT int g_cdd_fail_audit_stats_init = 0;
/** @brief Test definition g_mapping_fail_init. */
C_CDD_EXPORT int g_mapping_fail_init = 0;
/** @brief Test definition g_doc_fail_parse_optional_bool_attr. */
C_CDD_EXPORT int g_doc_fail_parse_optional_bool_attr = 0;
/** @brief Test definition g_doc_fail_skip_ws. */
C_CDD_EXPORT int g_doc_fail_skip_ws = 0;
/** @brief Test definition g_doc_fail_trim_segment. */
C_CDD_EXPORT int g_doc_fail_trim_segment = 0;
/** @brief Test definition g_doc_fail_sec_text. */
C_CDD_EXPORT int g_doc_fail_sec_text = 0;
/** @brief Test definition g_doc_fail_find_key_token. */
C_CDD_EXPORT int g_doc_fail_find_key_token = 0;
/** @brief Test definition g_cdd_pp_match_fail. */
C_CDD_EXPORT int g_cdd_pp_match_fail = 0;
/** @brief Test definition g_cdd_pp_skip_ws_fail. */
C_CDD_EXPORT int g_cdd_pp_skip_ws_fail = 0;
/** @brief Test definition g_cdd_pp_eval_expr_fail. */
C_CDD_EXPORT int g_cdd_pp_eval_expr_fail = 0;
/** @brief Test definition g_cdd_pp_is_defined_macro_fail. */
C_CDD_EXPORT int g_cdd_pp_is_defined_macro_fail = 0;
/** @brief Test definition g_cdd_pp_peek_fail. */
C_CDD_EXPORT int g_cdd_pp_peek_fail = 0;
/** @brief Test definition g_cdd_pp_primary_fail. */
C_CDD_EXPORT int g_cdd_pp_primary_fail = 0;
/** @brief Test definition g_cdd_pp_unary_fail. */
C_CDD_EXPORT int g_cdd_pp_unary_fail = 0;
/** @brief Test definition g_cdd_pp_multiplicative_fail. */
C_CDD_EXPORT int g_cdd_pp_multiplicative_fail = 0;
/** @brief Test definition g_cdd_pp_additive_fail. */
C_CDD_EXPORT int g_cdd_pp_additive_fail = 0;
/** @brief Test definition g_cdd_pp_shift_fail. */
C_CDD_EXPORT int g_cdd_pp_shift_fail = 0;
/** @brief Test definition g_cdd_pp_relational_fail. */
C_CDD_EXPORT int g_cdd_pp_relational_fail = 0;
/** @brief Test definition g_cdd_pp_equality_fail. */
C_CDD_EXPORT int g_cdd_pp_equality_fail = 0;
/** @brief Test definition g_cdd_pp_logic_and_fail. */
C_CDD_EXPORT int g_cdd_pp_logic_and_fail = 0;
/** @brief Test definition g_cdd_fail_identify_keyword_or_id. */
C_CDD_EXPORT int g_cdd_fail_identify_keyword_or_id = 0;
/** @brief Test definition g_cdd_fail_json_set_value. */
C_CDD_EXPORT volatile int g_cdd_fail_json_set_value = 0;
/** @brief Test definition g_cdd_fail_find_schema_by_anchor. */
C_CDD_EXPORT int g_cdd_fail_find_schema_by_anchor = 0;
/** @brief Test definition g_cdd_fail_find_schema_by_id. */
C_CDD_EXPORT int g_cdd_fail_find_schema_by_id = 0;
