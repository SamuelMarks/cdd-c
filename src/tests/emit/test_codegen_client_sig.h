/**
 * @file test_codegen_client_sig.h
 * @brief Unit tests for C Client Signature Generation.
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_H
#define TEST_CODEGEN_CLIENT_SIG_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/client_sig.h"
#include "openapi/parse/openapi.h"

static int gen_sig(const struct OpenAPI_Operation *op,
                     const struct CodegenSigConfig *cfg, char * *_out_val) {
  FILE *tmp = tmpfile();
  long sz;
  char *content;

  if (!tmp)
    { *_out_val = NULL; return 0; }

  if (codegen_client_write_signature(tmp, op, cfg) != 0) {
    fclose(tmp);
    { *_out_val = NULL; return 0; }
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  if (sz > 0)
    fread(content, 1, sz, tmp);

  fclose(tmp);
  { *_out_val = content; return 0; }
}

TEST test_sig_simple_get(void) { char * _ast_gen_sig_0; 
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

  /* Verify standard signature including ApiError */
  ASSERT(strstr(code,
                ""
"int get_pet(struct HttpClient *ctx, int id, struct Pet **out, "
                "struct ApiError **api_error) {"));

  free(code);
  PASS();
}

TEST test_sig_verify_apierror(void) { char * _ast_gen_sig_1; 
  struct OpenAPI_Operation op = {0};
  char *code;
  op.operation_id = (char *)"do";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_1), _ast_gen_sig_1);
  ASSERT(code);

  ASSERT(strstr(code, ", struct ApiError **api_error)"));

  free(code);
  PASS();
}

TEST test_sig_grouped(void) { char * _ast_gen_sig_2; 
  struct OpenAPI_Operation op = {0};
  struct CodegenSigConfig cfg = {0};
  char *code;

  op.operation_id = "getById";

  cfg.prefix = "api_";
  cfg.group_name = "Pet";

  code = (gen_sig(&op, &cfg, &_ast_gen_sig_2), _ast_gen_sig_2);
  ASSERT(code);

  /* Expect: Pet_api_getById */
  ASSERT(strstr(code, "int Pet_api_getById(struct HttpClient *ctx"));

  free(code);
  PASS();
}

TEST test_sig_success_range_response(void) { char * _ast_gen_sig_3; 
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
  ASSERT(strstr(code, "struct Pet **out") != NULL);

  free(code);
  PASS();
}

TEST test_sig_default_response_success(void) { char * _ast_gen_sig_4; 
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
  ASSERT(strstr(code, "struct Pet **out") != NULL);

  free(code);
  PASS();
}

TEST test_sig_inline_response_string(void) { char * _ast_gen_sig_5; 
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
  ASSERT(strstr(code, "char **out") != NULL);

  free(code);
  PASS();
}

TEST test_sig_inline_response_array(void) { char * _ast_gen_sig_6; 
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
  ASSERT(strstr(code, "int **out, size_t *out_len") != NULL);

  free(code);
  PASS();
}

TEST test_sig_inline_request_body_string(void) { char * _ast_gen_sig_7; 
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = "postInline";
  op.req_body.content_type = "application/json";
  op.req_body.inline_type = "string";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_7), _ast_gen_sig_7);
  ASSERT(code);
  ASSERT(strstr(code, "const char *req_body") != NULL);

  free(code);
  PASS();
}

TEST test_sig_inline_request_body_array(void) { char * _ast_gen_sig_8; 
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = "postInlineArr";
  op.req_body.content_type = "application/json";
  op.req_body.is_array = 1;
  op.req_body.inline_type = "number";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_8), _ast_gen_sig_8);
  ASSERT(code);
  ASSERT(strstr(code, "const double *body, size_t body_len") != NULL);

  free(code);
  PASS();
}

TEST test_sig_multipart_encoding_headers(void) { char * _ast_gen_sig_9; 
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
  ASSERT(strstr(code, "const char *file_hdr_X_Trace") != NULL);
  ASSERT(strstr(code, "const int *file_hdr_X_Ids, size_t file_hdr_X_Ids_len") !=
         NULL);
  ASSERT(strstr(code, "file_hdr_Content_Type") == NULL);

  free(code);
  PASS();
}

TEST test_sig_text_plain_request_body(void) { char * _ast_gen_sig_10; 
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = "postText";
  op.req_body.content_type = "text/plain";
  op.req_body.inline_type = "string";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_10), _ast_gen_sig_10);
  ASSERT(code);
  ASSERT(strstr(code, "const char *req_body") != NULL);

  free(code);
  PASS();
}

TEST test_sig_textual_request_body_xml(void) { char * _ast_gen_sig_11; 
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = "postXml";
  op.req_body.content_type = "application/xml";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_11), _ast_gen_sig_11);
  ASSERT(code);
  ASSERT(strstr(code, "const char *req_body") != NULL);

  free(code);
  PASS();
}

TEST test_sig_octet_stream_request_body(void) { char * _ast_gen_sig_12; 
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = "postBinary";
  op.req_body.content_type = "application/octet-stream";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_12), _ast_gen_sig_12);
  ASSERT(code);
  ASSERT(strstr(code, "const unsigned char *body, size_t body_len") != NULL);

  free(code);
  PASS();
}

TEST test_sig_binary_request_body_pdf(void) { char * _ast_gen_sig_13; 
  struct OpenAPI_Operation op = {0};
  char *code;

  op.operation_id = "postPdf";
  op.req_body.content_type = "application/pdf";

  code = (gen_sig(&op, NULL, &_ast_gen_sig_13), _ast_gen_sig_13);
  ASSERT(code);
  ASSERT(strstr(code, "const unsigned char *body, size_t body_len") != NULL);

  free(code);
  PASS();
}

TEST test_sig_octet_stream_response_body(void) { char * _ast_gen_sig_14; 
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
  ASSERT(strstr(code, "unsigned char **out, size_t *out_len") != NULL);

  free(code);
  PASS();
}

TEST test_sig_binary_response_body_pdf(void) { char * _ast_gen_sig_15; 
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
  ASSERT(strstr(code, "unsigned char **out, size_t *out_len") != NULL);

  free(code);
  PASS();
}

TEST test_sig_querystring_form_object(void) { char * _ast_gen_sig_16; 
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
  ASSERT(strstr(code,
                ""
"int search(struct HttpClient *ctx, const struct OpenAPI_KV "
                "*qs, size_t qs_len, struct ApiError **api_error) {") != NULL);

  free(code);
  PASS();
}

TEST test_sig_querystring_json_ref(void) { char * _ast_gen_sig_17; 
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
  ASSERT(strstr(code,
                ""
"int searchJson(struct HttpClient *ctx, const struct Pet *qs, "
                "struct ApiError **api_error) {") != NULL);

  free(code);
  PASS();
}

TEST test_sig_querystring_json_primitive(void) { char * _ast_gen_sig_18; 
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
  ASSERT(strstr(code, ""
"int searchJsonInt(struct HttpClient *ctx, int qs, "
                      "struct ApiError **api_error) {") != NULL);

  free(code);
  PASS();
}

TEST test_sig_querystring_json_array(void) { char * _ast_gen_sig_19; 
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
  ASSERT(strstr(code,
                ""
"int searchJsonTags(struct HttpClient *ctx, const char **qs, "
                "size_t qs_len, struct ApiError **api_error) {") != NULL);

  free(code);
  PASS();
}

TEST test_sig_querystring_json_array_object(void) { char * _ast_gen_sig_20; 
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
  ASSERT(strstr(code,
                ""
"int searchJsonPets(struct HttpClient *ctx, const struct Pet "
                "**qs, size_t qs_len, struct ApiError **api_error) {") != NULL);

  free(code);
  PASS();
}

TEST test_sig_querystring_raw_string(void) { char * _ast_gen_sig_21; 
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
  ASSERT(strstr(code, ""
"int searchRaw(struct HttpClient *ctx, const char *qs, "
                      "struct ApiError **api_error) {") != NULL);

  free(code);
  PASS();
}

TEST test_sig_querystring_raw_integer(void) { char * _ast_gen_sig_22; 
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
  ASSERT(strstr(code, ""
"int searchRawInt(struct HttpClient *ctx, int qs, "
                      "struct ApiError **api_error) {") != NULL);

  free(code);
  PASS();
}

TEST test_sig_query_object_param_kv(void) { char * _ast_gen_sig_23; 
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
  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  PASS();
}

TEST test_sig_path_object_param_kv(void) { char * _ast_gen_sig_24; 
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
  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  PASS();
}

TEST test_sig_header_object_param_kv(void) { char * _ast_gen_sig_25; 
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
  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  PASS();
}

TEST test_sig_cookie_object_param_kv(void) { char * _ast_gen_sig_26; 
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
  ASSERT(strstr(code, "const struct OpenAPI_KV *prefs, size_t prefs_len") !=
         NULL);

  free(code);
  PASS();
}

TEST test_sig_json_content_query_ref(void) { char * _ast_gen_sig_27; 
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
  ASSERT(strstr(code, "const struct Filter *filter") != NULL);

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
}

#endif /* TEST_CODEGEN_CLIENT_SIG_H */
