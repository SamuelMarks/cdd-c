/**
 * @file test_codegen_struct_io.h
 * @brief Unit tests for struct codegen IO error handling and exhaustive IO.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_IO_H
#define TEST_CODEGEN_STRUCT_IO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers_export.h"
#include "classes/emit/struct.h"
/* clang-format on */

CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);

extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;

/**
 * @brief test_struct_io_errors
 * @return TEST
 */
TEST test_struct_io_errors(void) {
  FILE *readonly_f;
#if defined(_MSC_VER)
  if (((readonly_f = cdd_test_tmpfile_global()) == NULL))
    readonly_f = NULL;
#else
  readonly_f = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    struct CodegenStructConfig config = {0};

    struct_fields_init(&sf);
    struct_fields_add(&sf, "id", "int", NULL, NULL, NULL);
    struct_fields_add(&sf, "data", "string", NULL, NULL, NULL);
    struct_fields_add(&sf, "arr", "array", "string", NULL, NULL);
    struct_fields_add(&sf, "obj", "object", "Obj", NULL, NULL);
    struct_fields_add(&sf, "enm", "enum", "Enum", "VAL", NULL);
    struct_fields_add(&sf, "num", "number", NULL, NULL, NULL);
    struct_fields_add(&sf, "b", "boolean", NULL, NULL, NULL);

    if (readonly_f) {
      g_fail_io_after = 0;
      g_io_calls = 0;
      {
        int _rc = write_struct_cleanup_func(readonly_f, "Test", &sf, &config);
        printf("write_struct_cleanup_func RC %d\n", _rc);
        ASSERT_EQ(CDD_C_ERROR_IO, _rc);
      }
      g_fail_io_after = 0;
      g_io_calls = 0;
      {
        int _rc = write_struct_default_func(readonly_f, "Test", &sf, &config);
        printf("write_struct_default_func RC %d\n", _rc);
        ASSERT_EQ(CDD_C_ERROR_IO, _rc);
      }
      g_fail_io_after = 0;
      g_io_calls = 0;
      ASSERT_EQ(CDD_C_ERROR_IO,
                write_struct_deepcopy_func(readonly_f, "Test", &sf, &config));
      g_fail_io_after = 0;
      g_io_calls = 0;
      ASSERT_EQ(CDD_C_ERROR_IO,
                write_struct_eq_func(readonly_f, "Test", &sf, &config));
      g_fail_io_after = 0;
      g_io_calls = 0;
      ASSERT_EQ(CDD_C_ERROR_IO,
                write_struct_debug_func(readonly_f, "Test", &sf, &config));
      fclose(readonly_f);
    }
    struct_fields_free(NULL);
    struct_fields_free(&sf);
    g_fail_io_after = -1;

    PASS();
  }
}

/**
 * @brief test_struct_exhaustive_io
 * @return TEST
 */
TEST test_struct_exhaustive_io(void) {
#ifdef CDD_BUILD_TESTS
  int i, rc;
  struct StructFields sf;
  struct CodegenStructConfig config = {0};

  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "int", NULL, NULL, NULL);
  struct_fields_add(&sf, "data", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr", "array", "string", NULL, NULL);
  struct_fields_add(&sf, "arr_obj", "array", "Obj", NULL, NULL);
  struct_fields_add(&sf, "arr_int", "array", "integer", NULL, NULL);
  struct_fields_add(&sf, "arr_num", "array", "number", NULL, NULL);
  struct_fields_add(&sf, "arr_enum", "array", "enum", "Enum", NULL);
  struct_fields_add(&sf, "arr_bool", "array", "boolean", NULL, NULL);
  struct_fields_add(&sf, "def_123", "integer", NULL, "123", NULL);
  struct_fields_add(&sf, "def_hex", "integer", NULL, "0x10", NULL);
  struct_fields_add(&sf, "obj", "object", "Obj", NULL, NULL);
  struct_fields_add(&sf, "enm", "enum", "Enum", "VAL", NULL);
  struct_fields_add(&sf, "num", "number", NULL, NULL, NULL);
  struct_fields_add(&sf, "b", "boolean", NULL, NULL, NULL);
  struct_fields_add(&sf, "unk", "unknown", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr_unk", "array", "unknown", NULL, NULL);
  struct_fields_add(&sf, "arr_obj2", "array", "object", NULL, NULL);
  struct_fields_add(&sf, "arr_bool", "array", "boolean", NULL, NULL);
  struct_fields_add(&sf, "def_str", "string", NULL, "\"test\"", NULL);
  struct_fields_add(&sf, "def_str_null", "string", NULL, "nullptr", NULL);
  struct_fields_add(&sf, "def_prim_null", "integer", NULL, "nullptr", NULL);
  struct_fields_add(&sf, "def_bin", "integer", NULL, "0b10", NULL);
  struct_fields_add(&sf, "def_bin_B", "integer", NULL, "0B10", NULL);
  struct_fields_add(&sf, "def_bin_bad", "integer", NULL, "0bXX", NULL);
  struct_fields_add(&sf, "def_prim", "integer", NULL, "42", NULL);

  /* Optional fields */
  struct_fields_add(&sf, "opt_str", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].required = 0;

  sf.is_union = 1;
  sf.union_is_anyof = 0;
  sf.union_discriminator = (char *)(size_t)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_discriminator, 5, "type");
#else
  strcpy(sf.union_discriminator, "type");
#endif
  sf.n_union_variants = 1;
  sf.union_variants =
      (struct UnionVariantMeta *)calloc(1, sizeof(struct UnionVariantMeta));
  sf.union_variants[0].n_property_names = 1;
  sf.union_variants[0].property_names = (char **)calloc(1, sizeof(char *));
  sf.union_variants[0].property_names[0] = (char *)(size_t)malloc(2);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[0].property_names[0], 2, "a");
#else
  strcpy(sf.union_variants[0].property_names[0], "a");
#endif
  sf.union_variants[0].n_required_props = 1;
  sf.union_variants[0].required_props = (char **)calloc(1, sizeof(char *));
  sf.union_variants[0].required_props[0] = (char *)(size_t)malloc(2);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[0].required_props[0], 2, "a");
#else
  strcpy(sf.union_variants[0].required_props[0], "a");
#endif
  sf.union_variants[0].disc_value = (char *)(size_t)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[0].disc_value, 5, "val1");
#else
  strcpy(sf.union_variants[0].disc_value, "val1");
#endif

  for (i = 0; i < 50; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_cleanup_func(tmp, "Test", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 50; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_default_func(tmp, "Test", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 50; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_deepcopy_func(tmp, "Test", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 200; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_eq_func(tmp, "Test", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 200; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_debug_func(tmp, "Test", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 200; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_display_func(tmp, "Test", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 50; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_cleanup_func(tmp, "Test", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  g_fail_io_after = -1;
  struct_fields_free(NULL);
  struct_fields_free(&sf);
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief codegen_struct_io_suite
 */
SUITE(codegen_struct_io_suite) {
  RUN_TEST(test_struct_io_errors);
  RUN_TEST(test_struct_exhaustive_io);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_STRUCT_IO_H */
