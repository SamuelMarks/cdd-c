/**
 * @file cdd_c_integration.h
 * @brief Generic integration headers for embedding `cdd-c` in external ORMs.
 *
 * This file consolidates all public API headers needed by consumer frameworks
 * (like `c-orm`) to instantiate the router, execute abstract hydration,
 * and dispatch generated rows safely.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_INTEGRATION_H
#define C_CDD_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* To embed seamlessly, we expose the primary metadata blocks used for mappings
 */
/* clang-format off */
#include "../src/classes/emit/c_orm_meta.h"
#include "../src/classes/parse/abstract_struct.h"
#include "../src/classes/parse/hydrate_router.h"
/* clang-format on */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_INTEGRATION_H */