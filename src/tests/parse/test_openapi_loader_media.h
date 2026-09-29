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
#include "openapi/parse/openapi_test_helpers.h"
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

TEST test_openapi_responses_full_coverage(void) {
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_Response resp;
  struct OpenAPI_Operation op;
  struct OpenAPI_Callback cb;
  struct OpenAPI_Callback *out_cbs = NULL;
  size_t out_cb_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Spec spec;
  int k;
  size_t i;

  /* 1. NULL checks */
  memset(&rb, 0, sizeof(rb));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_request_body_object(NULL, &rb, NULL, 0, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_request_body_object((const JSON_Object *)(size_t)1,
                                               NULL, NULL, 0, NULL));

  memset(&resp, 0, sizeof(resp));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_response_object(NULL, &resp, NULL, 0, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_response_object((const JSON_Object *)(size_t)1, NULL,
                                           NULL, 0, NULL, NULL));

  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_responses(NULL, &op, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_responses((const JSON_Object *)(size_t)1, NULL, NULL,
                                     NULL));

  memset(&cb, 0, sizeof(cb));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callback_object(NULL, &cb, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callback_object(
                               (const JSON_Object *)(size_t)1, NULL, NULL, 0));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_callbacks_object(NULL, NULL, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_callbacks_object((const JSON_Object *)(size_t)1,
                                            NULL, &out_cb_count, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_callbacks_object((const JSON_Object *)(size_t)1,
                                            &out_cbs, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callbacks_object(
                               NULL, &out_cbs, &out_cb_count, NULL, 0));

  /* 2. is_valid_response_code_key tests */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key(""));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_is_valid_response_code_key("default"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key("20"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key("2000"));
  ASSERT_EQ(1, cdd_test_is_valid_response_code_key("2XX"));
  ASSERT_EQ(0, cdd_test_is_valid_response_code_key("6XX"));
  ASSERT_EQ(0, cdd_test_is_valid_response_code_key("0XX"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_valid_response_code_key("200"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key("2a0"));
  ASSERT_EQ(0, cdd_test_is_valid_response_code_key("2X0"));

  /* Pre-set description on request body */
  memset(&rb, 0, sizeof(rb));
  rb.description = strdup("existing");
  jv = json_parse_string(
      "{\"description\":\"new\",\"content\":{\"application/json\":{}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, NULL, 0, NULL));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  /* 3. Request body ref resolution and overrides */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_request_bodies = 1;
  spec.component_request_body_names = (char **)calloc(1, sizeof(char *));
  spec.component_request_body_names[0] = strdup("MyRb");
  spec.component_request_bodies = (struct OpenAPI_RequestBody *)calloc(
      1, sizeof(struct OpenAPI_RequestBody));
  spec.component_request_bodies[0].description = strdup("comp_desc");
  spec.component_request_bodies[0].required = 1;

  jv = json_parse_string("{\"$ref\":\"#/components/requestBodies/"
                         "MyRb\",\"description\":\"new_desc\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 1, "op1"));
    ASSERT_STR_EQ("new_desc", rb.description);
    ASSERT_EQ(1, rb.required);
    cdd_test_free_request_body(&rb);

    /* ref with resolve_refs = 0 and spec = NULL */
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, NULL, 0, "op1"));
    cdd_test_free_request_body(&rb);

    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, NULL, 1, "op1"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"$ref\":\"#/components/requestBodies/NotFound\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 1, "op1"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 4. Request body invalid content */
  {
    const char *bad_rb[2];
    size_t n_bad = 2;
    bad_rb[0] = "{}";
    bad_rb[1] = "{\"content\":{}}";
    for (i = 0; i < n_bad; ++i) {
      jv = json_parse_string(bad_rb[i]);
      if (jv) {
        jo = json_value_get_object(jv);
        memset(&rb, 0, sizeof(rb));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_test_parse_request_body_object(jo, &rb, NULL, 0, "op1"));
        cdd_test_free_request_body(&rb);
        json_value_free(jv);
      }
    }
  }

  /* 5. Request body inline schemas: array items, object, itemSchema */
  memset(&spec, 0, sizeof(spec));
  jv = json_parse_string(
      "{\"description\":\"desc\",\"required\":false,\"content\":{\"application/"
      "json\":"
      "{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"object\","
      "\"properties\":{\"x\":{\"type\":\"string\"}}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  /* Non-object schemas / non-object items in request body */
  jv = json_parse_string("{\"content\":{\"application/"
                         "json\":{\"schema\":{\"type\":\"array\",\"items\":{"
                         "\"type\":\"string\"}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"content\":{\"application/"
                         "json\":{\"schema\":{\"type\":\"string\"}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"content\":{\"application/"
                         "json\":{\"itemSchema\":{\"type\":\"string\"}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  /* Array schema without items */
  jv = json_parse_string(
      "{\"content\":{\"application/json\":{\"schema\":{\"type\":\"array\"}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  /* Properties without type */
  jv = json_parse_string(
      "{\"content\":{\"application/"
      "json\":{\"schema\":{\"properties\":{\"x\":{\"type\":\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  /* itemSchema boolean true and itemSchema properties without type */
  jv = json_parse_string(
      "{\"content\":{\"application/json\":{\"itemSchema\":true}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"content\":{\"application/"
                         "json\":{\"itemSchema\":{\"properties\":{\"x\":{"
                         "\"type\":\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  /* Request body with boolean schema */
  jv = json_parse_string(
      "{\"content\":{\"application/json\":{\"schema\":true}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  /* Request body without description, and with invalid media value */
  jv = json_parse_string("{\"content\":{\"application/json\":123}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"content\":{\"application/json\":{}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"content\":{\"application/"
                         "json\":{\"schema\":{\"type\":\"object\","
                         "\"properties\":{\"x\":{\"type\":\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"content\":{\"application/"
                         "json\":{\"itemSchema\":{\"type\":\"object\","
                         "\"properties\":{\"x\":{\"type\":\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  /* Request body with examples array and single example */
  jv = json_parse_string(
      "{\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"},"
      "\"examples\":{\"e1\":{\"value\":\"v1\"},\"e2\":{\"value\":\"v2\"}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"},"
      "\"example\":\"single_val\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }

  /* Request body with content media ref */
  jv = json_parse_string("{\"content\":{\"application/json\":{\"$ref\":\"#/"
                         "components/mediaTypes/Mt\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    cdd_test_free_request_body(&rb);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 6. Response ref resolution and overrides */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_responses = 1;
  spec.component_response_names = (char **)calloc(1, sizeof(char *));
  spec.component_response_names[0] = strdup("MyResp");
  spec.component_responses =
      (struct OpenAPI_Response *)calloc(1, sizeof(struct OpenAPI_Response));
  spec.component_responses[0].description = strdup("comp_resp_desc");

  jv = json_parse_string(
      "{\"$ref\":\"#/components/responses/"
      "MyResp\",\"summary\":\"sum\",\"description\":\"override\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 1, "op", "200"));
    ASSERT_STR_EQ("override", resp.description);
    ASSERT_STR_EQ("sum", resp.summary);
    cdd_test_free_response(&resp);

    /* ref with resolve_refs = 0 and spec = NULL */
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, NULL, 0, "op", "200"));
    cdd_test_free_response(&resp);

    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, NULL, 1, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"$ref\":\"#/components/responses/NotFound\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 1, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 7. Response Swagger 2.0 schema and headers and links */
  memset(&spec, 0, sizeof(spec));
  spec.swagger_version = strdup("2.0");
  jv = json_parse_string(
      "{\"description\":\"desc\",\"schema\":{\"type\":\"string\"},"
      "\"headers\":{\"X-Rate\":{\"description\":\"rate\"}},"
      "\"links\":{\"GetItem\":{\"operationId\":\"getItem\"}},"
      "\"x-custom\":\"val\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    ASSERT_EQ(1, resp.schema_set);
    ASSERT_EQ(1, resp.n_headers);
    ASSERT_EQ(1, resp.n_links);
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  /* Swagger 2.0 with boolean schema */
  jv = json_parse_string("{\"description\":\"desc\",\"schema\":true}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  /* Swagger 2.0 schema error */
  jv = json_parse_string(
      "{\"description\":\"d\",\"schema\":{\"$ref\":\"#/definitions/Bad\"}}");
  if (jv) {
    g_cdd_strdup_fail = 2;
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_parse_response_object(json_value_get_object(jv), &resp,
                                             &spec, 0, "op", "200"));
    g_cdd_strdup_fail = 0;
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 8. Response inline schemas, itemSchema, and media ref */
  memset(&spec, 0, sizeof(spec));
  jv = json_parse_string(
      "{\"description\":\"desc\",\"content\":{\"application/json\":"
      "{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"object\","
      "\"properties\":{\"k\":{\"type\":\"string\"}}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  /* Array schema without items */
  jv = json_parse_string("{\"description\":\"d\",\"content\":{\"application/"
                         "json\":{\"schema\":{\"type\":\"array\"}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  /* Properties without type */
  jv = json_parse_string(
      "{\"description\":\"d\",\"content\":{\"application/"
      "json\":{\"schema\":{\"properties\":{\"x\":{\"type\":\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  /* itemSchema boolean true and itemSchema properties without type */
  jv = json_parse_string("{\"description\":\"d\",\"content\":{\"application/"
                         "json\":{\"itemSchema\":true}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"description\":\"d\",\"content\":{\"application/"
                         "json\":{\"itemSchema\":{\"properties\":{\"x\":{"
                         "\"type\":\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  /* Non-object schemas in response */
  jv = json_parse_string(
      "{\"description\":\"desc\",\"content\":{\"application/json\":"
      "{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"string\"}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"description\":\"desc\",\"content\":{\"application/json\":"
      "{\"schema\":{\"type\":\"string\"}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);

    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, NULL, 0, "op", "200"));
    cdd_test_free_response(&resp);

    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", NULL));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"description\":\"desc\",\"content\":{\"application/json\":"
      "{\"itemSchema\":{\"type\":\"string\"}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  /* Response with boolean schema and invalid media content */
  jv = json_parse_string("{\"description\":\"desc\",\"content\":{\"application/"
                         "json\":{\"schema\":true}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"description\":\"desc\",\"content\":{\"application/json\":123}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"description\":\"desc\",\"content\":{\"application/json\":"
      "{\"schema\":{\"type\":\"object\",\"properties\":{\"k\":{\"type\":"
      "\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"description\":\"desc\",\"content\":{\"application/json\":"
      "{\"itemSchema\":{\"type\":\"object\",\"properties\":{\"k\":{\"type\":"
      "\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"description\":\"desc\",\"content\":{\"application/json\":"
      "{\"$ref\":\"#/components/mediaTypes/Mt\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  /* Response with examples array and single example */
  jv = json_parse_string("{\"description\":\"desc\",\"content\":{\"application/"
                         "json\":{\"schema\":{\"type\":\"string\"},"
                         "\"examples\":{\"e1\":{\"value\":\"v1\"}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"description\":\"desc\",\"content\":{\"application/"
                         "json\":{\"schema\":{\"type\":\"string\"},"
                         "\"example\":\"val\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    cdd_test_free_response(&resp);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 9. parse_responses error and success branches */
  memset(&op, 0, sizeof(op));
  jv = json_parse_string("{}");
  if (jv) {
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_test_parse_responses(json_value_get_object(jv), &op, NULL, "op"));
    json_value_free(jv);
  }

  jv = json_parse_string("{\"x-ext\":\"custom\"}");
  if (jv) {
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_test_parse_responses(json_value_get_object(jv), &op, NULL, "op"));
    json_value_free(jv);
  }

  jv = json_parse_string("{\"bad_code\":{\"description\":\"desc\"}}");
  if (jv) {
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_test_parse_responses(json_value_get_object(jv), &op, NULL, "op"));
    json_value_free(jv);
  }

  /* Responses with non-object response value (resp_obj == NULL) */
  jv = json_parse_string("{\"200\": 123, \"x-custom\": \"val\"}");
  if (jv) {
    memset(&op, 0, sizeof(op));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_responses(json_value_get_object(jv),
                                                      &op, NULL, "op"));
    cdd_test_free_operation(&op);
    json_value_free(jv);
  }

  /* Responses without extensions */
  jv = json_parse_string("{\"200\": {\"description\": \"OK\"}}");
  if (jv) {
    memset(&op, 0, sizeof(op));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_responses(json_value_get_object(jv),
                                                      &op, NULL, "op"));
    cdd_test_free_operation(&op);
    json_value_free(jv);
  }

  /* 10. Callback object ref and extensions */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_callbacks = 1;
  spec.component_callbacks =
      (struct OpenAPI_Callback *)calloc(1, sizeof(struct OpenAPI_Callback));
  spec.component_callbacks[0].name = strdup("MyCb");
  spec.component_callbacks[0].summary = strdup("comp_sum");

  jv =
      json_parse_string("{\"$ref\":\"#/components/callbacks/"
                        "MyCb\",\"summary\":\"sum\",\"description\":\"desc\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callback_object(jo, &cb, &spec, 1));
    ASSERT_STR_EQ("sum", cb.summary);
    ASSERT_STR_EQ("desc", cb.description);
    cdd_test_free_callback(&cb);

    /* ref with resolve_refs = 0 and spec = NULL */
    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callback_object(jo, &cb, NULL, 0));
    cdd_test_free_callback(&cb);

    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callback_object(jo, &cb, NULL, 1));
    cdd_test_free_callback(&cb);
    json_value_free(jv);

    /* ref without summary and without description */
    jv = json_parse_string("{\"$ref\":\"#/components/callbacks/MyCb\"}");
    if (jv) {
      memset(&cb, 0, sizeof(cb));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callback_object(
                                   json_value_get_object(jv), &cb, &spec, 1));
      cdd_test_free_callback(&cb);
      json_value_free(jv);
    }
  }

  jv = json_parse_string("{\"$ref\":\"#/components/callbacks/NotFound\"}");
  if (jv) {
    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callback_object(
                                 json_value_get_object(jv), &cb, &spec, 1));
    cdd_test_free_callback(&cb);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* Callback with paths and extensions */
  jv =
      json_parse_string("{\"x-ext\":\"val\",\"{$request.query.url}\":{\"post\":"
                        "{\"responses\":{\"200\":{\"description\":\"OK\"}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callback_object(jo, &cb, NULL, 0));
    cdd_test_free_callback(&cb);
    json_value_free(jv);
  }

  /* Pre-set summary and description on callback */
  memset(&cb, 0, sizeof(cb));
  cb.summary = strdup("existing_sum");
  cb.description = strdup("existing_desc");
  jv = json_parse_string("{\"$ref\":\"#/components/callbacks/"
                         "MyCb\",\"summary\":\"s\",\"description\":\"d\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_callback_object(jo, &cb, NULL, 0));
    cdd_test_free_callback(&cb);
    json_value_free(jv);
  }

  /* Callback with invalid method in paths */
  jv = json_parse_string("{\"/path\":{\"invalid_method\":{}}}");
  if (jv) {
    memset(&cb, 0, sizeof(cb));
    cdd_test_parse_callback_object(json_value_get_object(jv), &cb, NULL, 0);
    cdd_test_free_callback(&cb);
    json_value_free(jv);
  }

  /* 11. parse_callbacks_object */
  jv = json_parse_string("{}");
  if (jv) {
    out_cbs = NULL;
    out_cb_count = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_callbacks_object(
                  json_value_get_object(jv), &out_cbs, &out_cb_count, NULL, 0));
    ASSERT_EQ(0, out_cb_count);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"cb1\":{\"x-ext\":\"v\"},\"cb2\":123}");
  if (jv) {
    out_cbs = NULL;
    out_cb_count = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_callbacks_object(
                  json_value_get_object(jv), &out_cbs, &out_cb_count, NULL, 0));
    ASSERT_EQ(2, out_cb_count);
    cdd_test_free_callback(&out_cbs[0]);
    cdd_test_free_callback(&out_cbs[1]);
    free(out_cbs);
    json_value_free(jv);
  }

  /* 12. OOM loop on responses, request bodies, and callbacks */
  {
    const char *oom_resps[13];
    size_t n_oom = 13;
    oom_resps[0] =
        "{\"description\":\"desc\",\"summary\":\"sum\","
        "\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"object\",\"properties\":{\"x\":{"
        "\"type\":\"string\"}}},"
        "\"examples\":{\"e1\":{\"value\":\"v1\"}}}},"
        "\"headers\":{\"X-H\":{\"description\":\"h\"}},"
        "\"links\":{\"L\":{\"operationId\":\"op\"}},\"x-ext\":\"val\"}";
    oom_resps[1] = "{\"content\":{\"application/"
                   "json\":{\"schema\":{\"type\":\"object\",\"properties\":{"
                   "\"x\":{\"type\":\"string\"}}},"
                   "\"examples\":{\"e1\":{\"value\":\"v1\"}}}},\"description\":"
                   "\"desc\",\"x-ext\":\"val\"}";
    oom_resps[2] =
        "{\"200\":{\"description\":\"OK\",\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"string\"}}}},\"x-resp-ext\":\"val\"}";
    oom_resps[3] = "{\"myCb\":{\"x-ext\":\"val\"}}";
    oom_resps[4] =
        "{\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"object\","
        "\"properties\":{\"a\":{\"type\":\"string\"}}}}}}}";
    oom_resps[5] = "{\"content\":{\"application/"
                   "json\":{\"itemSchema\":{\"type\":\"object\",\"properties\":"
                   "{\"a\":{\"type\":\"string\"}}}}}}";
    oom_resps[6] =
        "{\"description\":\"d\",\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"object\","
        "\"properties\":{\"a\":{\"type\":\"string\"}}}}}}}";
    oom_resps[7] = "{\"description\":\"d\",\"content\":{\"application/"
                   "json\":{\"itemSchema\":{\"type\":\"object\",\"properties\":"
                   "{\"a\":{\"type\":\"string\"}}}}}}";
    oom_resps[8] = "{\"content\":{\"application/json\":{\"$ref\":\"#/"
                   "components/mediaTypes/Mt\"}}}";
    oom_resps[9] = "{\"description\":\"d\",\"content\":{\"application/"
                   "json\":{\"$ref\":\"#/components/mediaTypes/Mt\"}}}";
    oom_resps[10] = "{\"content\":{\"application/"
                    "json\":{\"schema\":{\"type\":\"string\"}}}}";
    oom_resps[11] = "{\"myCb\":{\"$ref\":\"#/components/callbacks/"
                    "MyCb\",\"summary\":\"s\",\"description\":\"d\"}}";
    oom_resps[12] = "{\"description\":\"d\",\"content\":{\"application/"
                    "json\":{\"example\":\"single_val\"}}}";

    memset(&spec, 0, sizeof(spec));
    spec.n_component_callbacks = 1;
    spec.component_callbacks =
        (struct OpenAPI_Callback *)calloc(1, sizeof(struct OpenAPI_Callback));
    spec.component_callbacks[0].name = strdup("MyCb");

    for (i = 0; i < n_oom; ++i) {
      jv = json_parse_string(oom_resps[i]);
      if (jv) {
        jo = json_value_get_object(jv);
        for (k = 1; k <= 30; ++k) {
          if (i == 0 || i == 6 || i == 7 || i == 9 || i == 12) {
            g_cdd_alloc_fail = k;
            memset(&resp, 0, sizeof(resp));
            cdd_test_parse_response_object(jo, &resp, &spec, 1, "op", "200");
            cdd_test_free_response(&resp);
            g_cdd_alloc_fail = 0;

            g_cdd_strdup_fail = k;
            memset(&resp, 0, sizeof(resp));
            cdd_test_parse_response_object(jo, &resp, &spec, 1, "op", "200");
            cdd_test_free_response(&resp);
            g_cdd_strdup_fail = 0;
          } else if (i == 1 || i == 4 || i == 5 || i == 8 || i == 10) {
            g_cdd_alloc_fail = k;
            memset(&rb, 0, sizeof(rb));
            cdd_test_parse_request_body_object(jo, &rb, &spec, 1, "op");
            cdd_test_free_request_body(&rb);
            g_cdd_alloc_fail = 0;

            g_cdd_strdup_fail = k;
            memset(&rb, 0, sizeof(rb));
            cdd_test_parse_request_body_object(jo, &rb, &spec, 1, "op");
            cdd_test_free_request_body(&rb);
            g_cdd_strdup_fail = 0;
          } else if (i == 2) {
            g_cdd_alloc_fail = k;
            memset(&op, 0, sizeof(op));
            cdd_test_parse_responses(jo, &op, &spec, "op");
            cdd_test_free_operation(&op);
            g_cdd_alloc_fail = 0;

            g_cdd_strdup_fail = k;
            memset(&op, 0, sizeof(op));
            cdd_test_parse_responses(jo, &op, &spec, "op");
            cdd_test_free_operation(&op);
            g_cdd_strdup_fail = 0;
          } else if (i == 3 || i == 11) {
            g_cdd_alloc_fail = k;
            out_cbs = NULL;
            out_cb_count = 0;
            cdd_test_parse_callbacks_object(jo, &out_cbs, &out_cb_count, &spec,
                                            1);
            if (out_cbs) {
              size_t m;
              for (m = 0; m < out_cb_count; ++m)
                cdd_test_free_callback(&out_cbs[m]);
              free(out_cbs);
            }
            g_cdd_alloc_fail = 0;

            g_cdd_strdup_fail = k;
            out_cbs = NULL;
            out_cb_count = 0;
            cdd_test_parse_callbacks_object(jo, &out_cbs, &out_cb_count, &spec,
                                            1);
            if (out_cbs) {
              size_t m;
              for (m = 0; m < out_cb_count; ++m)
                cdd_test_free_callback(&out_cbs[m]);
              free(out_cbs);
            }
            g_cdd_strdup_fail = 0;
          }
        }
        json_value_free(jv);
      }
    }
    openapi_spec_free(&spec);
  }

  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 0;
  PASS();
}

TEST test_openapi_media_all_branches(void) {
  struct OpenAPI_MediaType mt;
  struct OpenAPI_MediaType *out_mts = NULL;
  size_t out_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  size_t blen = 0;
  int spec_val = 0;
  int rank_val = 0;
  int idx_val = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_MediaType comp_mt;
  char *comp_name = (char *)(size_t) "comp_mt";

  memset(&spec, 0, sizeof(spec));
  memset(&comp_mt, 0, sizeof(comp_mt));
  comp_mt.name = (char *)(size_t) "application/json";
  spec.n_component_media_types = 1;
  spec.component_media_type_names = &comp_name;
  spec.component_media_types = &comp_mt;

  /* 1. media_type_base_len */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_base_len(NULL, &blen));
  ASSERT_EQ(0, blen);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_base_len("text/plain", &blen));
  ASSERT_EQ(10, blen);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_base_len("text/plain; charset=utf-8", &blen));
  ASSERT_EQ(10, blen);

  /* 2. media_type_base_equal */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_base_equal(NULL, "text/plain"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_base_equal("text/plain", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_base_equal(NULL, NULL));
  ASSERT_EQ(0,
            cdd_test_media_type_base_equal("text/plain", "application/json"));
  ASSERT_EQ(0, cdd_test_media_type_base_equal("text/plain", "text/p"));
  ASSERT_EQ(
      1, cdd_test_media_type_base_equal("text/plain; a=1", "text/plain; b=2"));
  ASSERT_EQ(0, cdd_test_media_type_base_equal("text/plaiX", "text/plain"));

  /* 3. media_type_is_json */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_is_json(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_is_json(""));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_is_json("text"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_is_json("application/jpeg"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_is_json("text/plain"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_media_type_is_json("application/json"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_media_type_is_json("application/json; charset=utf-8"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_media_type_is_json("application/problem+json"));
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_media_type_is_json("application/problem+json; charset=utf-8"));

  /* 4. media_type_specificity */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity(NULL, &spec_val));
  ASSERT_EQ(0, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity("", &spec_val));
  ASSERT_EQ(0, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity("", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_specificity("invalidnoslash", &spec_val));
  ASSERT_EQ(2, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_specificity("invalidnoslash", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_specificity("/leading", &spec_val));
  ASSERT_EQ(0, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity("/leading", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_specificity("trailing/", &spec_val));
  ASSERT_EQ(2, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity("*/*", &spec_val));
  ASSERT_EQ(0, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity("*/*", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_specificity("text/*", &spec_val));
  ASSERT_EQ(1, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity("text/*", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity("x/*", &spec_val));
  ASSERT_EQ(1, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity("*/a", &spec_val));
  ASSERT_EQ(2, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_specificity("*/json", &spec_val));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_specificity("text/plain", &spec_val));
  ASSERT_EQ(2, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_specificity("text/plain", NULL));

  /* 5. media_type_preference_rank */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_preference_rank(NULL, &rank_val));
  ASSERT_EQ(0, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_preference_rank(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_preference_rank("application/json", &rank_val));
  ASSERT_EQ(3, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_preference_rank("application/json", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_preference_rank(
                               "application/x-www-form-urlencoded", &rank_val));
  ASSERT_EQ(2, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_preference_rank(
                               "application/x-www-form-urlencoded", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_preference_rank(
                               "multipart/form-data", &rank_val));
  ASSERT_EQ(1, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_preference_rank("multipart/form-data", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_preference_rank("image/png", &rank_val));
  ASSERT_EQ(0, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_preference_rank("image/png", NULL));

  /* 6. select_primary_media_type_index */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_select_primary_media_type_index(NULL, 0, &idx_val));
  ASSERT_EQ(-1, idx_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_select_primary_media_type_index(NULL, 0, NULL));
  {
    struct OpenAPI_MediaType test_mts[6];
    memset(test_mts, 0, sizeof(test_mts));
    test_mts[0].name = (char *)(size_t) "*/*";
    test_mts[1].name = (char *)(size_t) "text/plain";
    test_mts[2].name = (char *)(size_t) "application/json";
    test_mts[3].name = (char *)(size_t) "multipart/form-data";
    test_mts[4].name = (char *)(size_t) "image/png";
    test_mts[5].name = (char *)(size_t) "*/*";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_select_primary_media_type_index(test_mts, 6, &idx_val));
    ASSERT_EQ(2, idx_val);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_select_primary_media_type_index(test_mts, 6, NULL));
  }

  /* 7. find_media_object_by_name */
  {
    JSON_Object *found = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_media_object_by_name(
                                 NULL, "application/json", &found));
    ASSERT_EQ(NULL, found);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_media_object_by_name(NULL, NULL, &found));
    ASSERT_EQ(NULL, found);

    jv = json_parse_string(
        "{\"application/json\": {\"schema\": {\"type\": \"string\"}}, "
        "\"text/plain; charset=utf-8\": {\"schema\": {\"type\": \"string\"}}}");
    ASSERT(jv != NULL);
    jo = json_value_get_object(jv);

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_media_object_by_name(jo, NULL, &found));
    ASSERT_EQ(NULL, found);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_media_object_by_name(
                                 jo, "application/json", &found));
    ASSERT(found != NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_media_object_by_name(jo, "text/plain", &found));
    ASSERT(found != NULL);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_media_object_by_name(
                                 jo, "nonexistent/mime", &found));
    ASSERT_EQ(NULL, found);
    json_value_free(jv);
  }

  /* 8. parse_media_type_object */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_media_type_object(NULL, &mt, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_media_type_object(NULL, NULL, &spec, 0));

  jv = json_parse_string("{\"encoding\": {}, \"prefixEncoding\": []}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_media_type_object(jo, NULL, &spec, 0));
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  json_value_free(jv);

  jv = json_parse_string("{\"encoding\": {}, \"itemEncoding\": {}}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  json_value_free(jv);

  /* $ref with and without resolution */
  jv = json_parse_string("{\"$ref\": \"#/components/mediaTypes/comp_mt\"}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_type_object(jo, &mt, &spec, 1));
  cdd_test_free_media_type(&mt);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  cdd_test_free_media_type(&mt);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_type_object(jo, &mt, NULL, 1));
  cdd_test_free_media_type(&mt);
  json_value_free(jv);

  /* $ref with missing component */
  jv = json_parse_string("{\"$ref\": \"#/components/mediaTypes/missing\"}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_type_object(jo, &mt, &spec, 1));
  cdd_test_free_media_type(&mt);

  /* $ref OOM */
  g_cdd_strdup_fail = 1;
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_media_type_object(jo, &mt, &spec, 1));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* schema boolean vs object vs other */
  jv = json_parse_string("{\"schema\": true, \"itemSchema\": false}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  ASSERT_EQ(1, mt.schema_set);
  ASSERT_EQ(1, mt.schema.schema_is_boolean);
  ASSERT_EQ(1, mt.item_schema_set);
  ASSERT_EQ(1, mt.item_schema.schema_is_boolean);
  cdd_test_free_media_type(&mt);
  json_value_free(jv);

  jv = json_parse_string("{\"schema\": \"non_obj\", \"itemSchema\": 123}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  cdd_test_free_media_type(&mt);
  json_value_free(jv);

  /* itemEncoding with OOM and success */
  jv = json_parse_string(
      "{\"itemEncoding\": {\"contentType\": \"text/plain\"}}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  g_cdd_alloc_fail = 0;
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  cdd_test_free_media_type(&mt);
  json_value_free(jv);

  /* itemEncoding error branch */
  jv = json_parse_string(
      "{\"itemEncoding\": {\"encoding\": {}, \"prefixEncoding\": []}}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  cdd_test_free_media_type(&mt);
  json_value_free(jv);

  /* prefixEncoding error branch */
  jv = json_parse_string(
      "{\"prefixEncoding\": [{\"encoding\": {}, \"prefixEncoding\": []}]}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  cdd_test_free_media_type(&mt);
  json_value_free(jv);

  /* collect_extensions failure */
  jv = json_parse_string("{\"x-ext\": \"val\"}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_media_type_object(jo, &mt, &spec, 0));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 9. parse_content_object */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_content_object(NULL, NULL, &out_count, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_content_object(NULL, &out_mts, NULL, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_content_object(NULL, &out_mts,
                                                         &out_count, &spec, 0));
  ASSERT_EQ(NULL, out_mts);
  ASSERT_EQ(0, out_count);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  ASSERT_EQ(NULL, out_mts);
  ASSERT_EQ(0, out_count);
  json_value_free(jv);

  /* Content with non-object values -> valid == 0 */
  jv = json_parse_string("{\"application/json\": \"not_an_obj\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  ASSERT_EQ(NULL, out_mts);
  ASSERT_EQ(0, out_count);
  json_value_free(jv);

  /* Content OOM on calloc */
  jv = json_parse_string("{\"application/json\": {}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  g_cdd_alloc_fail = 0;

  /* Content OOM on strdup */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  g_cdd_strdup_fail = 0;

  /* Content with child parse error */
  json_value_free(jv);
  jv = json_parse_string(
      "{\"application/json\": {\"encoding\": {}, \"prefixEncoding\": []}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  json_value_free(jv);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_MEDIA_H */
