/**
 * @file test_cli_c2openapi_coverage.h
 * @brief Unit tests for c2openapi 100% full coverage.
 */

#ifndef TEST_CLI_C2OPENAPI_COVERAGE_H
#define TEST_CLI_C2OPENAPI_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_test_helpers/cdd_helpers.h"
#include "c_cdd/memory.h"
#include "routes/parse/cli.h"
#include <greatest.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifndef CDD_TEST_CLI_C2OPENAPI_HELPERS_DEFINED
#define CDD_TEST_CLI_C2OPENAPI_HELPERS_DEFINED
static int g_c2o_oom_fail_at = -1;
static void *mock_c2o_oom_malloc(size_t sz) {
  if (g_c2o_oom_fail_at == 0) {
    g_c2o_oom_fail_at = -1;
    return NULL;
  }
  if (g_c2o_oom_fail_at > 0)
    g_c2o_oom_fail_at--;
  return malloc(sz);
}
static void mock_c2o_oom_free(void *ptr) { free(ptr); }

static const char *get_mocks_dir(void) {
  FILE *f;
  f = fopen("src/tests/mocks/emit/simple.schema.json", "r");
  if (f) {
    fclose(f);
    return "src/tests/mocks";
  }
  return "../src/tests/mocks";
}

static const char *get_simple_schema(void) {
  FILE *f;
  f = fopen("src/tests/mocks/emit/simple.schema.json", "r");
  if (f) {
    fclose(f);
    return "src/tests/mocks/emit/simple.schema.json";
  }
  return "../src/tests/mocks/emit/simple.schema.json";
}
#endif /* CDD_TEST_CLI_C2OPENAPI_HELPERS_DEFINED */

TEST test_c2openapi_full_coverage_100(void) {
  struct OpenAPI_Spec spec;
  cdd_c_error_t rc;
  int is_src = 0;
  int has_tag = 0;
  struct OpenAPI_Tag *tag = NULL;
  enum OpenAPI_SecurityType sec_type;
  enum OpenAPI_SecurityIn sec_in;
  enum OpenAPI_OAuthFlowType flow_type;
  struct OpenAPI_Server srv;
  struct DocServer doc_srv;
  struct DocServerVar doc_vars[2];
  char *enum_vals[2];
  struct OpenAPI_OAuthFlow flow_dst;
  struct DocOAuthFlow doc_flow;
  struct DocOAuthScope doc_scopes[2];
  struct OpenAPI_SecurityScheme scheme;
  struct DocSecurityScheme doc_sec;
  struct DocMetadata meta;
  struct DocSecurityRequirement doc_req;
  char *req_scopes[2];
  struct DocTagMeta doc_tag;
  struct OpenAPI_Operation op;
  char *op_tags[2];
  struct OpenAPI_Path path;
  struct C2OpenAPI_ParsedSig psig;
  char *argv[10];

  rc = openapi_spec_init(&spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 1. is_source_file */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_source_file("file.c", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_source_file(NULL, &is_src));

  /* 2. spec_has_tag with null tag name in spec */
  memset(&spec, 0, sizeof(spec));
  spec.n_tags = 1;
  spec.tags = (struct OpenAPI_Tag *)calloc(1, sizeof(struct OpenAPI_Tag));
  spec.tags[0].name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, spec_has_tag(&spec, "tag", &has_tag));
  ASSERT_EQ(0, has_tag);
  free(spec.tags);
  spec.tags = NULL;
  spec.n_tags = 0;

  /* 3. spec_add_tag NULL and OOM */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_add_tag(NULL, "tag"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_add_tag(&spec, NULL));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_tag(&spec, "tag"));
  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_tag(&spec, "tag"));
  g_cdd_strdup_fail = 0;

  /* 4. spec_find_tag NULL and missing */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_find_tag(NULL, "tag", &tag));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_find_tag(&spec, NULL, &tag));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_find_tag(&spec, "tag", NULL));
  spec.n_tags = 1;
  spec.tags = (struct OpenAPI_Tag *)calloc(1, sizeof(struct OpenAPI_Tag));
  spec.tags[0].name = NULL;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, spec_find_tag(&spec, "tag", &tag));
  free(spec.tags);
  spec.tags = NULL;
  spec.n_tags = 0;

  /* 5. c2openapi_map_doc_security_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c2openapi_map_doc_security_type(DOC_SEC_APIKEY, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            c2openapi_map_doc_security_type(DOC_SEC_APIKEY, &sec_type));
  ASSERT_EQ(OA_SEC_APIKEY, sec_type);
  ASSERT_EQ(CDD_C_SUCCESS,
            c2openapi_map_doc_security_type(DOC_SEC_HTTP, &sec_type));
  ASSERT_EQ(OA_SEC_HTTP, sec_type);
  ASSERT_EQ(CDD_C_SUCCESS,
            c2openapi_map_doc_security_type(DOC_SEC_MUTUALTLS, &sec_type));
  ASSERT_EQ(OA_SEC_MUTUALTLS, sec_type);
  ASSERT_EQ(CDD_C_SUCCESS,
            c2openapi_map_doc_security_type(DOC_SEC_OAUTH2, &sec_type));
  ASSERT_EQ(OA_SEC_OAUTH2, sec_type);
  ASSERT_EQ(CDD_C_SUCCESS,
            c2openapi_map_doc_security_type(DOC_SEC_OPENID, &sec_type));
  ASSERT_EQ(OA_SEC_OPENID, sec_type);
  ASSERT_EQ(CDD_C_SUCCESS,
            c2openapi_map_doc_security_type(DOC_SEC_UNSET, &sec_type));
  ASSERT_EQ(OA_SEC_UNKNOWN, sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_map_doc_security_type(
                               (enum DocSecurityType)999, &sec_type));
  ASSERT_EQ(OA_SEC_UNKNOWN, sec_type);

  /* 6. map_doc_security_in */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            map_doc_security_in(DOC_SEC_IN_QUERY, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, map_doc_security_in(DOC_SEC_IN_QUERY, &sec_in));
  ASSERT_EQ(OA_SEC_IN_QUERY, sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, map_doc_security_in(DOC_SEC_IN_HEADER, &sec_in));
  ASSERT_EQ(OA_SEC_IN_HEADER, sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, map_doc_security_in(DOC_SEC_IN_COOKIE, &sec_in));
  ASSERT_EQ(OA_SEC_IN_COOKIE, sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, map_doc_security_in(DOC_SEC_IN_UNSET, &sec_in));
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sec_in);
  ASSERT_EQ(CDD_C_SUCCESS,
            map_doc_security_in((enum DocSecurityIn)999, &sec_in));

  /* 7. map_doc_flow_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            map_doc_flow_type(DOC_OAUTH_FLOW_IMPLICIT, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            map_doc_flow_type(DOC_OAUTH_FLOW_IMPLICIT, &flow_type));
  ASSERT_EQ(OA_OAUTH_FLOW_IMPLICIT, flow_type);
  ASSERT_EQ(CDD_C_SUCCESS,
            map_doc_flow_type(DOC_OAUTH_FLOW_PASSWORD, &flow_type));
  ASSERT_EQ(OA_OAUTH_FLOW_PASSWORD, flow_type);
  ASSERT_EQ(CDD_C_SUCCESS,
            map_doc_flow_type(DOC_OAUTH_FLOW_CLIENT_CREDENTIALS, &flow_type));
  ASSERT_EQ(OA_OAUTH_FLOW_CLIENT_CREDENTIALS, flow_type);
  ASSERT_EQ(CDD_C_SUCCESS,
            map_doc_flow_type(DOC_OAUTH_FLOW_AUTHORIZATION_CODE, &flow_type));
  ASSERT_EQ(OA_OAUTH_FLOW_AUTHORIZATION_CODE, flow_type);
  ASSERT_EQ(CDD_C_SUCCESS,
            map_doc_flow_type(DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION, &flow_type));
  ASSERT_EQ(OA_OAUTH_FLOW_DEVICE_AUTHORIZATION, flow_type);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            map_doc_flow_type(DOC_OAUTH_FLOW_UNSET, &flow_type));
  ASSERT_EQ(OA_OAUTH_FLOW_UNKNOWN, flow_type);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            map_doc_flow_type((enum DocOAuthFlowType)999, &flow_type));

  /* 8. free_openapi_server_variables */
  free_openapi_server_variables(NULL);
  memset(&srv, 0, sizeof(srv));
  free_openapi_server_variables(&srv);

  /* 9. copy_doc_server_variables */
  memset(&srv, 0, sizeof(srv));
  memset(&doc_srv, 0, sizeof(doc_srv));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_doc_server_variables(NULL, &doc_srv));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_doc_server_variables(&srv, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables(&srv, &doc_srv));
  doc_srv.n_variables = 1;
  doc_srv.variables = doc_vars;
  memset(doc_vars, 0, sizeof(doc_vars));
  doc_vars[0].name = (char *)(size_t) "var1";
  doc_vars[0].default_value = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_doc_server_variables(&srv, &doc_srv));
  doc_vars[0].name = NULL;
  doc_vars[0].default_value = (char *)(size_t) "def";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_doc_server_variables(&srv, &doc_srv));
  doc_vars[0].name = (char *)(size_t) "var1";
  doc_vars[0].default_value = (char *)(size_t) "def";
  doc_vars[0].description = (char *)(size_t) "desc";
  enum_vals[0] = (char *)(size_t) "val1";
  enum_vals[1] = (char *)(size_t) "def";
  doc_vars[0].enum_values = enum_vals;
  doc_vars[0].n_enum_values = 2;
  /* strdup fail for default_val */
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables(&srv, &doc_srv));
  g_cdd_strdup_fail = 0;
  /* strdup fail for description */
  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables(&srv, &doc_srv));
  g_cdd_strdup_fail = 0;
  /* strdup fail for enum_values[1] */
  g_cdd_strdup_fail = 5;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables(&srv, &doc_srv));
  g_cdd_strdup_fail = 0;
  /* alloc fail for enum_values */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables(&srv, &doc_srv));
  g_cdd_alloc_fail = 0;
  /* found_default == 0 */
  enum_vals[1] = (char *)(size_t) "no_match";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_doc_server_variables(&srv, &doc_srv));
  enum_vals[1] = (char *)(size_t) "def";

  /* 10. merge_scopes */
  memset(&flow_dst, 0, sizeof(flow_dst));
  memset(&doc_flow, 0, sizeof(doc_flow));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, merge_scopes(NULL, &doc_flow));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, merge_scopes(&flow_dst, NULL));
  doc_flow.n_scopes = 1;
  doc_flow.scopes = doc_scopes;
  memset(doc_scopes, 0, sizeof(doc_scopes));
  doc_scopes[0].name = (char *)(size_t) "read";
  doc_scopes[0].description = (char *)(size_t) "Read access";
  /* strdup fail for name */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, merge_scopes(&flow_dst, &doc_flow));
  g_cdd_strdup_fail = 0;
  /* strdup fail for desc */
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, merge_scopes(&flow_dst, &doc_flow));
  g_cdd_strdup_fail = 0;
  /* success with non-null name and desc */
  doc_scopes[0].name = (char *)(size_t)(size_t) "read";
  doc_scopes[0].description = (char *)(size_t)(size_t) "read desc";
  ASSERT_EQ(CDD_C_SUCCESS, merge_scopes(&flow_dst, &doc_flow));
  if (flow_dst.scopes) {
    size_t si;
    for (si = 0; si < flow_dst.n_scopes; si++) {
      if (flow_dst.scopes[si].name)
        free(flow_dst.scopes[si].name);
      if (flow_dst.scopes[si].description)
        free(flow_dst.scopes[si].description);
    }
    free(flow_dst.scopes);
    flow_dst.scopes = NULL;
    flow_dst.n_scopes = 0;
  }

  /* 11. find_oauth_flow */
  memset(&scheme, 0, sizeof(scheme));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            find_oauth_flow(&scheme, OA_OAUTH_FLOW_IMPLICIT, NULL));
  {
    struct OpenAPI_OAuthFlow *f = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, find_oauth_flow(NULL, OA_OAUTH_FLOW_IMPLICIT, &f));
    ASSERT_EQ(NULL, f);
  }

  /* 12. add_oauth_flows */
  memset(&scheme, 0, sizeof(scheme));
  memset(&doc_sec, 0, sizeof(doc_sec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_oauth_flows(NULL, &doc_sec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_oauth_flows(&scheme, NULL));
  doc_sec.n_flows = 1;
  doc_sec.flows = &doc_flow;
  memset(&doc_flow, 0, sizeof(doc_flow));
  doc_flow.type = DOC_OAUTH_FLOW_UNSET;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_oauth_flows(&scheme, &doc_sec));
  doc_flow.type = DOC_OAUTH_FLOW_IMPLICIT;
  doc_flow.authorization_url = (char *)(size_t) "http://auth";
  doc_flow.token_url = (char *)(size_t) "http://token";
  doc_flow.refresh_url = (char *)(size_t) "http://refresh";
  doc_flow.device_authorization_url = (char *)(size_t) "http://device";
  doc_flow.n_scopes = 1;
  doc_flow.scopes = doc_scopes;
  doc_scopes[0].name = (char *)(size_t) "scope1";
  doc_scopes[0].description = (char *)(size_t) "desc1";
  /* URL strdup fails */
  g_cdd_strdup_fail = 2; /* token_url fail */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_oauth_flows(&scheme, &doc_sec));
  g_cdd_strdup_fail = 0;
  g_cdd_strdup_fail = 3; /* refresh_url fail */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_oauth_flows(&scheme, &doc_sec));
  g_cdd_strdup_fail = 0;
  g_cdd_strdup_fail = 4; /* device_auth fail */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_oauth_flows(&scheme, &doc_sec));
  g_cdd_strdup_fail = 0;
  /* scopes calloc fail */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_oauth_flows(&scheme, &doc_sec));
  g_cdd_alloc_fail = 0;
  /* scope name fail */
  g_cdd_strdup_fail = 5;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_oauth_flows(&scheme, &doc_sec));
  g_cdd_strdup_fail = 0;
  /* scope desc fail */
  g_cdd_strdup_fail = 6;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_oauth_flows(&scheme, &doc_sec));
  g_cdd_strdup_fail = 0;

  /* 13. spec_add_security_scheme */
  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(NULL, &doc_sec));
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, NULL));
  doc_sec.name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));
  doc_sec.name = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));
  /* realloc fail */
  doc_sec.name = (char *)(size_t) "MySec";
  doc_sec.type = DOC_SEC_HTTP;
  doc_sec.scheme = (char *)(size_t) "bearer";
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&spec, &doc_sec));
  g_cdd_alloc_fail = 0;
  /* name strdup fail */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&spec, &doc_sec));
  g_cdd_strdup_fail = 0;
  /* deprecated conflict */
  doc_sec.deprecated_set = 1;
  doc_sec.deprecated = 1;
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));
  doc_sec.deprecated = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));
  /* API key invalid in */
  doc_sec.name = (char *)(size_t) "ApiKeyBad";
  doc_sec.type = DOC_SEC_APIKEY;
  doc_sec.param_name = (char *)(size_t) "X-Api-Key";
  doc_sec.in = DOC_SEC_IN_UNSET;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));
  /* OAuth2 empty flows */
  doc_sec.name = (char *)(size_t) "OAuth2Empty";
  doc_sec.type = DOC_SEC_OAUTH2;
  doc_sec.n_flows = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));
  openapi_spec_free(&spec);

  /* 14. apply_doc_security_schemes */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_security_schemes(NULL, &meta));
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_security_schemes(&spec, NULL));
  meta.n_security_schemes = 1;
  meta.security_schemes = &doc_sec;
  doc_sec.name = (char *)(size_t) "SecMemFail";
  doc_sec.type = DOC_SEC_HTTP;
  doc_sec.scheme = (char *)(size_t) "bearer";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_doc_security_schemes(&spec, &meta));
  g_cdd_strdup_fail = 0;
  openapi_spec_free(&spec);

  /* 15. append_root_security */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  ASSERT_EQ(CDD_C_SUCCESS, append_root_security(NULL, &meta));
  ASSERT_EQ(CDD_C_SUCCESS, append_root_security(&spec, NULL));
  meta.n_security = 1;
  meta.security = &doc_req;
  memset(&doc_req, 0, sizeof(doc_req));
  doc_req.scheme = (char *)(size_t) "oauth";
  doc_req.n_scopes = 1;
  doc_req.scopes = req_scopes;
  req_scopes[0] = (char *)(size_t) "read";
  /* calloc requirements fail */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_security(&spec, &meta));
  g_cdd_alloc_fail = 0;
  /* scheme strdup fail */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_security(&spec, &meta));
  g_cdd_strdup_fail = 0;
  /* scopes calloc fail */
  g_cdd_alloc_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_security(&spec, &meta));
  g_cdd_alloc_fail = 0;
  /* scope strdup fail */
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_security(&spec, &meta));
  g_cdd_strdup_fail = 0;
  openapi_spec_free(&spec);

  /* 16. append_root_servers */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  ASSERT_EQ(CDD_C_SUCCESS, append_root_servers(NULL, &meta));
  ASSERT_EQ(CDD_C_SUCCESS, append_root_servers(&spec, NULL));
  meta.n_servers = 1;
  meta.servers = &doc_srv;
  memset(&doc_srv, 0, sizeof(doc_srv));
  doc_srv.url = (char *)(size_t) "http://srv";
  doc_srv.name = (char *)(size_t) "srv1";
  doc_srv.description = (char *)(size_t) "Main srv";
  /* url strdup fail */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_servers(&spec, &meta));
  g_cdd_strdup_fail = 0;
  /* name strdup fail */
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_servers(&spec, &meta));
  g_cdd_strdup_fail = 0;
  /* description strdup fail */
  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_servers(&spec, &meta));
  g_cdd_strdup_fail = 0;
  openapi_spec_free(&spec);

  /* 17. apply_doc_global_meta */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(NULL, &meta));
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, NULL));
  /* license conflicts */
  meta.license_name = (char *)(size_t) "MIT";
  meta.license_url = (char *)(size_t) "http://lic";
  meta.license_identifier = (char *)(size_t) "MIT-id";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, apply_doc_global_meta(&spec, &meta));
  meta.license_url = NULL;
  meta.license_identifier = (char *)(size_t) "MIT-id";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  meta.license_identifier = NULL;
  meta.license_url = (char *)(size_t) "http://lic";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, apply_doc_global_meta(&spec, &meta));
  openapi_spec_free(&spec);

  /* externalDocs url conflict */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.external_docs_url = (char *)(size_t) "http://docs1";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  meta.external_docs_url = (char *)(size_t) "http://docs2";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, apply_doc_global_meta(&spec, &meta));
  openapi_spec_free(&spec);

  /* 18. spec_apply_tag_meta */
  memset(&spec, 0, sizeof(spec));
  memset(&doc_tag, 0, sizeof(doc_tag));
  ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(NULL, &doc_tag));
  ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, NULL));
  doc_tag.name = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
  doc_tag.name = (char *)(size_t) "Tag1";
  doc_tag.summary = (char *)(size_t) "Summ";
  doc_tag.description = (char *)(size_t) "Desc";
  doc_tag.parent = (char *)(size_t) "Par";
  doc_tag.kind = (char *)(size_t) "K";
  doc_tag.external_docs_url = (char *)(size_t) "http://doc";
  doc_tag.external_docs_description = (char *)(size_t) "ExtDesc";
  ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
  /* apply second time: all fields already populated */
  ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
  openapi_spec_free(&spec);

  /* 19. apply_doc_tag_meta */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_tag_meta(NULL, &meta));
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_tag_meta(&spec, NULL));

  /* 20. collect_tags_from_op, paths, spec */
  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, collect_tags_from_op(NULL, &op));
  ASSERT_EQ(CDD_C_SUCCESS, collect_tags_from_op(&spec, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, collect_tags_from_paths(NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, collect_tags_from_paths(&spec, NULL, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, collect_spec_tags(NULL));
  memset(&path, 0, sizeof(path));
  path.n_operations = 1;
  path.operations = &op;
  op.n_tags = 1;
  op.tags = op_tags;
  op_tags[0] = (char *)(size_t) "OpTag";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, collect_tags_from_paths(&spec, &path, 1));
  g_cdd_strdup_fail = 0;
  path.n_operations = 0;
  path.n_additional_operations = 1;
  path.additional_operations = &op;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, collect_tags_from_paths(&spec, &path, 1));
  g_cdd_strdup_fail = 0;
  openapi_spec_free(&spec);

  /* 21. parse_c_signature_string edge cases */
  memset(&psig, 0, sizeof(psig));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_c_signature_string(NULL, &psig));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_c_signature_string("void foo()", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_c_signature_string("(", &psig));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_c_signature_string("123(int a)", &psig));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_c_signature_string("void foo(int a", &psig));
  /* cleanup args loop when realloc fail on 2nd arg */
  g_cdd_alloc_fail = 4;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_c_signature_string("void foo(int a, int b)", &psig));
  g_cdd_alloc_fail = 0;
  /* free_parsed_sig NULL */
  free_parsed_sig(NULL);

  /* 22. process_file */
  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, process_file(NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, process_file("file.c", NULL));

  /* 23. walker_cb */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, walker_cb(NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, walker_cb("file.c", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, walker_cb("file.txt", &spec));

  /* 24. load_base_spec */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_base_spec(NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_base_spec("base.json", NULL));

  /* 25. c2openapi_cli_main options */
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(0, NULL));
  argv[0] = (char *)(size_t) "c2openapi";
  argv[1] = (char *)(size_t) "--jsonSchemaDialect";
  argv[2] = (char *)(size_t) "http://dialect";
  argv[3] = (char *)(size_t) "-b";
  argv[4] = (char *)(size_t) "nonexistent.json";
  argv[5] = (char *)(size_t) "src";
  argv[6] = (char *)(size_t) "out.json";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(7, argv));

  /* 26. generate_bindings_cli_main short options */
  argv[0] = (char *)(size_t) "bind";
  argv[1] = (char *)(size_t) "-i";
  argv[2] = (char *)(size_t) "include";
  argv[3] = (char *)(size_t) "-o";
  argv[4] = (char *)(size_t) "out_dir";
  argv[5] = (char *)(size_t) "-l";
  argv[6] = (char *)(size_t) "python";
  argv[7] = (char *)(size_t) "-n";
  argv[8] = (char *)(size_t) "mylib";
  argv[9] = (char *)(size_t) "-m";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));

  /* 27. c2openapi_register_types */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2openapi_register_types(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c2openapi_register_types(&spec, NULL));

  /* 30. More apply_doc_global_meta strdup fail */
  {
    memset(&spec, 0, sizeof(spec));
    memset(&meta, 0, sizeof(meta));
    meta.info_title = (char *)(size_t) "NewTitle";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_doc_global_meta(&spec, &meta));
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* 31. merge_scopes with NULL scope name in dst */
  memset(&flow_dst, 0, sizeof(flow_dst));
  flow_dst.n_scopes = 1;
  flow_dst.scopes =
      (struct OpenAPI_OAuthScope *)calloc(1, sizeof(struct OpenAPI_OAuthScope));
  flow_dst.scopes[0].name = NULL;
  memset(&doc_flow, 0, sizeof(doc_flow));
  doc_flow.n_scopes = 1;
  doc_flow.scopes = doc_scopes;
  memset(doc_scopes, 0, sizeof(doc_scopes));
  doc_scopes[0].name = (char *)(size_t) "read";
  ASSERT_EQ(CDD_C_SUCCESS, merge_scopes(&flow_dst, &doc_flow));
  if (flow_dst.scopes) {
    if (flow_dst.scopes[1].name)
      free(flow_dst.scopes[1].name);
    free(flow_dst.scopes);
    flow_dst.scopes = NULL;
    flow_dst.n_scopes = 0;
  }

  /* 32. add_oauth_flows with NULL URLs and NULL scopes */
  memset(&scheme, 0, sizeof(scheme));
  memset(&doc_sec, 0, sizeof(doc_sec));
  doc_sec.n_flows = 1;
  doc_sec.flows = &doc_flow;
  memset(&doc_flow, 0, sizeof(doc_flow));
  doc_flow.type = DOC_OAUTH_FLOW_IMPLICIT;
  doc_flow.authorization_url = NULL;
  doc_flow.token_url = NULL;
  doc_flow.refresh_url = NULL;
  doc_flow.device_authorization_url = NULL;
  doc_flow.scopes = NULL;
  doc_flow.n_scopes = 0;
  ASSERT_EQ(CDD_C_SUCCESS, add_oauth_flows(&scheme, &doc_sec));
  if (scheme.flows) {
    free(scheme.flows);
    scheme.flows = NULL;
    scheme.n_flows = 0;
  }

  /* 33. spec_add_security_scheme branches */
  memset(&spec, 0, sizeof(spec));
  memset(&doc_sec, 0, sizeof(doc_sec));
  doc_sec.name = (char *)(size_t) "ApiKeyEmptyParam";
  doc_sec.type = DOC_SEC_APIKEY;
  doc_sec.param_name = (char *)(size_t) "";
  doc_sec.in = DOC_SEC_IN_HEADER;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));
  doc_sec.name = (char *)(size_t) "HttpNullBearer";
  doc_sec.type = DOC_SEC_HTTP;
  doc_sec.scheme = (char *)(size_t) "bearer";
  doc_sec.bearer_format = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));
  doc_sec.name = (char *)(size_t) "OAuthNullMetaUrl";
  doc_sec.type = DOC_SEC_OAUTH2;
  doc_sec.oauth2_metadata_url = NULL;
  doc_sec.n_flows = 1;
  doc_sec.flows = &doc_flow;
  doc_flow.type = DOC_OAUTH_FLOW_CLIENT_CREDENTIALS;
  doc_flow.token_url = (char *)(size_t) "http://token";
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));
  openapi_spec_free(&spec);

  /* 34. append_root_security with NULL scheme and NULL scope */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.n_security = 1;
  meta.security = &doc_req;
  memset(&doc_req, 0, sizeof(doc_req));
  doc_req.scheme = NULL;
  doc_req.n_scopes = 1;
  doc_req.scopes = req_scopes;
  req_scopes[0] = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, append_root_security(&spec, &meta));
  openapi_spec_free(&spec);

  /* 35. append_root_servers with NULL fields and vrc failure */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.n_servers = 1;
  meta.servers = &doc_srv;
  memset(&doc_srv, 0, sizeof(doc_srv));
  doc_srv.url = (char *)(size_t) "http://srv";
  doc_srv.name = (char *)(size_t) "srv_name";
  doc_srv.description = (char *)(size_t) "srv_desc";
  doc_srv.n_variables = 1;
  doc_srv.variables = doc_vars;
  memset(doc_vars, 0, sizeof(doc_vars));
  doc_vars[0].name = (char *)(size_t) "var_bad";
  doc_vars[0].default_value = NULL; /* causes vrc != CDD_C_SUCCESS */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, append_root_servers(&spec, &meta));
  /* NULL url, name, description */
  doc_srv.url = NULL;
  doc_srv.name = NULL;
  doc_srv.description = NULL;
  doc_srv.n_variables = 0;
  ASSERT_EQ(CDD_C_SUCCESS, append_root_servers(&spec, &meta));
  openapi_spec_free(&spec);

  /* 36. apply_doc_global_meta license & ext docs */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  spec.info.license.name = (char *)(size_t) "Apache-2.0";
  meta.license_name = NULL;
  meta.license_url = (char *)(size_t) "http://apache.org";
  meta.license_identifier = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  spec.info.license.name = NULL; /* avoid double free */
  openapi_spec_free(&spec);

  /* ext docs description when url already set */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  spec.external_docs.url = (char *)(size_t) "http://docs";
  meta.external_docs_url = (char *)(size_t) "http://docs";
  meta.external_docs_description = (char *)(size_t) "New Desc";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  spec.external_docs.url = NULL;
  openapi_spec_free(&spec);

  /* 37. spec_apply_tag_meta */
  memset(&spec, 0, sizeof(spec));
  memset(&doc_tag, 0, sizeof(doc_tag));
  doc_tag.name = (char *)(size_t) "TagNullUrl";
  doc_tag.external_docs_url = NULL;
  doc_tag.external_docs_description = (char *)(size_t) "DescOnly";
  ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
  /* strdup fail for description and parent */
  memset(&doc_tag, 0, sizeof(doc_tag));
  doc_tag.name = (char *)(size_t) "TagFail";
  doc_tag.description = (char *)(size_t) "Desc";
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_apply_tag_meta(&spec, &doc_tag));
  g_cdd_strdup_fail = 0;
  memset(&doc_tag, 0, sizeof(doc_tag));
  doc_tag.name = (char *)(size_t) "TagFail2";
  doc_tag.parent = (char *)(size_t) "Parent";
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_apply_tag_meta(&spec, &doc_tag));
  g_cdd_strdup_fail = 0;
  openapi_spec_free(&spec);

  /* 38. collect_spec_tags with paths and webhooks failure */
  memset(&spec, 0, sizeof(spec));
  spec.n_paths = 1;
  spec.paths = &path;
  memset(&path, 0, sizeof(path));
  path.n_operations = 1;
  path.operations = &op;
  memset(&op, 0, sizeof(op));
  op.n_tags = 1;
  op.tags = op_tags;
  op_tags[0] = (char *)(size_t) "TagX";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, collect_spec_tags(&spec));
  g_cdd_strdup_fail = 0;
  spec.n_paths = 0;
  spec.paths = NULL;
  spec.n_webhooks = 1;
  spec.webhooks = &path;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, collect_spec_tags(&spec));
  g_cdd_strdup_fail = 0;
  spec.n_webhooks = 0;
  spec.webhooks = NULL;

  /* 39. parse_c_signature_string empty parens and arg cleanup */
  memset(&psig, 0, sizeof(psig));
  ASSERT_EQ(CDD_C_SUCCESS, parse_c_signature_string("void foo()", &psig));
  free_parsed_sig(&psig);
  g_cdd_alloc_fail = 5;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_c_signature_string("void foo(int a, int b)", &psig));
  g_cdd_alloc_fail = 0;

  /* 40. c2openapi_cli_main -s, --dialect, and overwrite options */
  {
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t) "-s";
    argv[2] = (char *)(size_t) "http://self_s";
    argv[3] = (char *)(size_t) "--dialect";
    argv[4] = (char *)(size_t) "http://dialect_d";
    argv[5] = (char *)(size_t) "nonexistent_src_dir_99";
    argv[6] = (char *)(size_t) "out.json";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(7, argv));
  }

  /* 41. to_docs_json_cli_main with --no-imports and --no-wrapping */
  {
    const char *tmp_spec = "test_docs_json_flags.json";
    FILE *fp = fopen(tmp_spec, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},\"paths\":{\"/"
            "r\":{\"get\":{\"summary\":\"S\"},\"post\":{\"operationId\":\"op_"
            "post\"}}}}",
            fp);
      fclose(fp);
    }
    argv[0] = (char *)(size_t) "to_docs_json";
    argv[1] = (char *)(size_t) "--input";
    argv[2] = (char *)(size_t)tmp_spec;
    argv[3] = (char *)(size_t) "--no-imports";
    argv[4] = (char *)(size_t) "--no-wrapping";
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(5, argv));
    remove(tmp_spec);
  }

  /* 42. generate_bindings_cli_main flags */
  {
    argv[0] = (char *)(size_t) "bind";
    argv[1] = (char *)(size_t) "--input";
    argv[2] = (char *)(size_t) "include";
    argv[3] = (char *)(size_t) "--output-dir";
    argv[4] = (char *)(size_t) "out_dir";
    argv[5] = (char *)(size_t) "--lang";
    argv[6] = (char *)(size_t) "python";
    argv[7] = (char *)(size_t) "--skip-static";
    argv[8] = (char *)(size_t) "--opaque-pointers";
    argv[9] = (char *)(size_t) "--generate-tests";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));
  }

  /* 43. c2openapi_register_types with NULL fields/members and other kind */
  {
    struct TypeDefList types;
    memset(&types, 0, sizeof(types));
    types.items =
        (struct TypeDefinition *)calloc(4, sizeof(struct TypeDefinition));
    types.size = 4;
    types.capacity = 4;

    types.items[0].kind = KIND_STRUCT;
    types.items[0].name = (char *)(size_t) "StructNullFields";
    types.items[0].details.struct_fields = NULL;

    types.items[1].kind = KIND_ENUM;
    types.items[1].name = (char *)(size_t) "EnumNullMembers";
    types.items[1].details.enum_members = NULL;

    types.items[2].kind = (enum TypeDefinitionKind)999;
    types.items[2].name = (char *)(size_t) "OtherKind";

    types.items[3].kind = KIND_STRUCT;
    types.items[3].name = NULL;

    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_register_types(&spec, &types));

    free(types.items);
    openapi_spec_free(&spec);
  }

  /* 44. process_file error in function doc comment via title conflict */
  {
    const char *fpath = "test_func_doc_err.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @infoTitle DocTitleNew\n */\nvoid bad_route(void) {}\n",
            fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      spec.info.title = (char *)(size_t)c_cdd_strdup_macro("DocTitleExisting");
      rc = process_file(fpath, &spec);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* 45. process_file error in standalone doc comment via title conflict */
  {
    const char *fpath = "test_standalone_doc_err.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @infoTitle DocTitleStandaloneNew\n */\n", fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      spec.info.title = (char *)(size_t)c_cdd_strdup_macro("DocTitleExisting");
      rc = process_file(fpath, &spec);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* 46. process_file function declaration without brace */
  {
    const char *fpath = "test_no_brace.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route GET /nobrace\n */\nvoid no_brace(void);\n", fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      ASSERT_EQ(CDD_C_SUCCESS, process_file(fpath, &spec));
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* 47. c2openapi_cli_main duplicate --self and --dialect */
  {
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t) "--self";
    argv[2] = (char *)(size_t) "http://self1";
    argv[3] = (char *)(size_t) "--self";
    argv[4] = (char *)(size_t) "http://self2";
    argv[5] = (char *)(size_t) "--dialect";
    argv[6] = (char *)(size_t) "http://d1";
    argv[7] = (char *)(size_t) "--dialect";
    argv[8] = (char *)(size_t) "http://d2";
    /* missing dir/file -> error */
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(9, argv));
  }

  /* 48. c2openapi_cli_main --dialect strdup fail */
  {
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t) "--dialect";
    argv[2] = (char *)(size_t) "http://d1";
    argv[3] = (char *)(size_t) "my_empty_dir";
    argv[4] = (char *)(size_t) "out.json";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(5, argv));
    g_cdd_strdup_fail = 0;
  }

  /* 49. c2openapi_register_types with enum_members non-NULL */
  {
    struct TypeDefList types;
    struct EnumMembers em;
    enum_members_init(&em);
    enum_members_add(&em, "VAL1");
    enum_members_add(&em, "VAL2");

    memset(&types, 0, sizeof(types));
    types.items =
        (struct TypeDefinition *)calloc(1, sizeof(struct TypeDefinition));
    types.size = 1;
    types.capacity = 1;
    types.items[0].kind = KIND_ENUM;
    types.items[0].name = (char *)(size_t) "ValidEnum";
    types.items[0].details.enum_members = &em;

    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_register_types(&spec, &types));
    ASSERT_EQ(1, spec.n_defined_schemas);

    free(types.items);
    enum_members_free(&em);
    openapi_spec_free(&spec);
  }

  openapi_spec_free(&spec);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_C2OPENAPI_COVERAGE_H */
