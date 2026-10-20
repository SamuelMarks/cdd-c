/**
 * @file test_codegen_client_sig_params.h
 * @brief Unit tests for C Client Signature Generation (Parameters, objects, and
 * types).
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_PARAMS_H
#define TEST_CODEGEN_CLIENT_SIG_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_sig_common.h"
/* clang-format on */

/**
 * @brief Unit test test_sig_query_object_param_kv.
 * @return Greatest test result.
 */
TEST test_sig_query_object_param_kv(void) {
  char *_ast_gen_sig_23 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = cdd_test_mut_str("list");

  param.name = cdd_test_mut_str("filter");
  param.type = cdd_test_mut_str("object");
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

  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_sig_path_object_param_kv.
 * @return Greatest test result.
 */
TEST test_sig_path_object_param_kv(void) {
  char *_ast_gen_sig_24 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = cdd_test_mut_str("byPath");

  param.name = cdd_test_mut_str("filter");
  param.type = cdd_test_mut_str("object");
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

  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_sig_header_object_param_kv.
 * @return Greatest test result.
 */
TEST test_sig_header_object_param_kv(void) {
  char *_ast_gen_sig_25 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = cdd_test_mut_str("byHeader");

  param.name = cdd_test_mut_str("filter");
  param.type = cdd_test_mut_str("object");
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

  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_sig_cookie_object_param_kv.
 * @return Greatest test result.
 */
TEST test_sig_cookie_object_param_kv(void) {
  char *_ast_gen_sig_26 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = cdd_test_mut_str("byCookie");

  param.name = cdd_test_mut_str("prefs");
  param.type = cdd_test_mut_str("object");
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

  ASSERT(strstr(code, "const struct OpenAPI_KV *prefs, size_t prefs_len") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_sig_json_content_query_ref.
 * @return Greatest test result.
 */
TEST test_sig_json_content_query_ref(void) {
  char *_ast_gen_sig_27 = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;

  op.operation_id = cdd_test_mut_str("list");

  param.name = cdd_test_mut_str("filter");
  param.in = OA_PARAM_IN_QUERY;
  param.content_type = cdd_test_mut_str("application/json");
  param.schema.ref_name = cdd_test_mut_str("Filter");
  param.type = cdd_test_mut_str("Filter");
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

  ASSERT(strstr(code, "const struct Filter *filter") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_sig_header_param_boolean.
 * @return Greatest test result.
 */
TEST test_sig_header_param_boolean(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_sig_0_uniq = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("200");
  op.responses = &resp;
  op.n_responses = 1;

  param.name = cdd_test_mut_str("X-Bool");
  param.in = OA_PARAM_IN_HEADER;
  param.type = cdd_test_mut_str("boolean");
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

  ASSERT(strstr(code, "int X-Bool") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_sig_header_param_number.
 * @return Greatest test result.
 */
TEST test_sig_header_param_number(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_sig_5_uniq = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("200");
  op.responses = &resp;
  op.n_responses = 1;

  param.name = cdd_test_mut_str("X-Num");
  param.in = OA_PARAM_IN_HEADER;
  param.type = cdd_test_mut_str("number");
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

  ASSERT(strstr(code, "double X-Num") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_sig_header_param_integer.
 * @return Greatest test result.
 */
TEST test_sig_header_param_integer(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_sig_10_uniq = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("200");
  op.responses = &resp;
  op.n_responses = 1;

  param.name = cdd_test_mut_str("X-Int");
  param.in = OA_PARAM_IN_HEADER;
  param.type = cdd_test_mut_str("integer");
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

  ASSERT(strstr(code, "int X-Int") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_sig_header_param_string.
 * @return Greatest test result.
 */
TEST test_sig_header_param_string(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_sig_15_uniq = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("200");
  op.responses = &resp;
  op.n_responses = 1;

  param.name = cdd_test_mut_str("X-String");
  param.in = OA_PARAM_IN_HEADER;
  param.type = cdd_test_mut_str("string");
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

  ASSERT(strstr(code, "char *X-String") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_sig_header_param_json.
 * @return Greatest test result.
 */
TEST test_sig_header_param_json(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code = NULL;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  op.operation_id = cdd_test_mut_str("testHeaderJson");
  op.n_parameters = 1;
  op.parameters = &param;

  param.name = cdd_test_mut_str("X-MyHeader");
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = cdd_test_mut_str("application/json");

  /* Test primitive JSON type */
  param.type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test primitive JSON array */
  param.type = cdd_test_mut_str("array");
  param.is_array = 1;
  param.items_type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test non-primitive object */
  param.type = cdd_test_mut_str("object");
  param.is_array = 0;
  param.items_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test non-primitive array */
  param.type = cdd_test_mut_str("array");
  param.is_array = 1;
  param.items_type = cdd_test_mut_str("MyType");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test ref */
  param.type = NULL;
  param.is_array = 0;
  param.schema.ref_name = cdd_test_mut_str("MyType");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Test inline object array */
  param.type = cdd_test_mut_str("array");
  param.is_array = 1;
  param.schema.ref_name = NULL;
  param.items_type = cdd_test_mut_str("object");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  PASS();
}

/**
 * @brief Unit test test_sig_media_type_branches.
 * @return Greatest test result.
 */
TEST test_sig_media_type_branches(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_MediaType mt;
  char *code = NULL;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(&mt, 0, sizeof(mt));

  op.operation_id = cdd_test_mut_str("testMediaTypes");
  op.n_parameters = 1;
  op.parameters = &param;

  param.name = cdd_test_mut_str("myParam");
  param.in = OA_PARAM_IN_QUERY;

  /* upper case testing */
  param.content_type = cdd_test_mut_str("APPLICATION/JSON");
  param.type = cdd_test_mut_str("object");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* text/plain */
  param.content_type = cdd_test_mut_str("TEXT/PLAIN");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* application/xml */
  param.content_type = cdd_test_mut_str("APPLICATION/XML");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* multipart/form-data */
  param.content_type = cdd_test_mut_str("MULTIPART/FORM-DATA");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* application/x-www-form-urlencoded */
  param.content_type = cdd_test_mut_str("APPLICATION/"
                                        "X-WWW-FORM-URLENCODED");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  /* default fallback */
  param.content_type = cdd_test_mut_str("UNKNOWN/TYPE");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  free(code);
  code = NULL;

  PASS();
}

/**
 * @brief Unit test test_sig_response_array_string_ref.
 * @return Greatest test result.
 */
TEST test_sig_response_array_string_ref(void) {
  char *_ast_gen_sig_20_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;
  op.operation_id = cdd_test_mut_str("getArrStr");
  resp.code = cdd_test_mut_str("200");
  resp.schema.is_array = 1;
  resp.schema.ref_name = cdd_test_mut_str("string");
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

  ASSERT(strstr(code, "char ***out, size_t *out_len") != NULL);
  free(code);
  PASS();
}

/**
 * @brief Unit test test_sig_response_array_integer_ref.
 * @return Greatest test result.
 */
TEST test_sig_response_array_integer_ref(void) {
  char *_ast_gen_sig_25_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;
  op.operation_id = cdd_test_mut_str("getArrInt");
  resp.code = cdd_test_mut_str("200");
  resp.schema.is_array = 1;
  resp.schema.ref_name = cdd_test_mut_str("integer");
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
  PASS();
}

/**
 * @brief Unit test test_sig_response_array_struct_ref.
 * @return Greatest test result.
 */
TEST test_sig_response_array_struct_ref(void) {
  char *_ast_gen_sig_30_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *code;
  op.operation_id = cdd_test_mut_str("getArrStruct");
  resp.code = cdd_test_mut_str("200");
  resp.schema.is_array = 1;
  resp.schema.ref_name = cdd_test_mut_str("Pet");
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

  ASSERT(strstr(code, "struct Pet ***out, size_t *out_len") != NULL);
  free(code);
  PASS();
}

/**
 * @brief Unit test test_sig_null_args.
 * @return Greatest test result.
 */
TEST test_sig_null_args(void) {
  char *code = NULL;
  ASSERT(codegen_client_write_signature(NULL, NULL, NULL) ==
         CDD_C_ERROR_INVALID_ARGUMENT);
  ASSERT(gen_sig(NULL, NULL, &code) != CDD_C_SUCCESS);
  g_sig_fail_tmpfile = 1;
  ASSERT(gen_sig(NULL, NULL, &code) == CDD_C_ERROR_IO);
  g_sig_fail_tmpfile = 0;
  PASS();
}

/**
 * @brief Unit test test_sig_io_errors.
 * @return Greatest test result.
 */
TEST test_sig_io_errors(void) {
  int i;
  char *_ast_gen_sig_35_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  char *code = NULL;

  op.operation_id = cdd_test_mut_str("getAllBranches");
  op.n_responses = 1;
  op.responses = &resp;
  resp.code = cdd_test_mut_str("200");
  resp.schema.is_array = 1;
  resp.schema.ref_name = cdd_test_mut_str("Pet");

  op.n_parameters = 1;
  op.parameters = &param;
  param.name = cdd_test_mut_str("myParam");
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.schema.inline_type = cdd_test_mut_str("string");

  for (i = 0; i < 2; ++i) {
    g_io_calls = 0;
    g_fail_io_after = (i == 0) ? 1 : 100;
    code = NULL;
    if (gen_sig(&op, NULL, &code) == CDD_C_SUCCESS) {
      free(code);
    } else {
      free(code);
    }
  }
  g_fail_io_after = -1;
  (void)_ast_gen_sig_35_uniq;
  PASS();
}

/**
 * @brief Unit test test_sig_unsupported_prefix.
 * @return Greatest test result.
 */
TEST test_sig_unsupported_prefix(void) {
  char *_ast_gen_sig_36_uniq = NULL;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *code;
  op.operation_id = cdd_test_mut_str("prefixTest");
  op.n_parameters = 1;
  op.parameters = &param;
  param.name = cdd_test_mut_str("myParam");
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = cdd_test_mut_str("string");
  param.content_type = cdd_test_mut_str("unsupported/type");
  gen_sig(&op, NULL, &code);
  ASSERT(code);
  free(code);
  (void)_ast_gen_sig_36_uniq;
  PASS();
}

/**
 * @brief Executes the client_sig_params_suite test suite.
 */
SUITE(client_sig_params_suite) {
  RUN_TEST(test_sig_query_object_param_kv);
  RUN_TEST(test_sig_path_object_param_kv);
  RUN_TEST(test_sig_header_object_param_kv);
  RUN_TEST(test_sig_cookie_object_param_kv);
  RUN_TEST(test_sig_json_content_query_ref);
  RUN_TEST(test_sig_header_param_boolean);
  RUN_TEST(test_sig_header_param_number);
  RUN_TEST(test_sig_header_param_integer);
  RUN_TEST(test_sig_header_param_string);
  RUN_TEST(test_sig_header_param_json);
  RUN_TEST(test_sig_media_type_branches);
  RUN_TEST(test_sig_response_array_string_ref);
  RUN_TEST(test_sig_response_array_integer_ref);
  RUN_TEST(test_sig_response_array_struct_ref);
  RUN_TEST(test_sig_null_args);
  RUN_TEST(test_sig_io_errors);
  RUN_TEST(test_sig_unsupported_prefix);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_SIG_PARAMS_H */
