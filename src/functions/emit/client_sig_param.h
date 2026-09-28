/**
 * @file client_sig_param.h
 * @brief Parameter and query string inspection for client signature
 * generation.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_CLIENT_SIG_PARAM_H
#define C_CDD_CODEGEN_CLIENT_SIG_PARAM_H

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
 * @brief Checks if a parameter represents an object Key-Value structure.
 *
 * @param[in] p Parameter definition.
 * @param[out] out Pointer to receive 1 if object KV, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
param_is_object_kv(const struct OpenAPI_Parameter *p, int *out);

/**
 * @brief Checks if a querystring parameter is a form object for client
 * signatures.
 *
 * @param[in] p Parameter definition.
 * @param[out] out Pointer to receive 1 if form object, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t sig_querystring_param_is_form_object(
    const struct OpenAPI_Parameter *p, int *out);

/**
 * @brief Checks if a querystring parameter is a JSON schema reference for
 * client signatures.
 *
 * @param[in] p Parameter definition.
 * @param[out] out Pointer to receive 1 if JSON ref, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
sig_querystring_param_is_json_ref(const struct OpenAPI_Parameter *p, int *out);

/**
 * @brief Determines the JSON primitive type of a querystring parameter for
 * client signatures.
 *
 * @param[in] p Parameter definition.
 * @param[out] _out_val Pointer to receive primitive type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t sig_querystring_param_json_primitive_type(
    const struct OpenAPI_Parameter *p, const char **_out_val);

/**
 * @brief Determines the JSON array item type of a querystring parameter for
 * client signatures.
 *
 * @param[in] p Parameter definition.
 * @param[out] _out_val Pointer to receive item type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t sig_querystring_param_json_array_item_type(
    const struct OpenAPI_Parameter *p, const char **_out_val);

/**
 * @brief Determines the JSON array item reference of a querystring parameter
 * for client signatures.
 *
 * @param[in] p Parameter definition.
 * @param[out] _out_val Pointer to receive item reference string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t sig_querystring_param_json_array_item_ref(
    const struct OpenAPI_Parameter *p, const char **_out_val);

/**
 * @brief Determines raw primitive type for non-JSON/form querystring parameters
 * for client signatures.
 *
 * @param[in] p Parameter definition.
 * @param[out] _out_val Pointer to receive primitive type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t sig_querystring_param_raw_primitive_type(
    const struct OpenAPI_Parameter *p, const char **_out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_CODEGEN_CLIENT_SIG_PARAM_H */
