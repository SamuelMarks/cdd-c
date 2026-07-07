#ifndef TEST_CODEGEN_DEFAULTS_H
#define TEST_CODEGEN_DEFAULTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/codegen.h"
/* clang-format on */
/* LCOV_EXCL_START */

#ifdef _WIN32
#else
#endif

/* Helper to generate code and return as string buffer */
static enum cdd_c_error generate_def_code(const char *struct_name,
                                          struct StructFields *sf,
                                          char **_out_val) {
  FILE *tmp = tmpfile();
  long sz;
  char *content = NULL;

  if (!tmp) {
    *_out_val = NULL;
    return 0;
  }

  if (write_struct_default_func(tmp, struct_name, sf, NULL) != 0) {
    fclose(tmp);
    {
      *_out_val = NULL;
      return 0;
    }
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  if (sz > 0)
    fread(content, 1, sz, tmp);

  fclose(tmp);
  {
    *_out_val = content;
    return 0;
  }
}

/**
 * @brief test_default_primitive
 * @return TEST
 */
TEST test_default_primitive(void) {
  char *_ast_generate_def_code_0 = NULL;
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "x", "integer", NULL, "42", NULL);
  struct_fields_add(&sf, "flag", "boolean", NULL, "1", NULL);

  code = (generate_def_code("Prim", &sf, &_ast_generate_def_code_0),
          _ast_generate_def_code_0);
  ASSERT(code != NULL);

  ASSERT(strstr(code, "enum cdd_c_error Prim_default(struct Prim **out)"));
  ASSERT(strstr(code, "(*out)->x = 42;"));
  ASSERT(strstr(code, "(*out)->flag = 1;"));

  free(code);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_default_string
 * @return TEST
 */
TEST test_default_string(void) {
  char *_ast_generate_def_code_1 = NULL;
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  /* JSON string defaults usually come quoted e.g. "\"foo\"" from parser */
  struct_fields_add(&sf, "s", "string", NULL, "\"hello\"", NULL);

  code = (generate_def_code("StrS", &sf, &_ast_generate_def_code_1),
          _ast_generate_def_code_1);
  ASSERT(code != NULL);

  /* Platform-specific strdup selection is internal, check generic call or
   * result logic */
#if defined(_MSC_VER)
  ASSERT(strstr(code, "(*out)->s = _strdup(\"hello\");"));
#else
  ASSERT(strstr(code, "(*out)->s = strdup(\"hello\");"));
#endif
  ASSERT(strstr(code, "if (!(*out)->s) { StrS_cleanup(*out); *out=NULL; return "
                      "CDD_C_ERROR_MEMORY; }"));

  free(code);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_default_enum
 * @return TEST
 */
TEST test_default_enum(void) {
  char *_ast_generate_def_code_2 = NULL;
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "e", "enum", "Color", "\"RED\"", NULL);

  code = (generate_def_code("EnumStruct", &sf, &_ast_generate_def_code_2),
          _ast_generate_def_code_2);
  ASSERT(code != NULL);

  /* Expect from_str call */
  ASSERT(strstr(code, "rc = Color_from_str(\"RED\", &(*out)->e);"));
  ASSERT(strstr(
      code,
      "if (rc != 0) { EnumStruct_cleanup(*out); *out=NULL; return rc; }"));

  free(code);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_default_no_defaults
 * @return TEST
 */
TEST test_default_no_defaults(void) {
  char *_ast_generate_def_code_3 = NULL;
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

  code = (generate_def_code("NoDef", &sf, &_ast_generate_def_code_3),
          _ast_generate_def_code_3);
  ASSERT(code != NULL);

  /* Should just be calloc */
  ASSERT(strstr(code, "calloc(1, sizeof(**out))"));
  ASSERT(strstr(code, "(*out)->x = ") == NULL);

  free(code);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_default_nullptr
 * @return TEST
 */
TEST test_default_nullptr(void) {
  char *_ast_generate_def_code_4 = NULL;
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "ptr_val", "integer", NULL, "nullptr",
                    NULL); /* treated as raw */
  struct_fields_add(&sf, "str_ptr", "string", NULL, "nullptr", NULL);

  code = (generate_def_code("PtrStruct", &sf, &_ast_generate_def_code_4),
          _ast_generate_def_code_4);
  ASSERT(code != NULL);

  /* Should map nullptr literal to NULL */
  ASSERT(strstr(code, "(*out)->ptr_val = NULL;"));
  ASSERT(strstr(code, "(*out)->str_ptr = NULL;"));

  free(code);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_default_binary_literal
 * @return TEST
 */
TEST test_default_binary_literal(void) {
  char *_ast_generate_def_code_5 = NULL;
  struct StructFields sf;
  char *code;

  struct_fields_init(&sf);
  /* 0b101 -> 5 */
  struct_fields_add(&sf, "bin_val", "integer", NULL, "0b101", NULL);
  /* 0B11 -> 3 */
  struct_fields_add(&sf, "bin_cap", "integer", NULL, "0B11", NULL);

  code = (generate_def_code("BinStruct", &sf, &_ast_generate_def_code_5),
          _ast_generate_def_code_5);
  ASSERT(code != NULL);

  /* Should emit decimal values for C89 compatibility */
  /* Note: Assumes numeric_parser available and linked */
  /* Verify conversion */
  ASSERT(strstr(code, "(*out)->bin_val = 5;"));
  ASSERT(strstr(code, "(*out)->bin_cap = 3;"));

  free(code);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief codegen_defaults_suite
 */

TEST test_write_forward_decl_bounds(void) {
  FILE *tmp = tmpfile();
  ASSERT(tmp);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_forward_decl(NULL, "X"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_forward_decl(tmp, NULL));
  ASSERT_EQ(0, write_forward_decl(tmp, "X"));
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_write_forward_decl_io_fail(void) {
  FILE *tmp = tmpfile();
  int rc;
  (void)rc;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT(tmp);
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO, write_forward_decl(tmp, "X"));
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_write_enum_declaration_h_io_fail(void) {
  struct StructFields sf;
  struct CodegenConfig cfg;
  FILE *tmp = tmpfile();
  int rc;
  (void)rc;
  g_fail_io_after = 0;
  g_io_calls = 0;
  memset(&cfg, 0, sizeof(cfg));
  struct_fields_init(&sf);

  ASSERT(tmp);
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  rc = write_enum_declaration_h(tmp, "E", &sf, &cfg);
  printf("RC WAS %d\n", rc);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  fclose(tmp);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

TEST test_write_struct_declaration_h_io_fail(void) {
  struct StructFields sf;
  struct CodegenConfig cfg;
  FILE *tmp = tmpfile();
  int rc;
  (void)rc;
  g_fail_io_after = 0;
  g_io_calls = 0;
  memset(&cfg, 0, sizeof(cfg));
  struct_fields_init(&sf);

  ASSERT(tmp);
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO, write_struct_declaration_h(tmp, "S", &sf, &cfg));
  fclose(tmp);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

TEST test_write_union_declaration_h_io_fail(void) {
  struct StructFields sf;
  struct CodegenConfig cfg;
  FILE *tmp = tmpfile();
  int rc;
  (void)rc;
  g_fail_io_after = 0;
  g_io_calls = 0;
  memset(&cfg, 0, sizeof(cfg));
  struct_fields_init(&sf);

  ASSERT(tmp);
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  rc = write_union_declaration_h(tmp, "U", &sf, &cfg);
  printf("RC WAS %d\n", rc);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  fclose(tmp);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

TEST test_codegen_h_bounds(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  struct CodegenConfig cfg;
  memset(&cfg, 0, sizeof(cfg));
  struct_fields_init(&sf);

  ASSERT(tmp);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_enum_declaration_h(NULL, "E", &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_enum_declaration_h(tmp, NULL, &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_enum_declaration_h(tmp, "E", NULL, &cfg));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_declaration_h(NULL, "U", &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_declaration_h(tmp, NULL, &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_declaration_h(tmp, "U", NULL, &cfg));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_declaration_h(NULL, "S", &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_declaration_h(tmp, NULL, &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_declaration_h(tmp, "S", NULL, &cfg));

  fclose(tmp);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

SUITE(codegen_defaults_suite) {
  RUN_TEST(test_default_primitive);
  RUN_TEST(test_default_string);
  RUN_TEST(test_default_enum);
  RUN_TEST(test_default_no_defaults);
  RUN_TEST(test_default_nullptr);
  RUN_TEST(test_default_binary_literal);
  RUN_TEST(test_write_forward_decl_bounds);
  RUN_TEST(test_write_forward_decl_io_fail);
  RUN_TEST(test_write_enum_declaration_h_io_fail);
  RUN_TEST(test_write_struct_declaration_h_io_fail);
  RUN_TEST(test_write_union_declaration_h_io_fail);
  RUN_TEST(test_codegen_h_bounds);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_DEFAULTS_H */

/* LCOV_EXCL_STOP */
