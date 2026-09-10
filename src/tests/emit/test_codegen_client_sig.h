#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
#include "routes/emit/client_gen.h"
/**
 * @file test_codegen_client_sig.h
 * @brief Unit tests for C Client Signature Generation.
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_H
#define TEST_CODEGEN_CLIENT_SIG_H

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

#include "functions/emit/client_sig.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

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

static cdd_c_error_t gen_sig(const struct OpenAPI_Operation *op,
                             const struct CodegenSigConfig *cfg,
                             char **_out_val) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
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
      return 0;
    }
  }
}

TEST test_sig_simple_get(void) {
  char *_ast_gen_sig_0 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "get_pet";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "id";
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";
  op.parameters = &param;
  op.n_parameters = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "Pet";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_0), _ast_gen_sig_0);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  /* Verify standard signature including ApiError */
  ASSERT(strstr(code,
                ""
                "int get_pet(struct HttpClient *ctx, int id, struct Pet **out, "
                "struct ApiError **api_error) {"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_verify_apierror(void) {
  char *_ast_gen_sig_1 = NULL;
  struct OpenAPI_Operation op = {0};
  char *code;
  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "do";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_1), _ast_gen_sig_1);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, ", struct ApiError **api_error)"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_grouped(void) {
  char *_ast_gen_sig_2 = NULL;
  struct OpenAPI_Operation op = {0};
  struct CodegenSigConfig cfg = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "getById";

  cfg.prefix = (char *)(size_t)(size_t)(size_t)(size_t) "api_";
  cfg.group_name = (char *)(size_t)(size_t)(size_t)(size_t) "Pet";

  code = (gen_sig(&op, &cfg, &_ast_gen_sig_2), _ast_gen_sig_2);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, &cfg, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, &cfg, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  /* Expect: Pet_api_getById */
  ASSERT(strstr(code, "int Pet_api_getById(struct HttpClient *ctx"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_success_range_response(void) {
  char *_ast_gen_sig_3 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "listPets";

  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "2XX";
  resp.schema.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "Pet";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_3), _ast_gen_sig_3);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "struct Pet **out") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_default_response_success(void) {
  char *_ast_gen_sig_4 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "defaultPet";

  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "default";
  resp.schema.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "Pet";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_4), _ast_gen_sig_4);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "struct Pet **out") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_inline_response_string(void) {
  char *_ast_gen_sig_5 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "getInline";

  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  resp.schema.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "string";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_5), _ast_gen_sig_5);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "char **out") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_inline_response_array(void) {
  char *_ast_gen_sig_6 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "getInlineArr";

  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  resp.schema.is_array = 1;
  resp.schema.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_6), _ast_gen_sig_6);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "int **out, size_t *out_len") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_inline_request_body_string(void) {
  char *_ast_gen_sig_7 = NULL;
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "postInline";
  op.req_body.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/json";
  op.req_body.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "string";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_7), _ast_gen_sig_7);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const char *req_body") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_inline_request_body_array(void) {
  char *_ast_gen_sig_8 = NULL;
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "postInlineArr";
  op.req_body.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/json";
  op.req_body.is_array = 1;
  op.req_body.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "number";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_8), _ast_gen_sig_8);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const double *body, size_t body_len") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_multipart_encoding_headers(void) {
  char *_ast_gen_sig_9 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_MediaType mt = {0};
  struct OpenAPI_Encoding enc = {0};
  struct OpenAPI_Header headers[3] = {{0}};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "upload";

  op.req_body.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "Upload";
  op.req_body.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "multipart/form-data";

  headers[0].name = (char *)(size_t)(size_t)(size_t)(size_t) "X-Trace";
  headers[0].type = (char *)(size_t)(size_t)(size_t)(size_t) "string";
  headers[1].name = (char *)(size_t)(size_t)(size_t)(size_t) "X-Ids";
  headers[1].type = (char *)(size_t)(size_t)(size_t)(size_t) "array";
  headers[1].is_array = 1;
  headers[1].items_type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";
  headers[2].name = (char *)(size_t)(size_t)(size_t)(size_t) "Content-Type";
  headers[2].type = (char *)(size_t)(size_t)(size_t)(size_t) "string";

  mt.name = (char *)(size_t)(size_t)(size_t)(size_t) "multipart/form-data";
  enc.name = (char *)(size_t)(size_t)(size_t)(size_t) "file";
  enc.headers = headers;
  enc.n_headers = 3;
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_9), _ast_gen_sig_9);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const char *file_hdr_X_Trace") != NULL);
  ASSERT(strstr(code, "const int *file_hdr_X_Ids, size_t file_hdr_X_Ids_len") !=
         NULL);
  ASSERT(strstr(code, "file_hdr_Content_Type") == NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_text_plain_request_body(void) {
  char *_ast_gen_sig_10 = NULL;
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "postText";
  op.req_body.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "text/plain";
  op.req_body.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "string";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_10), _ast_gen_sig_10);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const char *req_body") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_textual_request_body_xml(void) {
  char *_ast_gen_sig_11 = NULL;
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "postXml";
  op.req_body.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/xml";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_11), _ast_gen_sig_11);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const char *req_body") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_octet_stream_request_body(void) {
  char *_ast_gen_sig_12 = NULL;
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "postBinary";
  op.req_body.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/octet-stream";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_12), _ast_gen_sig_12);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const unsigned char *body, size_t body_len") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_binary_request_body_pdf(void) {
  char *_ast_gen_sig_13 = NULL;
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "postPdf";
  op.req_body.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/pdf";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_13), _ast_gen_sig_13);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const unsigned char *body, size_t body_len") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_octet_stream_response_body(void) {
  char *_ast_gen_sig_14 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "download";
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  resp.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/octet-stream";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_14), _ast_gen_sig_14);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "unsigned char **out, size_t *out_len") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_binary_response_body_pdf(void) {
  char *_ast_gen_sig_15 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "downloadPdf";
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  resp.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/pdf";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_15), _ast_gen_sig_15);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "unsigned char **out, size_t *out_len") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_querystring_form_object(void) {
  char *_ast_gen_sig_16 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "search";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "object";
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/"
                                               "x-www-form-urlencoded";
  param.schema.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "object";

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_16), _ast_gen_sig_16);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "qs") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_querystring_json_ref(void) {
  char *_ast_gen_sig_17 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "searchJson";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "object";
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/json";
  param.schema.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "Pet";

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_17), _ast_gen_sig_17);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "qs") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_querystring_json_primitive(void) {
  char *_ast_gen_sig_18 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "searchJsonInt";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/json";
  param.schema.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_18), _ast_gen_sig_18);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "qs") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_querystring_json_array(void) {
  char *_ast_gen_sig_19 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "searchJsonTags";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "array";
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/json";
  param.schema.is_array = 1;
  param.schema.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "string";

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_19), _ast_gen_sig_19);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code,
                ""
                "int searchJsonTags(struct HttpClient *ctx, const char **qs, "
                "size_t qs_len, struct ApiError **api_error) {") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_querystring_json_array_object(void) {
  char *_ast_gen_sig_20 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "searchJsonPets";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "array";
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/json";
  param.schema.is_array = 1;
  param.items_type = (char *)(size_t)(size_t)(size_t)(size_t) "Pet";

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_20), _ast_gen_sig_20);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code,
                ""
                "int searchJsonPets(struct HttpClient *ctx, const struct Pet "
                "**qs, size_t qs_len, struct ApiError **api_error) {") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_querystring_raw_string(void) {
  char *_ast_gen_sig_21 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "searchRaw";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "string";
  param.content_type = (char *)(size_t)(size_t)(size_t)(size_t) "text/plain";
  param.schema.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "string";

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_21), _ast_gen_sig_21);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "qs") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_querystring_raw_integer(void) {
  char *_ast_gen_sig_22 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "searchRawInt";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/jsonpath";
  param.schema.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_22), _ast_gen_sig_22);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "qs") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_query_object_param_kv(void) {
  char *_ast_gen_sig_23 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "list";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "filter";
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "object";
  param.in = OA_PARAM_IN_QUERY;

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_23), _ast_gen_sig_23);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_path_object_param_kv(void) {
  char *_ast_gen_sig_24 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "byPath";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "filter";
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "object";
  param.in = OA_PARAM_IN_PATH;

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_24), _ast_gen_sig_24);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_header_object_param_kv(void) {
  char *_ast_gen_sig_25 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "byHeader";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "filter";
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "object";
  param.in = OA_PARAM_IN_HEADER;

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_25), _ast_gen_sig_25);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_cookie_object_param_kv(void) {
  char *_ast_gen_sig_26 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "byCookie";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "prefs";
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "object";
  param.in = OA_PARAM_IN_COOKIE;

  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_26), _ast_gen_sig_26);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const struct OpenAPI_KV *prefs, size_t prefs_len") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_json_content_query_ref(void) {
  char *_ast_gen_sig_27 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "list";

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/json";
  param.schema.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "Filter";
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "Filter";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_sig(&op, NULL, &_ast_gen_sig_27), _ast_gen_sig_27);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "const struct Filter *filter") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_header_param_boolean(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_sig_0_uniq = NULL;

  (void)openapi_spec_init(&spec);
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "X-Bool";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "boolean";
  op.parameters = &param;
  op.n_parameters = 1;

  gen_sig(&op, NULL, &code);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      (void)_ast_gen_sig_0_uniq;
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "int X-Bool") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_header_param_number(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_sig_5_uniq = NULL;

  (void)openapi_spec_init(&spec);
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "X-Num";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "number";
  op.parameters = &param;
  op.n_parameters = 1;

  gen_sig(&op, NULL, &code);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      (void)_ast_gen_sig_5_uniq;
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "double X-Num") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_header_param_integer(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_sig_10_uniq = NULL;

  (void)openapi_spec_init(&spec);
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "X-Int";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";
  op.parameters = &param;
  op.n_parameters = 1;

  gen_sig(&op, NULL, &code);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      (void)_ast_gen_sig_10_uniq;
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "int X-Int") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_header_param_string(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_sig_15_uniq = NULL;

  (void)openapi_spec_init(&spec);
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "X-String";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "string";
  op.parameters = &param;
  op.n_parameters = 1;

  gen_sig(&op, NULL, &code);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      (void)_ast_gen_sig_15_uniq;
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "char *X-String") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sig_header_param_json(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code = NULL;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "testHeaderJson";
  op.n_parameters = 1;
  op.parameters = &param;

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "X-MyHeader";
  param.in = OA_PARAM_IN_HEADER;
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "application/json";

  /* Test primitive JSON type */
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test primitive JSON array */
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "array";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test non-primitive object */
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "object";
  param.is_array = 0;
  param.items_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test non-primitive array */
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "array";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t)(size_t)(size_t) "MyType";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test ref */
  param.type = NULL;
  param.is_array = 0;
  param.schema.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "MyType";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test inline object array */
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "array";
  param.is_array = 1;
  param.schema.ref_name = NULL;
  param.items_type = (char *)(size_t)(size_t)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  PASS();
}
TEST test_sig_media_type_branches(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_MediaType mt;
  char *code = NULL;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(&mt, 0, sizeof(mt));

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "testMediaTypes";
  op.n_parameters = 1;
  op.parameters = &param;

  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "myParam";
  param.in = OA_PARAM_IN_QUERY;

  /* upper case testing */
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "APPLICATION/JSON";
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* text/plain */
  param.content_type = (char *)(size_t)(size_t)(size_t)(size_t) "TEXT/PLAIN";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* application/xml */
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "APPLICATION/XML";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* multipart/form-data */
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "MULTIPART/FORM-DATA";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* application/x-www-form-urlencoded */
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "APPLICATION/"
                                               "X-WWW-FORM-URLENCODED";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* default fallback */
  param.content_type = (char *)(size_t)(size_t)(size_t)(size_t) "UNKNOWN/TYPE";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  PASS();
}

TEST test_sig_response_array_string_ref(void) {
  char *_ast_gen_sig_20_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;
  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "getArrStr";
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  resp.schema.is_array = 1;
  resp.schema.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "string";
  op.responses = &resp;
  op.n_responses = 1;
  gen_sig(&op, NULL, &code);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      (void)_ast_gen_sig_20_uniq;
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "char ***out, size_t *out_len") != NULL);
  free(code);
  PASS();
}

TEST test_sig_response_array_integer_ref(void) {
  char *_ast_gen_sig_25_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;
  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "getArrInt";
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  resp.schema.is_array = 1;
  resp.schema.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "integer";
  op.responses = &resp;
  op.n_responses = 1;
  gen_sig(&op, NULL, &code);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      (void)_ast_gen_sig_25_uniq;
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "int **out, size_t *out_len") != NULL);
  free(code);
  PASS();
}

TEST test_sig_response_array_struct_ref(void) {
  char *_ast_gen_sig_30_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;
  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "getArrStruct";
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  resp.schema.is_array = 1;
  resp.schema.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "Pet";
  op.responses = &resp;
  op.n_responses = 1;
  gen_sig(&op, NULL, &code);
  ASSERT(code);
  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      (void)_ast_gen_sig_30_uniq;
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  {
    int io_i;
    for (io_i = 0; io_i < 50; ++io_i) {
      char *io_code = NULL;
      g_io_calls = 0;
      g_fail_io_after = io_i;
      if (gen_sig(&op, NULL, &io_code) == CDD_C_SUCCESS) {
        if (io_code)
          free(io_code);
        break;
      }
      if (io_code)
        free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "struct Pet ***out, size_t *out_len") != NULL);
  free(code);
  PASS();
}

TEST test_sig_null_args(void) {
  ASSERT(codegen_client_write_signature(NULL, NULL, NULL) ==
         CDD_C_ERROR_INVALID_ARGUMENT);
  PASS();
}

TEST test_sig_io_errors(void) {
  int i;
  int success_count = 0;
  char *_ast_gen_sig_35_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  char *code = NULL;

  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "getAllBranches";
  op.n_responses = 1;
  op.responses = &resp;
  resp.code = (char *)(size_t)(size_t)(size_t)(size_t) "200";
  resp.schema.is_array = 1;
  resp.schema.ref_name = (char *)(size_t)(size_t)(size_t)(size_t) "Pet";

  op.n_parameters = 1;
  op.parameters = &param;
  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "myParam";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.schema.inline_type = (char *)(size_t)(size_t)(size_t)(size_t) "string";

  for (i = 0; i < 50; ++i) {
    if (g_io_calls > 0 && g_io_calls < i)
      break;
    g_io_calls = 0;
    g_fail_io_after = i;
    code = NULL;
    if (gen_sig(&op, NULL, &code) == CDD_C_SUCCESS) {
      if (code)
        free(code);
      success_count++;
      if (success_count > 1)
        break; /* Once it succeeds consistently, stop */
    } else {
      if (code)
        free(code);
    }
  }
  g_fail_io_after = -1;
  (void)_ast_gen_sig_35_uniq;
  PASS();
}

TEST test_sig_unsupported_prefix(void) {
  char *_ast_gen_sig_36_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;
  op.operation_id = (char *)(size_t)(size_t)(size_t)(size_t) "prefixTest";
  op.n_parameters = 1;
  op.parameters = &param;
  param.name = (char *)(size_t)(size_t)(size_t)(size_t) "myParam";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t)(size_t)(size_t) "string";
  param.content_type =
      (char *)(size_t)(size_t)(size_t)(size_t) "unsupported/type";
  gen_sig(&op, NULL, &code);
  ASSERT(code);
  free(code);
  (void)_ast_gen_sig_36_uniq;
  PASS();
}

TEST test_sig_helpers_coverage_1(void) {
  const char *str_val = NULL;
  int int_val = 0;
  size_t sz_val = 0;

  /* map_type_to_c_arg */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_map_type_to_c_arg("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg(NULL, &str_val));
  ASSERT_STR_EQ("const void *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("integer", &str_val));
  ASSERT_STR_EQ("int ", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("string", &str_val));
  ASSERT_STR_EQ("const char *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("boolean", &str_val));
  ASSERT_STR_EQ("int ", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("number", &str_val));
  ASSERT_STR_EQ("double ", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("other", &str_val));
  ASSERT_STR_EQ("const void *", str_val);

  /* is_primitive_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_is_primitive_type("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("integer", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("string", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("boolean", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("number", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("custom", &int_val));
  ASSERT_EQ(0, int_val);

  /* param_is_object_kv */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_param_is_object_kv(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  {
    struct OpenAPI_Parameter p;
    memset(&p, 0, sizeof(p));
    p.type = (char *)(size_t)(size_t) "string";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(0, int_val);

    p.type = (char *)(size_t)(size_t) "object";
    p.in = OA_PARAM_IN_QUERY;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(1, int_val);
    p.in = OA_PARAM_IN_PATH;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(1, int_val);
    p.in = OA_PARAM_IN_HEADER;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(1, int_val);
    p.in = OA_PARAM_IN_COOKIE;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(1, int_val);
    p.in = OA_PARAM_IN_QUERYSTRING;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(0, int_val);
    p.is_array = 1;
    p.in = OA_PARAM_IN_QUERY;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(0, int_val);
  }

  /* media_type_base_len */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_base_len("test", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_base_len(NULL, &sz_val));
  ASSERT_EQ((size_t)0, sz_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_base_len(
                               "application/json;charset=utf-8", &sz_val));
  ASSERT_EQ((size_t)16, sz_val);

  /* media_type_has_prefix */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_has_prefix("a", "b", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_prefix(NULL, "a", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_prefix("a", NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_has_prefix(
                               "multipart/form-data", "multipart/", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_has_prefix(
                               "text/plain", "multipart/", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_has_suffix */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_has_suffix("a", "b", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_suffix(NULL, "a", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_suffix("a", NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_suffix("a", "longersuffix", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_has_suffix(
                               "app+json;charset=utf8", "+json", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_suffix("app+xml", "+json", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_ieq */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_ieq("a", "b", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_ieq(NULL, "a", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_ieq("a", NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_ieq(
                               "text/plain", "text/plain;param", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_ieq("text/plain;charset=utf8", "text/plain",
                                        &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_ieq("text/plain", "text/html", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_json */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_is_json("a", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_json(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_json("application/json", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_json(
                               "application/vnd.api+json", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_json("text/plain", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_form */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_form(
                               "application/x-www-form-urlencoded", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_form("text/plain", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_text_plain */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_text_plain("text/plain", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_text_plain(
                               "application/json", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_multipart */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_multipart("multipart/mixed", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_multipart("text/plain", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_multipart_form */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_multipart_form(
                               "multipart/form-data", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_multipart_form(
                               "multipart/mixed", &int_val));
  ASSERT_EQ(0, int_val);

  PASS();
}

TEST test_sig_helpers_coverage_2(void) {
  const char *str_val = NULL;
  int int_val = 0;
  struct OpenAPI_MediaType mts[2];
  const struct OpenAPI_MediaType *found_mt = NULL;
  struct OpenAPI_Parameter p;

  /* find_media_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_find_media_type(NULL, 0, "name", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_find_media_type(NULL, 0, "name", &found_mt));
  ASSERT(found_mt == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_find_media_type(mts, 2, NULL, &found_mt));
  ASSERT(found_mt == NULL);
  memset(mts, 0, sizeof(mts));
  mts[0].name = (char *)(size_t)(size_t) "application/json";
  mts[1].name = (char *)(size_t)(size_t) "multipart/form-data";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_find_media_type(
                               mts, 2, "multipart/form-data", &found_mt));
  ASSERT(found_mt == &mts[1]);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_find_media_type(mts, 2, "text/plain", &found_mt));
  ASSERT(found_mt == NULL);

  /* media_type_is_textual */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_is_textual("text/plain", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_textual(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("text/plain", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("text/html", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("application/xml", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_textual(
                               "application/atom+xml", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_textual(
                               "application/octet-stream", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_binary */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_is_binary("bin", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_binary(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_binary("application/json", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_binary(
                               "application/x-www-form-urlencoded", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_binary("multipart/form-data", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_binary("text/plain", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_binary("application/pdf", &int_val));
  ASSERT_EQ(1, int_val);

  /* querystring_param_is_form_object */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_is_form_object(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_form_object(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  memset(&p, 0, sizeof(p));
  p.schema.ref_name = (char *)(size_t)(size_t) "MyRef";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_form_object(&p, &int_val));
  ASSERT_EQ(1, int_val);
  p.schema.ref_name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_form_object(&p, &int_val));
  ASSERT_EQ(0, int_val);

  /* querystring_param_is_json_ref */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_is_json_ref(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_json_ref(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  memset(&p, 0, sizeof(p));
  p.schema.ref_name = (char *)(size_t)(size_t) "MyRef";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_json_ref(&p, &int_val));
  ASSERT_EQ(1, int_val);
  p.schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_json_ref(&p, &int_val));
  ASSERT_EQ(0, int_val);
  p.schema.is_array = 0;
  p.type = (char *)(size_t)(size_t) "array";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_json_ref(&p, &int_val));
  ASSERT_EQ(0, int_val);

  /* querystring_param_json_primitive_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_json_primitive_type(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(NULL, &str_val));
  ASSERT(str_val == NULL);
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t)(size_t) "text/plain";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.content_type = (char *)(size_t)(size_t) "application/json";
  p.schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.schema.is_array = 0;
  p.type = (char *)(size_t)(size_t) "array";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.schema.inline_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("integer", str_val);
  p.schema.inline_type = (char *)(size_t)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.schema.inline_type = NULL;
  p.type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);
  p.type = (char *)(size_t)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("number", str_val);
  p.type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("boolean", str_val);

  /* querystring_param_json_array_item_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_json_array_item_type(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_type(
                               NULL, &str_val));
  ASSERT(str_val == NULL);
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t)(size_t) "application/json";
  p.is_array = 1;
  p.items_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT_STR_EQ("integer", str_val);
  p.items_type = (char *)(size_t)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.items_type = NULL;
  p.schema.inline_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);
  p.schema.inline_type = (char *)(size_t)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT_STR_EQ("number", str_val);
  p.schema.inline_type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT_STR_EQ("boolean", str_val);
  p.schema.inline_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.is_array = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT(str_val == NULL);

  /* querystring_param_json_array_item_ref */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_json_array_item_ref(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(NULL, &str_val));
  ASSERT(str_val == NULL);
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t)(size_t) "application/json";
  p.is_array = 1;
  p.items_type = (char *)(size_t)(size_t) "MyItemRef";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT_STR_EQ("MyItemRef", str_val);
  p.items_type = (char *)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT(str_val == NULL);
  p.items_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT(str_val == NULL);
  p.items_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT(str_val == NULL);

  /* querystring_param_raw_primitive_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_raw_primitive_type(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(NULL, &str_val));
  ASSERT(str_val == NULL);
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t)(size_t) "text/plain";
  p.type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("integer", str_val);
  p.type = (char *)(size_t)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("number", str_val);
  p.type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("boolean", str_val);
  p.type = (char *)(size_t)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);
  p.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);
  p.schema.inline_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);

  PASS();
}

TEST test_sig_helpers_coverage_3(void) {
  const char *str_val = NULL;
  int int_val = 0;
  char buf[64];
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp[3];
  const struct OpenAPI_Response *out_resp = NULL;
  const struct OpenAPI_SchemaRef *out_schema = NULL;
  struct OpenAPI_SchemaRef schema;

  /* map_array_item_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_map_array_item_type("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_array_item_type(NULL, &str_val));
  ASSERT_STR_EQ("const void *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("integer", &str_val));
  ASSERT_STR_EQ("const int *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("boolean", &str_val));
  ASSERT_STR_EQ("const int *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("string", &str_val));
  ASSERT_STR_EQ("const char **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("number", &str_val));
  ASSERT_STR_EQ("const double *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("custom", &str_val));
  ASSERT_STR_EQ("const void *", str_val);

  /* sanitize_ident */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_sanitize_ident(NULL, 10, "abc"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_sanitize_ident(buf, 0, "abc"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), "123-abc.xyz"));
  ASSERT_STR_EQ("_123_abc_xyz", buf);
  {
    char small_buf[2];
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_sanitize_ident(small_buf, 2, "1"));
    ASSERT_STR_EQ("_", small_buf);
  }

  /* multipart_header_param_name */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_multipart_header_param_name(NULL, 10, "f", "h"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_multipart_header_param_name(buf, 0, "f", "h"));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_test_sig_multipart_header_param_name(buf, sizeof(buf), NULL, "h"));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_test_sig_multipart_header_param_name(buf, sizeof(buf), "f", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_multipart_header_param_name(
                               buf, sizeof(buf), "photo", "X-Custom-Hdr"));
  ASSERT_STR_EQ("photo_hdr_X_Custom_Hdr", buf);

  /* header_name_is_content_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_header_name_is_content_type("ct", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_header_name_is_content_type(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_header_name_is_content_type("Content-Type", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_header_name_is_content_type(
                               "Authorization", &int_val));
  ASSERT_EQ(0, int_val);

  /* map_type_to_c_out */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_map_type_to_c_out("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out(NULL, &str_val));
  ASSERT_STR_EQ("void *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("integer", &str_val));
  ASSERT_STR_EQ("int *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("boolean", &str_val));
  ASSERT_STR_EQ("int *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("string", &str_val));
  ASSERT_STR_EQ("char **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("number", &str_val));
  ASSERT_STR_EQ("double *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("custom", &str_val));
  ASSERT_STR_EQ("void *", str_val);

  /* map_array_item_type_out */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_map_array_item_type_out("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out(NULL, &str_val));
  ASSERT_STR_EQ("void **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("integer", &str_val));
  ASSERT_STR_EQ("int **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("boolean", &str_val));
  ASSERT_STR_EQ("int **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("string", &str_val));
  ASSERT_STR_EQ("char ***", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("number", &str_val));
  ASSERT_STR_EQ("double **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("custom", &str_val));
  ASSERT_STR_EQ("void **", str_val);

  /* schema_has_inline */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_schema_has_inline(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_schema_has_inline(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  memset(&schema, 0, sizeof(schema));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_schema_has_inline(&schema, &int_val));
  ASSERT_EQ(0, int_val);
  schema.inline_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_schema_has_inline(&schema, &int_val));
  ASSERT_EQ(1, int_val);

  /* get_success_response */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_get_success_response(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(NULL, &out_resp));
  ASSERT(out_resp == NULL);
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.responses = resp;
  op.n_responses = 3;
  resp[0].code = (char *)(size_t)(size_t) "default";
  resp[1].code = (char *)(size_t)(size_t) "404";
  resp[2].code = (char *)(size_t)(size_t) "200";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &out_resp));
  ASSERT(out_resp == &resp[2]);
  resp[2].code = (char *)(size_t)(size_t) "2XX";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &out_resp));
  ASSERT(out_resp == &resp[2]);
  resp[2].code = (char *)(size_t)(size_t) "500";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &out_resp));
  ASSERT(out_resp == &resp[0]);

  /* response_is_binary_success */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_response_is_binary_success(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_response_is_binary_success(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  resp[2].code = (char *)(size_t)(size_t) "200";
  resp[2].content_type = (char *)(size_t)(size_t) "application/octet-stream";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_response_is_binary_success(&op, &int_val));
  ASSERT_EQ(1, int_val);

  /* get_success_schema */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_get_success_schema(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(NULL, &out_schema));
  ASSERT(out_schema == NULL);
  resp[2].schema.ref_name = (char *)(size_t)(size_t) "SuccessModel";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[2].schema);

  PASS();
}

TEST test_sig_codegen_branches_coverage(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdr;
  struct CodegenSigConfig cfg;
  char *code = NULL;

  /* op.operation_id = NULL, group_name = "" */
  memset(&op, 0, sizeof(op));
  memset(&cfg, 0, sizeof(cfg));
  cfg.group_name = "";
  cfg.include_semicolon = 0;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, &cfg, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "unnamed_op(struct HttpClient *ctx, struct ApiError "
                      "**api_error) {\n") != NULL);
  free(code);

  /* Parameter is JSON array with object item */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = (char *)(size_t)(size_t) "testArrayObj";
  op.n_parameters = 1;
  op.parameters = &param;
  param.name = (char *)(size_t)(size_t) "items";
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const void *items, size_t items_len") != NULL);
  free(code);

  /* Parameter is JSON array with custom struct item */
  param.items_type = (char *)(size_t)(size_t) "CustomItem";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct CustomItem **items, size_t items_len") !=
         NULL);
  free(code);

  /* Parameter is JSON object with type "object" */
  param.is_array = 0;
  param.items_type = NULL;
  param.type = (char *)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *items, size_t items_len") !=
         NULL);
  free(code);

  /* Request body is multipart/form-data with encodings and headers */
  memset(&op, 0, sizeof(op));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(&hdr, 0, sizeof(hdr));
  op.operation_id = (char *)(size_t)(size_t) "testMultipartHdr";
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = &mt;
  mt.name = (char *)(size_t)(size_t) "multipart/form-data";
  mt.n_encoding = 1;
  mt.encoding = &enc;
  enc.name = (char *)(size_t)(size_t) "avatar";
  enc.n_headers = 1;
  enc.headers = &hdr;
  hdr.name = (char *)(size_t)(size_t) "X-Meta";
  hdr.type = (char *)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *avatar_hdr_X_Meta, size_t "
                      "avatar_hdr_X_Meta_len") != NULL);
  free(code);

  /* Header is array of string */
  hdr.type = (char *)(size_t)(size_t) "array";
  hdr.items_type = (char *)(size_t)(size_t) "string";
  hdr.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(
      strstr(code,
             "const char **avatar_hdr_X_Meta, size_t avatar_hdr_X_Meta_len") !=
      NULL);
  free(code);

  /* Header is integer */
  hdr.type = (char *)(size_t)(size_t) "integer";
  hdr.is_array = 0;
  hdr.items_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int avatar_hdr_X_Meta") != NULL);
  free(code);

  /* Request body array of integers in req_body */
  memset(&op, 0, sizeof(op));
  op.operation_id = (char *)(size_t)(size_t) "testReqBodyIntArray";
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body.ref_name = (char *)(size_t)(size_t) "integer";
  op.req_body.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const int *body, size_t body_len") != NULL);
  free(code);

  /* Success schema array of numbers */
  memset(&op, 0, sizeof(op));
  op.operation_id = (char *)(size_t)(size_t) "testSuccessNumArray";
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body.inline_type = (char *)(size_t)(size_t) "number";
  op.req_body.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "double **out, size_t *out_len") != NULL);
  free(code);

  /* Success schema inline number not array */
  memset(&op, 0, sizeof(op));
  op.operation_id = (char *)(size_t)(size_t) "testSuccessNum";
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body.inline_type = (char *)(size_t)(size_t) "number";
  op.req_body.is_array = 0;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "double *out") != NULL);
  free(code);

  PASS();
}

TEST test_sig_all_error_percolations_and_branches(void) {
  int int_val = 0;
  size_t sz_val = 0;
  const char *str_val = NULL;
  char buf[64];
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdr;
  char *code = NULL;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(&hdr, 0, sizeof(hdr));

  /* Helper error injections */
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_map_type_to_c_arg("int", &str_val));
  g_cdd_fail_is_primitive_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_is_primitive_type("int", &int_val));
  g_cdd_fail_param_is_object_kv = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_param_is_object_kv(&param, &int_val));
  g_cdd_fail_media_type_base_len = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_base_len("test", &sz_val));
  g_cdd_fail_media_type_has_prefix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_has_prefix("a", "b", &int_val));
  g_cdd_fail_media_type_has_suffix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_has_suffix("a", "b", &int_val));
  g_cdd_fail_media_type_ieq = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_ieq("a", "b", &int_val));
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_json("a", &int_val));
  g_cdd_fail_media_type_is_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_form("a", &int_val));
  g_cdd_fail_media_type_is_text_plain = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_text_plain("a", &int_val));
  g_cdd_fail_media_type_is_multipart = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_multipart("a", &int_val));
  g_cdd_fail_media_type_is_multipart_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_multipart_form("a", &int_val));
  g_cdd_fail_find_media_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_find_media_type(
                &mt, 1, "a", (const struct OpenAPI_MediaType **)&str_val));
  g_cdd_fail_media_type_is_textual = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("a", &int_val));
  g_cdd_fail_media_type_is_binary = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("a", &int_val));
  g_cdd_fail_qs_form_obj = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_querystring_param_is_form_object(&param, &int_val));
  g_cdd_fail_qs_json_ref = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_querystring_param_is_json_ref(&param, &int_val));
  g_cdd_fail_qs_json_prim = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_primitive_type(&param, &str_val));
  g_cdd_fail_qs_json_array_item_type = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_array_item_type(&param, &str_val));
  g_cdd_fail_qs_json_array_item_ref = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_array_item_ref(&param, &str_val));
  g_cdd_fail_qs_raw = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_raw_primitive_type(&param, &str_val));
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_map_array_item_type("int", &str_val));
  g_cdd_fail_header_name_is_content_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_header_name_is_content_type("Content-Type", &int_val));
  g_cdd_fail_multipart_header_param_name = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_sig_multipart_header_param_name(
                                     buf, sizeof(buf), "f", "h"));
  g_cdd_fail_map_type_to_c_out = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_map_type_to_c_out("int", &str_val));
  g_cdd_fail_map_array_item_type_out = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_map_array_item_type_out("int", &str_val));
  g_cdd_fail_schema_has_inline = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_schema_has_inline(
                (const struct OpenAPI_SchemaRef *)&param, &int_val));
  g_cdd_fail_get_success_response = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_get_success_response(
                &op, (const struct OpenAPI_Response **)&str_val));
  g_cdd_fail_response_is_binary_success = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_response_is_binary_success(&op, &int_val));
  g_cdd_fail_get_success_schema = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_get_success_schema(
                &op, (const struct OpenAPI_SchemaRef **)&str_val));

  /* Now test internal sub-call failures in helpers */
  g_cdd_fail_media_type_base_len = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_has_suffix("a+json", "+json", &int_val));
  g_cdd_fail_media_type_base_len = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_ieq("application/json", "application/json",
                                        &int_val));
  g_cdd_fail_media_type_ieq = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_json("application/json", &int_val));
  g_cdd_fail_media_type_has_suffix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_sig_media_type_is_json(
                                     "application/vnd.api+json", &int_val));

  g_cdd_fail_media_type_is_text_plain = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("text/plain", &int_val));
  g_cdd_fail_media_type_has_prefix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("text/html", &int_val));
  g_cdd_fail_media_type_ieq = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("other", &int_val));
  g_cdd_fail_media_type_has_suffix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("other", &int_val));

  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("bin", &int_val));
  g_cdd_fail_media_type_is_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("bin", &int_val));
  g_cdd_fail_media_type_is_multipart = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("bin", &int_val));
  g_cdd_fail_media_type_is_textual = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("bin", &int_val));

  memset(&param, 0, sizeof(param));
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.content_type = (char *)(size_t)(size_t) "application/json";
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_primitive_type(&param, &str_val));
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_array_item_type(&param, &str_val));
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_array_item_ref(&param, &str_val));
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_raw_primitive_type(&param, &str_val));
  param.content_type = (char *)(size_t)(size_t) "text/plain";
  g_cdd_fail_media_type_is_form = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_raw_primitive_type(&param, &str_val));

  memset(&op, 0, sizeof(op));
  g_cdd_fail_get_success_response = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_response_is_binary_success(&op, &int_val));
  {
    struct OpenAPI_Response r;
    memset(&r, 0, sizeof(r));
    r.code = (char *)(size_t)(size_t) "200";
    op.responses = &r;
    op.n_responses = 1;
    g_cdd_fail_schema_has_inline = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_sig_get_success_schema(
                  &op, (const struct OpenAPI_SchemaRef **)&str_val));
  }

  /* Now test each failure branch in codegen_client_write_signature */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = (char *)(size_t)(size_t) "failOp";
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = (char *)(size_t)(size_t) "p";
  param.in = OA_PARAM_IN_QUERYSTRING;

  g_cdd_fail_qs_json_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_json_array_item_ref = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_json_prim = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_raw = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_form_obj = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_json_ref = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* qs_json_item array type map error */
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* qs_json_prim map type error */
  param.is_array = 0;
  param.items_type = NULL;
  param.type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* qs_raw map type error */
  param.content_type = (char *)(size_t)(size_t) "text/plain";
  param.type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* non-querystring param failures */
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = (char *)(size_t)(size_t) "application/json";
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_is_primitive_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_is_primitive_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.is_array = 0;
  param.items_type = NULL;
  param.type = (char *)(size_t)(size_t) "string";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.content_type = NULL;
  g_cdd_fail_param_is_object_kv = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.is_array = 0;
  param.items_type = NULL;
  param.type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* Request body failures */
  op.n_parameters = 0;
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  g_cdd_fail_media_type_is_binary = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_media_type_is_multipart = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_media_type_is_multipart_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_media_type_is_textual = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  op.req_body.inline_type = (char *)(size_t)(size_t) "string";
  op.req_body.is_array = 1;
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  op.req_body.is_array = 0;
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* Multipart per-part failures */
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";
  op.req_body.inline_type = NULL;
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(&hdr, 0, sizeof(hdr));
  op.n_req_body_media_types = 1;
  op.req_body_media_types = &mt;
  mt.name = (char *)(size_t)(size_t) "multipart/form-data";
  mt.n_encoding = 1;
  mt.encoding = &enc;
  enc.name = (char *)(size_t)(size_t) "field";
  enc.n_headers = 1;
  enc.headers = &hdr;
  hdr.name = (char *)(size_t)(size_t) "X-Header";

  g_cdd_fail_media_type_is_multipart_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_find_media_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_header_name_is_content_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_multipart_header_param_name = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  hdr.is_array = 1;
  hdr.items_type = (char *)(size_t)(size_t) "string";
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  hdr.is_array = 0;
  hdr.items_type = NULL;
  hdr.type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* Success output failures */
  op.req_body.content_type = NULL;
  op.n_req_body_media_types = 0;
  g_cdd_fail_response_is_binary_success = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_get_success_schema = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  {
    struct OpenAPI_Response r;
    memset(&r, 0, sizeof(r));
    r.code = (char *)(size_t)(size_t) "200";
    r.schema.inline_type = (char *)(size_t)(size_t) "string";
    r.schema.is_array = 1;
    op.responses = &r;
    op.n_responses = 1;

    g_cdd_fail_schema_has_inline = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

    g_cdd_fail_map_array_item_type_out = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

    r.schema.is_array = 0;
    g_cdd_fail_map_type_to_c_out = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  }

  PASS();
}

TEST test_sig_complete_branches_coverage(void) {
  int int_val = 0;
  const char *str_val = NULL;
  char buf[64];
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp[3];
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdr[3];
  const struct OpenAPI_Response *out_resp = NULL;
  const struct OpenAPI_SchemaRef *out_schema = NULL;
  char *code = NULL;

  /* 1. media_type_is_textual: text/html and application/xml triggers */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("text/html", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("application/xml", &int_val));
  ASSERT_EQ(1, int_val);
  g_cdd_fail_media_type_ieq = 2;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("application/custom", &int_val));

  /* 2. querystring_param_is_form_object: json error and non-json with ref_name
   */
  memset(&param, 0, sizeof(param));
  param.content_type = (char *)(size_t)(size_t) "application/json";
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_querystring_param_is_form_object(&param, &int_val));
  param.content_type = NULL;
  param.schema.ref_name = (char *)(size_t)(size_t) "MyFormObj";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_form_object(&param, &int_val));
  ASSERT_EQ(1, int_val);

  /* 2b. p.in != OA_PARAM_IN_QUERYSTRING in json_array_item_type and
   * json_array_item_ref */
  param.in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_type(
                               &param, &str_val));
  ASSERT(str_val == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_ref(
                               &param, &str_val));
  ASSERT(str_val == NULL);

  /* 3. g_cdd_fail_sanitize_ident inside multipart_header_param_name */
  g_cdd_fail_sanitize_ident = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_sig_multipart_header_param_name(
                                     buf, sizeof(buf), "f", "h"));

  /* 3b. g_cdd_fail_c_cdd_str_iequal in header_name_is_content_type */
  g_cdd_fail_c_cdd_str_iequal = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_header_name_is_content_type("Content-Type", &int_val));

  /* 4. get_success_response with NULL code */
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.responses = resp;
  op.n_responses = 2;
  resp[0].code = NULL;
  resp[1].code = (char *)(size_t)(size_t) "200";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &out_resp));
  ASSERT(out_resp == &resp[1]);

  /* 5. get_success_schema: NULL code, 2XX without schema, and default response
   * with schema */
  resp[0].code = NULL;
  resp[1].code = (char *)(size_t)(size_t) "2XX"; /* no schema, so continue */
  resp[1].schema.ref_name = NULL;
  resp[1].schema.inline_type = NULL;
  resp[1].schema.is_array = 0;
  op.n_responses = 3;
  resp[2].code = (char *)(size_t)(size_t) "default";
  resp[2].schema.ref_name = (char *)(size_t)(size_t) "DefaultModel";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[2].schema);

  /* 5b. get_success_schema: default response fail and default response fallback
   */
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.responses = resp;
  op.n_responses = 1;
  resp[0].code = (char *)(size_t)(size_t) "default";
  resp[0].schema.ref_name = (char *)(size_t)(size_t) "DefaultModel";
  g_cdd_fail_schema_has_inline = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_get_success_schema(&op, &out_schema));

  resp[0].schema.ref_name = NULL;
  resp[0].schema.inline_type = NULL;
  resp[0].schema.is_array = 0;
  op.req_body.ref_name = (char *)(size_t)(size_t) "FallbackBody";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &op.req_body);

  /* 6. codegen_client_write_signature: is_json_ref_val and is_form_obj in
   * querystring param */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = (char *)(size_t)(size_t) "testQsJsonRef";
  op.n_parameters = 1;
  op.parameters = &param;
  param.name = (char *)(size_t)(size_t) "filter";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.schema.ref_name = (char *)(size_t)(size_t) "FilterRef";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct FilterRef *filter") != NULL);
  free(code);
  code = NULL;

  param.content_type = NULL;
  param.schema.ref_name = (char *)(size_t)(size_t) "FormRef";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);
  free(code);
  code = NULL;

  /* 7. codegen_client_write_signature: header JSON param with ref_name =
   * p->type */
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.schema.ref_name = NULL;
  param.type = (char *)(size_t)(size_t) "CustomType";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct CustomType *filter") != NULL);
  free(code);
  code = NULL;

  /* 8. codegen_client_write_signature: header JSON param with array of
   * primitives (string) */
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "string";
  param.type = (char *)(size_t)(size_t) "array";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const char **filter, size_t filter_len") != NULL);
  free(code);
  code = NULL;

  /* failure of is_primitive_type on array items */
  param.type = NULL;
  g_cdd_fail_is_primitive_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* 9. Request body array of strings and array of custom structs */
  memset(&op, 0, sizeof(op));
  op.operation_id = (char *)(size_t)(size_t) "testReqBodyArrays";
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body.is_array = 1;
  op.req_body.ref_name = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const char **body, size_t body_len") != NULL);
  free(code);
  code = NULL;

  op.req_body.ref_name = (char *)(size_t)(size_t) "MyCustomItem";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "struct MyCustomItem **body, size_t body_len") != NULL);
  free(code);
  code = NULL;

  /* 10. Multipart with NULL name, NULL headers, 0 headers, Content-Type skip,
   * and object */
  memset(&op, 0, sizeof(op));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(hdr, 0, sizeof(hdr));
  op.operation_id = (char *)(size_t)(size_t) "testMultipartCases";
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = &mt;
  mt.name = (char *)(size_t)(size_t) "multipart/form-data";
  mt.n_encoding = 1;
  mt.encoding = &enc;
  enc.name = (char *)(size_t)(size_t) "part1";
  enc.n_headers = 3;
  enc.headers = hdr;
  hdr[0].name = NULL;                                    /* skipped */
  hdr[1].name = (char *)(size_t)(size_t) "Content-Type"; /* skipped */
  hdr[2].name = (char *)(size_t)(size_t) "X-Meta-Obj";
  hdr[2].type = (char *)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *part1_hdr_X_Meta_Obj, size_t "
                      "part1_hdr_X_Meta_Obj_len") != NULL);
  free(code);
  code = NULL;

  /* Empty encoding array */
  mt.n_encoding = 0;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Encoding with NULL headers */
  mt.n_encoding = 1;
  enc.headers = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;
  enc.headers = hdr;

  /* Multipart step 2b failure */
  g_cdd_fail_media_type_is_multipart_form = 2;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* 11. Success schema variants: struct array, string/int/bool/custom inline,
   * and binary */
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.operation_id = (char *)(size_t)(size_t) "testSuccessCases";
  op.responses = resp;
  op.n_responses = 1;
  resp[0].code = (char *)(size_t)(size_t) "200";

  /* Failure of schema_has_inline in step 3 */
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "string";
  g_cdd_fail_schema_has_inline = 2;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* Array of custom structs */
  resp[0].schema.is_array = 1;
  resp[0].schema.ref_name = (char *)(size_t)(size_t) "MyRespStruct";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "struct MyRespStruct ***out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Array of inline string */
  resp[0].schema.ref_name = NULL;
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "char ***out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Array of inline int */
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int **out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Array of inline bool */
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int **out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Array of inline custom */
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "void **out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Non-array custom struct */
  resp[0].schema.is_array = 0;
  resp[0].schema.inline_type = NULL;
  resp[0].schema.ref_name = (char *)(size_t)(size_t) "SingleStruct";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "struct SingleStruct **out") != NULL);
  free(code);
  code = NULL;

  /* Non-array inline string */
  resp[0].schema.ref_name = NULL;
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "char **out") != NULL);
  free(code);
  code = NULL;

  /* Non-array inline int */
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int *out") != NULL);
  free(code);
  code = NULL;

  /* Non-array inline bool */
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int *out") != NULL);
  free(code);
  code = NULL;

  /* Non-array inline custom */
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "void *out") != NULL);
  free(code);
  code = NULL;

  /* Binary response success */
  resp[0].schema.inline_type = NULL;
  resp[0].content_type = (char *)(size_t)(size_t) "application/octet-stream";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "unsigned char **out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* 12. IO failure loops to hit CHECK_IO error returns */
  {
    int io_i;
    FILE *fp = cdd_test_tmpfile_global();
    if (fp) {
      /* hitting qs json ref */
      memset(&op, 0, sizeof(op));
      memset(&param, 0, sizeof(param));
      op.operation_id = (char *)(size_t)(size_t) "testQsJsonRef";
      op.n_parameters = 1;
      op.parameters = &param;
      param.name = (char *)(size_t)(size_t) "filter";
      param.in = OA_PARAM_IN_QUERYSTRING;
      param.content_type = (char *)(size_t)(size_t) "application/json";
      param.schema.ref_name = (char *)(size_t)(size_t) "FilterRef";
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      /* hitting header json array of objects */
      param.in = OA_PARAM_IN_HEADER;
      param.is_array = 1;
      param.items_type = (char *)(size_t)(size_t) "object";
      param.schema.ref_name = NULL;
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      /* hitting multipart headers */
      memset(&op, 0, sizeof(op));
      memset(&mt, 0, sizeof(mt));
      memset(&enc, 0, sizeof(enc));
      memset(hdr, 0, sizeof(hdr));
      op.operation_id = (char *)(size_t)(size_t) "testMultipartCases";
      op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";
      op.n_req_body_media_types = 1;
      op.req_body_media_types = &mt;
      mt.name = (char *)(size_t)(size_t) "multipart/form-data";
      mt.n_encoding = 1;
      mt.encoding = &enc;
      enc.name = (char *)(size_t)(size_t) "part1";
      enc.n_headers = 2;
      enc.headers = hdr;
      hdr[0].name = (char *)(size_t)(size_t) "X-Arr";
      hdr[0].type = (char *)(size_t)(size_t) "array";
      hdr[0].is_array = 1;
      hdr[0].items_type = (char *)(size_t)(size_t) "string";
      hdr[1].name = (char *)(size_t)(size_t) "X-Obj";
      hdr[1].type = (char *)(size_t)(size_t) "object";
      for (io_i = 0; io_i < 10; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      /* hitting struct array response and binary response */
      memset(&op, 0, sizeof(op));
      memset(resp, 0, sizeof(resp));
      op.operation_id = (char *)(size_t)(size_t) "testRespCases";
      op.responses = resp;
      op.n_responses = 1;
      resp[0].code = (char *)(size_t)(size_t) "200";
      resp[0].schema.is_array = 1;
      resp[0].schema.ref_name = (char *)(size_t)(size_t) "MyRespStruct";
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      resp[0].schema.is_array = 0;
      resp[0].schema.ref_name = NULL;
      resp[0].content_type =
          (char *)(size_t)(size_t) "application/octet-stream";
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  PASS();
}

TEST test_sig_ultra_coverage(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp[3];
  const struct OpenAPI_SchemaRef *out_schema = NULL;
  const char *out_schema_str = NULL;
  int int_val = 0;
  char buf[64];
  char *code = NULL;

  /* sanitize_ident branch tests */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_sanitize_ident(buf, sizeof(buf), ""));
  ASSERT_STR_EQ("", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), "[abc]"));
  ASSERT_STR_EQ("_abc_", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), "abc"));
  ASSERT_STR_EQ("abc", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), "_abc"));
  ASSERT_STR_EQ("_abc", buf);

  /* find_media_type with NULL entry name */
  {
    struct OpenAPI_MediaType mts[2];
    const struct OpenAPI_MediaType *found = NULL;
    memset(mts, 0, sizeof(mts));
    mts[0].name = NULL;
    mts[1].name = (char *)(size_t)(size_t) "text/plain";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_sig_find_media_type(mts, 2, "text/plain", &found));
    ASSERT(found == &mts[1]);
  }

  /* querystring_param_raw_primitive_type with NULL content_type */
  memset(&param, 0, sizeof(param));
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.content_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_raw_primitive_type(
                               &param, &out_schema_str));
  ASSERT(out_schema_str == NULL);

  /* param_is_object_kv with !is_array and type == NULL */
  memset(&param, 0, sizeof(param));
  param.is_array = 0;
  param.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&param, &int_val));
  ASSERT_EQ(0, int_val);

  /* querystring_param_json_array_item_ref with "number" and "boolean" */
  memset(&param, 0, sizeof(param));
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_ref(
                               &param, &out_schema_str));
  ASSERT(out_schema_str == NULL);
  param.items_type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_ref(
                               &param, &out_schema_str));
  ASSERT(out_schema_str == NULL);

  /* response codes "20X" and "2XY" to test branch in strlen == 3 && c[0]=='2'
   * && c[1]=='X' && c[2]=='X' */
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.responses = resp;
  op.n_responses = 2;
  resp[0].code = (char *)(size_t)(size_t) "20X";
  resp[1].code = (char *)(size_t)(size_t) "2XY";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  {
    const struct OpenAPI_Response *r_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &r_out));
    ASSERT(r_out == &resp[0]);
  }

  /* schema_has_inline and is_array branches in get_success_schema */
  resp[0].code = (char *)(size_t)(size_t) "200";
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "string";
  resp[0].schema.is_array = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[0].schema);

  resp[0].schema.inline_type = NULL;
  resp[0].schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[0].schema);

  /* default_resp with inline_type, and with is_array */
  resp[0].code = (char *)(size_t)(size_t) "default";
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "string";
  resp[0].schema.is_array = 0;
  op.n_responses = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[0].schema);

  resp[0].schema.inline_type = NULL;
  resp[0].schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[0].schema);

  /* header JSON param variations */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = (char *)(size_t)(size_t) "testVariations";
  op.n_parameters = 1;
  op.parameters = &param;
  param.name = (char *)(size_t)(size_t) "p";
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = (char *)(size_t)(size_t) "application/json";

  /* items_type NULL, inline_type set */
  param.is_array = 1;
  param.items_type = NULL;
  param.schema.inline_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* items_type NULL, inline_type NULL (void* array) */
  param.schema.inline_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* is_array = 0, p.type NULL, schema.inline_type set */
  param.is_array = 0;
  param.type = NULL;
  param.schema.inline_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* is_array = 0, p.type NULL, schema.inline_type NULL ("string" fallback) */
  param.schema.inline_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* config with prefix == NULL, ctx_type == NULL, group_name == NULL */
  {
    struct CodegenSigConfig cfg_empty;
    memset(&cfg_empty, 0, sizeof(cfg_empty));
    cfg_empty.include_semicolon = 1;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, &cfg_empty, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* op with no responses and no req_body (success_schema == NULL &&
   * success_is_binary == 0) */
  {
    struct OpenAPI_Operation op_no_resp;
    memset(&op_no_resp, 0, sizeof(op_no_resp));
    op_no_resp.operation_id = (char *)(size_t)(size_t) "noResp";
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_no_resp, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* Multipart with enc.name == NULL and with enc.n_headers == 0 and
   * hdr.items_type == NULL */
  {
    struct OpenAPI_Operation op_mp;
    struct OpenAPI_MediaType mt_mp;
    struct OpenAPI_Encoding enc_mp[2];
    struct OpenAPI_Header hdr_mp;
    memset(&op_mp, 0, sizeof(op_mp));
    memset(&mt_mp, 0, sizeof(mt_mp));
    memset(enc_mp, 0, sizeof(enc_mp));
    memset(&hdr_mp, 0, sizeof(hdr_mp));
    op_mp.operation_id = (char *)(size_t)(size_t) "mpEdge";
    op_mp.req_body.content_type =
        (char *)(size_t)(size_t) "multipart/form-data";
    op_mp.n_req_body_media_types = 1;
    op_mp.req_body_media_types = &mt_mp;
    mt_mp.name = (char *)(size_t)(size_t) "multipart/form-data";
    mt_mp.n_encoding = 2;
    mt_mp.encoding = enc_mp;
    enc_mp[0].name = NULL; /* enc->name == NULL branch */
    enc_mp[1].name = (char *)(size_t)(size_t) "part";
    enc_mp[1].headers = &hdr_mp;
    enc_mp[1].n_headers = 0; /* enc->n_headers == 0 branch */
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_mp, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;

    /* hdr.items_type == NULL in array */
    enc_mp[1].n_headers = 1;
    hdr_mp.name = (char *)(size_t)(size_t) "X-Arr-Def";
    hdr_mp.is_array = 1;
    hdr_mp.items_type = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_mp, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* Success schema with ref_name == NULL, inline_type == NULL, is_array == 0 */
  {
    struct OpenAPI_Operation op_empty_schema;
    struct OpenAPI_Response resp_empty;
    memset(&op_empty_schema, 0, sizeof(op_empty_schema));
    memset(&resp_empty, 0, sizeof(resp_empty));
    op_empty_schema.operation_id = (char *)(size_t)(size_t) "emptySchema";
    op_empty_schema.responses = &resp_empty;
    op_empty_schema.n_responses = 1;
    resp_empty.code = (char *)(size_t)(size_t) "200";
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_empty_schema, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  PASS();
}

SUITE(client_sig_suite) {

  RUN_TEST(test_sig_helpers_coverage_1);
  RUN_TEST(test_sig_helpers_coverage_2);
  RUN_TEST(test_sig_helpers_coverage_3);
  RUN_TEST(test_sig_codegen_branches_coverage);
  RUN_TEST(test_sig_all_error_percolations_and_branches);
  RUN_TEST(test_sig_complete_branches_coverage);
  RUN_TEST(test_sig_ultra_coverage);
  RUN_TEST(test_sig_simple_get);
  RUN_TEST(test_sig_verify_apierror);
  RUN_TEST(test_sig_grouped);
  RUN_TEST(test_sig_success_range_response);
  RUN_TEST(test_sig_default_response_success);
  RUN_TEST(test_sig_inline_response_string);
  RUN_TEST(test_sig_inline_response_array);
  RUN_TEST(test_sig_inline_request_body_string);
  RUN_TEST(test_sig_inline_request_body_array);
  RUN_TEST(test_sig_multipart_encoding_headers);
  RUN_TEST(test_sig_text_plain_request_body);
  RUN_TEST(test_sig_textual_request_body_xml);
  RUN_TEST(test_sig_octet_stream_request_body);
  RUN_TEST(test_sig_binary_request_body_pdf);
  RUN_TEST(test_sig_octet_stream_response_body);
  RUN_TEST(test_sig_binary_response_body_pdf);
  RUN_TEST(test_sig_querystring_form_object);
  RUN_TEST(test_sig_querystring_json_ref);
  RUN_TEST(test_sig_querystring_json_primitive);
  RUN_TEST(test_sig_querystring_json_array);
  RUN_TEST(test_sig_querystring_json_array_object);
  RUN_TEST(test_sig_querystring_raw_string);
  RUN_TEST(test_sig_querystring_raw_integer);
  RUN_TEST(test_sig_json_content_query_ref);
  RUN_TEST(test_sig_query_object_param_kv);
  RUN_TEST(test_sig_path_object_param_kv);
  RUN_TEST(test_sig_header_object_param_kv);
  RUN_TEST(test_sig_cookie_object_param_kv);
  RUN_TEST(test_sig_header_param_string);
  RUN_TEST(test_sig_header_param_integer);
  RUN_TEST(test_sig_header_param_number);
  RUN_TEST(test_sig_header_param_boolean);
  RUN_TEST(test_sig_header_param_json);
  RUN_TEST(test_sig_media_type_branches);
  RUN_TEST(test_sig_response_array_string_ref);
  RUN_TEST(test_sig_response_array_integer_ref);
  RUN_TEST(test_sig_response_array_struct_ref);
  RUN_TEST(test_sig_null_args);
  RUN_TEST(test_sig_unsupported_prefix);
  RUN_TEST(test_sig_io_errors);
}

#endif /* TEST_CODEGEN_CLIENT_SIG_H */

#ifdef __cplusplus
}
#endif /* __cplusplus */
