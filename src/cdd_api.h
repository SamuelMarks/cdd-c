#ifndef CDD_API_H
#define CDD_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd_export.h"
#if defined(_MSC_VER) && _MSC_VER < 1800
/* stdbool.h is not available before MSVC 2013 */
#ifndef _STDBOOL_H
#define _STDBOOL_H
#ifndef __cplusplus
#define bool unsigned char
#define true 1
#define false 0
#endif
#define __bool_true_false_are_defined 1
#endif
#else
#include <stdbool.h>
#endif
/* clang-format on */

/**
 * @brief Configuration for from_openapi command.
 */
typedef struct {
  const char *input;
  const char *input_dir;
  const char *output;
  bool no_github_actions;
  bool no_installable_package;
  bool tests;
  const char *subcommand; /* "to_sdk", "to_sdk_cli", "to_server" */
} cdd_from_openapi_config_t;

/**
 * @brief Configuration for to_openapi command.
 */
typedef struct {
  const char *input;
  const char *output;
} cdd_to_openapi_config_t;

/**
 * @brief Configuration for to_docs_json command.
 */
typedef struct {
  const char *input;
  const char *output;
  bool no_imports;
  bool no_wrapping;
} cdd_docs_json_config_t;

/**
 * @brief Configuration for serve_json_rpc command.
 */
typedef struct {
  int port;
  const char *listen_host;
} cdd_serve_json_rpc_config_t;

/**
 * @brief Generate code from an OpenAPI specification.
 * @param config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int
cdd_generate_from_openapi(const cdd_from_openapi_config_t *config);

/**
 * @brief Generate an OpenAPI specification from source code.
 * @param config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int cdd_generate_to_openapi(const cdd_to_openapi_config_t *config);

/**
 * @brief Generate JSON documentation with code snippets for an OpenAPI
 * specification.
 * @param config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int cdd_generate_docs_json(const cdd_docs_json_config_t *config);

/**
 * @brief Expose CLI interface as a JSON-RPC server.
 * @param config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int cdd_serve_json_rpc(const cdd_serve_json_rpc_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* CDD_API_H */
