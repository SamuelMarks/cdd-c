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
 * @cdd cli(name = "from_openapi", about = "Generate C code from OpenAPI")
 */
typedef struct cdd_from_openapi_config {
  /** @brief input @cdd cli(short = 'i', long = "input", env = "CDD_INPUT") */
  const char *input;
  /** @brief input_dir @cdd cli(long = "input-dir", env = "CDD_INPUT_DIR") */
  const char *input_dir;
  /** @brief output @cdd cli(short = 'o', long = "output", env = "CDD_OUTPUT")
   */
  const char *output;
  /** @brief no_github_actions @cdd cli(long = "no-github-actions") */
  c_cdd_bool_t no_github_actions;
  /** @brief no_installable_package @cdd cli(long = "no-installable-package") */
  c_cdd_bool_t no_installable_package;
  /** @brief tests @cdd cli(long = "tests") */
  c_cdd_bool_t tests;
  /** @brief subcommand @cdd cli(positional, index = 0) */
  enum cdd_openapi_subcommand_t subcommand;
} cdd_from_openapi_config_t;

/**
 * @brief Configuration for to_openapi command.
 * @cdd cli(name = "to_openapi", about = "Generate OpenAPI from C code")
 */
typedef struct cdd_to_openapi_config {
  /** @brief input @cdd cli(short = 'i', long = "input", env = "CDD_INPUT") */
  const char *input;
  /** @brief output @cdd cli(short = 'o', long = "output", env = "CDD_OUTPUT",
   * default = "openapi.json") */
  const char *output;
  /** @brief base_file @cdd cli(long = "base") */
  const char *base_file;
  /** @brief self_uri @cdd cli(long = "self") */
  const char *self_uri;
  /** @brief dialect_uri @cdd cli(long = "dialect") */
  const char *dialect_uri;
} cdd_to_openapi_config_t;

/**
 * @brief Configuration for to_docs_json command.
 * @cdd cli(name = "to_docs_json", about = "Generate JSON docs from OpenAPI")
 */
typedef struct cdd_docs_json_config {
  /** @brief input @cdd cli(short = 'i', long = "input", env = "CDD_INPUT") */
  const char *input;
  /** @brief output @cdd cli(short = 'o', long = "output", env = "CDD_OUTPUT")
   */
  const char *output;
  /** @brief no_imports @cdd cli(long = "no-imports", env = "CDD_NO_IMPORTS") */
  c_cdd_bool_t no_imports;
  /** @brief no_wrapping @cdd cli(long = "no-wrapping", env = "CDD_NO_WRAPPING")
   */
  c_cdd_bool_t no_wrapping;
} cdd_docs_json_config_t;

/**
 * @brief Configuration for serve_json_rpc command.
 * @cdd cli(name = "serve_json_rpc", about = "Run JSON-RPC server")
 */
typedef struct cdd_serve_json_rpc_config {
  /** @brief Port number to bind (default: 8080) @cdd cli(short = 'p', long =
   * "port", default = "8080") */
  int port;
  /** @brief Optional bind address (e.g. "127.0.0.1" or NULL for INADDR_ANY)
   * @cdd cli(long = "bind") */
  const char *bind_address;
  /** @brief Maximum number of requests to serve before exiting (0 = none) @cdd
   * cli(long = "max-requests") */
  size_t max_requests;
} cdd_serve_json_rpc_config_t;

/**
 * @brief Configuration for generating SWIG-like bindings from C headers/source.
 * @cdd cli(name = "generate_bindings", about = "Generate FFI bindings")
 */
typedef struct cdd_generate_bindings_config {
  /** @brief Path to input C header/source or directory @cdd cli(short = 'i',
   * long = "input") */
  const char *input;
  /** @brief Root directory to write generated bindings @cdd cli(short = 'o',
   * long = "output-dir") */
  const char *output_dir;
  /** @brief Comma-separated list of languages, or "all" @cdd cli(short = 'l',
   * long = "lang") */
  const char *target_langs;
  /** @brief Shared object library name (e.g., "sqlite3") @cdd cli(long =
   * "library") */
  const char *library_name;
  /** @brief Name of the generated namespace/module @cdd cli(short = 'm', long =
   * "module") */
  const char *module_name;
  /** @brief Skip static inline functions @cdd cli(long = "skip-static") */
  c_cdd_bool_t skip_static;
  /** @brief Treat unknown structs as void* (opaque) @cdd cli(long =
   * "opaque-pointers") */
  c_cdd_bool_t opaque_pointers;
  /** @brief Generate a basic sanity-check test file in the target lang @cdd
   * cli(long = "tests") */
  c_cdd_bool_t generate_tests;
  /** @brief Recursively parse and merge \#include files into FFI IR @cdd
   * cli(short = 'r', long = "recursive") */
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
 * @cdd cli(name = "from_arazzo", about = "Generate C code from Arazzo")
 */
typedef struct cdd_from_arazzo_config {
  /** @brief Target subcommand @cdd cli(positional, index = 0) */
  enum cdd_arazzo_subcommand_t subcommand;
  /** @brief Path to input Arazzo v1.1.0 JSON/YAML file @cdd cli(short = 'i',
   * long = "input") */
  const char *input;
  /** @brief Optional path to linked OpenAPI JSON/YAML file @cdd cli(long =
   * "openapi") */
  const char *openapi_input;
  /** @brief Output directory or base path for generated C artifacts @cdd
   * cli(short = 'o', long = "output") */
  const char *output;
  /** @brief Optional function name prefix (e.g. `"api_"`) @cdd cli(long =
   * "prefix") */
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
