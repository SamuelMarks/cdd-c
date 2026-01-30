#ifndef C_CDD_CST_PARSER_HELPERS_H
#define C_CDD_CST_PARSER_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "cst_parser_types.h"
#include "tokenizer_types.h"
#include <stddef.h>

extern C_CDD_EXPORT
    size_t eatFunction(const struct tokenizer_az_span_arr *, size_t, size_t,
                       const struct cst_node_arr *const *const *cst_arr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_CST_PARSER_HELPERS_H */
