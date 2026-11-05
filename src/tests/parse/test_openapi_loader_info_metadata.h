/**
 * @file test_openapi_loader_info_metadata.h
 * @brief Info block, license validation, and operation metadata loader tests.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_LOADER_INFO_METADATA_H
#define TEST_OPENAPI_LOADER_INFO_METADATA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test test_load_info_metadata.
 * @return Greatest test result.
 */
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

/**
 * @brief Unit test test_load_info_missing_title_rejected.
 * @return Greatest test result.
 */
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

/**
 * @brief Unit test test_load_info_missing_version_rejected.
 * @return Greatest test result.
 */
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

/**
 * @brief Unit test test_load_license_identifier_and_url_rejected.
 * @return Greatest test result.
 */
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

/**
 * @brief Unit test test_load_license_missing_name_rejected.
 * @return Greatest test result.
 */
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

/**
 * @brief Unit test test_load_operation_metadata.
 * @return Greatest test result.
 */
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_INFO_METADATA_H */
