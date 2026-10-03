/**
 * @file openapi.h
 * @brief Parser for OpenAPI v3.2 definitions.
 *
 * Provides functionalities to load an OpenAPI JSON specification into memory
 * structures. Supports:
 * - Paths and Operations
 * - Parameters (serialization styles, explode, allowEmptyValue, content)
 * - Request Bodies and content-types
 * - Response descriptions
 * - Component Schemas (parsed into StructFields for generation lookups)
 * - Security Schemes
 * - Root-level Servers
 * - Tags for resource grouping
 * - Component Path Items and Media Types (OAS 3.2)
 * - Path Item additionalOperations (OAS 3.2)
 * - Content-level $ref for Media Type Objects (OAS 3.2)
 * - Response Links and Callback Objects (OAS 3.2)
 * - components.links and components.callbacks (OAS 3.2)
 * - Example Objects and components.examples (OAS 3.2)
 * - OAuth2 flows in components.securitySchemes (OAS 3.2)
 * - Media Type Encoding by name and position: `encoding`, `prefixEncoding`,
 *   and `itemEncoding` (OAS 3.2)
 * - Parameter/Header content Media Type Objects (encoding/examples/schema)
 * - x- extensions on Paths, Webhooks, and Components objects
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_OPENAPI_LOADER_H
#define C_CDD_OPENAPI_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <parson.h>
#include <stddef.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "classes/emit/struct.h"
#include "openapi/parse/openapi_types.h"
/* clang-format on */

/* --- Lifecycle --- */

/**
 * @brief Initialize a Spec structure to zero.
 * @param[out] spec Pointer to spec structure to initialize.
 */
extern C_CDD_EXPORT cdd_c_error_t openapi_spec_init(struct OpenAPI_Spec *spec);

/**
 * @brief Free a Spec structure and all nested allocations.
 * @param[in] spec Pointer to spec structure to free.
 */
extern C_CDD_EXPORT void openapi_spec_free(struct OpenAPI_Spec *spec);

extern C_CDD_EXPORT void
openapi_free_servers_array(struct OpenAPI_Server *servers, size_t n_servers);

/* --- Loader --- */

/**
 * @brief Initialize a document registry.
 * @param[out] registry Pointer to registry to initialize.
 */
extern C_CDD_EXPORT cdd_c_error_t
openapi_doc_registry_init(struct OpenAPI_DocRegistry *registry);

/**
 * @brief Free a document registry and its URI entries.
 *
 * Does NOT free the OpenAPI_Spec instances referenced by the registry.
 *
 * @param[in] registry Pointer to registry to free.
 */
extern C_CDD_EXPORT void
openapi_doc_registry_free(struct OpenAPI_DocRegistry *registry);

/**
 * @brief Register a parsed document with the registry.
 *
 * Uses the document's resolved base URI for matching $ref targets.
 *
 * @param[in,out] registry Registry to add to.
 * @param[in] spec Parsed OpenAPI specification.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */
extern C_CDD_EXPORT cdd_c_error_t openapi_doc_registry_add(
    struct OpenAPI_DocRegistry *registry, struct OpenAPI_Spec *spec);

/**
 * @brief Parse an OpenAPI or Schema document from a JSON Value.
 *
 * Traverses the JSON to populate the Spec structure. For OpenAPI documents:
 * 1. Extracts Paths, Operations, Params, Bodies, and Tags.
 * 2. Extracts Security Schemes from components.
 * 3. Extracts and flattening Schemas definitions.
 *
 * For Schema documents (JSON Schema at the root), the loader sets
 * `is_schema_document` and stores the serialized root schema in
 * `schema_root_json`.
 *
 * @param[in] root The root JSON Value of the OpenAPI or Schema document.
 * @param[out] out Destination structure to populate.
 * @return 0 on success, error code (EINVAL/ENOMEM) on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
openapi_load_from_json(const JSON_Value *root, struct OpenAPI_Spec *out);

/**
 * @brief Parse a JSON Value with document context for multi-doc resolution.
 *
 * Resolves relative $self and $ref bases using the provided retrieval URI,
 * and optionally consults a registry for external component references.
 *
 * For Schema documents, $id (when present) is used as the base URI for
 * registry resolution.
 *
 * If a registry is provided, this function registers the parsed document on
 * success.
 *
 * @param[in] root The root JSON Value of the OpenAPI file.
 * @param[in] retrieval_uri Retrieval/base URI for this document (optional).
 * @param[out] out The spec structure to populate (must be initialized).
 * @param[in,out] registry Optional registry for multi-doc resolution.
 * @return 0 on success, EINVAL on invalid input, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the openapi load from json with context
                     * operation.
                     */
    cdd_c_error_t
    openapi_load_from_json_with_context(const JSON_Value *root,
                                        const char *retrieval_uri,
                                        struct OpenAPI_Spec *out,
                                        struct OpenAPI_DocRegistry *registry);

/**
 * @param[out] _out_val Pointer to store the result
 * @brief Look up a schema definition by name in the loaded spec.
 *
 * @param[in] spec The spec to search.
 * @param[in] name The schema name (e.g. "LoginRequest").
 * @return Pointer to StructFields if found, NULL otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
openapi_spec_find_schema(const struct OpenAPI_Spec *spec, const char *name,
                         struct StructFields **_out_val);

/**
 * @param[out] _out_val Pointer to store the result
 * @brief Find a schema definition by following a SchemaRef.
 *
 * Uses the document registry when present to resolve external component refs.
 *
 * @param[in] spec The current specification.
 * @param[in] ref The schema reference.
 * @return Pointer to StructFields if found, NULL otherwise.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the openapi spec find schema for ref
                     * operation.
                     */
    cdd_c_error_t
    openapi_spec_find_schema_for_ref(const struct OpenAPI_Spec *spec,
                                     const struct OpenAPI_SchemaRef *ref,
                                     struct StructFields **_out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_LOADER_H */
