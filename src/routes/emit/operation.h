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
#include "cdd_c_error.h"
#include "classes/parse/inspector.h"
#include "classes/parse/mapping.h"
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
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_build_operation(
    const struct OpBuilderContext *ctx, struct OpenAPI_Operation *out_op);

/**
 * @brief Parses an example string into an OpenAPI Any structure.
 *
 * @param[in] example The string containing the example value or JSON.
 * @param[out] out Pointer to OpenAPI_Any destination structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_example_any(const char *example,
                                                    struct OpenAPI_Any *out);

/**
 * @brief Checks if a header name is an HTTP reserved header name.
 *
 * @param[in] name The header name to inspect.
 * @param[out] out_is_reserved Pointer to int receiving 1 if reserved, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t is_reserved_header_name(const char *name,
                                                          int *out_is_reserved);

/**
 * @brief Converts a JSON_Value into an OpenAPI Any structure.
 *
 * @param[in] val Pointer to JSON_Value to convert.
 * @param[out] out Pointer to destination OpenAPI_Any structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t any_from_json_value(const JSON_Value *val,
                                                      struct OpenAPI_Any *out);

/**
 * @brief Parses a JSON string representation of link parameters.
 *
 * @param[in] json JSON string encoding key-value link parameters.
 * @param[out] out Pointer to array pointer receiving allocated link parameters.
 * @param[out] out_count Pointer to size_t receiving number of link parameters.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_link_params_json(
    const char *json, struct OpenAPI_LinkParam **out, size_t *out_count);

/**
 * @brief Copies an OpenAPI Any value structure.
 *
 * @param[out] dst Destination OpenAPI_Any.
 * @param[in] src Source OpenAPI_Any.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
copy_any_value_local(struct OpenAPI_Any *dst, const struct OpenAPI_Any *src);

/**
 * @brief Frees members of an OpenAPI Any value structure.
 *
 * @param[in,out] val Pointer to OpenAPI_Any whose contents should be freed.
 */
extern C_CDD_EXPORT void free_any_value_local(struct OpenAPI_Any *val);

/**
 * @brief Finds a documented parameter by name.
 *
 * @param[in] doc Pointer to DocMetadata.
 * @param[in] name Parameter name.
 * @param[out] _out_val Pointer receiving pointer to found DocParam or NULL.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t find_doc_param(const struct DocMetadata *doc,
                                                 const char *name,
                                                 struct DocParam **_out_val);

/**
 * @brief Frees server variable members within an OpenAPI Server structure.
 *
 * @param[in,out] srv Pointer to OpenAPI_Server whose variables should be freed.
 */
extern C_CDD_EXPORT void
free_openapi_server_variables_op(struct OpenAPI_Server *srv);

/**
 * @brief Copies server variable definitions from DocServer to OpenAPI_Server.
 *
 * @param[out] dst Destination OpenAPI_Server.
 * @param[in] src Source DocServer containing variables.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_doc_server_variables_op(
    struct OpenAPI_Server *dst, const struct DocServer *src);

/**
 * @brief Finds an existing OpenAPI Response by its HTTP status code.
 *
 * @param[in] op Pointer to the OpenAPI Operation.
 * @param[in] code HTTP status code string (e.g. "200").
 * @param[out] _out_val Pointer receiving pointer to found response or NULL.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_response_by_code(struct OpenAPI_Operation *op, const char *code,
                      struct OpenAPI_Response **_out_val);

/**
 * @brief Finds a media type within an array of media types by name.
 *
 * @param[in] mts Array of media types.
 * @param[in] n Number of elements in mts array.
 * @param[in] name Name of the media type (e.g. "application/json").
 * @param[out] _out_val Pointer receiving pointer to found media type or NULL.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_media_type_op(struct OpenAPI_MediaType *mts, size_t n, const char *name,
                   struct OpenAPI_MediaType **_out_val);

/**
 * @brief Applies an example value string to a media type structure.
 *
 * @param[in,out] mt Pointer to media type.
 * @param[in] example Example value string.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
apply_example_to_media_type(struct OpenAPI_MediaType *mt, const char *example);

/**
 * @brief Applies an example value string to an OpenAPI Response.
 *
 * @param[in,out] resp Pointer to response structure.
 * @param[in] example Example value string.
 * @param[in] content_type Optional media type filter, or NULL for all media
 * types.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
apply_example_to_response(struct OpenAPI_Response *resp, const char *example,
                          const char *content_type);

/**
 * @brief Ensures an OpenAPI Response exists for the specified code, creating if
 * needed.
 *
 * @param[in,out] op Pointer to OpenAPI Operation.
 * @param[in] code HTTP status code string.
 * @param[out] _out_val Pointer receiving created or existing OpenAPI_Response
 * pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
ensure_response_for_code(struct OpenAPI_Operation *op, const char *code,
                         struct OpenAPI_Response **_out_val);

/**
 * @brief Adds or merges a header into an OpenAPI Response.
 *
 * @param[in,out] resp Pointer to OpenAPI Response.
 * @param[in] dh Pointer to documented response header metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t add_header_to_response(
    struct OpenAPI_Response *resp, const struct DocResponseHeader *dh);

/**
 * @brief Adds a link to an OpenAPI Response.
 *
 * @param[in,out] resp Pointer to OpenAPI Response.
 * @param[in] dl Pointer to documented link metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
add_link_to_response(struct OpenAPI_Response *resp, const struct DocLink *dl);

/**
 * @brief Adds a parameter to an OpenAPI Operation.
 *
 * @param[in,out] op Pointer to OpenAPI Operation.
 * @param[in] p Pointer to parameter to append.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t add_param_to_op(struct OpenAPI_Operation *op,
                                                  struct OpenAPI_Parameter *p);

/**
 * @brief Checks if a basic schema reference contains data.
 *
 * @param[in] ref Pointer to schema reference to inspect.
 * @param[out] out_has_data Pointer to int receiving 1 if ref has data, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t schema_ref_has_data_basic(
    const struct OpenAPI_SchemaRef *ref, int *out_has_data);

/**
 * @brief Copies basic schema reference fields from src to dst.
 *
 * @param[out] dst Destination OpenAPI_SchemaRef structure.
 * @param[in] src Source OpenAPI_SchemaRef structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_schema_ref_basic(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *src);

/**
 * @brief Checks if an OpenAPI Response contains a media type with the specified
 * name.
 *
 * @param[in] resp Pointer to OpenAPI Response.
 * @param[in] name Media type name to check.
 * @param[out] out_has Pointer receiving 1 if present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t response_has_media_type(
    const struct OpenAPI_Response *resp, const char *name, int *out_has);

/**
 * @brief Checks if a C type represents a struct pointer or double pointer.
 *
 * @param[in] type C type string.
 * @param[out] is_double_ptr Pointer receiving 1 if double pointer, 0 otherwise.
 * @param[out] out_is_struct_ptr Pointer receiving 1 if struct pointer, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t is_struct_pointer(const char *type,
                                                    int *is_double_ptr,
                                                    int *out_is_struct_ptr);

/**
 * @brief Maps documentation parameter style enum to OpenAPI Style enum.
 *
 * @param[in] style DocParamStyle value.
 * @param[out] _out_val Pointer receiving mapped OpenAPI_Style value.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
doc_style_to_openapi(enum DocParamStyle style, enum OpenAPI_Style *_out_val);

/**
 * @brief Checks if a parameter name corresponds to a path parameter in route.
 *
 * @param[in] route Route string template.
 * @param[in] name Parameter name.
 * @param[out] out_is_path Pointer to int receiving 1 if path param, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t is_path_param(const char *route,
                                                const char *name,
                                                int *out_is_path);

/**
 * @brief Checks if an OpenAPI type name is a primitive type.
 *
 * @param[in] type Type name string.
 * @param[out] out_is_primitive Pointer to int receiving 1 if primitive, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t oa_type_is_primitive(const char *type,
                                                       int *out_is_primitive);

/**
 * @brief Applies format from type mapping (or override) to a SchemaRef.
 *
 * @param[in,out] schema Pointer to OpenAPI SchemaRef to update.
 * @param[in] map Type mapping containing default format and type.
 * @param[in] override_format Explicit format override if specified, or NULL.
 * @param[out] out_applied Pointer to int receiving 1 if format was applied, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure.
 */
extern C_CDD_EXPORT cdd_c_error_t apply_format_to_schema_ref(
    struct OpenAPI_SchemaRef *schema, const struct OpenApiTypeMapping *map,
    const char *override_format, int *out_applied);

/**
 * @brief Sets querystring schema on a parameter from a type mapping.
 *
 * @param[in,out] param Pointer to OpenAPI Parameter to configure.
 * @param[in] type_map Pointer to OpenAPI Type Mapping.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t set_querystring_schema_from_type_map(
    struct OpenAPI_Parameter *param, const struct OpenApiTypeMapping *type_map);

/**
 * @brief Initializes a media type structure from an OpenAPI Response.
 *
 * @param[out] mt Pointer to OpenAPI MediaType to initialize.
 * @param[in] name Name of the media type.
 * @param[in] resp Pointer to source OpenAPI Response.
 * @param[in] is_item_schema 1 if mapping to item_schema, 0 for schema.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t init_media_type_from_response(
    struct OpenAPI_MediaType *mt, const char *name,
    const struct OpenAPI_Response *resp, int is_item_schema);

/**
 * @brief Adds or ensures a media type exists in an OpenAPI Response.
 *
 * @param[in,out] resp Pointer to OpenAPI Response.
 * @param[in] name Media type name.
 * @param[in] is_item_schema 1 if mapping to item_schema, 0 for schema.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t add_response_media_type(
    struct OpenAPI_Response *resp, const char *name, int is_item_schema);

/**
 * @brief Checks if request body already has a specific media type.
 *
 * @param[in] op Pointer to OpenAPI Operation.
 * @param[in] name Media type name.
 * @param[out] out_has Pointer to int receiving 1 if present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t request_body_has_media_type(
    const struct OpenAPI_Operation *op, const char *name, int *out_has);

/**
 * @brief Initializes a media type structure from an OpenAPI Operation request
 * body.
 *
 * @param[out] mt Pointer to OpenAPI MediaType to initialize.
 * @param[in] name Name of the media type.
 * @param[in] op Pointer to source OpenAPI Operation.
 * @param[in] is_item_schema 1 if mapping to item_schema, 0 for schema.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t init_media_type_from_request_body(
    struct OpenAPI_MediaType *mt, const char *name,
    const struct OpenAPI_Operation *op, int is_item_schema);

/**
 * @brief Adds or ensures a media type exists in an OpenAPI Operation request
 * body.
 *
 * @param[in,out] op Pointer to OpenAPI Operation.
 * @param[in] name Media type name.
 * @param[in] is_item_schema 1 if mapping to item_schema, 0 for schema.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t add_request_body_media_type(
    struct OpenAPI_Operation *op, const char *name, int is_item_schema);

/**
 * @brief Cleans up dynamically allocated fields of an OpenAPI Link.
 *
 * @param[in,out] link Pointer to OpenAPI Link structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
cleanup_link_fields(struct OpenAPI_Link *link);

/**
 * @brief Frees dynamically allocated fields of an OpenAPI Parameter.
 *
 * @param[in,out] p Pointer to OpenAPI Parameter structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
free_param_fields(struct OpenAPI_Parameter *p);

/**
 * @brief Frees dynamically allocated fields of an OpenAPI Encoding.
 *
 * @param[in,out] enc Pointer to OpenAPI Encoding structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
free_encoding_fields(struct OpenAPI_Encoding *enc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C2OPENAPI_OPERATION_H */
