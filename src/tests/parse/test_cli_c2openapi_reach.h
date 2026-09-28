/**
 * @file test_cli_c2openapi_reach.h
 * @brief Unit tests for reaching 100% c2openapi coverage.
 */

#ifndef TEST_CLI_C2OPENAPI_REACH_H
#define TEST_CLI_C2OPENAPI_REACH_H

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

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cli_fail_spec_find_tag;
extern C_CDD_EXPORT int g_cdd_fail_token_find_next;
extern C_CDD_EXPORT int g_cli_fail_is_source_file;
extern C_CDD_EXPORT int g_openapi_spec_init_fail;
extern C_CDD_EXPORT int g_cli_fail_collect_spec_tags;
extern C_CDD_EXPORT int g_cli_fail_spec_has_tag;
extern C_CDD_EXPORT int g_cli_fail_find_oauth_flow;
extern C_CDD_EXPORT int g_cli_fail_spec_find_security_scheme;
extern C_CDD_EXPORT int g_cli_fail_map_doc_security_in;
extern C_CDD_EXPORT int g_cli_fail_map_doc_security_type;
#endif

TEST test_c2openapi_reach_100_percent(void) {
  /* 1. spec_add_tag realloc fail */
  {
    struct OpenAPI_Spec s;
    memset(&s, 0, sizeof(s));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_tag(&s, "tag"));
    g_cdd_alloc_fail = 0;
    openapi_spec_free(&s);
  }

  /* 1b. spec_add_tag spec_has_tag fail */
  {
    struct OpenAPI_Spec s;
    memset(&s, 0, sizeof(s));
    g_cli_fail_spec_has_tag = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_add_tag(&s, "tag"));
    g_cli_fail_spec_has_tag = 0;
    openapi_spec_free(&s);
  }

  /* 2. spec_add_security_scheme add_oauth_flows realloc fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    struct DocOAuthFlow f1;
    struct DocOAuthFlow f2;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    memset(&f1, 0, sizeof(f1));
    memset(&f2, 0, sizeof(f2));

    d.name = (char *)(size_t) "oauth_sch";
    d.type = DOC_SEC_OAUTH2;
    f1.type = DOC_OAUTH_FLOW_IMPLICIT;
    f1.authorization_url = (char *)(size_t) "http://auth";
    d.flows = &f1;
    d.n_flows = 1;
    ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&s, &d));

    f2.type = DOC_OAUTH_FLOW_PASSWORD;
    f2.token_url = (char *)(size_t) "http://token";
    d.flows = &f2;
    d.n_flows = 1;
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&s, &d));
    g_cdd_alloc_fail = 0;
    openapi_spec_free(&s);
  }

  /* 2b. spec_add_security_scheme find_oauth_flow fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    struct DocOAuthFlow f1;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    memset(&f1, 0, sizeof(f1));
    d.name = (char *)(size_t) "oauth_sch";
    d.type = DOC_SEC_OAUTH2;
    f1.type = DOC_OAUTH_FLOW_IMPLICIT;
    f1.authorization_url = (char *)(size_t) "http://auth";
    d.flows = &f1;
    d.n_flows = 1;
    g_cli_fail_find_oauth_flow = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_add_security_scheme(&s, &d));
    g_cli_fail_find_oauth_flow = 0;
    openapi_spec_free(&s);
  }

  /* 3. spec_add_security_scheme new_schemes realloc fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    d.name = (char *)(size_t) "sch_alloc_fail";
    d.type = DOC_SEC_HTTP;
    d.scheme = (char *)(size_t) "bearer";
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&s, &d));
    g_cdd_alloc_fail = 0;
    openapi_spec_free(&s);
  }

  /* 3b. spec_add_security_scheme c2openapi_map_doc_security_type fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    d.name = (char *)(size_t) "map_fail_sch";
    d.type = DOC_SEC_HTTP;
    g_cli_fail_map_doc_security_type = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_add_security_scheme(&s, &d));
    g_cli_fail_map_doc_security_type = 0;
    openapi_spec_free(&s);
  }

  /* 3c. spec_add_security_scheme spec_find_security_scheme fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    d.name = (char *)(size_t) "find_fail_sch";
    d.type = DOC_SEC_HTTP;
    d.scheme = (char *)(size_t) "bearer";
    g_cli_fail_spec_find_security_scheme = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_add_security_scheme(&s, &d));
    g_cli_fail_spec_find_security_scheme = 0;
    openapi_spec_free(&s);
  }

  /* 4. spec_add_security_scheme doc->name strdup fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    d.name = (char *)(size_t) "sch_strdup_fail";
    d.type = DOC_SEC_HTTP;
    d.scheme = (char *)(size_t) "bearer";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&s, &d));
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&s);
  }

  /* 5. spec_add_security_scheme apikey key_name strdup fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    d.name = (char *)(size_t) "api_key_sch";
    d.type = DOC_SEC_APIKEY;
    d.in = DOC_SEC_IN_HEADER;
    d.param_name = (char *)(size_t) "X-API-Key";
    ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&s, &d));

    free(s.security_schemes[0].key_name);
    s.security_schemes[0].key_name = NULL;
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&s, &d));
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&s);
  }

  /* 5b. spec_add_security_scheme map_doc_security_in fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    d.name = (char *)(size_t) "apikey_map_fail";
    d.type = DOC_SEC_APIKEY;
    d.in = DOC_SEC_IN_HEADER;
    d.param_name = (char *)(size_t) "X-Key";
    g_cli_fail_map_doc_security_in = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, spec_add_security_scheme(&s, &d));
    g_cli_fail_map_doc_security_in = 0;
    openapi_spec_free(&s);
  }

  /* 6. spec_apply_tag_meta with spec_find_tag returning error */
  {
    struct OpenAPI_Spec s;
    struct DocTagMeta meta;
    memset(&s, 0, sizeof(s));
    memset(&meta, 0, sizeof(meta));
    meta.name = (char *)(size_t) "tag1";
    g_cli_fail_spec_find_tag = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_apply_tag_meta(&s, &meta));
    g_cli_fail_spec_find_tag = 0;
    openapi_spec_free(&s);
  }

  /* 7. parse_c_fn_sig token_find_next error */
  {
    struct C2OpenAPI_ParsedSig sig;
    const char *test_code = "int my_fn(int a, int b);";
    memset(&sig, 0, sizeof(sig));
    g_cdd_fail_token_find_next = 2;
    ASSERT_EQ(CDD_C_SUCCESS,
              c2openapi_parse_c_signature_string(test_code, &sig));
    c2openapi_free_parsed_sig(&sig);
    memset(&sig, 0, sizeof(sig));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_parse_c_signature_string(test_code, &sig));
    g_cdd_fail_token_find_next = 0;
    c2openapi_free_parsed_sig(&sig);
  }

  /* 8. walker_cb is_source_file error */
  {
    struct OpenAPI_Spec s;
    memset(&s, 0, sizeof(s));
    g_cli_fail_is_source_file = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, walker_cb("foo.c", &s));
    g_cli_fail_is_source_file = 0;
    openapi_spec_free(&s);
  }

  /* 9. c2openapi_cli_main openapi_spec_init fail */
  {
    char *argv[3];
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t) "src";
    argv[2] = (char *)(size_t) "out.json";
    g_openapi_spec_init_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_cli_main(3, argv));
    g_openapi_spec_init_fail = 0;
  }

  /* 10. c2openapi_cli_main collect_spec_tags fail */
  {
    char *argv[3];
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)get_mocks_dir();
    argv[2] = (char *)(size_t) "out.json";
    g_cli_fail_collect_spec_tags = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_cli_main(3, argv));
    g_cli_fail_collect_spec_tags = 0;
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_C2OPENAPI_REACH_H */
