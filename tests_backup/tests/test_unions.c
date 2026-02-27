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
#include "functions/emit_codegen.h"

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

TEST test_write_union_from_json_object(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "pet", "object", "Pet", NULL, NULL);

  /* Generate */
  ASSERT_EQ(0, write_union_from_jsonObject_func(tmp, "ObjU", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = malloc(sz + 1);
  fread(content, 1, sz, tmp);
  content[sz] = 0;

  /* Check malloc */
  ASSERT(strstr(content, "malloc(sizeof(struct ObjU))"));
  ASSERT(strstr(content, "match_count"));
  ASSERT(strstr(content, "json_object_get_count"));
  ASSERT(strstr(content, "ret->tag = ObjU_pet;"));
  ASSERT(strstr(content, "Pet_from_jsonObject"));

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
  struct_fields_add(&sf, "s", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "i", "integer", NULL, NULL, NULL);

  ASSERT_EQ(0, write_union_from_json_func(tmp, "MixU", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = malloc(sz + 1);
  fread(content, 1, sz, tmp);
  content[sz] = 0;

  ASSERT(strstr(content, "json_parse_string"));
  ASSERT(strstr(content, "case JSONString"));
  ASSERT(strstr(content, "ret->tag = MixU_s;"));
  ASSERT(strstr(content, "case JSONNumber"));
  ASSERT(strstr(content, "ret->tag = MixU_i;"));

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
  RUN_TEST(test_write_union_from_json_object);
  RUN_TEST(test_write_union_from_json);
  RUN_TEST(test_write_union_cleanup);
}

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(unions_suite);
  GREATEST_MAIN_END();
}
