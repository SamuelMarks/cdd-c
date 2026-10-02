/**
 * @file test_openapi_client_gen_expanded.h
 * @brief Tests for OpenAPI Client Generator extra cases, expanded mocks, and IO
 * failures.
 */

#ifndef TEST_OPENAPI_CLIENT_GEN_EXPANDED_H
#define TEST_OPENAPI_CLIENT_GEN_EXPANDED_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_client_gen_fail;

TEST test_client_gen_extra_cases(void) {
  struct OpenAPI_Server srv;
  char *out = NULL;
  char *escaped = NULL;
  int rc = 0;

  /* 1. Unclosed brace */
  memset(&srv, 0, sizeof(srv));
  srv.url = (char *)(size_t)(size_t) "http://api.com/{unclosed";
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* 2. Empty variable */
  srv.url = (char *)(size_t)(size_t) "http://api.com/{}";
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* 3. Missing variable */
  srv.url = (char *)(size_t)(size_t) "http://api.com/{missing}";
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* 4. Escaping special chars: \r, \t, \n, \", \\ */
  rc = escape_c_string_literal("a\rb\tc\nd\"e\\f", &escaped);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(escaped != NULL);
  ASSERT(strstr(escaped, "\\r") != NULL);
  ASSERT(strstr(escaped, "\\t") != NULL);
  ASSERT(strstr(escaped, "\\n") != NULL);
  ASSERT(strstr(escaped, "\\\"") != NULL);
  ASSERT(strstr(escaped, "\\\\") != NULL);
  free(escaped);

  /* 5. select_operation_server fallback to path->servers */
  {
    struct OpenAPI_Path p;
    struct OpenAPI_Server p_srv;
    struct OpenAPI_Server *res_srv = NULL;
    memset(&p, 0, sizeof(p));
    memset(&p_srv, 0, sizeof(p_srv));
    p_srv.url = (char *)(size_t)(size_t) "http://path-server.com";
    p.servers = &p_srv;
    p.n_servers = 1;
    rc = select_operation_server(&p, NULL, &res_srv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(res_srv == &p_srv);

    /* Both NULL */
    rc = select_operation_server(NULL, NULL, &res_srv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(res_srv == NULL);
  }

  {
    struct OpenAPI_Spec s_extra;
    struct OpenAPI_Path p_extra;
    struct OpenAPI_Operation ops_extra[2];
    struct OpenApiClientConfig cfg_extra;
    struct OpenAPI_Response resp_extra;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&s_extra));
    memset(&p_extra, 0, sizeof(p_extra));
    memset(ops_extra, 0, sizeof(ops_extra));
    memset(&cfg_extra, 0, sizeof(cfg_extra));
    memset(&resp_extra, 0, sizeof(resp_extra));
    resp_extra.code = (char *)(size_t) "200";

    /* op 0: operation_id != NULL, description == NULL, summary == NULL */
    ops_extra[0].verb = OA_VERB_GET;
    ops_extra[0].operation_id = (char *)(size_t) "op_no_desc_no_sum";
    ops_extra[0].description = NULL;
    ops_extra[0].summary = NULL;
    ops_extra[0].responses = &resp_extra;
    ops_extra[0].n_responses = 1;

    /* op 1: description == NULL, summary != NULL */
    ops_extra[1].verb = OA_VERB_POST;
    ops_extra[1].operation_id = (char *)(size_t) "op_summary_only";
    ops_extra[1].summary = (char *)(size_t) "Only summary";
    ops_extra[1].description = NULL;
    ops_extra[1].responses = &resp_extra;
    ops_extra[1].n_responses = 1;

    p_extra.route = (char *)(size_t) "/extra/valid";
    p_extra.operations = ops_extra;
    p_extra.n_operations = 2;

    s_extra.paths = &p_extra;
    s_extra.n_paths = 1;

    cfg_extra.filename_base = (char *)(size_t) "build/test_out/test_extra_ops";
    cfg_extra.no_installable_package = 1;

    rc = openapi_client_generate(&s_extra, &cfg_extra);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    remove("build/test_out/src/test_extra_ops.h");
    remove("build/test_out/src/test_extra_ops.c");
    remove("build/test_out/src/test_extra_ops_models.h");
    remove("build/test_out/src/test_extra_ops_models.c");
  }

  PASS();
}

TEST test_client_gen_create_tests_mocks_expanded(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resps[2];
  struct OpenAPI_Path path;
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter params[2];
  struct OpenAPI_Server servers[2];
  struct OpenAPI_MediaType req_media;
  int rc = 0;

  memset(&op, 0, sizeof(op));
  memset(&spec, 0, sizeof(spec));
  memset(resps, 0, sizeof(resps));
  memset(&path, 0, sizeof(path));
  memset(&config, 0, sizeof(config));
  memset(params, 0, sizeof(params));
  memset(servers, 0, sizeof(servers));
  memset(&req_media, 0, sizeof(req_media));

  path.route = (char *)(size_t)(size_t) "/items/{id}";
  params[0].name = (char *)(size_t)(size_t) "p1";
  params[0].in = OA_PARAM_IN_QUERY;
  params[1].name = (char *)(size_t)(size_t) "p2";
  params[1].in = OA_PARAM_IN_QUERYSTRING;

  op.operation_id = (char *)(size_t)(size_t) "doExpanded";
  op.verb = OA_VERB_POST;
  op.parameters = params;
  op.n_parameters = 2;

  /* First response 200 with schema ref, second default */
  resps[0].code = (char *)(size_t)(size_t) "200";
  resps[0].schema.ref_name = (char *)(size_t)(size_t) "ItemResponse";
  resps[0].schema.is_array = 0;
  resps[1].code = (char *)(size_t)(size_t) "default";
  op.responses = resps;
  op.n_responses = 2;

  /* Custom req body media type */
  req_media.name = (char *)(size_t)(size_t) "application/xml";
  op.req_body_media_types = &req_media;
  op.n_req_body_media_types = 1;

  path.operations = &op;
  path.n_operations = 1;
  spec.paths = &path;
  spec.n_paths = 1;

  /* Server 1: http://api.com/v1 (has :// with path) */
  servers[0].url = (char *)(size_t)(size_t) "http://api.com/v1";
  spec.servers = servers;
  spec.n_servers = 1;

  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp1";
  config.create_tests_and_mocks = 1;
  config.no_installable_package = 0; /* test generate_cmake_project */

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_exp1_models.h");
  remove("build/test_out/src/test_client_sdk_exp1_models.c");
  remove("build/test_out/src/test_client_sdk_exp1.h");
  remove("build/test_out/src/test_client_sdk_exp1.c");
  remove("build/test_out/CMakeLists.txt");

  /* Server 2: http://api.com (no path after hostname) */
  servers[0].url = (char *)(size_t)(size_t) "http://api.com";
  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp2";
  config.no_installable_package = 1;
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_exp2_models.h");
  remove("build/test_out/src/test_client_sdk_exp2_models.c");
  remove("build/test_out/src/test_client_sdk_exp2.h");
  remove("build/test_out/src/test_client_sdk_exp2.c");

  /* Server 3: /v1 (no ://) */
  servers[0].url = (char *)(size_t)(size_t) "/v1";
  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp3";
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_exp3_models.h");
  remove("build/test_out/src/test_client_sdk_exp3_models.c");
  remove("build/test_out/src/test_client_sdk_exp3.h");
  remove("build/test_out/src/test_client_sdk_exp3.c");

  /* Swagger 2.0 basePath: "/" */
  spec.servers = NULL;
  spec.n_servers = 0;
  spec.basePath = (char *)(size_t)(size_t) "/";
  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp4";
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_exp4_models.h");
  remove("build/test_out/src/test_client_sdk_exp4_models.c");
  remove("build/test_out/src/test_client_sdk_exp4.h");
  remove("build/test_out/src/test_client_sdk_exp4.c");

  /* Swagger 2.0 basePath: "/v2" and is_array = 1 */
  spec.servers = NULL;
  spec.n_servers = 0;
  spec.basePath = (char *)(size_t)(size_t) "/v2";
  resps[0].schema.is_array = 1;
  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp5";
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_exp5_models.h");
  remove("build/test_out/src/test_client_sdk_exp5_models.c");
  remove("build/test_out/src/test_client_sdk_exp5.h");
  remove("build/test_out/src/test_client_sdk_exp5.c");

  /* Long route and unclosed brace */
  {
    char long_route[300];
    struct OpenAPI_Path long_path;
    memset(long_route, 'a', 280);
    long_route[0] = '/';
    long_route[275] = '{';
    long_route[276] = 'x';
    long_route[277] = '}';
    long_route[278] = '\0';
    memset(&long_path, 0, sizeof(long_path));
    long_path.route = long_route;
    long_path.operations = &op;
    long_path.n_operations = 1;
    spec.paths = &long_path;
    config.filename_base =
        (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp_long";
    rc = openapi_client_generate(&spec, &config);
    ASSERT_EQ(0, rc);

    remove("build/test_out/src/test/test_sdk.c");
    remove("build/test_out/src/test_client_sdk_exp_long_models.h");
    remove("build/test_out/src/test_client_sdk_exp_long_models.c");
    remove("build/test_out/src/test_client_sdk_exp_long.h");
    remove("build/test_out/src/test_client_sdk_exp_long.c");
  }

  /* Test response with default code and null ref_name, plus spec without
   * basePath or servers */
  {
    struct OpenAPI_Response def_resp;
    memset(&def_resp, 0, sizeof(def_resp));
    def_resp.code = (char *)(size_t)(size_t) "default";
    def_resp.schema.ref_name = NULL;

    op.responses = &def_resp;
    op.n_responses = 1;
    spec.paths = &path;
    spec.n_paths = 1;
    spec.servers = NULL;
    spec.n_servers = 0;
    spec.basePath = NULL;
    path.route = (char *)(size_t)(size_t) "/items";
    config.filename_base =
        (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp6";
    rc = openapi_client_generate(&spec, &config);
    ASSERT_EQ(0, rc);

    remove("build/test_out/src/test/test_sdk.c");
    remove("build/test_out/src/test_client_sdk_exp6_models.h");
    remove("build/test_out/src/test_client_sdk_exp6_models.c");
    remove("build/test_out/src/test_client_sdk_exp6.h");
    remove("build/test_out/src/test_client_sdk_exp6.c");
  }

  /* Test response with ref_name and is_array = 1 */
  {
    struct OpenAPI_Response arr_resp;
    memset(&arr_resp, 0, sizeof(arr_resp));
    arr_resp.code = (char *)(size_t)(size_t) "200";
    arr_resp.schema.ref_name = (char *)(size_t)(size_t) "Item";
    arr_resp.schema.is_array = 1;

    op.responses = &arr_resp;
    op.n_responses = 1;
    spec.paths = &path;
    spec.n_paths = 1;
    spec.servers = NULL;
    spec.n_servers = 0;
    config.filename_base =
        (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp7";
    rc = openapi_client_generate(&spec, &config);
    ASSERT_EQ(0, rc);

    remove("build/test_out/src/test/test_sdk.c");
    remove("build/test_out/src/test_client_sdk_exp7_models.h");
    remove("build/test_out/src/test_client_sdk_exp7_models.c");
    remove("build/test_out/src/test_client_sdk_exp7.h");
    remove("build/test_out/src/test_client_sdk_exp7.c");
  }

  /* Test spec with servers > 0 but servers[0].url == NULL, and op with
   * n_responses == 0 */
  {
    struct OpenAPI_Server null_srv[1];
    memset(null_srv, 0, sizeof(null_srv));
    null_srv[0].url = NULL;

    op.responses = NULL;
    op.n_responses = 0;
    spec.paths = &path;
    spec.n_paths = 1;
    spec.servers = null_srv;
    spec.n_servers = 1;
    spec.basePath = NULL;
    config.filename_base =
        (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp8";
    rc = openapi_client_generate(&spec, &config);
    ASSERT_EQ(0, rc);

    remove("build/test_out/src/test/test_sdk.c");
    remove("build/test_out/src/test_client_sdk_exp8_models.h");
    remove("build/test_out/src/test_client_sdk_exp8_models.c");
    remove("build/test_out/src/test_client_sdk_exp8.h");
    remove("build/test_out/src/test_client_sdk_exp8.c");
  }

  /* Test spec with param in header, and response with code == NULL */
  {
    struct OpenAPI_Parameter hdr_param;
    struct OpenAPI_Response nocode_resp;
    memset(&hdr_param, 0, sizeof(hdr_param));
    memset(&nocode_resp, 0, sizeof(nocode_resp));

    hdr_param.name = (char *)(size_t)(size_t) "X-Header";
    hdr_param.in = OA_PARAM_IN_HEADER;
    hdr_param.type = (char *)(size_t)(size_t) "string";

    nocode_resp.code = NULL;

    op.parameters = &hdr_param;
    op.n_parameters = 1;
    op.responses = &nocode_resp;
    op.n_responses = 1;
    spec.paths = &path;
    spec.n_paths = 1;
    spec.servers = NULL;
    spec.n_servers = 0;
    spec.basePath = NULL;
    config.filename_base =
        (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp9";
    rc = openapi_client_generate(&spec, &config);
    ASSERT_EQ(0, rc);

    remove("build/test_out/src/test/test_sdk.c");
    remove("build/test_out/src/test_client_sdk_exp9_models.h");
    remove("build/test_out/src/test_client_sdk_exp9_models.c");
    remove("build/test_out/src/test_client_sdk_exp9.h");
    remove("build/test_out/src/test_client_sdk_exp9.c");
  }

  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_io_failures(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  struct StructFields sf[1];
  char *snames[1];
  int i;
  int rc = 0;
  static struct OpenAPI_Callback cb;
  static struct OpenAPI_Link link;
  static struct OpenAPI_SecurityRequirementSet sec;
  static struct OpenAPI_Parameter p_io;

  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  memset(&p_io, 0, sizeof(p_io));
  p_io.name = (char *)(size_t)(size_t) "param1";
  p_io.in = OA_PARAM_IN_QUERY;
  p_io.type = (char *)(size_t)(size_t) "string";
  op.parameters = &p_io;
  op.n_parameters = 1;
  op.description = (char *)(size_t)(size_t) "Operation description";
  op.summary = (char *)(size_t)(size_t) "Operation summary";
  memset(&cb, 0, sizeof(cb));
  memset(&link, 0, sizeof(link));
  memset(&sec, 0, sizeof(sec));
  op.external_docs.url = (char *)(size_t)(size_t) "http://example.com";
  op.callbacks = &cb;
  op.responses[0].links = &link;
  op.security = &sec;
  op.deprecated = 1;

  memset(sf, 0, sizeof(sf));
  snames[0] = (char *)(size_t)(size_t) "SampleSchema";
  spec.defined_schemas = sf;
  spec.defined_schema_names = snames;
  spec.n_defined_schemas = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = (char *)(size_t)(size_t) "build/test_out/test_cg_io";
  config.no_installable_package = 1;

  for (i = 0; i <= 450; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    rc = openapi_client_generate(&spec, &config);
    if (rc != CDD_C_SUCCESS) {
    }
  }

  config.no_installable_package = 0;
  for (i = 0; i <= 60; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    rc = openapi_client_generate(&spec, &config);
    if (rc != CDD_C_SUCCESS) {
    }
  }

  config.create_tests_and_mocks = 1;
  for (i = 0; i <= 100; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    rc = openapi_client_generate(&spec, &config);
    if (rc != CDD_C_SUCCESS) {
    }
  }

  g_fail_io_after = -1;
  remove("build/test_out/src/test_cg_io.h");
  remove("build/test_out/src/test_cg_io.c");
  remove("build/test_out/src/test_cg_io_models.h");
  remove("build/test_out/src/test_cg_io_models.c");
  remove("build/test_out/src/url_utils.h");
  remove("build/test_out/src/url_utils.c");
  remove("build/test_out/CMakeLists.txt");
  PASS();
}

SUITE(openapi_client_gen_expanded_suite) {
  RUN_TEST(test_client_gen_extra_cases);
  RUN_TEST(test_client_gen_create_tests_mocks_expanded);
  RUN_TEST(test_client_gen_io_failures);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_CLIENT_GEN_EXPANDED_H */
