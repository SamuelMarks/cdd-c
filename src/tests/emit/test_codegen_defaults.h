#ifndef TEST_CODEGEN_DEFAULTS_H
#define TEST_CODEGEN_DEFAULTS_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/codegen.h"

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
  struct_fields_add(&sf, "x", "integer", NULL, "42", NULL);
  struct_fields_add(&sf, "flag", "boolean", NULL, "1", NULL);

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
  struct_fields_add(&sf, "s", "string", NULL, "\"hello\"", NULL);

  code = generate_def_code("StrS", &sf);
  ASSERT(code != NULL);

  /* Platform-specific strdup selection is internal, check generic call or
   * result logic */
#if defined(_MSC_VER)
  ASSERT(strstr(code, "(*out)->s = _strdup(\"hello\");"));
#else
  ASSERT(strstr(code, "(*out)->s = strdup(\"hello\");"));
#endif
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
  struct_fields_add(&sf, "e", "enum", "Color", "\"RED\"", NULL);

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
  struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

  code = generate_def_code("NoDef", &sf);
  ASSERT(code != NULL);

  /* Should just be calloc */
  ASSERT(strstr(code, "calloc(1, sizeof(**out))"));
  ASSERT(strstr(code, "(*out)->x = ") == NULL);

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_default_nullptr(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "ptr_val", "integer", NULL, "nullptr",
                    NULL); /* treated as raw */
  struct_fields_add(&sf, "str_ptr", "string", NULL, "nullptr", NULL);

  code = generate_def_code("PtrStruct", &sf);
  ASSERT(code != NULL);

  /* Should map nullptr literal to NULL */
  ASSERT(strstr(code, "(*out)->ptr_val = NULL;"));
  ASSERT(strstr(code, "(*out)->str_ptr = NULL;"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_default_binary_literal(void) {
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  /* 0b101 -> 5 */
  struct_fields_add(&sf, "bin_val", "integer", NULL, "0b101", NULL);
  /* 0B11 -> 3 */
  struct_fields_add(&sf, "bin_cap", "integer", NULL, "0B11", NULL);

  code = generate_def_code("BinStruct", &sf);
  ASSERT(code != NULL);

  /* Should emit decimal values for C89 compatibility */
  /* Note: Assumes numeric_parser available and linked */
  /* Verify conversion */
  ASSERT(strstr(code, "(*out)->bin_val = 5;"));
  ASSERT(strstr(code, "(*out)->bin_cap = 3;"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

SUITE(codegen_defaults_suite) {
  RUN_TEST(test_default_primitive);
  RUN_TEST(test_default_string);
  RUN_TEST(test_default_enum);
  RUN_TEST(test_default_no_defaults);
  RUN_TEST(test_default_nullptr);
  RUN_TEST(test_default_binary_literal);
}

#endif /* TEST_CODEGEN_DEFAULTS_H */
