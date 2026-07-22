#ifndef TEST_CLI_C2OPENAPI_H
#define TEST_CLI_C2OPENAPI_H

#include "routes/parse/cli.h"
#include <greatest.h>

TEST test_c2openapi_cli_main_invalid_args(void) {
  char *argv1[] = {"c2openapi"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(1, argv1));
  PASS();
}

TEST test_c2openapi_cli_main_valid_args(void) {
  char *argv1[] = {"c2openapi", "../src/tests/mocks", "out.json"};
  int rc = c2openapi_cli_main(3, argv1);
  if (rc != CDD_C_SUCCESS)
    printf("test_c2openapi_cli_main_valid_args failed with %d\n", rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  PASS();
}

TEST test_c2openapi_cli_main_valid_args_with_options(void) {
  char *argv1[] = {"c2openapi",
                   "--base",
                   "../src/tests/mocks/emit/simple.schema.json",
                   "--self",
                   "http://example.com/api",
                   "--dialect",
                   "http://example.com/dialect",
                   "../src/tests/mocks",
                   "out2.json"};
  int rc = c2openapi_cli_main(9, argv1);
  if (rc != CDD_C_SUCCESS)
    printf("test_c2openapi_cli_main_valid_args_with_options failed with %d\n",
           rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  PASS();
}

TEST test_to_docs_json_cli_main_help(void) {
  char *argv1[] = {"to_docs_json", "--help"};
  ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(2, argv1));
  PASS();
}

TEST test_to_docs_json_cli_main_no_input(void) {
  char *argv1[] = {"to_docs_json"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_docs_json_cli_main(1, argv1));
  PASS();
}

TEST test_to_docs_json_cli_main_valid(void) {
  char *argv1[] = {"to_docs_json", "-i",
                   "../src/tests/mocks/emit/simple.schema.json", "--no-imports",
                   "--no-wrapping"};
  int rc = to_docs_json_cli_main(5, argv1);
  if (rc != CDD_C_SUCCESS)
    printf("test_to_docs_json_cli_main_valid failed with %d\n", rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  PASS();
}

TEST test_generate_bindings_cli_main(void) {
  char *argv1[] = {"bind"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(1, argv1));
  PASS();
}

TEST test_generate_bindings_cli_main_help(void) {
  char *argv1[] = {"bind", "--help"};
  ASSERT_EQ(CDD_C_SUCCESS, generate_bindings_cli_main(2, argv1));
  PASS();
}

TEST test_c2openapi_cli_main_doc_tags(void) {
  FILE *f = fopen("../src/tests/mocks/parse/test_doc_tags.c", "w");
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
    fclose(f);
    char *argv2[] = {"c2openapi", "../src/tests/mocks/parse/test_doc_tags.c",
                     "out3.json"};
    c2openapi_cli_main(3, argv2);
    remove("../src/tests/mocks/parse/test_doc_tags.c");
    remove("out3.json");
  }
  f = fopen("../src/tests/mocks/parse/test_doc_tags_invalid2.c", "w");
  if (f) {
    fprintf(f, "/**\n");
    fprintf(f, " * @securityScheme MyAuth14 [type:apiKey] [in:invalid]\n");
    fprintf(f, " */\n");
    fprintf(f, "void my_func(void) {}\n");
    fclose(f);
    char *argv2[] = {"c2openapi",
                     "../src/tests/mocks/parse/test_doc_tags_invalid2.c",
                     "out52.json"};
    c2openapi_cli_main(3, argv2);
    remove("../src/tests/mocks/parse/test_doc_tags_invalid2.c");
    remove("out52.json");
  }
  f = fopen("../src/tests/mocks/parse/test_doc_tags_oauth.c", "w");
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
    fclose(f);
    char *argv2[] = {"c2openapi",
                     "../src/tests/mocks/parse/test_doc_tags_oauth.c",
                     "out4.json"};
    c2openapi_cli_main(3, argv2);
    remove("../src/tests/mocks/parse/test_doc_tags_oauth.c");
    remove("out4.json");
  }
  f = fopen("../src/tests/mocks/parse/test_doc_tags_invalid.c", "w");
  if (f) {
    fprintf(f, "/**\n");
    fprintf(f, " * @securityScheme MyAuth6 [type:invalid]\n");
    fprintf(
        f, " * @securityScheme MyAuth3_invalid [type:oauth2] [flow:invalid]\n");
    fprintf(f, " */\n");
    fprintf(f, "void my_func(void) {}\n");
    fclose(f);
    char *argv2[] = {"c2openapi",
                     "../src/tests/mocks/parse/test_doc_tags_invalid.c",
                     "out5.json"};
    c2openapi_cli_main(3, argv2);
    remove("../src/tests/mocks/parse/test_doc_tags_invalid.c");
    remove("out5.json");
  }
  f = fopen("../src/tests/mocks/parse/test_doc_tags_oauth_merge.c", "w");
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
    fclose(f);
    char *argv2[] = {"c2openapi",
                     "../src/tests/mocks/parse/test_doc_tags_oauth_merge.c",
                     "out6.json"};
    c2openapi_cli_main(3, argv2);
    remove("../src/tests/mocks/parse/test_doc_tags_oauth_merge.c");
    remove("out6.json");
  }
  PASS();
}

SUITE(cli_c2openapi_suite) {
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
#endif /* TEST_CLI_C2OPENAPI_H */
