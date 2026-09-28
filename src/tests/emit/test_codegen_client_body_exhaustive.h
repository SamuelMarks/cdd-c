/**
 * @file test_codegen_client_body_exhaustive.h
 * @brief Exhaustive coverage tests for client body (part 2).
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_EXHAUSTIVE_H
#define TEST_CODEGEN_CLIENT_BODY_EXHAUSTIVE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/emit/struct.h"
#include "functions/emit/client_body.h"
#include "functions/parse/str.h"
#include "greatest.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

#ifndef C_CDD_STR_LIT
#define C_CDD_STR_LIT(s) ((char *)(size_t)(size_t)(s))
#endif

extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_fail_is_primitive_type;

#include "emit/test_codegen_client_body_helpers.h"

TEST test_client_body_exhaustive_remaining(void) {
  FILE *fp;

  /* Section 5: client_body_write_multipart_part_headers full branches */
  {
    struct OpenAPI_Encoding enc;
    struct OpenAPI_Header hdrs[4];
    memset(&enc, 0, sizeof(enc));
    memset(hdrs, 0, sizeof(hdrs));

    /* NULL / empty enc checks */
    fp = cdd_test_tmpfile_global();
    ASSERT(fp);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              client_body_write_multipart_part_headers(NULL, &enc));
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, &enc));
    enc.name = C_CDD_STR_LIT("part1");
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, &enc));
    enc.headers = hdrs;
    enc.n_headers = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, &enc));
    enc.n_headers = 1;
    enc.name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, &enc));
    enc.name = C_CDD_STR_LIT("part1");
    fclose(fp);

    /* Headers with full branch coverage */
    hdrs[0].name = C_CDD_STR_LIT("X-Hdr-Arr-Bool");
    hdrs[0].is_array = 0;
    hdrs[0].type = C_CDD_STR_LIT("array");
    hdrs[0].items_type = C_CDD_STR_LIT("boolean");

    hdrs[1].name = C_CDD_STR_LIT("X-Hdr-Obj-Exp");
    hdrs[1].type = C_CDD_STR_LIT("object");
    hdrs[1].explode_set = 1;
    hdrs[1].explode = 1;

    hdrs[2].name = C_CDD_STR_LIT("X-Hdr-Int");
    hdrs[2].type = C_CDD_STR_LIT("integer");

    hdrs[3].name = C_CDD_STR_LIT("X-Hdr-Num");
    hdrs[3].type = C_CDD_STR_LIT("number");

    enc.headers = hdrs;
    enc.n_headers = 4;

    test_helper_run_io_part_headers(&enc);
  }

  /* Section 6: client_body_write_multipart_body empty content_type */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation op;
    struct OpenAPI_MediaType mt;
    struct OpenAPI_Encoding encs[6];
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    spec.defined_schemas =
        (struct StructFields *)calloc(4, sizeof(struct StructFields));
    spec.defined_schema_names = (char **)calloc(4, sizeof(char *));
    ASSERT(spec.defined_schemas);
    ASSERT(spec.defined_schema_names);
    memset(&op, 0, sizeof(op));
    memset(&mt, 0, sizeof(mt));
    memset(encs, 0, sizeof(encs));

    /* NULL checks */
    fp = cdd_test_tmpfile_global();
    ASSERT(fp);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              client_body_write_multipart_body(NULL, &op, &spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              client_body_write_multipart_body(fp, NULL, &spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              client_body_write_multipart_body(fp, &op, NULL));
    fclose(fp);

    /* Schema with all types */
    struct_fields_init(&spec.defined_schemas[0]);
    struct_fields_add(&spec.defined_schemas[0], "f_arr", "array", "string",
                      NULL, NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_arr_unsupp", "array", "enum",
                      NULL, NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_str", "string", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_int", "integer", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_num", "number", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_bool", "boolean", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_obj", "object", "SubM", NULL,
                      NULL);
    c_cdd_strdup("MultiEmptyCT", &spec.defined_schema_names[0]);

    struct_fields_init(&spec.defined_schemas[1]);
    struct_fields_add(&spec.defined_schemas[1], "id", "integer", NULL, NULL,
                      NULL);
    c_cdd_strdup("SubM", &spec.defined_schema_names[1]);
    spec.n_defined_schemas = 2;

    /* Encodings with empty string content_type */
    encs[0].name = C_CDD_STR_LIT("f_arr");
    encs[0].content_type = C_CDD_STR_LIT("");
    encs[1].name = C_CDD_STR_LIT("f_str");
    encs[1].content_type = C_CDD_STR_LIT("");
    encs[2].name = C_CDD_STR_LIT("f_int");
    encs[2].content_type = C_CDD_STR_LIT("");
    encs[3].name = C_CDD_STR_LIT("f_num");
    encs[3].content_type = C_CDD_STR_LIT("");
    encs[4].name = C_CDD_STR_LIT("f_bool");
    encs[4].content_type = C_CDD_STR_LIT("");
    encs[5].name = C_CDD_STR_LIT("f_obj");
    encs[5].content_type = C_CDD_STR_LIT("");

    mt.name = C_CDD_STR_LIT("multipart/form-data");
    mt.encoding = encs;
    mt.n_encoding = 6;

    op.operation_id = C_CDD_STR_LIT("testMultiEmptyCT");
    op.verb = OA_VERB_POST;
    op.method = C_CDD_STR_LIT("post");
    op.req_body.ref_name = C_CDD_STR_LIT("MultiEmptyCT");
    op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");
    op.req_body_media_types = &mt;
    op.n_req_body_media_types = 1;

    test_helper_run_io_multipart(&op, &spec);
    openapi_spec_free(&spec);
  }

  /* Section 7: codegen_client_write_body all remaining branches */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation op;
    struct OpenAPI_Response responses[3];
    struct OpenAPI_SecurityScheme sch_q;
    struct OpenAPI_SecurityScheme sch_c;
    struct OpenAPI_Parameter params[3];

    /* 7a: spec == NULL */
    memset(&op, 0, sizeof(op));
    op.operation_id = C_CDD_STR_LIT("testNoSpec");
    op.verb = OA_VERB_GET;
    op.method = C_CDD_STR_LIT("get");
    test_helper_run_io_client_body(&op, NULL, "/no_spec");

    /* 7b: security_query without query params, security_cookie without cookie
     * params */
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&sch_q, 0, sizeof(sch_q));
    sch_q.type = OA_SEC_APIKEY;
    sch_q.in = OA_SEC_IN_QUERY;
    c_cdd_strdup("sec_query", &sch_q.name);
    c_cdd_strdup("api_key", &sch_q.key_name);

    memset(&sch_c, 0, sizeof(sch_c));
    sch_c.type = OA_SEC_APIKEY;
    sch_c.in = OA_SEC_IN_COOKIE;
    c_cdd_strdup("sec_cookie", &sch_c.name);
    c_cdd_strdup("session_id", &sch_c.key_name);

    spec.security_schemes =
        (struct OpenAPI_SecurityScheme *)calloc(2, sizeof(sch_q));
    ASSERT(spec.security_schemes);
    spec.security_schemes[0] = sch_q;
    spec.security_schemes[1] = sch_c;
    spec.n_security_schemes = 2;
    spec.security_set = 0;
    op.security_set = 0;

    /* Also path parameter with name NULL, and regular parameter with name NULL
     */
    memset(params, 0, sizeof(params));
    params[0].in = OA_PARAM_IN_PATH;
    params[0].name = NULL;
    params[1].in = OA_PARAM_IN_UNKNOWN;
    params[1].name = NULL;
    op.parameters = params;
    op.n_parameters = 2;

    /* req_body with ref_name but content_type NULL, is_array 0 */
    op.req_body.ref_name = C_CDD_STR_LIT("ReqModel");
    op.req_body.content_type = NULL;
    op.req_body.is_array = 0;
    test_helper_run_io_client_body(&op, &spec, "/sec_and_null_names");

    /* Also test req_body.ref_name with is_array 1 for line 4285 and req_is_json
     * for line 4418 */
    op.req_body.is_array = 1;
    op.req_body.content_type = C_CDD_STR_LIT("application/json");
    test_helper_run_io_client_body(&op, &spec, "/ref_arr_body");

    /* Responses with is_array 0 */
    memset(responses, 0, sizeof(responses));
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.ref_name = C_CDD_STR_LIT("ResModel");
    responses[0].schema.is_array = 0;
    op.responses = responses;
    op.n_responses = 1;

    openapi_spec_free(&spec);

    /* 7c: Inline req_body JSON array & scalar types (string, integer, number,
     * boolean, custom) */
    {
      const char *inl_types[5];
      size_t t;
      inl_types[0] = "string";
      inl_types[1] = "integer";
      inl_types[2] = "number";
      inl_types[3] = "boolean";
      inl_types[4] = "custom_unsupported";

      for (t = 0; t < 5; ++t) {
        ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
        memset(&op, 0, sizeof(op));
        op.operation_id = C_CDD_STR_LIT("testInlReq");
        op.verb = OA_VERB_POST;
        op.method = C_CDD_STR_LIT("post");
        op.req_body.content_type = C_CDD_STR_LIT("application/json");
        op.req_body.inline_type = C_CDD_STR_LIT(inl_types[t]);
        op.req_body.is_array = 1;
        test_helper_run_io_client_body(&op, &spec, "/inl_arr");

        op.req_body.is_array = 0;
        test_helper_run_io_client_body(&op, &spec, "/inl_sc");
        openapi_spec_free(&spec);
      }
    }

    /* 7d: op.method empty string and custom unsupported method */
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&op, 0, sizeof(op));
    op.operation_id = C_CDD_STR_LIT("testEmptyMethod");
    op.verb = OA_VERB_UNKNOWN;
    op.is_additional = 1;
    op.method = C_CDD_STR_LIT("");
    test_helper_run_io_client_body(&op, &spec, "/empty_method");

    op.method = C_CDD_STR_LIT("CUSTOM");
    test_helper_run_io_client_body(&op, &spec, "/custom_method");
    openapi_spec_free(&spec);

    /* 7e: Default response mismatch checks and def_has_inline */
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&op, 0, sizeof(op));
    memset(responses, 0, sizeof(responses));
    op.operation_id = C_CDD_STR_LIT("testDefMismatch");
    op.verb = OA_VERB_GET;
    op.method = C_CDD_STR_LIT("get");

    /* Success 200 response */
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.ref_name = C_CDD_STR_LIT("ModelA");
    responses[0].schema.inline_type = C_CDD_STR_LIT("string");
    responses[0].schema.is_array = 0;

    /* Default response with different ref_name and different inline_type */
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].schema.ref_name = C_CDD_STR_LIT("ModelB");
    responses[1].schema.inline_type = C_CDD_STR_LIT("integer");
    responses[1].schema.is_array = 1;
    op.responses = responses;
    op.n_responses = 2;
    test_helper_run_io_client_body(&op, &spec, "/def_mismatch");

    /* Response schema has is_array 1 but ref_name and inline_type NULL (L4324)
     */
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.ref_name = NULL;
    responses[0].schema.inline_type = NULL;
    responses[0].schema.is_array = 1;
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].schema.ref_name = NULL;
    responses[1].schema.inline_type = NULL;
    responses[1].schema.is_array = 0;
    test_helper_run_io_client_body(&op, &spec, "/null_ref_arr_resps");

    /* Default response schema has is_array 1 with no success response (L4340)
     */
    responses[0].code = C_CDD_STR_LIT("404");
    responses[0].schema.is_array = 0;
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].schema.is_array = 1;
    test_helper_run_io_client_body(&op, &spec, "/null_def_arr_resps");

    /* Two 2xx responses with inline types (L4651) */
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.inline_type = C_CDD_STR_LIT("string");
    responses[0].schema.is_array = 0;
    responses[1].code = C_CDD_STR_LIT("201");
    responses[1].schema.inline_type = C_CDD_STR_LIT("integer");
    responses[1].schema.is_array = 0;
    test_helper_run_io_client_body(&op, &spec, "/two_2xx_inlines");

    /* Default response matching inline type but differing is_array (L4844) */
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
    responses[0].schema.is_array = 0;
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].schema.inline_type = C_CDD_STR_LIT("integer");
    responses[1].schema.is_array = 1;
    test_helper_run_io_client_body(&op, &spec, "/def_arr_mismatch");

    /* Default response matches success with def_has_inline when
     * default_is_success */
    responses[0].code = C_CDD_STR_LIT("400");
    responses[0].schema.ref_name = NULL;
    responses[0].schema.inline_type = NULL;
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].content_type = C_CDD_STR_LIT("application/json");
    responses[1].schema.ref_name = NULL;
    responses[1].schema.inline_type = C_CDD_STR_LIT("string");
    responses[1].schema.is_array = 0;
    test_helper_run_io_client_body(&op, &spec, "/def_inline_success");

    openapi_spec_free(&spec);
  }
  /* 7f: Form body in codegen_client_write_body */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation f_op;
    struct OpenAPI_MediaType f_mt;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    spec.defined_schemas =
        (struct StructFields *)calloc(1, sizeof(struct StructFields));
    spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
    struct_fields_init(&spec.defined_schemas[0]);
    struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                      NULL);
    c_cdd_strdup("FormModel", &spec.defined_schema_names[0]);
    spec.n_defined_schemas = 1;

    memset(&f_op, 0, sizeof(f_op));
    memset(&f_mt, 0, sizeof(f_mt));
    f_op.operation_id = C_CDD_STR_LIT("testFormBody");
    f_op.verb = OA_VERB_POST;
    f_op.method = C_CDD_STR_LIT("post");
    f_op.req_body.ref_name = C_CDD_STR_LIT("FormModel");
    f_op.req_body.content_type =
        C_CDD_STR_LIT("application/x-www-form-urlencoded");
    f_mt.name = C_CDD_STR_LIT("application/x-www-form-urlencoded");
    f_op.req_body_media_types = &f_mt;
    f_op.n_req_body_media_types = 1;
    test_helper_run_io_client_body(&f_op, &spec, "/form_body");
    openapi_spec_free(&spec);
  }

  /* 7g: Text plain body in codegen_client_write_body */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation txt_op;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&txt_op, 0, sizeof(txt_op));
    txt_op.operation_id = C_CDD_STR_LIT("testTxtBody");
    txt_op.verb = OA_VERB_POST;
    txt_op.method = C_CDD_STR_LIT("post");
    txt_op.req_body.content_type = C_CDD_STR_LIT("text/plain");
    test_helper_run_io_client_body(&txt_op, &spec, "/txt_body");
    openapi_spec_free(&spec);
  }

  /* 7h: No success responses (only 404), hitting line 4351 success_schema ==
   * NULL */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation no_succ_op;
    struct OpenAPI_Response no_succ_res;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&no_succ_op, 0, sizeof(no_succ_op));
    memset(&no_succ_res, 0, sizeof(no_succ_res));
    no_succ_op.operation_id = C_CDD_STR_LIT("testNoSuccess");
    no_succ_op.verb = OA_VERB_GET;
    no_succ_op.method = C_CDD_STR_LIT("get");
    no_succ_res.code = C_CDD_STR_LIT("404");
    no_succ_op.responses = &no_succ_res;
    no_succ_op.n_responses = 1;
    test_helper_run_io_client_body(&no_succ_op, &spec, "/no_success");
    openapi_spec_free(&spec);
  }

  /* 7i: 2XX has inline type but 200 does not, hitting line 4659 */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation r2_op;
    struct OpenAPI_Response r2_res[2];
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&r2_op, 0, sizeof(r2_op));
    memset(r2_res, 0, sizeof(r2_res));
    r2_op.operation_id = C_CDD_STR_LIT("testR2Inline");
    r2_op.verb = OA_VERB_GET;
    r2_op.method = C_CDD_STR_LIT("get");
    r2_res[0].code = C_CDD_STR_LIT("200");
    r2_res[0].schema.ref_name = C_CDD_STR_LIT("ModelA");
    r2_res[1].code = C_CDD_STR_LIT("2XX");
    r2_res[1].schema.inline_type = C_CDD_STR_LIT("integer");
    r2_op.responses = r2_res;
    r2_op.n_responses = 2;
    test_helper_run_io_client_body(&r2_op, &spec, "/r2_inline");
    openapi_spec_free(&spec);
  }

  /* 7j: Default response has ref_name when success has matching ref_name,
   * hitting lines 4869-4882 */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation def_ref_op;
    struct OpenAPI_Response def_ref_res[2];
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&def_ref_op, 0, sizeof(def_ref_op));
    memset(def_ref_res, 0, sizeof(def_ref_res));
    def_ref_op.operation_id = C_CDD_STR_LIT("testDefRef");
    def_ref_op.verb = OA_VERB_GET;
    def_ref_op.method = C_CDD_STR_LIT("get");
    def_ref_res[0].code = C_CDD_STR_LIT("200");
    def_ref_res[0].schema.ref_name = C_CDD_STR_LIT("CommonModel");
    def_ref_res[1].code = C_CDD_STR_LIT("default");
    def_ref_res[1].schema.ref_name = C_CDD_STR_LIT("CommonModel");
    def_ref_res[1].schema.is_array = 0;
    def_ref_op.responses = def_ref_res;
    def_ref_op.n_responses = 2;
    test_helper_run_io_client_body(&def_ref_op, &spec, "/def_ref");

    /* Also default response array with ref_name for line 4863 */
    def_ref_res[1].schema.is_array = 1;
    test_helper_run_io_client_body(&def_ref_op, &spec, "/def_ref_arr");
    openapi_spec_free(&spec);
  }
  /* Binary request body (lines 4550-4552) */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation bin_op;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&bin_op, 0, sizeof(bin_op));
    bin_op.operation_id = C_CDD_STR_LIT("testBinBody");
    bin_op.verb = OA_VERB_POST;
    bin_op.method = C_CDD_STR_LIT("post");
    bin_op.req_body.content_type = C_CDD_STR_LIT("application/octet-stream");
    test_helper_run_io_client_body(&bin_op, &spec, "/bin_body");
    openapi_spec_free(&spec);
  }

  /* Range 2XX empty schema (line 4786) and 2XX text/plain (lines 4778-4782) */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation r_empty_op;
    struct OpenAPI_Response r_resps[2];
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&r_empty_op, 0, sizeof(r_empty_op));
    memset(r_resps, 0, sizeof(r_resps));
    r_empty_op.operation_id = C_CDD_STR_LIT("testRangeEmpty");
    r_empty_op.verb = OA_VERB_GET;
    r_empty_op.method = C_CDD_STR_LIT("get");
    r_resps[0].code = C_CDD_STR_LIT("2XX");
    r_empty_op.responses = r_resps;
    r_empty_op.n_responses = 1;
    test_helper_run_io_client_body(&r_empty_op, &spec, "/r_empty");

    r_resps[0].content_type = C_CDD_STR_LIT("application/octet-stream");
    test_helper_run_io_client_body(&r_empty_op, &spec, "/r_bin");

    /* 2XX with ref_name for lines 4778-4782 */
    r_resps[0].content_type = NULL;
    r_resps[0].schema.ref_name = C_CDD_STR_LIT("CommonModel");
    test_helper_run_io_client_body(&r_empty_op, &spec, "/r_ref");
    openapi_spec_free(&spec);
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_EXHAUSTIVE_H */
