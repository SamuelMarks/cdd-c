/**
 * @file cdd_api.h
 * @brief Core cdd api definitions and implementation.
 */
#ifndef CDD_API_H
#define CDD_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Strongly-typed boolean enum for CDD public API configurations.
 */
typedef enum cdd_bool_t {
  CDD_FALSE = 0, /**< Boolean false */
  CDD_TRUE = 1   /**< Boolean true */
} c_cdd_bool_t;

/**
 * @brief Target subcommand for `cdd_generate_from_openapi`.
 */
typedef enum cdd_openapi_subcommand_t {
  CDD_OPENAPI_SUBCOMMAND_TO_SDK = 0,     /**< Generate C Client SDK */
  CDD_OPENAPI_SUBCOMMAND_TO_SDK_CLI = 1, /**< Generate C Client SDK + CLI */
  CDD_OPENAPI_SUBCOMMAND_TO_SERVER = 2   /**< Generate C Backend Server */
} cdd_openapi_subcommand_t;

/**
 * @brief Target subcommand for `cdd_generate_from_arazzo`.
 */
typedef enum cdd_arazzo_subcommand_t {
  CDD_ARAZZO_SUBCOMMAND_TO_SDK = 0,     /**< Generate C Arazzo SDK workflows */
  CDD_ARAZZO_SUBCOMMAND_TO_SDK_CLI = 1, /**< Generate C Arazzo CLI runner */
  CDD_ARAZZO_SUBCOMMAND_TO_SERVER = 2   /**< Generate C Arazzo Server */
} cdd_arazzo_subcommand_t;

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
  c_cdd_bool_t no_github_actions;
  /** @brief no_installable_package */
  c_cdd_bool_t no_installable_package;
  /** @brief tests */
  c_cdd_bool_t tests;
  /** @brief subcommand */
  enum cdd_openapi_subcommand_t subcommand;
} cdd_from_openapi_config_t;

/**
 * @brief Configuration for to_openapi command.
 */
typedef struct {
  /** @brief input */
  const char *input;
  /** @brief output */
  const char *output;
  /** @brief base_file */
  const char *base_file;
  /** @brief self_uri */
  const char *self_uri;
  /** @brief dialect_uri */
  const char *dialect_uri;
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
  c_cdd_bool_t no_imports;
  /** @brief no_wrapping */
  c_cdd_bool_t no_wrapping;
} cdd_docs_json_config_t;

/**
 * @brief Configuration for serve_json_rpc command.
 */
typedef struct {
  /** @brief Port number to bind (default: 8080) */
  int port;
  /** @brief Optional bind address (e.g. "127.0.0.1" or NULL for INADDR_ANY) */
  const char *bind_address;
  /** @brief Maximum number of requests to serve before exiting (0 = none) */
  size_t max_requests;
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
  c_cdd_bool_t skip_static;
  /** @brief Treat unknown structs as void* (opaque) */
  c_cdd_bool_t opaque_pointers;
  /** @brief Generate a basic sanity-check test file in the target lang */
  c_cdd_bool_t generate_tests;
  /** @brief Recursively parse and merge \#include files into FFI IR */
  c_cdd_bool_t recursive_includes;
} cdd_generate_bindings_config_t;

/**
 * @brief Generate code from an OpenAPI specification.
 * @param[in] config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_generate_from_openapi(const cdd_from_openapi_config_t *config);

/**
 * @brief Generate an OpenAPI specification from source code.
 * @param[in] config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_generate_to_openapi(const cdd_to_openapi_config_t *config);

/**
 * @brief Generate JSON documentation with code snippets for an OpenAPI
 * specification.
 * @param[in] config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_generate_docs_json(const cdd_docs_json_config_t *config);

/**
 * @brief Run the JSON RPC server.
 * @param[in] config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_serve_json_rpc(const cdd_serve_json_rpc_config_t *config);

/**
 * @brief Generate SWIG-like FFI bindings for multiple target languages.
 * @param[in] config The configuration struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_generate_bindings(const cdd_generate_bindings_config_t *config);

/**
 * @brief Parses an FFI IR DSL string and generates FFI bindings.
 * @param[in] source The DSL string.
 * @param[in] config The configuration struct.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t cdd_generate_bindings_from_dsl(
    const char *source, const cdd_generate_bindings_config_t *config);

/**
 * @brief Configuration for `cdd_generate_from_arazzo`.
 */
typedef struct {
  /** @brief Target subcommand */
  enum cdd_arazzo_subcommand_t subcommand;
  /** @brief Path to input Arazzo v1.1.0 JSON/YAML file */
  const char *input;
  /** @brief Optional path to linked OpenAPI JSON/YAML file */
  const char *openapi_input;
  /** @brief Output directory or base path for generated C artifacts */
  const char *output;
  /** @brief Optional function name prefix (e.g. `"api_"`) */
  const char *func_prefix;
} cdd_from_arazzo_config_t;

/**
 * @brief Configuration for `cdd_generate_to_arazzo`.
 */
typedef struct {
  /** @brief Input C source/header file or directory path */
  const char *input;
  /** @brief Output Arazzo v1.1.0 JSON file path */
  const char *output;
  /** @brief Optional `$self` canonical URI override */
  const char *self_uri;
} cdd_to_arazzo_config_t;

/**
 * @brief Generates C SDK workflows, CLI workflow runner, or Backend Server
 * orchestrator & E2E tests from an Arazzo v1.1.0 specification.
 *
 * @param[in] config Arazzo code generation configuration.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_generate_from_arazzo(const cdd_from_arazzo_config_t *config);

/**
 * @brief Extracts an Arazzo v1.1.0 specification from C source/header files.
 *
 * @param[in] config C-to-Arazzo extraction configuration.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_generate_to_arazzo(const cdd_to_arazzo_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_API_H */
