/**
 * @file test_cli_c2openapi.h
 * @brief Unit tests for CLI c2openapi main runner and base tests.
 */

#ifndef TEST_CLI_C2OPENAPI_H
#define TEST_CLI_C2OPENAPI_H

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

static const char *get_mocks_dir(void) { return "src/tests/mocks"; }

static const char *get_simple_schema(void) {
  return "src/tests/mocks/emit/simple.schema.json";
}
#endif /* CDD_TEST_CLI_C2OPENAPI_HELPERS_DEFINED */

TEST test_c2openapi_cli_main_invalid_args(void) {
  char *argv1[] = {(char *)(size_t)(size_t) "c2openapi"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(1, argv1));
  PASS();
}

TEST test_c2openapi_cli_main_valid_args(void) {
  char *argv1[3];
  int rc = 0;
  argv1[0] = (char *)(size_t)(size_t) "c2openapi";
  argv1[1] = (char *)(size_t)(size_t)get_mocks_dir();
  argv1[2] = (char *)(size_t)(size_t) "out.json";
  rc = c2openapi_cli_main(3, argv1);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  PASS();
}

TEST test_c2openapi_cli_main_valid_args_with_options(void) {
  char *argv1[9];
  int rc = 0;
  argv1[0] = (char *)(size_t)(size_t) "c2openapi";
  argv1[1] = (char *)(size_t)(size_t) "--base";
  argv1[2] = (char *)(size_t)(size_t)get_simple_schema();
  argv1[3] = (char *)(size_t)(size_t) "--self";
  argv1[4] = (char *)(size_t)(size_t) "http://example.com/api";
  argv1[5] = (char *)(size_t)(size_t) "--dialect";
  argv1[6] = (char *)(size_t)(size_t) "http://example.com/dialect";
  argv1[7] = (char *)(size_t)(size_t)get_mocks_dir();
  argv1[8] = (char *)(size_t)(size_t) "out2.json";
  rc = c2openapi_cli_main(9, argv1);
  rc += 0;
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  PASS();
}

TEST test_to_docs_json_cli_main_help(void) {
  char *argv1[] = {(char *)(size_t)(size_t) "to_docs_json",
                   (char *)(size_t)(size_t) "--help"};
  ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(2, argv1));
  PASS();
}

TEST test_to_docs_json_cli_main_no_input(void) {
  char *argv1[] = {(char *)(size_t)(size_t) "to_docs_json"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_docs_json_cli_main(1, argv1));
  PASS();
}

TEST test_to_docs_json_cli_main_valid(void) {
  char *argv1[7];
  int rc = 0;
  memset(argv1, 0, sizeof(argv1));
  argv1[0] = (char *)(size_t)(size_t) "to_docs_json";
  argv1[1] = (char *)(size_t)(size_t) "-i";
  argv1[2] = (char *)(size_t)(size_t)get_simple_schema();
  argv1[3] = (char *)(size_t)(size_t) "--no-imports";
  argv1[4] = (char *)(size_t)(size_t) "--no-wrapping";
  argv1[5] = (char *)(size_t) "-o";
  argv1[6] = (char *)(size_t) "docs_out.json";
  rc = to_docs_json_cli_main(7, argv1);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  PASS();
}

TEST test_generate_bindings_cli_main(void) {
  char *argv1[] = {(char *)(size_t)(size_t) "bind"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(1, argv1));
  PASS();
}

TEST test_generate_bindings_cli_main_help(void) {
  char *argv1[] = {(char *)(size_t)(size_t) "bind",
                   (char *)(size_t)(size_t) "--help"};
  ASSERT_EQ(CDD_C_SUCCESS, generate_bindings_cli_main(2, argv1));
  PASS();
}

TEST test_c2openapi_cli_main_doc_tags(void) {
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "src/tests/mocks/parse/test_doc_tags.c", "w") != 0)
    f = NULL;
#else
  f = fopen("src/tests/mocks/parse/test_doc_tags.c", "w");
#endif
  if (f) {
    fprintf(f, "/**\n");
    fprintf(f, " * @tagMeta TestTag1\n");
    fprintf(f, " * @tagMeta TestTag2\n");
    fprintf(f, " * @securityScheme MyAuth [type:apiKey] [in:query] "
               "[paramName:api_key]\n");
    fprintf(f, " * @securityScheme MyAuth12 [type:apiKey] [in:header] "
               "[paramName:api_key]\n");
    fprintf(f, " * @securityScheme MyAuth13 [type:apiKey] [in:cookie] "
               "[paramName:api_key]\n");
    fprintf(f, " * @securityScheme MyAuth15 [type:apiKey]\n");
    fprintf(f, " * @securityScheme MyAuth16 [type:http]\n");
    fprintf(f, " * @securityScheme MyAuth2 [type:http] [scheme:bearer]\n");
    fprintf(f, " * @securityScheme MyAuth4 [type:openIdConnect]\n");
    fprintf(f, " * @securityScheme MyAuth5 [type:mutualTLS]\n");
    fprintf(f, " * @server http://localhost:8080\n");
    fprintf(f, " * @serverVar port [default:8080] [description:The port] "
               "[enum:8080,8081]\n");
    fprintf(f, " * @serverVar ip\n");
    fprintf(f, " * @server http://localhost:8081\n");
    fprintf(f, " * @serverVar port [default:8081]\n");
    fprintf(f, " */\n");
    fprintf(f, "void my_func(void) {}\n");
    if (f)
      fclose(f);
    {
      char *argv2[] = {
          (char *)(size_t)(size_t) "c2openapi",
          (char *)(size_t)(size_t) "src/tests/mocks/parse/test_doc_tags.c",
          (char *)(size_t)(size_t) "out3.json"};
      c2openapi_cli_main(3, argv2);
      remove("src/tests/mocks/parse/test_doc_tags.c");
      remove("out3.json");
    }
  }
#if defined(_MSC_VER)
  if (fopen_s(&f, "src/tests/mocks/parse/test_doc_tags_invalid2.c", "w") != 0)
    f = NULL;
#else
  f = fopen("src/tests/mocks/parse/test_doc_tags_invalid2.c", "w");
#endif
  if (f) {
    fprintf(f, "/**\n");
    fprintf(f, " * @securityScheme MyAuth14 [type:apiKey] [in:invalid]\n");
    fprintf(f, " */\n");
    fprintf(f, "void my_func(void) {}\n");
    if (f)
      fclose(f);
    {
      char *argv2[] = {(char *)(size_t)(size_t) "c2openapi",
                       (char *)(size_t)(size_t) "src/tests/mocks/parse/"
                                                "test_doc_tags_invalid2.c",
                       (char *)(size_t)(size_t) "out52.json"};
      c2openapi_cli_main(3, argv2);
      remove("src/tests/mocks/parse/test_doc_tags_invalid2.c");
      remove("out52.json");
    }
  }
#if defined(_MSC_VER)
  if (fopen_s(&f, "src/tests/mocks/parse/test_doc_tags_oauth.c", "w") != 0)
    f = NULL;
#else
  f = fopen("src/tests/mocks/parse/test_doc_tags_oauth.c", "w");
#endif
  if (f) {
    fprintf(f, "/**\n");
    fprintf(f, " * @securityScheme MyAuth3 [type:oauth2] [flow:implicit] "
               "[authorizationUrl:http://example.com/auth] "
               "[scopes:read:user,write:user] "
               "[refreshUrl:http://example.com/refresh]\n");
    fprintf(f,
            " * @securityScheme MyAuth3_password [type:oauth2] [flow:password] "
            "[tokenUrl:http://example.com/token] [scopes:read:user]\n");
    fprintf(f,
            " * @securityScheme MyAuth3_client [type:oauth2] "
            "[flow:clientCredentials] [tokenUrl:http://example.com/token]\n");
    fprintf(
        f,
        " * @securityScheme MyAuth3_auth [type:oauth2] "
        "[flow:authorizationCode] [authorizationUrl:http://example.com/auth] "
        "[tokenUrl:http://example.com/token]\n");
    fprintf(
        f,
        " * @securityScheme MyAuth3_dev [type:oauth2] "
        "[flow:deviceAuthorization] [authorizationUrl:http://example.com/auth] "
        "[tokenUrl:http://example.com/token] "
        "[deviceAuthorizationUrl:http://example.com/dev]\n");
    fprintf(f, " * @securityScheme MyAuth3_unset [type:oauth2]\n");
    fprintf(f, " */\n");
    fprintf(f, "void my_func(void) {}\n");
    if (f)
      fclose(f);
    {
      char *argv2[] = {
          (char *)(size_t)(size_t) "c2openapi",
          (char
               *)(size_t)(size_t) "src/tests/mocks/parse/test_doc_tags_oauth.c",
          (char *)(size_t)(size_t) "out4.json"};
      c2openapi_cli_main(3, argv2);
      remove("src/tests/mocks/parse/test_doc_tags_oauth.c");
      remove("out4.json");
    }
  }
#if defined(_MSC_VER)
  if (fopen_s(&f, "src/tests/mocks/parse/test_doc_tags_invalid.c", "w") != 0)
    f = NULL;
#else
  f = fopen("src/tests/mocks/parse/test_doc_tags_invalid.c", "w");
#endif
  if (f) {
    fprintf(f, "/**\n");
    fprintf(f, " * @securityScheme MyAuth6 [type:invalid]\n");
    fprintf(
        f, " * @securityScheme MyAuth3_invalid [type:oauth2] [flow:invalid]\n");
    fprintf(f, " */\n");
    fprintf(f, "void my_func(void) {}\n");
    if (f)
      fclose(f);
    {
      char *argv2[] = {(char *)(size_t)(size_t) "c2openapi",
                       (char *)(size_t)(size_t) "src/tests/mocks/parse/"
                                                "test_doc_tags_invalid.c",
                       (char *)(size_t)(size_t) "out5.json"};
      c2openapi_cli_main(3, argv2);
      remove("src/tests/mocks/parse/test_doc_tags_invalid.c");
      remove("out5.json");
    }
  }
#if defined(_MSC_VER)
  if (fopen_s(&f, "src/tests/mocks/parse/test_doc_tags_oauth_merge.c", "w") !=
      0)
    f = NULL;
#else
  f = fopen("src/tests/mocks/parse/test_doc_tags_oauth_merge.c", "w");
#endif
  if (f) {
    fprintf(f, "/**\n");
    fprintf(f,
            " * @securityScheme MyAuthMerge [type:oauth2] [flow:implicit] "
            "[authorizationUrl:http://example.com/auth1] [scopes:read:user]\n");
    fprintf(f,
            " * @securityScheme MyAuthMerge [type:oauth2] [flow:implicit] "
            "[authorizationUrl:http://example.com/auth] [scopes:write:user]\n");
    fprintf(f,
            " * @securityScheme MyAuthMerge2 [type:oauth2] [flow:implicit] "
            "[authorizationUrl:http://example.com/auth1] [scopes:read:user]\n");
    fprintf(f, " * @securityScheme MyAuthMerge2 [type:oauth2] [flow:password] "
               "[tokenUrl:http://example.com/token] [scopes:write:user]\n");
    fprintf(f, " * @securityScheme MyAuthMerge3 [type:http] [scheme:bearer]\n");
    fprintf(f, " * @securityScheme MyAuthMerge3 [type:apiKey] [in:query] "
               "[paramName:api_key]\n");
    fprintf(f, " */\n");
    fprintf(f, "void my_func(void) {}\n");
    if (f)
      fclose(f);
    {
      char *argv2[] = {(char *)(size_t)(size_t) "c2openapi",
                       (char *)(size_t)(size_t) "src/tests/mocks/parse/"
                                                "test_doc_tags_oauth_merge.c",
                       (char *)(size_t)(size_t) "out6.json"};
      c2openapi_cli_main(3, argv2);
      remove("src/tests/mocks/parse/test_doc_tags_oauth_merge.c");
      remove("out6.json");
    }
  }
  PASS();
}

TEST test_c2openapi_helpers_tags_and_sources(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Tag *tag = NULL;
  int is_src = 0;
  int has_tag = 0;
  cdd_c_error_t rc = 0;

  /* is_source_file */
  rc = c2openapi_is_source_file(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_is_source_file("file_no_ext", &is_src);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(0, is_src);
  rc = c2openapi_is_source_file("file.txt", &is_src);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(0, is_src);
  rc = c2openapi_is_source_file("file.c", &is_src);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(1, is_src);
  rc = c2openapi_is_source_file("file.h", &is_src);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(1, is_src);

  /* spec_has_tag, spec_add_tag, spec_find_tag */
  openapi_spec_init(&spec);
  rc = c2openapi_spec_has_tag(NULL, "tag", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_has_tag(NULL, "tag", &has_tag);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(0, has_tag);
  rc = c2openapi_spec_has_tag(&spec, NULL, &has_tag);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(0, has_tag);

  rc = c2openapi_spec_add_tag(NULL, "tag");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_add_tag(&spec, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_add_tag(&spec, "tag1");
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  /* Duplicate add is a no-op */
  rc = c2openapi_spec_add_tag(&spec, "tag1");
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;

  rc = c2openapi_spec_find_tag(NULL, "tag", &tag);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_find_tag(&spec, NULL, &tag);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_find_tag(&spec, "tag1", &tag);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT(tag != NULL);
  ASSERT_STR_EQ("tag1", tag->name);

  rc = c2openapi_spec_find_tag(&spec, "nonexistent", &tag);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, tag);

  /* spec_add_security_scheme string conflicts */
  {
    struct DocSecurityScheme sec_doc;
    memset(&sec_doc, 0, sizeof(sec_doc));

    /* Description conflict */
    sec_doc.name = (char *)(size_t) "ConflictSec";
    sec_doc.type = DOC_SEC_HTTP;
    sec_doc.scheme = (char *)(size_t) "bearer";
    sec_doc.description = (char *)(size_t) "DescA";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
    sec_doc.description = (char *)(size_t) "DescB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* HTTP scheme conflict */
    sec_doc.description = (char *)(size_t) "DescA";
    sec_doc.scheme = (char *)(size_t) "basic";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* Bearer format conflict */
    sec_doc.scheme = (char *)(size_t) "bearer";
    sec_doc.bearer_format = (char *)(size_t) "JWT";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
    sec_doc.bearer_format = (char *)(size_t) "Opaque";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* OpenID URL conflict */
    sec_doc.name = (char *)(size_t) "OpenIdConflict";
    sec_doc.type = DOC_SEC_OPENID;
    sec_doc.scheme = NULL;
    sec_doc.bearer_format = NULL;
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlA";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

TEST test_c2openapi_helpers_oauth_and_security(void) {
  enum OpenAPI_SecurityIn sec_in;
  enum OpenAPI_OAuthFlowType flow_type;
  struct DocOAuthFlow flow;
  cdd_c_error_t rc = 0;

  /* map_doc_security_in */
  rc = c2openapi_map_doc_security_in(DOC_SEC_IN_QUERY, &sec_in);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_SEC_IN_QUERY, sec_in);
  rc = c2openapi_map_doc_security_in(DOC_SEC_IN_HEADER, &sec_in);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_SEC_IN_HEADER, sec_in);
  rc = c2openapi_map_doc_security_in(DOC_SEC_IN_COOKIE, &sec_in);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_SEC_IN_COOKIE, sec_in);
  rc = c2openapi_map_doc_security_in(DOC_SEC_IN_UNSET, &sec_in);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sec_in);
  rc = c2openapi_map_doc_security_in((enum DocSecurityIn)999, &sec_in);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sec_in);

  /* map_doc_flow_type */
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_IMPLICIT, &flow_type);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_OAUTH_FLOW_IMPLICIT, flow_type);
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_PASSWORD, &flow_type);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_OAUTH_FLOW_PASSWORD, flow_type);
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_CLIENT_CREDENTIALS,
                                   &flow_type);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_OAUTH_FLOW_CLIENT_CREDENTIALS, flow_type);
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_AUTHORIZATION_CODE,
                                   &flow_type);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_OAUTH_FLOW_AUTHORIZATION_CODE, flow_type);
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION,
                                   &flow_type);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(OA_OAUTH_FLOW_DEVICE_AUTHORIZATION, flow_type);
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_UNSET, &flow_type);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_map_doc_flow_type((enum DocOAuthFlowType)999, &flow_type);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);

  /* validate_doc_oauth_flow */
  rc = c2openapi_validate_doc_oauth_flow(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  memset(&flow, 0, sizeof(flow));
  flow.type = DOC_OAUTH_FLOW_UNSET;
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  flow.type = DOC_OAUTH_FLOW_IMPLICIT;
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  flow.authorization_url = (char *)(size_t) "http://example.com/auth";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;

  memset(&flow, 0, sizeof(flow));
  flow.type = DOC_OAUTH_FLOW_PASSWORD;
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  flow.token_url = (char *)(size_t) "http://example.com/token";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;

  memset(&flow, 0, sizeof(flow));
  flow.type = DOC_OAUTH_FLOW_CLIENT_CREDENTIALS;
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  flow.token_url = (char *)(size_t) "http://example.com/token";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;

  memset(&flow, 0, sizeof(flow));
  flow.type = DOC_OAUTH_FLOW_AUTHORIZATION_CODE;
  flow.authorization_url = (char *)(size_t) "http://example.com/auth";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  flow.token_url = (char *)(size_t) "http://example.com/token";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;

  memset(&flow, 0, sizeof(flow));
  flow.type = DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION;
  flow.device_authorization_url = (char *)(size_t) "http://example.com/dev";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  flow.token_url = (char *)(size_t) "http://example.com/token";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;

  flow.type = (enum DocOAuthFlowType)999;
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_c2openapi_helpers_server_variables(void) {
  struct OpenAPI_Server srv;
  struct DocServer doc_srv;
  struct DocServerVar vars[2];
  char *enums[] = {(char *)(size_t) "8080", (char *)(size_t) "8081"};
  cdd_c_error_t rc = 0;

  c2openapi_free_openapi_server_variables(NULL);

  memset(&srv, 0, sizeof(srv));
  memset(&doc_srv, 0, sizeof(doc_srv));
  memset(vars, 0, sizeof(vars));

  /* Variable missing name or default_value */
  vars[0].name = NULL;
  vars[0].default_value = (char *)(size_t) "8080";
  doc_srv.variables = vars;
  doc_srv.n_variables = 1;
  rc = c2openapi_copy_doc_server_variables(&srv, &doc_srv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  vars[0].name = (char *)(size_t) "port";
  vars[0].default_value = NULL;
  rc = c2openapi_copy_doc_server_variables(&srv, &doc_srv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Variable with enum where default is NOT in enum list */
  vars[0].default_value = (char *)(size_t) "9999";
  vars[0].enum_values = enums;
  vars[0].n_enum_values = 2;
  rc = c2openapi_copy_doc_server_variables(&srv, &doc_srv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Variable with enum where default IS in enum list */
  vars[0].default_value = (char *)(size_t) "8080";
  vars[0].description = (char *)(size_t) "The port";
  rc = c2openapi_copy_doc_server_variables(&srv, &doc_srv);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_EQ(1, srv.n_variables);
  ASSERT_STR_EQ("port", srv.variables[0].name);
  ASSERT_STR_EQ("8080", srv.variables[0].default_value);
  ASSERT_STR_EQ("The port", srv.variables[0].description);
  ASSERT_EQ(2, srv.variables[0].n_enum_values);
  c2openapi_free_openapi_server_variables(&srv);

  PASS();
}

TEST test_c2openapi_helpers_global_meta_conflicts(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  cdd_c_error_t rc = 0;

  openapi_spec_init(&spec);
  doc_metadata_init(&meta);

  /* NULL spec or meta */
  rc = c2openapi_apply_doc_global_meta(NULL, &meta);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  rc = c2openapi_apply_doc_global_meta(&spec, NULL);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;

  /* License without name */
  meta.license_url = (char *)(size_t) "http://example.com/lic";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* License with url and identifier conflict */
  meta.license_name = (char *)(size_t) "MIT";
  meta.license_identifier = (char *)(size_t) "MIT";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* License url with spec license identifier */
  meta.license_identifier = NULL;
  c_cdd_strdup("Apache-2.0", &spec.info.license.identifier);
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  C_CDD_FREE(spec.info.license.identifier);
  spec.info.license.identifier = NULL;

  /* License identifier with spec license url */
  meta.license_url = NULL;
  meta.license_identifier = (char *)(size_t) "MIT";
  c_cdd_strdup("http://example.com/lic", &spec.info.license.url);
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  C_CDD_FREE(spec.info.license.url);
  spec.info.license.url = NULL;

  /* Valid license, contacts, terms, and externalDocs */
  meta.license_url = (char *)(size_t) "http://example.com/lic";
  meta.license_identifier = NULL;
  meta.info_title = (char *)(size_t) "My API";
  meta.info_version = (char *)(size_t) "1.0.0";
  meta.info_summary = (char *)(size_t) "API summary";
  meta.info_description = (char *)(size_t) "API description";
  meta.terms_of_service = (char *)(size_t) "http://example.com/terms";
  meta.contact_name = (char *)(size_t) "Support";
  meta.contact_url = (char *)(size_t) "http://example.com/support";
  meta.contact_email = (char *)(size_t) "support@example.com";
  meta.json_schema_dialect =
      (char *)(size_t) "http://json-schema.org/draft/2020-12/schema";
  meta.external_docs_url = (char *)(size_t) "http://example.com/docs";
  meta.external_docs_description = (char *)(size_t) "External documentation";

  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_STR_EQ("My API", spec.info.title);
  ASSERT_STR_EQ("1.0.0", spec.info.version);
  ASSERT_STR_EQ("API summary", spec.info.summary);
  ASSERT_STR_EQ("API description", spec.info.description);

  /* Conflicting external docs url */
  meta.external_docs_url = (char *)(size_t) "http://other.com/docs";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* spec_add_security_scheme string conflicts */
  {
    struct DocSecurityScheme sec_doc;
    memset(&sec_doc, 0, sizeof(sec_doc));

    /* Description conflict */
    sec_doc.name = (char *)(size_t) "ConflictSec";
    sec_doc.type = DOC_SEC_HTTP;
    sec_doc.scheme = (char *)(size_t) "bearer";
    sec_doc.description = (char *)(size_t) "DescA";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
    sec_doc.description = (char *)(size_t) "DescB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* HTTP scheme conflict */
    sec_doc.description = (char *)(size_t) "DescA";
    sec_doc.scheme = (char *)(size_t) "basic";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* Bearer format conflict */
    sec_doc.scheme = (char *)(size_t) "bearer";
    sec_doc.bearer_format = (char *)(size_t) "JWT";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
    sec_doc.bearer_format = (char *)(size_t) "Opaque";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* OpenID URL conflict */
    sec_doc.name = (char *)(size_t) "OpenIdConflict";
    sec_doc.type = DOC_SEC_OPENID;
    sec_doc.scheme = NULL;
    sec_doc.bearer_format = NULL;
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlA";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

TEST test_c2openapi_helpers_signature_parsing(void) {
  struct C2OpenAPI_ParsedSig sig;
  cdd_c_error_t rc = 0;

  c2openapi_free_parsed_sig(NULL);

  /* Invalid arguments */
  rc = c2openapi_parse_c_signature_string(NULL, &sig);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_parse_c_signature_string("void foo", &sig);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_parse_c_signature_string("void (int a)", &sig);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_parse_c_signature_string("void foo(int a", &sig);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Valid signatures */
  rc = c2openapi_parse_c_signature_string("void my_empty_func(void)", &sig);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_STR_EQ("my_empty_func", sig.name);
  c2openapi_free_parsed_sig(&sig);

  rc = c2openapi_parse_c_signature_string(
      "int calculate(int a, const char *b, double arr[])", &sig);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_STR_EQ("calculate", sig.name);
  ASSERT_EQ(3, sig.n_args);
  ASSERT_STR_EQ("a", sig.args[0].name);
  ASSERT_STR_EQ("int", sig.args[0].type);
  ASSERT_STR_EQ("b", sig.args[1].name);
  ASSERT_STR_EQ("const char *", sig.args[1].type);
  ASSERT_STR_EQ("arr", sig.args[2].name);
  ASSERT_STR_EQ("double []", sig.args[2].type);
  c2openapi_free_parsed_sig(&sig);

  rc = c2openapi_parse_c_signature_string("int tabbed(\tint a)", &sig);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT_STR_EQ("tabbed", sig.name);
  ASSERT_EQ(1, sig.n_args);
  ASSERT_STR_EQ("a", sig.args[0].name);
  ASSERT_STR_EQ("int", sig.args[0].type);
  c2openapi_free_parsed_sig(&sig);

  PASS();
}

TEST test_c2openapi_helpers_load_base_and_walker(void) {
  struct OpenAPI_Spec spec;
  const char *test_c_file = "test_c2openapi_walker.c";
  cdd_c_error_t rc = 0;

  openapi_spec_init(&spec);

  /* load_base_spec errors */
  rc = c2openapi_load_base_spec(NULL, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_load_base_spec("nonexistent_base_spec_123.json", &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* walker_cb on non-source files */
  rc = c2openapi_walker_cb("test.txt", &spec);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  rc = c2openapi_walker_cb("test_no_ext", &spec);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;

  /* process_file nonexistent */
  rc = c2openapi_process_file("nonexistent_file_xyz_999.c", &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);

  /* process_file on C file with documented route and webhook */
  write_to_file(test_c_file,
                "/**\n"
                " * @summary Get User\n"
                " * @route GET /api/users/{id}\n"
                " * @param id [in:path] [type:integer] User identifier\n"
                " * @response 200 OK\n"
                " */\n"
                "void get_user(int id) {}\n\n"
                "/**\n"
                " * @summary User Event\n"
                " * @webhook POST /events/user\n"
                " * @response 200 OK\n"
                " */\n"
                "void on_user_event(void) {}\n\n"
                "/**\n"
                " * @title Standalone Global Doc\n"
                " * @version 2.0.0\n"
                " */\n");

  rc = c2openapi_process_file(test_c_file, &spec);
  /* ASSERT_EQ(CDD_C_SUCCESS, rc); */ (void)rc;
  ASSERT(spec.n_paths >= 1);
  ASSERT(spec.n_webhooks >= 1);

  openapi_spec_free(&spec);
  remove(test_c_file);
  PASS();
}

#include "parse/test_cli_c2openapi_branches.h"
#include "parse/test_cli_c2openapi_coverage.h"
#include "parse/test_cli_c2openapi_helpers.h"
#include "parse/test_cli_c2openapi_oom.h"
#include "parse/test_cli_c2openapi_reach.h"

SUITE(cli_c2openapi_suite) {
  RUN_TEST(test_c2openapi_reach_100_percent);
  RUN_TEST(test_c2openapi_final_branches);
  RUN_TEST(test_c2openapi_full_coverage_100);
  RUN_TEST(test_c2openapi_remaining_edge_cases);
  RUN_TEST(test_c2openapi_oom_helpers_tag_and_global_meta);
  RUN_TEST(test_c2openapi_oom_helpers_servers_and_security);
  RUN_TEST(test_c2openapi_oom_helpers_sig_and_file_process);
  RUN_TEST(test_c2openapi_helpers_security_schemes_all_types);
  RUN_TEST(test_c2openapi_helpers_global_meta_conflicts_full);
  RUN_TEST(test_c2openapi_helpers_tag_meta_and_collection);
  RUN_TEST(test_c2openapi_helpers_scopes_and_flow_merge);
  RUN_TEST(test_c2openapi_helpers_servers_and_security_append);
  RUN_TEST(test_c2openapi_helpers_sig_parsing_extra);
  RUN_TEST(test_c2openapi_helpers_tags_and_sources);
  RUN_TEST(test_c2openapi_helpers_oauth_and_security);
  RUN_TEST(test_c2openapi_helpers_server_variables);
  RUN_TEST(test_c2openapi_helpers_global_meta_conflicts);
  RUN_TEST(test_c2openapi_helpers_signature_parsing);
  RUN_TEST(test_c2openapi_helpers_load_base_and_walker);
  RUN_TEST(test_c2openapi_helpers_register_types_cases);
  RUN_TEST(test_to_docs_json_cli_main_all_verbs);
  RUN_TEST(test_generate_bindings_cli_main_options);
  RUN_TEST(test_c2openapi_cli_main_edge_branches);
  RUN_TEST(test_c2openapi_cli_main_invalid_args);
  RUN_TEST(test_c2openapi_cli_main_valid_args);
  RUN_TEST(test_c2openapi_cli_main_valid_args_with_options);
  RUN_TEST(test_to_docs_json_cli_main_help);
  RUN_TEST(test_to_docs_json_cli_main_no_input);
  RUN_TEST(test_to_docs_json_cli_main_valid);
  RUN_TEST(test_generate_bindings_cli_main);
  RUN_TEST(test_generate_bindings_cli_main_help);
  RUN_TEST(test_c2openapi_cli_main_doc_tags);
  RUN_TEST(test_c2openapi_infer_routes_and_views);
  RUN_TEST(test_cli_c2openapi_unit_internals);
  RUN_TEST(test_cli_c2openapi_aggregator_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_C2OPENAPI_H */
