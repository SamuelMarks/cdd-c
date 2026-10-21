/**
 * @file g_fail_io_after.c
 * @brief Unit tests and mocks in g_fail_io_after.c.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/suppress_gui.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <c89stringutils_string_extras.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
/** @brief Disable CRT redirection macros inside the CRT bridge implementation */
#define CDD_CRT_BRIDGE_NO_MACROS
#include "../cdd_test_helpers/cdd_crt_bridge.h"
/**
 * @brief Macro c89stringutils_jasprintf.
 */
#define c89stringutils_jasprintf jasprintf
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

/**
 * @brief Initializes the CRT inside c_cdd to suppress GUI assertion dialogs.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_init_runtime(void) {
  return cdd_suppress_gui_popups();
}

/**
 * @brief Allocates memory on c_cdd's CRT heap.
 * @param[in] sz Number of bytes to allocate.
 * @param[out] out_ptr Output pointer receiving allocated memory.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_malloc(size_t sz, void **out_ptr) {
  if (!out_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ptr = malloc(sz);
  return *out_ptr ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Zero-allocates memory on c_cdd's CRT heap.
 * @param[in] n Number of elements.
 * @param[in] sz Size of each element.
 * @param[out] out_ptr Output pointer receiving allocated memory.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_calloc(size_t n, size_t sz,
                                            void **out_ptr) {
  if (!out_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ptr = calloc(n, sz);
  return *out_ptr ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Reallocates memory on c_cdd's CRT heap.
 * @param[in,out] p Existing pointer.
 * @param[in] sz New size in bytes.
 * @param[out] out_ptr Output pointer receiving resized memory.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_realloc(void *p, size_t sz,
                                             void **out_ptr) {
  if (!out_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ptr = realloc(p, sz);
  return *out_ptr ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Duplicates a string on c_cdd's CRT heap.
 * @param[in] s Source string.
 * @param[out] out_str Output pointer receiving duplicated string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_strdup(const char *s, char **out_str) {
  size_t len;
  char *dup;
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!s) {
    *out_str = NULL;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  len = strlen(s) + 1;
  dup = (char *)malloc(len);
  if (dup)
    memcpy(dup, s, len);
  *out_str = dup;
  return dup ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Frees memory on c_cdd's CRT heap.
 * @param[in,out] p Pointer to free.
 */
C_CDD_EXPORT void c_cdd_crt_free(void *p) { free(p); }

/**
 * @brief Opens a file stream in c_cdd's CRT file table.
 * @param[in] path File path.
 * @param[in] mode Access mode.
 * @param[out] out_fp Output FILE pointer.
 * @param[out] out_err Output errno value.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fopen(const char *path, const char *mode,
                                           FILE **out_fp, int *out_err) {
  FILE *f = NULL;
  int err = 0;
  if (!out_fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_fp = NULL;
  if (!path || !mode) {
    if (out_err)
      *out_err = EINVAL;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#if defined(_MSC_VER)
  err = (int)fopen_s(&f, path, mode);
  if (err != 0)
    f = NULL;
#else
  errno = 0;
  f = fopen(path, mode);
  if (!f)
    err = errno;
#endif
  if (out_err)
    *out_err = err;
  *out_fp = f;
  return f ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Closes a file stream in c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[out] out_rc Output fclose return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fclose(FILE *fp, int *out_rc) {
  int rc;
  if (!fp) {
    if (out_rc)
      *out_rc = EOF;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = fclose(fp);
  if (out_rc)
    *out_rc = rc;
  return (rc == 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Seeks within a file stream in c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[in] offset Byte offset.
 * @param[in] origin Seek origin.
 * @param[out] out_rc Output fseek return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fseek(FILE *fp, long offset, int origin,
                                           int *out_rc) {
  int rc;
  if (!fp) {
    if (out_rc)
      *out_rc = -1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = fseek(fp, offset, origin);
  if (out_rc)
    *out_rc = rc;
  return (rc == 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Queries current position of a file stream in c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[out] out_pos Output position.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_ftell(FILE *fp, long *out_pos) {
  long pos;
  if (!fp || !out_pos)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  pos = ftell(fp);
  *out_pos = pos;
  return (pos >= 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Rewinds a file stream in c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_rewind(FILE *fp) {
  if (!fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rewind(fp);
  return CDD_C_SUCCESS;
}

/**
 * @brief Reads from a file stream in c_cdd's CRT file table.
 * @param[out] buf Destination buffer.
 * @param[in] sz Element size.
 * @param[in] n Element count.
 * @param[in,out] fp File stream.
 * @param[out] out_read Output elements read.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fread(void *buf, size_t sz, size_t n,
                                           FILE *fp, size_t *out_read) {
  size_t r;
  if (!buf || !fp || !out_read)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  r = fread(buf, sz, n, fp);
  *out_read = r;
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes to a file stream in c_cdd's CRT file table.
 * @param[in] buf Source buffer.
 * @param[in] sz Element size.
 * @param[in] n Element count.
 * @param[in,out] fp File stream.
 * @param[out] out_written Output elements written.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fwrite(const void *buf, size_t sz,
                                            size_t n, FILE *fp,
                                            size_t *out_written) {
  size_t w;
  if (!buf || !fp || !out_written)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  w = fwrite(buf, sz, n, fp);
  *out_written = w;
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes a string to a file stream in c_cdd's CRT file table.
 * @param[in] s String to write.
 * @param[in,out] fp File stream.
 * @param[out] out_rc Output fputs return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fputs(const char *s, FILE *fp,
                                           int *out_rc) {
  int rc;
  if (!s || !fp) {
    if (out_rc)
      *out_rc = EOF;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = fputs(s, fp);
  if (out_rc)
    *out_rc = rc;
  return (rc >= 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Reads a line from a file stream in c_cdd's CRT file table.
 * @param[out] s Destination buffer.
 * @param[in] n Buffer capacity.
 * @param[in,out] fp File stream.
 * @param[out] out_res Output pointer result.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fgets(char *s, int n, FILE *fp,
                                           char **out_res) {
  if (!s || !fp || !out_res)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_res = fgets(s, n, fp);
  return CDD_C_SUCCESS;
}

/**
 * @brief Flushes a file stream in c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[out] out_rc Output fflush return code.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fflush(FILE *fp, int *out_rc) {
  int rc = fflush(fp);
  if (out_rc)
    *out_rc = rc;
  return (rc == 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Clears error indicators on a file stream in c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_clearerr(FILE *fp) {
  if (!fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  clearerr(fp);
  return CDD_C_SUCCESS;
}

/**
 * @brief Formatted print to a file stream in c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[in] fmt Format string.
 * @param[in] args Variadic argument list.
 * @param[out] out_rc Output vfprintf return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 0)))
#endif
C_CDD_EXPORT cdd_c_error_t
c_cdd_crt_vfprintf(FILE *fp, const char *fmt, va_list args, int *out_rc) {
  int rc;
  if (!fp || !fmt) {
    if (out_rc)
      *out_rc = -1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = vfprintf(fp, fmt, args);
  if (out_rc)
    *out_rc = rc;
  return (rc >= 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Formatted string allocation on c_cdd's CRT heap via c89stringutils.
 * @param[out] out_str Output allocated string.
 * @param[in] fmt Format string.
 * @param[in] args Variadic argument list.
 * @param[out] out_rc Output vasprintf return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 0)))
#endif
C_CDD_EXPORT cdd_c_error_t
c_cdd_crt_vasprintf(char **out_str, const char *fmt, va_list args,
                    int *out_rc) {
  int rc;
  if (!out_str || !fmt) {
    if (out_rc)
      *out_rc = -1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = c89stringutils_vasprintf(out_str, fmt, args);
  if (out_rc)
    *out_rc = rc;
  return (rc >= 0) ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Appends a string on c_cdd's CRT heap via c89stringutils_jasprintf.
 * @param[in,out] unto Target string pointer.
 * @param[in] str String to append.
 * @param[out] out_rc Output jasprintf return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_jasprintf(char **unto, const char *str,
                                               int *out_rc) {
  int rc;
  if (!unto) {
    if (out_rc)
      *out_rc = -1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = c89stringutils_jasprintf(unto, "%s", str ? str : "");
  if (out_rc)
    *out_rc = rc;
  return (rc >= 0) ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Updates an environment variable in c_cdd's CRT environment block.
 * @param[in] envstr "KEY=VALUE" string.
 * @param[out] out_rc Output _putenv return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_putenv(const char *envstr, int *out_rc) {
  int rc = -1;
  if (!envstr) {
    if (out_rc)
      *out_rc = -1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#if defined(_MSC_VER)
  rc = _putenv(envstr);
#elif defined(_WIN32)
  rc = putenv(envstr);
#else
  rc = (envstr[0] != '\0') ? 0 : -1;
#endif
  if (out_rc)
    *out_rc = rc;
  return (rc == 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Retrieves stdin (0), stdout (1), or stderr (2) from c_cdd's CRT.
 * @param[in] idx Stream index (0=stdin, 1=stdout, 2=stderr).
 * @param[out] out_fp Output FILE pointer.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_std_stream(int idx, FILE **out_fp) {
  if (!out_fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (idx == 0) {
    *out_fp = stdin;
    return CDD_C_SUCCESS;
  }
  if (idx == 1) {
    *out_fp = stdout;
    return CDD_C_SUCCESS;
  }
  if (idx == 2) {
    *out_fp = stderr;
    return CDD_C_SUCCESS;
  }
  *out_fp = NULL;
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Reopens a file stream in c_cdd's CRT file table.
 * @param[in] path File path.
 * @param[in] mode Access mode.
 * @param[in,out] stream Existing FILE stream.
 * @param[out] out_fp Output FILE pointer.
 * @param[out] out_err Output errno value.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_freopen(const char *path, const char *mode,
                                             FILE *stream, FILE **out_fp,
                                             int *out_err) {
  FILE *f = NULL;
  int err = 0;
  if (!out_fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_fp = NULL;
  if (!path || !mode || !stream) {
    if (out_err)
      *out_err = EINVAL;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#if defined(_MSC_VER)
  err = (int)freopen_s(&f, path, mode, stream);
  if (err != 0)
    f = NULL;
#else
  errno = 0;
  f = freopen(path, mode, stream);
  if (!f)
    err = errno;
#endif
  if (out_err)
    *out_err = err;
  *out_fp = f;
  return f ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Gets stream position indicator in c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[out] pos Output fpos_t.
 * @param[out] out_rc Output fgetpos return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fgetpos(FILE *fp, fpos_t *pos,
                                             int *out_rc) {
  int rc;
  if (!fp || !pos) {
    if (out_rc)
      *out_rc = -1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = fgetpos(fp, pos);
  if (out_rc)
    *out_rc = rc;
  return (rc == 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Sets stream position indicator in c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[in] pos Input fpos_t.
 * @param[out] out_rc Output fsetpos return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fsetpos(FILE *fp, const fpos_t *pos,
                                             int *out_rc) {
  int rc;
  if (!fp || !pos) {
    if (out_rc)
      *out_rc = -1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = fsetpos(fp, pos);
  if (out_rc)
    *out_rc = rc;
  return (rc == 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Gets the low-level file descriptor of a stream in c_cdd's CRT.
 * @param[in] fp File stream.
 * @param[out] out_fd Output file descriptor.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fileno(FILE *fp, int *out_fd) {
  if (!fp || !out_fd)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#if defined(_MSC_VER)
  *out_fd = _fileno(fp);
#else
  *out_fd = fileno(fp);
#endif
  return (*out_fd >= 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Duplicates a file descriptor in c_cdd's CRT file table.
 * @param[in] fd File descriptor to duplicate.
 * @param[out] out_fd Output duplicated file descriptor.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_dup(int fd, int *out_fd) {
  if (!out_fd)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#if defined(_MSC_VER)
  *out_fd = _dup(fd);
#else
  *out_fd = dup(fd);
#endif
  return (*out_fd >= 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Reassigns a file descriptor in c_cdd's CRT file table.
 * @param[in] fd1 Source file descriptor.
 * @param[in] fd2 Target file descriptor.
 * @param[out] out_rc Output _dup2 return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_dup2(int fd1, int fd2, int *out_rc) {
  int rc = -1;
#if defined(_MSC_VER)
  rc = _dup2(fd1, fd2);
#else
  rc = dup2(fd1, fd2);
#endif
  if (out_rc)
    *out_rc = rc;
  return (rc >= 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Closes a low-level file descriptor in c_cdd's CRT file table.
 * @param[in] fd File descriptor.
 * @param[out] out_rc Output _close return code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_crt_close(int fd, int *out_rc) {
  int rc = -1;
#if defined(_MSC_VER)
  rc = _close(fd);
#else
  rc = close(fd);
#endif
  if (out_rc)
    *out_rc = rc;
  return (rc == 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}
