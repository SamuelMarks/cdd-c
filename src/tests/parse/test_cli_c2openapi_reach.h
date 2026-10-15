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

  /* 2. spec_add_security_scheme add_oauth_flows realloc fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    struct DocOAuthFlow f1;
    struct DocOAuthFlow f2;
    struct DocOAuthScope sc[2];
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    memset(&f1, 0, sizeof(f1));
    memset(&f2, 0, sizeof(f2));
    memset(sc, 0, sizeof(sc));

    d.name = cdd_test_mut_str("oauth_sch");
    d.type = DOC_SEC_OAUTH2;
    f1.type = DOC_OAUTH_FLOW_IMPLICIT;
    f1.authorization_url = cdd_test_mut_str("http://auth");
    d.flows = &f1;
    d.n_flows = 1;
    ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&s, &d));

    f2.type = DOC_OAUTH_FLOW_PASSWORD;
    f2.token_url = cdd_test_mut_str("http://token");
    d.flows = &f2;
    d.n_flows = 1;
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&s, &d));
    g_cdd_alloc_fail = 0;

    /* Test multi-scope OOM rollback in add_oauth_flows */
    sc[0].name = cdd_test_mut_str("read");
    sc[0].description = cdd_test_mut_str("Read access");
    sc[1].name = cdd_test_mut_str("write");
    sc[1].description = cdd_test_mut_str("Write access");
    f2.scopes = sc;
    f2.n_scopes = 2;
    g_cdd_strdup_fail = 4; /* Fail on 2nd scope name */
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&s, &d));
    g_cdd_strdup_fail = 0;
    g_cdd_strdup_fail = 5; /* Fail on 2nd scope description */
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&s, &d));
    g_cdd_strdup_fail = 0;

    openapi_spec_free(&s);
  }

  /* 3. spec_add_security_scheme new_schemes realloc fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    d.name = cdd_test_mut_str("sch_alloc_fail");
    d.type = DOC_SEC_HTTP;
    d.scheme = cdd_test_mut_str("bearer");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&s, &d));
    g_cdd_alloc_fail = 0;
    openapi_spec_free(&s);
  }

  /* 4. spec_add_security_scheme doc->name strdup fail */
  {
    struct OpenAPI_Spec s;
    struct DocSecurityScheme d;
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    d.name = cdd_test_mut_str("sch_strdup_fail");
    d.type = DOC_SEC_HTTP;
    d.scheme = cdd_test_mut_str("bearer");
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
    d.name = cdd_test_mut_str("api_key_sch");
    d.type = DOC_SEC_APIKEY;
    d.in = DOC_SEC_IN_HEADER;
    d.param_name = cdd_test_mut_str("X-API-Key");
    ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&s, &d));

    free(s.security_schemes[0].key_name);
    s.security_schemes[0].key_name = NULL;
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, spec_add_security_scheme(&s, &d));
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&s);
  }

  /* 6. c2openapi_cli_main collect_spec_tags fail via base spec tag OOM */
  {
    const char *base_path = "build/test_c2o_base_tags.json";
    char *argv[5];
    int step;
    int hit_tag_oom = 0;
    TEST_MKDIR("build/nonexistent_empty_src_dir");
    fs_write_to_file(base_path,
                     "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\","
                     "\"version\":\"1\"},\"paths\":{\"/p\":{\"get\":{"
                     "\"tags\":[\"my_op_tag\"],\"responses\":{\"200\":{"
                     "\"description\":\"ok\"}}}}}}");
    argv[0] = cdd_test_mut_str("c2openapi");
    argv[1] = cdd_test_mut_str("--base");
    argv[2] = cdd_test_mut_str(base_path);
    argv[3] = cdd_test_mut_str("build/nonexistent_empty_src_dir");
    argv[4] = cdd_test_mut_str("build/out_tags_fail.json");
    for (step = 1; step <= 20; ++step) {
      cdd_c_error_t rc_main;
      g_cdd_alloc_fail = step;
      rc_main = c2openapi_cli_main(5, argv);
      g_cdd_alloc_fail = 0;
      if (rc_main == CDD_C_ERROR_MEMORY) {
        hit_tag_oom = 1;
        break;
      }
    }
    ASSERT_EQ(1, hit_tag_oom);
    remove(base_path);
    remove("build/out_tags_fail.json");
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_C2OPENAPI_REACH_H */
