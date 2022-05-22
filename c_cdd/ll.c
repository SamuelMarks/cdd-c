#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "c_cdd_utils.h"
#include "ll.h"

/*
 * `const char *`
 */

struct str_elem **ll_append_str(struct str_elem **root, const char *s) {
  struct str_elem **insert = &*root;
  while (*insert)
    insert = &insert[0]->next;
  *insert = malloc(sizeof **insert);
  if (!*insert)
    exit(ENOMEM);
  insert[0]->s = s;
  insert[0]->n = strlen(s);
  insert[0]->next = NULL;
  return insert;
}

struct str_elem **ll_push_str(size_t *ll_n, struct str_elem ***ll_root,
                              const char *s) {
  if (s != NULL) {
    (*ll_n)++;
    return ll_append_str(*ll_root, s);
  }
  return *ll_root;
}

const char *slice_(const char *s, const size_t i, size_t *start_index) {
  const size_t substr_length = i + *start_index + 1;
  if (substr_length > 0) {
    char *substr = malloc(sizeof *substr * substr_length);
    const size_t wrote_length = snprintf(substr, substr_length + 1, "%.*s",
                                         (int)substr_length, s + *start_index) -
                                1;
    assert(wrote_length == substr_length);
    *start_index = i + 1;
    return substr;
  }
  return NULL;
}

const char *make_slice(const char *s, const size_t i, size_t *start_index) {
  const size_t substr_length = i - *start_index + 1;
  if (substr_length > 0) {
    char *substr = malloc(sizeof *substr * substr_length);
    const int sn = snprintf(substr, substr_length + 1, "%.*s",
                            (int)substr_length, s + *start_index);
    assert(sn == substr_length);
    substr[substr_length] = '\0';
    print_escaped("make_slice::substr", substr);
    //*start_index = i + 1;
    *start_index = substr_length + i;
    return substr;
  }
  return NULL;
}

/*
 * `size_t`
 */

struct size_t_elem **size_t_list_end(struct size_t_elem **size_t_elem) {
  assert(size_t_elem);
  while (*size_t_elem)
    size_t_elem = &size_t_elem[0]->next;
  return size_t_elem;
}

struct size_t_elem **size_t_list_prepend(struct size_t_elem **size_t_elem,
                                         const size_t lu) {
  struct size_t_elem *x = malloc(sizeof *x);
  assert(size_t_elem);
  if (!x)
    return NULL;
  x->lu = lu;
  x->next = *size_t_elem;
  *size_t_elem = x;
  return &x->next;
}

struct size_t_elem **size_t_list_append(struct size_t_elem **p,
                                        const size_t lu) {
  return size_t_list_prepend(size_t_list_end(p), lu);
}

void size_t_list_push(uint32_t *ll_n, struct size_t_elem ***ll_root,
                      const size_t lu) {
  (*ll_n)++;
  *ll_root = size_t_list_append(*ll_root, lu);
}

struct size_t_elem **size_t_cleanup(struct size_t_elem **size_t_elem) {
  if (size_t_elem == NULL) return size_t_elem;
  while (*size_t_elem) {
    struct size_t_elem *cur = size_t_elem[0];
    size_t_elem = &size_t_elem[0]->next;
    free(cur);
  }
  return size_t_elem;
}

/*
 * `az_span`
 */

struct az_span_elem **az_span_list_end(struct az_span_elem **span_elem) {
  assert(span_elem);
  while (*span_elem)
    span_elem = &span_elem[0]->next;
  return span_elem;
}

struct az_span_elem **az_span_list_prepend(struct az_span_elem **span_elem,
                                           const az_span span) {
  struct az_span_elem *x = malloc(sizeof *x);
  assert(span_elem);
  if (!x)
    return NULL;
  x->span = span;
  x->next = *span_elem;
  *span_elem = x;
  return &x->next;
}

struct az_span_elem **az_span_list_append(struct az_span_elem **p,
                                          const az_span span) {
  return az_span_list_prepend(az_span_list_end(p), span);
}

void az_span_list_push(uint32_t *ll_n, struct az_span_elem ***ll_root,
                       const az_span span) {
  if (az_span_ptr(span) != NULL && az_span_size(span) > 0) {
    (*ll_n)++;
    *ll_root = az_span_list_append(*ll_root, span);
  }
}
