/**
 * @file test_codegen_struct.h
 * @brief Unit tests for Struct Lifecycle generation logic.
 *
 * Verifies that C utility string generation includes necessary null checks,
 * memory allocations, and recursion.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_H
#define TEST_CODEGEN_STRUCT_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit_struct.h"

static void setup_struct_fields(struct StructFields *sf) {
  struct_fields_init(sf);
  struct_fields_add(sf, "id", "integer", NULL, "0", NULL);
  struct_fields_add(sf, "name", "string", NULL, "\"test\"", NULL);
}

TEST test_cleanup_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  long sz;
  char *content;

  ASSERT(tmp);
  setup_struct_fields(&sf);

  ASSERT_EQ(0, write_struct_cleanup_func(tmp, "User", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "void User_cleanup(struct User *const obj)"));
  ASSERT(strstr(content, "if (!obj) return;"));
  ASSERT(strstr(content, "if (obj->name) free((void*)obj->name);"));
  ASSERT(strstr(content, "free(obj);"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_default_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content;
  long sz;

  ASSERT(tmp);
  setup_struct_fields(&sf);

  ASSERT_EQ(0, write_struct_default_func(tmp, "User", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "*out = calloc(1, sizeof(**out));"));
  /* Check literals injected */
  ASSERT(strstr(content, "(*out)->id = 0;"));
  /* Check string duplication */
  ASSERT(strstr(content, "strdup(\"test\");"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_deepcopy_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content;
  long sz;

  ASSERT(tmp);
  setup_struct_fields(&sf);

  ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "User", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "memcpy(*dest, src, sizeof(struct User));"));
  ASSERT(strstr(content, "if (src->name) {"));
  ASSERT(strstr(content, "(*dest)->name = strdup(src->name);"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_eq_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content;
  long sz;

  ASSERT(tmp);
  setup_struct_fields(&sf);

  ASSERT_EQ(0, write_struct_eq_func(tmp, "User", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "if (a == b) return 1;"));
  ASSERT(strstr(content, "a->id != b->id"));
  ASSERT(strstr(content, "strcmp(a->name, b->name)"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_guards_injection(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  struct CodegenStructConfig cfg;
  char *content;
  long sz;

  ASSERT(tmp);
  setup_struct_fields(&sf);
  cfg.guard_macro = "MY_GUARD";

  ASSERT_EQ(0, write_struct_cleanup_func(tmp, "User", &sf, &cfg));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "#ifdef MY_GUARD"));
  ASSERT(strstr(content, "#endif /* MY_GUARD */"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_null_args(void) {
  ASSERT_EQ(EINVAL, write_struct_cleanup_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(EINVAL, write_struct_default_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(EINVAL, write_struct_deepcopy_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(EINVAL, write_struct_eq_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(EINVAL, write_struct_debug_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(EINVAL, write_struct_display_func(NULL, "U", NULL, NULL));
  PASS();
}

TEST test_struct_fields_add_bitwidth(void) {
  struct StructFields sf;
  struct_fields_init(&sf);

  /* Add bitfield */
  ASSERT_EQ(0, struct_fields_add(&sf, "flag", "integer", NULL, NULL, "3"));
  ASSERT_EQ(1, sf.size);
  ASSERT_STR_EQ("flag", sf.fields[0].name);
  ASSERT_STR_EQ("3", sf.fields[0].bit_width);

  /* Add normal */
  ASSERT_EQ(0, struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL));
  ASSERT_EQ(2, sf.size);
  ASSERT_STR_EQ("\0", sf.fields[1].bit_width);

  struct_fields_free(&sf);
  PASS();
}

SUITE(codegen_struct_suite) {
  RUN_TEST(test_cleanup_generation);
  RUN_TEST(test_default_generation);
  RUN_TEST(test_deepcopy_generation);
  RUN_TEST(test_eq_generation);
  RUN_TEST(test_guards_injection);
  RUN_TEST(test_null_args);
  RUN_TEST(test_struct_fields_add_bitwidth);
}

#endif /* TEST_CODEGEN_STRUCT_H */
