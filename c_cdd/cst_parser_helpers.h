#ifndef C_CDD_CST_PARSER_HELPERS_H
#define C_CDD_CST_PARSER_HELPERS_H

#include "c_cdd_export.h"
#include "ll.h"
#include <stddef.h>

extern C_CDD_EXPORT size_t eatFunction(struct tokenizer_az_span_element **,
                                       size_t, size_t,
                                       struct tokenizer_az_span_elem ***,
                                       struct parse_cst_list *);

#endif /* !C_CDD_CST_PARSER_HELPERS_H */
