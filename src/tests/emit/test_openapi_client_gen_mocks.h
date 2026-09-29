/**
 * @file test_openapi_client_gen_mocks.h
 * @brief Tests for OpenAPI Client Generator test/mock generation and error
 * injection.
 */

#ifndef TEST_OPENAPI_CLIENT_GEN_MOCKS_H
#define TEST_OPENAPI_CLIENT_GEN_MOCKS_H

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

TEST test_client_gen_defined_schemas(void) {
  struct OpenAPI_Spec spec;
  struct StructFields sf[3];
  char *names[3];
  struct OpenApiClientConfig config;
  char *content = NULL;
  size_t sz;
  int rc;

  memset(&spec, 0, sizeof(spec));
  memset(sf, 0, sizeof(sf));
  memset(&config, 0, sizeof(config));

  names[0] = (char *)(size_t)(size_t) "MyEnum";
  sf[0].is_enum = 1;

  names[1] = (char *)(size_t)(size_t) "MyUnion";
  sf[1].is_union = 1;

  names[2] = (char *)(size_t)(size_t) "MyStruct";

  spec.defined_schemas = sf;
  spec.defined_schema_names = names;
  spec.n_defined_schemas = 3;

  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_gen_schemas";
  config.func_prefix = (char *)(size_t)(size_t) "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file("build/test_out/src/test_client_gen_schemas_models.h", "r",
               &content, &sz);
  ASSERT(content != NULL);
  free(content);

  remove("build/test_out/src/test_client_gen_schemas_models.h");
  remove("build/test_out/src/test_client_gen_schemas_models.c");
  remove("build/test_out/src/test_client_gen_schemas.h");
  remove("build/test_out/src/test_client_gen_schemas.c");
  remove("build/test_out/src/url_utils.h");
  remove("build/test_out/src/url_utils.c");

  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_create_tests_mocks(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation ops[5];
  struct OpenAPI_Response resps[5];
  struct OpenAPI_Path path;
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter param;
  int rc;

  memset(&spec, 0, sizeof(spec));
  memset(ops, 0, sizeof(ops));
  memset(resps, 0, sizeof(resps));
  memset(&path, 0, sizeof(path));
  memset(&config, 0, sizeof(config));
  memset(&param, 0, sizeof(param));

  path.route = (char *)(size_t)(size_t) "/items/{id}";
  param.name = (char *)(size_t)(size_t) "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.type = (char *)(size_t)(size_t) "string";

  ops[0].operation_id = (char *)(size_t)(size_t) "getItems";
  ops[0].verb = OA_VERB_GET;
  resps[0].code = (char *)(size_t)(size_t) "200";
  ops[0].responses = &resps[0];
  ops[0].n_responses = 1;
  ops[0].parameters = &param;
  ops[0].n_parameters = 1;

  ops[1].operation_id = (char *)(size_t)(size_t) "postItem";
  ops[1].verb = OA_VERB_POST;
  ops[1].req_body.is_array = 0;
  resps[1].code = (char *)(size_t)(size_t) "201";
  ops[1].responses = &resps[1];
  ops[1].n_responses = 1;

  ops[2].operation_id = (char *)(size_t)(size_t) "putItem";
  ops[2].verb = OA_VERB_PUT;
  ops[2].req_body.is_array = 1;
  resps[2].code = (char *)(size_t)(size_t) "200";
  ops[2].responses = &resps[2];
  ops[2].n_responses = 1;

  ops[3].operation_id = (char *)(size_t)(size_t) "deleteItem";
  ops[3].verb = OA_VERB_DELETE;
  resps[3].code = (char *)(size_t)(size_t) "204";
  ops[3].responses = &resps[3];
  ops[3].n_responses = 1;

  ops[4].operation_id = (char *)(size_t)(size_t) "patchItem";
  ops[4].verb = OA_VERB_PATCH;
  resps[4].code = (char *)(size_t)(size_t) "200";
  ops[4].responses = &resps[4];
  ops[4].n_responses = 1;

  path.operations = ops;
  path.n_operations = 5;

  spec.paths = &path;
  spec.n_paths = 1;

  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk";
  config.create_tests_and_mocks = 1;
  config.no_installable_package = 1;

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_models.h");
  remove("build/test_out/src/test_client_sdk_models.c");
  remove("build/test_out/src/test_client_sdk.h");
  remove("build/test_out/src/test_client_sdk.c");

  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_mock_errors(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  struct OpenAPI_Server srv;
  struct OpenAPI_ServerVariable var;
  struct StructFields sf[3];
  char *names[3];
  char *out = NULL;
  int rc;

  memset(&op, 0, sizeof(op));

  /* Setup server with variable for render_server_url_default mocks */
  memset(&srv, 0, sizeof(srv));
  memset(&var, 0, sizeof(var));
  var.name = (char *)(size_t)(size_t) "v";
  var.default_value = (char *)(size_t)(size_t) "val";
  srv.url = (char *)(size_t)(size_t) "http://api.com/{v}";
  srv.variables = &var;
  srv.n_variables = 1;

  /* mock 1: pass 1 malloc fail */
  g_client_gen_fail = 1;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 2: out malloc fail */
  g_client_gen_fail = 2;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 3: pass 2 name malloc fail */
  g_client_gen_fail = 3;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 47: pass 2 end == NULL */
  g_client_gen_fail = 47;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 48: pass 2 name_len == 0 */
  g_client_gen_fail = 48;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 49: pass 2 var == NULL */
  g_client_gen_fail = 49;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 4: escape_c_string_literal malloc fail */
  g_client_gen_fail = 4;
  rc = escape_c_string_literal("test", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 50: build_base_url_literal escaped == NULL */
  g_client_gen_fail = 50;
  rc = build_base_url_literal("http://test", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 5: build_base_url_literal malloc fail */
  g_client_gen_fail = 5;
  rc = build_base_url_literal("http://test", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 6: generate_guard malloc fail */
  g_client_gen_fail = 6;
  rc = generate_guard("myguard", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 7: derive_model_header malloc fail */
  g_client_gen_fail = 7;
  rc = derive_model_header("mybase", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 8: sanitize_tag malloc fail */
  g_client_gen_fail = 8;
  rc = sanitize_tag("my-tag", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 9: build_effective_parameters calloc fail */
  {
    struct OpenAPI_Parameter *p_out = NULL;
    size_t p_count = 0;
    struct OpenAPI_Path p_path;
    struct OpenAPI_Parameter p_param;
    memset(&p_path, 0, sizeof(p_path));
    memset(&p_param, 0, sizeof(p_param));
    p_path.parameters = &p_param;
    p_path.n_parameters = 1;
    g_client_gen_fail = 9;
    rc = build_effective_parameters(&p_path, NULL, &p_out, &p_count);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_client_gen_fail = 0;
  }

  {
    const struct OpenAPI_ServerVariable *sv_out = NULL;
    struct OpenAPI_Server srv_nullvarname;
    struct OpenAPI_ServerVariable var_noname;
    ASSERT_EQ(CDD_C_SUCCESS, find_server_variable(NULL, "v", &sv_out));
    ASSERT(sv_out == NULL);
    ASSERT_EQ(CDD_C_SUCCESS, find_server_variable(&srv, NULL, &sv_out));
    ASSERT(sv_out == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_server_variable(&srv, "nonexistent", &sv_out));
    ASSERT(sv_out == NULL);
    {
      struct OpenAPI_Server srv_novars;
      memset(&srv_novars, 0, sizeof(srv_novars));
      ASSERT_EQ(CDD_C_SUCCESS, find_server_variable(&srv_novars, "v", &sv_out));
      ASSERT(sv_out == NULL);
    }
    memset(&srv_nullvarname, 0, sizeof(srv_nullvarname));
    memset(&var_noname, 0, sizeof(var_noname));
    var_noname.name = NULL;
    srv_nullvarname.variables = &var_noname;
    srv_nullvarname.n_variables = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              find_server_variable(&srv_nullvarname, "v", &sv_out));
    ASSERT(sv_out == NULL);
  }

  {
    ASSERT_EQ(CDD_C_SUCCESS, render_server_url_default(NULL, &out));
    ASSERT(out == NULL);
    {
      struct OpenAPI_Server srv_nourl;
      memset(&srv_nourl, 0, sizeof(srv_nourl));
      ASSERT_EQ(CDD_C_SUCCESS, render_server_url_default(&srv_nourl, &out));
      ASSERT(out == NULL);
    }
    {
      struct OpenAPI_Server srv_novarval;
      struct OpenAPI_ServerVariable var_nodef;
      memset(&srv_novarval, 0, sizeof(srv_novarval));
      memset(&var_nodef, 0, sizeof(var_nodef));
      var_nodef.name = (char *)(size_t)(size_t) "v";
      var_nodef.default_value = NULL;
      srv_novarval.url = (char *)(size_t)(size_t) "http://api.com/{v}";
      srv_novarval.variables = &var_nodef;
      srv_novarval.n_variables = 1;
      ASSERT_EQ(CDD_C_SUCCESS, render_server_url_default(&srv_novarval, &out));
      ASSERT(out == NULL);
    }
  }

  {
    struct OpenAPI_Server *s_out = NULL;
    struct OpenAPI_Operation op_empty;
    struct OpenAPI_Path path_empty;
    struct OpenAPI_Server srv_dummy;
    memset(&op_empty, 0, sizeof(op_empty));
    memset(&path_empty, 0, sizeof(path_empty));
    ASSERT_EQ(CDD_C_SUCCESS, select_operation_server(NULL, NULL, &s_out));
    ASSERT(s_out == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              select_operation_server(&path_empty, &op_empty, &s_out));
    ASSERT(s_out == NULL);

    op_empty.servers = &srv_dummy;
    op_empty.n_servers = 0;
    ASSERT_EQ(CDD_C_SUCCESS, select_operation_server(NULL, &op_empty, &s_out));
    ASSERT(s_out == NULL);
  }

  {
    ASSERT_EQ(CDD_C_SUCCESS, sanitize_tag("", &out));
    ASSERT(out != NULL);
    ASSERT_STR_EQ("", out);
    free(out);
    out = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, sanitize_tag("Upper", &out));
    ASSERT(out != NULL);
    ASSERT_STR_EQ("Upper", out);
    free(out);
    out = NULL;
  }

  {
    struct OpenAPI_Parameter p_a, p_b;
    memset(&p_a, 0, sizeof(p_a));
    memset(&p_b, 0, sizeof(p_b));
    ASSERT_EQ(CDD_C_SUCCESS, param_keys_match(NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, param_keys_match(&p_a, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, param_keys_match(NULL, &p_b));
    ASSERT_EQ(CDD_C_SUCCESS, param_keys_match(&p_a, &p_b));
  }

  {
    struct OpenAPI_Parameter *p_out = NULL;
    size_t p_cnt = 0;
    struct OpenAPI_Path p_empty;
    struct OpenAPI_Operation op_with_p;
    struct OpenAPI_Parameter p_single;
    memset(&p_empty, 0, sizeof(p_empty));
    memset(&op_with_p, 0, sizeof(op_with_p));
    memset(&p_single, 0, sizeof(p_single));
    ASSERT_EQ(CDD_C_SUCCESS,
              build_effective_parameters(&p_empty, NULL, &p_out, &p_cnt));
    ASSERT_EQ(0, p_cnt);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              build_effective_parameters(NULL, NULL, NULL, NULL));

    op_with_p.parameters = &p_single;
    op_with_p.n_parameters = 1;
    p_empty.parameters = NULL;
    p_empty.n_parameters = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              build_effective_parameters(&p_empty, &op_with_p, &p_out, &p_cnt));
    ASSERT_EQ(1, p_cnt);
    free(p_out);
  }

  {
    FILE *devnull = cdd_test_tmpfile_global();
    ASSERT_EQ(CDD_C_SUCCESS, write_header_preamble(devnull, "MY_GUARD", NULL));
    fclose(devnull);
  }

  {
    struct OpenAPI_Operation doc_op;
    struct OpenAPI_Path doc_path;
    struct OpenAPI_Callback doc_cb;
    struct OpenAPI_Response doc_resp;
    struct OpenAPI_Link doc_link;
    struct OpenAPI_SecurityRequirementSet doc_sec;
    struct OpenAPI_Parameter doc_params[2];
    FILE *devnull = cdd_test_tmpfile_global();

    memset(&doc_op, 0, sizeof(doc_op));
    memset(&doc_path, 0, sizeof(doc_path));
    memset(&doc_cb, 0, sizeof(doc_cb));
    memset(&doc_resp, 0, sizeof(doc_resp));
    memset(&doc_link, 0, sizeof(doc_link));
    memset(&doc_sec, 0, sizeof(doc_sec));
    memset(doc_params, 0, sizeof(doc_params));

    doc_path.route = (char *)(size_t) "/doc_test";
    doc_op.summary = (char *)(size_t) "Summary";
    doc_op.description = (char *)(size_t) "Description";
    doc_op.operation_id = (char *)(size_t) "docOp";
    doc_op.external_docs.url = (char *)(size_t) "http://example.com/docs";
    doc_op.callbacks = &doc_cb;
    doc_resp.links = &doc_link;
    doc_op.responses = &doc_resp;
    doc_op.n_responses = 1;
    doc_op.security = &doc_sec;
    doc_op.deprecated = 1;

    doc_params[0].name = (char *)(size_t) "p0";
    doc_params[0].in = OA_PARAM_IN_QUERY;
    doc_params[0].allow_empty_value = 1;
    doc_params[0].allow_reserved = 1;

    doc_params[1].name = (char *)(size_t) "p1";
    doc_params[1].in = OA_PARAM_IN_UNKNOWN;

    doc_op.parameters = doc_params;
    doc_op.n_parameters = 2;

    ASSERT_EQ(CDD_C_SUCCESS, write_docblock(devnull, &doc_path, &doc_op));
    fclose(devnull);
  }

  {
    struct OpenAPI_Spec spec_srv;
    struct OpenAPI_Server srv_nourl[1];
    FILE *h_devnull = cdd_test_tmpfile_global();
    FILE *c_devnull = cdd_test_tmpfile_global();

    memset(&spec_srv, 0, sizeof(spec_srv));
    memset(srv_nourl, 0, sizeof(srv_nourl));
    spec_srv.servers = srv_nourl;
    spec_srv.n_servers = 1;
    srv_nourl[0].url = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              write_lifecycle_funcs(h_devnull, c_devnull, "test_", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              write_lifecycle_funcs(h_devnull, c_devnull, "test_", &spec_srv));

    spec_srv.n_servers = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              write_lifecycle_funcs(h_devnull, c_devnull, "test_", &spec_srv));

    /* unclosed variable -> default_url is NULL */
    spec_srv.n_servers = 1;
    srv_nourl[0].url = (char *)(size_t) "http://api.com/{unclosed";
    ASSERT_EQ(CDD_C_SUCCESS,
              write_lifecycle_funcs(h_devnull, c_devnull, "test_", &spec_srv));

    fclose(h_devnull);
    fclose(c_devnull);
  }

  {
    FILE *devnull = cdd_test_tmpfile_global();
    struct OpenAPI_Operation doc_op2;
    struct OpenAPI_Parameter doc_p;
    struct OpenAPI_Response r_nocode;
    struct OpenAPI_Path path_noroute;
    memset(&doc_op2, 0, sizeof(doc_op2));
    memset(&doc_p, 0, sizeof(doc_p));
    memset(&r_nocode, 0, sizeof(r_nocode));
    memset(&path_noroute, 0, sizeof(path_noroute));

    /* 1. summary == NULL, operation_id != NULL */
    doc_op2.operation_id = (char *)(size_t) "myOp";
    ASSERT_EQ(CDD_C_SUCCESS, write_docblock(devnull, NULL, &doc_op2));

    /* 2. summary == NULL, operation_id == NULL */
    doc_op2.operation_id = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, write_docblock(devnull, NULL, &doc_op2));

    /* 3. parameter name == NULL */
    doc_p.name = NULL;
    doc_op2.parameters = &doc_p;
    doc_op2.n_parameters = 1;
    ASSERT_EQ(CDD_C_SUCCESS, write_docblock(devnull, NULL, &doc_op2));

    /* 4. responses == NULL */
    doc_op2.parameters = NULL;
    doc_op2.n_parameters = 0;
    doc_op2.responses = NULL;
    doc_op2.n_responses = 0;
    ASSERT_EQ(CDD_C_SUCCESS, write_docblock(devnull, NULL, &doc_op2));

    /* 5. response code == NULL */
    r_nocode.code = NULL;
    doc_op2.responses = &r_nocode;
    doc_op2.n_responses = 1;
    ASSERT_EQ(CDD_C_SUCCESS, write_docblock(devnull, NULL, &doc_op2));

    /* 6. path without route */
    path_noroute.route = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, write_docblock(devnull, &path_noroute, &doc_op2));

    /* 7. IO fail on unnamed operation */
    doc_op2.summary = NULL;
    doc_op2.operation_id = NULL;
    g_io_calls = 0;
    g_fail_io_after = 1;
    write_docblock(devnull, NULL, &doc_op2);
    g_fail_io_after = -1;

    /* 8. IO fail on operation with operation_id */
    doc_op2.summary = NULL;
    doc_op2.operation_id = (char *)(size_t) "myOp";
    g_io_calls = 0;
    g_fail_io_after = 1;
    write_docblock(devnull, NULL, &doc_op2);
    g_fail_io_after = -1;

    fclose(devnull);
  }

  {
    struct OpenAPI_Path p_nosrv;
    struct OpenAPI_Server srv_dummy;
    struct OpenAPI_Server *s_out = NULL;
    memset(&p_nosrv, 0, sizeof(p_nosrv));
    p_nosrv.servers = &srv_dummy;
    p_nosrv.n_servers = 0;
    ASSERT_EQ(CDD_C_SUCCESS, select_operation_server(&p_nosrv, NULL, &s_out));
    ASSERT(s_out == NULL);
  }

  {
    struct OpenAPI_Parameter *p_out = NULL;
    size_t p_cnt = 0;
    struct OpenAPI_Operation op_with_p;
    struct OpenAPI_Parameter p_single;
    memset(&op_with_p, 0, sizeof(op_with_p));
    memset(&p_single, 0, sizeof(p_single));
    op_with_p.parameters = &p_single;
    op_with_p.n_parameters = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              build_effective_parameters(NULL, &op_with_p, &p_out, &p_cnt));
    ASSERT_EQ(1, p_cnt);
    free(p_out);
  }

  /* openapi_client_generate mocks */
  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  memset(&config, 0, sizeof(config));
  config.filename_base = (char *)(size_t)(size_t) "build/test_out/test_cg_mock";
  config.no_installable_package = 1;

  /* Setup schemas */
  memset(sf, 0, sizeof(sf));
  names[0] = (char *)(size_t)(size_t) "Enum1";
  sf[0].is_enum = 1;
  names[1] = (char *)(size_t)(size_t) "Union1";
  sf[1].is_union = 1;
  names[2] = (char *)(size_t)(size_t) "Struct1";
  spec.defined_schemas = sf;
  spec.defined_schema_names = names;
  spec.n_defined_schemas = 3;

  /* mock 24: get_dirname fail */
  g_client_gen_fail = 24;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(&spec, &config));

  /* mock 25: get_basename fail */
  g_client_gen_fail = 25;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(&spec, &config));

  /* mock 10: src_dir fail */
  g_client_gen_fail = 10;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* mock 62: makedirs(src_dir) fail */
  g_client_gen_fail = 62;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 11: actual_base fail */
  g_client_gen_fail = 11;
  rc = openapi_client_generate(&spec, &config);
  ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);

  /* mock 61: strdup actual_base fail */
  g_client_gen_fail = 61;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* mock 12: filenames malloc fail */
  g_client_gen_fail = 12;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));
  g_client_gen_fail = 75;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));
  g_client_gen_fail = 76;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));
  g_client_gen_fail = 77;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* mock 16: file open fail */
  g_client_gen_fail = 16;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));
  g_client_gen_fail = 78;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));
  g_client_gen_fail = 79;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));
  g_client_gen_fail = 80;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 17: guard fail */
  g_client_gen_fail = 17;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));
  g_client_gen_fail = 81;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* mock 18: model_guard fail */
  g_client_gen_fail = 18;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* schema loop mocks 26..38 */
  g_client_gen_fail = 26;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 27;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 28;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 29;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 30;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 31;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 32;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 33;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 34;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 35;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 36;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 37;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 38;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 66;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 67;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 68;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 69;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 65: get_basename for mh_name fail */
  g_client_gen_fail = 65;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(&spec, &config));

  /* Test with a NULL schema name to test continue branch */
  names[0] = NULL;
  g_client_gen_fail = 0;
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  names[0] = (char *)(size_t)(size_t) "Enum1";

  /* mock 21: write_header_preamble fail */
  g_client_gen_fail = 21;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 53: get_basename(h_name) fail */
  g_client_gen_fail = 53;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(&spec, &config));

  /* mock 22: write_source_preamble fail */
  g_client_gen_fail = 22;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 23: write_lifecycle_funcs fail */
  g_client_gen_fail = 23;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* Test with config->header_guard and config->model_header explicitly set */
  config.header_guard = (char *)(size_t)(size_t) "CUSTOM_GUARD_H";
  config.model_header = (char *)(size_t)(size_t) "custom_models.h";
  g_client_gen_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));
  config.header_guard = NULL;
  config.model_header = NULL;

  /* mock 54: uh fopen fail */
  config.no_installable_package = 0;
  g_client_gen_fail = 54;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 84: client_gen_emit_url_utils_h fail */
  g_client_gen_fail = 84;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 82: client_gen_emit_url_utils_c1 fail */
  g_client_gen_fail = 82;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 83: client_gen_emit_url_utils_c2 fail */
  g_client_gen_fail = 83;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 55: uc fopen fail */
  g_client_gen_fail = 55;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 64: fputs fail */
  g_client_gen_fail = 64;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 63: fprintf fail */
  g_client_gen_fail = 63;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 20: generate_cmake_project fail */
  g_client_gen_fail = 20;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));
  config.no_installable_package = 1;

  /* mock 51: makedirs(tdir) fail */
  config.create_tests_and_mocks = 1;
  g_client_gen_fail = 51;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 52: fopen(tfile) fail */
  g_client_gen_fail = 52;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));
  config.create_tests_and_mocks = 0;

  /* mock 70: newline after forward decls */
  g_client_gen_fail = 70;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 71: emit_operation fail */
  g_client_gen_fail = 71;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 72: emit_operation additional fail */
  spec.paths[0].additional_operations = spec.paths[0].operations;
  spec.paths[0].n_additional_operations = 1;
  g_client_gen_fail = 72;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));
  spec.paths[0].additional_operations = NULL;
  spec.paths[0].n_additional_operations = 0;

  /* mock 74: endif guard fprintf fail */
  g_client_gen_fail = 74;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));
  g_client_gen_fail = 0;

  /* emit_operation mocks 40..46, 56..60 */
  {
    FILE *hf = cdd_test_tmpfile_global();
    FILE *cf = cdd_test_tmpfile_global();
    char *tag = (char *)(size_t)(size_t) "mytag";
    op.tags = &tag;
    op.n_tags = 1;
    config.namespace_prefix = (char *)(size_t)(size_t) "ns";

    g_client_gen_fail = 40;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 56;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 41;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 42;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    /* namespace only */
    op.n_tags = 0;
    g_client_gen_fail = 43;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    /* tag only */
    op.n_tags = 1;
    config.namespace_prefix = NULL;
    g_client_gen_fail = 44;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    /* base_url_expr fail with server override */
    op.servers = &srv;
    op.n_servers = 1;
    g_client_gen_fail = 45;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 60;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 57;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 58;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 59;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 46;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    /* emit_operation invalid args */
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        emit_operation(NULL, cf, spec.paths, &op, &spec, &config, "api_"));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        emit_operation(hf, NULL, spec.paths, &op, &spec, &config, "api_"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              emit_operation(hf, cf, NULL, &op, &spec, &config, "api_"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              emit_operation(hf, cf, spec.paths, NULL, &spec, &config, "api_"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              emit_operation(hf, cf, spec.paths, &op, &spec, NULL, "api_"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, NULL));

    /* tags[0] == NULL */
    {
      char *null_t = NULL;
      op.tags = &null_t;
      op.n_tags = 1;
      g_client_gen_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, emit_operation(hf, cf, spec.paths, &op, &spec,
                                              &config, "api_"));
      op.tags = NULL;
      op.n_tags = 0;
    }

    /* server_override with url == NULL */
    {
      struct OpenAPI_Server s_nourl;
      memset(&s_nourl, 0, sizeof(s_nourl));
      s_nourl.url = NULL;
      op.servers = &s_nourl;
      op.n_servers = 1;
      g_client_gen_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, emit_operation(hf, cf, spec.paths, &op, &spec,
                                              &config, "api_"));
    }

    /* server_override with url containing unclosed brace */
    {
      struct OpenAPI_Server s_unclosed;
      memset(&s_unclosed, 0, sizeof(s_unclosed));
      s_unclosed.url = (char *)(size_t) "http://api.com/{unclosed";
      op.servers = &s_unclosed;
      op.n_servers = 1;
      g_client_gen_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, emit_operation(hf, cf, spec.paths, &op, &spec,
                                              &config, "api_"));
    }

    if (hf)
      fclose(hf);
    if (cf)
      fclose(cf);
  }

  g_client_gen_fail = 0;
  remove("build/test_out/src/test_cg_mock.h");
  remove("build/test_out/src/test_cg_mock.c");
  remove("build/test_out/src/test_cg_mock_models.h");
  remove("build/test_out/src/test_cg_mock_models.c");
  remove("build/test_out/src/test/test_sdk.c");
  PASS();
}

SUITE(openapi_client_gen_mocks_suite) {
  RUN_TEST(test_client_gen_defined_schemas);
  RUN_TEST(test_client_gen_create_tests_mocks);
  RUN_TEST(test_client_gen_mock_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_CLIENT_GEN_MOCKS_H */
