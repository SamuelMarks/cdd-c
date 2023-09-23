#ifndef C_CDD_LL_H
#define C_CDD_LL_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdlib.h>
#endif /* __cplusplus */

#include <c_cdd_export.h>

#include "cst_parser_types.h"
#include "tokenizer_types.h"

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

extern C_CDD_EXPORT int ll_append_str(struct str_elem **root, const char *s,
                                      struct str_elem ***result);

/*
extern C_CDD_EXPORT const char *make_slice(const char *s, size_t i,
                                           size_t *start_index);
*/
extern C_CDD_EXPORT int ll_push_str(size_t *ll_n, struct str_elem ***ll_root,
                                    const char *s, struct str_elem ***);

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

extern C_CDD_EXPORT int size_t_list_end(struct size_t_elem **,
                                                         struct size_t_elem ***);

extern C_CDD_EXPORT int size_t_list_prepend(struct size_t_elem **, size_t, struct size_t_elem ***);

extern C_CDD_EXPORT int size_t_list_append(struct size_t_elem **, size_t, struct size_t_elem ***);

extern C_CDD_EXPORT int size_t_list_push(uint32_t *, struct size_t_elem ***,
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
 * `tokenizer_az_span`
 */

struct tokenizer_az_span_elem_arr {
  az_span span;
  enum TokenizerKind kind;
};

struct tokenizer_az_span_elem {
  az_span span;
  enum TokenizerKind kind;
  struct tokenizer_az_span_elem *next;
};

struct tokenizer_az_span_element {
  az_span span;
  enum TokenizerKind kind;
};

/* List structure requiring manual bookkeeping for size */
struct tokenizer_az_span_list {
  size_t size;
  const struct tokenizer_az_span_elem *list;
};

extern C_CDD_EXPORT int
tokenizer_az_span_list_to_array(struct tokenizer_az_span_element ***,
                                const struct tokenizer_az_span_list *);

extern C_CDD_EXPORT struct tokenizer_az_span_elem **
tokenizer_az_span_list_end(struct tokenizer_az_span_elem **);

extern C_CDD_EXPORT struct tokenizer_az_span_elem **
tokenizer_az_span_list_prepend(struct tokenizer_az_span_elem **,
                               enum TokenizerKind, az_span);

extern C_CDD_EXPORT struct tokenizer_az_span_elem **
tokenizer_az_span_list_append(struct tokenizer_az_span_elem **,
                              enum TokenizerKind, az_span);

extern C_CDD_EXPORT void
tokenizer_az_span_list_push(size_t *, struct tokenizer_az_span_elem ***,
                            enum TokenizerKind, az_span);

extern C_CDD_EXPORT void
tokenizer_az_span_elem_cleanup(struct tokenizer_az_span_elem **);

extern C_CDD_EXPORT void
tokenizer_az_span_list_cleanup(struct tokenizer_az_span_list *);

/*
 * `parse_cst`
 */

struct parse_cst_elem {
  enum CstNodeKind kind;
  union CstNodeType type;
  struct parse_cst_elem *next;
};

/* List structure requiring manual bookkeeping for size */
struct parse_cst_list {
  size_t size;
  const struct parse_cst_elem *list;
};

extern C_CDD_EXPORT struct parse_cst_elem **
parse_cst_list_end(struct parse_cst_elem **);

extern C_CDD_EXPORT struct parse_cst_elem **
parse_cst_list_prepend(struct parse_cst_elem **, enum TokenizerKind, az_span);

extern C_CDD_EXPORT struct parse_cst_elem **
parse_cst_list_append(struct parse_cst_elem **, enum TokenizerKind, az_span);

extern C_CDD_EXPORT void parse_cst_list_push(size_t *,
                                             struct parse_cst_elem ***,
                                             enum TokenizerKind, az_span);

extern C_CDD_EXPORT void parse_cst_elem_cleanup(struct parse_cst_elem **);

extern C_CDD_EXPORT void parse_cst_list_cleanup(struct parse_cst_list *);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* !C_CDD_LL_H */
