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

struct str_elem {
  const char *s;
  size_t n;
  struct str_elem *next;
};

extern C_CDD_EXPORT struct str_elem **append(struct str_elem **, const char *);

extern C_CDD_EXPORT const char *add_word(const char *, size_t, size_t *);

extern C_CDD_EXPORT struct str_elem **
str_push_to_ll(size_t *, struct str_elem ***, const char *);

#ifdef __cplusplus
}
#endif

#endif /* !C_CDD_LL_H */
