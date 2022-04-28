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

const char *slice(const char *s, const size_t i, size_t *scan_from) {
  const size_t substr_length = i + *scan_from + 1;
  if (substr_length > 0) {
    char *substr = (char *)malloc(sizeof(char) * substr_length);
    assert(snprintf(substr, substr_length + 1, "%.*s", (int)substr_length,
                    s + *scan_from) -
               1 ==
           substr_length);
    *scan_from = i + 1;
    return substr;
  }
  return NULL;
}

const char *add_word(const char *s, const size_t i, size_t *scan_from) {
  const size_t substr_length = i - *scan_from + 1;
  if (substr_length > 0) {
    char *substr = (char *)malloc(sizeof(char) * substr_length);
    const int sn = snprintf(substr, substr_length + 1, "%.*s",
                            (int)substr_length, s + *scan_from);
    printf("add_word::n           = %ld\n"
           "add_word::snprintf    = %d\n",
           substr_length, sn);
    assert(sn == substr_length);
    substr[substr_length] = '\0';
    print_escaped("add_word::substr", substr);
    //*scan_from = i + 1;
    *scan_from = substr_length + i;
    return substr;
  }
  return NULL;
}

/*
 * `az_span`
 */

struct az_span_elem **list_end(struct az_span_elem **p) {
  assert(p);
  while (*p)
    p = &p[0]->next;
  return p;
}

struct az_span_elem **ll_prepend_span(struct az_span_elem **p, az_span span) {
  struct az_span_elem *x = malloc(sizeof *x);
  assert(p);
  if (!x)
    return NULL;
  x->span = span;
  x->next = *p;
  *p = x;
  return &x->next;
}

struct az_span_elem **ll_append_span(struct az_span_elem **p, az_span span) {
  return ll_prepend_span(list_end(p), span);
}

struct az_span_elem **ll_push_span(size_t *ll_n, struct az_span_elem ***ll_root,
                                   const az_span span) {
  if (az_span_ptr(span) != NULL && az_span_size(span) > 0) {
    (*ll_n)++;
    return ll_append_span(*ll_root, span);
  }
  return *ll_root;
}
