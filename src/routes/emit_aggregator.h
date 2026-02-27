/**
 * @file openapi_aggregator.h
 * @brief Aggregator for organizing OpenAPI Operations into Paths.
 *
 * Provides functionality to insert individual Operation definition structs
 * into the main OpenAPI Specification structure. It manages:
 * - Deduplication of path strings.
 * - Merging multiple verbs (GET, POST) under the same route key.
 * - Dynamic resizing of the internal path lists.
 *
 * This acts as the reducer step in the "map-reduce" architecture of the
 * C-to-OpenAPI pipeline.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_OPENAPI_AGGREGATOR_H
#define C_CDD_OPENAPI_AGGREGATOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "openapi/parse_openapi.h"

/**
 * @brief Add an operation to the OpenAPI Spec, organizing by Path.
 *
 * Locates an existing Path Item in `spec` that matches `route`.
 * If found, adds the operation to that Path's list.
 * If not found, creates a new Path Item.
 *
 * It copies the operation data (shallow assignment where ownership transfers
 * to the spec container, or deep copy if implemented defensively).
 * The current implementation assumes OWNERSHIP TRANSFER: The inputs inside
 * `op` are now owned by `spec`, so the caller should NOT free `op` contents
 * afterward if this returns success.
 *
 * @param[in,out] spec The main specification object.
 * @param[in] route The route string (e.g. "/users").
 * @param[in] op The operation to add.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on bad args.
 */
extern C_CDD_EXPORT int
openapi_aggregator_add_operation(struct OpenAPI_Spec *spec, const char *route,
                                 struct OpenAPI_Operation *op);

/**
 * @brief Add a webhook operation to the OpenAPI Spec, organizing by Path.
 *
 * Mirrors openapi_aggregator_add_operation but targets spec->webhooks.
 *
 * @param[in,out] spec The main specification object.
 * @param[in] route The webhook route string (e.g. "/events").
 * @param[in] op The operation to add.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on bad args.
 */
extern C_CDD_EXPORT int openapi_aggregator_add_webhook_operation(
    struct OpenAPI_Spec *spec, const char *route, struct OpenAPI_Operation *op);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_AGGREGATOR_H */
