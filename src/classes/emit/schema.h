/**
 * @file c2openapi_schema.h
 * @brief Schema Registry Integration.
 *
 * Implements logic to register C types detected in source code into the
 * central OpenAPI Components. It handles:
 * - Conversion of `c_inspector` types to `StructFields`.
 * - Deduplication (avoiding overwriting existing schemas).
 * - Recursive dependency resolution (e.g. if A uses B, register B).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_C2OPENAPI_SCHEMA_H
#define C_CDD_C2OPENAPI_SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "classes/parse/inspector.h"
#include "openapi/parse/openapi.h"

/**
 * @brief Register a list of types discovered in a file into the Spec.
 *
 * Iterates over the definitions in `types`. For each `struct`:
 * - Checks if it's already in `spec->components.schemas`.
 * - If not, adds it.
 *
 * Note: Enums are currently registered if `c_inspector` supports them deeply,
 * but this focuses primarily on Structs for Schema Objects.
 *
 * @param[in,out] spec The OpenAPI specification to update.
 * @param[in] types The list of types found in a file by `c_inspector`.
 * @return 0 on success, ENOMEM/EINVAL on failure.
 */
extern C_CDD_EXPORT int
c2openapi_register_types(struct OpenAPI_Spec *spec,
                         const struct TypeDefList *types);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C2OPENAPI_SCHEMA_H */
