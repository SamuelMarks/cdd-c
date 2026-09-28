/**
 * @file test_codegen_client_sig.h
 * @brief Unit tests for C Client Signature Generation (Basic operations).
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_H
#define TEST_CODEGEN_CLIENT_SIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_sig_common.h"
/* clang-format on */

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

  ASSERT(strstr(code, "const double *body, size_t body_len") != NULL);

  free(code);
  g_fail_io_after = -1;
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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_SIG_H */
