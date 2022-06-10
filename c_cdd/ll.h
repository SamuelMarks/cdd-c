#ifndef C_CDD_LL_H
#define C_CDD_LL_H

#ifdef __cplusplus
extern "C" {
#else
#include "c_cdd_export.h"
#include "cst.h"
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

extern C_CDD_EXPORT const char *make_slice(const char *s, size_t i,
                                           size_t *start_index);

extern C_CDD_EXPORT struct str_elem **
ll_push_str(size_t *ll_n, struct str_elem ***ll_root, const char *s);

/*
 * `size_t`
 */

struct size_t_elem {
  size_t lu;
  struct size_t_elem *next;
};

/* List structure requiring manual bookkeeping for size */
struct size_t_list {
  uint32_t size;
  const struct size_t_elem *list;
};

extern C_CDD_EXPORT struct size_t_elem **size_t_list_end(struct size_t_elem **);

extern C_CDD_EXPORT struct size_t_elem **
size_t_list_prepend(struct size_t_elem **, size_t);

extern C_CDD_EXPORT struct size_t_elem **
size_t_list_append(struct size_t_elem **, size_t);

extern C_CDD_EXPORT void size_t_list_push(uint32_t *, struct size_t_elem ***,
                                          size_t);

extern C_CDD_EXPORT void size_t_elem_cleanup(struct size_t_elem **);

extern C_CDD_EXPORT void size_t_list_cleanup(struct size_t_list *);

/*
 * `az_span`
 */

struct az_span_elem {
  az_span span;
  struct az_span_elem *next;
};

/* List structure requiring manual bookkeeping for size */
struct az_span_list {
  uint32_t size;
  const struct az_span_elem *list;
};

extern C_CDD_EXPORT struct az_span_elem **
az_span_list_end(struct az_span_elem **);

extern C_CDD_EXPORT struct az_span_elem **
az_span_list_prepend(struct az_span_elem **, az_span);

extern C_CDD_EXPORT struct az_span_elem **
az_span_list_append(struct az_span_elem **, az_span);

extern C_CDD_EXPORT void az_span_list_push(uint32_t *, struct az_span_elem ***,
                                           az_span);

extern C_CDD_EXPORT void az_span_elem_cleanup(struct az_span_elem **);

extern C_CDD_EXPORT void az_span_list_cleanup(struct az_span_list *);

/*
 * `az_span`
 */

struct scan_az_span_elem {
  az_span span;
  enum CstNodeKind kind;
  struct scan_az_span_elem *next;
};

/* List structure requiring manual bookkeeping for size */
struct scan_az_span_list {
  uint32_t size;
  const struct scan_az_span_elem *list;
};

extern C_CDD_EXPORT struct scan_az_span_elem **
scan_az_span_list_end(struct scan_az_span_elem **);

extern C_CDD_EXPORT struct scan_az_span_elem **
scan_az_span_list_prepend(struct scan_az_span_elem **, az_span, enum CstNodeKind);

extern C_CDD_EXPORT struct scan_az_span_elem **
scan_az_span_list_append(struct scan_az_span_elem **, az_span);

extern C_CDD_EXPORT void scan_az_span_list_push(uint32_t *, struct scan_az_span_elem ***,
                                           az_span);

extern C_CDD_EXPORT void scan_az_span_elem_cleanup(struct scan_az_span_elem **);

extern C_CDD_EXPORT void scan_az_span_list_cleanup(struct scan_az_span_list *);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* !C_CDD_LL_H */
