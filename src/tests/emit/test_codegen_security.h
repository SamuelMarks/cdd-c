extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_codegen_security.h
 * @brief Unit tests for Security Code Generator.
 */

#ifndef TEST_CODEGEN_SECURITY_H
#define TEST_CODEGEN_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openapi/parse/openapi.h"
#include "routes/emit/security.h"
/* clang-format on */

/* Helper to capture output */
static int gen_sec_code(const struct OpenAPI_Spec *spec,
                        const struct OpenAPI_Operation *op_in,
                        char **_out_val) {
  FILE *tmp = tmpfile();
  struct OpenAPI_Operation op_local;
  const struct OpenAPI_Operation *op = op_in;
  long sz;
  char *content = NULL;

  if (!tmp) {
    *_out_val = NULL;
    return 0;
  }

  /* Op is unused currently but required by signature */
  if (!op) {
    memset(&op_local, 0, sizeof(op_local));
    op = &op_local;
  }
  if (codegen_security_write_apply(tmp, op, spec) != 0) {
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
    fread(content, 1, sz, tmp);

  fclose(tmp);
  {
    *_out_val = content;
    return 0;
  }
}

TEST test_sec_bearer_token(void) {
  char *_ast_gen_sec_code_0 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = "bearerAuth";
  sch.type = OA_SEC_HTTP;
  sch.scheme = "bearer";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_0), _ast_gen_sec_code_0);
  ASSERT(code);

  /* Check context check */
  ASSERT(strstr(code, "if (0 /* bearer_token */) {"));
  /* Check helper call */
  ASSERT(strstr(code,
                "http_request_set_auth_bearer(&req, NULL /* bearer_token */)"));
  /* Check error handling */
  ASSERT(strstr(code, "if (rc != 0) goto cleanup;"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_oauth2_bearer_token(void) {
  char *_ast_gen_sec_code_1 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = "oauth2";
  sch.type = OA_SEC_OAUTH2;

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_1), _ast_gen_sec_code_1);
  ASSERT(code);

  ASSERT(strstr(code, "if (0 /* bearer_token */) {"));
  ASSERT(strstr(code,
                "http_request_set_auth_bearer(&req, NULL /* bearer_token */)"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_openid_bearer_token(void) {
  char *_ast_gen_sec_code_2 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = "openid";
  sch.type = OA_SEC_OPENID;

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_2), _ast_gen_sec_code_2);
  ASSERT(code);

  ASSERT(strstr(code, "if (0 /* bearer_token */) {"));
  ASSERT(strstr(code,
                "http_request_set_auth_bearer(&req, NULL /* bearer_token */)"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_basic_token(void) {
  char *_ast_gen_sec_code_3 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = "basicAuth";
  sch.type = OA_SEC_HTTP;
  sch.scheme = "basic";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_3), _ast_gen_sec_code_3);
  ASSERT(code);

  ASSERT(strstr(code, "if (0 /* basic_token */) {"));
  ASSERT(strstr(code,
                "http_request_set_auth_basic(&req, NULL /* basic_token */)"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_api_key_header(void) {
  char *_ast_gen_sec_code_4 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = "ApiKeyAuth";
  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_HEADER;
  sch.key_name = "X-API-KEY";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_4), _ast_gen_sec_code_4);
  ASSERT(code);

  /* Check context check using scheme identifier name */
  ASSERT(strstr(code, "if (0 /* api_key_ApiKeyAuth */) {"));
  /* Check injection */
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-API-KEY\", "
                      "NULL /* api_key_ApiKeyAuth */)"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_uri_requirement_matches_component(void) {
  char *_ast_gen_sec_code_5 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  struct OpenAPI_SecurityRequirement req;
  struct OpenAPI_SecurityRequirementSet set;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));
  memset(&req, 0, sizeof(req));
  memset(&set, 0, sizeof(set));

  spec.self_uri = "/api/openapi";

  sch.name = "ApiKeyAuth";
  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_HEADER;
  sch.key_name = "X-API-KEY";

  req.scheme =
      "https://example.com/api/openapi#/components/securitySchemes/ApiKeyAuth";
  req.scopes = NULL;
  req.n_scopes = 0;

  set.requirements = &req;
  set.n_requirements = 1;

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;
  spec.security = &set;
  spec.n_security = 1;
  spec.security_set = 1;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_5), _ast_gen_sec_code_5);
  ASSERT(code);

  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-API-KEY\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_api_key_query(void) {
  char *_ast_gen_sec_code_6 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = "QueryKey";
  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_QUERY;
  sch.key_name = "api_key";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_6), _ast_gen_sec_code_6);
  ASSERT(code);

  ASSERT(strstr(code, "if (!qp_initialized)") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"api_key\", "
                      "NULL /* api_key_QueryKey */)") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_api_key_cookie(void) {
  char *_ast_gen_sec_code_7 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = "CookieKey";
  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_COOKIE;
  sch.key_name = "session_id";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_7), _ast_gen_sec_code_7);
  ASSERT(code);

  ASSERT(strstr(code, "cookie_str") != NULL);
  ASSERT(strstr(code, "session_id") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_multiple_schemes(void) {
  char *_ast_gen_sec_code_8 = NULL;
  /* Test mixing Bearer and API Key */
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme schemes[2];
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(schemes, 0, sizeof(schemes));

  schemes[0].name = "bearer";
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = "bearer";

  schemes[1].name = "key";
  schemes[1].type = OA_SEC_APIKEY;
  schemes[1].in = OA_SEC_IN_HEADER;
  schemes[1].key_name = "X-Key";

  spec.security_schemes = schemes;
  spec.n_security_schemes = 2;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_8), _ast_gen_sec_code_8);
  ASSERT(code);

  ASSERT(strstr(code, "bearer_token"));
  ASSERT(strstr(code, "api_key_key"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_null_safety(void) {
  ASSERT_EQ(EINVAL, codegen_security_write_apply(NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, codegen_security_write_server_apply(NULL, NULL, NULL));
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_server_apply_basic_and_bearer(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme schemes[2];
  struct OpenAPI_Operation op;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  memset(&spec, 0, sizeof(spec));
  memset(schemes, 0, sizeof(schemes));
  memset(&op, 0, sizeof(op));

  schemes[0].name = "bearerAuth";
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = "bearer";

  schemes[1].name = "basicAuth";
  schemes[1].type = OA_SEC_HTTP;
  schemes[1].scheme = "basic";

  spec.security_schemes = schemes;
  spec.n_security_schemes = 2;

  ASSERT(tmp);
  ASSERT_EQ(0, codegen_security_write_server_apply(tmp, &op, &spec));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "Validate Bearer Token / OAuth2"));
  ASSERT(strstr(content, "c_rest_middleware_bearer_auth"));
  ASSERT(strstr(content, "Validate Basic Auth"));
  ASSERT(strstr(content, "c_rest_middleware_basic_auth"));

  free(content);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_security_requirements_filter(void) {
  char *_ast_gen_sec_code_9 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme schemes[2];
  struct OpenAPI_SecurityRequirementSet set;
  struct OpenAPI_SecurityRequirement req;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(schemes, 0, sizeof(schemes));
  memset(&set, 0, sizeof(set));
  memset(&req, 0, sizeof(req));

  schemes[0].name = "bearerAuth";
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = "bearer";

  schemes[1].name = "ApiKeyAuth";
  schemes[1].type = OA_SEC_APIKEY;
  schemes[1].in = OA_SEC_IN_HEADER;
  schemes[1].key_name = "X-API-KEY";

  spec.security_schemes = schemes;
  spec.n_security_schemes = 2;

  req.scheme = "ApiKeyAuth";
  set.requirements = &req;
  set.n_requirements = 1;
  spec.security = &set;
  spec.n_security = 1;
  spec.security_set = 1;

  code = (gen_sec_code(&spec, NULL, &_ast_gen_sec_code_9), _ast_gen_sec_code_9);
  ASSERT(code);
  ASSERT(strstr(code, "api_key_ApiKeyAuth"));
  ASSERT(!strstr(code, "bearer_token"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

SUITE(codegen_security_suite) {
  RUN_TEST(test_sec_bearer_token);
  RUN_TEST(test_sec_oauth2_bearer_token);
  RUN_TEST(test_sec_openid_bearer_token);
  RUN_TEST(test_sec_basic_token);
  RUN_TEST(test_sec_api_key_header);
  RUN_TEST(test_sec_uri_requirement_matches_component);
  RUN_TEST(test_sec_api_key_query);
  RUN_TEST(test_sec_api_key_cookie);
  RUN_TEST(test_sec_multiple_schemes);
  RUN_TEST(test_sec_security_requirements_filter);
  RUN_TEST(test_sec_server_apply_basic_and_bearer);
  RUN_TEST(test_sec_null_safety);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_SECURITY_H */
