/**
 * @file test_codegen_security.h
 * @brief Unit tests for Security Code Generator.
 *
 * Verifies that the correct authentication injection logic is emitted
 * for various security schemes (Bearer, API Key).
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_SECURITY_H
#define TEST_CODEGEN_SECURITY_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openapi/parse_openapi.h"
#include "routes/emit_security.h"

/* Helper to capture output */
static char *gen_sec_code(const struct OpenAPI_Spec *spec,
                          const struct OpenAPI_Operation *op_in) {
  FILE *tmp = tmpfile();
  struct OpenAPI_Operation op_local;
  const struct OpenAPI_Operation *op = op_in;
  long sz;
  char *content;

  if (!tmp)
    return NULL;

  /* Op is unused currently but required by signature */
  if (!op) {
    memset(&op_local, 0, sizeof(op_local));
    op = &op_local;
  }
  if (codegen_security_write_apply(tmp, op, spec) != 0) {
    fclose(tmp);
    return NULL;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  if (sz > 0)
    fread(content, 1, sz, tmp);

  fclose(tmp);
  return content;
}

TEST test_sec_bearer_token(void) {
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

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);

  /* Check context check */
  ASSERT(strstr(code, "if (ctx->security.bearer_token) {"));
  /* Check helper call */
  ASSERT(strstr(
      code, "http_request_set_auth_bearer(&req, ctx->security.bearer_token)"));
  /* Check error handling */
  ASSERT(strstr(code, "if (rc != 0) goto cleanup;"));

  free(code);
  PASS();
}

TEST test_sec_oauth2_bearer_token(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = "oauth2";
  sch.type = OA_SEC_OAUTH2;

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);

  ASSERT(strstr(code, "if (ctx->security.bearer_token) {"));
  ASSERT(strstr(
      code, "http_request_set_auth_bearer(&req, ctx->security.bearer_token)"));

  free(code);
  PASS();
}

TEST test_sec_openid_bearer_token(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = "openid";
  sch.type = OA_SEC_OPENID;

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);

  ASSERT(strstr(code, "if (ctx->security.bearer_token) {"));
  ASSERT(strstr(
      code, "http_request_set_auth_bearer(&req, ctx->security.bearer_token)"));

  free(code);
  PASS();
}

TEST test_sec_basic_token(void) {
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

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);

  ASSERT(strstr(code, "if (ctx->security.basic_token) {"));
  ASSERT(strstr(
      code, "http_request_set_auth_basic(&req, ctx->security.basic_token)"));

  free(code);
  PASS();
}

TEST test_sec_api_key_header(void) {
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

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);

  /* Check context check using scheme identifier name */
  ASSERT(strstr(code, "if (ctx->security.api_key_ApiKeyAuth) {"));
  /* Check injection */
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-API-KEY\", "
                      "ctx->security.api_key_ApiKeyAuth)"));

  free(code);
  PASS();
}

TEST test_sec_uri_requirement_matches_component(void) {
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

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);

  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-API-KEY\"") != NULL);

  free(code);
  PASS();
}

TEST test_sec_api_key_query(void) {
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

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);

  ASSERT(strstr(code, "if (!qp_initialized)") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"api_key\", "
                      "ctx->security.api_key_QueryKey)") != NULL);

  free(code);
  PASS();
}

TEST test_sec_api_key_cookie(void) {
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

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);

  ASSERT(strstr(code, "cookie_str") != NULL);
  ASSERT(strstr(code, "session_id") != NULL);

  free(code);
  PASS();
}

TEST test_sec_multiple_schemes(void) {
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

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);

  ASSERT(strstr(code, "bearer_token"));
  ASSERT(strstr(code, "api_key_key"));

  free(code);
  PASS();
}

TEST test_sec_null_safety(void) {
  ASSERT_EQ(EINVAL, codegen_security_write_apply(NULL, NULL, NULL));
  PASS();
}

TEST test_sec_security_requirements_filter(void) {
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

  code = gen_sec_code(&spec, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "api_key_ApiKeyAuth"));
  ASSERT(!strstr(code, "bearer_token"));

  free(code);
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
  RUN_TEST(test_sec_null_safety);
}

#endif /* TEST_CODEGEN_SECURITY_H */
