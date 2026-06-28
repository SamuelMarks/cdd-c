#ifndef CDD_API_H
#define CDD_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

/**
 * @brief Configuration for from_openapi command.
 */
typedef struct {
  /** @brief input */
  const char *input;
  /** @brief input_dir */
  const char *input_dir;
  /** @brief output */
  const char *output;
  /** @brief no_github_actions */
  int no_github_actions;
  /** @brief no_installable_package */
  int no_installable_package;
  /** @brief tests */
  int tests;
  /** @brief subcommand */
  const char *subcommand; /* "to_sdk", "to_sdk_cli", "to_server" */
} cdd_from_openapi_config_t;

/**
 * @brief Configuration for to_openapi command.
 */
typedef struct {
  /** @brief input */
  const char *input;
  /** @brief output */
  const char *output;
  /** @brief cdd_to_openapi_config_t */
} cdd_to_openapi_config_t;

/**
 * @brief Configuration for to_docs_json command.
 */
typedef struct {
  /** @brief input */
  const char *input;
  /** @brief output */
  const char *output;
  /** @brief no_imports */
  int no_imports;
  /** @brief no_wrapping */
  int no_wrapping;
  /** @brief cdd_docs_json_config_t */
} cdd_docs_json_config_t;

/**
 * @brief Configuration for serve_json_rpc command.
 */
typedef struct {
  /** @brief port */
  int port;
  /** @brief listen_host */
  const char *listen_host;
  /** @brief cdd_serve_json_rpc_config_t */
} cdd_serve_json_rpc_config_t;

/**
 * @brief Configuration for generating SWIG-like bindings from C headers/source.
 */
typedef struct cdd_generate_bindings_config {
  /** @brief Path to input C header/source or directory */
  const char *input;
  /** @brief Root directory to write generated bindings */
  const char *output_dir;
  /** @brief Comma-separated list of languages, or "all" */
  const char *target_langs;
  /** @brief Shared object library name (e.g., "sqlite3") */
  const char *library_name;
  /** @brief Name of the generated namespace/module */
  const char *module_name;
  /** @brief Skip static inline functions */
  int skip_static;
  /** @brief Treat unknown structs as void* (opaque) */
  int opaque_pointers;
  /** @brief Generate a basic sanity-check test file in the target lang */
  int generate_tests;
  /** @brief Recursively parse and merge #include files into FFI IR */
  int recursive_includes;
  /** @brief cdd_generate_bindings_config_t */
} cdd_generate_bindings_config_t;

/**
 * @brief Generate code from an OpenAPI specification.
 * @param config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT enum cdd_c_error
cdd_generate_from_openapi(const cdd_from_openapi_config_t *config);

/**
 * @brief Generate an OpenAPI specification from source code.
 * @param config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT enum cdd_c_error
cdd_generate_to_openapi(const cdd_to_openapi_config_t *config);

/**
 * @brief Generate JSON documentation with code snippets for an OpenAPI
 * specification.
 * @param config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT enum cdd_c_error
cdd_generate_docs_json(const cdd_docs_json_config_t *config);

/**
 * @brief Run the JSON RPC server.
 * @param config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT enum cdd_c_error
cdd_serve_json_rpc(const cdd_serve_json_rpc_config_t *config);

/**
 * @brief Generate SWIG-like FFI bindings for multiple target languages.
 * @param config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT enum cdd_c_error
cdd_generate_bindings(const cdd_generate_bindings_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_API_H */
