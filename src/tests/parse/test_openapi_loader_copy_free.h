/**
 * @file test_openapi_loader_copy_free.h
 * @brief Umbrella header for OpenAPI loader copy, free, and OOM branch tests.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_LOADER_COPY_FREE_H
#define TEST_OPENAPI_LOADER_COPY_FREE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_copy_components.h"
#include "parse/test_openapi_copy_operations.h"
#include "parse/test_openapi_copy_schema.h"
#include "parse/test_openapi_free_and_registry.h"
/* clang-format on */

/**
 * @brief Macro executing all OpenAPI copy, free, and registry unit tests.
 */
#define OPENAPI_COPY_FREE_TESTS()                                              \
  do {                                                                         \
    RUN_TEST(test_openapi_copy_components_helpers);                            \
    RUN_TEST(test_openapi_copy_components_oom);                                \
    RUN_TEST(test_openapi_copy_components_subobject_failures);                 \
    RUN_TEST(test_openapi_copy_components_branches);                           \
    RUN_TEST(test_openapi_copy_operations_helpers);                            \
    RUN_TEST(test_openapi_copy_operations_oom);                                \
    RUN_TEST(test_openapi_copy_operations_advanced);                           \
    RUN_TEST(test_openapi_copy_operations_branches);                           \
    RUN_TEST(test_openapi_copy_schema_helpers);                                \
    RUN_TEST(test_openapi_copy_schema_comprehensive);                          \
    RUN_TEST(test_openapi_copy_schema_subobject_failures);                     \
    RUN_TEST(test_openapi_copy_schema_branches);                               \
    RUN_TEST(test_openapi_free_helpers);                                       \
    RUN_TEST(test_openapi_doc_registry);                                       \
    RUN_TEST(test_openapi_free_extra_and_branches);                            \
  } while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_COPY_FREE_H */
