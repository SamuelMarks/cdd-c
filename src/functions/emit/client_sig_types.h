/**
 * @file client_sig_types.h
 * @brief Type mapping and identifier sanitization for client signature
 * generation.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_CLIENT_SIG_TYPES_H
#define C_CDD_CODEGEN_CLIENT_SIG_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <stddef.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

/**
 * @brief Maps an OpenAPI type string to a C function argument type.
 *
 * @param[in] oa_type OpenAPI type name.
 * @param[out] _out_val Pointer to receive the C argument type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t map_type_to_c_arg(const char *oa_type,
                                                    const char **_out_val);

/**
 * @brief Checks if an OpenAPI type is a primitive type.
 *
 * @param[in] oa_type OpenAPI type name.
 * @param[out] out Pointer to receive 1 if primitive, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t is_primitive_type(const char *oa_type,
                                                    int *out);

/**
 * @brief Maps an OpenAPI array item type to a C pointer type.
 *
 * @param[in] oa_type OpenAPI array item type.
 * @param[out] _out_val Pointer to receive C pointer type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t map_array_item_type(const char *oa_type,
                                                      const char **_out_val);

/**
 * @brief Sanitizes an identifier to be a valid C identifier.
 *
 * @param[out] out Output buffer.
 * @param[in] outsz Size of output buffer.
 * @param[in] in Input string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL/empty.
 */
extern C_CDD_EXPORT cdd_c_error_t sanitize_ident(char *out, size_t outsz,
                                                 const char *in);

/**
 * @brief Generates a sanitized multipart header parameter name.
 *
 * @param[out] out Destination buffer.
 * @param[in] outsz Destination buffer size.
 * @param[in] field Multipart field name.
 * @param[in] header Header name.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t multipart_header_param_name(
    char *out, size_t outsz, const char *field, const char *header);

/**
 * @brief Checks if a header name is Content-Type (case-insensitive).
 *
 * @param[in] name Header name string.
 * @param[out] out Pointer to receive 1 if Content-Type, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t header_name_is_content_type(const char *name,
                                                              int *out);

/**
 * @brief Maps an OpenAPI type to a C output pointer parameter type.
 *
 * @param[in] oa_type OpenAPI type name.
 * @param[out] _out_val Pointer to receive C type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t map_type_to_c_out(const char *oa_type,
                                                    const char **_out_val);

/**
 * @brief Maps an OpenAPI array item type to a C array output pointer parameter
 * type.
 *
 * @param[in] oa_type OpenAPI array item type name.
 * @param[out] _out_val Pointer to receive C type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
map_array_item_type_out(const char *oa_type, const char **_out_val);

/**
 * @brief Checks if a schema reference contains an inline type definition.
 *
 * @param[in] schema Schema reference structure.
 * @param[out] out Pointer to receive 1 if inline type exists, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
schema_has_inline(const struct OpenAPI_SchemaRef *schema, int *out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_CODEGEN_CLIENT_SIG_TYPES_H */
