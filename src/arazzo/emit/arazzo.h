/**
 * @file arazzo.h
 * @brief Emitter for Arazzo Specification v1.1.0 documents.
 *
 * Serializes an in-memory `Arazzo_Spec` structure into canonical Arazzo v1.1.0
 * JSON (`JSON_Value *` or formatted string).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ARAZZO_EMIT_H
/** @brief Header guard for arazzo/emit/arazzo.h */
#define C_CDD_ARAZZO_EMIT_H

/* clang-format off */
#include "arazzo/parse/arazzo_types.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <parson.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Serializes an `Arazzo_Spec` structure into a newly allocated
 * `JSON_Value` tree.
 *
 * @param[in] spec Source `Arazzo_Spec` structure.
 * @param[out] out_root Output pointer receiving newly allocated `JSON_Value *`
 * object.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * invalid/NULL, or `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_write_spec_to_json_value(
    const struct Arazzo_Spec *spec, JSON_Value **out_root);

/**
 * @brief Serializes an `Arazzo_Spec` structure into a formatted JSON string.
 *
 * @param[in] spec Source `Arazzo_Spec` structure.
 * @param[out] out_json Output pointer receiving newly allocated JSON string.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * invalid/NULL, or `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_write_spec_to_json(const struct Arazzo_Spec *spec, char **out_json);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ARAZZO_EMIT_H */
