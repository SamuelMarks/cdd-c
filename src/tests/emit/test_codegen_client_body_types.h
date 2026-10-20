/**
 * @file test_codegen_client_body_types.h
 * @brief Unit tests for client body primitive and schema types.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_TYPES_H
#define TEST_CODEGEN_CLIENT_BODY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_body_common.h"
/* clang-format on */

/**
 * @brief Unit test test_body_header_param_string.
 * @return Greatest test result.
 */
TEST test_body_header_param_string(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_body_h1 = NULL;

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

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_h1), _ast_gen_body_h1);
  ASSERT(code);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"X-String\", X-String)") !=
      NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_body_header_param_integer.
 * @return Greatest test result.
 */
TEST test_body_header_param_integer(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_body_h2 = NULL;

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

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_h2), _ast_gen_body_h2);
  ASSERT(code);
  ASSERT(strstr(code, "spr"
                      "intf(num_buf, \"%d\", X-Int);") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_body_header_param_number.
 * @return Greatest test result.
 */
TEST test_body_header_param_number(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_body_h3 = NULL;

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

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_h3), _ast_gen_body_h3);
  ASSERT(code);
  ASSERT(strstr(code, "spr"
                      "intf(num_buf, \"%g\", X-Num);") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_body_header_param_boolean.
 * @return Greatest test result.
 */
TEST test_body_header_param_boolean(void) {
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Operation op = {0};
  char *code = NULL;
  char *_ast_gen_body_h4 = NULL;

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

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_h4), _ast_gen_body_h4);
  ASSERT(code);
  ASSERT(strstr(code, "X-Bool ? \"true\" : \"false\"") != NULL);
  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_client_body_all_primitive_types.
 * @return Greatest test result.
 */
TEST test_client_body_all_primitive_types(void) {
  int io_fail;
  for (io_fail = 0; io_fail < 3000; ++io_fail) {
    /* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
    /* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
    {
      struct OpenAPI_Spec spec = {0};
      struct OpenAPI_Operation op = {0};
      FILE *fp;
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      {
        int rc = 0;

        memset(&spec, 0, sizeof(spec));
        memset(&op, 0, sizeof(op));

        op.method = cdd_test_mut_str("put");
        op.verb = OA_VERB_PUT;
        op.operation_id = cdd_test_mut_str("testPrimitives");
        op.n_parameters = 21;
        op.parameters = calloc(21, sizeof(*op.parameters));
        op.parameters[0].in = OA_PARAM_IN_HEADER;
        op.parameters[0].name = cdd_test_mut_str("X-Num");
        op.parameters[0].type = cdd_test_mut_str("number");
        op.parameters[0].required = 1;
        op.parameters[1].in = OA_PARAM_IN_HEADER;
        op.parameters[1].name = cdd_test_mut_str("X-Bool");
        op.parameters[1].type = cdd_test_mut_str("boolean");
        op.parameters[1].required = 0;
        op.parameters[2].in = OA_PARAM_IN_PATH;
        op.parameters[2].name = cdd_test_mut_str("pNum");
        op.parameters[2].type = cdd_test_mut_str("number");
        op.parameters[2].required = 1;
        op.parameters[3].in = OA_PARAM_IN_PATH;
        op.parameters[3].name = cdd_test_mut_str("pBool");
        op.parameters[3].type = cdd_test_mut_str("boolean");
        op.parameters[3].required = 1;
        op.parameters[4].in = OA_PARAM_IN_QUERY;
        op.parameters[4].name = cdd_test_mut_str("qNum");
        op.parameters[4].type = cdd_test_mut_str("number");
        op.parameters[4].required = 0;
        op.parameters[4].style = OA_STYLE_FORM;
        op.parameters[4].explode = 1;
        op.parameters[4].explode_set = 1;
        op.parameters[5].in = OA_PARAM_IN_QUERY;
        op.parameters[5].name = cdd_test_mut_str("qBool");
        op.parameters[5].type = cdd_test_mut_str("boolean");
        op.parameters[5].required = 1;
        op.parameters[5].style = OA_STYLE_FORM;
        op.parameters[5].explode = 1;
        op.parameters[5].explode_set = 1;
        op.parameters[6].in = OA_PARAM_IN_COOKIE;
        op.parameters[6].name = cdd_test_mut_str("cNum");
        op.parameters[6].type = cdd_test_mut_str("number");
        op.parameters[6].required = 0;
        op.parameters[7].in = OA_PARAM_IN_COOKIE;
        op.parameters[7].name = cdd_test_mut_str("cBool");
        op.parameters[7].type = cdd_test_mut_str("boolean");
        op.parameters[7].required = 1;
        op.parameters[8].in = OA_PARAM_IN_QUERY;
        op.parameters[8].name = cdd_test_mut_str("qArrNum");
        op.parameters[8].type = cdd_test_mut_str("array");
        op.parameters[8].items_type = cdd_test_mut_str("number");
        op.parameters[8].style = OA_STYLE_FORM;
        op.parameters[8].explode = 1;
        op.parameters[8].explode_set = 1;
        op.parameters[8].is_array = 1;
        op.parameters[9].in = OA_PARAM_IN_QUERY;
        op.parameters[9].name = cdd_test_mut_str("qArrBool");
        op.parameters[9].type = cdd_test_mut_str("array");
        op.parameters[9].items_type = cdd_test_mut_str("boolean");
        op.parameters[9].style = OA_STYLE_FORM;
        op.parameters[9].explode = 1;
        op.parameters[9].explode_set = 1;
        op.parameters[9].is_array = 1;
        op.parameters[10].in = OA_PARAM_IN_QUERY;
        op.parameters[10].name = cdd_test_mut_str("qArrInt");
        op.parameters[10].type = cdd_test_mut_str("array");
        op.parameters[10].items_type = cdd_test_mut_str("integer");
        op.parameters[10].style = OA_STYLE_FORM;
        op.parameters[10].explode = 1;
        op.parameters[10].explode_set = 1;
        op.parameters[10].is_array = 1;
        op.parameters[11].in = OA_PARAM_IN_QUERY;
        op.parameters[11].name = cdd_test_mut_str("qArrStr");
        op.parameters[11].type = cdd_test_mut_str("array");
        op.parameters[11].items_type = cdd_test_mut_str("string");
        op.parameters[11].style = OA_STYLE_FORM;
        op.parameters[11].explode = 1;
        op.parameters[11].explode_set = 1;
        op.parameters[11].is_array = 1;
        op.parameters[12].in = OA_PARAM_IN_HEADER;
        op.parameters[12].name = cdd_test_mut_str("hArrNum");
        op.parameters[12].type = cdd_test_mut_str("array");
        op.parameters[12].items_type = cdd_test_mut_str("number");
        op.parameters[12].is_array = 1;
        op.parameters[13].in = OA_PARAM_IN_COOKIE;
        op.parameters[13].name = cdd_test_mut_str("cArrNum");
        op.parameters[13].type = cdd_test_mut_str("array");
        op.parameters[13].items_type = cdd_test_mut_str("number");
        op.parameters[13].is_array = 1;
        op.parameters[14].in = OA_PARAM_IN_HEADER;
        op.parameters[14].name = cdd_test_mut_str("hArrJson");
        op.parameters[14].type = cdd_test_mut_str("array");
        op.parameters[14].items_type = cdd_test_mut_str("object");
        op.parameters[14].content_type = cdd_test_mut_str("application/json");
        op.parameters[14].is_array = 1;
        op.parameters[15].in = OA_PARAM_IN_QUERY;
        op.parameters[15].name = cdd_test_mut_str("qObjNoExp");
        op.parameters[15].type = cdd_test_mut_str("object");
        op.parameters[15].style = OA_STYLE_FORM;
        op.parameters[15].explode = 0;
        op.parameters[15].explode_set = 1;
        op.parameters[16].in = OA_PARAM_IN_QUERY;
        op.parameters[16].name = cdd_test_mut_str("qArrIntEnc");
        op.parameters[16].type = cdd_test_mut_str("array");
        op.parameters[16].items_type = cdd_test_mut_str("integer");
        op.parameters[16].style = OA_STYLE_FORM;
        op.parameters[16].explode = 0;
        op.parameters[16].explode_set = 1;
        op.parameters[16].is_array = 1;
        op.parameters[17].in = OA_PARAM_IN_QUERY;
        op.parameters[17].name = cdd_test_mut_str("qArrNumEnc");
        op.parameters[17].type = cdd_test_mut_str("array");
        op.parameters[17].items_type = cdd_test_mut_str("number");
        op.parameters[17].style = OA_STYLE_FORM;
        op.parameters[17].explode = 0;
        op.parameters[17].explode_set = 1;
        op.parameters[17].is_array = 1;
        op.parameters[18].in = OA_PARAM_IN_QUERY;
        op.parameters[18].name = cdd_test_mut_str("qArrBoolEnc");
        op.parameters[18].type = cdd_test_mut_str("array");
        op.parameters[18].items_type = cdd_test_mut_str("boolean");
        op.parameters[18].style = OA_STYLE_FORM;
        op.parameters[18].explode = 0;
        op.parameters[18].explode_set = 1;
        op.parameters[18].is_array = 1;
        op.parameters[19].in = OA_PARAM_IN_HEADER;
        op.parameters[19].name = cdd_test_mut_str("hObjNoExp");
        op.parameters[19].type = cdd_test_mut_str("object");
        op.parameters[19].style = OA_STYLE_FORM;
        op.parameters[19].explode = 0;
        op.parameters[19].explode_set = 1;
        op.parameters[20].in = OA_PARAM_IN_COOKIE;
        op.parameters[20].name = cdd_test_mut_str("cObjNoExp");
        op.parameters[20].type = cdd_test_mut_str("object");
        op.parameters[20].style = OA_STYLE_FORM;
        op.parameters[20].explode = 0;
        op.parameters[20].explode_set = 1;

        op.req_body.is_array = 1;
        op.req_body.inline_type = cdd_test_mut_str("number");
        op.req_body.content_type = cdd_test_mut_str("application/json");

        g_io_calls = 0;
        g_fail_io_after = io_fail;
        rc = codegen_client_write_body(fp, &op, &spec, "/path/{pNum}/{pBool}",
                                       NULL);
        g_fail_io_after = -1;

        free(op.parameters);
        if (fp)
          fclose(fp);
        if (rc == CDD_C_SUCCESS)
          break;
      }
    }
  }
  PASS();
}

/**
 * @brief Unit test test_client_body_inline_response_types.
 * @return Greatest test result.
 */
TEST test_client_body_inline_response_types(void) {
  int io_fail;
  for (io_fail = 0; io_fail < 3000; ++io_fail) {
    /* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
    /* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
    {
      struct OpenAPI_Spec spec = {0};
      struct OpenAPI_Operation op = {0};
      struct OpenAPI_Response resp = {0};
      FILE *fp;
      int rc = 0;
      int all_success = 1;

      /* integer response */
      memset(&op, 0, sizeof(op));
      memset(&resp, 0, sizeof(resp));
      resp.code = cdd_test_mut_str("200");
      resp.schema.inline_type = cdd_test_mut_str("integer");
      op.n_responses = 1;
      op.responses = &resp;
      op.n_req_body_media_types = 1;
      op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
      op.req_body_media_types[0].name = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      free(op.req_body_media_types);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      /* boolean response */
      memset(&op, 0, sizeof(op));
      memset(&resp, 0, sizeof(resp));
      resp.code = cdd_test_mut_str("200");
      resp.schema.inline_type = cdd_test_mut_str("boolean");
      op.n_responses = 1;
      op.responses = &resp;
      op.n_req_body_media_types = 1;
      op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
      op.req_body_media_types[0].name = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      free(op.req_body_media_types);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      /* number response */
      memset(&op, 0, sizeof(op));
      memset(&resp, 0, sizeof(resp));
      resp.code = cdd_test_mut_str("200");
      resp.schema.inline_type = cdd_test_mut_str("number");
      op.n_responses = 1;
      op.responses = &resp;
      op.n_req_body_media_types = 1;
      op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
      op.req_body_media_types[0].name = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      free(op.req_body_media_types);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      /* string response */
      memset(&op, 0, sizeof(op));
      memset(&resp, 0, sizeof(resp));
      resp.code = cdd_test_mut_str("200");
      resp.schema.inline_type = cdd_test_mut_str("string");
      op.n_responses = 1;
      op.responses = &resp;
      op.n_req_body_media_types = 1;
      op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
      op.req_body_media_types[0].name = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      free(op.req_body_media_types);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      /* array of boolean response */
      memset(&op, 0, sizeof(op));
      memset(&resp, 0, sizeof(resp));
      resp.code = cdd_test_mut_str("200");
      resp.schema.is_array = 1;
      resp.schema.inline_type = cdd_test_mut_str("boolean");
      op.n_responses = 1;
      op.responses = &resp;
      op.n_req_body_media_types = 1;
      op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
      op.req_body_media_types[0].name = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      free(op.req_body_media_types);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      /* array of string response */
      memset(&op, 0, sizeof(op));
      memset(&resp, 0, sizeof(resp));
      resp.code = cdd_test_mut_str("200");
      resp.schema.is_array = 1;
      resp.schema.inline_type = cdd_test_mut_str("string");
      op.n_responses = 1;
      op.responses = &resp;
      op.n_req_body_media_types = 1;
      op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
      op.req_body_media_types[0].name = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      free(op.req_body_media_types);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      if (all_success)
        break;
    }
  }

  /* invalid response inline type */
  {
    struct OpenAPI_Spec spec = {0};
    struct OpenAPI_Operation op = {0};
    struct OpenAPI_Response resp = {0};
    FILE *fp;
    int rc = 0;

    memset(&op, 0, sizeof(op));
    memset(&resp, 0, sizeof(resp));
    resp.code = cdd_test_mut_str("200");
    resp.schema.inline_type = cdd_test_mut_str("invalid_type");
    op.responses = &resp;
    op.n_responses = 1;
    op.n_req_body_media_types = 1;
    op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
    op.req_body_media_types[0].name = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
    if (((fp = cdd_test_tmpfile_global()) == NULL))
      fp = NULL;
#else
    fp = cdd_test_tmpfile_global();
#endif
    rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
    if (fp)
      fclose(fp);
    free(op.req_body_media_types);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  PASS();
}

/**
 * @brief Unit test test_client_body_inline_types.
 * @return Greatest test result.
 */
TEST test_client_body_inline_types(void) {
  int io_fail;
  for (io_fail = 0; io_fail < 3000; ++io_fail) {
    /* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
    /* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
    {
      struct OpenAPI_Spec spec = {0};
      struct OpenAPI_Operation op = {0};
      FILE *fp;
      int rc = 0;
      int all_success = 1;

      /* integer */
      memset(&op, 0, sizeof(op));
      op.req_body.inline_type = cdd_test_mut_str("integer");
      op.req_body.content_type = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      /* boolean */
      memset(&op, 0, sizeof(op));
      op.req_body.inline_type = cdd_test_mut_str("boolean");
      op.req_body.content_type = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      /* number */
      memset(&op, 0, sizeof(op));
      op.req_body.inline_type = cdd_test_mut_str("number");
      op.req_body.content_type = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      /* array of integer */
      memset(&op, 0, sizeof(op));
      op.req_body.is_array = 1;
      op.req_body.inline_type = cdd_test_mut_str("integer");
      op.req_body.content_type = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      /* array of boolean */
      memset(&op, 0, sizeof(op));
      op.req_body.is_array = 1;
      op.req_body.inline_type = cdd_test_mut_str("boolean");
      op.req_body.content_type = cdd_test_mut_str("application/json");
#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      if (rc != CDD_C_SUCCESS)
        all_success = 0;

      if (all_success)
        break;
    }
  }
  PASS();
}

/**
 * @brief Unit test test_client_body_form_types.
 * @return Greatest test result.
 */
TEST test_client_body_form_types(void) {
  int io_fail;
  for (io_fail = 0; io_fail < 3000; ++io_fail) {
    /* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
    /* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
    {
      struct OpenAPI_Spec spec = {0};
      struct OpenAPI_Operation op = {0};
      FILE *fp;
      int rc = 0;
      int all_success = 1;

      memset(&op, 0, sizeof(op));
      op.req_body.content_type =
          cdd_test_mut_str("application/x-www-form-urlencoded");
      op.req_body.n_multipart_fields = 4;
      op.req_body.multipart_fields =
          calloc(4, sizeof(*op.req_body.multipart_fields));
      op.req_body.multipart_fields[0].name = cdd_test_mut_str("fStr");
      op.req_body.multipart_fields[0].type = cdd_test_mut_str("string");
      op.req_body.multipart_fields[1].name = cdd_test_mut_str("fInt");
      op.req_body.multipart_fields[1].type = cdd_test_mut_str("integer");
      op.req_body.multipart_fields[2].name = cdd_test_mut_str("fNum");
      op.req_body.multipart_fields[2].type = cdd_test_mut_str("number");
      op.req_body.multipart_fields[3].name = cdd_test_mut_str("fBool");
      op.req_body.multipart_fields[3].type = cdd_test_mut_str("boolean");

#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      free(op.req_body.multipart_fields);

      if (rc != CDD_C_SUCCESS)
        all_success = 0;
      if (all_success)
        break;
    }
  }
  PASS();
}

/**
 * @brief Unit test test_client_body_multipart_types.
 * @return Greatest test result.
 */
TEST test_client_body_multipart_types(void) {
  int io_fail;
  for (io_fail = 0; io_fail < 3000; ++io_fail) {
    /* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
    /* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
    {
      struct OpenAPI_Spec spec = {0};
      struct OpenAPI_Operation op = {0};
      FILE *fp;
      int rc = 0;
      int all_success = 1;

      memset(&op, 0, sizeof(op));
      op.req_body.content_type = cdd_test_mut_str("multipart/form-data");
      op.req_body.n_multipart_fields = 4;
      op.req_body.multipart_fields =
          calloc(4, sizeof(*op.req_body.multipart_fields));
      op.req_body.multipart_fields[0].name = cdd_test_mut_str("mStr");
      op.req_body.multipart_fields[0].type = cdd_test_mut_str("string");
      op.req_body.multipart_fields[1].name = cdd_test_mut_str("mInt");
      op.req_body.multipart_fields[1].type = cdd_test_mut_str("integer");
      op.req_body.multipart_fields[2].name = cdd_test_mut_str("mNum");
      op.req_body.multipart_fields[2].type = cdd_test_mut_str("number");
      op.req_body.multipart_fields[3].name = cdd_test_mut_str("mBool");
      op.req_body.multipart_fields[3].type = cdd_test_mut_str("boolean");

#if defined(_MSC_VER)
      if (((fp = cdd_test_tmpfile_global()) == NULL))
        fp = NULL;
#else
      fp = cdd_test_tmpfile_global();
#endif
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
      g_fail_io_after = -1;
      if (fp)
        fclose(fp);
      free(op.req_body.multipart_fields);

      if (rc != CDD_C_SUCCESS)
        all_success = 0;
      if (all_success)
        break;
    }
  }
  PASS();
}

/**
 * @brief Executes the client_body_types_suite test suite.
 */
SUITE(client_body_types_suite) {
  RUN_TEST(test_body_header_param_string);
  RUN_TEST(test_body_header_param_integer);
  RUN_TEST(test_body_header_param_number);
  RUN_TEST(test_body_header_param_boolean);
  RUN_TEST(test_client_body_all_primitive_types);
  RUN_TEST(test_client_body_inline_response_types);
  RUN_TEST(test_client_body_inline_types);
  RUN_TEST(test_client_body_form_types);
  RUN_TEST(test_client_body_multipart_types);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_TYPES_H */
