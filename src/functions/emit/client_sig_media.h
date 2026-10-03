/**
 * @file client_sig_media.h
 * @brief Media type detection and categorization for client signature
 * generation.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_CLIENT_SIG_MEDIA_H
#define C_CDD_CODEGEN_CLIENT_SIG_MEDIA_H

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
 * @brief Computes length of media type up to optional semicolon parameter.
 *
 * @param[in] media_type Media type string.
 * @param[out] _out_val Pointer to receive base length.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_base_len(const char *media_type,
                                                      size_t *_out_val);

/**
 * @brief Checks if media type starts with a specified prefix.
 *
 * @param[in] media_type Media type string.
 * @param[in] prefix Prefix string to test.
 * @param[out] out Pointer to receive 1 if prefix matches, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_has_prefix(const char *media_type,
                                                        const char *prefix,
                                                        int *out);

/**
 * @brief Checks if base media type ends with a specified suffix.
 *
 * @param[in] media_type Media type string.
 * @param[in] suffix Suffix string to test.
 * @param[out] out Pointer to receive 1 if suffix matches, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_has_suffix(const char *media_type,
                                                        const char *suffix,
                                                        int *out);

/**
 * @brief Compares media type base portion with an expected string.
 *
 * @param[in] media_type Media type string.
 * @param[in] expected Expected media type string.
 * @param[out] out Pointer to receive 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_ieq(const char *media_type,
                                                 const char *expected,
                                                 int *out);

/**
 * @brief Checks if a media type represents JSON.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if JSON, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_is_json(const char *media_type,
                                                     int *out);

/**
 * @brief Checks if a media type represents URL-encoded form data.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if form data, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_is_form(const char *media_type,
                                                     int *out);

/**
 * @brief Checks if a media type is text/plain.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if text/plain, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
media_type_is_text_plain(const char *media_type, int *out);

/**
 * @brief Checks if a media type is multipart.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if multipart, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
media_type_is_multipart(const char *media_type, int *out);

/**
 * @brief Checks if a media type is multipart/form-data.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if multipart/form-data, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
media_type_is_multipart_form(const char *media_type, int *out);

/**
 * @brief Finds a media type definition by name in an array.
 *
 * @param[in] mts Array of OpenAPI_MediaType structures.
 * @param[in] n Array size.
 * @param[in] name Media type name to find.
 * @param[out] _out_val Pointer to receive matched media type pointer.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_media_type(const struct OpenAPI_MediaType *mts, size_t n, const char *name,
                const struct OpenAPI_MediaType **_out_val);

/**
 * @brief Checks if a media type represents textual content.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if textual, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_is_textual(const char *media_type,
                                                        int *out);

/**
 * @brief Checks if a media type represents binary content.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if binary, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_is_binary(const char *media_type,
                                                       int *out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_CODEGEN_CLIENT_SIG_MEDIA_H */
