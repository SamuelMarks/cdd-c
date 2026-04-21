/* clang-format off */
#include "cdd_cst_escape.h"
#include <errno.h>
#include <string.h>
/* clang-format on */

int cdd_cst_analyze_escapes(cdd_cst_tree_t *tree, cdd_cst_scope_env_t *env) {
  if (!tree || !env)
    return EINVAL;
  /* Stub: In a full implementation, we would traverse the tree looking for
     unary '&' operators applied to variables, and also track variable
     references inside nested functions. For now, we just return success. */
  return 0;
}

int cdd_cst_symbol_escapes(cdd_cst_symbol_t *symbol, int *out_escapes) {
  if (!symbol || !out_escapes)
    return EINVAL;
  /* Stub: In a full implementation, this state would be stored on the symbol
     itself during the cdd_cst_analyze_escapes pass. */
  *out_escapes = 0;
  return 0;
}
