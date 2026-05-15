/**
 * @file operation.h
 * @brief Builder for constructing OpenAPI Operations from C functions.
 *
 * Merges extracted C signature details (arguments, return type) with
 * documentation metadata (\@route, @param) to produce a semantic OpenAPI
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

/* clang-format off */
#include <stddef.h>

#include "c_cdd_export.h"
#include "classes/parse/inspector.h"
#include "docstrings/parse/doc.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

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

extern C_CDD_EXPORT int parse_example_any(const char *example,
                                          struct OpenAPI_Any *out);
extern C_CDD_EXPORT int is_reserved_header_name(const char *name);
extern C_CDD_EXPORT int any_from_json_value(const JSON_Value *val,
                                            struct OpenAPI_Any *out);
extern C_CDD_EXPORT int parse_link_params_json(const char *json,
                                               struct OpenAPI_LinkParam **out,
                                               size_t *out_count);
extern C_CDD_EXPORT void
free_openapi_server_variables_op(struct OpenAPI_Server *srv);
extern C_CDD_EXPORT int
copy_doc_server_variables_op(struct OpenAPI_Server *dst,
                             const struct DocServer *src);
extern C_CDD_EXPORT int
find_response_by_code(struct OpenAPI_Operation *op, const char *code,
                      struct OpenAPI_Response **_out_val);
extern C_CDD_EXPORT int find_media_type_op(struct OpenAPI_MediaType *mts,
                                           size_t n, const char *name,
                                           struct OpenAPI_MediaType **_out_val);
extern C_CDD_EXPORT int
apply_example_to_media_type(struct OpenAPI_MediaType *mt, const char *example);
extern C_CDD_EXPORT int apply_example_to_response(struct OpenAPI_Response *resp,
                                                  const char *example,
                                                  const char *content_type);

extern C_CDD_EXPORT int
ensure_response_for_code(struct OpenAPI_Operation *op, const char *code,
                         struct OpenAPI_Response **_out_val);
extern C_CDD_EXPORT int
add_header_to_response(struct OpenAPI_Response *resp,
                       const struct DocResponseHeader *dh);
extern C_CDD_EXPORT int add_link_to_response(struct OpenAPI_Response *resp,
                                             const struct DocLink *dl);
extern C_CDD_EXPORT int add_param_to_op(struct OpenAPI_Operation *op,
                                        struct OpenAPI_Parameter *p);
extern C_CDD_EXPORT int
schema_ref_has_data_basic(const struct OpenAPI_SchemaRef *ref);

extern C_CDD_EXPORT int
copy_schema_ref_basic(struct OpenAPI_SchemaRef *dst,
                      const struct OpenAPI_SchemaRef *src);
extern C_CDD_EXPORT int
response_has_media_type(const struct OpenAPI_Response *resp, const char *name);
extern C_CDD_EXPORT int is_struct_pointer(const char *type, int *is_double_ptr);
extern C_CDD_EXPORT int doc_style_to_openapi(enum DocParamStyle style,
                                             enum OpenAPI_Style *_out_val);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C2OPENAPI_OPERATION_H */
