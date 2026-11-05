/**
 * @file test_openapi_loader_media.h
 * @brief Umbrella header for OpenAPI response content types, media encodings,
 * info/operation metadata, inline schemas, responses, request bodies, and
 * callbacks tests.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_LOADER_MEDIA_H
#define TEST_OPENAPI_LOADER_MEDIA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_media_encodings.h"
#include "parse/test_openapi_loader_info_metadata.h"
#include "parse/test_openapi_loader_inline_schemas.h"
#include "parse/test_openapi_request_body_coverage.h"
#include "parse/test_openapi_responses_coverage.h"
#include "parse/test_openapi_callbacks_and_oom_coverage.h"
#include "parse/test_openapi_media_coverage.h"
/* clang-format on */

/**
 * @brief Macro executing all OpenAPI request body, response, callback, and OOM
 * coverage tests.
 */
#define OPENAPI_RESPONSES_COVERAGE_TESTS()                                     \
  do {                                                                         \
    RUN_TEST(test_openapi_request_body_refs_and_validation);                   \
    RUN_TEST(test_openapi_request_body_inline_schemas_and_examples);           \
    RUN_TEST(test_openapi_response_codes_and_refs);                            \
    RUN_TEST(test_openapi_response_inline_schemas_and_parse_responses);        \
    RUN_TEST(test_openapi_callbacks_coverage);                                 \
    RUN_TEST(test_openapi_responses_request_bodies_callbacks_oom);             \
  } while (0)

/**
 * @brief Macro executing all OpenAPI media type helper and content object
 * branch coverage tests.
 */
#define OPENAPI_MEDIA_COVERAGE_TESTS()                                         \
  do {                                                                         \
    RUN_TEST(test_openapi_media_type_helpers_branches);                        \
    RUN_TEST(test_openapi_media_type_and_content_parse_branches);              \
  } while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_MEDIA_H */
