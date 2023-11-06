#ifndef C_CDD_CST_PARSER_HELPERS_H
#define C_CDD_CST_PARSER_HELPERS_H

#include "c_cdd_export.h"
#include "cst_parser_types.h"
#include "tokenizer_types.h"
#include <stddef.h>

extern C_CDD_EXPORT size_t eatFunction(const struct tokenizer_az_span_arr *,
                                       size_t, size_t,
                                       struct cst_node_arr *cst_arr);

#endif /* !C_CDD_CST_PARSER_HELPERS_H */
