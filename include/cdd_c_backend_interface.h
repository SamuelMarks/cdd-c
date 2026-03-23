/**
 * @file cdd_c_backend_interface.h
 * @brief Agnostic database driver bindings and abstractions.
 *
 * Provides an interface for `cdd-c` to safely interoperate with raw database
 * row outputs, regardless of backend (Postgres, SQLite, MySQL) when building
 * specific structs.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_BACKEND_INTERFACE_H
#define C_CDD_BACKEND_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_c_integration.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Standardized backend row representation wrapper.
 *
 * Contains a generic list of columns, their metadata, and arbitrary driver
 * context hooks if memory ownership must map securely out of the router phase.
 */
typedef struct CddCDriverRow {
  void **row_data; /* Dynamic generic array of pointers to driver primitives */
  cdd_c_column_meta_t
      *cols;        /* Column definitions bound from driver's schema APIs */
  size_t n_cols;    /* Number of columns */
  void *driver_ctx; /* Opaque pointer storing native handle for external
                       Identity Maps (e.g., sqlite3_stmt) */
} cdd_c_driver_row_t;

/**
 * @brief Adapter function bridging generic struct row logic down to native
 * database backends.
 *
 * Memory Ownership Rules:
 * 1. String properties (VARCHAR, TEXT, CHAR) extracted from drivers are
 *    DEEP-COPIED into the `out_struct` using standard dynamic allocation (e.g.
 * `strdup`), or copied into fixed length buffers if bounds are statically
 * known. The driver string lifecycle terminates after hydration returns.
 * 2. Blob structures must have their buffers allocated by the consumer if
 * ownership transfer persists beyond hydration scope.
 * 3. External integrations MUST execute `<struct>_free()` or
 * `cdd_c_abstract_struct_free()` on all outputted properties to reclaim dynamic
 * fields (nested arrays and strings) safely. Driver handles inside `row` MUST
 * NOT be explicitly closed or stepped by hydrators.
 */
typedef int (*cdd_c_row_hydrator_fn)(void *out_struct,
                                     const cdd_c_driver_row_t *row);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_BACKEND_INTERFACE_H */