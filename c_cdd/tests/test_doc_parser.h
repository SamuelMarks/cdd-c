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

#include "doc_parser.h"

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
                        " * @return 200 Success\n"
                        " * @return 404 Not Found\n"
                        " */";

  doc_metadata_init(&meta);
  ASSERT_EQ(0, doc_parse_block(comment, &meta));

  ASSERT_EQ(2, meta.n_returns);

  ASSERT_STR_EQ("200", meta.returns[0].code);
  ASSERT_STR_EQ("Success", meta.returns[0].description);

  ASSERT_STR_EQ("404", meta.returns[1].code);
  ASSERT_STR_EQ("Not Found", meta.returns[1].description);

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

SUITE(doc_parser_suite) {
  RUN_TEST(test_doc_init_free);
  RUN_TEST(test_doc_parse_simple_route);
  RUN_TEST(test_doc_parse_route_no_verb);
  RUN_TEST(test_doc_parse_params);
  RUN_TEST(test_doc_parse_returns);
  RUN_TEST(test_doc_parse_param_attributes_extended);
  RUN_TEST(test_doc_parse_param_content_type);
  RUN_TEST(test_doc_parse_return_content_type);
  RUN_TEST(test_doc_parse_summary);
  RUN_TEST(test_doc_parse_operation_id);
  RUN_TEST(test_doc_parse_description_and_deprecated);
  RUN_TEST(test_doc_parse_tags_and_external_docs);
  RUN_TEST(test_doc_parse_security);
  RUN_TEST(test_doc_parse_server_and_request_body);
  RUN_TEST(test_doc_parse_invalid_inputs);
  RUN_TEST(test_doc_parse_malformed_lines);
}

#endif /* TEST_DOC_PARSER_H */
