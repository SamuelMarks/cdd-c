/**
 * @file test_integration_c2openapi_cli.h
 * @brief Integration tests for C to OpenAPI CLI and schema options.
 *
 * @author Samuel Marks
 */

#ifndef TEST_INTEGRATION_C2OPENAPI_CLI_H
#define TEST_INTEGRATION_C2OPENAPI_CLI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/format_specifiers.h"

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "routes/parse/cli.h"
/* clang-format on */

extern C_CDD_EXPORT int g_fail_io_after;

TEST test_c2o_cli_source_file_checks(void) {
  int rc;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *txt_file = NULL;
  char *no_ext_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf(&txt_file, "%s%cnotes.txt", src_dir, PATH_SEP_C);
  asprintf(&no_ext_file, "%s%cREADME", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, "int foo(void);\n");
  write_to_file(txt_file, "just some notes");
  write_to_file(no_ext_file, "no extension here");

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(txt_file);
    remove(no_ext_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free(txt_file);
    free(no_ext_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_doc_sec_unset(void) {
  int rc;
  const char *snippets[] = {
      "/**\n * @securityScheme my_bad_sec\n */\nint foo1(void);\n",
      "/**\n * @securityScheme my_bad_sec2 [type:unknownType]\n */\nint "
      "foo2(void);\n"
      "/**\n * @securityScheme my_http [type:http] [in:unknownIn]\n */\nint "
      "foo3(void);\n",
      "/**\n * @securityScheme my_apikey [type:apiKey] [in:unknownIn]\n "
      "*/\nint foo4(void);\n",
      "/**\n * @securityScheme my_apikey2 [type:apiKey]\n */\nint "
      "foo5(void);\n",
      "/**\n * @securityScheme my_oauth2 [type:oauth2] "
      "[flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device] "
      "[tokenUrl:https://auth.com/token] [refreshUrl:https://auth.com/refresh] "
      "[scopes:scope1=Scope1]\n */\nint foo6(void);\n",
      "/**\n * @securityScheme my_oauth2_bad [type:oauth2] "
      "[flow:unknownFlow]\n */\nint foo7(void);\n",
      "/**\n * @securityScheme my_oauth2_unset [type:oauth2]\n */\nint "
      "foo8(void);\n",
      "/**\n * @securityScheme my_mutual [type:mutualTLS]\n */\nint "
      "foo9(void);\n",
      "/**\n * @securityScheme my_openid [type:openIdConnect] "
      "[openIdConnectUrl:https://auth.com/openid]\n */\nint foo10(void);\n",
      "/**\n * @securityScheme my_apikey_query [type:apiKey] [in:query] "
      "[name:foo]\n */\nint foo11(void);\n",
      "/**\n * @securityScheme my_apikey_cookie [type:apiKey] [in:cookie] "
      "[name:foo]\n */\nint foo12(void);\n",
      "/**\n * @securityScheme my_oauth2_implicit [type:oauth2] "
      "[flow:implicit] [authorizationUrl:https://auth.com/auth]\n */\nint "
      "foo13(void);\n",
      "/**\n * @securityScheme my_oauth2_password [type:oauth2] "
      "[flow:password] [tokenUrl:https://auth.com/token]\n */\nint "
      "foo14(void);\n",
      "/**\n * @securityScheme my_oauth2_client [type:oauth2] "
      "[flow:clientCredentials] [tokenUrl:https://auth.com/token]\n */\nint "
      "foo15(void);\n",
      "/**\n * @securityScheme my_oauth2_auth [type:oauth2] "
      "[flow:authorizationCode] [authorizationUrl:https://auth.com/auth] "
      "[tokenUrl:https://auth.com/token]\n */\nint foo16(void);\n"};

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  {
    size_t i;
    for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
      char *c_file = NULL;
      asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
               (unsigned long)i);
      write_to_file(c_file, snippets[i]);
      free((void *)(size_t)c_file);
    }

    {
      char *argv[3]; /* ... */

      argv[0] = (char *)(size_t)(size_t) "c2openapi";
      argv[1] = (char *)(size_t)src_dir;
      argv[2] = (char *)(size_t)out_json;
      rc = c2openapi_cli_main(3, argv);
      ASSERT_EQ(0, rc);

      for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
        char *c_file = NULL;
        asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
                 (unsigned long)i);
        remove(c_file);
        free((void *)(size_t)c_file);
      }
      remove(out_json);
      rmdir(src_dir);
      free((void *)(size_t)out_json);
      free((void *)(size_t)src_dir);
      free((void *)(size_t)tmp_dir);
      g_fail_io_after = -1;

      (void)rc;
      PASS();
    }
  }
}

TEST test_c2o_cli_spec_has_tag_nulls(void) {
  int rc;
  const char *src = "/**\n"
                    " * @tag duplicated\n"
                    " * @tag duplicated\n"
                    " */\n"
                    "int foo(void);\n";

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_mappings_errors_find(void) {
  int rc;
  const char src[] = {
      47,  42,  42,  10,  32,  42,  32,  71,  76,  79,  66,  65,  76,  32,  77,
      69,  84,  65,  58,  10,  32,  42,  32,  64,  115, 101, 99,  117, 114, 105,
      116, 121, 83,  99,  104, 101, 109, 101, 32,  109, 121, 95,  104, 116, 116,
      112, 32,  91,  116, 121, 112, 101, 58,  104, 116, 116, 112, 93,  32,  91,
      115, 99,  104, 101, 109, 101, 58,  98,  101, 97,  114, 101, 114, 93,  32,
      91,  98,  101, 97,  114, 101, 114, 70,  111, 114, 109, 97,  116, 58,  74,
      87,  84,  93,  10,  32,  42,  32,  64,  115, 101, 99,  117, 114, 105, 116,
      121, 83,  99,  104, 101, 109, 101, 32,  109, 121, 95,  104, 116, 116, 112,
      32,  91,  116, 121, 112, 101, 58,  104, 116, 116, 112, 93,  32,  91,  115,
      99,  104, 101, 109, 101, 58,  98,  101, 97,  114, 101, 114, 93,  32,  91,
      98,  101, 97,  114, 101, 114, 70,  111, 114, 109, 97,  116, 58,  74,  87,
      84,  93,  10,  32,  42,  32,  64,  115, 101, 99,  117, 114, 105, 116, 121,
      83,  99,  104, 101, 109, 101, 32,  109, 121, 95,  111, 97,  117, 116, 104,
      50,  32,  91,  116, 121, 112, 101, 58,  111, 97,  117, 116, 104, 50,  93,
      32,  91,  102, 108, 111, 119, 58,  100, 101, 118, 105, 99,  101, 65,  117,
      116, 104, 111, 114, 105, 122, 97,  116, 105, 111, 110, 93,  32,  91,  97,
      117, 116, 104, 111, 114, 105, 122, 97,  116, 105, 111, 110, 85,  114, 108,
      58,  104, 116, 116, 112, 115, 58,  47,  47,  97,  117, 116, 104, 46,  99,
      111, 109, 47,  97,  117, 116, 104, 93,  32,  91,  116, 111, 107, 101, 110,
      85,  114, 108, 58,  104, 116, 116, 112, 115, 58,  47,  47,  97,  117, 116,
      104, 46,  99,  111, 109, 47,  116, 111, 107, 101, 110, 93,  32,  91,  114,
      101, 102, 114, 101, 115, 104, 85,  114, 108, 58,  104, 116, 116, 112, 115,
      58,  47,  47,  97,  117, 116, 104, 46,  99,  111, 109, 47,  114, 101, 102,
      114, 101, 115, 104, 93,  32,  91,  115, 99,  111, 112, 101, 115, 58,  115,
      99,  111, 112, 101, 49,  61,  83,  99,  111, 112, 101, 49,  93,  10,  32,
      42,  47,  10,  105, 110, 116, 32,  102, 111, 111, 40,  118, 111, 105, 100,
      41,  59,  10,  0};

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_set_str_mismatch(void) {
  int rc;
  const char *src = "/**\n"
                    " * @securityScheme my_http [type:http] [scheme:bearer]\n"
                    " * @securityScheme my_http [type:http] [scheme:basic]\n"
                    " */\n"
                    "int foo17(void);\n";

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_server_variables(void) {
  int rc;
  const char *src = "/**\n"
                    " * GLOBAL META:\n"
                    " * @server https://api.com [description:prod]\n"
                    " * @serverVar env [default:prod] [enum:prod,dev]\n"
                    " * @server https://api.com [description:mismatch_fail]\n"
                    " */\n"
                    "int foo18(void);\n";

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_server_variables_validation(void) {
  int rc;
  const char *src =
      "/**\n"
      " * GLOBAL META:\n"
      " * @server https://api.com [description:prod]\n"
      " * @serverVar env [default:prod] [enum:prod,dev] [description:desc]\n"
      " * @serverVar bad [default:wrong] [enum:prod,dev] [description:fail]\n"
      " */\n"
      "int foo19(void);\n";

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_merge_oauth_scopes(void) {
  int rc;
  const char src[] = {
      47,  42,  42,  10,  32,  42,  32,  71,  76,  79,  66,  65,  76,  32,  77,
      69,  84,  65,  58,  10,  32,  42,  32,  64,  115, 101, 99,  117, 114, 105,
      116, 121, 83,  99,  104, 101, 109, 101, 32,  109, 101, 114, 103, 101, 95,
      111, 97,  117, 116, 104, 32,  91,  116, 121, 112, 101, 58,  111, 97,  117,
      116, 104, 50,  93,  32,  91,  102, 108, 111, 119, 58,  105, 109, 112, 108,
      105, 99,  105, 116, 93,  32,  91,  97,  117, 116, 104, 111, 114, 105, 122,
      97,  116, 105, 111, 110, 85,  114, 108, 58,  104, 116, 116, 112, 115, 58,
      47,  47,  97,  117, 116, 104, 46,  99,  111, 109, 47,  97,  117, 116, 104,
      93,  32,  91,  115, 99,  111, 112, 101, 115, 58,  114, 101, 97,  100, 44,
      119, 114, 105, 116, 101, 93,  10,  32,  42,  32,  64,  115, 101, 99,  117,
      114, 105, 116, 121, 83,  99,  104, 101, 109, 101, 32,  109, 101, 114, 103,
      101, 95,  111, 97,  117, 116, 104, 32,  91,  116, 121, 112, 101, 58,  111,
      97,  117, 116, 104, 50,  93,  32,  91,  102, 108, 111, 119, 58,  105, 109,
      112, 108, 105, 99,  105, 116, 93,  32,  91,  97,  117, 116, 104, 111, 114,
      105, 122, 97,  116, 105, 111, 110, 85,  114, 108, 58,  104, 116, 116, 112,
      115, 58,  47,  47,  97,  117, 116, 104, 46,  99,  111, 109, 47,  97,  117,
      116, 104, 93,  32,  91,  115, 99,  111, 112, 101, 115, 58,  114, 101, 97,
      100, 44,  97,  100, 109, 105, 110, 93,  10,  32,  42,  32,  64,  115, 101,
      99,  117, 114, 105, 116, 121, 83,  99,  104, 101, 109, 101, 32,  109, 101,
      114, 103, 101, 95,  111, 97,  117, 116, 104, 32,  91,  116, 121, 112, 101,
      58,  111, 97,  117, 116, 104, 50,  93,  32,  91,  102, 108, 111, 119, 58,
      112, 97,  115, 115, 119, 111, 114, 100, 93,  32,  91,  116, 111, 107, 101,
      110, 85,  114, 108, 58,  104, 116, 116, 112, 115, 58,  47,  47,  97,  117,
      116, 104, 46,  99,  111, 109, 47,  116, 111, 107, 101, 110, 93,  10,  32,
      42,  47,  10,  105, 110, 116, 32,  102, 111, 111, 50,  48,  40,  118, 111,
      105, 100, 41,  59,  10,  0};

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_oauth_validation_errors(void) {
  int rc;
  const char *snippets[] = {
      "/**\n * @securityScheme oauth_bad1 [type:oauth2] [flow:implicit]\n "
      "*/\nint foo21(void);\n" /* Missing authorizationUrl */
      "/**\n * @securityScheme oauth_bad2 [type:oauth2] [flow:password]\n "
      "*/\nint foo22(void);\n", /* Missing tokenUrl */
      "/**\n * @securityScheme oauth_bad3 [type:oauth2] "
      "[flow:clientCredentials]\n */\nint foo23(void);\n", /* Missing tokenUrl
                                                            */
      "/**\n * @securityScheme oauth_bad4 [type:oauth2] "
      "[flow:authorizationCode] [authorizationUrl:https://auth.com/auth]\n "
      "*/\nint foo24(void);\n", /* Missing tokenUrl */
      "/**\n * @securityScheme oauth_bad5 [type:oauth2] "
      "[flow:authorizationCode] [tokenUrl:https://auth.com/token]\n */\nint "
      "foo25(void);\n", /* Missing authUrl */
      "/**\n * @securityScheme oauth_bad6 [type:oauth2] "
      "[flow:deviceAuthorization] [tokenUrl:https://auth.com/token]\n */\nint "
      "foo26(void);\n" /* Missing deviceAuthorizationUrl */
  };

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  {
    size_t i;
    for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
      char *c_file = NULL;
      asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
               (unsigned long)i);
      write_to_file(c_file, snippets[i]);
      free((void *)(size_t)c_file);
    }

    {
      char *argv[3]; /* ... */

      argv[0] = (char *)(size_t)(size_t) "c2openapi";
      argv[1] = (char *)(size_t)src_dir;
      argv[2] = (char *)(size_t)out_json;
      rc = c2openapi_cli_main(3, argv);
      ASSERT_EQ(0, rc);

      for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
        char *c_file = NULL;
        asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
                 (unsigned long)i);
        remove(c_file);
        free((void *)(size_t)c_file);
      }
      remove(out_json);
      rmdir(src_dir);
      free((void *)(size_t)out_json);
      free((void *)(size_t)src_dir);
      free((void *)(size_t)tmp_dir);
      g_fail_io_after = -1;

      (void)rc;
      PASS();
    }
  }
}

TEST test_c2o_cli_merge_oauth_flow_collisions(void) {
  int rc;
  const char *snippets[] = {
      "/**\n * GLOBAL META:\n * @securityScheme merge_oauth [type:oauth2] "
      "[flow:implicit] [authorizationUrl:https://auth.com/auth1]\n * "
      "@securityScheme merge_oauth [type:oauth2] [flow:implicit] "
      "[authorizationUrl:https://auth.com/auth2]\n */\nint foo27(void);\n",
      "/**\n * GLOBAL META:\n * @securityScheme merge_oauth [type:oauth2] "
      "[flow:password] [tokenUrl:https://auth.com/token1]\n * @securityScheme "
      "merge_oauth [type:oauth2] [flow:password] "
      "[tokenUrl:https://auth.com/token2]\n */\nint foo28(void);\n",
      "/**\n * GLOBAL META:\n * @securityScheme merge_oauth [type:oauth2] "
      "[flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device1] "
      "[tokenUrl:https://auth.com/token]\n * @securityScheme merge_oauth "
      "[type:oauth2] [flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device2] "
      "[tokenUrl:https://auth.com/token]\n */\nint foo29(void);\n",
      "/**\n * GLOBAL META:\n * @securityScheme merge_oauth [type:oauth2] "
      "[flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device] "
      "[refreshUrl:https://auth.com/refresh1] "
      "[tokenUrl:https://auth.com/token]\n * @securityScheme merge_oauth "
      "[type:oauth2] [flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device] "
      "[refreshUrl:https://auth.com/refresh2] "
      "[tokenUrl:https://auth.com/token]\n */\nint foo30(void);\n"};

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  {
    size_t i;
    for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
      char *c_file = NULL;
      asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
               (unsigned long)i);
      write_to_file(c_file, snippets[i]);
      free((void *)(size_t)c_file);
    }

    {
      char *argv[3]; /* ... */

      argv[0] = (char *)(size_t)(size_t) "c2openapi";
      argv[1] = (char *)(size_t)src_dir;
      argv[2] = (char *)(size_t)out_json;
      rc = c2openapi_cli_main(3, argv);
      ASSERT_EQ(0, rc);

      for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
        char *c_file = NULL;
        asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
                 (unsigned long)i);
        remove(c_file);
        free((void *)(size_t)c_file);
      }
      remove(out_json);
      rmdir(src_dir);
      free((void *)(size_t)out_json);
      free((void *)(size_t)src_dir);
      free((void *)(size_t)tmp_dir);
      g_fail_io_after = -1;

      (void)rc;
      PASS();
    }
  }
}

SUITE(integration_c2openapi_cli_suite) {
  RUN_TEST(test_c2o_cli_source_file_checks);
  RUN_TEST(test_c2o_cli_doc_sec_unset);
  RUN_TEST(test_c2o_cli_spec_has_tag_nulls);
  RUN_TEST(test_c2o_cli_mappings_errors_find);
  RUN_TEST(test_c2o_cli_set_str_mismatch);
  RUN_TEST(test_c2o_cli_server_variables);
  RUN_TEST(test_c2o_cli_server_variables_validation);
  RUN_TEST(test_c2o_cli_merge_oauth_scopes);
  RUN_TEST(test_c2o_cli_oauth_validation_errors);
  RUN_TEST(test_c2o_cli_merge_oauth_flow_collisions);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_INTEGRATION_C2OPENAPI_CLI_H */
