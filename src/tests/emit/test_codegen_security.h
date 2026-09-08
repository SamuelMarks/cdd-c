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
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openapi/parse/openapi.h"
#include "routes/emit/security.h"
/* clang-format on */

extern cdd_c_error_t uri_has_scheme_prefix_test(const char *uri, size_t len);
extern cdd_c_error_t ref_base_matches_self_uri_test(const char *self_uri,
                                                    const char *ref,
                                                    size_t base_len);
extern cdd_c_error_t
scheme_ref_matches_name_test(const char *req_scheme, const char *scheme_name,
                             const struct OpenAPI_Spec *spec);
extern cdd_c_error_t
scheme_in_security_sets_test(const struct OpenAPI_SecurityRequirementSet *sets,
                             size_t n_sets, const char *scheme_name,
                             const struct OpenAPI_Spec *spec);
extern cdd_c_error_t resolve_active_security_test(
    const struct OpenAPI_Operation *op, const struct OpenAPI_Spec *spec,
    const struct OpenAPI_SecurityRequirementSet **out_sets, size_t *out_count,
    int *out_set_flag);
extern cdd_c_error_t
scheme_is_active_test(const struct OpenAPI_SecurityScheme *sch,
                      const struct OpenAPI_SecurityRequirementSet *sets,
                      size_t n_sets, int security_set,
                      const struct OpenAPI_Spec *spec);

/* Helper to capture output */
static cdd_c_error_t gen_sec_code(const struct OpenAPI_Spec *spec,
                                  const struct OpenAPI_Operation *op_in,
                                  char **_out_val) {
  FILE *tmp = TMPFILE();
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
    if (tmp)
      fclose(tmp);
    {
      *_out_val = NULL;
      return 0;
    }
  }

  fseek(tmp, 0, SEEK_END);
  sz = FTELL(tmp);
  rewind(tmp);

  content = (char *)(size_t)C_CDD_CALLOC(1, (size_t)sz + 1);
  if (sz > 0)
    FREAD(content, 1, (size_t)sz, tmp);

  if (tmp)
    fclose(tmp);
  {
    *_out_val = content;
    return 0;
  }
}

TEST test_sec_bearer_token(void) {
  char *_astgen_sec_code_0 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = (char *)(size_t)(size_t) "bearerAuth";
  sch.type = OA_SEC_HTTP;
  sch.scheme = (char *)(size_t)(size_t) "bearer";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_0), _astgen_sec_code_0);
  ASSERT(code);

  /* Check context check */
  ASSERT(strstr(code, "if (bearer_token) {"));
  /* Check helper call */
  ASSERT(strstr(code,
                "http_request_set_auth_bearer(&req, NULL /* bearer_token */)"));
  /* Check error handling */
  ASSERT(strstr(code, "if (rc != CDD_C_SUCCESS) goto cleanup;"));

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_oauth2_bearer_token(void) {
  char *_astgen_sec_code_1 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = (char *)(size_t)(size_t) "oauth2";
  sch.type = OA_SEC_OAUTH2;

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_1), _astgen_sec_code_1);
  ASSERT(code);

  ASSERT(strstr(code, "if (bearer_token) {"));
  ASSERT(strstr(code,
                "http_request_set_auth_bearer(&req, NULL /* bearer_token */)"));

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_openid_bearer_token(void) {
  char *_astgen_sec_code_2 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = (char *)(size_t)(size_t) "openid";
  sch.type = OA_SEC_OPENID;

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_2), _astgen_sec_code_2);
  ASSERT(code);

  ASSERT(strstr(code, "if (bearer_token) {"));
  ASSERT(strstr(code,
                "http_request_set_auth_bearer(&req, NULL /* bearer_token */)"));

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_basic_token(void) {
  char *_astgen_sec_code_3 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = (char *)(size_t)(size_t) "basicAuth";
  sch.type = OA_SEC_HTTP;
  sch.scheme = (char *)(size_t)(size_t) "basic";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_3), _astgen_sec_code_3);
  ASSERT(code);

  ASSERT(strstr(code, "if (0 /* basic_token */) {"));
  ASSERT(strstr(code,
                "http_request_set_auth_basic(&req, NULL /* basic_token */)"));

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_api_key_header(void) {
  char *_astgen_sec_code_4 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = (char *)(size_t)(size_t) "ApiKeyAuth";
  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_HEADER;
  sch.key_name = (char *)(size_t)(size_t) "X-API-KEY";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_4), _astgen_sec_code_4);
  ASSERT(code);

  /* Check context check using scheme identifier name */
  ASSERT(strstr(code, "if (0 /* api_key_ApiKeyAuth */) {"));
  /* Check injection */
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-API-KEY\", "
                      "NULL /* api_key_ApiKeyAuth */)"));

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_uri_requirement_matches_component(void) {
  char *_astgen_sec_code_5 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  struct OpenAPI_SecurityRequirement req;
  struct OpenAPI_SecurityRequirementSet set;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));
  memset(&req, 0, sizeof(req));
  memset(&set, 0, sizeof(set));

  spec.self_uri = (char *)(size_t)(size_t) "/api/openapi";

  sch.name = (char *)(size_t)(size_t) "ApiKeyAuth";
  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_HEADER;
  sch.key_name = (char *)(size_t)(size_t) "X-API-KEY";

  req.scheme =
      (char *)(size_t)(size_t) "https://example.com/api/openapi#/components/"
                               "securitySchemes/ApiKeyAuth";
  req.scopes = NULL;
  req.n_scopes = 0;

  set.requirements = &req;
  set.n_requirements = 1;

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;
  spec.security = &set;
  spec.n_security = 1;
  spec.security_set = 1;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_5), _astgen_sec_code_5);
  ASSERT(code);

  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-API-KEY\"") != NULL);

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_api_key_query(void) {
  char *_astgen_sec_code_6 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = (char *)(size_t)(size_t) "QueryKey";
  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_QUERY;
  sch.key_name = (char *)(size_t)(size_t) "api_key";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_6), _astgen_sec_code_6);
  ASSERT(code);

  ASSERT(strstr(code, "if (!qp_initialized)") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"api_key\", "
                      "NULL /* api_key_QueryKey */)") != NULL);

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_api_key_cookie(void) {
  char *_astgen_sec_code_7 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme sch;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(&sch, 0, sizeof(sch));

  sch.name = (char *)(size_t)(size_t) "CookieKey";
  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_COOKIE;
  sch.key_name = (char *)(size_t)(size_t) "session_id";

  spec.security_schemes = &sch;
  spec.n_security_schemes = 1;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_7), _astgen_sec_code_7);
  ASSERT(code);

  ASSERT(strstr(code, "cookie_str") != NULL);
  ASSERT(strstr(code, "session_id") != NULL);

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_multiple_schemes(void) {
  char *_astgen_sec_code_8 = NULL;
  /* Test mixing Bearer and API Key */
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme schemes[5];
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(schemes, 0, sizeof(schemes));

  schemes[0].name = (char *)(size_t)(size_t) "bearer";
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = (char *)(size_t)(size_t) "bearer";

  schemes[1].name = (char *)(size_t)(size_t) "key";
  schemes[1].type = OA_SEC_APIKEY;
  schemes[1].in = OA_SEC_IN_HEADER;
  schemes[1].key_name = (char *)(size_t)(size_t) "X-Key";

  spec.security_schemes = schemes;

  schemes[2].type = OA_SEC_APIKEY;
  schemes[2].in = OA_SEC_IN_HEADER;
  schemes[2].name = NULL;

  schemes[3].type = OA_SEC_APIKEY;
  schemes[3].in = OA_SEC_IN_QUERY;
  schemes[3].name = NULL;

  schemes[4].type = OA_SEC_APIKEY;
  schemes[4].in = OA_SEC_IN_COOKIE;
  schemes[4].name = NULL;

  spec.n_security_schemes = 5;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_8), _astgen_sec_code_8);
  ASSERT(code);

  ASSERT(strstr(code, "bearer_token"));
  ASSERT(strstr(code, "api_key_key"));

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_null_safety(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_security_write_apply(NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_security_write_server_apply(NULL, NULL, NULL));
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_server_apply_basic_and_bearer(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme schemes[5];
  struct OpenAPI_Operation op;
  FILE *tmp = TMPFILE();
  char *content = NULL;
  long sz;

  memset(&spec, 0, sizeof(spec));
  memset(schemes, 0, sizeof(schemes));
  memset(&op, 0, sizeof(op));

  schemes[0].name = (char *)(size_t)(size_t) "bearerAuth";
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = (char *)(size_t)(size_t) "bearer";

  schemes[1].name = (char *)(size_t)(size_t) "basicAuth";
  schemes[1].type = OA_SEC_HTTP;
  schemes[1].scheme = (char *)(size_t)(size_t) "basic";

  spec.security_schemes = schemes;

  schemes[2].type = OA_SEC_APIKEY;
  schemes[2].in = OA_SEC_IN_HEADER;
  schemes[2].name = NULL;

  schemes[3].type = OA_SEC_APIKEY;
  schemes[3].in = OA_SEC_IN_QUERY;
  schemes[3].name = NULL;

  schemes[4].type = OA_SEC_APIKEY;
  schemes[4].in = OA_SEC_IN_COOKIE;
  schemes[4].name = NULL;

  spec.n_security_schemes = 5;

  ASSERT(tmp);
  ASSERT_EQ(0, codegen_security_write_server_apply(tmp, &op, &spec));

  fseek(tmp, 0, SEEK_END);
  sz = FTELL(tmp);
  rewind(tmp);
  content = (char *)(size_t)C_CDD_CALLOC(1, (size_t)sz + 1);
  FREAD(content, 1, (size_t)sz, tmp);

  ASSERT(strstr(content, "Validate Bearer Token / OAuth2"));
  ASSERT(strstr(content, "c_rest_middleware_bearer_auth"));
  ASSERT(strstr(content, "Validate Basic Auth"));
  ASSERT(strstr(content, "c_rest_middleware_basic_auth"));

  free(content);
  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sec_security_requirements_filter(void) {
  char *_astgen_sec_code_9 = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme schemes[5];
  struct OpenAPI_SecurityRequirementSet set;
  struct OpenAPI_SecurityRequirement req;
  char *code;

  memset(&spec, 0, sizeof(spec));
  memset(schemes, 0, sizeof(schemes));
  memset(&set, 0, sizeof(set));
  memset(&req, 0, sizeof(req));

  schemes[0].name = (char *)(size_t)(size_t) "bearerAuth";
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = (char *)(size_t)(size_t) "bearer";

  schemes[1].name = (char *)(size_t)(size_t) "ApiKeyAuth";
  schemes[1].type = OA_SEC_APIKEY;
  schemes[1].in = OA_SEC_IN_HEADER;
  schemes[1].key_name = (char *)(size_t)(size_t) "X-API-KEY";

  spec.security_schemes = schemes;

  schemes[2].type = OA_SEC_APIKEY;
  schemes[2].in = OA_SEC_IN_HEADER;
  schemes[2].name = NULL;

  schemes[3].type = OA_SEC_APIKEY;
  schemes[3].in = OA_SEC_IN_QUERY;
  schemes[3].name = NULL;

  schemes[4].type = OA_SEC_APIKEY;
  schemes[4].in = OA_SEC_IN_COOKIE;
  schemes[4].name = NULL;

  spec.n_security_schemes = 5;

  req.scheme = (char *)(size_t)(size_t) "ApiKeyAuth";
  set.requirements = &req;
  set.n_requirements = 1;
  spec.security = &set;
  spec.n_security = 1;
  spec.security_set = 1;

  code = (gen_sec_code(&spec, NULL, &_astgen_sec_code_9), _astgen_sec_code_9);
  ASSERT(code);
  ASSERT(strstr(code, "api_key_ApiKeyAuth"));
  ASSERT(!strstr(code, "bearer_token"));

  C_CDD_FREE(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_uri_has_scheme_prefix(void) {
  ASSERT_EQ(CDD_C_SUCCESS, uri_has_scheme_prefix_test(NULL, 1));
  ASSERT_EQ(CDD_C_SUCCESS, uri_has_scheme_prefix_test("abc", 0));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, uri_has_scheme_prefix_test("http:x", 6));
  ASSERT_EQ(CDD_C_SUCCESS, uri_has_scheme_prefix_test("http/x", 6));
  ASSERT_EQ(CDD_C_SUCCESS, uri_has_scheme_prefix_test("http?x", 6));
  ASSERT_EQ(CDD_C_SUCCESS, uri_has_scheme_prefix_test("http#x", 6));
  PASS();
}

TEST test_ref_base_matches_self_uri(void) {
  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test(NULL, "a", 1));
  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test("a", NULL, 1));
  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test("a", "a", 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            ref_base_matches_self_uri_test("./self", "abself", 6));
  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test("././", "abc", 3));

  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test("", "a", 1));
  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test(".a", "a", 1));

  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            ref_base_matches_self_uri_test("abc", "abc", 3));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            ref_base_matches_self_uri_test("abc#def", "abc", 3));

  /* uri_has_scheme_prefix check fails so it falls back to ./ removal */
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            ref_base_matches_self_uri_test("./abc", "abc", 3));
  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test("./", "abc", 3));

  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            ref_base_matches_self_uri_test("abc", "/abc", 4));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            ref_base_matches_self_uri_test("abc", "xyz/abc", 7));
  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test("abc", "abc", 2));
  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test("a", "ba", 2));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, ref_base_matches_self_uri_test("a", "/a", 2));

  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            ref_base_matches_self_uri_test("/self", "ref/self", 8));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            ref_base_matches_self_uri_test("./self", "self", 4));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            ref_base_matches_self_uri_test("./self", "a/self", 6));

  PASS();
}

TEST test_scheme_ref_matches_name(void) {
  struct OpenAPI_Spec spec;
  memset(&spec, 0, sizeof(spec));

  ASSERT_EQ(CDD_C_SUCCESS, scheme_ref_matches_name_test(NULL, "a", &spec));
  ASSERT_EQ(CDD_C_SUCCESS, scheme_ref_matches_name_test("a", NULL, &spec));

  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            scheme_ref_matches_name_test("#/components/securitySchemes/abc",
                                         "abc", &spec));
  ASSERT_EQ(CDD_C_SUCCESS,
            scheme_ref_matches_name_test("#/components/securitySchemes/def",
                                         "abc", &spec));

  ASSERT_EQ(CDD_C_SUCCESS,
            scheme_ref_matches_name_test(
                "http://x#/components/securitySchemes/abc", "abc", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            scheme_ref_matches_name_test(
                "http://x#/components/securitySchemes/abc", "abc", &spec));

  spec.self_uri = (char *)(size_t)(size_t) "http://x";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            scheme_ref_matches_name_test(
                "http://x#/components/securitySchemes/abc", "abc", &spec));

  spec.self_uri = (char *)(size_t)(size_t) "http://y";
  ASSERT_EQ(CDD_C_SUCCESS,
            scheme_ref_matches_name_test(
                "http://x#/components/securitySchemes/abc", "abc", &spec));

  PASS();
}

TEST test_scheme_in_security_sets(void) {
  struct OpenAPI_SecurityRequirement req;
  struct OpenAPI_SecurityRequirementSet set;
  struct OpenAPI_Spec spec;
  memset(&spec, 0, sizeof(spec));
  memset(&req, 0, sizeof(req));
  memset(&set, 0, sizeof(set));

  req.scheme = (char *)(size_t)(size_t) "abc";
  set.requirements = &req;
  set.n_requirements = 1;

  ASSERT_EQ(CDD_C_SUCCESS, scheme_in_security_sets_test(NULL, 1, "abc", &spec));
  ASSERT_EQ(CDD_C_SUCCESS, scheme_in_security_sets_test(&set, 1, NULL, &spec));

  PASS();
}

TEST test_resolve_active_security(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Spec spec;
  const struct OpenAPI_SecurityRequirementSet *out_sets;
  size_t out_count;
  int out_set_flag;

  memset(&op, 0, sizeof(op));
  memset(&spec, 0, sizeof(spec));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            resolve_active_security_test(&op, NULL, &out_sets, &out_count,
                                         &out_set_flag));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            resolve_active_security_test(&op, NULL, NULL, NULL, NULL));

  op.security_set = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            resolve_active_security_test(&op, &spec, &out_sets, &out_count,
                                         &out_set_flag));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            resolve_active_security_test(&op, &spec, NULL, NULL, NULL));

  op.security_set = 0;
  spec.security_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            resolve_active_security_test(&op, &spec, NULL, NULL, NULL));

  PASS();
}

TEST test_scheme_is_active(void) {
  struct OpenAPI_SecurityScheme sch;
  struct OpenAPI_SecurityRequirementSet set;
  struct OpenAPI_Spec spec;
  memset(&sch, 0, sizeof(sch));
  memset(&set, 0, sizeof(set));
  memset(&spec, 0, sizeof(spec));

  ASSERT_EQ(CDD_C_SUCCESS, scheme_is_active_test(NULL, &set, 1, 1, &spec));

  PASS();
}

TEST test_codegen_security_requires_nulls(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityRequirementSet set;
  struct OpenAPI_SecurityScheme sch;
  memset(&op, 0, sizeof(op));
  memset(&spec, 0, sizeof(spec));
  memset(&set, 0, sizeof(set));
  memset(&sch, 0, sizeof(sch));

  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_query(&op, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_cookie(&op, NULL));

  op.security_set = 1;
  op.n_security = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_query(&op, &spec));
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_cookie(&op, &spec));

  op.security_set = 0;
  op.n_security = 1;
  op.security = &set;
  spec.n_security_schemes = 1;
  spec.security_schemes = &sch;
  spec.security_set = 1;
  spec.n_security = 0;

  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_QUERY;
  sch.name = (char *)(size_t)(size_t) "query_key";

  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_query(&op, &spec));

  spec.security_set = 1;
  spec.n_security = 1;
  spec.security = &set;

  /* active set, but req not matching */
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_query(&op, &spec));

  sch.in = OA_SEC_IN_COOKIE;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_cookie(&op, &spec));

  PASS();
}

TEST test_codegen_security_write_apply_nulls(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityRequirementSet set;
  struct OpenAPI_SecurityScheme sch;
  FILE *tmp = TMPFILE();

  memset(&op, 0, sizeof(op));
  memset(&spec, 0, sizeof(spec));
  memset(&set, 0, sizeof(set));
  memset(&sch, 0, sizeof(sch));

  op.security_set = 1;
  op.n_security = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_write_apply(tmp, &op, &spec));

  if (tmp)
    fclose(tmp);
  PASS();
}

TEST test_codegen_security_write_server_apply_nulls(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityRequirementSet set;
  struct OpenAPI_SecurityScheme sch;
  struct OpenAPI_SecurityRequirement req;
  FILE *tmp = TMPFILE();

  memset(&op, 0, sizeof(op));
  memset(&spec, 0, sizeof(spec));
  memset(&set, 0, sizeof(set));
  memset(&sch, 0, sizeof(sch));
  memset(&req, 0, sizeof(req));

  op.security_set = 1;
  op.n_security = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  op.security_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  op.security_set = 1;
  op.n_security = 1;
  op.security = &set;

  set.n_requirements = 1;
  set.requirements = &req;
  req.scheme = (char *)(size_t)(size_t) "some_scheme";

  spec.n_security_schemes = 1;
  spec.security_schemes = &sch;
  sch.name = (char *)(size_t)(size_t) "not_matching";

  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  if (tmp)
    fclose(tmp);
  PASS();
}

TEST test_codegen_security_branches(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme schemes[10];
  struct OpenAPI_SecurityRequirement req;
  struct OpenAPI_SecurityRequirementSet set;
  FILE *tmp = TMPFILE();

  memset(&op, 0, sizeof(op));
  memset(&spec, 0, sizeof(spec));
  memset(schemes, 0, sizeof(schemes));
  memset(&req, 0, sizeof(req));
  memset(&set, 0, sizeof(set));

  /* 1. Line 61: strncmp != 0 when base_len >= self_len */
  ASSERT_EQ(CDD_C_SUCCESS, ref_base_matches_self_uri_test(
                               "self_longer_str", "different_prefix", 16));

  /* 2. Line 96: strncmp != 0 with hash present */
  ASSERT_EQ(CDD_C_SUCCESS, scheme_ref_matches_name_test(
                               "file.json#not_components", "foo", &spec));

  /* 3. Line 148: op == NULL in resolve_active_security */
  spec.security_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            resolve_active_security_test(NULL, &spec, NULL, NULL, NULL));

  /* 4. Line 206 & 236: sch->type == OA_SEC_APIKEY but sch->in mismatch, and
   * sch->type != OA_SEC_APIKEY */
  schemes[0].type = OA_SEC_APIKEY;
  schemes[0].in = OA_SEC_IN_HEADER;
  schemes[0].name = (char *)(size_t) "hdrKey";
  spec.security_schemes = schemes;
  spec.n_security_schemes = 1;
  /* Make scheme active by matching requirement or no security_set */
  op.security_set = 0;
  spec.security_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_query(&op, &spec));
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_cookie(&op, &spec));

  /* Non-APIKEY scheme (e.g. OA_SEC_HTTP) */
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].name = (char *)(size_t) "httpSch";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_query(&op, &spec));
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_requires_cookie(&op, &spec));

  /* 5. Line 257 & 387: null checks on fp, op, spec */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_security_write_apply(tmp, NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_security_write_apply(tmp, &op, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_security_write_server_apply(tmp, NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_security_write_server_apply(tmp, &op, NULL));

  /* 6. Line 268 & 397: security_schemes != NULL but n_security_schemes == 0,
   * AND security_schemes == NULL with n_security_schemes > 0 */
  spec.security_schemes = schemes;
  spec.n_security_schemes = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_write_apply(tmp, &op, &spec));
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  spec.security_schemes = NULL;
  spec.n_security_schemes = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_write_apply(tmp, &op, &spec));
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  /* 7. Lines 279, 291, 304, 314, 315, 332, 333, 367, 406, 408, 416:
     Incomplete schemes (name set but key_name NULL, http with scheme NULL or
     other, etc.) */
  /* Scheme 0: APIKEY HEADER with name but key_name NULL */
  schemes[0].type = OA_SEC_APIKEY;
  schemes[0].in = OA_SEC_IN_HEADER;
  schemes[0].name = (char *)(size_t) "hdr";
  schemes[0].key_name = NULL;

  /* Scheme 1: HTTP with scheme NULL */
  schemes[1].type = OA_SEC_HTTP;
  schemes[1].name = (char *)(size_t) "http_no_scheme";
  schemes[1].scheme = NULL;

  /* Scheme 2: HTTP with scheme other than bearer or basic */
  schemes[2].type = OA_SEC_HTTP;
  schemes[2].name = (char *)(size_t) "http_digest";
  schemes[2].scheme = (char *)(size_t) "digest";

  /* Scheme 3: APIKEY QUERY with name but key_name NULL */
  schemes[3].type = OA_SEC_APIKEY;
  schemes[3].in = OA_SEC_IN_QUERY;
  schemes[3].name = (char *)(size_t) "qry";
  schemes[3].key_name = NULL;

  /* Scheme 4: APIKEY COOKIE with name but key_name NULL */
  schemes[4].type = OA_SEC_APIKEY;
  schemes[4].in = OA_SEC_IN_COOKIE;
  schemes[4].name = (char *)(size_t) "ck";
  schemes[4].key_name = NULL;

  /* Scheme 5: MutualTLS (type other than any handled) */
  schemes[5].type = OA_SEC_MUTUALTLS;
  schemes[5].name = (char *)(size_t) "mtls";

  /* Scheme 6: OpenID connect */
  schemes[6].type = OA_SEC_OPENID;
  schemes[6].name = (char *)(size_t) "oidc";

  /* Scheme 7: APIKEY with unknown in */
  schemes[7].type = OA_SEC_APIKEY;
  schemes[7].in = OA_SEC_IN_UNKNOWN;
  schemes[7].name = (char *)(size_t) "unk_key";

  spec.n_security_schemes = 8;
  spec.security_schemes = schemes;

  /* Explicitly make them active via requirement sets */
  req.scheme = (char *)(size_t) "hdr";
  set.requirements = &req;
  set.n_requirements = 1;
  op.security = &set;
  op.n_security = 1;
  op.security_set = 1;

  /* Test with security_set = 0 so all schemes are unconditionally active! */
  op.security_set = 0;
  spec.security_set = 0;

  /* Write apply with these incomplete schemes - covers key_name == NULL, scheme
   * == NULL, digest, etc. */
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_write_apply(tmp, &op, &spec));
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  /* Now test has_security == 0 by having only unhandled schemes */
  spec.n_security_schemes = 1;
  schemes[0].type = OA_SEC_MUTUALTLS;
  schemes[0].name = (char *)(size_t) "mtls_only";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_write_apply(tmp, &op, &spec));

  /* Test single scheme cases for exact branches */
  /* HTTP without scheme in server apply */
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = NULL;
  schemes[0].name = (char *)(size_t) "h1";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  /* HTTP with unknown scheme (digest) in server apply */
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = (char *)(size_t) "digest";
  schemes[0].name = (char *)(size_t) "h2";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  /* HTTP with basic in server apply */
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = (char *)(size_t) "basic";
  schemes[0].name = (char *)(size_t) "h3";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  /* HTTP with bearer in server apply */
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = (char *)(size_t) "bearer";
  schemes[0].name = (char *)(size_t) "h4";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  /* OAuth2 in server apply */
  schemes[0].type = OA_SEC_OAUTH2;
  schemes[0].name = (char *)(size_t) "h5";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  /* OpenID in server apply */
  schemes[0].type = OA_SEC_OPENID;
  schemes[0].name = (char *)(size_t) "h6";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_security_write_server_apply(tmp, &op, &spec));

  /* APIKey Query with name and key_name in write_apply */
  schemes[0].type = OA_SEC_APIKEY;
  schemes[0].in = OA_SEC_IN_QUERY;
  schemes[0].name = (char *)(size_t) "q1";
  schemes[0].key_name = (char *)(size_t) "key1";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_write_apply(tmp, &op, &spec));

  /* APIKey Cookie with name and key_name in write_apply */
  schemes[0].type = OA_SEC_APIKEY;
  schemes[0].in = OA_SEC_IN_COOKIE;
  schemes[0].name = (char *)(size_t) "c1";
  schemes[0].key_name = (char *)(size_t) "ck1";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_write_apply(tmp, &op, &spec));

  /* APIKey Header with name and key_name in write_apply */
  schemes[0].type = OA_SEC_APIKEY;
  schemes[0].in = OA_SEC_IN_HEADER;
  schemes[0].name = (char *)(size_t) "hd1";
  schemes[0].key_name = (char *)(size_t) "X-Hdr";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_write_apply(tmp, &op, &spec));

  /* HTTP Basic with name, scheme="basic" in write_apply */
  schemes[0].type = OA_SEC_HTTP;
  schemes[0].scheme = (char *)(size_t) "basic";
  schemes[0].name = (char *)(size_t) "b1";
  spec.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_security_write_apply(tmp, &op, &spec));

  if (tmp)
    fclose(tmp);
  PASS();
}

TEST test_security_errors(void) {
  char *_out = NULL;
  struct OpenAPI_Spec spec;
  memset(&spec, 0, sizeof(spec));

  g_io_calls = 0;
  g_fail_io_after = 1; /* TMPFILE fails */
  ASSERT_EQ(0, gen_sec_code(&spec, NULL, &_out));
  if (_out) {
    C_CDD_FREE(_out);
    _out = NULL;
  }

  g_io_calls = 0;
  g_fail_io_after = 2; /* codegen_security_write_apply fails */
  ASSERT_EQ(0, gen_sec_code(&spec, NULL, &_out));
  if (_out) {
    C_CDD_FREE(_out);
    _out = NULL;
  }

  g_fail_io_after = 999; /* FTELL returns 0 */
  ASSERT_EQ(0, gen_sec_code(&spec, NULL, &_out));
  ASSERT_NEQ(NULL, _out);
  ASSERT_EQ(0, strlen(_out));
  C_CDD_FREE(_out);

  g_fail_io_after = 998; /* FREAD fails */
  ASSERT_EQ(0, gen_sec_code(&spec, NULL, &_out));
  ASSERT_NEQ(NULL, _out);
  C_CDD_FREE(_out);

  g_fail_io_after = -1;
  PASS();
}

SUITE(codegen_security_suite) {
  RUN_TEST(test_codegen_security_branches);
  RUN_TEST(test_security_errors);
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
  RUN_TEST(test_uri_has_scheme_prefix);
  RUN_TEST(test_ref_base_matches_self_uri);
  RUN_TEST(test_scheme_ref_matches_name);
  RUN_TEST(test_scheme_in_security_sets);
  RUN_TEST(test_resolve_active_security);
  RUN_TEST(test_scheme_is_active);
  RUN_TEST(test_codegen_security_requires_nulls);
  RUN_TEST(test_codegen_security_write_apply_nulls);
  RUN_TEST(test_codegen_security_write_server_apply_nulls);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_SECURITY_H */
