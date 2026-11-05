/**
 * @file test_openapi_find_coverage.h
 * @brief Umbrella header for OpenAPI component lookup, schema constraints,
 * examples, OAuth flows, and validation coverage tests.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_FIND_COVERAGE_H
#define TEST_OPENAPI_FIND_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_find_components.h"
#include "parse/test_openapi_schemas_coverage.h"
#include "parse/test_openapi_examples_coverage.h"
#include "parse/test_openapi_oauth_coverage.h"
#include "parse/test_openapi_validation_querystring.h"
#include "parse/test_openapi_validation_op_ids.h"
/* clang-format on */

/**
 * @brief Macro executing all OpenAPI find, schemas, examples, OAuth, and
 * validation branch coverage tests.
 */
#define OPENAPI_FIND_COVERAGE_TESTS()                                          \
  do {                                                                         \
    RUN_TEST(test_openapi_find_local_components);                              \
    RUN_TEST(test_openapi_find_external_components);                           \
    RUN_TEST(test_openapi_schemas_type_and_constraints);                       \
    RUN_TEST(test_openapi_schemas_enums_and_refs);                             \
    RUN_TEST(test_openapi_examples_copy_and_find);                             \
    RUN_TEST(test_openapi_examples_parse_all_branches);                        \
    RUN_TEST(test_openapi_oauth_scopes_and_flows_branches);                    \
    RUN_TEST(test_openapi_validation_querystring_branches);                    \
    RUN_TEST(test_openapi_validation_operation_ids_branches);                  \
  } while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_FIND_COVERAGE_H */
