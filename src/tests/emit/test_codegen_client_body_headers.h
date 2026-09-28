/**
 * @file test_codegen_client_body_headers.h
 * @brief Unit tests for client body header, cookie, and security params.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_HEADERS_H
#define TEST_CODEGEN_CLIENT_BODY_HEADERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_body_common.h"
/* clang-format on */

TEST test_body_header_array_param(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_14 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "X-Ids";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t)(size_t) "array";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "integer";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_14), _ast_gen_body_14);
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Ids") != NULL);
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-Ids\", joined)") !=
         NULL);
  ASSERT(strstr(code, "joined_len") != NULL);

  free(code);

  /* Test string array */
  param.items_type = (char *)(size_t)(size_t) "string";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_14), _ast_gen_body_14);
  ASSERT(code);
  free(code);

  /* Test number array */
  param.items_type = (char *)(size_t)(size_t) "number";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_14), _ast_gen_body_14);
  ASSERT(code);
  free(code);

  /* Test boolean array */
  param.items_type = (char *)(size_t)(size_t) "boolean";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_14), _ast_gen_body_14);
  ASSERT(code);
  free(code);

  /* Test unsupported array */
  param.items_type = (char *)(size_t)(size_t) "unsupported";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_14), _ast_gen_body_14);
  ASSERT(code);
  free(code);

  g_fail_io_after = -1;
  PASS();
}

TEST test_body_header_object_param(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_15 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "X-Filter";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t)(size_t) "object";
  param.style = OA_STYLE_SIMPLE;
  param.explode = 1;
  param.explode_set = 1;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_15), _ast_gen_body_15);
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Filter") != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *kv = &X-Filter[i]") != NULL);
  ASSERT(strstr(code, "joined[joined_len++] = '='") != NULL);
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-Filter\", joined)") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_header_json_param_ref(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_16 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "X-Filter";
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.schema.ref_name = (char *)(size_t)(size_t) "Filter";
  param.type = (char *)(size_t)(size_t) "Filter";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Filter") != NULL);
  ASSERT(strstr(code, "Filter_to_json") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"X-Filter\", hdr_json)") !=
      NULL);

  free(code);

  /* Test primitive arrays */
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "string";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "integer";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "number";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "boolean";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "unsupported";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  /* Test JSON object */
  param.is_array = 0;
  param.type = (char *)(size_t)(size_t) "object";
  param.schema.ref_name = NULL;
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  /* Test JSON array (no is_array flag) */
  param.type = (char *)(size_t)(size_t) "array";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  /* Test JSON primitive string */
  param.type = (char *)(size_t)(size_t) "string";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  /* Test JSON primitive integer */
  param.type = (char *)(size_t)(size_t) "integer";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  /* Test JSON primitive number */
  param.type = (char *)(size_t)(size_t) "number";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  /* Test JSON primitive boolean */
  param.type = (char *)(size_t)(size_t) "boolean";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  /* Test JSON primitive unsupported */
  param.type = (char *)(size_t)(size_t) "unsupported";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  /* Test JSON missing type */
  param.type = NULL;
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  /* Test JSON unknown type */
  param.type = (char *)(size_t)(size_t) "unknown";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  free(code);

  g_fail_io_after = -1;
  PASS();
}

TEST test_body_header_number_param(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_17 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "X-Rate";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t)(size_t) "number";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_17), _ast_gen_body_17);
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Rate") != NULL);
  ASSERT(strstr(code, "spr"
                      "intf(num_buf, \"%g\", X-Rate)") != NULL);
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-Rate\", num_buf)") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_18 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "session";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = (char *)(size_t)(size_t) "string";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_18), _ast_gen_body_18);
  ASSERT(code);
  ASSERT(strstr(code, "Cookie Parameters") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);
  free(code);

  /* Test primitive arrays */
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "string";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_18), _ast_gen_body_18);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "integer";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_18), _ast_gen_body_18);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "number";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_18), _ast_gen_body_18);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "boolean";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_18), _ast_gen_body_18);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "unsupported";
  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_18), _ast_gen_body_18);
  ASSERT(code);
  free(code);

  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param_number_array(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_19 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "weights";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = (char *)(size_t)(size_t) "array";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "number";
  param.explode = 1;
  param.explode_set = 1;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_19), _ast_gen_body_19);
  ASSERT(code);
  ASSERT(strstr(code, "Cookie Parameters") != NULL);
  ASSERT(strstr(code, "spr"
                      "intf(num_buf, \"%g\", weights[i])") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param_array_explode_false(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_20 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "session";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = (char *)(size_t)(size_t) "array";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "string";
  param.explode_set = 1;
  param.explode = 0;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_20), _ast_gen_body_20);
  ASSERT(code);
  ASSERT(strstr(code, "joined_len") != NULL);
  ASSERT(strstr(code, "joined[joined_len++] = ','") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param_object_form(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_21 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "prefs";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = (char *)(size_t)(size_t) "object";
  param.style = OA_STYLE_FORM;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_21), _ast_gen_body_21);
  ASSERT(code);
  ASSERT(strstr(code, "const struct OpenAPI_KV *kv = &prefs[i]") != NULL);
  ASSERT(strstr(code, "url_encode(") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param_string_allow_reserved(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_22 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "session";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = (char *)(size_t)(size_t) "string";
  param.style = OA_STYLE_FORM;
  param.allow_reserved = 1;
  param.allow_reserved_set = 1;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_22), _ast_gen_body_22);
  ASSERT(code);
  ASSERT(strstr(code, "url_encode_allow_reserved") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_security_query_api_key(void) {
  struct OpenAPI_SecurityRequirement req;
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_SecurityScheme scheme;
  char *code = NULL;
  char *_ast_gen_body_23 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&spec, 0, sizeof(spec));
  memset(&scheme, 0, sizeof(scheme));
  memset(&req, 0, sizeof(req));
  memset(&sec_set, 0, sizeof(sec_set));

  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  scheme.name = (char *)(size_t)(size_t) "QueryKey";
  scheme.type = OA_SEC_APIKEY;
  scheme.in = OA_SEC_IN_QUERY;
  scheme.key_name = (char *)(size_t)(size_t) "api_key";
  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  /* Add global security requirement to activate the scheme */
  req.scheme = (char *)(size_t)(size_t) "QueryKey";

  sec_set.requirements = &req;
  sec_set.n_requirements = 1;

  spec.security = &sec_set;
  spec.n_security = 1;
  spec.security_set = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_23), _ast_gen_body_23);
  ASSERT(code);
  printf("\n--- SEC QUERY KEY ---\n%s\n--------------------\n", code);
  ASSERT(strstr(code, "struct UrlQueryParams qp") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"api_key\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_security_cookie_api_key(void) {
  struct OpenAPI_SecurityRequirement req;
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_SecurityScheme scheme;
  char *code = NULL;
  char *_ast_gen_body_24 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&spec, 0, sizeof(spec));
  memset(&scheme, 0, sizeof(scheme));

  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  scheme.name = (char *)(size_t)(size_t) "CookieKey";
  scheme.type = OA_SEC_APIKEY;
  scheme.in = OA_SEC_IN_COOKIE;
  scheme.key_name = (char *)(size_t)(size_t) "session_id";
  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  memset(&req, 0, sizeof(req));
  req.scheme = (char *)(size_t)(size_t) "CookieKey";

  memset(&sec_set, 0, sizeof(sec_set));
  sec_set.requirements = &req;
  sec_set.n_requirements = 1;

  spec.security = &sec_set;
  spec.n_security = 1;
  spec.security_set = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_24), _ast_gen_body_24);
  ASSERT(code);
  printf("\n--- SEC COOKIE KEY ---\n%s\n--------------------\n", code);
  ASSERT(strstr(code, "cookie_str") != NULL);
  ASSERT(strstr(code, "session_id") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

SUITE(client_body_headers_suite) {
  RUN_TEST(test_body_header_array_param);
  RUN_TEST(test_body_header_object_param);
  RUN_TEST(test_body_header_json_param_ref);
  RUN_TEST(test_body_header_number_param);
  RUN_TEST(test_body_cookie_param);
  RUN_TEST(test_body_cookie_param_number_array);
  RUN_TEST(test_body_cookie_param_array_explode_false);
  RUN_TEST(test_body_cookie_param_object_form);
  RUN_TEST(test_body_cookie_param_string_allow_reserved);
  RUN_TEST(test_body_security_query_api_key);
  RUN_TEST(test_body_security_cookie_api_key);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_HEADERS_H */
