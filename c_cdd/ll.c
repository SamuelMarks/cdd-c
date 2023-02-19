#if defined(__linux) || defined(__linux__) || defined(linux)
#define _GNU_SOURCE
#elif defined(BSD) || defined(__FreeBSD__) || defined(__FreeBSD__) ||          \
    defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) ||        \
    defined(__DragonFly__)
#define _BSD_SOURCE
#endif
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "c_cdd_utils.h"
#include "ll.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <c89stringutils_string_extras.h>
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

/*
 * `const char *`
 */

struct str_elem **ll_append_str(struct str_elem **root, const char *const s) {
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

struct str_elem **ll_push_str(size_t *const ll_n, struct str_elem ***ll_root,
                              const char *const s) {
  if (s != NULL) {
    (*ll_n)++;
    return ll_append_str(*ll_root, s);
  }
  return *ll_root;
}

const char *slice_(const char *const s, const size_t i,
                   size_t *const start_index) {
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

const char *make_slice(const char *const s, const size_t i,
                       size_t *const start_index) {
  const size_t substr_length = i - *start_index + 1;
  if (substr_length > 0) {
    char *substr = malloc(sizeof *substr * substr_length);
    const int sn = snprintf(substr, substr_length + 1, "%.*s",
                            (int)substr_length, s + *start_index);
    assert(sn == substr_length);
    substr[substr_length] = '\0';
    print_escaped("make_slice::substr", substr);
    /* *start_index = i + 1; */
    *start_index = substr_length + i;
    return substr;
  }
  return NULL;
}

/*
 * `size_t`
 */

struct size_t_elem **size_t_list_end(struct size_t_elem **size_t_elem) {
  if (!size_t_elem)
    return size_t_elem;
  while (*size_t_elem)
    size_t_elem = &size_t_elem[0]->next;
  return size_t_elem;
}

struct size_t_elem **size_t_list_prepend(struct size_t_elem **size_t_elem,
                                         const size_t lu) {
  struct size_t_elem *new_size_t_elem = malloc(sizeof *new_size_t_elem);
  if (!new_size_t_elem || !size_t_elem)
    return NULL;
  new_size_t_elem->lu = lu;
  new_size_t_elem->next = *size_t_elem;
  *size_t_elem = new_size_t_elem;
  return &new_size_t_elem->next;
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

void size_t_elem_cleanup(struct size_t_elem **size_t_elem) {
  if (*size_t_elem == NULL)
    return;
  {
    struct size_t_elem *cur = *size_t_elem;
    while (cur != NULL) {
      struct size_t_elem *tmp = cur;
      cur = cur->next;
      free(tmp);
    }
  }
  *size_t_elem = NULL;
}

void size_t_list_cleanup(struct size_t_list *size_t_ll) {
  struct size_t_elem *list = (struct size_t_elem *)size_t_ll->list;
  size_t_elem_cleanup(&list);
  size_t_ll->list = NULL, size_t_ll->size = 0;
}

/*
 * `az_span`
 */

struct az_span_elem **az_span_list_end(struct az_span_elem **span_elem) {
  assert(span_elem);
  while (*span_elem)
    span_elem = &(**span_elem).next;
  return span_elem;
}

struct az_span_elem **az_span_list_prepend(struct az_span_elem **span_elem,
                                           const az_span span) {
  struct az_span_elem *new_span_elem = malloc(sizeof *new_span_elem);
  if (!new_span_elem || !span_elem)
    return span_elem;
  new_span_elem->span = span;
  new_span_elem->next = *span_elem;
  *span_elem = new_span_elem;
  return &new_span_elem->next;
}

struct az_span_elem **az_span_list_append(struct az_span_elem **p,
                                          const az_span span) {
  /* print_escaped_span("az_span_list_append::span", span); */
  return az_span_list_prepend(az_span_list_end(p), span);
}

void az_span_list_push(uint32_t *ll_n, struct az_span_elem ***ll_root,
                       const az_span span) {
  (*ll_n)++;
  *ll_root = az_span_list_append(*ll_root, span);
}

void az_span_elem_cleanup(struct az_span_elem **az_span_element) {
  if (az_span_element == NULL)
    return;

  {
    struct az_span_elem *cur = *az_span_element;
    while (cur != NULL) {
      struct az_span_elem *tmp = cur;
      cur = cur->next;
      free(tmp);
    }
  }
  *az_span_element = NULL;
}

void az_span_list_cleanup(struct az_span_list *size_t_ll) {
  struct az_span_elem *list = (struct az_span_elem *)size_t_ll->list;
  az_span_elem_cleanup(&list);
  size_t_ll->list = NULL, size_t_ll->size = 0;
}

/*
 * `tokenizer_az_span`
 */

struct tokenizer_az_span_elem **
tokenizer_az_span_list_end(struct tokenizer_az_span_elem **span_elem) {
  assert(span_elem);
  while (*span_elem)
    span_elem = &(**span_elem).next;
  return span_elem;
}

struct tokenizer_az_span_elem **
tokenizer_az_span_list_prepend(struct tokenizer_az_span_elem **const span_elem,
                               enum TokenizerKind kind, const az_span span) {
  struct tokenizer_az_span_elem *new_span_elem = malloc(sizeof *new_span_elem);
  if (!new_span_elem || !span_elem)
    return span_elem;
  new_span_elem->span = span, new_span_elem->next = *span_elem,
  new_span_elem->kind = kind;
  *span_elem = new_span_elem;
  return &new_span_elem->next;
}

struct tokenizer_az_span_elem **
tokenizer_az_span_list_append(struct tokenizer_az_span_elem **p,
                              const enum TokenizerKind kind,
                              const az_span span) {
  /* print_escaped_span("az_span_list_append::span", span); */
  return tokenizer_az_span_list_prepend(tokenizer_az_span_list_end(p), kind,
                                        span);
}

void tokenizer_az_span_list_push(size_t *ll_n,
                                 struct tokenizer_az_span_elem ***ll_root,
                                 const enum TokenizerKind kind,
                                 const az_span span) {
  (*ll_n)++;
  *ll_root = tokenizer_az_span_list_append(*ll_root, kind, span);
}

void tokenizer_az_span_elem_cleanup(
    struct tokenizer_az_span_elem **tokenizer_az_span_element) {
  if (tokenizer_az_span_element == NULL)
    return;

  {
    struct tokenizer_az_span_elem *cur = *tokenizer_az_span_element;
    while (cur != NULL) {
      struct tokenizer_az_span_elem *tmp = cur;
      cur = cur->next;
      free(tmp);
    }
  }
  *tokenizer_az_span_element = NULL;
}

void tokenizer_az_span_list_cleanup(struct tokenizer_az_span_list *size_t_ll) {
  struct tokenizer_az_span_elem *list =
      (struct tokenizer_az_span_elem *)size_t_ll->list;
  tokenizer_az_span_elem_cleanup(&list);
  size_t_ll->list = NULL, size_t_ll->size = 0;
}

int tokenizer_az_span_list_to_array(
    struct tokenizer_az_span_element ***arr,
    const struct tokenizer_az_span_list *const ll) {
  struct tokenizer_az_span_elem *iter;
  size_t i;
  *arr = malloc((ll->size + 1) * sizeof(*arr));

  for (iter = (struct tokenizer_az_span_elem *)ll->list, i = 0; iter != NULL;
       iter = iter->next, i++) {
    char *name;

    (*arr)[i] = malloc(sizeof((*arr)[i][0]));
    (*arr)[i]->span = iter->span, (*arr)[i]->kind = iter->kind;

    asprintf(&name, "lis[%ld]:%s", i, TokenizerKind_to_str((*arr)[i]->kind));
    print_escaped_span(name, (*arr)[i]->span);
    free(name);
  }

  putchar('\n');

  {
    struct tokenizer_az_span_element *el;
    for (el = **arr, i = 0; el != NULL && i < ll->size; el++, i++) {
      char *nom;
      /*
      char *name;
      asprintf(&name, "ele[%ld]:%s", i, TokenizerKind_to_str(el->kind));
      print_escaped_span(name, el->span);
      free(name);
      */

      asprintf(&nom, "arr[%" NUM_LONG_FMT "d]:%s", i,
               TokenizerKind_to_str((*arr)[i]->kind));
      print_escaped_span(nom, (*arr)[i]->span);
      free(nom);
      /* putchar('\n'); */
    }
  }

  assert(ll->size == i);
  print_escaped_span("(*arr)[25]->span", (*arr)[25]->span);
  arr[ll->size] = NULL;
  printf("\n"
         "tok_span_ll_a::i                  = %" NUM_LONG_FMT "u\n"
         "tok_span_ll_a::ll->n              = %" NUM_LONG_FMT "u\n\n",
         i, ll->size);

  return EXIT_SUCCESS;
}

/*
 * `parse_cst`
 */

struct parse_cst_elem **parse_cst_list_end(struct parse_cst_elem **span_elem) {
  assert(span_elem);
  while (*span_elem)
    span_elem = &(**span_elem).next;
  return span_elem;
}

struct parse_cst_elem **
parse_cst_list_prepend(struct parse_cst_elem **span_elem,
                       enum TokenizerKind kind, const az_span span) {
  struct parse_cst_elem *new_span_elem = malloc(sizeof *new_span_elem);
  if (!new_span_elem || !span_elem)
    return span_elem;
  /* new_span_elem->type = malloc(sizeof(struct Expression *)); */
  new_span_elem->kind = Expression;
  new_span_elem->next = *span_elem;
  *span_elem = new_span_elem;
  return &new_span_elem->next;
}

struct parse_cst_elem **parse_cst_list_append(struct parse_cst_elem **const p,
                                              const enum TokenizerKind kind,
                                              const az_span span) {
  /* print_escaped_span("az_span_list_append::span", span); */
  return parse_cst_list_prepend(parse_cst_list_end(p), kind, span);
}

void parse_cst_list_push(size_t *ll_n, struct parse_cst_elem ***const ll_root,
                         const enum TokenizerKind kind, const az_span span) {
  (*ll_n)++;
  *ll_root = parse_cst_list_append(*ll_root, kind, span);
}

void parse_cst_elem_cleanup(struct parse_cst_elem **const parse_cst_element) {
  if (parse_cst_element == NULL)
    return;

  {
    struct parse_cst_elem *cur = *parse_cst_element;
    while (cur != NULL) {
      struct parse_cst_elem *tmp = cur;
      cur = cur->next;
      free(tmp);
    }
  }
  *parse_cst_element = NULL;
}

void parse_cst_list_cleanup(struct parse_cst_list *const ll) {
  struct parse_cst_elem *list = (struct parse_cst_elem *)ll->list;
  parse_cst_elem_cleanup(&list);
  ll->list = NULL, ll->size = 0;
}
