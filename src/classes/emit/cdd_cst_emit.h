#ifndef CDD_CST_EMIT_H
#define CDD_CST_EMIT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../parse/cdd_cst_node.h"
#include <c_str_span.h>
/* clang-format on */

/**
 * @brief Walks a CST tree and unparses it back to a string.
 *
 * @param tree The CST tree.
 * @param out_str Pointer to a dynamically allocated null-terminated string.
 *                Caller must free it.
 * @return 0 on success.
 */
int cdd_cst_emit(cdd_cst_tree_t *tree, char **out_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_EMIT_H */
