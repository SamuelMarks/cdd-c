#ifndef C_CDD_CST_PARSER_HELPERS_H
#define C_CDD_CST_PARSER_HELPERS_H

#include "c_cdd_export.h"
#include "tokenizer_types.h"
#include <stddef.h>

extern C_CDD_EXPORT size_t eatFunction(struct tokenizer_az_span_elem **, size_t,
                                       size_t);

#endif /* !C_CDD_CST_PARSER_HELPERS_H */
