/**
 * @file test_codegen_client_sig_common.h
 * @brief Common test helpers and fixtures for client signature generation
 * tests.
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_COMMON_H
#define TEST_CODEGEN_CLIENT_SIG_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers_export.h"
#include "functions/emit/client_sig.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);

extern C_CDD_EXPORT int g_cdd_fail_media_type_base_len;
extern C_CDD_EXPORT int g_cdd_fail_media_type_has_suffix;
extern C_CDD_EXPORT int g_cdd_fail_media_type_ieq;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_json;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_text_plain;
extern C_CDD_EXPORT int g_cdd_fail_media_type_has_prefix;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_form;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_multipart;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_textual;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_binary;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_multipart_form;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_array_item_type;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_array_item_ref;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_prim;
extern C_CDD_EXPORT int g_cdd_fail_qs_raw;
extern C_CDD_EXPORT int g_cdd_fail_qs_form_obj;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_ref;
extern C_CDD_EXPORT int g_cdd_fail_map_array_item_type;
extern C_CDD_EXPORT int g_cdd_fail_sanitize_ident;
extern C_CDD_EXPORT int g_cdd_fail_map_type_to_c_arg;
extern C_CDD_EXPORT int g_cdd_fail_is_primitive_type;
extern C_CDD_EXPORT int g_cdd_fail_param_is_object_kv;
extern C_CDD_EXPORT int g_cdd_fail_find_media_type;
extern C_CDD_EXPORT int g_cdd_fail_header_name_is_content_type;
extern C_CDD_EXPORT int g_cdd_fail_c_cdd_str_iequal;
extern C_CDD_EXPORT int g_cdd_fail_multipart_header_param_name;
extern C_CDD_EXPORT int g_cdd_fail_response_is_binary_success;
extern C_CDD_EXPORT int g_cdd_fail_get_success_schema;
extern C_CDD_EXPORT int g_cdd_fail_get_success_response;
extern C_CDD_EXPORT int g_cdd_fail_schema_has_inline;
extern C_CDD_EXPORT int g_cdd_fail_map_type_to_c_out;
extern C_CDD_EXPORT int g_cdd_fail_map_array_item_type_out;

static int g_sig_fail_tmpfile = 0;

static cdd_c_error_t gen_sig(const struct OpenAPI_Operation *op,
                             const struct CodegenSigConfig *cfg,
                             char **_out_val) {
  FILE *tmp;
  if (g_sig_fail_tmpfile) {
    tmp = NULL;
  } else {
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
  }
  {
    long sz;
    char *content = NULL;

    cdd_c_error_t rc;

    if (!tmp) {
      *_out_val = NULL;
      return CDD_C_ERROR_IO;
    }

    rc = codegen_client_write_signature(tmp, op, cfg);
    if (rc != CDD_C_SUCCESS) {
      if (tmp)
        fclose(tmp);
      *_out_val = NULL;
      return rc;
    }

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);

    content = (char *)(size_t)(size_t)(size_t)calloc(1, (size_t)sz + 1);
    if (sz > 0)
      if (fread(content, 1, (size_t)sz, tmp)) {
      }

    if (tmp)
      fclose(tmp);
    {
      *_out_val = content;
      return CDD_C_SUCCESS;
    }
  }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_SIG_COMMON_H */
