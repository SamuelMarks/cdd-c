#ifndef C_CDD_LL_H
#define C_CDD_LL_H

#ifdef __cplusplus
extern "C" {
#else
#include "c_cdd_export.h"
#include <stdlib.h>
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#include <BaseTsd.h>
#include <errno.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/errno.h>
#endif

#include <c_str_span.h>

/*
 * `const char *`
 */

struct str_elem {
  const char *s;
  size_t n;
  struct str_elem *next;
};

extern C_CDD_EXPORT struct str_elem **ll_append_str(struct str_elem **root,
                                                    const char *s);

extern C_CDD_EXPORT const char *add_word(const char *, size_t, size_t *);

extern C_CDD_EXPORT struct str_elem **
ll_push_str(size_t *ll_n, struct str_elem ***ll_root, const char *s);

/*
 * `az_span`
 */

struct az_span_elem {
  az_span span;
  struct az_span_elem *next;
};

extern C_CDD_EXPORT struct az_span_elem **
ll_append_span(struct az_span_elem **root, az_span);

extern C_CDD_EXPORT struct az_span_elem **
ll_push_span(int32_t *, struct az_span_elem ***, az_span);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* !C_CDD_LL_H */
