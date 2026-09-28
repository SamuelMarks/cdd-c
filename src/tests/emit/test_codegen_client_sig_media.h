/**
 * @file test_codegen_client_sig_media.h
 * @brief Unit tests for C Client Signature Generation (Media types, payloads,
 * and querystrings).
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_MEDIA_H
#define TEST_CODEGEN_CLIENT_SIG_MEDIA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_sig_common.h"
/* clang-format on */

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
      free(io_code);
    }
    g_fail_io_after = -1;
  }

  ASSERT(strstr(code, "qs") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

SUITE(client_sig_media_suite) {
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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_SIG_MEDIA_H */
