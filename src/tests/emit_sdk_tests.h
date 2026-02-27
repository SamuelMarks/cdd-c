/**
 * @file codegen_sdk_tests.h
 * @brief Logic for generating integration tests for the C SDK.
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_SDK_TESTS_H
#define C_CDD_CODEGEN_SDK_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "c_cdd_export.h"
#include "routes/parse_openapi.h"

/**
 * @brief Configuration for test generation.
 */
struct SdkTestsConfig {
  const char *client_header; /**< Name of the generated client header to include
                                (e.g. "petstore.h") */
  const char *mock_server_url; /**< Base URL for the mock server execution (e.g.
                                  "http://localhost:8080") */
  const char
      *func_prefix; /**< Prefix used in generated functions (e.g. "api_") */
};

/**
 * @brief Generate a standalone C file containing tests for the SDK.
 *
 * Iterates through all operations in the spec, generating a `TEST` function
 * for each using `greatest.h`. The tests will instantiate the client, call the
 * operation (passing simplified/dummy args), and verify the return code
 * structure. This ensures the generated code compiles, links, and runs
 * correctly against a mock backend.
 *
 * @param[in] fp The output file stream.
 * @param[in] spec The parsed OpenAPI spec.
 * @param[in] config The generator configuration.
 * @return 0 on success, error code (EINVAL/EIO) on failure.
 */
extern C_CDD_EXPORT int
codegen_sdk_tests_generate(FILE *fp, const struct OpenAPI_Spec *spec,
                           const struct SdkTestsConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_SDK_TESTS_H */
