/**
 * @file test_unions.c
 * @brief Unit tests for Tagged Union code generation.
 * Verifies that the generator produces code that correctly handles
 * polymorphism (oneOf) via a discriminator tag.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "codegen.h"

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

TEST test_write_union_to_json(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "name", "string", NULL, NULL, NULL);

  /* Generate */
  ASSERT_EQ(0, write_union_to_json_func(tmp, "MyUnion", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = malloc(sz + 1);
  fread(content, 1, sz, tmp);
  content[sz] = 0;

  /* Check for switch on tag */
  ASSERT(strstr(content, "switch (obj->tag)"));
  /* Check case for id */
  ASSERT(strstr(content, "case MyUnion_id:"));
  ASSERT(strstr(content, "obj->data.id"));
  /* Check case for name */
  ASSERT(strstr(content, "case MyUnion_name:"));
  ASSERT(strstr(content, "obj->data.name"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_write_union_from_json(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "val", "integer", NULL, NULL, NULL);

  /* Generate */
  ASSERT_EQ(0, write_union_from_jsonObject_func(tmp, "NumU", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = malloc(sz + 1);
  fread(content, 1, sz, tmp);
  content[sz] = 0;

  /* Check malloc */
  ASSERT(strstr(content, "malloc(sizeof(struct NumU))"));
  /* Check discriminator check */
  ASSERT(strstr(content, "if (json_object_has_value(jsonObject, \"val\"))"));
  ASSERT(strstr(content, "ret->tag = NumU_val;"));
  ASSERT(strstr(content, "ret->data.val = (int)json_object_get_number"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_write_union_cleanup(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "str", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "num", "integer", NULL, NULL, NULL);

  ASSERT_EQ(0, write_union_cleanup_func(tmp, "U", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = malloc(sz + 1);
  fread(content, 1, sz, tmp);
  content[sz] = 0;

  /* Check switch */
  ASSERT(strstr(content, "switch (obj->tag)"));
  /* Integer should do nothing (just break) */
  ASSERT(strstr(content, "case U_num:\n      break;"));
  /* String should free */
  ASSERT(strstr(content, "case U_str:\n      free((void*)obj->data.str);"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

SUITE(unions_suite) {
  RUN_TEST(test_write_union_to_json);
  RUN_TEST(test_write_union_from_json);
  RUN_TEST(test_write_union_cleanup);
}

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(unions_suite);
  GREATEST_MAIN_END();
}
