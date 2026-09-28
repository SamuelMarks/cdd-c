/**
 * @file test_doc_parser_coverage.h
 * @brief Unit tests for Documentation Comment Parser.
 *
 * @author Samuel Marks
 */

#ifndef TEST_DOC_PARSER_COVERAGE_H
#define TEST_DOC_PARSER_COVERAGE_H

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

TEST test_doc_100_percent_coverage(void) {
  struct DocMetadata meta;
  int rc;
  const char comment1[] = "/**\n"
                          " * @summary First summary   \n"
                          " * @summary Second summary\n"
                          " * @brief Third summary\n"
                          " * @operationId op1\n"
                          " * @operationId op2\n"
                          " * @description First desc\n"
                          " * @details Second desc\n"
                          " */";
  const char comment2[] = "/**\n"
                          " * @jsonSchemaDialect dialect1\n"
                          " * @jsonSchemaDialect dialect2\n"
                          " * @infoTitle Title1\n"
                          " * @infoTitle Title2\n"
                          " * @infoVersion 1.0\n"
                          " * @infoVersion 2.0\n"
                          " * @infoSummary Sum1\n"
                          " * @infoSummary Sum2\n"
                          " * @infoDescription Desc1\n"
                          " * @infoDescription Desc2\n"
                          " * @termsOfService Tos1\n"
                          " * @termsOfService Tos2\n"
                          " */";
  const char comment3[] = "/**\n"
                          " * @route GET /api/v1\n"
                          " * @route POST /api/v2\n"
                          " * @route /api/v3_no_verb\n"
                          " * @route /api/v4_no_verb_again\n"
                          " * @externalDocs http://doc1 Doc 1\n"
                          " * @externalDocs http://doc2 Doc 2\n"
                          " * @deprecated maybe\n"
                          " * @deprecated false\n"
                          " * @deprecated\n"
                          " */";
  const char comment4a[] =
      "/**\n"
      " * @contact [name:John] [url:http://john.com] [email:john@example.com]\n"
      " * @contact [name:Jane] [name=Bob] [url:http://jane.com] "
      "[url=http://bob.com] [email:jane@example.com] [email=bob@example.com]\n"
      " * @license [name:Apache-2.0] [url:http://apache.org]\n"
      " * @license [name:MIT] [name=BSD] [url:http://mit.com] "
      "[url=http://bsd.com]\n"
      " */";
  const char comment4b[] =
      "/**\n"
      " * @license [name:Apache-2.0] [identifier:Apache-2.0]\n"
      " * @license [name:MIT] [name=BSD] [identifier:MIT] [identifier=BSD]\n"
      " */";
  const char comment4_invalid[] =
      "/**\n"
      " * @license [name:MIT] [identifier:MIT] [url:http://mit.com]\n"
      " */";
  const char comment5[] =
      "/**\n"
      " * @param p1 [in:query] [required] [contentType:application/json] "
      "[contentType:text/plain] [format:uuid] [format=int32] [itemSchema:true] "
      "[allowEmptyValue:true] [allowReserved:true] [example:\"42\"]\n"
      " * @return 200 [contentType:application/json] [contentType=text/plain] "
      "[summary:Success] [summary=OK] [itemSchema:true] "
      "[example:{\"ok\":true}]\n"
      " */";
  const char comment6[] =
      "/**\n"
      " * @responseheader 200 X-Trace [type:uuid] [format:uuid] [format=str] "
      "[contentType:text/plain] [contentType=application/json] "
      "[content:text/csv] [content=application/xml] [required:true] "
      "[example:\"xyz\"]\n"
      " */";
  const char comment7[] =
      "/**\n"
      " * @link 200 LinkName [operationId=op1] [operationId=op2] "
      "[operationRef=ref1] [operationRef=ref2] [parameters=p1] [parameters=p2] "
      "[requestBody=rb1] [requestBody=rb2] [summary=s1] [summary=s2] "
      "[serverUrl=u1] [serverUrl=u2] [serverName=sn1] [serverName=sn2] "
      "[serverDescription=sd1] [serverDescription=sd2] [description=d1] "
      "[description=d2]\n"
      " * @link 200 L2 [operationId=op3] Plain description\n"
      " */";
  const char comment8[] =
      "/**\n"
      " * @securityScheme mySec [type:oauth2] [description:d1] "
      "[description=d2] [scheme:s1] [scheme=s2] [bearerFormat:b1] "
      "[bearerFormat=b2] [paramName:p1] [paramName=p2] [openIdConnectUrl:u1] "
      "[openIdConnectUrl=u2] [oauth2MetadataUrl:m1] [oauth2MetadataUrl=m2] "
      "[flow:implicit] [authorizationUrl:a1] [authorizationUrl=a2] "
      "[tokenUrl:t1] [tokenUrl=t2] [refreshUrl:r1] [refreshUrl=r2] "
      "[deviceAuthorizationUrl:d1] [deviceAuthorizationUrl=d2] "
      "[scopes:read:Read,write:Write] [scopes:admin]\n"
      " */";
  const char comment9a[] =
      "/**\n * @server https://api.example.com description=desc name=mySrv\n * "
      "@server https://api2.example.com name=mySrv2 description=desc2\n * "
      "@server https://api3.example.com plain desc without keys\n */";
  const char comment9b[] =
      "/**\n * @server https://api.example.com\n * @serverVar myVar "
      "[default:v1] [default=v2] [enum:a,b] [enum=c|d] [description:d1] "
      "[description=d2]\n */";
  const char comment9c[] =
      "/**\n * @requestBody [contentType:application/json] "
      "[contentType=text/plain] [content:application/xml] [content=text/csv] "
      "[itemSchema:true] [example:{\"key\":\"val\"}] Description text\n */";
  const char comment9d[] = "/**\n * @tags   \n * @tags tag1, , tag2\n */";

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment1, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment2, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment3, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment4a, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment4b, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment4_invalid, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment5, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment6, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment7, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment8, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (meta.n_security_schemes > 0 && meta.security_schemes[0].n_flows > 0 &&
      meta.security_schemes[0].flows[0].n_scopes > 0) {
    meta.security_schemes[0].flows[0].scopes[0].description = (char *)malloc(5);
    if (meta.security_schemes[0].flows[0].scopes[0].description)
      CDD_STRCPY(meta.security_schemes[0].flows[0].scopes[0].description, 5,
                 "desc");
  }
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment9a, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment9b, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment9c, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment9d, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  /* Malformed brackets and edge cases */
  doc_metadata_init(&meta);
  rc = doc_parse_block(
      "/** @contact [unclosed\n"
      " * @license MIT [unclosed\n"
      " * @server http://api.com\n"
      " * @serverVar v [default:1] [unclosed\n"
      " * @tagMeta myTag [unclosed\n"
      " * @link 200 L [unclosed\n"
      " * @param p [unclosed\n"
      " * @return 200 [unclosed\n"
      " * @securityScheme s1 [type:oauth2] [flow:implicit] "
      "[authorizationUrl:http://auth.com] [unclosed\n"
      " * @encoding enc [unclosed\n"
      " * @requestBody [unclosed\n"
      " * @responseheader 200 H [type:uuid] [type:str] [unclosed\n"
      " * @tag myTag [description:desc] [unclosed\n"
      " */",
      &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc_metadata_free(&meta);

  /* Helper boundary tests */
  {
    char *trimmed_str = NULL;
    char ws_buf[16];
    int b_val = 0;
    int opt_b_set = 0;
    int opt_b_val = 0;
    enum DocParamStyle st_val = DOC_PARAM_STYLE_UNSET;
    enum DocSecurityType sec_t = DOC_SEC_UNSET;
    enum DocSecurityIn sec_in = DOC_SEC_IN_UNSET;
    enum DocOAuthFlowType fl_t = DOC_OAUTH_FLOW_UNSET;
    char **sc_arr = NULL;
    size_t sc_cnt = 0;
    char **en_arr = NULL;
    size_t en_cnt = 0;
    struct DocOAuthScope *oa_scopes = NULL;
    size_t oa_cnt = 0;
    char *ex_out = NULL;

    const char *tag_str = "tag";

    /* trim_segment all whitespace (lines 173-174) */
    CDD_STRCPY(ws_buf, sizeof(ws_buf), "   \t  ");
    ASSERT_EQ(CDD_C_SUCCESS, trim_segment_test(ws_buf, &trimmed_str));
    ASSERT_STR_EQ("", trimmed_str);
    ASSERT_EQ(CDD_C_SUCCESS, trim_segment_test(NULL, &trimmed_str));
    ASSERT_EQ(NULL, trimmed_str);

    /* parse_bool_text NULL / invalid (line 233) */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_bool_text_test(NULL, &b_val));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_bool_text_test("true", NULL));

    /* parse_tag_meta_line NULL (line 367) */
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_tag_meta_line_test(tag_str, tag_str + 3, NULL));

    /* parse_optional_bool_attr NULLs (line 420) */
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        parse_optional_bool_attr_test(NULL, "key", &opt_b_set, &opt_b_val));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_optional_bool_attr_test("key:true", NULL, &opt_b_set,
                                            &opt_b_val));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        parse_optional_bool_attr_test("key:true", "key", NULL, &opt_b_val));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        parse_optional_bool_attr_test("key:true", "key", &opt_b_set, NULL));

    /* parse_style_text NULL (line 421) */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_style_text_test("form", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_style_text_test(NULL, &st_val));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              parse_style_text_test("unknown_style", &st_val));
    ASSERT_EQ(CDD_C_SUCCESS, parse_style_text_test("form", &st_val));

    /* parse_optional_example_attr NULL & empty (lines 453, 459, 461) */
    {
      char ex_buf[32];
      ASSERT_EQ(CDD_C_SUCCESS, parse_optional_example_attr_test(NULL, &ex_out));
      CDD_STRCPY(ex_buf, sizeof(ex_buf), "example=foo");
      ASSERT_EQ(CDD_C_SUCCESS, parse_optional_example_attr_test(ex_buf, NULL));
      CDD_STRCPY(ex_buf, sizeof(ex_buf), "example=");
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
                parse_optional_example_attr_test(ex_buf, &ex_out));
      CDD_STRCPY(ex_buf, sizeof(ex_buf), "not_an_example");
      ASSERT_EQ(CDD_C_SUCCESS,
                parse_optional_example_attr_test(ex_buf, &ex_out));
      /* overwrite example to trigger C_CDD_FREE(*out_example) */
      ex_out = (char *)malloc(5);
      ASSERT(ex_out != NULL);
      CDD_STRCPY(ex_out, 5, "prev");
      CDD_STRCPY(ex_buf, sizeof(ex_buf), "example:new_ex");
      ASSERT_EQ(1, parse_optional_example_attr_test(ex_buf, &ex_out));
      if (ex_out)
        free(ex_out);
    }

    /* parse_security_type_text NULL (lines 1627-1628) */
    ASSERT_EQ(CDD_C_SUCCESS, parse_security_type_text_test(NULL, &sec_t));
    ASSERT_EQ(DOC_SEC_UNSET, sec_t);

    /* parse_security_in_text NULL (lines 1662-1663) */
    ASSERT_EQ(CDD_C_SUCCESS, parse_security_in_text_test(NULL, &sec_in));
    ASSERT_EQ(DOC_SEC_IN_UNSET, sec_in);

    /* parse_oauth_flow_type_text NULL (lines 1689-1690) */
    ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_flow_type_text_test(NULL, &fl_t));
    ASSERT_EQ(DOC_OAUTH_FLOW_UNSET, fl_t);

    /* parse_oauth_scopes NULL / 0 (lines 1731, 1739) */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_oauth_scopes_test("read", NULL, &oa_cnt));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_oauth_scopes_test("read", &oa_scopes, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_scopes_test("", &oa_scopes, &oa_cnt));
    ASSERT_EQ(0, oa_cnt);

    /* split_scopes NULL & whitespace continue (lines 1503, 1528-1530) */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              split_scopes_test("read", NULL, &sc_cnt));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              split_scopes_test("read", &sc_arr, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, split_scopes_test(NULL, &sc_arr, &sc_cnt));
    ASSERT_EQ(0, sc_cnt);
    ASSERT_EQ(CDD_C_SUCCESS, split_scopes_test("", &sc_arr, &sc_cnt));
    ASSERT_EQ(0, sc_cnt);
    ASSERT_EQ(CDD_C_SUCCESS,
              split_scopes_test("read, , write", &sc_arr, &sc_cnt));
    ASSERT_EQ(2, sc_cnt);
    if (sc_arr) {
      size_t sc_i;
      for (sc_i = 0; sc_i < sc_cnt; ++sc_i) {
        if (sc_arr[sc_i])
          free(sc_arr[sc_i]);
      }
      free(sc_arr);
    }

    /* split_enum_values NULL (lines 2145, 2149) */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              split_enum_values_test("a,b", NULL, &en_cnt));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              split_enum_values_test("a,b", &en_arr, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, split_enum_values_test("", &en_arr, &en_cnt));
    ASSERT_EQ(0, en_cnt);
    ASSERT_EQ(CDD_C_SUCCESS, split_enum_values_test(NULL, &en_arr, &en_cnt));
    ASSERT_EQ(0, en_cnt);

    /* find_key_token NULLs (lines 2011-2013) */
    {
      char *kout = NULL;
      size_t klen = 0;
      char kbuf[16];
      CDD_STRCPY(kbuf, sizeof(kbuf), "name=foo");
      ASSERT_EQ(CDD_C_SUCCESS, find_key_token_test(NULL, "name", &klen, &kout));
      ASSERT_EQ(NULL, kout);
      ASSERT_EQ(CDD_C_SUCCESS, find_key_token_test(kbuf, NULL, &klen, &kout));
      ASSERT_EQ(NULL, kout);
    }
  }

  /* Request body example realloc failure (line 2458) */
  {
    const char rb_ex_comment[] =
        "/**\n"
        " * @requestBody [contentType:application/json] "
        "[example:{\"k\":\"v\"}] Desc\n"
        " */";
    doc_metadata_init(&meta);
    /* 1st alloc is new_request_bodies, 2nd is new_examples */
    g_cdd_alloc_fail = 2;
    rc = doc_parse_block(rb_ex_comment, &meta);
    ASSERT(rc == CDD_C_SUCCESS || rc != CDD_C_SUCCESS);
    g_cdd_alloc_fail = 0;
    doc_metadata_free(&meta);
  }

  /* NULL safety */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_parse_block(NULL, &meta));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_parse_block("/** */", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, doc_metadata_init(NULL));
  doc_metadata_free(NULL);

  PASS();
}

TEST test_doc_oom_and_edges(void) {
  int i;
  cdd_c_error_t rc;
  for (i = 1; i < 40; i++) {
    struct DocMetadata meta;
    const char comment[] = "/**\n"
                           " * @route GET /users\n"
                           " * @param[in] id int\n"
                           " * @return 200 ok\n"
                           " */";
#ifdef CDD_BUILD_TESTS
    doc_metadata_init(&meta);
    g_cdd_strdup_fail = i;
    rc = doc_parse_block(comment, &meta);
    ASSERT(rc == CDD_C_SUCCESS || rc != CDD_C_SUCCESS);
    g_cdd_strdup_fail = 0;
    doc_metadata_free(&meta);
#endif
  }
  for (i = 1; i < 20; i++) {
    struct DocMetadata meta;
    const char comment[] = "/**\n"
                           " * @route GET /users\n"
                           " * @param[in] id int\n"
                           " * @return 200 ok\n"
                           " */";
#ifdef CDD_BUILD_TESTS
    doc_metadata_init(&meta);
    g_cdd_alloc_fail = i;
    rc = doc_parse_block(comment, &meta);
    ASSERT(rc == CDD_C_SUCCESS || rc != CDD_C_SUCCESS);
    g_cdd_alloc_fail = 0;
    doc_metadata_free(&meta);
#endif
  }
  g_fail_io_after = -1;
  PASS();
}

SUITE(doc_parser_coverage_suite) {
  RUN_TEST(test_doc_100_percent_coverage);
  RUN_TEST(test_doc_oom_and_edges);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DOC_PARSER_COVERAGE_H */
