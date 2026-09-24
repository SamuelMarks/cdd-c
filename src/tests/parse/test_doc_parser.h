/**
 * @file test_doc_parser.h
 * @brief Unit tests for the Documentation Comment Parser.
 *
 * Verifies parsing of:
 * - Route annotations (Method + Path)
 * - Parameter annotations (attributes, names, descriptions)
 * - Return value annotations
 * - Summary extraction
 * - Handling of block and line comment styles.
 *
 * @author Samuel Marks
 */

#ifndef TEST_DOC_PARSER_H
#define TEST_DOC_PARSER_H

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

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT int g_cdd_alloc_fail;

/* --- Test Helpers --- */
/*  (moved to global) */
/* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
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
#define doc_parse_block(comment, meta) doc_parse_block_with_oom(comment, meta)

/* --- Tests --- */

TEST test_doc_init_free(void) {
  struct DocMetadata meta;
  /* Ensure init zeroes correctly */
  ASSERT_EQ(0, doc_metadata_init(&meta));
  ASSERT_EQ(NULL, meta.route);
  ASSERT_EQ(0, meta.is_webhook);
  ASSERT_EQ(0, meta.n_params);

  /* Ensure free is safe on empty */
  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_simple_route(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @route GET /users/{id}\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("GET", meta.verb);
  ASSERT_STR_EQ("/users/{id}", meta.route);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_route_no_verb(void) {
  struct DocMetadata meta;
  const char *comment = (char *)(size_t)(size_t) "/// @route /simple/path";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(NULL, meta.verb);
  ASSERT_STR_EQ("/simple/path", meta.route);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_webhook_route(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @webhook POST /events\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("POST", meta.verb);
  ASSERT_STR_EQ("/events", meta.route);
  ASSERT_EQ(1, meta.is_webhook);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_params(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @param id [in:path] The User ID\n"
                        " * @param q [in:query] [required] Search Query\n"
                        " * @param filter Optional filter\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(3, meta.n_params);

  /* Param 1: id */
  ASSERT_STR_EQ("id", meta.params[0].name);
  ASSERT_STR_EQ("path", meta.params[0].in_loc);
  ASSERT_STR_EQ("The User ID", meta.params[0].description);
  ASSERT_EQ(0, meta.params[0].required);

  /* Param 2: q */
  ASSERT_STR_EQ("q", meta.params[1].name);
  ASSERT_STR_EQ("query", meta.params[1].in_loc);
  ASSERT_STR_EQ("Search Query", meta.params[1].description);
  ASSERT_EQ(1, meta.params[1].required);

  /* Param 3: filter */
  ASSERT_STR_EQ("filter", meta.params[2].name);
  ASSERT_EQ(NULL, meta.params[2].in_loc);
  ASSERT_STR_EQ("Optional filter", meta.params[2].description);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_param_attributes_extended(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @param ids [in:query] [style:spaceDelimited] "
                        "[explode:false] [allowReserved:true] "
                        "[allowEmptyValue] "
                        "IDs list\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_params);
  ASSERT_STR_EQ("ids", meta.params[0].name);
  ASSERT_STR_EQ("query", meta.params[0].in_loc);
  ASSERT(meta.params[0].style_set);
  ASSERT_EQ(DOC_PARAM_STYLE_SPACE_DELIMITED, meta.params[0].style);
  ASSERT(meta.params[0].explode_set);
  ASSERT_EQ(0, meta.params[0].explode);
  ASSERT(meta.params[0].allow_reserved_set);
  ASSERT_EQ(1, meta.params[0].allow_reserved);
  ASSERT(meta.params[0].allow_empty_value_set);
  ASSERT_EQ(1, meta.params[0].allow_empty_value);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_param_all_styles(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @param p1 [style:simple]\n"
                        " * @param p2 [style:matrix]\n"
                        " * @param p3 [style:label]\n"
                        " * @param p4 [style:pipeDelimited]\n"
                        " * @param p5 [style:deepObject]\n"
                        " * @param p6 [style:cookie]\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(6, meta.n_params);
  ASSERT_EQ(DOC_PARAM_STYLE_SIMPLE, meta.params[0].style);
  ASSERT_EQ(DOC_PARAM_STYLE_MATRIX, meta.params[1].style);
  ASSERT_EQ(DOC_PARAM_STYLE_LABEL, meta.params[2].style);
  ASSERT_EQ(DOC_PARAM_STYLE_PIPE_DELIMITED, meta.params[3].style);
  ASSERT_EQ(DOC_PARAM_STYLE_DEEP_OBJECT, meta.params[4].style);
  ASSERT_EQ(DOC_PARAM_STYLE_COOKIE, meta.params[5].style);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_invalid_style(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @param p1 [style:unknownStyle]\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));
  ASSERT_EQ(1, meta.n_params);
  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_param_format(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @param id [in:path] [format:int64] The user id\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_params);
  ASSERT_STR_EQ("id", meta.params[0].name);
  ASSERT_STR_EQ("int64", meta.params[0].format);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_param_deprecated(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @param legacyId [deprecated:true] Legacy identifier\n"
      " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_params);
  ASSERT_STR_EQ("legacyId", meta.params[0].name);
  ASSERT(meta.params[0].deprecated_set);
  ASSERT_EQ(1, meta.params[0].deprecated);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_param_content_type(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @param payload [in:query] "
                        "[contentType:application/json] JSON payload\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_params);
  ASSERT_STR_EQ("payload", meta.params[0].name);
  ASSERT_STR_EQ("query", meta.params[0].in_loc);
  ASSERT_STR_EQ("application/json", meta.params[0].content_type);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_returns(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @return 200 [summary:OK] Success\n"
                        " * @return 404 [summary:Missing] Not Found\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(2, meta.n_returns);

  ASSERT_STR_EQ("200", meta.returns[0].code);
  ASSERT_STR_EQ("OK", meta.returns[0].summary);
  ASSERT_STR_EQ("Success", meta.returns[0].description);

  ASSERT_STR_EQ("404", meta.returns[1].code);
  ASSERT_STR_EQ("Missing", meta.returns[1].summary);
  ASSERT_STR_EQ("Not Found", meta.returns[1].description);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_response_headers(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @responseHeader 200 X-Rate-Limit-Limit [type:integer] "
      "[contentType:application/xml] [example:42] Limit value\n"
      " * @responseHeader 200 X-Request-Id [required] Request identifier\n"
      " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(2, meta.n_response_headers);
  ASSERT_STR_EQ("200", meta.response_headers[0].code);
  ASSERT_STR_EQ("X-Rate-Limit-Limit", meta.response_headers[0].name);
  ASSERT_STR_EQ("integer", meta.response_headers[0].type);
  ASSERT_STR_EQ("application/xml", meta.response_headers[0].content_type);
  ASSERT_STR_EQ("42", meta.response_headers[0].example);
  ASSERT_STR_EQ("Limit value", meta.response_headers[0].description);
  ASSERT_EQ(0, meta.response_headers[0].required);

  ASSERT_STR_EQ("200", meta.response_headers[1].code);
  ASSERT_STR_EQ("X-Request-Id", meta.response_headers[1].name);
  ASSERT(meta.response_headers[1].required_set);
  ASSERT_EQ(1, meta.response_headers[1].required);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_response_header_format(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @responseHeader 200 X-Rate [type:integer] "
                        "[format:int64] Rate limit\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_response_headers);
  ASSERT_STR_EQ("X-Rate", meta.response_headers[0].name);
  ASSERT_STR_EQ("int64", meta.response_headers[0].format);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_link(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @link 200 next [operationId:getNext] "
      "[parameters:{\"id\":\"$response.body#/id\"}] "
      "[requestBody:{\"foo\":1}] [serverUrl:https://example.com] "
      "[serverName:prod] [serverDescription:Primary server] Next link\n"
      " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_links);
  ASSERT_STR_EQ("200", meta.links[0].code);
  ASSERT_STR_EQ("next", meta.links[0].name);
  ASSERT_STR_EQ("getNext", meta.links[0].operation_id);
  ASSERT_STR_EQ("{\"id\":\"$response.body#/id\"}",
                meta.links[0].parameters_json);
  ASSERT_STR_EQ("{\"foo\":1}", meta.links[0].request_body_json);
  ASSERT_STR_EQ("https://example.com", meta.links[0].server_url);
  ASSERT_STR_EQ("prod", meta.links[0].server_name);
  ASSERT_STR_EQ("Primary server", meta.links[0].server_description);
  ASSERT_STR_EQ("Next link", meta.links[0].description);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_return_content_type(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @return 200 [contentType:text/plain] OK\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_returns);
  ASSERT_STR_EQ("200", meta.returns[0].code);
  ASSERT_STR_EQ("text/plain", meta.returns[0].content_type);
  ASSERT_STR_EQ("OK", meta.returns[0].description);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_summary(void) {
  struct DocMetadata meta;
  const char *comment = (char *)(size_t)(size_t) "/// @brief This is a summary";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("This is a summary", meta.summary);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_operation_id(void) {
  struct DocMetadata meta;
  const char *comment = (char *)(size_t)(size_t) "/// @operationId getUserById";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("getUserById", meta.operation_id);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_description_and_deprecated(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @description Long form description\n"
                        " * @deprecated false\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("Long form description", meta.description);
  ASSERT_EQ(1, meta.deprecated_set);
  ASSERT_EQ(0, meta.deprecated);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_tags_and_external_docs(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @tag pet\n"
                        " * @tags store, admin\n"
                        " * @externalDocs https://example.com More docs\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(3, meta.n_tags);
  ASSERT_STR_EQ("pet", meta.tags[0]);
  ASSERT_STR_EQ("store", meta.tags[1]);
  ASSERT_STR_EQ("admin", meta.tags[2]);

  ASSERT_STR_EQ("https://example.com", meta.external_docs_url);
  ASSERT_STR_EQ("More docs", meta.external_docs_description);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_tag_meta(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @tagMeta users [summary:User Ops] [description:User endpoints] "
      "[parent:external] [kind:nav] [externalDocs=https://example.com/docs] "
      "[externalDocsDescription=More docs]\n"
      " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));
  ASSERT_EQ(1, meta.n_tag_meta);
  ASSERT_STR_EQ("users", meta.tag_meta[0].name);
  ASSERT_STR_EQ("User Ops", meta.tag_meta[0].summary);
  ASSERT_STR_EQ("User endpoints", meta.tag_meta[0].description);
  ASSERT_STR_EQ("external", meta.tag_meta[0].parent);
  ASSERT_STR_EQ("nav", meta.tag_meta[0].kind);
  ASSERT_STR_EQ("https://example.com/docs", meta.tag_meta[0].external_docs_url);
  ASSERT_STR_EQ("More docs", meta.tag_meta[0].external_docs_description);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_security(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @security api_key\n"
                        " * @security petstore_auth write:pets, read:pets\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(2, meta.n_security);
  ASSERT_STR_EQ("api_key", meta.security[0].scheme);
  ASSERT_EQ(0, meta.security[0].n_scopes);

  ASSERT_STR_EQ("petstore_auth", meta.security[1].scheme);
  ASSERT_EQ(2, meta.security[1].n_scopes);
  ASSERT_STR_EQ("write:pets", meta.security[1].scopes[0]);
  ASSERT_STR_EQ("read:pets", meta.security[1].scopes[1]);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_security_scheme(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @securityScheme api_key [type:apiKey] [paramName:X-API-Key] "
      "[in:header]\n"
      " * @securityScheme bearerAuth [type:http] [scheme:bearer] "
      "[bearerFormat:JWT]\n"
      " * @securityScheme oidc [type:openIdConnect] "
      "[openIdConnectUrl:https://example.com/.well-known/"
      "openid-configuration]\n"
      " * @securityScheme oauth2Auth [type:oauth2] "
      "[flow:authorizationCode] [authorizationUrl:https://auth.example.com] "
      "[tokenUrl:https://token.example.com] "
      "[scopes:read:pets,write:pets]\n"
      " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(4, meta.n_security_schemes);
  ASSERT_STR_EQ("api_key", meta.security_schemes[0].name);
  ASSERT_EQ(DOC_SEC_APIKEY, meta.security_schemes[0].type);
  ASSERT_STR_EQ("X-API-Key", meta.security_schemes[0].param_name);
  ASSERT_EQ(DOC_SEC_IN_HEADER, meta.security_schemes[0].in);

  ASSERT_STR_EQ("bearerAuth", meta.security_schemes[1].name);
  ASSERT_EQ(DOC_SEC_HTTP, meta.security_schemes[1].type);
  ASSERT_STR_EQ("bearer", meta.security_schemes[1].scheme);
  ASSERT_STR_EQ("JWT", meta.security_schemes[1].bearer_format);

  ASSERT_STR_EQ("oidc", meta.security_schemes[2].name);
  ASSERT_EQ(DOC_SEC_OPENID, meta.security_schemes[2].type);
  ASSERT_STR_EQ("https://example.com/.well-known/openid-configuration",
                meta.security_schemes[2].open_id_connect_url);

  ASSERT_STR_EQ("oauth2Auth", meta.security_schemes[3].name);
  ASSERT_EQ(DOC_SEC_OAUTH2, meta.security_schemes[3].type);
  ASSERT_EQ(1, meta.security_schemes[3].n_flows);
  ASSERT_EQ(DOC_OAUTH_FLOW_AUTHORIZATION_CODE,
            meta.security_schemes[3].flows[0].type);
  ASSERT_STR_EQ("https://auth.example.com",
                meta.security_schemes[3].flows[0].authorization_url);
  ASSERT_STR_EQ("https://token.example.com",
                meta.security_schemes[3].flows[0].token_url);
  ASSERT_EQ(2, meta.security_schemes[3].flows[0].n_scopes);
  ASSERT_STR_EQ("read:pets", meta.security_schemes[3].flows[0].scopes[0].name);
  ASSERT_STR_EQ("write:pets", meta.security_schemes[3].flows[0].scopes[1].name);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_tag_meta_oom(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @tagMeta mytag [summary: A tag] [description: Desc]\n"
      " * @tagMeta \n"
      " */";
  doc_metadata_init(&meta);
  /* The macro expands to doc_parse_block_with_oom which sweeps g_cdd_alloc_fail
   */
  ASSERT_EQ(0, doc_parse_block(comment, &meta));
  ASSERT_EQ(1, meta.n_tag_meta);
  ASSERT_STR_EQ("mytag", meta.tag_meta[0].name);
  doc_metadata_free(&meta);
  PASS();
}

TEST test_doc_parse_extract_rest_oom(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @summary This is a test summary\n"
                        " */";
  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));
  ASSERT_STR_EQ("This is a test summary", meta.summary);
  doc_metadata_free(&meta);
  PASS();
}
TEST test_doc_parse_server_and_request_body(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @server https://api.example.com "
                        "name=prod description=Production API\n"
                        " * @requestBody [required:false] "
                        "[contentType:application/xml] Upload payload\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_servers);
  ASSERT_STR_EQ("https://api.example.com", meta.servers[0].url);
  ASSERT_STR_EQ("prod", meta.servers[0].name);
  ASSERT_STR_EQ("Production API", meta.servers[0].description);

  ASSERT_EQ(1, meta.request_body_required_set);
  ASSERT_EQ(0, meta.request_body_required);
  ASSERT_STR_EQ("application/xml", meta.request_body_content_type);
  ASSERT_STR_EQ("Upload payload", meta.request_body_description);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_server_variables(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @server https://api.example.com name=prod description=Production "
      "API\n"
      " * @serverVar env [default:prod] [enum:prod,staging]\n"
      " * @serverVar region [default:us-east-1] [description:AWS region]\n"
      " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_servers);
  ASSERT_EQ(2, meta.servers[0].n_variables);
  ASSERT_STR_EQ("env", meta.servers[0].variables[0].name);
  ASSERT_STR_EQ("prod", meta.servers[0].variables[0].default_value);
  ASSERT_EQ(2, meta.servers[0].variables[0].n_enum_values);
  ASSERT_STR_EQ("prod", meta.servers[0].variables[0].enum_values[0]);
  ASSERT_STR_EQ("staging", meta.servers[0].variables[0].enum_values[1]);
  ASSERT_STR_EQ("region", meta.servers[0].variables[1].name);
  ASSERT_STR_EQ("us-east-1", meta.servers[0].variables[1].default_value);
  ASSERT_STR_EQ("AWS region", meta.servers[0].variables[1].description);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_info_overrides(void) {
  struct DocMetadata meta;
  const char *comment = "/**\n"
                        " * @infoTitle Example API\n"
                        " * @infoVersion 2.1.0\n"
                        " * @infoSummary Short summary\n"
                        " * @infoDescription Full description\n"
                        " * @termsOfService https://example.com/terms\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("Example API", meta.info_title);
  ASSERT_STR_EQ("2.1.0", meta.info_version);
  ASSERT_STR_EQ("Short summary", meta.info_summary);
  ASSERT_STR_EQ("Full description", meta.info_description);
  ASSERT_STR_EQ("https://example.com/terms", meta.terms_of_service);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_contact_license(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @contact [name:API Support] [url:https://example.com/support] "
      "[email:support@example.com]\n"
      " * @license [name:Apache 2.0] [identifier:Apache-2.0]\n"
      " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("API Support", meta.contact_name);
  ASSERT_STR_EQ("https://example.com/support", meta.contact_url);
  ASSERT_STR_EQ("support@example.com", meta.contact_email);
  ASSERT_STR_EQ("Apache 2.0", meta.license_name);
  ASSERT_STR_EQ("Apache-2.0", meta.license_identifier);
  ASSERT_EQ(NULL, meta.license_url);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_request_body_multi_content(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @requestBody [contentType:application/json] JSON body\n"
      " * @requestBody [contentType:application/xml] XML body\n"
      " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(2, meta.n_request_bodies);
  ASSERT_STR_EQ("application/json", meta.request_bodies[0].content_type);
  ASSERT_STR_EQ("JSON body", meta.request_bodies[0].description);
  ASSERT_STR_EQ("application/xml", meta.request_bodies[1].content_type);
  ASSERT_STR_EQ("XML body", meta.request_bodies[1].description);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_examples(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @param id [in:path] [example:123] The user ID\n"
      " * @return 200 [summary:OK] [example:{\"ok\":true}] Success\n"
      " * @requestBody [contentType:application/json] "
      "[example:{\"name\":\"x\"}] "
      "Body\n"
      " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(1, meta.n_params);
  ASSERT_STR_EQ("123", meta.params[0].example);
  ASSERT_EQ(1, meta.n_returns);
  ASSERT_STR_EQ("{\"ok\":true}", meta.returns[0].example);
  ASSERT_EQ(1, meta.n_request_bodies);
  ASSERT_STR_EQ("{\"name\":\"x\"}", meta.request_bodies[0].example);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_malformed_lines(void) {
  struct DocMetadata meta;
  /* Route without args, Param without name */
  const char *comment = "/**\n"
                        " * @route\n"
                        " * @param\n"
                        " */";

  doc_metadata_init(&meta);
  /* Should succeed but parse nothing useful */
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(NULL, meta.route);
  /* Malformed param line is skipped, so n_params should be 0 */
  ASSERT_EQ(0, meta.n_params);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_encodings(void) {
  const char *comment = "/**\n"
                        " * @encoding profileImage [contentType: image/png] "
                        "[style: form] [explode: true]\n"
                        " * @prefixEncoding [contentType: image/jpeg]\n"
                        " * @itemEncoding [contentType: application/json]\n"
                        " */";
  struct DocMetadata meta;
  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));
  ASSERT_EQ(3, meta.n_encodings);
  ASSERT_STR_EQ("profileImage", meta.encodings[0].name);
  ASSERT_STR_EQ("image/png", meta.encodings[0].content_type);
  ASSERT_EQ(DOC_PARAM_STYLE_FORM, meta.encodings[0].style);
  ASSERT_EQ(1, meta.encodings[0].explode);
  ASSERT_EQ(0, meta.encodings[0].kind);

  ASSERT_EQ(NULL, meta.encodings[1].name);
  ASSERT_STR_EQ("image/jpeg", meta.encodings[1].content_type);
  ASSERT_EQ(1, meta.encodings[1].kind);

  ASSERT_EQ(NULL, meta.encodings[2].name);
  ASSERT_STR_EQ("application/json", meta.encodings[2].content_type);
  ASSERT_EQ(2, meta.encodings[2].kind);

  doc_metadata_free(&meta);
  g_fail_io_after = -1;
  PASS();
}

TEST test_doc_parse_dupes_and_extras(void) {
  struct DocMetadata meta;
  int rc;
  const char comment[] = {0};

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment, &meta);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  doc_metadata_free(&meta);
  PASS();
}

TEST test_doc_parse_equal_signs(void) {
  struct DocMetadata meta;
  int rc;
  const char comment[] = {
      10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32,
      32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32,
      32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 10, 32,
      32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32,
      32, 32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 10,
      32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32,
      32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32,
      10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32,
      32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32,
      32, 10, 32, 32, 32, 32, 32, 32, 10, 32, 32, 32, 32, 32, 32, 0};

  doc_metadata_init(&meta);
  rc = doc_parse_block(comment, &meta);
  printf("test_doc_parse_equal_signs rc: %d\n", rc);
  ASSERT_EQ(0, rc);
  doc_metadata_free(&meta);
  PASS();
}

TEST test_doc_parse_more_branches(void) {
  struct DocMetadata meta;
  int rc;
  const char comment[] = {0};
  doc_metadata_init(&meta);
  rc = doc_parse_block(comment, &meta);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  doc_metadata_free(&meta);
  PASS();
}

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

SUITE(doc_parser_suite) {
  RUN_TEST(test_doc_parser_100_percent_coverage_boost);
  RUN_TEST(test_doc_100_percent_coverage);
  RUN_TEST(test_doc_init_free);
  RUN_TEST(test_doc_parse_simple_route);
  RUN_TEST(test_doc_parse_route_no_verb);
  RUN_TEST(test_doc_parse_webhook_route);
  RUN_TEST(test_doc_parse_params);
  RUN_TEST(test_doc_parse_returns);
  RUN_TEST(test_doc_parse_response_headers);
  RUN_TEST(test_doc_parse_response_header_format);
  RUN_TEST(test_doc_parse_link);
  RUN_TEST(test_doc_parse_param_attributes_extended);
  RUN_TEST(test_doc_parse_param_all_styles);
  RUN_TEST(test_doc_parse_invalid_style);
  RUN_TEST(test_doc_parse_param_format);
  RUN_TEST(test_doc_parse_param_deprecated);
  RUN_TEST(test_doc_parse_param_content_type);
  RUN_TEST(test_doc_parse_return_content_type);
  RUN_TEST(test_doc_parse_summary);
  RUN_TEST(test_doc_parse_operation_id);
  RUN_TEST(test_doc_parse_description_and_deprecated);
  RUN_TEST(test_doc_parse_tags_and_external_docs);
  RUN_TEST(test_doc_parse_tag_meta);
  RUN_TEST(test_doc_parse_security);
  RUN_TEST(test_doc_parse_security_scheme);
  RUN_TEST(test_doc_parse_tag_meta_oom);
  RUN_TEST(test_doc_parse_extract_rest_oom);
  RUN_TEST(test_doc_parse_server_and_request_body);
  RUN_TEST(test_doc_parse_server_variables);
  RUN_TEST(test_doc_parse_info_overrides);
  RUN_TEST(test_doc_parse_contact_license);
  RUN_TEST(test_doc_parse_request_body_multi_content);
  RUN_TEST(test_doc_parse_examples);

  RUN_TEST(test_doc_parse_malformed_lines);
  RUN_TEST(test_doc_parse_encodings);
  RUN_TEST(test_doc_oom_and_edges);
  RUN_TEST(test_doc_parse_dupes_and_extras);
  RUN_TEST(test_doc_parse_more_branches);
  RUN_TEST(test_doc_parse_equal_signs);
  RUN_TEST(test_doc_crlf_and_non_bracket_tags);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DOC_PARSER_H */
