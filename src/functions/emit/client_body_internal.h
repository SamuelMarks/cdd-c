/**
 * @file client_body_internal.h
 * @brief Internal declarations, definitions, and macros for client body
 * generation.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CLIENT_BODY_INTERNAL_H
#define C_CDD_CLIENT_BODY_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/safe_crt.h"
#include "c_cdd_stdbool.h"
#include "classes/emit/struct.h"
#include "functions/emit/client_body.h"
#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/security.h"
#include "routes/emit/url.h"
#include "win_compat_sym.h"
/* clang-format on */

#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#define verb_to_enum_str client_body_verb_to_enum_str
#define method_str_to_enum_str client_body_method_str_to_enum_str
#define mapped_err_code client_body_mapped_err_code
#define find_media_type client_body_find_media_type
#define find_encoding client_body_find_encoding
#define is_primitive_type client_body_is_primitive_type
#define is_object_ref_type client_body_is_object_ref_type
#define struct_fields_all_primitive client_body_struct_fields_all_primitive
#define schema_has_inline client_body_schema_has_inline
#define media_type_base_len client_body_media_type_base_len
#define media_type_has_prefix client_body_media_type_has_prefix
#define media_type_has_suffix client_body_media_type_has_suffix
#define media_type_ieq client_body_media_type_ieq
#define media_type_is_json client_body_media_type_is_json
#define media_type_is_form client_body_media_type_is_form
#define media_type_is_text_plain client_body_media_type_is_text_plain
#define media_type_is_multipart client_body_media_type_is_multipart
#define media_type_is_multipart_form client_body_media_type_is_multipart_form
#define first_content_type_entry client_body_first_content_type_entry
#define sanitize_ident client_body_sanitize_ident
#define multipart_header_param_name client_body_multipart_header_param_name
#define header_name_is_content_type client_body_header_name_is_content_type
#define media_type_is_textual client_body_media_type_is_textual
#define media_type_is_binary client_body_media_type_is_binary
#define schema_inline_is_string client_body_schema_inline_is_string
#define response_is_textual_string client_body_response_is_textual_string
#define response_is_binary client_body_response_is_binary
#define schema_has_payload client_body_schema_has_payload
#define write_text_plain_success client_body_write_text_plain_success
#define write_binary_success client_body_write_binary_success
#define write_inline_json_parse client_body_write_inline_json_parse
#define write_joined_form_array client_body_write_joined_form_array
#define write_form_urlencoded_body client_body_write_form_urlencoded_body
#define write_cookie_param_logic client_body_write_cookie_param_logic
#define write_header_param_logic client_body_write_header_param_logic
#define write_multipart_part_headers client_body_write_multipart_part_headers
#define write_multipart_body client_body_write_multipart_body
#define is_status_range_code client_body_is_status_range_code
#define status_range_prefix client_body_status_range_prefix
#define is_status_code_literal client_body_is_status_code_literal

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CLIENT_BODY_INTERNAL_H */
