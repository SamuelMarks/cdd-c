/**
 * @file arazzo.h
 * @brief Parser and semantic validator for Arazzo Specification v1.1.0.
 *
 * Provides functions to parse Arazzo v1.1.0 documents from JSON/YAML strings,
 * `JSON_Value` trees, or filesystem paths, resolve `$components.*` reusable
 * references, and validate structural and DAG constraints.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ARAZZO_PARSE_H
/** @brief Header guard for arazzo.h */
#define C_CDD_ARAZZO_PARSE_H

/* clang-format off */
#include "arazzo/parse/arazzo_types.h"
#include "c_cdd/reflect.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "c_cdd/json_dom.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Parses an Arazzo Specification v1.1.0 document from a `JSON_Value`
 * root.
 *
 * Populates `out_spec` with `arazzo`, `$self`, `info`, `sourceDescriptions`,
 * `workflows`, `components`, and `^x-` extensions, and runs semantic validation
 * and `$components.*` dereferencing.
 *
 * @param[in] root Root `JSON_Value` object of the Arazzo document.
 * @param[out] out_spec Destination `Arazzo_Spec` structure.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * malformed/invalid, or `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_load_from_json_value(
    const JSON_Value *root, struct Arazzo_Spec *out_spec);

/**
 * @brief Parses an Arazzo Specification v1.1.0 document from a JSON string.
 *
 * @param[in] json_str Null-terminated JSON string.
 * @param[out] out_spec Destination `Arazzo_Spec` structure.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on parse
 * or validation failure, or `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_load_from_json(const char *json_str,
                                                 struct Arazzo_Spec *out_spec);

/**
 * @brief Validates an in-memory `Arazzo_Spec` structure against Arazzo v1.1.0
 * rules.
 *
 * Verifies required root fields (`arazzo` version `1.0.x`/`1.1.x`,
 * `info.title`, `info.version`, non-empty `sourceDescriptions` and
 * `workflows`), uniqueness of `workflowId` and per-workflow `stepId`, mutual
 * exclusivity of Step targets
 * (`operationId`, `operationPath`, `channelPath`, `workflowId`), valid
 * `dependsOn` references, and valid `$components.*` `Reusable Object`
 * references.
 *
 * @param[in] spec Pointer to the `Arazzo_Spec` to validate.
 * @return `CDD_C_SUCCESS` if valid, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * invalid.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_validate_spec(const struct Arazzo_Spec *spec);

/**
 * @brief Deserializes an `Arazzo_Info` structure from a `JSON_Object`.
 *
 * @param[in] info_obj Source `info` JSON object.
 * @param[out] out_info Destination `Arazzo_Info` structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t Arazzo_Info_from_jsonObject(
    const JSON_Object *info_obj, struct Arazzo_Info *out_info);

/**
 * @brief Deserializes an `Arazzo_SourceDescription` structure from a
 * `JSON_Object`.
 *
 * @param[in] sd_obj Source description JSON object.
 * @param[out] out_sd Destination `Arazzo_SourceDescription` structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t Arazzo_SourceDescription_from_jsonObject(
    const JSON_Object *sd_obj, struct Arazzo_SourceDescription *out_sd);

/**
 * @brief Deserializes an `Arazzo_Criterion` structure from a `JSON_Object`.
 *
 * @param[in] cobj Source criterion JSON object.
 * @param[out] out_crit Destination `Arazzo_Criterion` structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t Arazzo_Criterion_from_jsonObject(
    const JSON_Object *cobj, struct Arazzo_Criterion *out_crit);

/** @brief Runtime reflection descriptor for `Arazzo_Info`. */
extern C_CDD_EXPORT const struct cdd_type_info Arazzo_Info_type_info;
/** @brief Runtime reflection descriptor for `Arazzo_SourceDescription`. */
extern C_CDD_EXPORT const struct cdd_type_info
    Arazzo_SourceDescription_type_info;
/** @brief Runtime reflection descriptor for `Arazzo_Criterion`. */
extern C_CDD_EXPORT const struct cdd_type_info Arazzo_Criterion_type_info;
/** @brief Runtime reflection descriptor for `Arazzo_Selector`. */
extern C_CDD_EXPORT const struct cdd_type_info Arazzo_Selector_type_info;
/** @brief Runtime reflection descriptor for `Arazzo_Reusable`. */
extern C_CDD_EXPORT const struct cdd_type_info Arazzo_Reusable_type_info;
/** @brief Runtime reflection descriptor for `Arazzo_SuccessAction`. */
extern C_CDD_EXPORT const struct cdd_type_info Arazzo_SuccessAction_type_info;
/** @brief Runtime reflection descriptor for `Arazzo_FailureAction`. */
extern C_CDD_EXPORT const struct cdd_type_info Arazzo_FailureAction_type_info;
/** @brief Runtime reflection descriptor for `Arazzo_Step`. */
extern C_CDD_EXPORT const struct cdd_type_info Arazzo_Step_type_info;
/** @brief Runtime reflection descriptor for `Arazzo_Workflow`. */
extern C_CDD_EXPORT const struct cdd_type_info Arazzo_Workflow_type_info;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ARAZZO_PARSE_H */
