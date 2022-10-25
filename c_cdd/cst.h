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

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#include <BaseTsd.h>
#include <errno.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/errno.h>
#endif

#include <c_str_span.h>

#include <c_cdd_export.h>

#include "ll.h"

struct CstNode;

#define DEBUG_SCANNER

extern C_CDD_EXPORT int tokenizer(az_span, struct tokenizer_az_span_list **);
extern C_CDD_EXPORT int cst_parser(const struct tokenizer_az_span_list *, struct parse_cst_list **);
/*const struct CstNode ** parser(struct az_span_elem *); */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_CST_H */
