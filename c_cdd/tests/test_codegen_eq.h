#ifndef TEST_CODEGEN_EQ_H
#define TEST_CODEGEN_EQ_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"

/* Helper to generate code and return as string buffer */
static char *generate_eq_code(const char *struct_name,
                              struct StructFields *sf) {
  FILE *tmp = tmpfile();
  long sz;
  char *content = NULL;

  if (!tmp)
    return NULL;

  if (write_struct_eq_func(tmp, struct_name, sf, NULL) != 0) {
    fclose(tmp);
    return NULL;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  if (sz > 0) {
    content = calloc(1, sz + 1);
    fread(content, 1, sz, tmp);
  } else {
    content = strdup("");
  }

  fclose(tmp);
  return content;
}

TEST test_eq_primitive(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "ival", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "dval", "number", NULL, NULL, NULL);

  code = generate_eq_code("Prim", &sf);
  ASSERT(code != NULL);

  /* Check signature */
  ASSERT(
      strstr(code, "int Prim_eq(const struct Prim *a, const struct Prim *b)"));
  /* Check trivial */
  ASSERT(strstr(code, "if (a == b) return 1;"));
  ASSERT(strstr(code, "if (!a || !b) return 0;"));

  /* Check int cmp */
  ASSERT(strstr(code, "if (a->ival != b->ival) return 0;"));
  /* Check double cmp */
  ASSERT(strstr(code, "if (a->dval != b->dval) return 0;"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_eq_string(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "s", "string", NULL, NULL, NULL);

  code = generate_eq_code("StrS", &sf);
  ASSERT(code != NULL);

  /* Check string logic */
  /* if (a->s != b->s && (!a->s || !b->s || strcmp(a->s, b->s) != 0)) return 0;
   */
  ASSERT(strstr(code, "if (a->s != b->s && (!a->s || !b->s || strcmp(a->s, "
                      "b->s) != 0)) return 0;"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_eq_recursive_object(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "child", "object", "Child", NULL, NULL);

  code = generate_eq_code("Parent", &sf);
  ASSERT(code != NULL);

  /* Should call Child_eq recursively */
  ASSERT(strstr(code, "if (!Child_eq(a->child, b->child)) return 0;"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_eq_array_primitive(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "nums", "array", "integer", NULL, NULL);

  code = generate_eq_code("Arr", &sf);
  ASSERT(code != NULL);

  /* Check length check */
  ASSERT(strstr(code, "if (a->n_nums != b->n_nums) return 0;"));
  /* Check loop */
  ASSERT(strstr(code, "for (i = 0; i < a->n_nums; ++i)"));
  /* Check access */
  ASSERT(strstr(code, "if (a->nums[i] != b->nums[i]) return 0;"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_eq_array_string(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "strs", "array", "string", NULL, NULL);

  code = generate_eq_code("ArrS", &sf);
  ASSERT(code != NULL);

  /* Check loop */
  ASSERT(strstr(code, "for (i = 0; i < a->n_strs; ++i)"));
  /* Check strcmp access on arrays */
  ASSERT(strstr(code, "strcmp(a->strs[i], b->strs[i])"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_eq_array_object(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "items", "array", "Item", NULL, NULL);

  code = generate_eq_code("Box", &sf);
  ASSERT(code != NULL);

  /* Check loop */
  ASSERT(strstr(code, "for (i = 0; i < a->n_items; ++i)"));
  /* Check recursive array access: Item_eq(a->items[i], b->items[i]) */
  ASSERT(strstr(code, "if (!Item_eq(a->items[i], b->items[i])) return 0;"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

SUITE(codegen_eq_suite) {
  RUN_TEST(test_eq_primitive);
  RUN_TEST(test_eq_string);
  RUN_TEST(test_eq_recursive_object);
  RUN_TEST(test_eq_array_primitive);
  RUN_TEST(test_eq_array_string);
  RUN_TEST(test_eq_array_object);
}

#endif /* TEST_CODEGEN_EQ_H */
