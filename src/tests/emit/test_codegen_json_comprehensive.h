#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_codegen_json_comprehensive.h
 * @brief Comprehensive and IO error tests for JSON code generator.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_JSON_COMPREHENSIVE_H
#define TEST_CODEGEN_JSON_COMPREHENSIVE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/json.h"
#include "classes/emit/struct.h"
/* clang-format on */

/** @brief Test definition g_fail_io_after. */
extern C_CDD_EXPORT int g_fail_io_after;
/** @brief Test definition g_io_calls. */
extern C_CDD_EXPORT int g_io_calls;

/**
 * @brief Unit test test_json_exhaustive_io.
 * @return Greatest test result.
 */
TEST test_json_exhaustive_io(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  int rc = 0;
  struct StructFields sf;
  struct CodegenJsonConfig config;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, "0", NULL);
  struct_fields_add(&sf, "data", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr", "array", "string", NULL, NULL);
  struct_fields_add(&sf, "obj", "object", "Obj", NULL, NULL);
  struct_fields_add(&sf, "enm", "enum", "Enum", NULL, NULL);
  struct_fields_add(&sf, "num", "number", NULL, NULL, NULL);
  struct_fields_add(&sf, "b", "boolean", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr_int", "array", "integer", NULL, NULL);
  struct_fields_add(&sf, "arr_obj", "array", "Obj", NULL, NULL);

  /* Optional fields */
  struct_fields_add(&sf, "opt_str", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].required = 0;

  /* Min/Max lengths */
  struct_fields_add(&sf, "len_str", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min_len = 1;
  sf.fields[sf.size - 1].min_len = 5;
  sf.fields[sf.size - 1].has_max_len = 1;
  sf.fields[sf.size - 1].max_len = 10;

  /* Min/Max items */
  struct_fields_add(&sf, "len_arr", "array", "string", NULL, NULL);
  sf.fields[sf.size - 1].has_min_items = 1;
  sf.fields[sf.size - 1].min_items = 1;
  sf.fields[sf.size - 1].has_max_items = 1;
  sf.fields[sf.size - 1].max_items = 3;

  /* Write-only and Read-only */
  struct_fields_add(&sf, "w_only", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].write_only = 1;
  struct_fields_add(&sf, "r_only", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].read_only = 1;

  /* Number with min/max */
  struct_fields_add(&sf, "num_bounded", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 100.0;
  sf.fields[sf.size - 1].exclusive_min = 1;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Integer with min/max exclusive */
  struct_fields_add(&sf, "int_bounded", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 10.0;
  sf.fields[sf.size - 1].exclusive_min = 1;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Number with inclusive min/max */
  struct_fields_add(&sf, "num_bounded_inc", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 100.0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 0;

  /* String with regex patterns */
  struct_fields_add(&sf, "pat_exact", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
  strcpy_s(sf.fields[sf.size - 1].pattern,
           sizeof(sf.fields[sf.size - 1].pattern), "^exact$");
#else
  strcpy(sf.fields[sf.size - 1].pattern, "^exact$");
#endif
  struct_fields_add(&sf, "pat_prefix", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
  strcpy_s(sf.fields[sf.size - 1].pattern,
           sizeof(sf.fields[sf.size - 1].pattern), "^prefix");
#else
  strcpy(sf.fields[sf.size - 1].pattern, "^prefix");
#endif
  struct_fields_add(&sf, "pat_suffix", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
  strcpy_s(sf.fields[sf.size - 1].pattern,
           sizeof(sf.fields[sf.size - 1].pattern), "suffix$");
#else
  strcpy(sf.fields[sf.size - 1].pattern, "suffix$");
#endif
  struct_fields_add(&sf, "pat_contains", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
  strcpy_s(sf.fields[sf.size - 1].pattern,
           sizeof(sf.fields[sf.size - 1].pattern), "contains");
#else
  strcpy(sf.fields[sf.size - 1].pattern, "contains");
#endif

  /* Integer with inclusive min/max */
  struct_fields_add(&sf, "int_bounded_inc", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 100.0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 0;

  /* Number with only max */
  struct_fields_add(&sf, "num_max_only", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 10.0;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Number with only exclusive min (bizarre but technically possible) */
  struct_fields_add(&sf, "num_ex_min_only", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 1;

  /* Number with only exclusive max */
  struct_fields_add(&sf, "num_ex_max_only", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Integer with only max */
  struct_fields_add(&sf, "int_max_only", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 10.0;
  sf.fields[sf.size - 1].exclusive_max = 0;

  /* Integer with only exclusive min */
  struct_fields_add(&sf, "int_ex_min_only", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 1;

  /* Integer with only exclusive max */
  struct_fields_add(&sf, "int_ex_max_only", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 1;

  config.guard_macro = cdd_test_mut_str("JSON_ENABLED");

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
    rc = write_struct_to_json_func(tmp, "Data", &sf, &config);
    if (tmp)
      fclose(tmp);
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
    rc = write_struct_from_json_func(tmp, "Data", &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
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
    rc = write_struct_array_from_json_func(tmp, "Data", &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
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
    rc = write_struct_from_jsonObject_func(tmp, "Data", &sf, &config);
    if (tmp)
      fclose(tmp);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  config.guard_macro = NULL;
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
    rc = write_struct_to_json_func(tmp, "Data", &sf, &config);
    if (tmp)
      fclose(tmp);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  g_fail_io_after = -1;
  struct_fields_free(&sf);
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_codegen_json_extra.
 * @return Greatest test result.
 */
TEST test_codegen_json_extra(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    struct CodegenJsonConfig config;

    ASSERT(tmp != NULL);
    struct_fields_init(&sf);

    /* Add fields for missing coverage:
       - exclusive_max, exclusive_min
       - max_len without min_len
       - min_items, max_items
     */
    {
      struct StructField *f;
      ASSERT_EQ(0, struct_fields_add(&sf, "v1", "number", NULL, NULL, NULL));
      f = &sf.fields[sf.size - 1];
      f->has_min = 1;
      f->min_val = 0;
      f->exclusive_min = 1;
      f->has_max = 1;
      f->max_val = 10;
      f->exclusive_max = 1;
    }
    {
      struct StructField *f;
      ASSERT_EQ(0, struct_fields_add(&sf, "v2", "string", NULL, NULL, NULL));
      f = &sf.fields[sf.size - 1];
      f->has_max_len = 1;
      f->max_len = 5;
    }
    {
      struct StructField *f;
      ASSERT_EQ(0, struct_fields_add(&sf, "v3", "array", "string", NULL, NULL));
      f = &sf.fields[sf.size - 1];
      f->has_min_items = 1;
      f->min_items = 1;
      f->has_max_items = 1;
      f->max_items = 5;
    }
    {
      struct StructField *f;
      ASSERT_EQ(0, struct_fields_add(&sf, "v4", "string", NULL, NULL, NULL));
      f = &sf.fields[sf.size - 1];
      f->has_min_len = 1;
      f->min_len = 1;
    }
    {
      struct StructField *f;
      ASSERT_EQ(0, struct_fields_add(&sf, "v5", "array", "string", NULL, NULL));
      f = &sf.fields[sf.size - 1];
      f->has_min_items = 0;
      f->has_max_items = 1;
      f->max_items = 10;
    }

    memset(&config, 0, sizeof(config));

    /* config == NULL tests */
    ASSERT_EQ(0, write_struct_from_json_func(tmp, "Extra", NULL));
    ASSERT_EQ(0, write_struct_to_json_func(tmp, "Extra", &sf, NULL));
    ASSERT_EQ(0, write_struct_array_from_json_func(tmp, "Extra", NULL));
    ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Extra", &sf, NULL));

    ASSERT_EQ(0, write_struct_from_json_func(tmp, "Extra", &config));
    ASSERT_EQ(0, write_struct_to_json_func(tmp, "Extra", &sf, &config));
    ASSERT_EQ(0, write_struct_array_from_json_func(tmp, "Extra", &config));
    ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Extra", &sf, &config));

    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_str_after_last;
#endif

/**
 * @brief Comprehensive test covering write_only, read_only, exclusive ranges,
 * array of objects, enum, and error paths in json emission.
 * @return Test or helper result.
 */
TEST test_codegen_json_comprehensive(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  if (tmp) {
    struct StructFields sf;
    struct StructField *f;
    struct CodegenJsonConfig config;
#ifdef CDD_BUILD_TESTS
    int i;
#endif
    memset(&config, 0, sizeof(config));

    struct_fields_init(&sf);

    /* 1. integer with exclusive_min and exclusive_max */
    struct_fields_add(&sf, "int_val", "integer", NULL, "0", NULL);
    f = &sf.fields[sf.size - 1];
    f->has_min = 1;
    f->min_val = 1.0;
    f->exclusive_min = 1;
    f->has_max = 1;
    f->max_val = 100.0;
    f->exclusive_max = 1;

    /* 2. number with exclusive_min and exclusive_max */
    struct_fields_add(&sf, "num_val", "number", NULL, "0.0", NULL);
    f = &sf.fields[sf.size - 1];
    f->has_min = 1;
    f->min_val = 0.5;
    f->exclusive_min = 1;
    f->has_max = 1;
    f->max_val = 99.5;
    f->exclusive_max = 1;

    /* 2b. number with non-exclusive min and max */
    struct_fields_add(&sf, "num_val2", "number", NULL, "0.0", NULL);
    f = &sf.fields[sf.size - 1];
    f->has_min = 1;
    f->min_val = 0.5;
    f->exclusive_min = 0;
    f->has_max = 1;
    f->max_val = 99.5;
    f->exclusive_max = 0;

    /* 2c. integer with non-exclusive min and max */
    struct_fields_add(&sf, "int_val2", "integer", NULL, "0", NULL);
    f = &sf.fields[sf.size - 1];
    f->has_min = 1;
    f->min_val = 1.0;
    f->exclusive_min = 0;
    f->has_max = 1;
    f->max_val = 100.0;
    f->exclusive_max = 0;

    /* 2d. integer with only min */
    struct_fields_add(&sf, "only_min_int", "integer", NULL, "0", NULL);
    f = &sf.fields[sf.size - 1];
    f->has_min = 1;
    f->min_val = 5.0;

    /* 2e. integer with only max */
    struct_fields_add(&sf, "only_max_int", "integer", NULL, "0", NULL);
    f = &sf.fields[sf.size - 1];
    f->has_max = 1;
    f->max_val = 50.0;

    /* 2f. number with only min */
    struct_fields_add(&sf, "only_min_num", "number", NULL, "0.0", NULL);
    f = &sf.fields[sf.size - 1];
    f->has_min = 1;
    f->min_val = 5.0;

    /* 2g. number with only max */
    struct_fields_add(&sf, "only_max_num", "number", NULL, "0.0", NULL);
    f = &sf.fields[sf.size - 1];
    f->has_max = 1;
    f->max_val = 50.0;

    /* 2h. string with only min_len */
    struct_fields_add(&sf, "only_min_str", "string", NULL, NULL, NULL);
    f = &sf.fields[sf.size - 1];
    f->has_min_len = 1;
    f->min_len = 2;

    /* 2i. string with only max_len */
    struct_fields_add(&sf, "only_max_str", "string", NULL, NULL, NULL);
    f = &sf.fields[sf.size - 1];
    f->has_max_len = 1;
    f->max_len = 20;

    /* 2j. string with both min_len and max_len */
    struct_fields_add(&sf, "both_len_str", "string", NULL, NULL, NULL);
    f = &sf.fields[sf.size - 1];
    f->has_min_len = 1;
    f->min_len = 2;
    f->has_max_len = 1;
    f->max_len = 20;

    /* 2k. integer with only exclusive min */
    struct_fields_add(&sf, "int_ex_min", "integer", NULL, "0", NULL);
    f = &sf.fields[sf.size - 1];
    f->exclusive_min = 1;

    /* 2l. integer with only exclusive max */
    struct_fields_add(&sf, "int_ex_max", "integer", NULL, "0", NULL);
    f = &sf.fields[sf.size - 1];
    f->exclusive_max = 1;

    /* 2m. number with only exclusive min */
    struct_fields_add(&sf, "num_ex_min", "number", NULL, "0.0", NULL);
    f = &sf.fields[sf.size - 1];
    f->exclusive_min = 1;

    /* 2n. number with only exclusive max */
    struct_fields_add(&sf, "num_ex_max", "number", NULL, "0.0", NULL);
    f = &sf.fields[sf.size - 1];
    f->exclusive_max = 1;

    /* 3. write_only field */
    struct_fields_add(&sf, "wo_val", "string", NULL, NULL, NULL);
    f = &sf.fields[sf.size - 1];
    f->write_only = 1;

    /* 4. read_only field */
    struct_fields_add(&sf, "ro_val", "string", NULL, NULL, NULL);
    f = &sf.fields[sf.size - 1];
    f->read_only = 1;

    /* 5. string array */
    struct_fields_add(&sf, "str_arr", "array", "string", NULL, NULL);

    /* 6. integer array */
    struct_fields_add(&sf, "int_arr", "array", "integer", NULL, NULL);

    /* 7. object array */
    struct_fields_add(&sf, "obj_arr", "array", "#/definitions/SubItem", NULL,
                      NULL);

    /* 8. object */
    struct_fields_add(&sf, "sub_obj", "object", "#/definitions/Child", NULL,
                      NULL);

    /* 9. enum */
    struct_fields_add(&sf, "status_val", "enum", "#/definitions/Status", NULL,
                      NULL);

    /* 10. plain integer (no min/max) */
    struct_fields_add(&sf, "plain_int", "integer", NULL, "0", NULL);

    /* 11. plain number (no min/max) */
    struct_fields_add(&sf, "plain_num", "number", NULL, "0.0", NULL);

    /* 12. plain string (no min/max/pattern) */
    struct_fields_add(&sf, "plain_str", "string", NULL, NULL, NULL);

    /* 13. exact pattern */
    struct_fields_add(&sf, "pat_exact", "string", NULL, NULL, NULL);
    f = &sf.fields[sf.size - 1];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strcpy_s(f->pattern, sizeof(f->pattern), "^exact$");
#else
    strcpy(f->pattern, "^exact$");
#endif

    /* 14. prefix pattern */
    struct_fields_add(&sf, "pat_prefix", "string", NULL, NULL, NULL);
    f = &sf.fields[sf.size - 1];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strcpy_s(f->pattern, sizeof(f->pattern), "^prefix");
#else
    strcpy(f->pattern, "^prefix");
#endif

    /* 15. suffix pattern */
    struct_fields_add(&sf, "pat_suffix", "string", NULL, NULL, NULL);
    f = &sf.fields[sf.size - 1];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strcpy_s(f->pattern, sizeof(f->pattern), "suffix$");
#else
    strcpy(f->pattern, "suffix$");
#endif

    /* 16. contains pattern */
    struct_fields_add(&sf, "pat_contains", "string", NULL, NULL, NULL);
    f = &sf.fields[sf.size - 1];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strcpy_s(f->pattern, sizeof(f->pattern), "contains");
#else
    strcpy(f->pattern, "contains");
#endif

    /* 17. array with min and max items */
    struct_fields_add(&sf, "arr_items", "array", "string", NULL, NULL);
    f = &sf.fields[sf.size - 1];
    f->has_min_items = 1;
    f->min_items = 1;
    f->has_max_items = 1;
    f->max_items = 10;

    /* Normal generation without guards */
    ASSERT_EQ(0, write_struct_to_json_func(tmp, "CompStruct", &sf, &config));
    ASSERT_EQ(
        0, write_struct_from_jsonObject_func(tmp, "CompStruct", &sf, &config));

    /* Normal generation with guards */
    config.guard_macro = "MY_GUARD";
    ASSERT_EQ(0, write_struct_to_json_func(tmp, "CompStruct", &sf, &config));
    ASSERT_EQ(
        0, write_struct_from_jsonObject_func(tmp, "CompStruct", &sf, &config));
    config.guard_macro = NULL;

#ifdef CDD_BUILD_TESTS
    {
      struct StructFields sf_single;

      /* object ref failure */
      struct_fields_init(&sf_single);
      struct_fields_add(&sf_single, "sub_obj", "object", "#/definitions/Child",
                        NULL, NULL);
      g_cdd_fail_str_after_last = 1;
      ASSERT_NEQ(0,
                 write_struct_to_json_func(tmp, "FailObj", &sf_single, NULL));
      g_cdd_fail_str_after_last = 0;
      g_cdd_fail_str_after_last = 1;
      ASSERT_NEQ(0, write_struct_from_jsonObject_func(tmp, "FailObj",
                                                      &sf_single, NULL));
      g_cdd_fail_str_after_last = 0;
      struct_fields_free(&sf_single);

      /* enum ref failure */
      struct_fields_init(&sf_single);
      struct_fields_add(&sf_single, "status_val", "enum",
                        "#/definitions/Status", NULL, NULL);
      g_cdd_fail_str_after_last = 1;
      ASSERT_NEQ(0,
                 write_struct_to_json_func(tmp, "FailEnum", &sf_single, NULL));
      g_cdd_fail_str_after_last = 0;
      g_cdd_fail_str_after_last = 1;
      ASSERT_NEQ(0, write_struct_from_jsonObject_func(tmp, "FailEnum",
                                                      &sf_single, NULL));
      g_cdd_fail_str_after_last = 0;
      struct_fields_free(&sf_single);

      /* array ref failure: to_json */
      struct_fields_init(&sf_single);
      struct_fields_add(&sf_single, "obj_arr", "array", "#/definitions/SubItem",
                        NULL, NULL);
      g_cdd_fail_str_after_last = 1;
      ASSERT_NEQ(0,
                 write_struct_to_json_func(tmp, "FailArr", &sf_single, NULL));
      g_cdd_fail_str_after_last = 0;

      /* array ref failure: from_jsonObject (call 1) */
      g_cdd_fail_str_after_last = 1;
      ASSERT_NEQ(0, write_struct_from_jsonObject_func(tmp, "FailArr",
                                                      &sf_single, NULL));
      g_cdd_fail_str_after_last = 0;

      /* array ref failure: from_jsonObject (call 2) */
      g_cdd_fail_str_after_last = 2;
      ASSERT_NEQ(0, write_struct_from_jsonObject_func(tmp, "FailArr",
                                                      &sf_single, NULL));
      g_cdd_fail_str_after_last = 0;
      struct_fields_free(&sf_single);

      /* Exhaustive IO failure across the complex struct */
      config.guard_macro = "MY_GUARD";
      {
        int total_calls;
        g_fail_io_after = 100000;
        g_io_calls = 0;
        write_struct_to_json_func(tmp, "CompStruct", &sf, &config);
        total_calls = g_io_calls;
        if (total_calls > 0) {
          g_fail_io_after = total_calls - 1;
          g_io_calls = 0;
          ASSERT_EQ(CDD_C_ERROR_IO,
                    write_struct_to_json_func(tmp, "CompStruct", &sf, &config));
        }
        g_fail_io_after = -1;
      }
      for (i = 0; i < 300; ++i) {
        cdd_c_error_t rc_io;
        g_fail_io_after = i;
        g_io_calls = 0;
        rc_io = write_struct_to_json_func(tmp, "CompStruct", &sf, &config);
        if (rc_io == CDD_C_SUCCESS) {
          g_fail_io_after = -1;
          break;
        }
        ASSERT_EQ(CDD_C_ERROR_IO, rc_io);
      }
      g_fail_io_after = -1;

      for (i = 0; i < 300; ++i) {
        cdd_c_error_t rc_io;
        g_fail_io_after = i;
        g_io_calls = 0;
        rc_io =
            write_struct_from_jsonObject_func(tmp, "CompStruct", &sf, &config);
        if (rc_io == CDD_C_SUCCESS) {
          g_fail_io_after = -1;
          break;
        }
        ASSERT_EQ(CDD_C_ERROR_IO, rc_io);
      }
      g_fail_io_after = -1;
      config.guard_macro = NULL;
    }
#endif

    struct_fields_free(&sf);
    fclose(tmp);
  }
  PASS();
}

/**
 * @brief Exhaustively tests Serde reflection branches in json.c.
 * @return greatest_test_res
 */
TEST test_json_serde_exhaustive(void) {
  struct StructFields sf;
  struct CodegenJsonConfig config;
  FILE *tmp = NULL;
  int i;

  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp != NULL);

  memset(&config, 0, sizeof(config));
  struct_fields_init(&sf);
  sf.serde_deny_unknown_fields = 1;
  sf.serde_untagged = 0;

  /* 0: skip_if = empty on string */
  struct_fields_add(&sf, "s_empty", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
  sf.fields[sf.size - 1].serde_has_default = 1;

  /* 1: skip_if = empty on array */
  struct_fields_add(&sf, "a_empty", "array", "string", NULL, NULL);
  sf.fields[sf.size - 1].serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
  struct_field_add_alias(&sf.fields[sf.size - 1], "a_alias");

  /* 2: skip_if = empty on object */
  struct_fields_add(&sf, "o_empty", "object", "ChildObj", NULL, NULL);
  sf.fields[sf.size - 1].serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
  struct_field_add_alias(&sf.fields[sf.size - 1], "o_alias");

  /* 3: skip_if = zero on integer with default_fn */
  struct_fields_add(&sf, "i_zero", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].serde_skip_if_kind = CDD_SERDE_SKIP_IF_ZERO;
  CDD_STRCPY(sf.fields[sf.size - 1].serde_default_fn,
             sizeof(sf.fields[sf.size - 1].serde_default_fn), "def_int");

  /* 4: skip_if = func on number with alias and default_val */
  struct_fields_add(&sf, "n_func", "number", NULL, "3.14", NULL);
  sf.fields[sf.size - 1].serde_skip_if_kind = CDD_SERDE_SKIP_IF_FUNC;
  CDD_STRCPY(sf.fields[sf.size - 1].serde_skip_if_fn,
             sizeof(sf.fields[sf.size - 1].serde_skip_if_fn), "is_neg");
  sf.fields[sf.size - 1].serde_has_default = 1;
  struct_field_add_alias(&sf.fields[sf.size - 1], "n_alias");

  /* 5: number with default_fn */
  struct_fields_add(&sf, "n_deffn", "number", NULL, NULL, NULL);
  CDD_STRCPY(sf.fields[sf.size - 1].serde_default_fn,
             sizeof(sf.fields[sf.size - 1].serde_default_fn), "def_num");

  /* 6: number with serde_has_default but empty default_val */
  struct_fields_add(&sf, "n_defempty", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].serde_has_default = 1;

  /* 7: integer with alias and default_val */
  struct_fields_add(&sf, "i_defval", "integer", NULL, "42", NULL);
  sf.fields[sf.size - 1].serde_has_default = 1;
  struct_field_add_alias(&sf.fields[sf.size - 1], "i_alias");

  /* 8: integer with serde_has_default and empty default_val */
  struct_fields_add(&sf, "i_defempty", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].serde_has_default = 1;

  /* 9: boolean with alias and default_fn */
  struct_fields_add(&sf, "b_deffn", "boolean", NULL, NULL, NULL);
  CDD_STRCPY(sf.fields[sf.size - 1].serde_default_fn,
             sizeof(sf.fields[sf.size - 1].serde_default_fn), "def_bool");
  struct_field_add_alias(&sf.fields[sf.size - 1], "b_alias");

  /* 10: boolean with default_val */
  struct_fields_add(&sf, "b_defval", "boolean", NULL, "1", NULL);
  sf.fields[sf.size - 1].serde_has_default = 1;

  /* 11: boolean with serde_has_default and empty default_val */
  struct_fields_add(&sf, "b_defempty", "boolean", NULL, NULL, NULL);
  sf.fields[sf.size - 1].serde_has_default = 1;

  /* 11b: boolean with alias only (serde_has_default = 0, default_val = "") */
  struct_fields_add(&sf, "b_alonly", "boolean", NULL, NULL, NULL);
  struct_field_add_alias(&sf.fields[sf.size - 1], "b_al2");

  /* 11c: read_only field in deny_unknown_fields struct */
  struct_fields_add(&sf, "ro_field", "string", NULL, "ro_def", NULL);
  sf.fields[sf.size - 1].read_only = 1;

  /* 11d: string with serde_has_default = 1 and default_val = "" */
  struct_fields_add(&sf, "s_defempty", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].serde_has_default = 1;

  /* 12: string with default_fn */
  struct_fields_add(&sf, "s_deffn", "string", NULL, NULL, NULL);
  CDD_STRCPY(sf.fields[sf.size - 1].serde_default_fn,
             sizeof(sf.fields[sf.size - 1].serde_default_fn), "def_str");

  /* 13: string with serde_has_default and default_val */
  struct_fields_add(&sf, "s_defval", "string", NULL, "hello", NULL);
  sf.fields[sf.size - 1].serde_has_default = 1;

  /* 14: enum with alias */
  struct_fields_add(&sf, "e_val", "enum", "MyEnum", NULL, NULL);
  struct_field_add_alias(&sf.fields[sf.size - 1], "e_alias");

  /* 15: flatten with skip_if */
  struct_fields_add(&sf, "flat_obj", "object", "#/definitions/FlatChild", NULL,
                    NULL);
  sf.fields[sf.size - 1].serde_flatten = 1;
  sf.fields[sf.size - 1].serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;

  /* 16: flatten non-object (serde_flatten = 1, type = "string") */
  struct_fields_add(&sf, "flat_str", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].serde_flatten = 1;

  /* 17: serialize_with and deserialize_with */
  struct_fields_add(&sf, "custom_rw", "string", NULL, NULL, NULL);
  CDD_STRCPY(sf.fields[sf.size - 1].serde_serialize_with,
             sizeof(sf.fields[sf.size - 1].serde_serialize_with), "my_ser");
  CDD_STRCPY(sf.fields[sf.size - 1].serde_deserialize_with,
             sizeof(sf.fields[sf.size - 1].serde_deserialize_with), "my_de");

  /* 18: skip_deserializing with default_fn */
  struct_fields_add(&sf, "skip_de_fn", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].serde_skip_deserializing = 1;
  CDD_STRCPY(sf.fields[sf.size - 1].serde_default_fn,
             sizeof(sf.fields[sf.size - 1].serde_default_fn), "init_skip_int");

  /* 19: skip_deserializing with non-string default_val */
  struct_fields_add(&sf, "skip_de_int", "integer", NULL, "77", NULL);
  sf.fields[sf.size - 1].serde_skip_deserializing = 1;

  /* Exhaustive IO failures on Serde struct */
  for (i = 0; i < 400; ++i) {
    cdd_c_error_t rc_io;
    g_fail_io_after = i;
    g_io_calls = 0;
    rc_io = write_struct_to_json_func(tmp, "SerdeExhaustive", &sf, &config);
    if (rc_io == CDD_C_SUCCESS) {
      g_fail_io_after = -1;
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_IO, rc_io);
  }
  g_fail_io_after = -1;

  for (i = 0; i < 400; ++i) {
    cdd_c_error_t rc_io;
    g_fail_io_after = i;
    g_io_calls = 0;
    rc_io =
        write_struct_from_jsonObject_func(tmp, "SerdeExhaustive", &sf, &config);
    if (rc_io == CDD_C_SUCCESS) {
      g_fail_io_after = -1;
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_IO, rc_io);
  }
  g_fail_io_after = -1;

  /* Test untagged with deny_unknown_fields */
  sf.serde_untagged = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_struct_from_jsonObject_func(
                               tmp, "SerdeUntagged", &sf, &config));
  sf.serde_untagged = 0;

  struct_fields_free(&sf);

  /* Test rc_needed specifically triggered by serde_serialize_with / serde_with
   * / serde_deserialize_with / serde_default_fn */
  {
    struct StructFields sf_rc;

    /* 1. to_json with serde_serialize_with only */
    struct_fields_init(&sf_rc);
    struct_fields_add(&sf_rc, "x", "integer", NULL, NULL, NULL);
    CDD_STRCPY(sf_rc.fields[0].serde_serialize_with,
               sizeof(sf_rc.fields[0].serde_serialize_with), "ser_x");
    ASSERT_EQ(CDD_C_SUCCESS,
              write_struct_to_json_func(tmp, "S1", &sf_rc, NULL));
    struct_fields_free(&sf_rc);

    /* 2. from_jsonObject with read_only + serde_default_fn only */
    struct_fields_init(&sf_rc);
    struct_fields_add(&sf_rc, "x", "integer", NULL, NULL, NULL);
    sf_rc.fields[0].read_only = 1;
    CDD_STRCPY(sf_rc.fields[0].serde_default_fn,
               sizeof(sf_rc.fields[0].serde_default_fn), "def_x");
    for (i = 0; i < 30; ++i) {
      cdd_c_error_t rc_io;
      g_fail_io_after = i;
      g_io_calls = 0;
      rc_io = write_struct_from_jsonObject_func(tmp, "S2", &sf_rc, NULL);
      if (rc_io == CDD_C_SUCCESS) {
        g_fail_io_after = -1;
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_IO, rc_io);
    }
    g_fail_io_after = -1;
    struct_fields_free(&sf_rc);

    /* 3. from_jsonObject with serde_deserialize_with only */
    struct_fields_init(&sf_rc);
    struct_fields_add(&sf_rc, "x", "integer", NULL, NULL, NULL);
    CDD_STRCPY(sf_rc.fields[0].serde_deserialize_with,
               sizeof(sf_rc.fields[0].serde_deserialize_with), "de_x");
    ASSERT_EQ(CDD_C_SUCCESS,
              write_struct_from_jsonObject_func(tmp, "S3", &sf_rc, NULL));
    struct_fields_free(&sf_rc);

    /* 4. from_jsonObject with serde_default_fn on normal field only */
    struct_fields_init(&sf_rc);
    struct_fields_add(&sf_rc, "x", "integer", NULL, NULL, NULL);
    CDD_STRCPY(sf_rc.fields[0].serde_default_fn,
               sizeof(sf_rc.fields[0].serde_default_fn), "def_x");
    ASSERT_EQ(CDD_C_SUCCESS,
              write_struct_from_jsonObject_func(tmp, "S4", &sf_rc, NULL));
    struct_fields_free(&sf_rc);

    /* 5. flatten get_type_from_ref failure in to_json and from_jsonObject */
#ifdef CDD_BUILD_TESTS
    struct_fields_init(&sf_rc);
    struct_fields_add(&sf_rc, "flat", "object", "#/definitions/Sub", NULL,
                      NULL);
    sf_rc.fields[0].serde_flatten = 1;
    struct_fields_add(&sf_rc, "flat2", "object", "#/definitions/Sub2", NULL,
                      NULL);
    sf_rc.fields[1].serde_flatten = 1;
    for (i = 0; i < 40; ++i) {
      cdd_c_error_t rc_io;
      g_fail_io_after = i;
      g_io_calls = 0;
      rc_io = write_struct_to_json_func(tmp, "TwoFlats", &sf_rc, NULL);
      if (rc_io == CDD_C_SUCCESS) {
        g_fail_io_after = -1;
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_IO, rc_io);
    }
    g_fail_io_after = -1;
    g_cdd_fail_str_after_last = 1;
    ASSERT_NEQ(CDD_C_SUCCESS,
               write_struct_to_json_func(tmp, "FailFlat", &sf_rc, NULL));
    g_cdd_fail_str_after_last = 1;
    ASSERT_NEQ(CDD_C_SUCCESS, write_struct_from_jsonObject_func(tmp, "FailFlat",
                                                                &sf_rc, NULL));
    g_cdd_fail_str_after_last = 0;
    struct_fields_free(&sf_rc);
#endif

    /* 6. empty field name failure in cdd_serde_resolve_field_names */
    struct_fields_init(&sf_rc);
    struct_fields_add(&sf_rc, "", "integer", NULL, NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_to_json_func(tmp, "FailName", &sf_rc, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_jsonObject_func(tmp, "FailName", &sf_rc, NULL));
    sf_rc.serde_deny_unknown_fields = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_jsonObject_func(tmp, "FailName", &sf_rc, NULL));
    struct_fields_free(&sf_rc);

    /* 7. IO failure on boolean with serde_default_fn only, boolean with
     * default_val, and string with alias + serde_has_default + default_val */
    struct_fields_init(&sf_rc);
    struct_fields_add(&sf_rc, "b_fn_only", "boolean", NULL, NULL, NULL);
    CDD_STRCPY(sf_rc.fields[0].serde_default_fn,
               sizeof(sf_rc.fields[0].serde_default_fn), "def_b");
    struct_fields_add(&sf_rc, "b_al", "boolean", NULL, NULL, NULL);
    struct_field_add_alias(&sf_rc.fields[1], "b2");
    struct_fields_add(&sf_rc, "b_def", "boolean", NULL, "1", NULL);
    sf_rc.fields[2].serde_has_default = 1;
    struct_fields_add(&sf_rc, "s_def", "string", NULL, "def_s", NULL);
    sf_rc.fields[3].serde_has_default = 1;
    struct_field_add_alias(&sf_rc.fields[3], "s_alias_io");
    for (i = 0; i < 50; ++i) {
      cdd_c_error_t rc_io;
      g_fail_io_after = i;
      g_io_calls = 0;
      rc_io = write_struct_from_jsonObject_func(tmp, "S7", &sf_rc, NULL);
      if (rc_io == CDD_C_SUCCESS) {
        g_fail_io_after = -1;
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_IO, rc_io);
    }
    g_fail_io_after = -1;
    struct_fields_free(&sf_rc);

    /* 8. Required fields, container serde_default, and flatten aliases in
     * deny_unknown_fields */
    struct_fields_init(&sf_rc);
    sf_rc.serde_deny_unknown_fields = 1;
    struct_fields_add(&sf_rc, "req_plain", "integer", NULL, NULL, NULL);
    sf_rc.fields[0].required = 1;
    struct_fields_add(&sf_rc, "req_al", "string", NULL, NULL, NULL);
    sf_rc.fields[1].required = 1;
    struct_field_add_alias(&sf_rc.fields[1], "req_alias1");
    struct_fields_add(&sf_rc, "req_def", "integer", NULL, "5", NULL);
    sf_rc.fields[2].required = 1;
    sf_rc.fields[2].serde_has_default = 1;
    struct_fields_add(&sf_rc, "req_fn", "integer", NULL, NULL, NULL);
    sf_rc.fields[3].required = 1;
    CDD_STRCPY(sf_rc.fields[3].serde_default_fn,
               sizeof(sf_rc.fields[3].serde_default_fn), "def_req_fn");
    struct_fields_add(&sf_rc, "flat_al", "object", "#/definitions/FlatSub",
                      NULL, NULL);
    sf_rc.fields[4].serde_flatten = 1;
    struct_field_add_alias(&sf_rc.fields[4], "flat_child_key");
    for (i = 0; i < 60; ++i) {
      cdd_c_error_t rc_io;
      g_fail_io_after = i;
      g_io_calls = 0;
      rc_io = write_struct_from_jsonObject_func(tmp, "S8", &sf_rc, NULL);
      if (rc_io == CDD_C_SUCCESS) {
        g_fail_io_after = -1;
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_IO, rc_io);
    }
    g_fail_io_after = -1;
    sf_rc.serde_default = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              write_struct_from_jsonObject_func(tmp, "S8Def", &sf_rc, NULL));
    struct_fields_free(&sf_rc);
  }

  fclose(tmp);
  PASS();
}

/**
 * @brief Executes the codegen_json_comprehensive_suite test suite.
 */
SUITE(codegen_json_comprehensive_suite) {
  RUN_TEST(test_json_exhaustive_io);
  RUN_TEST(test_codegen_json_extra);
  RUN_TEST(test_codegen_json_comprehensive);
  RUN_TEST(test_json_serde_exhaustive);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CODEGEN_JSON_COMPREHENSIVE_H */
