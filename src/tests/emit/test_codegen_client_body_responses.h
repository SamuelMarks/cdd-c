/**
 * @file test_codegen_client_body_responses.h
 * @brief Unit tests for client body response handling and media types.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_RESPONSES_H
#define TEST_CODEGEN_CLIENT_BODY_RESPONSES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_body_common.h"
/* clang-format on */

/**
 * @brief Unit test test_body_response_range_success.
 * @return Greatest test result.
 */
TEST test_body_response_range_success(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_36 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("2XX");
  resp.schema.ref_name = cdd_test_mut_str("Pet");
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_36), _ast_gen_body_36);
  ASSERT(code);
  ASSERT(strstr(code, "status_code >= 200") != NULL);
  ASSERT(strstr(code, "Pet_from_json") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_body_default_response_success.
 * @return Greatest test result.
 */
TEST test_body_default_response_success(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_37 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("default");
  resp.schema.ref_name = cdd_test_mut_str("Pet");
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_37), _ast_gen_body_37);
  ASSERT(code);
  ASSERT(strstr(code, "default response") != NULL);
  ASSERT(strstr(code, "Pet_from_json") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_body_text_plain_response_string.
 * @return Greatest test result.
 */
TEST test_body_text_plain_response_string(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_38 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("200");
  resp.content_type = cdd_test_mut_str("text/plain; charset=utf-8");
  resp.schema.inline_type = cdd_test_mut_str("string");
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_38), _ast_gen_body_38);
  ASSERT(code);
  ASSERT(strstr(code, "memcpy(tmp, res->body") != NULL);
  ASSERT(strstr(code, "*out = tmp") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_body_text_plain_response_range.
 * @return Greatest test result.
 */
TEST test_body_text_plain_response_range(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_39 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("2XX");
  resp.content_type = cdd_test_mut_str("text/plain");
  resp.schema.inline_type = cdd_test_mut_str("string");
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_39), _ast_gen_body_39);
  ASSERT(code);
  ASSERT(strstr(code, "status_code >= 200") != NULL);
  ASSERT(strstr(code, "memcpy(tmp, res->body") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_body_text_plain_response_default.
 * @return Greatest test result.
 */
TEST test_body_text_plain_response_default(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_40 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("default");
  resp.content_type = cdd_test_mut_str("text/plain");
  resp.schema.inline_type = cdd_test_mut_str("string");
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_40), _ast_gen_body_40);
  ASSERT(code);
  ASSERT(strstr(code, "default response") != NULL);
  ASSERT(strstr(code, "memcpy(tmp, res->body") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_body_textual_response_xml.
 * @return Greatest test result.
 */
TEST test_body_textual_response_xml(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_41 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("200");
  resp.content_type = cdd_test_mut_str("application/xml; charset=utf-8");
  resp.schema.inline_type = cdd_test_mut_str("string");
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_41), _ast_gen_body_41);
  ASSERT(code);
  ASSERT(strstr(code, "memcpy(tmp, res->body") != NULL);
  ASSERT(strstr(code, "*out = tmp") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_body_binary_response_pdf.
 * @return Greatest test result.
 */
TEST test_body_binary_response_pdf(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_42 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = cdd_test_mut_str("200");
  resp.content_type = cdd_test_mut_str("application/pdf");
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_42), _ast_gen_body_42);
  ASSERT(code);
  ASSERT(strstr(code, "unsigned char *tmp") != NULL);
  ASSERT(strstr(code, "*out_len = res->body_len") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_client_body_verb_mapping.
 * @return Greatest test result.
 */
TEST test_client_body_verb_mapping(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = cdd_test_mut_str("testVerb");

  /* Test additional non-standard verbs */
  op.is_additional = 1;

  op.method = NULL;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("get");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("post");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("put");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("delete");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("head");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("patch");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("options");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("trace");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("query");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = cdd_test_mut_str("unknown");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_client_body_mapped_err_code.
 * @return Greatest test result.
 */
TEST test_client_body_mapped_err_code(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = cdd_test_mut_str("testErrCode");
  op.n_responses = 5;
  op.responses = calloc(5, sizeof(*op.responses));
  op.responses[0].code = cdd_test_mut_str("400");
  op.responses[1].code = cdd_test_mut_str("401");
  op.responses[2].code = cdd_test_mut_str("403");
  op.responses[3].code = cdd_test_mut_str("404");
  op.responses[4].code = cdd_test_mut_str("500");

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(op.responses);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_client_body_media_type_matching.
 * @return Greatest test result.
 */
TEST test_client_body_media_type_matching(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = cdd_test_mut_str("post");
  op.verb = OA_VERB_POST;
  op.operation_id = cdd_test_mut_str("testMediaMatch");
  op.summary = cdd_test_mut_str("Test summary");

  op.req_body.content_type = cdd_test_mut_str("application/vnd.github+JSON");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type = cdd_test_mut_str("APPLICATION/XML");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type = cdd_test_mut_str("multipart/form-data");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type = cdd_test_mut_str("text/plain");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_client_body_find_media_type_not_found.
 * @return Greatest test result.
 */
TEST test_client_body_find_media_type_not_found(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = cdd_test_mut_str("testMediaTypeFindNotFound");
  op.method = cdd_test_mut_str("post");
  op.verb = OA_VERB_POST;

  op.req_body.content_type =
      cdd_test_mut_str("application/x-www-form-urlencoded");
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name =
      cdd_test_mut_str("application/json"); /* Doesn't match */

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type = cdd_test_mut_str("multipart/form-data");
  op.req_body_media_types[0].name =
      cdd_test_mut_str("application/xml"); /* Doesn't match */

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(op.req_body_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_client_body_find_encoding_not_found.
 * @return Greatest test result.
 */
TEST test_client_body_find_encoding_not_found(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = cdd_test_mut_str("testEncodingFindNotFound");
  op.method = cdd_test_mut_str("post");
  op.verb = OA_VERB_POST;

  op.req_body.content_type = cdd_test_mut_str("multipart/form-data");
  op.req_body.ref_name = cdd_test_mut_str("MockSchema");

  /* Provide req_body_media_types */
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = cdd_test_mut_str("multipart/form-data");

  memset(&enc, 0, sizeof(enc));
  enc.name = cdd_test_mut_str("other_prop"); /* Different from test_prop */
  enc.content_type = cdd_test_mut_str("text/plain");

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  /* Setup the global Spec schema definitions */
  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  c_cdd_strdup("MockSchema", &spec.defined_schema_names[0]);

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
  strcpy(spec.defined_schemas[0].fields[0].type, "string");
#endif
#endif

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  if (spec.defined_schema_names && spec.defined_schema_names[0])
    free(spec.defined_schema_names[0]);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_client_body_array_items_statics.
 * @return Greatest test result.
 */
TEST test_client_body_array_items_statics(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = cdd_test_mut_str("testArrayItemsStatics");
  op.method = cdd_test_mut_str("post");
  op.verb = OA_VERB_POST;

  op.req_body.content_type = cdd_test_mut_str("multipart/form-data");
  op.req_body.ref_name = cdd_test_mut_str("MockSchema2");

  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = cdd_test_mut_str("multipart/form-data");

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  c_cdd_strdup("MockSchema2", &spec.defined_schema_names[0]);

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 5;
  spec.defined_schemas[0].fields = calloc(5, sizeof(struct StructField));

/* Field 0: array of object */
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "arr_obj");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "arr_obj");
#else
  strcpy(spec.defined_schemas[0].fields[0].name, "arr_obj");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "array");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "array");
#else
  strcpy(spec.defined_schemas[0].fields[0].type, "array");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].ref,
           sizeof(spec.defined_schemas[0].fields[0].ref), "object");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].ref,
           sizeof(spec.defined_schemas[0].fields[0].ref), "object");
#else
  strcpy(spec.defined_schemas[0].fields[0].ref, "object");
#endif
#endif

/* Field 1: array of array */
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[1].name,
           sizeof(spec.defined_schemas[0].fields[1].name), "arr_arr");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[1].name,
           sizeof(spec.defined_schemas[0].fields[1].name), "arr_arr");
#else
  strcpy(spec.defined_schemas[0].fields[1].name, "arr_arr");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[1].type,
           sizeof(spec.defined_schemas[0].fields[1].type), "array");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[1].type,
           sizeof(spec.defined_schemas[0].fields[1].type), "array");
#else
  strcpy(spec.defined_schemas[0].fields[1].type, "array");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[1].ref,
           sizeof(spec.defined_schemas[0].fields[1].ref), "array");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[1].ref,
           sizeof(spec.defined_schemas[0].fields[1].ref), "array");
#else
  strcpy(spec.defined_schemas[0].fields[1].ref, "array");
#endif
#endif

/* Field 2: array of enum */
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[2].name,
           sizeof(spec.defined_schemas[0].fields[2].name), "arr_enum");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[2].name,
           sizeof(spec.defined_schemas[0].fields[2].name), "arr_enum");
#else
  strcpy(spec.defined_schemas[0].fields[2].name, "arr_enum");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[2].type,
           sizeof(spec.defined_schemas[0].fields[2].type), "array");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[2].type,
           sizeof(spec.defined_schemas[0].fields[2].type), "array");
#else
  strcpy(spec.defined_schemas[0].fields[2].type, "array");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[2].ref,
           sizeof(spec.defined_schemas[0].fields[2].ref), "enum");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[2].ref,
           sizeof(spec.defined_schemas[0].fields[2].ref), "enum");
#else
  strcpy(spec.defined_schemas[0].fields[2].ref, "enum");
#endif
#endif

/* Field 3: object */
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[3].name,
           sizeof(spec.defined_schemas[0].fields[3].name), "obj_field");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[3].name,
           sizeof(spec.defined_schemas[0].fields[3].name), "obj_field");
#else
  strcpy(spec.defined_schemas[0].fields[3].name, "obj_field");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[3].type,
           sizeof(spec.defined_schemas[0].fields[3].type), "object");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[3].type,
           sizeof(spec.defined_schemas[0].fields[3].type), "object");
#else
  strcpy(spec.defined_schemas[0].fields[3].type, "object");
#endif
#endif

/* Field 4: array with empty ref */
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[4].name,
           sizeof(spec.defined_schemas[0].fields[4].name), "arr_str");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[4].name,
           sizeof(spec.defined_schemas[0].fields[4].name), "arr_str");
#else
  strcpy(spec.defined_schemas[0].fields[4].name, "arr_str");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[4].type,
           sizeof(spec.defined_schemas[0].fields[4].type), "array");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[4].type,
           sizeof(spec.defined_schemas[0].fields[4].type), "array");
#else
  strcpy(spec.defined_schemas[0].fields[4].type, "array");
#endif
#endif
  spec.defined_schemas[0].fields[4].ref[0] = '\0';

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type =
      cdd_test_mut_str("application/x-www-form-urlencoded");
  op.req_body_media_types[0].name =
      cdd_test_mut_str("application/x-www-form-urlencoded");
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  if (spec.defined_schema_names && spec.defined_schema_names[0])
    free(spec.defined_schema_names[0]);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the client_body_responses_suite test suite.
 */
SUITE(client_body_responses_suite) {
  RUN_TEST(test_body_response_range_success);
  RUN_TEST(test_body_default_response_success);
  RUN_TEST(test_body_text_plain_response_string);
  RUN_TEST(test_body_text_plain_response_range);
  RUN_TEST(test_body_text_plain_response_default);
  RUN_TEST(test_body_textual_response_xml);
  RUN_TEST(test_body_binary_response_pdf);
  RUN_TEST(test_client_body_verb_mapping);
  RUN_TEST(test_client_body_mapped_err_code);
  RUN_TEST(test_client_body_media_type_matching);
  RUN_TEST(test_client_body_find_media_type_not_found);
  RUN_TEST(test_client_body_find_encoding_not_found);
  RUN_TEST(test_client_body_array_items_statics);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_RESPONSES_H */
