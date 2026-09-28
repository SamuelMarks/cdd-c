/**
 * @file test_codegen_client_body_forms.h
 * @brief Unit tests for client body form and multipart handling.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_FORMS_H
#define TEST_CODEGEN_CLIENT_BODY_FORMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_body_common.h"
/* clang-format on */

TEST test_body_form_urlencoded(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_25 = NULL;
  char *schema_name = (char *)(size_t)(size_t) "FormData";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "name", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "age", "integer", NULL, NULL,
                    NULL);
  c_cdd_strdup(schema_name, &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "FormData";
  op.req_body.content_type =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded";

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_25),
          _ast_gen_body_25);
  ASSERT(code);
  ASSERT(strstr(code, "Form URL-Encoded Body Construction") != NULL);
  ASSERT(strstr(code, "url_query_build_form(&form_qp, &form_body)") != NULL);
  ASSERT(strstr(code, "\"application/x-www-form-urlencoded\"") != NULL);
  ASSERT(strstr(code, "url_query_add(&form_qp, \"name\"") != NULL);
  ASSERT(strstr(code, "spr"
                      "intf(num_buf, \"%d\", req_body->age)") != NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_with_params(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_26 = NULL;
  char *schema_name = (char *)(size_t)(size_t) "FormData";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "name", "string", NULL, NULL,
                    NULL);
  c_cdd_strdup(schema_name, &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "FormData";
  op.req_body.content_type =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded; "
                               "charset=utf-8";

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_26),
          _ast_gen_body_26);
  ASSERT(code);
  ASSERT(strstr(code, "Form URL-Encoded Body Construction") != NULL);
  ASSERT(strstr(code, "url_query_build_form(&form_qp, &form_body)") != NULL);
  ASSERT(strstr(code, "\"application/x-www-form-urlencoded\"") != NULL);
  ASSERT(strstr(code, "url_query_add(&form_qp, \"name\"") != NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_object_fields(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_27 = NULL;
  char *schema_name = (char *)(size_t)(size_t) "FormData";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "pet", "object", "Pet", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "pets", "array", "Pet", NULL,
                    NULL);
  c_cdd_strdup(schema_name, &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "FormData";
  op.req_body.content_type =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded";

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_27),
          _ast_gen_body_27);
  ASSERT(code);
  ASSERT(strstr(code, "Pet_to_json(req_body->pet") != NULL);
  ASSERT(strstr(code, "Pet_to_json(req_body->pets[i]") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&form_qp, \"pet\"") != NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_object_style_form_explode_true(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_28 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(2, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "filter", "object", "Filter",
                    NULL, NULL);
  c_cdd_strdup("FormData", &spec.defined_schema_names[0]);

  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "color", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "limit", "integer", NULL, NULL,
                    NULL);
  c_cdd_strdup("Filter", &spec.defined_schema_names[1]);

  spec.n_defined_schemas = 2;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "FormData";
  op.req_body.content_type =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded";

  enc.name = (char *)(size_t)(size_t) "filter";
  enc.style = OA_STYLE_FORM;
  enc.style_set = 1;
  enc.explode = 1;
  enc.explode_set = 1;

  mt.name = (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_28),
          _ast_gen_body_28);
  ASSERT(code);
  ASSERT(strstr(code, "url_query_add(&form_qp, \"color\"") != NULL);
  ASSERT(strstr(code, "Filter_to_json") == NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_object_style_form_explode_false(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_29 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(2, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "filter", "object", "Filter",
                    NULL, NULL);
  c_cdd_strdup("FormData", &spec.defined_schema_names[0]);

  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "color", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "limit", "integer", NULL, NULL,
                    NULL);
  c_cdd_strdup("Filter", &spec.defined_schema_names[1]);

  spec.n_defined_schemas = 2;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "FormData";
  op.req_body.content_type =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded";

  enc.name = (char *)(size_t)(size_t) "filter";
  enc.style = OA_STYLE_FORM;
  enc.style_set = 1;
  enc.explode = 0;
  enc.explode_set = 1;

  mt.name = (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_29),
          _ast_gen_body_29);
  ASSERT(code);
  ASSERT(strstr(code, "openapi_kv_join_form") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&form_qp, \"filter\"") != NULL);
  ASSERT(strstr(code, "Filter_to_json") == NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_object_style_deep_object(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_30 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(2, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "filter", "object", "Filter",
                    NULL, NULL);
  c_cdd_strdup("FormData", &spec.defined_schema_names[0]);

  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "color", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "limit", "integer", NULL, NULL,
                    NULL);
  c_cdd_strdup("Filter", &spec.defined_schema_names[1]);

  spec.n_defined_schemas = 2;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "FormData";
  op.req_body.content_type =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded";

  enc.name = (char *)(size_t)(size_t) "filter";
  enc.style = OA_STYLE_DEEP_OBJECT;
  enc.style_set = 1;
  enc.explode = 1;
  enc.explode_set = 1;

  mt.name = (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_30),
          _ast_gen_body_30);
  ASSERT(code);
  ASSERT(strstr(code, "filter[color]") != NULL);
  ASSERT(strstr(code, "Filter_to_json") == NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_primitives_and_arrays(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_31 = NULL;
  char *schema_name = (char *)(size_t)(size_t) "Upload";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "count", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "ratio", "number", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "flag", "boolean", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "tags", "array", "string", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "nums", "array", "integer", NULL,
                    NULL);
  c_cdd_strdup(schema_name, &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "Upload";
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";

  code = (gen_body(&op, &spec, "/upload", NULL, &_ast_gen_body_31),
          _ast_gen_body_31);
  ASSERT(code);
  ASSERT(strstr(code, "Multipart Body Construction") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"title\"") != NULL);
  ASSERT(strstr(code, "spr"
                      "intf(num_buf, \"%g\", req_body->ratio)") != NULL);
  ASSERT(strstr(code, "req_body->flag ? \"true\" : \"false\"") != NULL);
  ASSERT(strstr(code, "for (i = 0; i < req_body->n_tags; ++i)") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"tags\"") != NULL);
  ASSERT(strstr(code, "for (i = 0; i < req_body->n_nums; ++i)") != NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_object_fields(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_32 = NULL;
  char *schema_name = (char *)(size_t)(size_t) "FormData";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "pet", "object", "Pet", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "pets", "array", "Pet", NULL,
                    NULL);
  c_cdd_strdup(schema_name, &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "FormData";
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_32),
          _ast_gen_body_32);
  ASSERT(code);
  ASSERT(strstr(code, "Pet_to_json(req_body->pet") != NULL);
  ASSERT(strstr(code, "Pet_to_json(req_body->pets[i]") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"pet\", NULL, "
                      "\"application/json\"") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"pets\", NULL, "
                      "\"application/json\"") != NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_encoding_content_type(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_33 = NULL;
  char *schema_name = (char *)(size_t)(size_t) "Upload";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                    NULL);
  c_cdd_strdup(schema_name, &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "Upload";
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";

  mt.name = (char *)(size_t)(size_t) "multipart/form-data";
  enc.name = (char *)(size_t)(size_t) "title";
  enc.content_type = (char *)(size_t)(size_t) "text/plain; charset=utf-8";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/upload", NULL, &_ast_gen_body_33),
          _ast_gen_body_33);
  ASSERT(code);
  ASSERT(strstr(code, "Multipart Body Construction") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"title\", NULL, "
                      "\"text/plain; charset=utf-8\"") != NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_encoding_content_type_list(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_34 = NULL;
  char *schema_name = (char *)(size_t)(size_t) "Upload";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "file", "string", NULL, NULL,
                    NULL);
  c_cdd_strdup(schema_name, &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "Upload";
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";

  mt.name = (char *)(size_t)(size_t) "multipart/form-data";
  enc.name = (char *)(size_t)(size_t) "file";
  enc.content_type = (char *)(size_t)(size_t) "image/png, image/jpeg";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/upload", NULL, &_ast_gen_body_34),
          _ast_gen_body_34);
  ASSERT(code);
  ASSERT(strstr(code, "Multipart Body Construction") != NULL);
  ASSERT(strstr(code, "\"image/png\"") != NULL);
  ASSERT(strstr(code, "image/jpeg") == NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_encoding_headers(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header headers[3];
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_35 = NULL;
  char *schema_name = (char *)(size_t)(size_t) "Upload";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(&headers, 0, sizeof(headers));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                    NULL);
  c_cdd_strdup(schema_name, &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = (char *)(size_t)(size_t) "Upload";
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";

  headers[0].name = (char *)(size_t)(size_t) "X-Trace";
  headers[0].type = (char *)(size_t)(size_t) "string";
  headers[1].name = (char *)(size_t)(size_t) "X-Ids";
  headers[1].type = (char *)(size_t)(size_t) "array";
  headers[1].is_array = 1;
  headers[1].items_type = (char *)(size_t)(size_t) "integer";
  headers[2].name = (char *)(size_t)(size_t) "Content-Type";
  headers[2].type = (char *)(size_t)(size_t) "string";

  mt.name = (char *)(size_t)(size_t) "multipart/form-data";
  enc.name = (char *)(size_t)(size_t) "title";
  enc.headers = headers;
  enc.n_headers = 3;
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/upload", NULL, &_ast_gen_body_35),
          _ast_gen_body_35);
  ASSERT(code);
  ASSERT(strstr(code, "http_request_add_part_header_last(&req, \"X-Trace\", "
                      "title_hdr_X_Trace") != NULL);
  ASSERT(strstr(code, "title_hdr_X_Ids_len") != NULL);
  ASSERT(strstr(code, "title_hdr_Content_Type") == NULL);

  free(code);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

SUITE(client_body_forms_suite) {
  RUN_TEST(test_body_form_urlencoded);
  RUN_TEST(test_body_form_urlencoded_with_params);
  RUN_TEST(test_body_form_urlencoded_object_fields);
  RUN_TEST(test_body_form_urlencoded_object_style_form_explode_true);
  RUN_TEST(test_body_form_urlencoded_object_style_form_explode_false);
  RUN_TEST(test_body_form_urlencoded_object_style_deep_object);
  RUN_TEST(test_body_multipart_primitives_and_arrays);
  RUN_TEST(test_body_multipart_object_fields);
  RUN_TEST(test_body_multipart_encoding_content_type);
  RUN_TEST(test_body_multipart_encoding_content_type_list);
  RUN_TEST(test_body_multipart_encoding_headers);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_FORMS_H */
