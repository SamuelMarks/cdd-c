/**
 * @file openapi_writer.h
 * @brief Writer module for OpenAPI v3.2 definitions.
 *
 * Provides functionality to serialize an in-memory `OpenAPI_Spec` structure
 * into a JSON string. This acts as the inverse of `openapi_loader`.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_OPENAPI_WRITER_H
#define C_CDD_OPENAPI_WRITER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "openapi_loader.h"

/**
 * @brief Serialize an OpenAPI Spec structure to a JSON string.
 *
 * Converts the full specification tree (Paths, Operations, Parameters,
 * Responses, and Components) into a formatted JSON string.
 *
 * The caller is responsible for freeing the output string.
 *
 * @param[in] spec The specification structure to serialize.
 * @param[out] json_out Pointer to a char* where the allocated JSON string will
 * be stored.
 * @return 0 on success, EINVAL if inputs are invalid, ENOMEM on allocation
 * failure.
 */
extern C_CDD_EXPORT int
openapi_write_spec_to_json(const struct OpenAPI_Spec *spec, char **json_out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_WRITER_H */
