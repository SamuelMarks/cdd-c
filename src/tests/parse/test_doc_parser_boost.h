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
#include "docstrings/parse/doc_internal.h"
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

TEST test_doc_parser_extra_branches(void) {
  char dummy_buf[16];
  const char *p = NULL;
  const char *next = NULL;
  char *word = NULL;
  char *trimmed = NULL;
  char *tok_val = NULL;
  size_t klen = 0;
  enum DocSecurityType sec_type;
  enum DocSecurityIn sec_in;
  enum DocOAuthFlowType flow_type;
  struct DocMetadata meta;
  cdd_c_error_t rc;
  const char *s_abc = "abc";

  /* 1. NULL checks on scanner / helper functions */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_skip_ws("abc", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, doc_skip_ws(NULL, &p));
  ASSERT(p == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, doc_skip_ws(" \r\n", &p));
  ASSERT(*p == '\r');

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            doc_extract_word("abc", s_abc + 3, NULL, &word));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            doc_extract_word("abc", s_abc + 3, &next, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            doc_extract_rest("abc", s_abc + 3, NULL));

  memcpy(dummy_buf, "abc", 4);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_trim_segment(dummy_buf, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, doc_trim_segment(NULL, &trimmed));
  ASSERT(trimmed == NULL);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            doc_find_key_token(dummy_buf, "b", &klen, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, doc_find_key_token(NULL, NULL, &klen, &tok_val));
  ASSERT(tok_val == NULL);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            doc_parse_security_type_text("apiKey", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            doc_parse_security_in_text("query", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            doc_parse_oauth_flow_type_text("implicit", NULL));

  ASSERT_EQ(CDD_C_SUCCESS, doc_parse_security_type_text(NULL, &sec_type));
  ASSERT_EQ(DOC_SEC_UNSET, sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, doc_parse_security_in_text(NULL, &sec_in));
  ASSERT_EQ(DOC_SEC_IN_UNSET, sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, doc_parse_oauth_flow_type_text(NULL, &flow_type));
  ASSERT_EQ(DOC_OAUTH_FLOW_UNSET, flow_type);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_add_tag(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_add_tag(NULL, "tag"));
  doc_metadata_init(&meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_add_tag(&meta, NULL));
  doc_metadata_free(&meta);

  /* 2. License with both url and identifier returns invalid argument */
  {
    const char *lic_comment =
        "/**\n * @license MIT [url:https://mit.edu] [identifier:MIT]\n */";
    doc_metadata_init(&meta);
    rc = doc_parse_block(lic_comment, &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    doc_metadata_free(&meta);
  }

  /* 3. Comprehensive tags, operations, security comments with examples and OOM
   */
  {
    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              doc_parse_block("/**\n * @route POST /pets\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              doc_parse_block("/**\n * @param id [in:path]\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              doc_parse_block("/**\n * @param id [example:123]\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        doc_parse_block("/**\n * @param id [deprecated:false]\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    rc = doc_parse_block(
        "/**\n * @return 200 [summary:OK] [example:ok] [itemSchema:true]\n */",
        &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        doc_parse_block(
            "/**\n * @responseHeader 200 X-Ex [example:hdr] [required]\n */",
            &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        doc_parse_block("/**\n * @requestBody [contentType:application/json] "
                        "[example:mybody] [required:true]\n */",
                        &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              doc_parse_block("/**\n * @tagmeta mytag [summary:sum] "
                              "[description:desc] [parent:par] [kind:k]\n */",
                              &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              doc_parse_block(
                  "/**\n * @contact [name:Owner] [url:https://owner.com]\n */",
                  &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        doc_parse_block(
            "/**\n * @tagmeta mytag2 [externalDocs:https://example.com/docs] "
            "[externalDocsDescription:docdesc]\n */",
            &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        doc_parse_block(
            "/**\n * @securityScheme oauthSec [type:oauth2] "
            "[openIdConnectUrl:https://example.com/oidc] "
            "[oauth2MetadataUrl:https://example.com/meta] [flow:implicit]\n */",
            &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        doc_parse_block(
            "/**\n * @license [name:Apache] [url:https://apache.org]\n */",
            &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        doc_parse_block("/**\n * @security oauthSec read:pets, write:pets\n */",
                        &meta));
    doc_metadata_free(&meta);
  }

  /* 4. Test error percolation via g_doc_fail_skip_ws and
   * g_doc_fail_trim_segment */
  {
    extern C_CDD_EXPORT int g_doc_fail_skip_ws;
    extern C_CDD_EXPORT int g_doc_fail_trim_segment;
    char *word_out = NULL;
    char *rest_out = NULL;
    char **scopes_out = NULL;
    size_t n_scopes_out = 0;
    const char *next_ptr = NULL;
    int b_set = 0;
    int b_val = 0;
    char *ex_val = NULL;
    const char *s_sp_abc = "   abc";

    g_doc_fail_skip_ws = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              doc_extract_word("   abc", s_sp_abc + 6, &next_ptr, &word_out));
    g_doc_fail_skip_ws = 0;

    g_doc_fail_skip_ws = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              doc_extract_rest("   abc", s_sp_abc + 6, &rest_out));
    g_doc_fail_skip_ws = 0;

    g_doc_fail_trim_segment = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              doc_split_scopes("read,write", &scopes_out, &n_scopes_out));
    g_doc_fail_trim_segment = 0;

    g_doc_fail_trim_segment = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              doc_split_scopes("read,write", &scopes_out, &n_scopes_out));
    g_doc_fail_trim_segment = 0;

    g_doc_fail_trim_segment = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              doc_parse_optional_bool_attr("deprecated:true", "deprecated",
                                           &b_set, &b_val));
    g_doc_fail_trim_segment = 0;

    g_doc_fail_trim_segment = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              doc_parse_optional_example_attr("example:123", &ex_val));
    g_doc_fail_trim_segment = 0;
  }

  /* 5. Test error percolation in security schemes */
  {
    extern C_CDD_EXPORT int g_doc_fail_sec_text;
    const char *sec_line = "/**\n * @securityScheme auth1 [type:apiKey] "
                           "[in:header] [flow:implicit]\n */";

    g_doc_fail_sec_text = 1;
    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_parse_block(sec_line, &meta));
    doc_metadata_free(&meta);
    g_doc_fail_sec_text = 0;

    g_doc_fail_sec_text = 2;
    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_parse_block(sec_line, &meta));
    doc_metadata_free(&meta);
    g_doc_fail_sec_text = 0;

    g_doc_fail_sec_text = 3;
    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_parse_block(sec_line, &meta));
    doc_metadata_free(&meta);
    g_doc_fail_sec_text = 0;
  }

  /* 6. Test OOM percolation across all directives */
  {
    const char *doc_oom1 =
        "/**\n"
        " * @param id [in:path] [example:123] [deprecated:false]\n"
        " * @return 200 [summary:OK] [example:ok] [itemSchema:true]\n"
        " * @responseHeader 200 X-Ex [example:hdr] [required]\n"
        " * @requestBody [contentType:application/json] [example:mybody] "
        "[required:true]\n"
        " * @tagmeta mytag [summary:sum] [description:desc] [parent:par] "
        "[kind:k] "
        "[externalDocs:https://example.com/docs] "
        "[externalDocsDescription:docdesc]\n"
        " */";
    const char *doc_oom2 =
        "/**\n"
        " * @contact [name:Owner] [url:https://owner.com] "
        "[email:owner@example.com]\n"
        " * @license [name:Apache] [url:https://apache.org]\n"
        " * @securityScheme oauthSec [type:oauth2] "
        "[openIdConnectUrl:https://example.com/oidc] "
        "[oauth2MetadataUrl:https://example.com/meta] [flow:implicit]\n"
        " * @security oauthSec read:pets, write:pets\n"
        " */";
    const char *doc_oom3 =
        "/**\n"
        " * @serverVar varName [default:val] [description:desc] [enum:a,b,c]\n"
        " * @encoding encName [contentType:app/json] [style:form] "
        "[explode:true] [allowReserved:true]\n"
        " * @infoTitle MyTitle\n"
        " * @infoVersion 1.0.0\n"
        " * @infoSummary MySummary\n"
        " * @infoDescription MyDesc\n"
        " * @termsOfService https://example.com/tos\n"
        " */";
    doc_parse_block_with_oom(doc_oom1, &meta);
    doc_parse_block_with_oom(doc_oom2, &meta);
    doc_parse_block_with_oom(doc_oom3, &meta);
  }

  /* 7. Test skip_ws failure across directive lines */
  {
    extern C_CDD_EXPORT int g_doc_fail_skip_ws;
    int k;

    for (k = 1; k <= 4; ++k) {
      g_doc_fail_skip_ws = k;
      doc_metadata_init(&meta);
      (doc_parse_block)("/**\n * @securityScheme auth1 [type:apiKey]\n */",
                        &meta);
      doc_metadata_free(&meta);

      g_doc_fail_skip_ws = k;
      doc_metadata_init(&meta);
      (doc_parse_block)("/**\n * @param id [in:path]\n */", &meta);
      doc_metadata_free(&meta);

      g_doc_fail_skip_ws = k;
      doc_metadata_init(&meta);
      (doc_parse_block)("/**\n * @return 200 [summary:OK]\n */", &meta);
      doc_metadata_free(&meta);

      g_doc_fail_skip_ws = k;
      doc_metadata_init(&meta);
      (doc_parse_block)("/**\n * @responseHeader 200 X-Hdr [required]\n */",
                        &meta);
      doc_metadata_free(&meta);

      g_doc_fail_skip_ws = k;
      doc_metadata_init(&meta);
      (doc_parse_block)("/**\n * @link myLink [operationId:op1]\n */", &meta);
      doc_metadata_free(&meta);

      g_doc_fail_skip_ws = k;
      doc_metadata_init(&meta);
      (doc_parse_block)("/**\n * @requestBody [required:true]\n */", &meta);
      doc_metadata_free(&meta);

      g_doc_fail_skip_ws = k;
      doc_metadata_init(&meta);
      (doc_parse_block)("/**\n * @tagmeta mytag [summary:sum]\n */", &meta);
      doc_metadata_free(&meta);
    }
    g_doc_fail_skip_ws = 0;

    g_doc_fail_skip_ws = 4;
    doc_metadata_init(&meta);
    (doc_parse_block)("/**\n * @link myLink op1 [operationId:op1]\n */", &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    g_doc_fail_skip_ws = 3;
    doc_metadata_init(&meta);
    (doc_parse_block)("/**\n * @requestBody [contentType:app/json] "
                      "[example:mybody] [required:true]\n */",
                      &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;
  }

  /* 8. Single-line OOM across directives */
  {
    doc_parse_block_with_oom("/**\n * @return 200 [summary:OK]\n */", &meta);
    doc_parse_block_with_oom("/**\n * @responseHeader 200 X-Hdr\n */", &meta);
    doc_parse_block_with_oom("/**\n * @link myLink op1\n */", &meta);
    doc_parse_block_with_oom("/**\n * @securityScheme auth1\n */", &meta);
    doc_parse_block_with_oom("/**\n * @requestBody [required:true]\n */",
                             &meta);
    doc_parse_block_with_oom("/**\n * @tags a,b,c\n */", &meta);
  }

  /* 9. Incomplete / malformed directive lines (empty fields) */
  {
    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS, (doc_parse_block)("/**\n * @return\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @responseHeader\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @responseHeader 200\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS, (doc_parse_block)("/**\n * @link\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @link myLink\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @securityScheme\n */", &meta));
    doc_metadata_free(&meta);
  }

  /* 10. Tags error percolation */
  {
    extern C_CDD_EXPORT int g_doc_fail_find_key_token;
    extern C_CDD_EXPORT int g_doc_fail_skip_ws;
    extern C_CDD_EXPORT int g_doc_fail_trim_segment;
    int j;

    g_doc_fail_find_key_token = 1;
    doc_metadata_init(&meta);
    (doc_parse_block)("/**\n * @server http://localhost name=srv\n */", &meta);
    doc_metadata_free(&meta);
    g_doc_fail_find_key_token = 0;

    g_doc_fail_find_key_token = 2;
    doc_metadata_init(&meta);
    (doc_parse_block)(
        "/**\n * @server http://localhost name=srv description=desc\n */",
        &meta);
    doc_metadata_free(&meta);
    g_doc_fail_find_key_token = 0;

    g_doc_fail_skip_ws = 2;
    doc_metadata_init(&meta);
    (doc_parse_block)(
        "/**\n * @server http://localhost\n * @serverVar v1 [default:d]\n */",
        &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    g_doc_fail_skip_ws = 3;
    doc_metadata_init(&meta);
    (doc_parse_block)(
        "/**\n * @server http://localhost\n * @serverVar v1 [default:d]\n */",
        &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    g_doc_fail_skip_ws = 1;
    doc_metadata_init(&meta);
    (doc_parse_block)("/**\n * @encoding prop1 [contentType:app/json]\n */",
                      &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    g_doc_fail_skip_ws = 2;
    doc_metadata_init(&meta);
    (doc_parse_block)("/**\n * @encoding prop1 [contentType:app/json]\n */",
                      &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    g_doc_fail_skip_ws = 3;
    doc_metadata_init(&meta);
    (doc_parse_block)("/**\n * @encoding prop1 [contentType:app/json]\n */",
                      &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    /* Duplicate attributes */
    doc_metadata_init(&meta);
    (doc_parse_block)(
        "/**\n"
        " * @tagmeta t1 [summary:s1] [summary:s2] [description:d1] "
        "[description:d2] [parent:p1] [parent:p2]\n"
        " * @tagmeta t2 [kind:k1] [kind:k2] [externalDocs:u1] "
        "[externalDocs:u2] [externalDocsDescription:e1] "
        "[externalDocsDescription:e2]\n"
        " * @contact [name:n1] [name:n2] [url:u1] [url:u2] [email:e1] "
        "[email:e2]\n"
        " * @contact [name:n3]\n"
        " * @license [name:l1] [name:l2] [url:u1] [url:u2]\n"
        " * @license [name:l3] [identifier:MIT]\n"
        " */",
        &meta);
    doc_metadata_free(&meta);

    for (j = 1; j <= 6; ++j) {
      g_cdd_strdup_fail = j;
      doc_metadata_init(&meta);
      (doc_parse_block)(
          "/**\n * @tagmeta mytag [summary:s] [description:d] [parent:p] "
          "[kind:k] [externalDocs:http://ex] [externalDocsDescription:ed]\n */",
          &meta);
      doc_metadata_free(&meta);
    }
    g_cdd_strdup_fail = 0;

    for (j = 1; j <= 4; ++j) {
      g_cdd_strdup_fail = j;
      doc_metadata_init(&meta);
      (doc_parse_block)(
          "/**\n * @contact [name:Owner] [url:http://ex] [email:o@ex]\n */",
          &meta);
      doc_metadata_free(&meta);
    }
    g_cdd_strdup_fail = 0;

    for (j = 1; j <= 4; ++j) {
      g_cdd_strdup_fail = j;
      doc_metadata_init(&meta);
      (doc_parse_block)("/**\n * @server http://localhost\n * @serverVar v1 "
                        "[default:d] [enum:a] [description:desc]\n */",
                        &meta);
      doc_metadata_free(&meta);
    }
    g_cdd_strdup_fail = 0;

    g_cdd_strdup_fail = 1;
    doc_metadata_init(&meta);
    (doc_parse_block)("/**\n * @encoding prop1 [contentType:app/json]\n */",
                      &meta);
    doc_metadata_free(&meta);
    g_cdd_strdup_fail = 0;

    /* Test contact name strdup failures with url & email present */
    g_cdd_strdup_fail = 3;
    doc_metadata_init(&meta);
    (doc_parse_block)(
        "/**\n * @contact [url:http://ex] [email:o@ex] [name:Owner]\n */",
        &meta);
    doc_metadata_free(&meta);
    g_cdd_strdup_fail = 0;

    g_cdd_strdup_fail = 3;
    doc_metadata_init(&meta);
    (doc_parse_block)(
        "/**\n * @contact [url:http://ex] [email:o@ex] Owner\n */", &meta);
    doc_metadata_free(&meta);
    g_cdd_strdup_fail = 0;

    /* Test serverVar skip_ws failure */
    g_doc_fail_skip_ws = 4;
    doc_metadata_init(&meta);
    (doc_parse_block)(
        "/**\n * @server http://localhost\n * @serverVar v1 [default:d]\n */",
        &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    g_doc_fail_skip_ws = 5;
    doc_metadata_init(&meta);
    (doc_parse_block)(
        "/**\n * @server http://localhost\n * @serverVar v1 [default:d]\n */",
        &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    /* Test encoding skip_ws failure */
    g_doc_fail_skip_ws = 1;
    doc_metadata_init(&meta);
    (doc_parse_block)("/**\n * @encoding prop1 [contentType:app/json]\n */",
                      &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    g_doc_fail_skip_ws = 4;
    doc_metadata_init(&meta);
    (doc_parse_block)("/**\n * @encoding prop1 [contentType:app/json]\n */",
                      &meta);
    doc_metadata_free(&meta);
    g_doc_fail_skip_ws = 0;

    /* Test tags trim_segment failure on line 83 */
    {
      const char *s_tags = " t1";
      g_doc_fail_trim_segment = 1;
      doc_metadata_init(&meta);
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                doc_parse_tags_line(s_tags, s_tags + strlen(s_tags), &meta));
      doc_metadata_free(&meta);
      g_doc_fail_trim_segment = 0;
    }

    /* Test serverVar cleanup on skip_ws failure with description */
    {
      const char *s_srv = " http://localhost";
      const char *s_var = " v1 [description:d] [default:val]";
      doc_metadata_init(&meta);
      rc = doc_parse_server_line(s_srv, s_srv + strlen(s_srv), &meta);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, meta.n_servers);
      g_doc_fail_skip_ws = 3;
      rc = doc_parse_server_var_line(s_var, s_var + strlen(s_var), &meta);
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      doc_metadata_free(&meta);
      g_doc_fail_skip_ws = 0;
    }

    /* Test encoding bracket skip_ws failure on line 878 */
    {
      const char *s_enc = " prop1 [contentType:app/json]";
      doc_metadata_init(&meta);
      g_doc_fail_skip_ws = 5;
      ASSERT_EQ(
          CDD_C_ERROR_MEMORY,
          doc_parse_encoding_line(s_enc, s_enc + strlen(s_enc), &meta, 0));
      doc_metadata_free(&meta);
      g_doc_fail_skip_ws = 0;
    }

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        (doc_parse_block)("/**\n * @license [url:http://ex]\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        (doc_parse_block)("/**\n * @license [identifier:MIT]\n */", &meta));
    doc_metadata_free(&meta);

    /* Empty attributes and multiple content_type attributes for
     * doc_operations.c */
    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n"
                                " * @param p [format:] [format=uuid] int\n"
                                " * @return 200 [contentType:] [summary:] "
                                "[contentType=app/json] [summary=OK] desc\n"
                                " * @responseHeader 200 X-H [format:] "
                                "[contentType:] [content:] "
                                "[contentType:text/plain] "
                                "[content:application/json] hdesc\n"
                                " */",
                                &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n"
                                " * @link L [operationId:] [operationRef:] "
                                "[parameters:] [requestBody:] [summary:] "
                                "[description:] [serverUrl:] [serverName:] "
                                "[serverDescription:] ldesc\n"
                                " * @requestBody [contentType:] [content:] "
                                "[contentType:text/plain] "
                                "[content:application/json] btype\n"
                                " */",
                                &meta));
    doc_metadata_free(&meta);

    /* Test memory error in body example attribute parsing */
    {
      const char *s_body = " [example=val] type";
      doc_metadata_init(&meta);
      g_doc_fail_trim_segment = 1;
      ASSERT_EQ(
          CDD_C_ERROR_MEMORY,
          doc_parse_request_body_line(s_body, s_body + strlen(s_body), &meta));
      g_doc_fail_trim_segment = 0;
      doc_metadata_free(&meta);
    }

    /* Test trim_segment failure on link operationId */
    {
      const char *s_link = " L op1 [operationId:op1]";
      doc_metadata_init(&meta);
      g_doc_fail_trim_segment = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                doc_parse_link_line(s_link, s_link + strlen(s_link), &meta));
      g_doc_fail_trim_segment = 0;
      doc_metadata_free(&meta);
    }

    /* Test trim_segment failure on requestBody contentType and content */
    {
      const char *s_body_ct = " [contentType:app/json] type";
      const char *s_body_ct2 =
          " [contentType:text/plain] [contentType:app/json] type";
      const char *s_body_c = " [content:app/json] type";
      doc_metadata_init(&meta);
      g_doc_fail_trim_segment = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                doc_parse_request_body_line(
                    s_body_ct, s_body_ct + strlen(s_body_ct), &meta));
      g_doc_fail_trim_segment = 0;
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      g_doc_fail_trim_segment = 2;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                doc_parse_request_body_line(
                    s_body_ct2, s_body_ct2 + strlen(s_body_ct2), &meta));
      g_doc_fail_trim_segment = 0;
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      g_doc_fail_trim_segment = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                doc_parse_request_body_line(
                    s_body_c, s_body_c + strlen(s_body_c), &meta));
      g_doc_fail_trim_segment = 0;
      doc_metadata_free(&meta);
    }

    /* Test content: with no prior contentType: set */
    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        (doc_parse_block)("/**\n"
                          " * @responseHeader 200 X-H1 "
                          "[content:application/json] hdesc\n"
                          " * @requestBody [content:application/json] btype1\n"
                          " */",
                          &meta));
    doc_metadata_free(&meta);

    /* Test trim_segment failure across operation attributes */
    {
      int t;
      for (t = 1; t <= 12; ++t) {
        g_doc_fail_trim_segment = t;
        doc_metadata_init(&meta);
        (doc_parse_block)(
            "/**\n"
            " * @param p [format:uuid] int\n"
            " * @return 200 [contentType:app/json] [summary:OK] desc\n"
            " * @responseHeader 200 X-H [format:uuid] [contentType:text/plain] "
            "[content:application/json] hdesc\n"
            " */",
            &meta);
        doc_metadata_free(&meta);
      }
      for (t = 1; t <= 20; ++t) {
        g_doc_fail_trim_segment = t;
        doc_metadata_init(&meta);
        (doc_parse_block)(
            "/**\n"
            " * @link L [operationId:op1] [operationRef:ref1] "
            "[parameters:p1] [requestBody:rb1] "
            "[summary:sum] [serverUrl:url1] [serverName:name1] "
            "[serverDescription:sdesc] [description:ldesc] ldesc\n"
            " * @requestBody [contentType:text/plain] "
            "[content:application/json] btype\n"
            " */",
            &meta);
        doc_metadata_free(&meta);
      }
      g_doc_fail_trim_segment = 0;
    }

    /* Security empty line and branches */
    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @security\n */", &meta));
    doc_metadata_free(&meta);

    {
      const char *s_sec =
          " s [type:http] [description:d] [scheme:s] [bearerFormat:b] "
          "[paramName:p] [in:header] [openIdConnectUrl:u] "
          "[oauth2MetadataUrl:m] "
          "[flow:implicit] [authorizationUrl:a] [tokenUrl:t] [refreshUrl:r] "
          "[deviceAuthorizationUrl:d] [scopes:s1]";
      const char *s_sec_empty =
          " s [flow:implicit] [description:] [scheme:] [bearerFormat:] "
          "[paramName:] [openIdConnectUrl:] [oauth2MetadataUrl:] "
          "[authorizationUrl:] [tokenUrl:] [refreshUrl:] "
          "[deviceAuthorizationUrl:]";
      const char *s_sec_noflow =
          " s [authorizationUrl:http://ex] [tokenUrl:http://ex] "
          "[refreshUrl:http://ex] [deviceAuthorizationUrl:http://ex] "
          "[scopes:read]";
      int t;

      for (t = 1; t <= 15; ++t) {
        doc_metadata_init(&meta);
        g_doc_fail_trim_segment = t;
        doc_parse_security_scheme_line(s_sec, s_sec + strlen(s_sec), &meta);
        g_doc_fail_trim_segment = 0;
        doc_metadata_free(&meta);
      }

      doc_metadata_init(&meta);
      ASSERT_EQ(CDD_C_SUCCESS,
                doc_parse_security_scheme_line(
                    s_sec_empty, s_sec_empty + strlen(s_sec_empty), &meta));
      doc_metadata_free(&meta);

      doc_metadata_init(&meta);
      ASSERT_EQ(CDD_C_SUCCESS,
                doc_parse_security_scheme_line(
                    s_sec_noflow, s_sec_noflow + strlen(s_sec_noflow), &meta));
      doc_metadata_free(&meta);
    }

    /* doc_tags.c branch coverage */
    {
      const char *s_tag = " t [summary:s] [description:d] [parent:p] [kind:k] "
                          "[externalDocs:e] [externalDocsDescription:ed]";
      const char *s_con = " [name:n] [url:u] [email:e]";
      const char *s_con_rest = " John Doe";
      const char *s_lic = " [name:n] [identifier:i] [url:u]";
      const char *s_lic_rest = " MIT";
      const char *s_srv1 = " http://ex name=prod description=Production";
      const char *s_srv2 = " http://ex OnlyDescription";
      const char *s_svar = " v [default:d] [enum:a,b] [description:desc]";
      const char *s_enc = " prop [contentType:text/plain] [style:form]";
      int t;

      for (t = 1; t <= 10; ++t) {
        doc_metadata_init(&meta);
        g_doc_fail_trim_segment = t;
        doc_parse_tag_meta_line(s_tag, s_tag + strlen(s_tag), &meta);
        g_doc_fail_trim_segment = 0;
        doc_metadata_free(&meta);
      }
      for (t = 1; t <= 10; ++t) {
        doc_metadata_init(&meta);
        g_doc_fail_trim_segment = t;
        doc_parse_contact_line(s_con, s_con + strlen(s_con), &meta);
        g_doc_fail_trim_segment = 0;
        doc_metadata_free(&meta);
      }
      doc_metadata_init(&meta);
      g_doc_fail_trim_segment = 1;
      doc_parse_contact_line(s_con_rest, s_con_rest + strlen(s_con_rest),
                             &meta);
      g_doc_fail_trim_segment = 0;
      doc_metadata_free(&meta);

      for (t = 1; t <= 10; ++t) {
        doc_metadata_init(&meta);
        g_doc_fail_trim_segment = t;
        doc_parse_license_line(s_lic, s_lic + strlen(s_lic), &meta);
        g_doc_fail_trim_segment = 0;
        doc_metadata_free(&meta);
      }
      doc_metadata_init(&meta);
      g_doc_fail_trim_segment = 1;
      doc_parse_license_line(s_lic_rest, s_lic_rest + strlen(s_lic_rest),
                             &meta);
      g_doc_fail_trim_segment = 0;
      doc_metadata_free(&meta);

      for (t = 1; t <= 3; ++t) {
        doc_metadata_init(&meta);
        g_doc_fail_trim_segment = t;
        doc_parse_server_line(s_srv1, s_srv1 + strlen(s_srv1), &meta);
        g_doc_fail_trim_segment = 0;
        doc_metadata_free(&meta);
      }
      doc_metadata_init(&meta);
      g_doc_fail_trim_segment = 1;
      doc_parse_server_line(s_srv2, s_srv2 + strlen(s_srv2), &meta);
      g_doc_fail_trim_segment = 0;
      doc_metadata_free(&meta);

      for (t = 1; t <= 5; ++t) {
        doc_metadata_init(&meta);
        (doc_parse_block)("/**\n * @server http://ex\n */", &meta);
        g_doc_fail_trim_segment = t;
        doc_parse_server_var_line(s_svar, s_svar + strlen(s_svar), &meta);
        g_doc_fail_trim_segment = 0;
        doc_metadata_free(&meta);
      }
      for (t = 1; t <= 5; ++t) {
        doc_metadata_init(&meta);
        g_doc_fail_trim_segment = t;
        doc_parse_encoding_line(s_enc, s_enc + strlen(s_enc), &meta, 0);
        g_doc_fail_trim_segment = 0;
        doc_metadata_free(&meta);
      }
    }

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS, (doc_parse_block)("/**\n * @contact\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS, (doc_parse_block)("/**\n * @license\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        (doc_parse_block)("/**\n"
                          " * @tag t [summary:] [description:] [parent:] "
                          "[kind:] [externalDocs:] [externalDocsDescription:]\n"
                          " * @tag t [summary:s1] [summary:s2] "
                          "[description:d1] "
                          "[description:d2] [parent:p1] [parent:p2] [kind:k1] "
                          "[kind:k2] [externalDocs:e1] [externalDocs:e2] "
                          "[externalDocsDescription:ed1] "
                          "[externalDocsDescription:ed2]\n"
                          " */",
                          &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        (doc_parse_block)("/**\n * @contact [] [name:] [url:] [email:]\n */",
                          &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @contact [url:http://u] John Doe\n */",
                                &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @contact [url:http://u]\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS, (doc_parse_block)("/**\n * @contact [name:n1] "
                                               "[name:n2] [url:u1] [url:u2] "
                                               "[email:e1] [email:e2]\n */",
                                               &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS, (doc_parse_block)("/**\n * @license [] [name:] "
                                               "[identifier:] [url:] MIT\n */",
                                               &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @license [url:http://u] Apache\n */",
                                &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS, (doc_parse_block)("/**\n * @license [name:n1] "
                                               "[name:n2] [identifier:i1] "
                                               "[identifier:i2]\n */",
                                               &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @license [name:n1] [name:n2] [url:u1] "
                                "[url:u2]\n */",
                                &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              (doc_parse_block)("/**\n * @license [name:n1] [identifier:i1] "
                                "[url:u1]\n */",
                                &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              (doc_parse_block)("/**\n * @license [url:http://u]\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        (doc_parse_block)("/**\n"
                          " * @server http://ex name= description=\n"
                          " * @serverVar v1 [default:] [enum:] [description:]\n"
                          " */",
                          &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        (doc_parse_block)("/**\n"
                          " * @server http://ex2 MyDescriptionOnly\n"
                          " * @server http://ex3    \n"
                          " * @serverVar v2 [default:d1] [default:d2] "
                          "[enum:e1] "
                          "[enum:e2] [description:desc1] [description:desc2]\n"
                          " */",
                          &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n"
                                " * @encoding prop []\n"
                                " * @encoding prop [contentType:] [style:]\n"
                                " */",
                                &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @externalDocs\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @route GET\n * @route GET /p1\n * "
                                "@route GET /p2\n */",
                                &meta));
    doc_metadata_free(&meta);

    /* Error cases: @serverVar with no server and with no name */
    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              (doc_parse_block)("/**\n * @serverVar v\n */", &meta));
    doc_metadata_free(&meta);

    doc_metadata_init(&meta);
    ASSERT_EQ(CDD_C_SUCCESS,
              (doc_parse_block)("/**\n * @server http://ex\n * @serverVar\n */",
                                &meta));
    doc_metadata_free(&meta);
  }

  PASS();
}

SUITE(doc_parser_boost_suite) {
  RUN_TEST(test_doc_parser_100_percent_coverage_boost);
  RUN_TEST(test_doc_crlf_and_non_bracket_tags);
  RUN_TEST(test_doc_parser_extra_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DOC_PARSER_BOOST_H */
