#include <stdio.h>
#include <string.h>

#include "c_cdd_utils.h"
#include "ll.h"

struct str_elem **append(struct str_elem **root, const char *s) {
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

struct str_elem **str_push_to_ll(size_t *ll_n, struct str_elem ***ll_root,
                                 const char *expr) {
  if (expr != NULL) {
    (*ll_n)++;
    return append(*ll_root, expr);
  }
  return *ll_root;
}

const char *add_word(const char *s, const size_t i, size_t *scan_from) {
  const size_t substr_length = i - *scan_from + 1;
  if (substr_length > 0) {
    char *substr = malloc(sizeof(char) * substr_length);
    snprintf(substr, substr_length + 1, "%.*s", (int)substr_length,
             s + *scan_from);
    substr[substr_length] = '\0';
    print_escaped("$substr", substr);
    *scan_from = i + 1;
    return substr;
  }
  return NULL;
}
