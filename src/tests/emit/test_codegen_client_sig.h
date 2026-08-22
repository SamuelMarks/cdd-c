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

static cdd_c_error_t gen_sig(const struct OpenAPI_Operation *op,
                             const struct CodegenSigConfig *cfg,
                             char **_out_val) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (tmpfile_s(&tmp) != 0)
    tmp = NULL;
#else
  tmp = tmpfile();
#endif
  long sz;
  char *content = NULL;

  if (!tmp) {
    *_out_val = NULL;
    return 0;
  }

  if (codegen_client_write_signature(tmp, op, cfg) != 0) {
    fclose(tmp);
    {
      *_out_val = NULL;
      return 0;
    }
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  if (sz > 0)
    if (fread(content, 1, sz, tmp)) {
    }

  fclose(tmp);
  {
    *_out_val = content;
    return 0;
  }
}

TEST test_sig_simple_get(void) {
  char *_ast_gen_sig_0 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = "get_pet";

  param.name = "id";
  param.type = "integer";
  op.parameters = &param;
  op.n_parameters = 1;

  op.req_body.ref_name = "Pet";

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
  op.operation_id = (char *)"do";

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

  op.operation_id = "getById";

  cfg.prefix = "api_";
  cfg.group_name = "Pet";

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

  op.operation_id = "listPets";

  resp.code = "2XX";
  resp.schema.ref_name = "Pet";
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

  op.operation_id = "defaultPet";

  resp.code = "default";
  resp.schema.ref_name = "Pet";
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

  op.operation_id = "getInline";

  resp.code = "200";
  resp.schema.inline_type = "string";
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

  op.operation_id = "getInlineArr";

  resp.code = "200";
  resp.schema.is_array = 1;
  resp.schema.inline_type = "integer";
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

  op.operation_id = "postInline";
  op.req_body.content_type = "application/json";
  op.req_body.inline_type = "string";

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

  op.operation_id = "postInlineArr";
  op.req_body.content_type = "application/json";
  op.req_body.is_array = 1;
  op.req_body.inline_type = "number";

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

  op.operation_id = "upload";

  op.req_body.ref_name = "Upload";
  op.req_body.content_type = "multipart/form-data";

  headers[0].name = "X-Trace";
  headers[0].type = "string";
  headers[1].name = "X-Ids";
  headers[1].type = "array";
  headers[1].is_array = 1;
  headers[1].items_type = "integer";
  headers[2].name = "Content-Type";
  headers[2].type = "string";

  mt.name = "multipart/form-data";
  enc.name = "file";
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

  op.operation_id = "postText";
  op.req_body.content_type = "text/plain";
  op.req_body.inline_type = "string";

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

  op.operation_id = "postXml";
  op.req_body.content_type = "application/xml";

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

  op.operation_id = "postBinary";
  op.req_body.content_type = "application/octet-stream";

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

  op.operation_id = "postPdf";
  op.req_body.content_type = "application/pdf";

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

  op.operation_id = "download";
  resp.code = "200";
  resp.content_type = "application/octet-stream";
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

  op.operation_id = "downloadPdf";
  resp.code = "200";
  resp.content_type = "application/pdf";
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

  op.operation_id = "search";

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "object";
  param.content_type = "application/x-www-form-urlencoded";
  param.schema.inline_type = "object";

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

  op.operation_id = "searchJson";

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "object";
  param.content_type = "application/json";
  param.schema.ref_name = "Pet";

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

  op.operation_id = "searchJsonInt";

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "integer";
  param.content_type = "application/json";
  param.schema.inline_type = "integer";

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

  op.operation_id = "searchJsonTags";

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "array";
  param.content_type = "application/json";
  param.schema.is_array = 1;
  param.schema.inline_type = "string";

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

  op.operation_id = "searchJsonPets";

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "array";
  param.content_type = "application/json";
  param.schema.is_array = 1;
  param.items_type = "Pet";

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

  op.operation_id = "searchRaw";

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "string";
  param.content_type = "text/plain";
  param.schema.inline_type = "string";

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

  op.operation_id = "searchRawInt";

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "integer";
  param.content_type = "application/jsonpath";
  param.schema.inline_type = "integer";

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

  op.operation_id = "list";

  param.name = "filter";
  param.type = "object";
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

  op.operation_id = "byPath";

  param.name = "filter";
  param.type = "object";
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

  op.operation_id = "byHeader";

  param.name = "filter";
  param.type = "object";
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

  op.operation_id = "byCookie";

  param.name = "prefs";
  param.type = "object";
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

  op.operation_id = "list";

  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.content_type = "application/json";
  param.schema.ref_name = "Filter";
  param.type = "Filter";
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
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "X-Bool";
  param.in = OA_PARAM_IN_HEADER;
  param.type = "boolean";
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
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "X-Num";
  param.in = OA_PARAM_IN_HEADER;
  param.type = "number";
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
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "X-Int";
  param.in = OA_PARAM_IN_HEADER;
  param.type = "integer";
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
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "X-String";
  param.in = OA_PARAM_IN_HEADER;
  param.type = "string";
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

  op.operation_id = "testHeaderJson";
  op.n_parameters = 1;
  op.parameters = &param;

  param.name = "X-MyHeader";
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = "application/json";

  /* Test primitive JSON type */
  param.type = "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test primitive JSON array */
  param.type = "array";
  param.is_array = 1;
  param.items_type = "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test non-primitive object */
  param.type = "object";
  param.is_array = 0;
  param.items_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test non-primitive array */
  param.type = "array";
  param.is_array = 1;
  param.items_type = "MyType";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test ref */
  param.type = NULL;
  param.is_array = 0;
  param.schema.ref_name = "MyType";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test inline object array */
  param.type = "array";
  param.is_array = 1;
  param.schema.ref_name = NULL;
  param.items_type = "object";
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

  op.operation_id = "testMediaTypes";
  op.n_parameters = 1;
  op.parameters = &param;

  param.name = "myParam";
  param.in = OA_PARAM_IN_QUERY;

  /* upper case testing */
  param.content_type = "APPLICATION/JSON";
  param.type = "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* text/plain */
  param.content_type = "TEXT/PLAIN";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* application/xml */
  param.content_type = "APPLICATION/XML";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* multipart/form-data */
  param.content_type = "MULTIPART/FORM-DATA";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* application/x-www-form-urlencoded */
  param.content_type = "APPLICATION/X-WWW-FORM-URLENCODED";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* default fallback */
  param.content_type = "UNKNOWN/TYPE";
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
  op.operation_id = "getArrStr";
  resp.code = "200";
  resp.schema.is_array = 1;
  resp.schema.ref_name = "string";
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
  op.operation_id = "getArrInt";
  resp.code = "200";
  resp.schema.is_array = 1;
  resp.schema.ref_name = "integer";
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
  op.operation_id = "getArrStruct";
  resp.code = "200";
  resp.schema.is_array = 1;
  resp.schema.ref_name = "Pet";
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

  op.operation_id = "getAllBranches";
  op.n_responses = 1;
  op.responses = &resp;
  resp.code = "200";
  resp.schema.is_array = 1;
  resp.schema.ref_name = "Pet";

  op.n_parameters = 1;
  op.parameters = &param;
  param.name = "myParam";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.schema.inline_type = "string";

  for (i = 0; i < 500; ++i) {
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
  PASS();
}

TEST test_sig_unsupported_prefix(void) {
  char *_ast_gen_sig_36_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;
  op.operation_id = "prefixTest";
  op.n_parameters = 1;
  op.parameters = &param;
  param.name = "myParam";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "string";
  param.content_type = "unsupported/type";
  gen_sig(&op, NULL, &code);
  ASSERT(code);
  free(code);
  PASS();
}

SUITE(client_sig_suite) {

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
