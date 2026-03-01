/**
 * @file test_doc_parser.h
 * @brief Unit tests for the Documentation Comment Parser.
 *
 * Verifies parsing of:
 * - Route annotations (Method + Path)
 * - Parameter annotations (attributes, names, descriptions)
 * - Return value annotations
 * - Summary extraction
 * - Handling of block (`/**`) and line (`///`) comment styles.
 *
 * @author Samuel Marks
 */

#ifndef TEST_DOC_PARSER_H
#define TEST_DOC_PARSER_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "docstrings/parse/doc.h"

/* --- Test Helpers --- */

static void reset_meta(struct DocMetadata *meta) {
  doc_metadata_free(meta);
  doc_metadata_init(meta);
}

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
  PASS();
}

TEST test_doc_parse_route_no_verb(void) {
  struct DocMetadata meta;
  const char *comment = "/// @route /simple/path";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(NULL, meta.verb);
  ASSERT_STR_EQ("/simple/path", meta.route);

  doc_metadata_free(&meta);
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
  PASS();
}

TEST test_doc_parse_summary(void) {
  struct DocMetadata meta;
  const char *comment = "/// @brief This is a summary";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("This is a summary", meta.summary);

  doc_metadata_free(&meta);
  PASS();
}

TEST test_doc_parse_operation_id(void) {
  struct DocMetadata meta;
  const char *comment = "/// @operationId getUserById";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("getUserById", meta.operation_id);

  doc_metadata_free(&meta);
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
  PASS();
}

TEST test_doc_parse_tag_meta(void) {
  struct DocMetadata meta;
  const char *comment =
      "/**\n"
      " * @tagMeta users [summary:User Ops] [description:User endpoints] "
      "[parent:external] [kind:nav] [externalDocs:https://example.com/docs] "
      "[externalDocsDescription:More docs]\n"
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
  PASS();
}

TEST test_doc_parse_invalid_inputs(void) {
  struct DocMetadata meta;
  doc_metadata_init(&meta);

  ASSERT_EQ(EINVAL, doc_parse_block(NULL, &meta));
  ASSERT_EQ(EINVAL, doc_parse_block("/* */", NULL));

  /* Should handle empty string without error */
  ASSERT_EQ(0, doc_parse_block("", &meta));

  doc_metadata_free(&meta);
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
  PASS();
}

SUITE(doc_parser_suite) {
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
  RUN_TEST(test_doc_parse_server_and_request_body);
  RUN_TEST(test_doc_parse_server_variables);
  RUN_TEST(test_doc_parse_info_overrides);
  RUN_TEST(test_doc_parse_contact_license);
  RUN_TEST(test_doc_parse_request_body_multi_content);
  RUN_TEST(test_doc_parse_examples);
  RUN_TEST(test_doc_parse_invalid_inputs);
  RUN_TEST(test_doc_parse_malformed_lines);
  RUN_TEST(test_doc_parse_encodings);
}

#endif /* TEST_DOC_PARSER_H */
