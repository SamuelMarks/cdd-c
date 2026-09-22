#ifndef TEST_CLI_C2OPENAPI_H
#define TEST_CLI_C2OPENAPI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "routes/parse/cli.h"
#include <greatest.h>
#include <parson.h>
/* clang-format on */

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
#if defined(_MSC_VER)
  if (fopen_s(&f, "src/tests/mocks/emit/simple.schema.json", "r") == 0 && f) {
    fclose(f);
    return "src/tests/mocks";
  }
  if (fopen_s(&f, "../src/tests/mocks/emit/simple.schema.json", "r") == 0 &&
      f) {
    fclose(f);
    return "../src/tests/mocks";
  }
#else
  f = fopen("src/tests/mocks/emit/simple.schema.json", "r");
  if (f) {
    fclose(f);
    return "src/tests/mocks";
  }
  f = fopen("../src/tests/mocks/emit/simple.schema.json", "r");
  if (f) {
    fclose(f);
    return "../src/tests/mocks";
  }
#endif
  return "src/tests/mocks";
}

static const char *get_simple_schema(void) {
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "src/tests/mocks/emit/simple.schema.json", "r") == 0 && f) {
    fclose(f);
    return "src/tests/mocks/emit/simple.schema.json";
  }
  if (fopen_s(&f, "../src/tests/mocks/emit/simple.schema.json", "r") == 0 &&
      f) {
    fclose(f);
    return "../src/tests/mocks/emit/simple.schema.json";
  }
#else
  f = fopen("src/tests/mocks/emit/simple.schema.json", "r");
  if (f) {
    fclose(f);
    return "src/tests/mocks/emit/simple.schema.json";
  }
  f = fopen("../src/tests/mocks/emit/simple.schema.json", "r");
  if (f) {
    fclose(f);
    return "../src/tests/mocks/emit/simple.schema.json";
  }
#endif
  return "src/tests/mocks/emit/simple.schema.json";
}

TEST test_c2openapi_cli_main_invalid_args(void) {
  char *argv1[] = {(char *)(size_t)(size_t) "c2openapi"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(1, argv1));
  PASS();
}

TEST test_c2openapi_cli_main_valid_args(void) {
  char *argv1[3];
  int rc;
  argv1[0] = (char *)(size_t)(size_t) "c2openapi";
  argv1[1] = (char *)(size_t)(size_t)get_mocks_dir();
  argv1[2] = (char *)(size_t)(size_t) "out.json";
  rc = c2openapi_cli_main(3, argv1);
  (void)rc;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  PASS();
}

TEST test_c2openapi_cli_main_valid_args_with_options(void) {
  char *argv1[9];
  int rc;
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
  (void)rc;
  if (rc != CDD_C_SUCCESS) {
    printf(
        "test_c2openapi_cli_main_valid_args_with_options failed with rc=%d\n",
        rc);
  }
  ASSERT_EQ(CDD_C_SUCCESS, rc);
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
  char *argv1[5];
  int rc;
  argv1[0] = (char *)(size_t)(size_t) "to_docs_json";
  argv1[1] = (char *)(size_t)(size_t) "-i";
  argv1[2] = (char *)(size_t)(size_t)get_simple_schema();
  argv1[3] = (char *)(size_t)(size_t) "--no-imports";
  argv1[4] = (char *)(size_t)(size_t) "--no-wrapping";
  rc = to_docs_json_cli_main(5, argv1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
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
  cdd_c_error_t rc;

  /* is_source_file */
  rc = c2openapi_is_source_file(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_is_source_file("file_no_ext", &is_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_src);
  rc = c2openapi_is_source_file("file.txt", &is_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_src);
  rc = c2openapi_is_source_file("file.c", &is_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_src);
  rc = c2openapi_is_source_file("file.h", &is_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_src);

  /* spec_has_tag, spec_add_tag, spec_find_tag */
  openapi_spec_init(&spec);
  rc = c2openapi_spec_has_tag(NULL, "tag", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_has_tag(NULL, "tag", &has_tag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, has_tag);
  rc = c2openapi_spec_has_tag(&spec, NULL, &has_tag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, has_tag);

  rc = c2openapi_spec_add_tag(NULL, "tag");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_add_tag(&spec, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_add_tag(&spec, "tag1");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  /* Duplicate add is a no-op */
  rc = c2openapi_spec_add_tag(&spec, "tag1");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2openapi_spec_find_tag(NULL, "tag", &tag);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_find_tag(&spec, NULL, &tag);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_spec_find_tag(&spec, "tag1", &tag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
  cdd_c_error_t rc;

  /* map_doc_security_in */
  rc = c2openapi_map_doc_security_in(DOC_SEC_IN_QUERY, &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_QUERY, sec_in);
  rc = c2openapi_map_doc_security_in(DOC_SEC_IN_HEADER, &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_HEADER, sec_in);
  rc = c2openapi_map_doc_security_in(DOC_SEC_IN_COOKIE, &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_COOKIE, sec_in);
  rc = c2openapi_map_doc_security_in(DOC_SEC_IN_UNSET, &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sec_in);
  rc = c2openapi_map_doc_security_in((enum DocSecurityIn)999, &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sec_in);

  /* map_doc_flow_type */
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_IMPLICIT, &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_IMPLICIT, flow_type);
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_PASSWORD, &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_PASSWORD, flow_type);
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_CLIENT_CREDENTIALS,
                                   &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_CLIENT_CREDENTIALS, flow_type);
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_AUTHORIZATION_CODE,
                                   &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_AUTHORIZATION_CODE, flow_type);
  rc = c2openapi_map_doc_flow_type(DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION,
                                   &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
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
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&flow, 0, sizeof(flow));
  flow.type = DOC_OAUTH_FLOW_PASSWORD;
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  flow.token_url = (char *)(size_t) "http://example.com/token";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&flow, 0, sizeof(flow));
  flow.type = DOC_OAUTH_FLOW_CLIENT_CREDENTIALS;
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  flow.token_url = (char *)(size_t) "http://example.com/token";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&flow, 0, sizeof(flow));
  flow.type = DOC_OAUTH_FLOW_AUTHORIZATION_CODE;
  flow.authorization_url = (char *)(size_t) "http://example.com/auth";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  flow.token_url = (char *)(size_t) "http://example.com/token";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&flow, 0, sizeof(flow));
  flow.type = DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION;
  flow.device_authorization_url = (char *)(size_t) "http://example.com/dev";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  flow.token_url = (char *)(size_t) "http://example.com/token";
  rc = c2openapi_validate_doc_oauth_flow(&flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

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
  cdd_c_error_t rc;

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
  ASSERT_EQ(CDD_C_SUCCESS, rc);
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
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  doc_metadata_init(&meta);

  /* NULL spec or meta */
  rc = c2openapi_apply_doc_global_meta(NULL, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_apply_doc_global_meta(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

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
  ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

TEST test_c2openapi_helpers_signature_parsing(void) {
  struct C2OpenAPI_ParsedSig sig;
  cdd_c_error_t rc;

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
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("my_empty_func", sig.name);
  c2openapi_free_parsed_sig(&sig);

  rc = c2openapi_parse_c_signature_string(
      "int calculate(int a, const char *b, double arr[])", &sig);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("calculate", sig.name);
  ASSERT_EQ(3, sig.n_args);
  ASSERT_STR_EQ("a", sig.args[0].name);
  ASSERT_STR_EQ("int", sig.args[0].type);
  ASSERT_STR_EQ("b", sig.args[1].name);
  ASSERT_STR_EQ("const char *", sig.args[1].type);
  ASSERT_STR_EQ("arr", sig.args[2].name);
  ASSERT_STR_EQ("double []", sig.args[2].type);
  c2openapi_free_parsed_sig(&sig);

  PASS();
}

TEST test_c2openapi_helpers_load_base_and_walker(void) {
  struct OpenAPI_Spec spec;
  const char *test_c_file = "test_c2openapi_walker.c";
  cdd_c_error_t rc;

  openapi_spec_init(&spec);

  /* load_base_spec errors */
  rc = c2openapi_load_base_spec(NULL, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_load_base_spec("nonexistent_base_spec_123.json", &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* walker_cb on non-source files */
  rc = c2openapi_walker_cb("test.txt", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_walker_cb("test_no_ext", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

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
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(spec.n_paths >= 1);
  ASSERT(spec.n_webhooks >= 1);

  openapi_spec_free(&spec);
  remove(test_c_file);
  PASS();
}

TEST test_c2openapi_helpers_register_types_cases(void) {
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  struct StructFields sf;
  struct EnumMembers em;
  char *unions[] = {(char *)(size_t) "string", (char *)(size_t) "integer"};
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  /* NULL checks */
  rc = c2openapi_register_types(NULL, &types);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_register_types(&spec, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Struct type with field unions */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "data", "string", NULL, NULL, 0);
  sf.fields[0].n_type_union = 2;
  sf.fields[0].type_union = (char **)unions;
  sf.fields[0].n_items_type_union = 2;
  sf.fields[0].items_type_union = (char **)unions;

  /* Enum type */
  enum_members_init(&em);
  enum_members_add(&em, "RED");
  enum_members_add(&em, "GREEN");

  types.items =
      (struct TypeDefinition *)calloc(3, sizeof(struct TypeDefinition));
  types.size = 3;
  types.capacity = 3;

  types.items[0].kind = KIND_STRUCT;
  types.items[0].name = (char *)(size_t) "MyDataStruct";
  types.items[0].details.struct_fields = &sf;

  types.items[1].kind = KIND_ENUM;
  types.items[1].name = (char *)(size_t) "MyColorEnum";
  types.items[1].details.enum_members = &em;

  /* Duplicate type check */
  types.items[2].kind = KIND_STRUCT;
  types.items[2].name = (char *)(size_t) "MyDataStruct";
  types.items[2].details.struct_fields = &sf;

  rc = c2openapi_register_types(&spec, &types);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, spec.n_defined_schemas);
  ASSERT_STR_EQ("MyDataStruct", spec.defined_schema_names[0]);
  ASSERT_STR_EQ("MyColorEnum", spec.defined_schema_names[1]);

  sf.fields[0].n_type_union = 0;
  sf.fields[0].type_union = NULL;
  sf.fields[0].n_items_type_union = 0;
  sf.fields[0].items_type_union = NULL;
  struct_fields_free(&sf);
  enum_members_free(&em);
  free(types.items);
  types.items = NULL;
  types.size = 0;
  type_def_list_free(&types);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

TEST test_to_docs_json_cli_main_all_verbs(void) {
  const char *spec_file = "test_docs_all_verbs.json";
  const char *invalid_spec = "test_docs_invalid_spec.json";
  char *argv1[5];
  char *argv2[3];
  cdd_c_error_t rc;

  /* Write spec containing operations with all HTTP verbs */
  {
    FILE *fp = fopen(spec_file, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},\"paths\":{\"/r\":{",
            fp);
      fputs("\"get\":{\"operationId\":\"g\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"post\":{\"operationId\":\"po\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"put\":{\"operationId\":\"u\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"delete\":{\"operationId\":\"d\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"patch\":{\"operationId\":\"p\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"head\":{\"operationId\":\"h\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"options\":{\"operationId\":\"o\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"trace\":{\"operationId\":\"t\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"query\":{\"operationId\":\"q\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}}}}}",
            fp);
      fclose(fp);
    }
  }

  argv1[0] = (char *)(size_t) "to_docs_json";
  argv1[1] = (char *)(size_t) "-i";
  argv1[2] = (char *)(size_t)spec_file;
  argv1[3] = (char *)(size_t) "--no-imports";
  argv1[4] = (char *)(size_t) "--no-wrapping";

  rc = to_docs_json_cli_main(5, argv1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Without --no-imports and --no-wrapping */
  argv2[0] = (char *)(size_t) "to_docs_json";
  argv2[1] = (char *)(size_t) "-i";
  argv2[2] = (char *)(size_t)spec_file;
  rc = to_docs_json_cli_main(3, argv2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Invalid OpenAPI spec returning error from openapi_load_from_json */
  write_to_file(invalid_spec, "[1, 2, 3]");
  argv2[2] = (char *)(size_t)invalid_spec;
  rc = to_docs_json_cli_main(3, argv2);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);

  remove(spec_file);
  remove(invalid_spec);
  PASS();
}

TEST test_generate_bindings_cli_main_options(void) {
  const char *dummy_header = "test_cli_bind_dummy.h";
  const char *out_dir = "test_cli_bind_out";
  char *argv_full[14];
  char *argv_missing[5];
  cdd_c_error_t rc;

  write_to_file(dummy_header, "int my_api_add(int a, int b);\n");
  makedir(out_dir);

  /* Full options test with valid directory and header */
  argv_full[0] = (char *)(size_t) "bind";
  argv_full[1] = (char *)(size_t) "-i";
  argv_full[2] = (char *)(size_t)dummy_header;
  argv_full[3] = (char *)(size_t) "-o";
  argv_full[4] = (char *)(size_t)out_dir;
  argv_full[5] = (char *)(size_t) "-l";
  argv_full[6] = (char *)(size_t) "python";
  argv_full[7] = (char *)(size_t) "-n";
  argv_full[8] = (char *)(size_t) "my_lib";
  argv_full[9] = (char *)(size_t) "-m";
  argv_full[10] = (char *)(size_t) "my_mod";
  argv_full[11] = (char *)(size_t) "--skip-static";
  argv_full[12] = (char *)(size_t) "--opaque-pointers";
  argv_full[13] = (char *)(size_t) "--generate-tests";

  rc = generate_bindings_cli_main(14, argv_full);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Missing required parameters */
  argv_missing[0] = (char *)(size_t) "bind";
  argv_missing[1] = (char *)(size_t) "-i";
  argv_missing[2] = (char *)(size_t)dummy_header;
  argv_missing[3] = (char *)(size_t) "-o";
  argv_missing[4] = (char *)(size_t)out_dir;
  rc = generate_bindings_cli_main(5, argv_missing);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  remove(dummy_header);
  PASS();
}

TEST test_c2openapi_cli_main_edge_branches(void) {
  char *argv_base_missing[] = {(char *)(size_t) "c2openapi",
                               (char *)(size_t) "--base"};
  char *argv_self_missing[] = {(char *)(size_t) "c2openapi",
                               (char *)(size_t) "--self"};
  char *argv_dialect_missing[] = {(char *)(size_t) "c2openapi",
                                  (char *)(size_t) "--dialect"};
  char *argv_base_fail[] = {
      (char *)(size_t) "c2openapi", (char *)(size_t) "--base",
      (char *)(size_t) "nonexistent_base_123.json", (char *)(size_t) "src",
      (char *)(size_t) "out.json"};
  char *argv_walk_fail[] = {(char *)(size_t) "c2openapi",
                            (char *)(size_t) "nonexistent_src_dir_123",
                            (char *)(size_t) "out.json"};
  char *argv_write_fail[3];
  cdd_c_error_t rc;

  argv_write_fail[0] = (char *)(size_t) "c2openapi";
  argv_write_fail[1] = (char *)(size_t)get_mocks_dir();
  argv_write_fail[2] = (char *)(size_t) "/nonexistent_dir_9999/out.json";

  /* Missing option arguments */
  rc = c2openapi_cli_main(2, argv_base_missing);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  rc = c2openapi_cli_main(2, argv_self_missing);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  rc = c2openapi_cli_main(2, argv_dialect_missing);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Base file load failure */
  rc = c2openapi_cli_main(5, argv_base_fail);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Walk failure on nonexistent dir */
  rc = c2openapi_cli_main(3, argv_walk_fail);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Write failure on invalid output path */
  rc = c2openapi_cli_main(3, argv_write_fail);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);

  PASS();
}

TEST test_c2openapi_helpers_tag_meta_and_collection(void) {
  struct OpenAPI_Spec spec;
  struct DocTagMeta meta;
  struct OpenAPI_Operation op;
  char *tag_names[] = {(char *)(size_t) "tagA", (char *)(size_t) "tagB"};
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  memset(&meta, 0, sizeof(meta));

  /* NULL checks */
  rc = c2openapi_spec_apply_tag_meta(NULL, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_spec_apply_tag_meta(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  meta.name = (char *)(size_t) "";
  rc = c2openapi_spec_apply_tag_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Full tag metadata */
  meta.name = (char *)(size_t) "MyTag";
  meta.summary = (char *)(size_t) "Tag Summary";
  meta.description = (char *)(size_t) "Tag Description";
  meta.parent = (char *)(size_t) "ParentTag";
  meta.kind = (char *)(size_t) "custom";
  meta.external_docs_url = (char *)(size_t) "http://example.com/docs";
  meta.external_docs_description = (char *)(size_t) "Docs description";

  rc = c2openapi_spec_apply_tag_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* apply_doc_tag_meta */
  {
    struct DocMetadata doc_meta;
    doc_metadata_init(&doc_meta);
    rc = c2openapi_apply_doc_tag_meta(&spec, &doc_meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_meta.n_tag_meta = 1;
    doc_meta.tag_meta = &meta;
    rc = c2openapi_apply_doc_tag_meta(&spec, &doc_meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_meta.tag_meta = NULL;
    doc_meta.n_tag_meta = 0;
    doc_metadata_free(&doc_meta);
  }

  /* collect_tags_from_op */
  rc = c2openapi_collect_tags_from_op(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  memset(&op, 0, sizeof(op));
  op.tags = tag_names;
  op.n_tags = 2;
  rc = c2openapi_collect_tags_from_op(&spec, &op);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* collect_tags_from_paths with operations and additional_operations */
  {
    struct OpenAPI_Path paths[1];
    memset(paths, 0, sizeof(paths));
    paths[0].operations = &op;
    paths[0].n_operations = 1;
    paths[0].additional_operations = &op;
    paths[0].n_additional_operations = 1;
    rc = c2openapi_collect_tags_from_paths(&spec, paths, 1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  /* collect_spec_tags */
  rc = c2openapi_collect_spec_tags(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_collect_spec_tags(&spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

TEST test_c2openapi_helpers_scopes_and_flow_merge(void) {
  struct OpenAPI_SecurityScheme scheme;
  struct OpenAPI_OAuthFlow flow;
  struct DocOAuthFlow doc_flow;
  struct DocOAuthScope scopes[2];
  struct OpenAPI_OAuthFlow *found = NULL;
  cdd_c_error_t rc;

  memset(&scheme, 0, sizeof(scheme));
  memset(&flow, 0, sizeof(flow));
  memset(&doc_flow, 0, sizeof(doc_flow));
  memset(scopes, 0, sizeof(scopes));

  /* find_oauth_flow */
  rc = c2openapi_find_oauth_flow(&scheme, OA_OAUTH_FLOW_IMPLICIT, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* merge_scopes */
  scopes[0].name = (char *)(size_t) "read";
  scopes[0].description = (char *)(size_t) "Read scope";
  scopes[1].name = (char *)(size_t) "write";
  scopes[1].description = NULL;
  doc_flow.scopes = scopes;
  doc_flow.n_scopes = 2;

  rc = c2openapi_merge_scopes(&flow, &doc_flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, flow.n_scopes);

  /* Merge duplicate scope */
  rc = c2openapi_merge_scopes(&flow, &doc_flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, flow.n_scopes);

  /* merge_oauth_flow */
  doc_flow.authorization_url = (char *)(size_t) "http://example.com/auth";
  doc_flow.token_url = (char *)(size_t) "http://example.com/token";
  doc_flow.refresh_url = (char *)(size_t) "http://example.com/refresh";
  doc_flow.device_authorization_url = (char *)(size_t) "http://example.com/dev";
  rc = c2openapi_merge_oauth_flow(&flow, &doc_flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(flow.authorization_url != NULL);
  ASSERT(flow.token_url != NULL);
  ASSERT(flow.refresh_url != NULL);
  ASSERT(flow.device_authorization_url != NULL);

  /* Clean up flow */
  C_CDD_FREE(flow.authorization_url);
  C_CDD_FREE(flow.token_url);
  C_CDD_FREE(flow.refresh_url);
  C_CDD_FREE(flow.device_authorization_url);
  if (flow.scopes) {
    size_t s;
    for (s = 0; s < flow.n_scopes; ++s) {
      C_CDD_FREE(flow.scopes[s].name);
      if (flow.scopes[s].description)
        C_CDD_FREE(flow.scopes[s].description);
    }
    C_CDD_FREE(flow.scopes);
  }

  PASS();
}

TEST test_c2openapi_helpers_servers_and_security_append(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  struct DocServer srv[1];
  struct DocServerVar vars[1];
  struct DocSecurityRequirement sec[1];
  char *sec_scopes[] = {(char *)(size_t) "read", (char *)(size_t) "write"};
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  doc_metadata_init(&meta);

  /* append_root_servers with NULL / empty */
  rc = c2openapi_append_root_servers(NULL, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_append_root_servers(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Server with name, description, variables */
  memset(srv, 0, sizeof(srv));
  memset(vars, 0, sizeof(vars));
  vars[0].name = (char *)(size_t) "port";
  vars[0].default_value = (char *)(size_t) "8080";
  srv[0].url = (char *)(size_t) "http://localhost:8080";
  srv[0].name = (char *)(size_t) "Local";
  srv[0].description = (char *)(size_t) "Local server";
  srv[0].variables = vars;
  srv[0].n_variables = 1;

  meta.servers = srv;
  meta.n_servers = 1;
  rc = c2openapi_append_root_servers(&spec, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, spec.n_servers);
  ASSERT_STR_EQ("Local", spec.servers[0].name);
  ASSERT_STR_EQ("Local server", spec.servers[0].description);

  /* append_root_security with NULL / empty */
  rc = c2openapi_append_root_security(NULL, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_append_root_security(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Security requirement with scopes */
  memset(sec, 0, sizeof(sec));
  sec[0].scheme = (char *)(size_t) "OAuth2";
  sec[0].scopes = sec_scopes;
  sec[0].n_scopes = 2;
  meta.security = sec;
  meta.n_security = 1;

  rc = c2openapi_append_root_security(&spec, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, spec.n_security);
  ASSERT_STR_EQ("OAuth2", spec.security[0].requirements[0].scheme);
  ASSERT_EQ(2, spec.security[0].requirements[0].n_scopes);

  meta.servers = NULL;
  meta.n_servers = 0;
  meta.security = NULL;
  meta.n_security = 0;
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

TEST test_c2openapi_helpers_sig_parsing_extra(void) {
  struct C2OpenAPI_ParsedSig sig;
  cdd_c_error_t rc;

  /* Function with unnamed/void argument */
  rc = c2openapi_parse_c_signature_string("void no_named_args(void)", &sig);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, sig.n_args);
  c2openapi_free_parsed_sig(&sig);

  PASS();
}

TEST test_c2openapi_helpers_security_schemes_all_types(void) {
  struct OpenAPI_Spec spec;
  struct DocSecurityScheme doc;
  struct DocOAuthFlow flow;
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  memset(&doc, 0, sizeof(doc));
  memset(&flow, 0, sizeof(flow));

  /* NULL checks */
  rc = c2openapi_spec_add_security_scheme(NULL, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_spec_add_security_scheme(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc.name = (char *)(size_t) "";
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Unknown type */
  doc.name = (char *)(size_t) "SecUnknown";
  doc.type = DOC_SEC_UNSET;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OAuth2 with invalid flow */
  doc.name = (char *)(size_t) "SecOAuthInvalid";
  doc.type = DOC_SEC_OAUTH2;
  flow.type = DOC_OAUTH_FLOW_IMPLICIT; /* Missing auth URL */
  doc.flows = &flow;
  doc.n_flows = 1;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* API Key errors */
  doc.name = (char *)(size_t) "ApiKey1";
  doc.type = DOC_SEC_APIKEY;
  doc.param_name = NULL;
  doc.in = DOC_SEC_IN_HEADER;
  doc.flows = NULL;
  doc.n_flows = 0;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  doc.param_name = (char *)(size_t) "X-API-Key";
  doc.in = DOC_SEC_IN_UNSET;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Valid API Key */
  doc.in = DOC_SEC_IN_HEADER;
  doc.description = (char *)(size_t) "API Key auth";
  doc.deprecated_set = 1;
  doc.deprecated = 1;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Colliding deprecated value */
  doc.deprecated = 0;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  doc.deprecated = 1;

  /* Type collision */
  doc.type = DOC_SEC_HTTP;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* HTTP scheme errors and valid */
  doc.name = (char *)(size_t) "HttpSec";
  doc.type = DOC_SEC_HTTP;
  doc.scheme = NULL;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  doc.scheme = (char *)(size_t) "bearer";
  doc.bearer_format = (char *)(size_t) "JWT";
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OpenID errors and valid */
  doc.name = (char *)(size_t) "OpenIdSec";
  doc.type = DOC_SEC_OPENID;
  doc.open_id_connect_url = NULL;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  doc.open_id_connect_url = (char *)(size_t) "http://example.com/openid";
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OAuth2 valid flow and metadata url */
  doc.name = (char *)(size_t) "OAuthValid";
  doc.type = DOC_SEC_OAUTH2;
  doc.oauth2_metadata_url = (char *)(size_t) "http://example.com/oauth_meta";
  flow.type = DOC_OAUTH_FLOW_IMPLICIT;
  flow.authorization_url = (char *)(size_t) "http://example.com/auth";
  doc.flows = &flow;
  doc.n_flows = 1;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OAuth2 with 0 flows -> error */
  doc.name = (char *)(size_t) "OAuthZeroFlows";
  doc.flows = NULL;
  doc.n_flows = 0;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Mutual TLS */
  doc.name = (char *)(size_t) "MtlsSec";
  doc.type = DOC_SEC_MUTUALTLS;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* apply_doc_security_schemes */
  {
    struct DocMetadata meta;
    doc_metadata_init(&meta);
    rc = c2openapi_apply_doc_security_schemes(NULL, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = c2openapi_apply_doc_security_schemes(&spec, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    meta.security_schemes = &doc;
    meta.n_security_schemes = 1;
    rc = c2openapi_apply_doc_security_schemes(&spec, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    meta.security_schemes = NULL;
    meta.n_security_schemes = 0;
    doc_metadata_free(&meta);
  }

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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

TEST test_c2openapi_helpers_global_meta_conflicts_full(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  doc_metadata_init(&meta);

  c_cdd_strdup("OrigTitle", &spec.info.title);
  meta.info_title = (char *)(size_t) "NewTitle";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.info_title = NULL;

  c_cdd_strdup("1.0", &spec.info.version);
  meta.info_version = (char *)(size_t) "2.0";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.info_version = NULL;

  c_cdd_strdup("Summary1", &spec.info.summary);
  meta.info_summary = (char *)(size_t) "Summary2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.info_summary = NULL;

  c_cdd_strdup("Desc1", &spec.info.description);
  meta.info_description = (char *)(size_t) "Desc2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.info_description = NULL;

  c_cdd_strdup("Terms1", &spec.info.terms_of_service);
  meta.terms_of_service = (char *)(size_t) "Terms2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.terms_of_service = NULL;

  c_cdd_strdup("Name1", &spec.info.contact.name);
  meta.contact_name = (char *)(size_t) "Name2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.contact_name = NULL;

  c_cdd_strdup("Url1", &spec.info.contact.url);
  meta.contact_url = (char *)(size_t) "Url2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.contact_url = NULL;

  c_cdd_strdup("Email1", &spec.info.contact.email);
  meta.contact_email = (char *)(size_t) "Email2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.contact_email = NULL;

  c_cdd_strdup("Dialect1", &spec.json_schema_dialect);
  meta.json_schema_dialect = (char *)(size_t) "Dialect2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.json_schema_dialect = NULL;

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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

TEST test_c2openapi_oom_helpers_tag_and_global_meta(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  struct DocTagMeta tag_meta;
  int k;

  /* spec_add_tag OOM */
  for (k = 1; k <= 3; ++k) {
    openapi_spec_init(&spec);
    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_spec_add_tag(&spec, "tag_oom");
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* spec_apply_tag_meta OOM */
  for (k = 1; k <= 8; ++k) {
    openapi_spec_init(&spec);
    memset(&tag_meta, 0, sizeof(tag_meta));
    tag_meta.name = (char *)(size_t) "tag_oom";
    tag_meta.summary = (char *)(size_t) "summary";
    tag_meta.description = (char *)(size_t) "description";
    tag_meta.parent = (char *)(size_t) "parent";
    tag_meta.kind = (char *)(size_t) "kind";
    tag_meta.external_docs_url = (char *)(size_t) "http://url";
    tag_meta.external_docs_description = (char *)(size_t) "desc";

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_spec_apply_tag_meta(&spec, &tag_meta);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* apply_doc_global_meta OOM */
  for (k = 1; k <= 5; ++k) {
    openapi_spec_init(&spec);
    doc_metadata_init(&meta);
    meta.external_docs_url = (char *)(size_t) "http://url";
    meta.external_docs_description = (char *)(size_t) "desc";

    g_cdd_strdup_fail = k;
    c2openapi_apply_doc_global_meta(&spec, &meta);
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  PASS();
}

TEST test_c2openapi_oom_helpers_servers_and_security(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  int k;

  /* copy_doc_server_variables OOM */
  for (k = 1; k <= 6; ++k) {
    struct OpenAPI_Server srv;
    struct DocServer doc_srv;
    struct DocServerVar vars[1];
    char *enums[] = {(char *)(size_t) "8080"};
    memset(&srv, 0, sizeof(srv));
    memset(&doc_srv, 0, sizeof(doc_srv));
    memset(vars, 0, sizeof(vars));
    vars[0].name = (char *)(size_t) "port";
    vars[0].default_value = (char *)(size_t) "8080";
    vars[0].description = (char *)(size_t) "desc";
    vars[0].enum_values = enums;
    vars[0].n_enum_values = 1;
    doc_srv.variables = vars;
    doc_srv.n_variables = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_copy_doc_server_variables(&srv, &doc_srv);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    c2openapi_free_openapi_server_variables(&srv);
  }

  /* merge_scopes OOM */
  for (k = 1; k <= 4; ++k) {
    struct OpenAPI_OAuthFlow flow;
    struct DocOAuthFlow doc_flow;
    struct DocOAuthScope scopes[1];
    memset(&flow, 0, sizeof(flow));
    memset(&doc_flow, 0, sizeof(doc_flow));
    memset(scopes, 0, sizeof(scopes));
    scopes[0].name = (char *)(size_t) "read";
    scopes[0].description = (char *)(size_t) "desc";
    doc_flow.scopes = scopes;
    doc_flow.n_scopes = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_merge_scopes(&flow, &doc_flow);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    if (flow.scopes) {
      if (flow.scopes[0].name)
        C_CDD_FREE(flow.scopes[0].name);
      if (flow.scopes[0].description)
        C_CDD_FREE(flow.scopes[0].description);
      C_CDD_FREE(flow.scopes);
    }
  }

  /* add_oauth_flows OOM */
  for (k = 1; k <= 7; ++k) {
    struct OpenAPI_SecurityScheme scheme;
    struct DocSecurityScheme doc;
    struct DocOAuthFlow flow;
    struct DocOAuthScope scopes[1];
    memset(&scheme, 0, sizeof(scheme));
    memset(&doc, 0, sizeof(doc));
    memset(&flow, 0, sizeof(flow));
    memset(scopes, 0, sizeof(scopes));
    scopes[0].name = (char *)(size_t) "read";
    scopes[0].description = (char *)(size_t) "desc";
    flow.type = DOC_OAUTH_FLOW_IMPLICIT;
    flow.authorization_url = (char *)(size_t) "http://auth";
    flow.token_url = (char *)(size_t) "http://token";
    flow.refresh_url = (char *)(size_t) "http://refresh";
    flow.device_authorization_url = (char *)(size_t) "http://dev";
    flow.scopes = scopes;
    flow.n_scopes = 1;
    doc.flows = &flow;
    doc.n_flows = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_add_oauth_flows(&scheme, &doc);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    if (scheme.flows) {
      size_t f_idx;
      for (f_idx = 0; f_idx < scheme.n_flows; ++f_idx) {
        if (scheme.flows[f_idx].authorization_url)
          C_CDD_FREE(scheme.flows[f_idx].authorization_url);
        if (scheme.flows[f_idx].token_url)
          C_CDD_FREE(scheme.flows[f_idx].token_url);
        if (scheme.flows[f_idx].refresh_url)
          C_CDD_FREE(scheme.flows[f_idx].refresh_url);
        if (scheme.flows[f_idx].device_authorization_url)
          C_CDD_FREE(scheme.flows[f_idx].device_authorization_url);
        if (scheme.flows[f_idx].scopes) {
          size_t s_idx;
          for (s_idx = 0; s_idx < scheme.flows[f_idx].n_scopes; ++s_idx) {
            if (scheme.flows[f_idx].scopes[s_idx].name)
              C_CDD_FREE(scheme.flows[f_idx].scopes[s_idx].name);
            if (scheme.flows[f_idx].scopes[s_idx].description)
              C_CDD_FREE(scheme.flows[f_idx].scopes[s_idx].description);
          }
          C_CDD_FREE(scheme.flows[f_idx].scopes);
        }
      }
      C_CDD_FREE(scheme.flows);
    }
  }

  /* spec_add_security_scheme OOM */
  for (k = 1; k <= 5; ++k) {
    struct DocSecurityScheme doc;
    openapi_spec_init(&spec);
    memset(&doc, 0, sizeof(doc));
    doc.name = (char *)(size_t) "SecOOM";
    doc.type = DOC_SEC_APIKEY;
    doc.in = DOC_SEC_IN_HEADER;
    doc.param_name = (char *)(size_t) "key";

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_spec_add_security_scheme(&spec, &doc);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* append_root_security OOM */
  for (k = 1; k <= 5; ++k) {
    struct DocSecurityRequirement sec[1];
    char *sec_scopes[] = {(char *)(size_t) "read"};
    openapi_spec_init(&spec);
    doc_metadata_init(&meta);
    memset(sec, 0, sizeof(sec));
    sec[0].scheme = (char *)(size_t) "OAuth2";
    sec[0].scopes = sec_scopes;
    sec[0].n_scopes = 1;
    meta.security = sec;
    meta.n_security = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_append_root_security(&spec, &meta);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    meta.security = NULL;
    meta.n_security = 0;
    openapi_spec_free(&spec);
  }

  /* append_root_servers OOM */
  for (k = 1; k <= 6; ++k) {
    struct DocServer srv[1];
    openapi_spec_init(&spec);
    doc_metadata_init(&meta);
    memset(srv, 0, sizeof(srv));
    srv[0].url = (char *)(size_t) "http://srv";
    srv[0].name = (char *)(size_t) "name";
    srv[0].description = (char *)(size_t) "desc";
    meta.servers = srv;
    meta.n_servers = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_append_root_servers(&spec, &meta);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    meta.servers = NULL;
    meta.n_servers = 0;
    openapi_spec_free(&spec);
  }

  PASS();
}

TEST test_c2openapi_oom_helpers_sig_and_file_process(void) {
  struct OpenAPI_Spec spec;
  const char *test_c = "test_c2openapi_oom_tmp.c";
  cdd_c_error_t rc;
  int k;

  /* parse_c_signature_string OOM */
  for (k = 1; k <= 6; ++k) {
    struct C2OpenAPI_ParsedSig sig;
    g_cdd_alloc_fail = k;
    c2openapi_parse_c_signature_string("int foo(int a, char *b)", &sig);
    g_cdd_alloc_fail = 0;
  }

  write_to_file(test_c, "/**\n"
                        " * @route GET /items\n"
                        " */\n"
                        "void list_items(void) {}\n");

  /* process_file OOM */
  for (k = 1; k <= 6; ++k) {
    openapi_spec_init(&spec);
    g_cdd_alloc_fail = k;
    c2openapi_process_file(test_c, &spec);
    g_cdd_alloc_fail = 0;
    openapi_spec_free(&spec);
  }

  remove(test_c);

  /* walker_cb warning path */
  {
    openapi_spec_init(&spec);
    rc = c2openapi_walker_cb("nonexistent_walker_file_xyz.c", &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    openapi_spec_free(&spec);
  }

  PASS();
}

TEST test_c2openapi_remaining_edge_cases(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  struct C2OpenAPI_ParsedSig sig;
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  doc_metadata_init(&meta);

  /* spec_find_tag with NULL _out_val */
  rc = c2openapi_spec_find_tag(&spec, "tag", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* free_parsed_sig with return_type */
  memset(&sig, 0, sizeof(sig));
  sig.return_type = (char *)(size_t)c_cdd_strdup_macro("int");
  c2openapi_free_parsed_sig(&sig);

  /* add_oauth_flows with unknown flow type */
  {
    struct OpenAPI_SecurityScheme scheme;
    struct DocSecurityScheme doc;
    struct DocOAuthFlow flow;
    memset(&scheme, 0, sizeof(scheme));
    memset(&doc, 0, sizeof(doc));
    memset(&flow, 0, sizeof(flow));
    flow.type = (enum DocOAuthFlowType)999;
    doc.flows = &flow;
    doc.n_flows = 1;
    rc = c2openapi_add_oauth_flows(&scheme, &doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }

  /* append_root_servers when copy_doc_server_variables fails with url and name
   * set */
  {
    struct DocServer srv[1];
    struct DocServerVar vars[1];
    memset(srv, 0, sizeof(srv));
    memset(vars, 0, sizeof(vars));
    srv[0].url = (char *)(size_t) "http://srv";
    srv[0].name = (char *)(size_t) "name";
    srv[0].description = (char *)(size_t) "desc";
    vars[0].name = NULL; /* causes copy_doc_server_variables to fail */
    srv[0].variables = vars;
    srv[0].n_variables = 1;
    meta.servers = srv;
    meta.n_servers = 1;
    rc = c2openapi_append_root_servers(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    meta.servers = NULL;
    meta.n_servers = 0;
  }

  /* apply_doc_global_meta license name/url/id conflicts and second
   * external_docs description */
  {
    spec.info.license.name = (char *)(size_t)c_cdd_strdup_macro("LicA");
    meta.license_name = (char *)(size_t) "LicB";
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    C_CDD_FREE(spec.info.license.name);
    spec.info.license.name = (char *)(size_t)c_cdd_strdup_macro("LicA");
    meta.license_name = (char *)(size_t) "LicA";

    spec.info.license.url = (char *)(size_t)c_cdd_strdup_macro("UrlA");
    meta.license_url = (char *)(size_t) "UrlB";
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    C_CDD_FREE(spec.info.license.url);
    spec.info.license.url = (char *)(size_t)c_cdd_strdup_macro("UrlA");
    meta.license_url = (char *)(size_t) "UrlA";

    C_CDD_FREE(spec.info.license.url);
    spec.info.license.url = NULL;
    meta.license_url = NULL;
    spec.info.license.identifier = (char *)(size_t)c_cdd_strdup_macro("IdA");
    meta.license_identifier = (char *)(size_t) "IdB";
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    C_CDD_FREE(spec.info.license.identifier);
    spec.info.license.identifier = NULL;
    meta.license_identifier = NULL;

    /* external_docs description OOM */
    spec.external_docs.url = (char *)(size_t)c_cdd_strdup_macro("http://docs");
    meta.external_docs_url = (char *)(size_t) "http://docs";
    meta.external_docs_description = (char *)(size_t) "Added Desc";
    g_cdd_strdup_fail = 1;
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_strdup_fail = 0;

    /* external_docs matching url, adding description */
    memset(&meta, 0, sizeof(meta));
    spec.external_docs.url = (char *)(size_t)c_cdd_strdup_macro("http://docs");
    meta.external_docs_url = (char *)(size_t) "http://docs";
    meta.external_docs_description = (char *)(size_t) "Added Desc";
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("Added Desc", spec.external_docs.description);
  }

  /* process_file with comment separated by blank line */
  {
    const char *blank_file = "test_cli_blank_line.c";
    write_to_file(blank_file, "/**\n"
                              " * @route GET /spaced\n"
                              " */\n\n"
                              "void spaced_func(void) {}\n");
    rc = c2openapi_process_file(blank_file, &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    remove(blank_file);
  }

  /* parse_c_signature_string failing midway on args (trigger cleanup out->args
   * loop) */
  {
    g_cdd_alloc_fail = 4;
    rc = c2openapi_parse_c_signature_string("void multi(int a, int b, int c)",
                                            &sig);
    g_cdd_alloc_fail = 0;
  }

  /* generate_bindings_cli_main failing execution */
  {
    char *argv_fail[7];
    argv_fail[0] = (char *)(size_t) "bind";
    argv_fail[1] = (char *)(size_t) "-i";
    argv_fail[2] = (char *)(size_t) "nonexistent_dummy.h";
    argv_fail[3] = (char *)(size_t) "-o";
    argv_fail[4] = (char *)(size_t) "/nonexistent_dir_xyz";
    argv_fail[5] = (char *)(size_t) "-l";
    argv_fail[6] = (char *)(size_t) "python";
    rc = generate_bindings_cli_main(7, argv_fail);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  }

  /* c2openapi_cli_main with base spec that has self_uri and dialect, replaced
   * by options */
  {
    const char *base_tmp = "test_cli_base_overwrite.json";
    char *argv_replace[9];
    FILE *fp = fopen(base_tmp, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"$self\":\"http://old.com/"
            "self\",\"jsonSchemaDialect\":\"http://old.com/"
            "dialect\",\"info\":{\"title\":\"T\",\"version\":\"1\"},\"paths\":{"
            "}}",
            fp);
      fclose(fp);
    }

    argv_replace[0] = (char *)(size_t) "c2openapi";
    argv_replace[1] = (char *)(size_t) "--base";
    argv_replace[2] = (char *)(size_t)base_tmp;
    argv_replace[3] = (char *)(size_t) "--self";
    argv_replace[4] = (char *)(size_t) "http://new.com/self";
    argv_replace[5] = (char *)(size_t) "--dialect";
    argv_replace[6] = (char *)(size_t) "http://new.com/dialect";
    argv_replace[7] = (char *)(size_t)get_mocks_dir();
    argv_replace[8] = (char *)(size_t) "out_replace.json";

    rc = c2openapi_cli_main(9, argv_replace);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    remove(base_tmp);
    remove("out_replace.json");
  }

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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
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
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

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
  /* success with null name and null desc */
  doc_scopes[0].name = NULL;
  doc_scopes[0].description = NULL;
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

TEST test_c2openapi_final_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Server srv;
  struct DocServer doc_srv;
  struct DocServerVar doc_vars[1];
  char *enum_vals[1];
  struct OpenAPI_OAuthFlow flow_dst;
  struct DocOAuthFlow doc_flow;
  struct DocOAuthScope doc_scopes[2];
  struct OpenAPI_SecurityScheme scheme;
  struct DocSecurityScheme doc_sec;
  struct DocMetadata meta;
  struct DocTagMeta doc_tag;
  struct OpenAPI_Operation op;
  char *argv[10];

  /* 1. line 303: empty string in apply_doc_global_meta */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.info_title = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));

  /* 2. line 396 & 412: sv->name strdup fail, and enum_values != NULL &&
   * n_enum_values == 0 */
  memset(&srv, 0, sizeof(srv));
  memset(&doc_srv, 0, sizeof(doc_srv));
  doc_srv.n_variables = 1;
  doc_srv.variables = doc_vars;
  memset(doc_vars, 0, sizeof(doc_vars));
  doc_vars[0].name = (char *)(size_t) "var";
  doc_vars[0].default_value = (char *)(size_t) "def";
  doc_vars[0].enum_values = enum_vals;
  doc_vars[0].n_enum_values = 0; /* line 412 false branch */
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables(&srv, &doc_srv));
  free_openapi_server_variables(&srv);

  g_cdd_strdup_fail = 1; /* line 396 */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables(&srv, &doc_srv));
  g_cdd_strdup_fail = 0;

  /* 3. line 472: merge_scopes with scope name == NULL matching non-null */
  memset(&flow_dst, 0, sizeof(flow_dst));
  flow_dst.n_scopes = 1;
  flow_dst.scopes =
      (struct OpenAPI_OAuthScope *)calloc(1, sizeof(struct OpenAPI_OAuthScope));
  flow_dst.scopes[0].name = (char *)(size_t)strdup("read");
  memset(&doc_flow, 0, sizeof(doc_flow));
  doc_flow.n_scopes = 1;
  doc_flow.scopes = doc_scopes;
  memset(doc_scopes, 0, sizeof(doc_scopes));
  doc_scopes[0].name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, merge_scopes(&flow_dst, &doc_flow));
  free(flow_dst.scopes[0].name);
  free(flow_dst.scopes[1].name);
  free(flow_dst.scopes);

  /* 4. line 657, 678, 688: add_oauth_flows */
  memset(&scheme, 0, sizeof(scheme));
  memset(&doc_sec, 0, sizeof(doc_sec));
  doc_sec.n_flows = 1;
  doc_sec.flows = &doc_flow;
  memset(&doc_flow, 0, sizeof(doc_flow));
  doc_flow.type = DOC_OAUTH_FLOW_IMPLICIT;
  doc_flow.authorization_url = (char *)(size_t) "http://auth";
  doc_flow.scopes = doc_scopes;
  doc_flow.n_scopes = 0; /* line 678 false */
  g_cdd_strdup_fail = 1; /* line 657 */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_oauth_flows(&scheme, &doc_sec));
  g_cdd_strdup_fail = 0;
  doc_flow.n_scopes = 1;
  doc_scopes[0].name = NULL; /* line 688 name ? name : "" */
  ASSERT_EQ(CDD_C_SUCCESS, add_oauth_flows(&scheme, &doc_sec));
  if (scheme.flows) {
    if (scheme.flows[0].authorization_url)
      free(scheme.flows[0].authorization_url);
    if (scheme.flows[0].scopes) {
      if (scheme.flows[0].scopes[0].name)
        free(scheme.flows[0].scopes[0].name);
      free(scheme.flows[0].scopes);
    }
    free(scheme.flows);
    scheme.flows = NULL;
    scheme.n_flows = 0;
  }

  /* 5. line 774: OA_SEC_MUTUALTLS, line 789: doc->scheme = "", line 804: openid
   * url = "", line 817: oauth2_metadata_url fail */
  memset(&spec, 0, sizeof(spec));
  memset(&doc_sec, 0, sizeof(doc_sec));
  doc_sec.name = (char *)(size_t) "MutualTlsSec";
  doc_sec.type = DOC_SEC_MUTUALTLS;
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));

  doc_sec.name = (char *)(size_t) "HttpEmptyScheme";
  doc_sec.type = DOC_SEC_HTTP;
  doc_sec.scheme = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));

  doc_sec.name = (char *)(size_t) "OpenIdEmptyUrl";
  doc_sec.type = DOC_SEC_OPENID;
  doc_sec.open_id_connect_url = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));

  doc_sec.name = (char *)(size_t) "OAuthMetaConflict";
  doc_sec.type = DOC_SEC_OAUTH2;
  doc_sec.oauth2_metadata_url = (char *)(size_t) "http://meta1";
  doc_sec.n_flows = 1;
  doc_sec.flows = &doc_flow;
  doc_flow.type = DOC_OAUTH_FLOW_CLIENT_CREDENTIALS;
  doc_flow.token_url = (char *)(size_t) "http://token";
  doc_flow.authorization_url = NULL;
  doc_flow.n_scopes = 0;
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));
  doc_sec.oauth2_metadata_url =
      (char *)(size_t) "http://meta2"; /* conflict -> line 817 */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));
  openapi_spec_free(&spec);

  /* 6. line 992, 1001, 1003, 1012, 1016: append_root_servers cleanups */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.n_servers = 1;
  meta.servers = &doc_srv;
  memset(&doc_srv, 0, sizeof(doc_srv));
  doc_srv.url = (char *)(size_t) "http://srv";
  doc_srv.name = (char *)(size_t) "name1";
  doc_srv.description = (char *)(size_t) "desc1";
  doc_srv.n_variables = 1;
  doc_srv.variables = doc_vars;
  memset(doc_vars, 0, sizeof(doc_vars));
  doc_vars[0].name = (char *)(size_t) "var1";
  doc_vars[0].default_value = (char *)(size_t) "def1";
  /* line 992: name strdup fail with url present */
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_servers(&spec, &meta));
  g_cdd_strdup_fail = 0;
  /* line 1001, 1003: description strdup fail with url and name present */
  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_servers(&spec, &meta));
  g_cdd_strdup_fail = 0;
  /* line 1012, 1016: copy_doc_server_variables fail with url, name, desc
   * present */
  doc_vars[0].name = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, append_root_servers(&spec, &meta));
  doc_vars[0].name = (char *)(size_t) "var1";
  openapi_spec_free(&spec);

  /* 7. line 1100: contact_email, line 1107: meta->license_name == NULL &&
   * spec->info.license.name != NULL, line 1160: append_root_security fail */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.contact_email = (char *)(size_t) "test@example.com";
  spec.info.license.name = (char *)(size_t)strdup("ExistingLic");
  meta.license_url = (char *)(size_t) "http://lic";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  /* line 1160: append_root_security fail */
  meta.n_security = 1;
  meta.security = (struct DocSecurityRequirement *)calloc(
      1, sizeof(struct DocSecurityRequirement));
  meta.security[0].scheme = (char *)(size_t) "oauth";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_doc_global_meta(&spec, &meta));
  g_cdd_strdup_fail = 0;
  free(meta.security);
  openapi_spec_free(&spec);

  /* 8. line 1189: meta->name == "", line 1258: tag_meta != NULL && n_tag_meta
   * == 0, line 1262: spec_apply_tag_meta fail */
  memset(&spec, 0, sizeof(spec));
  memset(&doc_tag, 0, sizeof(doc_tag));
  doc_tag.name = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
  memset(&meta, 0, sizeof(meta));
  meta.tag_meta = &doc_tag;
  meta.n_tag_meta = 0; /* line 1258 false */
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_tag_meta(&spec, &meta));
  meta.n_tag_meta = 1;
  doc_tag.name = (char *)(size_t) "TagAllocFail";
  g_cdd_alloc_fail = 1; /* line 1262 */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_doc_tag_meta(&spec, &meta));
  g_cdd_alloc_fail = 0;
  openapi_spec_free(&spec);

  /* 9. line 1291: op->tags == NULL */
  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));
  op.tags = NULL;
  op.n_tags = 0;
  ASSERT_EQ(CDD_C_SUCCESS, collect_tags_from_op(&spec, &op));

  /* 10. process_file edge cases: line 1681, 1685, 1697, 1707, 1716, 1743 */
  {
    const char *fpath = "test_func_no_route.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      /* Function without route (line 1681 false), function at index 0 without
       * doc (line 1716) */
      fputs("void no_route_first(void) {}\n/**\n * @summary just comment\n "
            "*/\nvoid no_route_second(void) {}\n",
            fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      ASSERT_EQ(CDD_C_SUCCESS, process_file(fpath, &spec));
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* Webhook operation (line 1707) and sig parsing error (line 1697) */
  {
    const char *fpath = "test_webhook_and_bad_sig.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route POST /hook\n * @webhook\n */\nvoid on_webhook(int "
            "x) {}\n/**\n * @route GET /badsig\n */\n123bad(void) {}\n",
            fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      ASSERT_EQ(CDD_C_SUCCESS, process_file(fpath, &spec));
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* 11. walker_cb line 1806 and 1816 */
  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, walker_cb(NULL, &spec));
  {
    const char *fpath = "test_walker_oom.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("void fn(void) {}\n", fp);
      fclose(fp);
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, walker_cb(fpath, &spec));
      g_cdd_alloc_fail = 0;
      remove(fpath);
    }
  }

  /* 12. c2openapi_cli_main line 1928 (-s), 1941, 1945 (self_uri fail), 1951
   * (*dialect_uri == "") */
  {
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t) "-s";
    argv[2] = (char *)(size_t) "http://s1";
    argv[3] = (char *)(size_t) "-s";
    argv[4] = (char *)(size_t) "http://s2"; /* line 1941 overwrite */
    argv[5] = (char *)(size_t) "--dialect";
    argv[6] = (char *)(size_t) ""; /* line 1951 empty */
    TEST_MKDIR("my_empty_dir");
    argv[7] = (char *)(size_t) "my_empty_dir";
    argv[8] = (char *)(size_t) "out.json";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_cli_main(9, argv));
    remove("out.json");

    /* self_uri strdup fail (line 1945) */
    argv[1] = (char *)(size_t) "--self";
    argv[2] = (char *)(size_t) "http://fail_self";
    argv[3] = (char *)(size_t) "my_empty_dir";
    argv[4] = (char *)(size_t) "out.json";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(5, argv));
    g_cdd_strdup_fail = 0;
  }

  /* 13. to_docs_json_cli_main CDD_INPUT env and duplicate path route */
  {
    const char *tmp_spec = "test_docs_multi_op.json";
    FILE *fp = fopen(tmp_spec, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},\"paths\":{\"/"
            "r\":{\"get\":{\"operationId\":\"g\"},\"post\":{\"operationId\":"
            "\"p\"}}}}",
            fp);
      fclose(fp);
    }
    argv[0] = (char *)(size_t) "to_docs_json";
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t)tmp_spec;
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(3, argv));

#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("CDD_INPUT=test_docs_multi_op.json");
#else
    setenv("CDD_INPUT", "test_docs_multi_op.json", 1);
#endif
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(1, argv));
#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("CDD_INPUT=");
#else
    unsetenv("CDD_INPUT");
#endif

#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("INPUT_FILE=test_docs_multi_op.json");
#else
    setenv("INPUT_FILE", "test_docs_multi_op.json", 1);
#endif
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(1, argv));
#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("INPUT_FILE=");
#else
    unsetenv("INPUT_FILE");
#endif
    remove(tmp_spec);
  }

  /* 14. generate_bindings_cli_main short/long flags and missing checks */
  {
    struct TypeDefList types;
    argv[0] = (char *)(size_t) "bind";
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t) "in";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(3, argv));

    argv[3] = (char *)(size_t) "-o";
    argv[4] = (char *)(size_t) "out";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(5, argv));

    argv[5] = (char *)(size_t) "-l";
    argv[6] = (char *)(size_t) "py";
    argv[7] = (char *)(size_t) "-n";
    argv[8] = (char *)(size_t) "lib";
    argv[9] = (char *)(size_t) "-m";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));

    /* long flags */
    argv[1] = (char *)(size_t) "--input";
    argv[2] = (char *)(size_t) "in";
    argv[3] = (char *)(size_t) "--output-dir";
    argv[4] = (char *)(size_t) "out";
    argv[5] = (char *)(size_t) "--lang";
    argv[6] = (char *)(size_t) "py";
    argv[7] = (char *)(size_t) "--lib-name";
    argv[8] = (char *)(size_t) "lib";
    argv[9] = (char *)(size_t) "--module-name";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));

    /* all short valid flags */
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t) "in";
    argv[3] = (char *)(size_t) "-o";
    argv[4] = (char *)(size_t) "out";
    argv[5] = (char *)(size_t) "-l";
    argv[6] = (char *)(size_t) "py";
    argv[7] = (char *)(size_t) "-n";
    argv[8] = (char *)(size_t) "lib";
    argv[9] = (char *)(size_t) "-m";
    /* valid 7 args */
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(7, argv));

    memset(&types, 0, sizeof(types));
    types.items =
        (struct TypeDefinition *)calloc(1, sizeof(struct TypeDefinition));
    types.size = 1;
    types.capacity = 1;
    types.items[0].kind = KIND_ENUM;
    types.items[0].name = NULL;
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_register_types(&spec, &types));
    free(types.items);
    openapi_spec_free(&spec);
  }

  /* 15. line 1084: contact_email without contact_name and contact_url */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.contact_email = (char *)(size_t) "only_email@test.com";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  openapi_spec_free(&spec);

  /* 16. line 1091: !meta->license_name && spec->info.license.name != NULL */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  spec.info.license.name =
      (char *)(size_t)c_cdd_strdup_macro("ExistingLicName");
  meta.license_url = (char *)(size_t) "http://license_url_only";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  openapi_spec_free(&spec);

  /* 17. line 1173: append_root_security fail in apply_doc_global_meta */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.n_security = 1;
  meta.security = (struct DocSecurityRequirement *)calloc(
      1, sizeof(struct DocSecurityRequirement));
  meta.security[0].scheme = (char *)(size_t) "oauth";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_doc_global_meta(&spec, &meta));
  g_cdd_strdup_fail = 0;
  free(meta.security);
  openapi_spec_free(&spec);

  /* 18. line 1385 and 1388: collect_spec_tags with paths and webhooks failure
   */
  memset(&spec, 0, sizeof(spec));
  spec.n_paths = 1;
  spec.paths = (struct OpenAPI_Path *)calloc(1, sizeof(struct OpenAPI_Path));
  spec.paths[0].n_operations = 1;
  spec.paths[0].operations =
      (struct OpenAPI_Operation *)calloc(1, sizeof(struct OpenAPI_Operation));
  spec.paths[0].operations[0].n_tags = 1;
  spec.paths[0].operations[0].tags = (char **)calloc(1, sizeof(char *));
  spec.paths[0].operations[0].tags[0] = (char *)(size_t) "PathTag";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, collect_spec_tags(&spec));
  g_cdd_strdup_fail = 0;

  spec.n_webhooks = 1;
  spec.webhooks = spec.paths;
  spec.n_paths = 0;
  spec.paths = NULL;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, collect_spec_tags(&spec));
  g_cdd_strdup_fail = 0;
  free(spec.webhooks[0].operations[0].tags);
  free(spec.webhooks[0].operations);
  free(spec.webhooks);
  spec.webhooks = NULL;
  spec.n_webhooks = 0;

  /* 19. process_file alloc fail loop to hit all allocation branches (sig_raw,
   * doc_text, comment_used) */
  {
    const char *fpath = "test_alloc_loop.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route GET /alloc\n */\nvoid fn_alloc(int x) {}\n/**\n * "
            "Standalone comment\n */\n",
            fp);
      fclose(fp);
      {
        int k;
        for (k = 1; k <= 15; ++k) {
          memset(&spec, 0, sizeof(spec));
          g_cdd_alloc_fail = k;
          (void)process_file(fpath, &spec);
          g_cdd_alloc_fail = 0;
          openapi_spec_free(&spec);
        }
      }
      remove(fpath);
    }
  }

  /* 20. process_file with bad C signature: int (int a) {} */
  {
    const char *fpath = "test_bad_csig.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route GET /bad_csig\n */\nint (int a) {}\n", fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      ASSERT_EQ(CDD_C_SUCCESS, process_file(fpath, &spec));
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* 21. c2openapi_cli_main -s, --self overwrite, --dialect overwrite */
  {
    const char *base_path = "test_base_with_uris.json";
    FILE *fp = fopen(base_path, "w");
    if (fp) {
      fputs(
          "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
          "\"$self\":\"http://orig_self\",\"jsonSchemaDialect\":\"http://"
          "orig_dialect\",\"paths\":{}}",
          fp);
      fclose(fp);

      argv[0] = (char *)(size_t) "c2openapi";
      argv[1] = (char *)(size_t) "-b";
      argv[2] = (char *)(size_t)base_path;
      argv[3] = (char *)(size_t) "-s";
      argv[4] = (char *)(size_t) "http://new_self";
      argv[5] = (char *)(size_t) "--dialect";
      argv[6] = (char *)(size_t) "http://new_dialect";
      argv[7] = (char *)(size_t) "my_empty_dir";
      argv[8] = (char *)(size_t) "out_uri.json";
      ASSERT_EQ(CDD_C_SUCCESS, c2openapi_cli_main(9, argv));
      remove(base_path);
      remove("out_uri.json");
    }
  }

  /* 22. c2openapi_cli_main collect_spec_tags fail and write fail */
  {
    const char *base_path = "test_base_tags.json";
    FILE *fp = fopen(base_path, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},\"paths\":{\"/r\":{\"get\":{\"tags\":[\"T1\"]}}}}",
            fp);
      fclose(fp);

      argv[0] = (char *)(size_t) "c2openapi";
      argv[1] = (char *)(size_t) "--base";
      argv[2] = (char *)(size_t)base_path;
      argv[3] = (char *)(size_t) "my_empty_dir";
      argv[4] = (char *)(size_t) "out_tag.json";
      g_cdd_strdup_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(5, argv));
      g_cdd_strdup_fail = 0;
      remove(base_path);
    }
  }

  /* 23. to_docs_json_cli_main nonexistent file */
  {
    argv[0] = (char *)(size_t) "to_docs_json";
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t) "nonexistent_docs_9999.json";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_docs_json_cli_main(3, argv));
  }

  /* 24. generate_bindings_cli_main short options and missing arguments */
  {
    argv[0] = (char *)(size_t) "bind";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(1, argv));
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t) "in";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(3, argv));
    argv[3] = (char *)(size_t) "-o";
    argv[4] = (char *)(size_t) "out";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(5, argv));
    argv[5] = (char *)(size_t) "-l";
    argv[6] = (char *)(size_t) "py";
    argv[7] = (char *)(size_t) "-n";
    argv[8] = (char *)(size_t) "lib";
    argv[9] = (char *)(size_t) "-m";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));
    /* All required options present (-i in -o out -l py) -> tests line 2184
     * false branch */
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(7, argv));
  }

  /* 25. apply_all_doc_meta tag and sec scheme failures */
  {
    memset(&spec, 0, sizeof(spec));
    memset(&meta, 0, sizeof(meta));
    meta.n_tag_meta = 1;
    meta.tag_meta = &doc_tag;
    memset(&doc_tag, 0, sizeof(doc_tag));
    doc_tag.name = (char *)(size_t) "TagAlloc";
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_apply_all_doc_meta(&spec, &meta));
    g_cdd_alloc_fail = 0;

    memset(&meta, 0, sizeof(meta));
    meta.n_security_schemes = 1;
    meta.security_schemes = &doc_sec;
    memset(&doc_sec, 0, sizeof(doc_sec));
    doc_sec.name = (char *)(size_t) "SecAlloc";
    doc_sec.type = DOC_SEC_HTTP;
    doc_sec.scheme = (char *)(size_t) "bearer";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_apply_all_doc_meta(&spec, &meta));
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* 26. contact_email == NULL (line 1084) and license_identifier only (line
   * 1091) */
  {
    memset(&spec, 0, sizeof(spec));
    memset(&meta, 0, sizeof(meta));
    meta.contact_name = (char *)(size_t) "OnlyName";
    meta.contact_url = NULL;
    meta.contact_email = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));

    memset(&meta, 0, sizeof(meta));
    meta.license_name = NULL;
    meta.license_url = NULL;
    meta.license_identifier = (char *)(size_t) "MIT";
    spec.info.license.name = (char *)(size_t)c_cdd_strdup_macro("LicName");
    ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));

    /* line 1173: doc_tag.name == NULL */
    memset(&doc_tag, 0, sizeof(doc_tag));
    doc_tag.name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
    openapi_spec_free(&spec);
  }

  /* 27. missing arguments for options in all CLI mains */
  {
    argv[0] = (char *)(size_t) "bind";
    argv[1] = (char *)(size_t) "-i";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--input";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-o";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--output-dir";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-l";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--lang";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-n";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--lib-name";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-m";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--module-name";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));

    /* to_docs_json -i and --input missing */
    argv[0] = (char *)(size_t) "to_docs_json";
    argv[1] = (char *)(size_t) "-i";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_docs_json_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--input";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_docs_json_cli_main(2, argv));

    /* c2openapi options missing args */
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t) "--base";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-b";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--self";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-s";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--dialect";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--jsonSchemaDialect";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));

    /* c2openapi --self empty string */
    argv[1] = (char *)(size_t) "--self";
    argv[2] = (char *)(size_t) "";
    argv[3] = (char *)(size_t) "my_empty_dir";
    argv[4] = (char *)(size_t) "out_self_empty.json";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_cli_main(5, argv));
    remove("out_self_empty.json");
  }

  /* 28. c2openapi_build_operation fail in process_file (line 1705) */
  {
    const char *fpath = "test_custom_method.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route FOOBAR /custom\n */\nvoid fn_custom(void) {}\n",
            fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      g_cdd_strdup_fail = 1;
      (void)process_file(fpath, &spec);
      g_cdd_strdup_fail = 0;
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* 29. Standalone doc_text alloc fail (line 1731) */
  {
    const char *fpath = "test_standalone_alloc.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @infoTitle StandaloneAlloc\n */\n", fp);
      fclose(fp);
      {
        int k;
        for (k = 1; k <= 8; ++k) {
          memset(&spec, 0, sizeof(spec));
          g_cdd_alloc_fail = k;
          (void)process_file(fpath, &spec);
          g_cdd_alloc_fail = 0;
          openapi_spec_free(&spec);
        }
      }
      remove(fpath);
    }
  }

  /* 30. c2openapi_cli_main write fail (line 1938) */
  {
    const char *empty_f = "test_empty_for_walk.txt";
    FILE *fp = fopen(empty_f, "w");
    if (fp) {
      fclose(fp);
      argv[0] = (char *)(size_t) "c2openapi";
      argv[1] = (char *)(size_t)empty_f;
      argv[2] = (char *)(size_t) "/nonexistent_dir_9999/out.json";
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(3, argv));
      remove(empty_f);
    }
  }

  /* 30b. c2openapi_cli_main openapi_write_spec_to_json failure */
  {
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)get_mocks_dir();
    argv[2] = (char *)(size_t) "out.json";
    c2openapi_set_json_allocators(mock_c2o_oom_malloc, mock_c2o_oom_free);
    g_c2o_oom_fail_at = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(3, argv));
    c2openapi_set_json_allocators(malloc, free);
    g_c2o_oom_fail_at = -1;
  }

  /* 31. to_docs_json_cli_main -h (line 1978) */
  {
    argv[0] = (char *)(size_t) "to_docs_json";
    argv[1] = (char *)(size_t) "-h";
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(2, argv));
  }

  /* 32. generate_bindings_cli_main reverse flags (line 2144) and -h (line 2106)
   */
  {
    argv[0] = (char *)(size_t) "bind";
    argv[1] = (char *)(size_t) "--module-name";
    argv[2] = (char *)(size_t) "mod";
    argv[3] = (char *)(size_t) "-m";
    argv[4] = (char *)(size_t) "mod2";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(5, argv));

    argv[1] = (char *)(size_t) "-h";
    ASSERT_EQ(CDD_C_SUCCESS, generate_bindings_cli_main(2, argv));
  }

  PASS();
}

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
    g_cdd_fail_token_find_next = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_parse_c_signature_string(test_code, &sig));
    g_cdd_fail_token_find_next = 0;
    free_parsed_sig(&sig);
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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_C2OPENAPI_H */
