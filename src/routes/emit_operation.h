/**
 * @file c2openapi_operation.h
 * @brief Builder for constructing OpenAPI Operations from C functions.
 *
 * Merges extracted C signature details (arguments, return type) with
 * documentation metadata (@route, @param) to produce a semantic OpenAPI
 * Operation definition.
 *
 * Implements heuristics to distinguish:
 * - Path Parameters (matched by name in route)
 * - Query Parameters (default scalar inputs)
 * - Request Bodies (non-const structs)
 * - Response Bodies (output pointers)
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_C2OPENAPI_OPERATION_H
#define C_CDD_C2OPENAPI_OPERATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

#include "c_cdd_export.h"
#include "classes/parse_inspector.h"
#include "docstrings/parse_doc.h"
#include "openapi/parse_openapi.h"

/**
 * @brief Represents a single argument parsed from a signature string.
 */
struct C2OpenAPI_ParsedArg {
  char *name; /**< Argument name (e.g. "x") */
  char *type; /**< Argument type (e.g. "int", "struct User *") */
};

/**
 * @brief Represents a fully parsed header/signature.
 * Used for semantic mapping, distinct from the CST-bases C_Inspector signature.
 */
struct C2OpenAPI_ParsedSig {
  char *name;                       /**< Function name */
  struct C2OpenAPI_ParsedArg *args; /**< Array of arguments */
  size_t n_args;                    /**< Arg count */
  char *return_type;                /**< Return type string */
};

/**
 * @brief Context for the build process.
 * Contains the source data (sig, docs) and configuration options.
 */
struct OpBuilderContext {
  const struct C2OpenAPI_ParsedSig *sig; /**< The parsed C function signature */
  const struct DocMetadata *doc; /**< Extracted documentation annotations */
  const char *func_name;         /**< Original function name */
};

/**
 * @brief Build an OpenAPI Operation from C source artifacts.
 *
 * The core logic flow:
 * 1. Initialize `out_op` with basic metadata (Summary, ID).
 * 2. Analyze the Route path to identify Path Parameters.
 * 3. Iterate signature arguments:
 *    - Match against `@param` docs for explicit overrides (`in: query`,
 * `required`).
 *    - Match against Path Parameters (implicit `in: path`).
 *    - Distinguish Body candidates (Struct pointers) from Inputs.
 *    - Identify Output parameters (pointer-to-pointer or non-const pointer
 * refs).
 * 4. Configure Responses based on `@return` docs and detected outputs.
 *
 * @param[in] ctx Context containing the signature and docs.
 * @param[out] out_op The operation structure to populate.
 * @return 0 on success, ENOMEM/EINVAL on failure.
 */
extern C_CDD_EXPORT int
c2openapi_build_operation(const struct OpBuilderContext *ctx,
                          struct OpenAPI_Operation *out_op);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C2OPENAPI_OPERATION_H */
