/**
 * @file test_openapi_loader_media_encodings.h
 * @brief Response content types, media type specificity, and encoding loader
 * tests.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_LOADER_MEDIA_ENCODINGS_H
#define TEST_OPENAPI_LOADER_MEDIA_ENCODINGS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test test_load_response_multiple_content.
 * @return Greatest test result.
 */
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
      const struct OpenAPI_MediaType *text_mt = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_media_type(resp->content_media_types,
                                resp->n_content_media_types, "text/plain",
                                &_ast_find_media_type_1));
      text_mt = _ast_find_media_type_1;
      ASSERT(text_mt != NULL);
      ASSERT_EQ(1, text_mt->schema_set);
      ASSERT_STR_EQ("string", text_mt->schema.inline_type);
    }
  }

  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_request_body_multiple_content_with_ref.
 * @return Greatest test result.
 */
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
      const struct OpenAPI_MediaType *mt = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_media_type(op->req_body_media_types,
                                op->n_req_body_media_types, "application/json",
                                &_ast_find_media_type_2));
      mt = _ast_find_media_type_2;
      ASSERT(mt != NULL);
      ASSERT_STR_EQ("#/components/mediaTypes/application~1json", mt->ref);
    }
  }

  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_media_type_encoding.
 * @return Greatest test result.
 */
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

  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_media_type_prefix_item_encoding.
 * @return Greatest test result.
 */
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

  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_response_content_type.
 * @return Greatest test result.
 */
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

  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_response_content_type_specificity.
 * @return Greatest test result.
 */
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
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_response_content_type_params_json.
 * @return Greatest test result.
 */
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
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_MEDIA_ENCODINGS_H */
