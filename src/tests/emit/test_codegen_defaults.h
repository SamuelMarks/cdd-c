/**
 * @file test_codegen_defaults.h
 * @brief Unit tests and mocks in test_codegen_defaults.h.
 */
#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
#ifndef TEST_CODEGEN_DEFAULTS_H
/**
 * @brief Macro TEST_CODEGEN_DEFAULTS_H.
 */
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
#include "c_cdd/memory.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */
/** @brief Test definition g_io_calls. */
extern C_CDD_EXPORT int g_io_calls;
/** @brief Test definition g_fail_io_after. */
extern C_CDD_EXPORT int g_fail_io_after;

#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
/* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
static FILE *mock_tmpfile_def(void) {
  if (g_fail_io_after >= 0 && ++g_io_calls == g_fail_io_after)
    return NULL;
  return cdd_test_tmpfile_global();
}
static long mock_ftell_def(FILE *stream) {
  if (g_fail_io_after == 999)
    return 0;
  return ftell(stream);
}
static size_t mock_fread_def(void *ptr, size_t size, size_t nitems,
                             FILE *stream) {
  if (g_fail_io_after == 998)
    return 0;
  return fread(ptr, size, nitems, stream);
}
#define TMPFILE mock_tmpfile_def
#define FTELL mock_ftell_def
#define FREAD mock_fread_def
#else
/**
 * @brief Macro TMPFILE.
 */
#define TMPFILE tmpfile
/**
 * @brief Macro FTELL.
 */
#define FTELL ftell
/**
 * @brief Macro FREAD.
 */
#define FREAD fread
#endif

#ifdef _WIN32
#else
#endif

/* Helper to generate code and return as string buffer */
/**
 * @brief Helper function generate_def_code.
 * @param[in] struct_name Parameter struct_name.
 * @param[in,out] sf Parameter sf.
 * @param[in,out] _out_val Parameter _out_val.
 * @return Result status or value.
 */
static cdd_c_error_t generate_def_code(const char *struct_name,
                                       struct StructFields *sf,
                                       char **_out_val) {
  FILE *tmp = TMPFILE();
  long sz;
  char *content = NULL;

  if (!tmp) {
    *_out_val = NULL;
    return 0;
  }

  if (write_struct_default_func(tmp, struct_name, sf, NULL) != 0) {
    if (tmp)
      fclose(tmp);
    {
      *_out_val = NULL;
      return 0;
    }
  }

  fseek(tmp, 0, SEEK_END);
  sz = FTELL(tmp);
  rewind(tmp);

  content = (char *)C_CDD_CALLOC(1, (size_t)sz + 1);
  if (sz > 0)
    FREAD(content, 1, (size_t)sz, tmp);

  if (tmp)
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

  ASSERT(strstr(code, "cdd_c_error_t Prim_default(struct Prim **out)"));
  ASSERT(strstr(code, "(*out)->x = 42;"));
  ASSERT(strstr(code, "(*out)->flag = 1;"));

  C_CDD_FREE(code);
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

  C_CDD_FREE(code);
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
  ASSERT(strstr(code, "if (rc != CDD_C_SUCCESS) { EnumStruct_cleanup(*out); "
                      "*out=NULL; return rc; }"));

  C_CDD_FREE(code);
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

  C_CDD_FREE(code);
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

  C_CDD_FREE(code);
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
  ASSERT(strstr(code, "(*out)->bin_val = 0x0000000000000005ULL;"));
  ASSERT(strstr(code, "(*out)->bin_cap = 0x0000000000000003ULL;"));

  C_CDD_FREE(code);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief codegen_defaults_suite
 * @return Test or helper result.
 */

TEST test_write_forward_decl_bounds(void) {
  FILE *tmp = TMPFILE();
  ASSERT(tmp);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_forward_decl(NULL, "X"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_forward_decl(tmp, NULL));
  ASSERT_EQ(0, write_forward_decl(tmp, "X"));
  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_write_forward_decl_io_fail.
 * @return Greatest test result.
 */
TEST test_write_forward_decl_io_fail(void) {
  FILE *tmp = TMPFILE();
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT(tmp);
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO, write_forward_decl(tmp, "X"));
  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_write_enum_declaration_h_io_fail.
 * @return Greatest test result.
 */
TEST test_write_enum_declaration_h_io_fail(void) {
  struct StructFields sf;
  struct CodegenConfig cfg;
  FILE *tmp = TMPFILE();
  int rc = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  memset(&cfg, 0, sizeof(cfg));
  struct_fields_init(&sf);
  enum_members_add(&sf.enum_members, "M1");
  enum_members_add(&sf.enum_members, "UNKNOWN");
  sf.enum_members.members[sf.enum_members.size] = NULL;
  sf.enum_members
      .size++; /* Intentionally leave one member NULL to test !member branch */
  cfg.enum_guard = cdd_test_mut_str("MY_GUARD");

  ASSERT(tmp);
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  {
    int j;
    for (j = 0; j < 20; j++) {
      g_fail_io_after = j;
      g_io_calls = 0;
      rc = write_enum_declaration_h(tmp, "E", &sf, &cfg);
      if (rc == 0)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
  }

  g_fail_io_after = -1;
  ASSERT_EQ(0, write_enum_declaration_h(tmp, "E", &sf, NULL));
  cfg.enum_guard = NULL;
  ASSERT_EQ(0, write_enum_declaration_h(tmp, "E", &sf, &cfg));

  if (tmp)
    fclose(tmp);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_write_struct_declaration_h_io_fail.
 * @return Greatest test result.
 */
TEST test_write_struct_declaration_h_io_fail(void) {
  struct StructFields sf;
  struct CodegenConfig cfg;
  FILE *tmp = TMPFILE();
  int rc = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  memset(&cfg, 0, sizeof(cfg));
  struct_fields_init(&sf);
  struct_fields_add(&sf, "s", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "i", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "n", "number", NULL, NULL, NULL);
  struct_fields_add(&sf, "b", "boolean", NULL, NULL, NULL);
  struct_fields_add(&sf, "e", "enum", "E", NULL, NULL);
  struct_fields_add(&sf, "r", "object", "R", NULL, NULL);
  struct_fields_add(&sf, "a_s", "array", "string", NULL, NULL);
  struct_fields_add(&sf, "a_i", "array", "integer", NULL, NULL);
  struct_fields_add(&sf, "a_n", "array", "number", NULL, NULL);
  struct_fields_add(&sf, "a_b", "array", "boolean", NULL, NULL);
  struct_fields_add(&sf, "a_r", "array", "R", NULL, NULL);
  struct_fields_add(&sf, "v", "void", NULL, NULL, NULL);
  struct_fields_add(&sf, "fa_s", "array", "string", NULL, NULL);
  sf.fields[sf.size - 1].fixed_array_len = 4;
  struct_fields_add(&sf, "fa_i", "array", "integer", NULL, NULL);
  sf.fields[sf.size - 1].fixed_array_len = 4;
  struct_fields_add(&sf, "fa_b", "array", "boolean", NULL, NULL);
  sf.fields[sf.size - 1].fixed_array_len = 4;
  struct_fields_add(&sf, "fa_n", "array", "number", NULL, NULL);
  sf.fields[sf.size - 1].fixed_array_len = 4;
  struct_fields_add(&sf, "fa_r", "array", "R", NULL, NULL);
  sf.fields[sf.size - 1].fixed_array_len = 4;
  struct_fields_add(&sf, "fs", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].fixed_array_len = 16;
  struct_fields_add(&sf, "bi", "integer", NULL, NULL, "4");
  struct_fields_add(&sf, "bb", "boolean", NULL, NULL, "1");
  cfg.json_guard = cdd_test_mut_str("JSON_G");
  cfg.utils_guard = cdd_test_mut_str("UTILS_G");

  ASSERT(tmp);
  {
    int j;
    for (j = 0; j < 250; j++) {
      g_fail_io_after = j;
      g_io_calls = 0;
      rc = write_struct_declaration_h(tmp, "S", &sf, &cfg);
      if (rc == 0)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
  }

  /* Empty struct test */
  {
    struct StructFields empty_sf;
    int j;
    struct_fields_init(&empty_sf);
    for (j = 1; j < 20; j++) {
      g_fail_io_after = j;
      g_io_calls = 0;
      rc = write_struct_declaration_h(tmp, "Empty", &empty_sf, &cfg);
      if (rc == 0)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    struct_fields_free(&empty_sf);
  }

  if (tmp)
    fclose(tmp);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_write_union_declaration_h_io_fail.
 * @return Greatest test result.
 */
TEST test_write_union_declaration_h_io_fail(void) {
  struct StructFields sf;
  struct CodegenConfig cfg;
  FILE *tmp = TMPFILE();
  int rc = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  memset(&cfg, 0, sizeof(cfg));
  struct_fields_init(&sf);
  struct_fields_add(&sf, "s", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "i", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "n", "number", NULL, NULL, NULL);
  struct_fields_add(&sf, "b", "boolean", NULL, NULL, NULL);
  struct_fields_add(&sf, "e", "enum", "E", NULL, NULL);
  struct_fields_add(&sf, "r", "object", "R", NULL, NULL);
  struct_fields_add(&sf, "a_s", "array", "string", NULL, NULL);
  struct_fields_add(&sf, "a_i", "array", "integer", NULL, NULL);
  struct_fields_add(&sf, "a_n", "array", "number", NULL, NULL);
  struct_fields_add(&sf, "a_b", "array", "boolean", NULL, NULL);
  struct_fields_add(&sf, "a_r", "array", "R", NULL, NULL);
  struct_fields_add(&sf, "v", "void", NULL, NULL, NULL);
  cfg.json_guard = cdd_test_mut_str("JSON_G");
  cfg.utils_guard = cdd_test_mut_str("UTILS_G");

  ASSERT(tmp);
  {
    int j;
    for (j = 0; j < 200; j++) {
      g_fail_io_after = j;
      g_io_calls = 0;
      rc = write_union_declaration_h(tmp, "U", &sf, &cfg);
      if (rc == 0)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
  }
  if (tmp)
    fclose(tmp);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_codegen_h_bounds.
 * @return Greatest test result.
 */
TEST test_codegen_h_bounds(void) {
  FILE *tmp = TMPFILE();
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

  if (tmp)
    fclose(tmp);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_def_errors.
 * @return Greatest test result.
 */
TEST test_def_errors(void) {
  char *_out = NULL;
  struct StructFields sf;
  struct_fields_init(&sf);
  struct_fields_add(&sf, "x", "integer", NULL, "42", NULL);

  g_io_calls = 0;
  g_fail_io_after = 1; /* TMPFILE fails */
  ASSERT_EQ(0, generate_def_code("Prim", &sf, &_out));
  ASSERT_EQ(NULL, _out);

  g_io_calls = 0;
  g_fail_io_after = 2; /* write_struct_default_func fails */
  ASSERT_EQ(0, generate_def_code("Prim", &sf, &_out));
  ASSERT_EQ(NULL, _out);

  g_io_calls = 0;
  g_fail_io_after = 998; /* FREAD returns 0 */
  ASSERT_EQ(0, generate_def_code("Prim", &sf, &_out));
  if (_out)
    C_CDD_FREE(_out);

  g_io_calls = 0;
  g_fail_io_after = 999; /* FTELL returns 0 */
  ASSERT_EQ(0, generate_def_code("Prim", &sf, &_out));
  ASSERT_NEQ(NULL, _out);
  ASSERT_EQ(0, strlen(_out));
  C_CDD_FREE(_out);

  g_fail_io_after = -1;
  struct_fields_free(&sf);
  PASS();
}

/**
 * @brief Exhaustive test for Serde and CLI comment emission in codegen.c.
 * @return greatest_test_res
 */
TEST test_codegen_serde_cli_exhaustive(void) {
  FILE *tmp = TMPFILE();
  struct StructFields sf;
  struct CodegenConfig cfg;
  int j, rc;

  ASSERT(tmp);
  memset(&cfg, 0, sizeof(cfg));

  /* 1. Enum with all container and variant attributes */
  struct_fields_init(&sf);
  enum_members_add(&sf.enum_members, "M1");
  enum_members_add(&sf.enum_members, "M2");
  enum_members_add(&sf.enum_members, "M3");
  enum_members_add(&sf.enum_members, "M4");

  CDD_STRCPY(sf.serde_rename, sizeof(sf.serde_rename), "EnumRen");
  sf.enum_members.serde_rename_all = CDD_SERDE_CASE_SNAKECASE;
  CDD_STRCPY(sf.enum_members.serde_replace_from,
             sizeof(sf.enum_members.serde_replace_from), "A");
  CDD_STRCPY(sf.enum_members.serde_replace_to,
             sizeof(sf.enum_members.serde_replace_to), "B");
  CDD_STRCPY(sf.enum_members.serde_tag, sizeof(sf.enum_members.serde_tag), "t");
  CDD_STRCPY(sf.enum_members.serde_content,
             sizeof(sf.enum_members.serde_content), "c");
  sf.enum_members.serde_untagged = 1;

  sf.enum_members.serde_names[0] = strdup("m1_ren");
  enum_member_add_alias(&sf.enum_members, 0, "m1_al");
  sf.enum_members.serde_skip[0] = 1;
  sf.enum_members.serde_other_idx = 0;

  /* M2: alias first, then skip */
  enum_member_add_alias(&sf.enum_members, 1, "m2_al");
  sf.enum_members.serde_skip[1] = 1;

  /* M3: skip first */
  sf.enum_members.serde_skip[2] = 1;

  /* M4: other first (need another enum where serde_other_idx == 3) */
  for (j = 0; j < 60; ++j) {
    g_fail_io_after = j;
    g_io_calls = 0;
    rc = write_enum_declaration_h(tmp, "RichEnum", &sf, &cfg);
    if (rc == 0)
      break;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }
  g_fail_io_after = -1;

  /* Enum where first_attr is false/true for each attribute and other is first
   */
  sf.serde_rename[0] = '\0';
  sf.enum_members.serde_other_idx = 3;
  ASSERT_EQ(0, write_enum_declaration_h(tmp, "RichEnum2", &sf, &cfg));
  sf.enum_members.serde_rename_all = CDD_SERDE_CASE_NONE;
  ASSERT_EQ(0, write_enum_declaration_h(tmp, "RichEnum3", &sf, &cfg));
  sf.enum_members.serde_replace_from[0] = '\0';
  ASSERT_EQ(0, write_enum_declaration_h(tmp, "RichEnum4", &sf, &cfg));
  sf.enum_members.serde_tag[0] = '\0';
  ASSERT_EQ(0, write_enum_declaration_h(tmp, "RichEnum5", &sf, &cfg));
  sf.enum_members.serde_content[0] = '\0';
  ASSERT_EQ(0, write_enum_declaration_h(tmp, "RichEnum6", &sf, &cfg));

  /* Also test sf-level fallbacks for enum */
  sf.enum_members.serde_untagged = 0;
  sf.serde_rename_all = CDD_SERDE_CASE_CAMELCASE;
  CDD_STRCPY(sf.serde_replace_from, sizeof(sf.serde_replace_from), "X");
  CDD_STRCPY(sf.serde_replace_to, sizeof(sf.serde_replace_to), "Y");
  CDD_STRCPY(sf.serde_tag, sizeof(sf.serde_tag), "st");
  CDD_STRCPY(sf.serde_content, sizeof(sf.serde_content), "sc");
  sf.serde_untagged = 1;
  /* Test NULL serde_names / serde_skip / n_serde_aliases pointers */
  {
    char **saved_names = sf.enum_members.serde_names;
    int *saved_skip = sf.enum_members.serde_skip;
    size_t *saved_nal = sf.enum_members.n_serde_aliases;
    sf.enum_members.serde_names = NULL;
    sf.enum_members.serde_skip = NULL;
    sf.enum_members.n_serde_aliases = NULL;
    ASSERT_EQ(0, write_enum_declaration_h(tmp, "RichEnumFallback", &sf, &cfg));
    sf.enum_members.serde_names = saved_names;
    sf.enum_members.serde_skip = saved_skip;
    sf.enum_members.n_serde_aliases = saved_nal;
  }
  struct_fields_free(&sf);

  /* 2. Union with all attributes and failure paths */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "u_s", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "u_e", "enum", "#/definitions/UE", NULL, NULL);
  struct_fields_add(&sf, "u_o", "object", "#/definitions/UO", NULL, NULL);
  struct_fields_add(&sf, "u_ar", "array", "#/definitions/UA", NULL, NULL);
  CDD_STRCPY(sf.serde_rename, sizeof(sf.serde_rename), "UnionRen");
  CDD_STRCPY(sf.serde_tag, sizeof(sf.serde_tag), "utag");
  CDD_STRCPY(sf.serde_content, sizeof(sf.serde_content), "ucontent");
  sf.serde_untagged = 1;

  for (j = 0; j < 60; ++j) {
    g_fail_io_after = j;
    g_io_calls = 0;
    rc = write_union_declaration_h(tmp, "RichUnion", &sf, NULL);
    if (rc == 0)
      break;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }
  g_fail_io_after = -1;

  /* Union first_attr permutations */
  sf.serde_rename[0] = '\0';
  ASSERT_EQ(0, write_union_declaration_h(tmp, "RichUnion2", &sf, &cfg));
  sf.serde_tag[0] = '\0';
  sf.union_discriminator = strdup("udisc");
  ASSERT_EQ(0, write_union_declaration_h(tmp, "RichUnion3", &sf, &cfg));
  sf.union_discriminator[0] = '\0';
  ASSERT_EQ(0, write_union_declaration_h(tmp, "RichUnion4", &sf, &cfg));
  C_CDD_FREE(sf.union_discriminator);
  sf.union_discriminator = NULL;
  sf.serde_content[0] = '\0';
  ASSERT_EQ(0, write_union_declaration_h(tmp, "RichUnion5", &sf, &cfg));

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_str_after_last;
    g_cdd_fail_str_after_last = 1;
    ASSERT_NEQ(0, write_union_declaration_h(tmp, "FailU1", &sf, NULL));
    g_cdd_fail_str_after_last = 2;
    ASSERT_NEQ(0, write_union_declaration_h(tmp, "FailU2", &sf, NULL));
    g_cdd_fail_str_after_last = 3;
    ASSERT_NEQ(0, write_union_declaration_h(tmp, "FailU3", &sf, NULL));
    g_cdd_fail_str_after_last = 0;
  }
#endif
  struct_fields_free(&sf);

  /* 3. Struct with all container and field Serde + CLI attributes */
  struct_fields_init(&sf);
  CDD_STRCPY(sf.serde_rename, sizeof(sf.serde_rename), "StructRen");
  sf.serde_rename_all = CDD_SERDE_CASE_SNAKECASE;
  sf.serde_rename_all_ser = CDD_SERDE_CASE_CAMELCASE;
  sf.serde_rename_all_de = CDD_SERDE_CASE_PASCALCASE;
  CDD_STRCPY(sf.serde_replace_from, sizeof(sf.serde_replace_from), "old");
  CDD_STRCPY(sf.serde_replace_to, sizeof(sf.serde_replace_to), "new");
  CDD_STRCPY(sf.serde_prefix, sizeof(sf.serde_prefix), "pre_");
  CDD_STRCPY(sf.serde_suffix, sizeof(sf.serde_suffix), "_suf");
  sf.serde_deny_unknown_fields = 1;
  sf.serde_default = 1;
  CDD_STRCPY(sf.serde_tag, sizeof(sf.serde_tag), "stag");
  CDD_STRCPY(sf.serde_content, sizeof(sf.serde_content), "scont");
  sf.serde_untagged = 1;
  sf.is_cli_command = 1;
  CDD_STRCPY(sf.cli_name, sizeof(sf.cli_name), "mycli");
  CDD_STRCPY(sf.cli_version, sizeof(sf.cli_version), "1.0");
  CDD_STRCPY(sf.cli_author, sizeof(sf.cli_author), "Auth");
  CDD_STRCPY(sf.cli_about, sizeof(sf.cli_about), "About");
  sf.cli_rename_all = CDD_SERDE_CASE_KEBABCASE;
  CDD_STRCPY(sf.cli_env_prefix, sizeof(sf.cli_env_prefix), "APP_");

  /* Field 0: array with rich serde + cli attributes */
  struct_fields_add(&sf, "f_arr", "array", "#/definitions/SubItem", NULL, NULL);
  CDD_STRCPY(sf.fields[0].serde_rename, sizeof(sf.fields[0].serde_rename),
             "f0_ren");
  CDD_STRCPY(sf.fields[0].serde_rename_ser,
             sizeof(sf.fields[0].serde_rename_ser), "f0_ser");
  CDD_STRCPY(sf.fields[0].serde_rename_de, sizeof(sf.fields[0].serde_rename_de),
             "f0_de");
  sf.fields[0].serde_case = CDD_SERDE_CASE_SNAKECASE;
  CDD_STRCPY(sf.fields[0].serde_replace_from,
             sizeof(sf.fields[0].serde_replace_from), "a");
  CDD_STRCPY(sf.fields[0].serde_replace_to,
             sizeof(sf.fields[0].serde_replace_to), "b");
  struct_field_add_alias(&sf.fields[0], "f0_al");
  sf.fields[0].serde_skip = 1;
  sf.fields[0].serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
  sf.fields[0].serde_flatten = 1;
  sf.fields[0].serde_has_default = 1;
  CDD_STRCPY(sf.fields[0].serde_default_fn,
             sizeof(sf.fields[0].serde_default_fn), "f0_def_fn");
  CDD_STRCPY(sf.fields[0].serde_with, sizeof(sf.fields[0].serde_with),
             "f0_with");
  CDD_STRCPY(sf.fields[0].serde_serialize_with,
             sizeof(sf.fields[0].serde_serialize_with), "f0_ser_w");
  CDD_STRCPY(sf.fields[0].serde_deserialize_with,
             sizeof(sf.fields[0].serde_deserialize_with), "f0_de_w");
  sf.fields[0].cli_short = 'a';
  CDD_STRCPY(sf.fields[0].cli_long, sizeof(sf.fields[0].cli_long), "arr");
  sf.fields[0].cli_negatable = 1;
  sf.fields[0].cli_positional = 1;
  sf.fields[0].cli_index = 0;
  CDD_STRCPY(sf.fields[0].cli_env, sizeof(sf.fields[0].cli_env), "ENV_ARR");
  CDD_STRCPY(sf.fields[0].cli_value_name, sizeof(sf.fields[0].cli_value_name),
             "VAL");
  CDD_STRCPY(sf.fields[0].cli_help, sizeof(sf.fields[0].cli_help), "Help text");
  sf.fields[0].cli_action = CDD_CLI_ACTION_APPEND;
  sf.fields[0].cli_value_delimiter = ',';
  sf.fields[0].cli_subcommand = 1;
  struct_field_add_cli_conflict(&sf.fields[0], "other_flag");
  struct_field_add_cli_requires(&sf.fields[0], "req_flag");

  /* Field 1: skip_serializing + skip_deserializing + skip_if_zero + default_val
   */
  struct_fields_add(&sf, "f1", "integer", NULL, "10", NULL);
  sf.fields[1].serde_skip_serializing = 1;
  sf.fields[1].serde_skip_deserializing = 1;
  sf.fields[1].serde_skip_if_kind = CDD_SERDE_SKIP_IF_ZERO;
  sf.fields[1].serde_has_default = 1;

  /* Field 2: write_only + read_only + skip_if_func + bare default */
  struct_fields_add(&sf, "f2", "string", NULL, NULL, NULL);
  sf.fields[2].write_only = 1;
  sf.fields[2].read_only = 1;
  sf.fields[2].serde_skip_if_kind = CDD_SERDE_SKIP_IF_FUNC;
  CDD_STRCPY(sf.fields[2].serde_skip_if_fn,
             sizeof(sf.fields[2].serde_skip_if_fn), "my_skip_fn");
  sf.fields[2].serde_has_default = 1;

  /* Field 3: skip_if_null + skip_if_func with empty fn */
  struct_fields_add(&sf, "f3", "string", NULL, NULL, NULL);
  sf.fields[3].serde_skip_if_kind = CDD_SERDE_SKIP_IF_FUNC;

  /* Field 4: enum and object fields for get_type_from_ref */
  struct_fields_add(&sf, "f_enum", "enum", "#/definitions/SE", NULL, NULL);
  struct_fields_add(&sf, "f_obj", "object", "#/definitions/SO", NULL, NULL);

  cfg.cli_guard = cdd_test_mut_str("CLI_G");
  cfg.emit_cli = 1;

  for (j = 0; j < 250; ++j) {
    g_fail_io_after = j;
    g_io_calls = 0;
    rc = write_struct_declaration_h(tmp, "RichStruct", &sf, &cfg);
    if (rc == 0)
      break;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }
  g_fail_io_after = -1;

  /* Test config == NULL when sf.is_cli_command == 1 */
  ASSERT_EQ(0, write_struct_declaration_h(tmp, "RichStructNoCfg", &sf, NULL));

  /* Test config != NULL && config->cli_guard == NULL when sf.is_cli_command ==
   * 1, and config != NULL && config->emit_cli == 1 when sf.is_cli_command == 0
   */
  cfg.cli_guard = NULL;
  ASSERT_EQ(0,
            write_struct_declaration_h(tmp, "RichStructNoCliGuard", &sf, &cfg));
  sf.is_cli_command = 0;
  cfg.emit_cli = 1;
  ASSERT_EQ(0, write_struct_declaration_h(tmp, "RichStructEmitCli", &sf, &cfg));
  sf.is_cli_command = 1;

  /* Test IO failure inside write_enum_declaration_h when serde_skip[0] == 0 and
   * serde_names != NULL, when v_n_al > 0 only, and when v_other is first */
  {
    struct StructFields sf_e;
    struct_fields_init(&sf_e);
    enum_members_add(&sf_e.enum_members, "V1");
    enum_members_add(&sf_e.enum_members, "V2");
    enum_member_add_alias(&sf_e.enum_members, 0, "v1_al_only");
    sf_e.enum_members.serde_other_idx = 1;
    for (j = 0; j < 30; ++j) {
      g_fail_io_after = j;
      g_io_calls = 0;
      rc = write_enum_declaration_h(tmp, "E_OtherFirst", &sf_e, NULL);
      if (rc == 0)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    g_fail_io_after = -1;
    struct_fields_free(&sf_e);
  }

  /* Test IO failure on field with serde_skip=0, default_val, bare default,
   * cli_short first, and container cli_name first */
  {
    struct StructFields sf_io2;
    struct_fields_init(&sf_io2);
    sf_io2.is_cli_command = 1;
    struct_fields_add(&sf_io2, "f_defval", "string", NULL, "dv", NULL);
    sf_io2.fields[0].serde_has_default = 1;
    sf_io2.fields[0].cli_short = 's';
    struct_fields_add(&sf_io2, "f_baredef", "integer", NULL, NULL, NULL);
    sf_io2.fields[1].serde_has_default = 1;
    struct_fields_add(&sf_io2, "f_clifirst", "integer", NULL, NULL, NULL);
    sf_io2.fields[2].cli_short = 'c';
    struct_fields_add(&sf_io2, "f_skip_if_null", "string", NULL, NULL, NULL);
    sf_io2.fields[3].serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
    struct_fields_add(&sf_io2, "f_ren_skipser", "string", NULL, NULL, NULL);
    CDD_STRCPY(sf_io2.fields[4].serde_rename,
               sizeof(sf_io2.fields[4].serde_rename), "r_ss");
    sf_io2.fields[4].serde_skip_serializing = 1;
    struct_fields_add(&sf_io2, "f_deffn_first", "integer", NULL, NULL, NULL);
    sf_io2.fields[5].serde_has_default = 1;
    CDD_STRCPY(sf_io2.fields[5].serde_default_fn,
               sizeof(sf_io2.fields[5].serde_default_fn), "dff");
    struct_fields_add(&sf_io2, "f_cliskip", "integer", NULL, NULL, NULL);
    sf_io2.fields[6].cli_skip = 1;
    struct_fields_add(&sf_io2, "f_cliskip_tf", "integer", NULL, NULL, NULL);
    sf_io2.fields[7].cli_skip_to_cli = 1;
    sf_io2.fields[7].cli_skip_from_cli = 1;
    for (j = 0; j < 100; ++j) {
      g_fail_io_after = j;
      g_io_calls = 0;
      rc = write_struct_declaration_h(tmp, "S_Io2", &sf_io2, NULL);
      if (rc == 0)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    g_fail_io_after = -1;
    struct_fields_free(&sf_io2);
  }

  /* Test array field serde comment IO failure when container has no comment */
  {
    struct StructFields sf_arr_io;
    struct_fields_init(&sf_arr_io);
    struct_fields_add(&sf_arr_io, "arr", "array", "string", NULL, NULL);
    sf_arr_io.fields[0].serde_skip = 1;
    for (j = 0; j < 10; ++j) {
      g_fail_io_after = j;
      g_io_calls = 0;
      rc = write_struct_declaration_h(tmp, "ArrIo", &sf_arr_io, NULL);
      if (rc == 0)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    g_fail_io_after = -1;
    struct_fields_free(&sf_arr_io);
  }

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_str_after_last;
    g_cdd_fail_str_after_last = 1;
    ASSERT_NEQ(0, write_struct_declaration_h(tmp, "FailS1", &sf, &cfg));
    g_cdd_fail_str_after_last = 2;
    ASSERT_NEQ(0, write_struct_declaration_h(tmp, "FailS2", &sf, &cfg));
    g_cdd_fail_str_after_last = 3;
    ASSERT_NEQ(0, write_struct_declaration_h(tmp, "FailS3", &sf, &cfg));
    g_cdd_fail_str_after_last = 0;
  }
#endif
  struct_fields_free(&sf);

  /* 4. Test each container attribute and field attribute as the FIRST attribute
   * (to cover both branches of `first_attr ? "" : ", "`, `f_first ? "" : ", "`,
   * `c_first ? "" : ", "`, and `has_cli` conditions) */
  {
    int step;
    for (step = 0; step < 30; ++step) {
      struct StructFields s_perm;
      struct_fields_init(&s_perm);
      struct_fields_add(&s_perm, "x", "integer", NULL, NULL, NULL);
      switch (step) {
      case 0:
        s_perm.serde_rename_all = CDD_SERDE_CASE_SNAKECASE;
        CDD_STRCPY(s_perm.fields[0].serde_rename_ser,
                   sizeof(s_perm.fields[0].serde_rename_ser), "rs");
        break;
      case 1:
        s_perm.serde_rename_all_ser = CDD_SERDE_CASE_SNAKECASE;
        CDD_STRCPY(s_perm.fields[0].serde_rename_de,
                   sizeof(s_perm.fields[0].serde_rename_de), "rd");
        break;
      case 2:
        s_perm.serde_rename_all_de = CDD_SERDE_CASE_SNAKECASE;
        s_perm.fields[0].serde_case = CDD_SERDE_CASE_SNAKECASE;
        break;
      case 3:
        CDD_STRCPY(s_perm.serde_replace_from, sizeof(s_perm.serde_replace_from),
                   "a");
        CDD_STRCPY(s_perm.fields[0].serde_replace_from,
                   sizeof(s_perm.fields[0].serde_replace_from), "x");
        break;
      case 4:
        CDD_STRCPY(s_perm.serde_prefix, sizeof(s_perm.serde_prefix), "p");
        struct_field_add_alias(&s_perm.fields[0], "al");
        break;
      case 5:
        CDD_STRCPY(s_perm.serde_suffix, sizeof(s_perm.serde_suffix), "s");
        s_perm.fields[0].serde_skip = 1;
        break;
      case 6:
        s_perm.serde_deny_unknown_fields = 1;
        s_perm.fields[0].serde_skip_serializing = 1;
        break;
      case 7:
        s_perm.serde_default = 1;
        s_perm.fields[0].serde_skip_deserializing = 1;
        break;
      case 8:
        CDD_STRCPY(s_perm.serde_tag, sizeof(s_perm.serde_tag), "t");
        s_perm.fields[0].serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
        break;
      case 9:
        CDD_STRCPY(s_perm.serde_content, sizeof(s_perm.serde_content), "c");
        s_perm.fields[0].serde_flatten = 1;
        break;
      case 10:
        s_perm.serde_untagged = 1;
        s_perm.fields[0].serde_has_default = 1;
        break;
      case 11:
        s_perm.is_cli_command = 1;
        CDD_STRCPY(s_perm.fields[0].serde_with,
                   sizeof(s_perm.fields[0].serde_with), "w");
        break;
      case 12:
        CDD_STRCPY(s_perm.cli_name, sizeof(s_perm.cli_name), "cn");
        CDD_STRCPY(s_perm.fields[0].serde_serialize_with,
                   sizeof(s_perm.fields[0].serde_serialize_with), "sw");
        break;
      case 13:
        CDD_STRCPY(s_perm.cli_version, sizeof(s_perm.cli_version), "cv");
        CDD_STRCPY(s_perm.fields[0].serde_deserialize_with,
                   sizeof(s_perm.fields[0].serde_deserialize_with), "dw");
        break;
      case 14:
        CDD_STRCPY(s_perm.cli_author, sizeof(s_perm.cli_author), "ca");
        CDD_STRCPY(s_perm.fields[0].cli_long, sizeof(s_perm.fields[0].cli_long),
                   "cl");
        break;
      case 15:
        CDD_STRCPY(s_perm.cli_about, sizeof(s_perm.cli_about), "cab");
        s_perm.fields[0].cli_negatable = 1;
        break;
      case 16:
        s_perm.cli_rename_all = CDD_SERDE_CASE_KEBABCASE;
        s_perm.fields[0].cli_positional = 1;
        break;
      case 17:
        CDD_STRCPY(s_perm.cli_env_prefix, sizeof(s_perm.cli_env_prefix), "CEP");
        s_perm.fields[0].cli_index = 1;
        break;
      case 18:
        CDD_STRCPY(s_perm.fields[0].cli_env, sizeof(s_perm.fields[0].cli_env),
                   "CE");
        break;
      case 19:
        CDD_STRCPY(s_perm.fields[0].cli_value_name,
                   sizeof(s_perm.fields[0].cli_value_name), "CVN");
        break;
      case 20:
        CDD_STRCPY(s_perm.fields[0].cli_help, sizeof(s_perm.fields[0].cli_help),
                   "CH");
        break;
      case 21:
        s_perm.fields[0].cli_action = CDD_CLI_ACTION_COUNT;
        break;
      case 22:
        s_perm.fields[0].cli_value_delimiter = ':';
        break;
      case 23:
        s_perm.fields[0].cli_subcommand = 1;
        break;
      case 24:
        struct_field_add_cli_conflict(&s_perm.fields[0], "cf");
        break;
      case 25:
        struct_field_add_cli_requires(&s_perm.fields[0], "rq");
        break;
      case 26:
        s_perm.fields[0].cli_skip = 1;
        break;
      case 27:
        s_perm.fields[0].cli_skip_to_cli = 1;
        break;
      case 28:
        s_perm.fields[0].cli_skip_from_cli = 1;
        break;
      case 29:
        s_perm.fields[0].cli_short = 's';
        s_perm.fields[0].cli_skip = 1;
        s_perm.fields[0].cli_skip_to_cli = 1;
        s_perm.fields[0].cli_skip_from_cli = 1;
        break;
      default:
        break;
      }
      ASSERT_EQ(0, write_struct_declaration_h(tmp, "PermS", &s_perm, NULL));
      struct_fields_free(&s_perm);
    }
  }

  fclose(tmp);
  PASS();
}

/**
 * @brief Executes the codegen_defaults_suite test suite.
 */
SUITE(codegen_defaults_suite) {
  RUN_TEST(test_def_errors);
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
  RUN_TEST(test_codegen_serde_cli_exhaustive);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_DEFAULTS_H */
