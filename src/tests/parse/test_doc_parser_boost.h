/**
 * @file test_doc_parser_boost.h
 * @brief Unit tests for Documentation Comment Parser.
 *
 * @author Samuel Marks
 */

#ifndef TEST_DOC_PARSER_BOOST_H
#define TEST_DOC_PARSER_BOOST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/safe_crt.h"
#include "docstrings/parse/doc.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT int g_cdd_alloc_fail;

#ifndef DOC_PARSE_BLOCK_WITH_OOM_DEFINED
#define DOC_PARSE_BLOCK_WITH_OOM_DEFINED
static int doc_parse_block_with_oom(const char *comment,
                                    struct DocMetadata *meta) {
  int i;
  for (i = 1; i < 50; ++i) {
    struct DocMetadata tmp;
    g_cdd_alloc_fail = i;
    if (doc_metadata_init(&tmp) == 0) {
      (doc_parse_block)(comment, &tmp);
      doc_metadata_free(&tmp);
    }
  }
  g_cdd_alloc_fail = 0;
  for (i = 1; i < 50; ++i) {
    struct DocMetadata tmp;
    g_cdd_strdup_fail = i;
    if (doc_metadata_init(&tmp) == 0) {
      (doc_parse_block)(comment, &tmp);
      doc_metadata_free(&tmp);
    }
  }
  g_cdd_strdup_fail = 0;
  return (doc_parse_block)(comment, meta);
}
#endif
#ifndef doc_parse_block
#define doc_parse_block(comment, meta) doc_parse_block_with_oom(comment, meta)
#endif

TEST test_doc_parser_100_percent_coverage_boost(void) {
  struct DocMetadata meta;
  cdd_c_error_t rc;
  int b_val = 0;
  enum DocParamStyle st_val = DOC_PARAM_STYLE_UNSET;
  extern C_CDD_EXPORT int g_cdd_fail_stricmp;
  extern C_CDD_EXPORT int g_doc_fail_parse_optional_bool_attr;

  const char backslash_comment[] = "/*\n"
                                   " * \\route GET /api/v1/test\n"
                                   " * \\param id int\n"
                                   " * \\returns 200 ok\n"
                                   " * \\brief summary text\n"
                                   " * \\details description text\n"
                                   " */";

  const char lowercase_comment1[] =
      "/**\n"
      " * @route GET /lower\n"
      " * @operationid lowerOp\n"
      " * @tagmeta lowerTag [name=lowerTag] [description=desc]\n"
      " * @externaldocs [url=https://docs.example.com]\n"
      " * @securityscheme secA [type:apiKey] [paramName=api_key] [in=header]\n"
      " * @server https://api.example.com\n"
      " * @servervar port [default=8080]\n"
      " */";

  const char lowercase_comment2[] =
      "/**\n"
      " * @requestbody [required] [contentType=application/json]\n"
      " * @prefixencoding pre1 [contentType=application/json]\n"
      " * @itemencoding it1 [contentType=application/json]\n"
      " * @jsonschemadialect https://json-schema.org/draft/2020-12/schema\n"
      " * @infotitle Title\n"
      " * @infoversion 1.0.0\n"
      " * @infosummary Summary\n"
      " * @infodescription Description\n"
      " * @termsofservice https://example.com/tos\n"
      " * @responseheader 200 X-Header [type=string]\n"
      " */";

  const char slash_comment[] = "/// @route GET /triple\n"
                               "/// @param x int\n"
                               "// @return 200 ok\n";

  const char empty_attr_comment1[] =
      "/**\n"
      " * @tagmeta t1 [summary=] [description=] [parent=] [kind=] "
      "[externalDocsUrl=] [externalDocsDescription=]\n"
      " * @contact Support [name=] [email=] [url=]\n"
      " * @license MIT [name=] [url=] [identifier=]\n"
      " * @responseheader 200 H1 [description=] [format=]\n"
      " * @link 200 L1 [operationId=] [operationRef=] [summary=] "
      "[description=]\n"
      " */";

  const char empty_attr_comment2[] =
      "/**\n"
      " * @param p int [description=] [format=] [contentType=]\n"
      " * @return 200 [description=]\n"
      " * @server https://example.com [name=] [description=]\n"
      " * @servervar v1 [default=8080] [description=] [enum=]\n"
      " * @encoding e1 [contentType=]\n"
      " * @requestbody [contentType=] [description=]\n"
      " */";

  const char empty_attr_comment3[] =
      "/**\n"
      " * @link 200 L1 [serverUrl=] [serverName=] [serverDescription=]\n"
      " * @securityscheme sec1 [type=apiKey] [name=] [in=] [scheme=] "
      "[bearerFormat=] [openIdConnectUrl=]\n"
      " * @securityscheme sec2 [oauth2MetadataUrl=] [flow=] "
      "[authorizationUrl=] [tokenUrl=] [refreshUrl=] "
      "[deviceAuthorizationUrl=] [scopes=]\n"
      " */";

  const char delim_comment1[] =
      "/**\n"
      " * @contact [name:cname] [email:c@example.com] [url:http://contact]\n"
      " * @license [name:MIT] [url:http://mit]\n"
      " * @tagmeta t2 [name:t2] [summary:s] [description:d] [parent:p] "
      "[kind:k] "
      "[externalDocsUrl:http://doc] [externalDocsDescription:ed]\n"
      " * @link 200 L2 [operationId:op2] [operationRef:ref2] [summary:sum2] "
      "[description:desc2]\n"
      " */";

  const char delim_comment2[] =
      "/**\n"
      " * @link 200 L2 [serverUrl:http://srv] [serverName:sn] "
      "[serverDescription:sd]\n"
      " * @securityscheme sec2 [type:apiKey] [paramName:k2] [in:query]\n"
      " * @securityscheme sec3 [type:oauth2] [flow:authorizationCode] "
      "[authorizationUrl:http://auth] [tokenUrl:http://token] "
      "[refreshUrl:http://ref] [deviceAuthorizationUrl:http://dev] "
      "[scopes:read]\n"
      " * @encoding e2 [contentType:application/xml] [style:pipeDelimited]\n"
      " */";

  const char itemschema_comment[] = "/**\n"
                                    " * @param p1 array [itemSchema:true]\n"
                                    " * @param p2 array [itemSchema=true]\n"
                                    " * @return 200 array [itemSchema:true]\n"
                                    " * @return 201 array [itemSchema=true]\n"
                                    " * @requestbody [itemSchema:true]\n"
                                    " * @requestbody [itemSchema=true]\n"
                                    " */";

  const char sec_types_comment[] =
      "/**\n"
      " * @securityscheme s1 [type:mutualTLS]\n"
      " * @securityscheme s2 [type:unknownType]\n"
      " * @securityscheme s3 [type:apiKey] [paramName:p] [in:cookie]\n"
      " * @securityscheme s4 [type:oauth2] [flow:password]\n"
      " * @securityscheme s5 [type:oauth2] [flow:clientCredentials]\n"
      " * @securityscheme s6 [type:oauth2] [flow:deviceAuthorization]\n"
      " */";

  /* 1. parse_bool_text branches */
  ASSERT_EQ(CDD_C_SUCCESS, parse_bool_text_test("1", &b_val));
  ASSERT_EQ(1, b_val);
  ASSERT_EQ(CDD_C_SUCCESS, parse_bool_text_test("true", &b_val));
  ASSERT_EQ(1, b_val);
  ASSERT_EQ(CDD_C_SUCCESS, parse_bool_text_test("yes", &b_val));
  ASSERT_EQ(1, b_val);
  ASSERT_EQ(CDD_C_SUCCESS, parse_bool_text_test("0", &b_val));
  ASSERT_EQ(0, b_val);
  ASSERT_EQ(CDD_C_SUCCESS, parse_bool_text_test("false", &b_val));
  ASSERT_EQ(0, b_val);
  ASSERT_EQ(CDD_C_SUCCESS, parse_bool_text_test("no", &b_val));
  ASSERT_EQ(0, b_val);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, parse_bool_text_test("invalid", &b_val));

  g_cdd_fail_stricmp = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_bool_text_test("x", &b_val));
  g_cdd_fail_stricmp = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_bool_text_test("x", &b_val));
  g_cdd_fail_stricmp = 3;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_bool_text_test("x", &b_val));
  g_cdd_fail_stricmp = 4;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_bool_text_test("x", &b_val));

  /* 2. parse_style_text branches */
  ASSERT_EQ(CDD_C_SUCCESS, parse_style_text_test("form", &st_val));
  ASSERT_EQ(CDD_C_SUCCESS, parse_style_text_test("simple", &st_val));
  ASSERT_EQ(CDD_C_SUCCESS, parse_style_text_test("matrix", &st_val));
  ASSERT_EQ(CDD_C_SUCCESS, parse_style_text_test("label", &st_val));
  ASSERT_EQ(CDD_C_SUCCESS, parse_style_text_test("spaceDelimited", &st_val));
  ASSERT_EQ(CDD_C_SUCCESS, parse_style_text_test("pipeDelimited", &st_val));
  ASSERT_EQ(CDD_C_SUCCESS, parse_style_text_test("deepObject", &st_val));
  ASSERT_EQ(CDD_C_SUCCESS, parse_style_text_test("cookie", &st_val));

  g_cdd_fail_stricmp = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_style_text_test("x", &st_val));
  g_cdd_fail_stricmp = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_style_text_test("x", &st_val));
  g_cdd_fail_stricmp = 3;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_style_text_test("x", &st_val));
  g_cdd_fail_stricmp = 4;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_style_text_test("x", &st_val));
  g_cdd_fail_stricmp = 5;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_style_text_test("x", &st_val));
  g_cdd_fail_stricmp = 6;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_style_text_test("x", &st_val));
  g_cdd_fail_stricmp = 7;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_style_text_test("x", &st_val));
  g_cdd_fail_stricmp = 8;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_style_text_test("x", &st_val));

  /* 3. Parse block comments with variants */
  doc_metadata_init(&meta);
  rc = doc_parse_block(backslash_comment, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(lowercase_comment1, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(lowercase_comment2, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(slash_comment, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(empty_attr_comment1, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(empty_attr_comment2, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(empty_attr_comment3, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(delim_comment1, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(delim_comment2, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(itemschema_comment, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(sec_types_comment, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  /* 4. parse_optional_bool_attr failure percolation */
  {
    const char c_hdr[] = "/**\n * @responseheader 200 H [required]\n */";
    const char c_param_exp[] = "/**\n * @param p [explode]\n */";
    const char c_param_res[] = "/**\n * @param p [allowReserved]\n */";
    const char c_param_emp[] = "/**\n * @param p [allowEmptyValue]\n */";
    const char c_param_dep[] = "/**\n * @param p [deprecated]\n */";
    const char c_sec_dep[] = "/**\n * @securityscheme sec [deprecated]\n */";
    const char c_enc_exp[] = "/**\n * @encoding enc [explode]\n */";
    const char c_enc_res[] = "/**\n * @encoding enc [allowReserved]\n */";
    const char c_rb_req[] = "/**\n * @requestbody [required]\n */";

    doc_metadata_init(&meta);
    g_doc_fail_parse_optional_bool_attr = 1;
    rc = (doc_parse_block)(c_hdr, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    g_doc_fail_parse_optional_bool_attr = 1;
    rc = (doc_parse_block)(c_param_exp, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    g_doc_fail_parse_optional_bool_attr = 2;
    rc = (doc_parse_block)(c_param_res, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    g_doc_fail_parse_optional_bool_attr = 3;
    rc = (doc_parse_block)(c_param_emp, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    g_doc_fail_parse_optional_bool_attr = 4;
    rc = (doc_parse_block)(c_param_dep, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    g_doc_fail_parse_optional_bool_attr = 1;
    rc = (doc_parse_block)(c_sec_dep, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    g_doc_fail_parse_optional_bool_attr = 1;
    rc = (doc_parse_block)(c_enc_exp, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    g_doc_fail_parse_optional_bool_attr = 2;
    rc = (doc_parse_block)(c_enc_res, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    g_doc_fail_parse_optional_bool_attr = 1;
    rc = (doc_parse_block)(c_rb_req, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    doc_metadata_free(&meta);
  }

  /* 5. Remaining corner cases & branch triggers */
  {
    int set = 0;
    int val = 0;
    char *ex_out = NULL;
    char *tok_out = NULL;
    char tok_buf[64];
    size_t klen = 0;
    enum DocOAuthFlowType flow = DOC_OAUTH_FLOW_UNSET;
    const char c_more1[] =
        "/**\n"
        " * @tagmeta [summary:s]\n"
        " * @tags a,b,\n"
        " * @tagmeta t1 [summary:] [description:] [parent:] [kind:] "
        "[externalDocs:] [externalDocsDescription:]\n"
        " * @link 200 L1 [parameters:] [requestBody:] [summary:] "
        "[description:]\n"
        " * @param p [contentType:] [itemSchema]\n"
        " * @return 200 [contentType:] [summary:] [itemSchema]\n"
        " * @requestbody [contentType:] [content:] [description:] "
        "[itemSchema]\n"
        " */";

    const char c_more2a[] =
        "/**\n"
        " * @securityscheme sec_noflow [authorizationUrl:http://auth] "
        "[tokenUrl:http://tok] [refreshUrl:http://ref] "
        "[deviceAuthorizationUrl:http://dev] [scopes:read,,write]\n"
        " * @securityscheme sec_emp [type:apiKey] [description:] [paramName:] "
        "[scheme:] [bearerFormat:]\n"
        " */";

    const char c_more2b[] =
        "/**\n"
        " * @securityscheme sec_dupe [flow:implicit] [authorizationUrl:u1] "
        "[authorizationUrl:u2] [tokenUrl:t1] [tokenUrl:t2] [refreshUrl:r1] "
        "[refreshUrl:r2] [deviceAuthorizationUrl:d1] "
        "[deviceAuthorizationUrl:d2]\n"
        " */";

    const char c_more3[] =
        "/**\n"
        " * @server https://example.com/api\n"
        " * @servervar v1 [default=8080] [description=desc1] Plain text desc\n"
        " * @server http://api description:desc name:n\n"
        " * @server http://api name:only\n"
        " * @server http://api description:only\n"
        " * @server http://api   \n"
        " * @encoding [contentType:application/json]\n"
        " * @encoding enc [unclosed\n"
        " * @encoding enc [style:invalid_style] [style=form]\n"
        " * @unknown_directive arg\n"
        " */";

    const char c_decorators[] = "/\n"
                                "//\n"
                                "/*\n"
                                "*\n"
                                "*/\n"
                                "/* comment\n"
                                "* comment\n"
                                "// comment\n"
                                "/// comment\n"
                                "*/\n";

    char bool_buf[64];
    CDD_STRCPY(bool_buf, sizeof(bool_buf), "required_field");
    parse_optional_bool_attr_test(bool_buf, "required", &set, &val);
    CDD_STRCPY(bool_buf, sizeof(bool_buf), "required=false");
    parse_optional_bool_attr_test(bool_buf, "required", &set, &val);
    CDD_STRCPY(bool_buf, sizeof(bool_buf), "required:notabool");
    parse_optional_bool_attr_test(bool_buf, "required", &set, &val);

    CDD_STRCPY(bool_buf, sizeof(bool_buf), "example:");
    parse_optional_example_attr_test(bool_buf, &ex_out);

    parse_oauth_flow_type_text_test(NULL, &flow);

    CDD_STRCPY(tok_buf, sizeof(tok_buf), "name:val");
    find_key_token_test(tok_buf, "name", NULL, &tok_out);
    find_key_token_test(tok_buf, "name", &klen, &tok_out);
    CDD_STRCPY(tok_buf, sizeof(tok_buf), "name=val");
    find_key_token_test(tok_buf, "name", &klen, &tok_out);
    CDD_STRCPY(tok_buf, sizeof(tok_buf), "xname=val");
    find_key_token_test(tok_buf, "name", &klen, &tok_out);
    CDD_STRCPY(tok_buf, sizeof(tok_buf), "name_other");
    find_key_token_test(tok_buf, "name", &klen, &tok_out);
    find_key_token_test(NULL, "name", &klen, &tok_out);
    CDD_STRCPY(tok_buf, sizeof(tok_buf), "s");
    find_key_token_test(tok_buf, NULL, &klen, &tok_out);

    doc_metadata_init(&meta);
    rc = (doc_parse_block)(c_more1, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    rc = (doc_parse_block)(c_more2a, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    rc = (doc_parse_block)("/**\n * @servervar v_orphaned [default=8080]\n */",
                           &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    rc = (doc_parse_block)(c_more2b, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    rc = (doc_parse_block)(c_more3, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    rc = (doc_parse_block)(c_decorators, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_metadata_free(&meta);

    {
      const char c_final_push1[] =
          "@route GET /direct_route\n"
          "/not_comment\n"
          "/**\n"
          " * @contact [] [unknown:val]\n"
          " * @license [] [name:Apache] [unknown:val]\n"
          " * @link 200 L [unknown:val]\n"
          " * @server https://example.com/api\n"
          " * @servervar v [default=1] [unknown:val]\n"
          " */";

      const char c_final_push2[] =
          "/**\n"
          " * @responseheader 200 H [contentType:] [content:]\n"
          " * @param p [format:] [itemSchema:false]\n"
          " * @return 200 [itemSchema=true]\n"
          " * @securityscheme sec_empflow [flow:implicit] [authorizationUrl=] "
          "[tokenUrl=] [refreshUrl=] [deviceAuthorizationUrl=] [scopes=]\n"
          " * @server http://api name: description:\n"
          " * @encoding enc [style:]\n"
          " */";

      struct DocOAuthScope *sc = NULL;
      size_t n_sc = 0;

      parse_oauth_scopes_test("read, ,write", &sc, &n_sc);
      if (sc) {
        size_t s_i;
        for (s_i = 0; s_i < n_sc; ++s_i) {
          free(sc[s_i].name);
        }
        free(sc);
      }

      doc_metadata_init(&meta);
      rc = (doc_parse_block)(c_final_push1, &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      rc = (doc_parse_block)(c_final_push2, &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      rc = (doc_parse_block)("/**\n * @server http://api\n * @servervar v2 "
                             "[default:]\n */",
                             &meta);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      rc = (doc_parse_block)("/**\n * @server\n */", &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      rc = (doc_parse_block)("/**\n * @param p [itemSchema=true]\n */", &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      rc = (doc_parse_block)("/**\n * @encoding myProp "
                             "[contentType:application/json]\n */",
                             &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      rc = (doc_parse_block)("/**\n * @encoding enc [unclosed_attr\n * "
                             "@encoding enc2 [contentType:application/json]\n "
                             "*/",
                             &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      rc = (doc_parse_block)("/**\n * @tagmeta\n * @tagmeta \"\"\n */", &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      rc = (doc_parse_block)("/**\n * @encoding myProp\n */", &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      rc = (doc_parse_block)("/**\n * @encoding enc [no_bracket", &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      doc_metadata_free(&meta);
    }
  }

  PASS();
}

TEST test_doc_crlf_and_non_bracket_tags(void) {
  struct DocMetadata meta;
  const char *comment = "/**\r\n"
                        " * @tagmeta mytag extra_text_not_bracket\r\n"
                        " * @securityscheme MyAuth extra_text_not_bracket\r\n"
                        " */";
  doc_metadata_init(&meta);
  ASSERT_EQ(CDD_C_SUCCESS, doc_parse_block(comment, &meta));
  doc_metadata_free(&meta);
  PASS();
}

SUITE(doc_parser_boost_suite) {
  RUN_TEST(test_doc_parser_100_percent_coverage_boost);
  RUN_TEST(test_doc_crlf_and_non_bracket_tags);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DOC_PARSER_BOOST_H */
