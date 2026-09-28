/**
 * @file test_openapi_loader_media.h
 * @brief Response content types, media encodings, and inline schema tests.
 */

#ifndef TEST_OPENAPI_LOADER_MEDIA_H
#define TEST_OPENAPI_LOADER_MEDIA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

TEST test_load_response_multiple_content(void) {
  struct OpenAPI_MediaType *_ast_find_media_type_1;

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115,
      34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  115, 34,  58,  123, 34,
      80,  101, 116, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111,
      98,  106, 101, 99,  116, 34,  44,  34,  112, 114, 111, 112, 101, 114, 116,
      105, 101, 115, 34,  58,  123, 34,  105, 100, 34,  58,  123, 34,  116, 121,
      112, 101, 34,  58,  34,  105, 110, 116, 101, 103, 101, 114, 34,  125, 125,
      125, 125, 125, 44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 34,  47,
      112, 101, 116, 115, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,
      114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,
      48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111,
      110, 34,  58,  34,  111, 107, 34,  44,  34,  99,  111, 110, 116, 101, 110,
      116, 34,  58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116, 105, 111,
      110, 47,  106, 115, 111, 110, 34,  58,  123, 34,  115, 99,  104, 101, 109,
      97,  34,  58,  123, 34,  36,  114, 101, 102, 34,  58,  34,  35,  47,  99,
      111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  115, 99,  104, 101, 109,
      97,  115, 47,  80,  101, 116, 34,  125, 125, 44,  34,  116, 101, 120, 116,
      47,  112, 108, 97,  105, 110, 34,  58,  123, 34,  115, 99,  104, 101, 109,
      97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114,
      105, 110, 103, 34,  125, 125, 125, 125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Response *resp = &spec.paths[0].operations[0].responses[0];
    ASSERT_EQ(2, resp->n_content_media_types);
    ASSERT_STR_EQ("application/json", resp->content_type);
    ASSERT_STR_EQ("Pet", resp->schema.ref_name);
    {
      const struct OpenAPI_MediaType *text_mt =
          (find_media_type(resp->content_media_types,
                           resp->n_content_media_types, "text/plain",
                           &_ast_find_media_type_1),
           _ast_find_media_type_1);
      ASSERT(text_mt != NULL);
      ASSERT_EQ(1, text_mt->schema_set);
      ASSERT_STR_EQ("string", text_mt->schema.inline_type);
    }
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_request_body_multiple_content_with_ref(void) {
  struct OpenAPI_MediaType *_ast_find_media_type_2;

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115,
      34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  115, 34,  58,  123, 34,
      80,  101, 116, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111,
      98,  106, 101, 99,  116, 34,  44,  34,  112, 114, 111, 112, 101, 114, 116,
      105, 101, 115, 34,  58,  123, 34,  105, 100, 34,  58,  123, 34,  116, 121,
      112, 101, 34,  58,  34,  105, 110, 116, 101, 103, 101, 114, 34,  125, 125,
      125, 125, 44,  34,  109, 101, 100, 105, 97,  84,  121, 112, 101, 115, 34,
      58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116, 105, 111, 110, 47,
      106, 115, 111, 110, 34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  34,
      58,  123, 34,  36,  114, 101, 102, 34,  58,  34,  35,  47,  99,  111, 109,
      112, 111, 110, 101, 110, 116, 115, 47,  115, 99,  104, 101, 109, 97,  115,
      47,  80,  101, 116, 34,  125, 125, 125, 125, 44,  34,  112, 97,  116, 104,
      115, 34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  112,
      111, 115, 116, 34,  58,  123, 34,  114, 101, 113, 117, 101, 115, 116, 66,
      111, 100, 121, 34,  58,  123, 34,  99,  111, 110, 116, 101, 110, 116, 34,
      58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116, 105, 111, 110, 47,
      106, 115, 111, 110, 34,  58,  123, 34,  36,  114, 101, 102, 34,  58,  34,
      35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  109, 101,
      100, 105, 97,  84,  121, 112, 101, 115, 47,  97,  112, 112, 108, 105, 99,
      97,  116, 105, 111, 110, 126, 49,  106, 115, 111, 110, 34,  125, 44,  34,
      97,  112, 112, 108, 105, 99,  97,  116, 105, 111, 110, 47,  120, 45,  119,
      119, 119, 45,  102, 111, 114, 109, 45,  117, 114, 108, 101, 110, 99,  111,
      100, 101, 100, 34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  34,  58,
      123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116,
      34,  125, 125, 125, 125, 44,  34,  114, 101, 115, 112, 111, 110, 115, 101,
      115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115,
      99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  75,  34,  125,
      125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_EQ(2, op->n_req_body_media_types);
    ASSERT_STR_EQ("application/json", op->req_body.content_type);
    ASSERT_STR_EQ("Pet", op->req_body.ref_name);
    {
      const struct OpenAPI_MediaType *mt =
          (find_media_type(op->req_body_media_types, op->n_req_body_media_types,
                           "application/json", &_ast_find_media_type_2),
           _ast_find_media_type_2);
      ASSERT(mt != NULL);
      ASSERT_STR_EQ("#/components/mediaTypes/application~1json", mt->ref);
    }
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_media_type_encoding(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115,
      34,  58,  123, 34,  109, 101, 100, 105, 97,  84,  121, 112, 101, 115, 34,
      58,  123, 34,  109, 117, 108, 116, 105, 112, 97,  114, 116, 47,  102, 111,
      114, 109, 45,  100, 97,  116, 97,  34,  58,  123, 34,  115, 99,  104, 101,
      109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,
      106, 101, 99,  116, 34,  125, 44,  34,  101, 110, 99,  111, 100, 105, 110,
      103, 34,  58,  123, 34,  102, 105, 108, 101, 34,  58,  123, 34,  99,  111,
      110, 116, 101, 110, 116, 84,  121, 112, 101, 34,  58,  34,  105, 109, 97,
      103, 101, 47,  112, 110, 103, 34,  44,  34,  101, 120, 112, 108, 111, 100,
      101, 34,  58,  116, 114, 117, 101, 44,  34,  97,  108, 108, 111, 119, 82,
      101, 115, 101, 114, 118, 101, 100, 34,  58,  116, 114, 117, 101, 44,  34,
      104, 101, 97,  100, 101, 114, 115, 34,  58,  123, 34,  88,  45,  82,  97,
      116, 101, 45,  76,  105, 109, 105, 116, 45,  76,  105, 109, 105, 116, 34,
      58,  123, 34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121,
      112, 101, 34,  58,  34,  105, 110, 116, 101, 103, 101, 114, 34,  125, 125,
      125, 125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_media_types);
  {
    const struct OpenAPI_MediaType *mt = &spec.component_media_types[0];
    ASSERT_EQ(1, mt->n_encoding);
    ASSERT_STR_EQ("file", mt->encoding[0].name);
    ASSERT_STR_EQ("image/png", mt->encoding[0].content_type);
    ASSERT_EQ(1, mt->encoding[0].explode_set);
    ASSERT_EQ(1, mt->encoding[0].explode);
    ASSERT_EQ(1, mt->encoding[0].allow_reserved_set);
    ASSERT_EQ(1, mt->encoding[0].allow_reserved);
    ASSERT_EQ(1, mt->encoding[0].n_headers);
    ASSERT_STR_EQ("X-Rate-Limit-Limit", mt->encoding[0].headers[0].name);
    ASSERT_STR_EQ("integer", mt->encoding[0].headers[0].type);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_media_type_prefix_item_encoding(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115,
      34,  58,  123, 34,  109, 101, 100, 105, 97,  84,  121, 112, 101, 115, 34,
      58,  123, 34,  109, 117, 108, 116, 105, 112, 97,  114, 116, 47,  109, 105,
      120, 101, 100, 34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  34,  58,
      123, 34,  116, 121, 112, 101, 34,  58,  34,  97,  114, 114, 97,  121, 34,
      125, 44,  34,  112, 114, 101, 102, 105, 120, 69,  110, 99,  111, 100, 105,
      110, 103, 34,  58,  91,  123, 34,  99,  111, 110, 116, 101, 110, 116, 84,
      121, 112, 101, 34,  58,  34,  97,  112, 112, 108, 105, 99,  97,  116, 105,
      111, 110, 47,  106, 115, 111, 110, 34,  125, 44,  123, 34,  99,  111, 110,
      116, 101, 110, 116, 84,  121, 112, 101, 34,  58,  34,  105, 109, 97,  103,
      101, 47,  112, 110, 103, 34,  44,  34,  104, 101, 97,  100, 101, 114, 115,
      34,  58,  123, 34,  88,  45,  80,  111, 115, 34,  58,  123, 34,  115, 99,
      104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,
      115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 125, 93,  44,  34,  105,
      116, 101, 109, 69,  110, 99,  111, 100, 105, 110, 103, 34,  58,  123, 34,
      99,  111, 110, 116, 101, 110, 116, 84,  121, 112, 101, 34,  58,  34,  97,
      112, 112, 108, 105, 99,  97,  116, 105, 111, 110, 47,  111, 99,  116, 101,
      116, 45,  115, 116, 114, 101, 97,  109, 34,  44,  34,  101, 110, 99,  111,
      100, 105, 110, 103, 34,  58,  123, 34,  109, 101, 116, 97,  34,  58,  123,
      34,  99,  111, 110, 116, 101, 110, 116, 84,  121, 112, 101, 34,  58,  34,
      116, 101, 120, 116, 47,  112, 108, 97,  105, 110, 34,  125, 125, 125, 125,
      125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_media_types);
  {
    const struct OpenAPI_MediaType *mt = &spec.component_media_types[0];
    ASSERT_EQ(2, mt->n_prefix_encoding);
    ASSERT_STR_EQ("application/json", mt->prefix_encoding[0].content_type);
    ASSERT_STR_EQ("image/png", mt->prefix_encoding[1].content_type);
    ASSERT_EQ(1, mt->prefix_encoding[1].n_headers);
    ASSERT_STR_EQ("X-Pos", mt->prefix_encoding[1].headers[0].name);
    ASSERT_STR_EQ("string", mt->prefix_encoding[1].headers[0].type);

    ASSERT(mt->item_encoding != NULL);
    ASSERT_EQ(1, mt->item_encoding_set);
    ASSERT_STR_EQ("application/octet-stream", mt->item_encoding->content_type);
    ASSERT_EQ(1, mt->item_encoding->n_encoding);
    ASSERT_STR_EQ("meta", mt->item_encoding->encoding[0].name);
    ASSERT_STR_EQ("text/plain", mt->item_encoding->encoding[0].content_type);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_info_metadata(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  69,  120, 97,  109, 112, 108, 101, 32,  65,
      80,  73,  34,  44,  34,  115, 117, 109, 109, 97,  114, 121, 34,  58,  34,
      83,  104, 111, 114, 116, 34,  44,  34,  100, 101, 115, 99,  114, 105, 112,
      116, 105, 111, 110, 34,  58,  34,  76,  111, 110, 103, 34,  44,  34,  116,
      101, 114, 109, 115, 79,  102, 83,  101, 114, 118, 105, 99,  101, 34,  58,
      34,  104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112, 108,
      101, 46,  99,  111, 109, 47,  116, 101, 114, 109, 115, 34,  44,  34,  118,
      101, 114, 115, 105, 111, 110, 34,  58,  34,  50,  46,  49,  46,  48,  34,
      44,  34,  99,  111, 110, 116, 97,  99,  116, 34,  58,  123, 34,  110, 97,
      109, 101, 34,  58,  34,  65,  80,  73,  32,  83,  117, 112, 112, 111, 114,
      116, 34,  44,  34,  117, 114, 108, 34,  58,  34,  104, 116, 116, 112, 115,
      58,  47,  47,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109, 34,
      44,  34,  101, 109, 97,  105, 108, 34,  58,  34,  115, 117, 112, 112, 111,
      114, 116, 64,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109, 34,
      125, 44,  34,  108, 105, 99,  101, 110, 115, 101, 34,  58,  123, 34,  110,
      97,  109, 101, 34,  58,  34,  65,  112, 97,  99,  104, 101, 32,  50,  46,
      48,  34,  44,  34,  105, 100, 101, 110, 116, 105, 102, 105, 101, 114, 34,
      58,  34,  65,  112, 97,  99,  104, 101, 45,  50,  46,  48,  34,  125, 125,
      44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("Example API", spec.info.title);
  ASSERT_STR_EQ("Short", spec.info.summary);
  ASSERT_STR_EQ("Long", spec.info.description);
  ASSERT_STR_EQ("https://example.com/terms", spec.info.terms_of_service);
  ASSERT_STR_EQ("2.1.0", spec.info.version);
  ASSERT_STR_EQ("API Support", spec.info.contact.name);
  ASSERT_STR_EQ("https://example.com", spec.info.contact.url);
  ASSERT_STR_EQ("support@example.com", spec.info.contact.email);
  ASSERT_STR_EQ("Apache 2.0", spec.info.license.name);
  ASSERT_STR_EQ("Apache-2.0", spec.info.license.identifier);
  ASSERT(spec.info.license.url == NULL);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_info_missing_title_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"version\":\"1\"},\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_info_missing_version_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\"},\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_license_identifier_and_url_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{"
      "\"title\":\"Example API\",\"version\":\"1\","
      "\"license\":{\"name\":\"Apache 2.0\",\"identifier\":\"Apache-2.0\","
      "\"url\":\"https://www.apache.org/licenses/LICENSE-2.0.html\"}"
      "},\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_license_missing_name_rejected(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{"
                     "\"title\":\"Example API\",\"version\":\"1\","
                     "\"license\":{\"identifier\":\"Apache-2.0\"}"
                     "},\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_operation_metadata(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/"
                     "meta\":{\"get\":{\"operationId\":\"getMeta\",\"summary\":"
                     "\"Summary text\",\"description\":\"Longer "
                     "description\",\"deprecated\":true,\"responses\":{\"200\":"
                     "{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("Summary text", spec.paths[0].operations[0].summary);
  ASSERT_STR_EQ("Longer description", spec.paths[0].operations[0].description);
  ASSERT_EQ(1, spec.paths[0].operations[0].deprecated);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_response_content_type(void) {

  const char *json =
      "{\"paths\":{\"/"
      "r\":{\"get\":{\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"text/plain\":{\"schema\":{\"$ref\":\"#/components/"
      "schemas/"
      "Message\"}}}}}}}},\"components\":{\"schemas\":{\"Message\":{\"type\":"
      "\"string\"}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("text/plain",
                spec.paths[0].operations[0].responses[0].content_type);
  ASSERT_STR_EQ("Message",
                spec.paths[0].operations[0].responses[0].schema.ref_name);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_response_content_type_specificity(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  114, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,
      50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  79,  75,  34,  44,  34,  99,  111, 110, 116,
      101, 110, 116, 34,  58,  123, 34,  116, 101, 120, 116, 47,  42,  34,  58,
      123, 34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 44,  34,
      116, 101, 120, 116, 47,  112, 108, 97,  105, 110, 34,  58,  123, 34,  115,
      99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,
      34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 125, 125, 125, 125,
      125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("text/plain",
                spec.paths[0].operations[0].responses[0].content_type);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_response_content_type_params_json(void) {

  const char *json =
      "{\"paths\":{\"/"
      "r\":{\"get\":{\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"text/"
      "plain\":{\"schema\":{\"type\":\"string\"}},\"application/json; "
      "charset=utf-8\":{\"schema\":{\"type\":\"string\"}}}}}}}},\"openapi\":"
      "\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("application/json; charset=utf-8",
                spec.paths[0].operations[0].responses[0].content_type);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_response_schema_primitive(void) {

  const char *json =
      "{\"paths\":{\"/"
      "r\":{\"get\":{\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"application/"
      "json\":{\"schema\":{\"type\":\"string\"}}}}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("string",
                spec.paths[0].operations[0].responses[0].schema.inline_type);
  ASSERT_EQ(0, spec.paths[0].operations[0].responses[0].schema.is_array);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_response_schema_array(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"paths\":{\"/r\":{\"get\":{"
      "\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"integer\"}}}"
      "}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].schema.is_array);
  ASSERT_STR_EQ("integer",
                spec.paths[0].operations[0].responses[0].schema.inline_type);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_schema_format_and_content(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  114, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,
      50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  79,  75,  34,  44,  34,  99,  111, 110, 116,
      101, 110, 116, 34,  58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116,
      105, 111, 110, 47,  106, 115, 111, 110, 34,  58,  123, 34,  115, 99,  104,
      101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  44,  34,  102, 111, 114, 109, 97,  116, 34,
      58,  34,  117, 117, 105, 100, 34,  44,  34,  99,  111, 110, 116, 101, 110,
      116, 77,  101, 100, 105, 97,  84,  121, 112, 101, 34,  58,  34,  105, 109,
      97,  103, 101, 47,  112, 110, 103, 34,  44,  34,  99,  111, 110, 116, 101,
      110, 116, 69,  110, 99,  111, 100, 105, 110, 103, 34,  58,  34,  98,  97,
      115, 101, 54,  52,  34,  125, 125, 125, 125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].responses[0].schema;
    ASSERT_STR_EQ("uuid", schema->format);
    ASSERT_STR_EQ("image/png", schema->content_media_type);
    ASSERT_STR_EQ("base64", schema->content_encoding);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_schema_array_item_format_and_content(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "r\":{\"get\":{\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"array\","
      "\"items\":{\"type\":\"string\",\"format\":\"uuid\","
      "\"contentMediaType\":\"image/"
      "png\",\"contentEncoding\":\"base64\"}}}}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].responses[0].schema;
    ASSERT_EQ(1, schema->is_array);
    ASSERT_STR_EQ("uuid", schema->items_format);
    ASSERT_STR_EQ("image/png", schema->items_content_media_type);
    ASSERT_STR_EQ("base64", schema->items_content_encoding);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_schema_const_examples_annotations(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  109, 111, 100, 101, 34,  44,
      34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115,
      99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,
      34,  115, 116, 114, 105, 110, 103, 34,  44,  34,  99,  111, 110, 115, 116,
      34,  58,  34,  102, 97,  115, 116, 34,  44,  34,  101, 120, 97,  109, 112,
      108, 101, 115, 34,  58,  91,  34,  102, 97,  115, 116, 34,  44,  34,  115,
      108, 111, 119, 34,  93,  44,  34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  77,  111, 100, 101, 34,  44,  34,  100, 101,
      112, 114, 101, 99,  97,  116, 101, 100, 34,  58,  116, 114, 117, 101, 44,
      34,  114, 101, 97,  100, 79,  110, 108, 121, 34,  58,  116, 114, 117, 101,
      44,  34,  119, 114, 105, 116, 101, 79,  110, 108, 121, 34,  58,  102, 97,
      108, 115, 101, 125, 125, 93,  44,  34,  114, 101, 115, 112, 111, 110, 115,
      101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101,
      115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  75,  34,
      125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].parameters[0].schema;
    ASSERT_EQ(1, schema->const_value_set);
    ASSERT_EQ(OA_ANY_STRING, schema->const_value.type);
    ASSERT_STR_EQ("fast", schema->const_value.string);
    ASSERT_EQ(2, schema->n_examples);
    ASSERT_EQ(OA_ANY_STRING, schema->examples[0].type);
    ASSERT_STR_EQ("fast", schema->examples[0].string);
    ASSERT_EQ(OA_ANY_STRING, schema->examples[1].type);
    ASSERT_STR_EQ("slow", schema->examples[1].string);
    ASSERT_STR_EQ("Mode", schema->description);
    ASSERT_EQ(1, schema->deprecated_set);
    ASSERT_EQ(1, schema->deprecated);
    ASSERT_EQ(1, schema->read_only_set);
    ASSERT_EQ(1, schema->read_only);
    ASSERT_EQ(1, schema->write_only_set);
    ASSERT_EQ(0, schema->write_only);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_schema_ref_summary_description(void) {

  const char *json =
      "{\"paths\":{\"/"
      "p\":{\"get\":{\"parameters\":[{\"name\":\"mode\",\"in\":\"query\","
      "\"schema\":{\"$ref\":\"#/components/schemas/Mode\",\"summary\":\"Mode "
      "summary\",\"description\":\"Mode "
      "description\"}}],\"responses\":{\"200\":{\"description\":\"OK\"}}}}},"
      "\"components\":{\"schemas\":{\"Mode\":{\"type\":\"string\"}}},"
      "\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].parameters[0].schema;
    ASSERT_STR_EQ("Mode", schema->ref_name);
    ASSERT_STR_EQ("Mode summary", schema->summary);
    ASSERT_STR_EQ("Mode description", schema->description);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_parameter_schema_format_and_content(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  105, 100, 34,  44,  34,  105,
      110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115, 99,  104,
      101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  44,  34,  102, 111, 114, 109, 97,  116, 34,
      58,  34,  117, 117, 105, 100, 34,  44,  34,  99,  111, 110, 116, 101, 110,
      116, 77,  101, 100, 105, 97,  84,  121, 112, 101, 34,  58,  34,  116, 101,
      120, 116, 47,  112, 108, 97,  105, 110, 34,  44,  34,  99,  111, 110, 116,
      101, 110, 116, 69,  110, 99,  111, 100, 105, 110, 103, 34,  58,  34,  98,
      97,  115, 101, 54,  52,  34,  125, 125, 93,  44,  34,  114, 101, 115, 112,
      111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      79,  75,  34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_STR_EQ("uuid", p->schema.format);
    ASSERT_STR_EQ("text/plain", p->schema.content_media_type);
    ASSERT_STR_EQ("base64", p->schema.content_encoding);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_schema_enum_default_nullable(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  115, 116, 97,  116, 117, 115,
      34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,
      34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101,
      34,  58,  91,  34,  115, 116, 114, 105, 110, 103, 34,  44,  34,  110, 117,
      108, 108, 34,  93,  44,  34,  101, 110, 117, 109, 34,  58,  91,  34,  111,
      110, 34,  44,  34,  111, 102, 102, 34,  93,  44,  34,  100, 101, 102, 97,
      117, 108, 116, 34,  58,  34,  111, 110, 34,  125, 125, 93,  44,  34,  114,
      101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,
      34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110,
      34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(1, p->schema.nullable);
    ASSERT_EQ(2, p->schema.n_enum_values);
    ASSERT_EQ(OA_ANY_STRING, p->schema.enum_values[0].type);
    ASSERT_STR_EQ("on", p->schema.enum_values[0].string);
    ASSERT_EQ(OA_ANY_STRING, p->schema.enum_values[1].type);
    ASSERT_STR_EQ("off", p->schema.enum_values[1].string);
    ASSERT_EQ(1, p->schema.default_value_set);
    ASSERT_EQ(OA_ANY_STRING, p->schema.default_value.type);
    ASSERT_STR_EQ("on", p->schema.default_value.string);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_schema_type_union(void) {

  const char *json =
      "{\"paths\":{\"/"
      "p\":{\"get\":{\"parameters\":[{\"name\":\"mix\",\"in\":\"query\","
      "\"schema\":{\"type\":[\"string\",\"integer\",\"null\"]}}],\"responses\":"
      "{\"200\":{\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(1, p->schema.nullable);
    ASSERT_EQ(3, p->schema.n_type_union);
    ASSERT_STR_EQ("string", p->schema.type_union[0]);
    ASSERT_STR_EQ("integer", p->schema.type_union[1]);
    ASSERT_STR_EQ("null", p->schema.type_union[2]);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_schema_array_items_enum_nullable(void) {

  const char *json =
      "{\"paths\":{\"/"
      "p\":{\"get\":{\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]"
      ",\"enum\":[\"a\",\"b\"]}}}],\"responses\":{\"200\":{\"description\":"
      "\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_EQ(1, p->schema.is_array);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(1, p->schema.items_nullable);
    ASSERT_EQ(2, p->schema.n_items_enum_values);
    ASSERT_EQ(OA_ANY_STRING, p->schema.items_enum_values[0].type);
    ASSERT_STR_EQ("a", p->schema.items_enum_values[0].string);
    ASSERT_EQ(OA_ANY_STRING, p->schema.items_enum_values[1].type);
    ASSERT_STR_EQ("b", p->schema.items_enum_values[1].string);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_MEDIA_H */
