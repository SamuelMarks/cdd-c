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

/* two phase parser */
extern C_CDD_EXPORT struct scan_az_span_list *scanner(az_span);
extern C_CDD_EXPORT const struct az_span_elem *
tokenizer(const struct scan_az_span_list *);
extern C_CDD_EXPORT const struct CstNode **parser(struct az_span_elem *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_CST_H */
