#ifndef C_CDD_CST_H
#define C_CDD_CST_H

#ifdef __cplusplus
#include <cstdlib>
#include <cstring>
extern "C" {
#else
#include <stdlib.h>
#include <string.h>

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#endif /* __cplusplus */

#include <c_str_span.h>

#include <c_cdd_export.h>

#include <tokenizer_types.h>
#include <cst_parser_types.h>

extern C_CDD_EXPORT int tokenizer(az_span,
                                  struct tokenizer_az_span_arr *const *);
extern C_CDD_EXPORT int cst_parser(const struct tokenizer_az_span_arr *,
                                   struct cst_node_arr *const *);
/*const struct CstNode ** parser(struct az_span_elem *); */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_CST_H */
