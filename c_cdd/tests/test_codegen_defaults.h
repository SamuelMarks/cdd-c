#ifndef TEST_CODEGEN_DEFAULTS_H
#define TEST_CODEGEN_DEFAULTS_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"

/* Helper to generate code and return as string buffer */
static char *generate_def_code(const char *struct_name,
                               struct StructFields *sf) {
  FILE *tmp = tmpfile();
  long sz;
  char *content;

  if (!tmp)
    return NULL;

  if (write_struct_default_func(tmp, struct_name, sf, NULL) != 0) {
    fclose(tmp);
    return NULL;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = calloc(1, sz + 1);
  if (sz > 0)
    fread(content, 1, sz, tmp);

  fclose(tmp);
  return content;
}

TEST test_default_primitive(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "x", "integer", NULL, "42");
  struct_fields_add(&sf, "flag", "boolean", NULL, "1");

  code = generate_def_code("Prim", &sf);
  ASSERT(code != NULL);

  ASSERT(strstr(code, "int Prim_default(struct Prim **out)"));
  ASSERT(strstr(code, "(*out)->x = 42;"));
  ASSERT(strstr(code, "(*out)->flag = 1;"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_default_string(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  /* JSON string defaults usually come quoted e.g. "\"foo\"" from parser */
  struct_fields_add(&sf, "s", "string", NULL, "\"hello\"");

  code = generate_def_code("StrS", &sf);
  ASSERT(code != NULL);

  ASSERT(strstr(code, "(*out)->s = strdup(\"hello\");"));
  ASSERT(strstr(
      code,
      "if (!(*out)->s) { StrS_cleanup(*out); *out=NULL; return ENOMEM; }"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_default_enum(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "e", "enum", "Color", "\"RED\"");

  code = generate_def_code("EnumStruct", &sf);
  ASSERT(code != NULL);

  /* Expect from_str call */
  ASSERT(strstr(code, "rc = Color_from_str(\"RED\", &(*out)->e);"));
  ASSERT(strstr(
      code,
      "if (rc != 0) { EnumStruct_cleanup(*out); *out=NULL; return rc; }"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_default_no_defaults(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "x", "integer", NULL, NULL);

  code = generate_def_code("NoDef", &sf);
  ASSERT(code != NULL);

  /* Should just be calloc */
  ASSERT(strstr(code, "calloc(1, sizeof(**out))"));
  ASSERT(strstr(code, "(*out)->x = ") == NULL);

  free(code);
  struct_fields_free(&sf);
  PASS();
}

SUITE(codegen_defaults_suite) {
  RUN_TEST(test_default_primitive);
  RUN_TEST(test_default_string);
  RUN_TEST(test_default_enum);
  RUN_TEST(test_default_no_defaults);
}

#endif /* TEST_CODEGEN_DEFAULTS_H */
