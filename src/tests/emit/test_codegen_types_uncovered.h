#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_codegen_types_uncovered.h
 * @brief Uncovered branches unit tests for Advanced Types generation.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_TYPES_UNCOVERED_H
#define TEST_CODEGEN_TYPES_UNCOVERED_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/struct.h"
#include "classes/emit/types.h"
/* clang-format on */

TEST test_types_uncovered(void) {
  int i, rc;
  struct StructFields sf;
  struct CodegenTypesConfig config = {0};
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  /* 1. Test meta with property names (to cover lines 341, 343) */
  struct_fields_init(&sf);
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "obj2");
#else
    strcpy(f->name, "obj2");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "object");
#else
    strcpy(f->type, "object");
#endif

    sf.union_variants =
        (struct UnionVariantMeta *)calloc(1, sizeof(struct UnionVariantMeta));
    sf.n_union_variants = 1;
    sf.union_variants[0].n_property_names = 3;
    sf.union_variants[0].property_names = (char **)calloc(3, sizeof(char *));
    sf.union_variants[0].property_names[0] = strdup("a");
    sf.union_variants[0].property_names[1] = NULL;
    sf.union_variants[0].property_names[2] = strdup("b");
  }

  /* 2. Test union_is_anyof = 0 and (int_count > 1) */
  sf.union_is_anyof = 0;
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "i1");
#else
    strcpy(f->name, "i1");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "integer");
#else
    strcpy(f->type, "integer");
#endif
  }
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "i2");
#else
    strcpy(f->name, "i2");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "integer");
#else
    strcpy(f->type, "integer");
#endif
  }

  /* 3. Test boolean count > 1 and null_count > 1 */
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "b1");
#else
    strcpy(f->name, "b1");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "boolean");
#else
    strcpy(f->type, "boolean");
#endif
  }
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "b2");
#else
    strcpy(f->name, "b2");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "boolean");
#else
    strcpy(f->type, "boolean");
#endif
  }
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "null1");
#else
    strcpy(f->name, "null1");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "null");
#else
    strcpy(f->type, "null");
#endif
  }
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "null2");
#else
    strcpy(f->name, "null2");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "null");
#else
    strcpy(f->type, "null");
#endif
  }
  struct_fields_add(&sf, "obj3", "object", "Object", NULL, NULL);
  struct_fields_add(&sf, "s1", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "s2", "string", NULL, NULL, NULL);

  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_jsonObject_func(t, "Union1", &sf, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(t, "Union1", &sf, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }

  struct_fields_free(&sf);

  /* 3.5 Test string count > 1 */
  struct_fields_init(&sf);
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "s1");
#else
    strcpy(f->name, "s1");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "string");
#else
    strcpy(f->type, "string");
#endif
  }
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "s2");
#else
    strcpy(f->name, "s2");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "string");
#else
    strcpy(f->type, "string");
#endif
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(t, "Union_Strings", &sf, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  struct_fields_free(&sf);

  /* 3.6 Test string count == 1 */
  struct_fields_init(&sf);
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "s1");
#else
    strcpy(f->name, "s1");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "string");
#else
    strcpy(f->type, "string");
#endif
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(t, "Union_String_1", &sf, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  struct_fields_free(&sf);

  /* 4. Test int_count == 0 && num_count == 1 and boolean count == 1 */
  struct_fields_init(&sf);
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "n1");
#else
    strcpy(f->name, "n1");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "number");
#else
    strcpy(f->type, "number");
#endif
  }
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "b1");
#else
    strcpy(f->name, "b1");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "boolean");
#else
    strcpy(f->type, "boolean");
#endif
  }

  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(t, "Union2", &sf, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  struct_fields_free(&sf);

  /* 4.2. Test num_count > 1 */
  struct_fields_init(&sf);
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "n1");
#else
    strcpy(f->name, "n1");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "number");
#else
    strcpy(f->type, "number");
#endif
  }
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "n2");
#else
    strcpy(f->name, "n2");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "number");
#else
    strcpy(f->type, "number");
#endif
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(t, "Union2_5", &sf, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  struct_fields_free(&sf);

  /* 4.5. Test exactly one integer to hit line 687 */
  struct_fields_init(&sf);
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "i1");
#else
    strcpy(f->name, "i1");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "integer");
#else
    strcpy(f->type, "integer");
#endif
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(t, "Union3", &sf, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  struct_fields_free(&sf);

  /* 4.6. Test NO ints and NO numbers to hit line 701 false branch */
  struct_fields_init(&sf);
  {
    struct StructField *f = &sf.fields[sf.size++];
#if defined(_MSC_VER)
    strcpy_s(f->name, sizeof(f->name), "b3");
#else
    strcpy(f->name, "b3");
#endif
#if defined(_MSC_VER)
    strcpy_s(f->type, sizeof(f->type), "boolean");
#else
    strcpy(f->type, "boolean");
#endif
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(t, "Union4", &sf, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  struct_fields_free(&sf);

  /* 5. Test root array string cleanup and object */
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_cleanup_func(t, "ArrStr", "string", NULL, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_to_json_func(t, "ArrStr", "string", NULL, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_from_json_func(t, "ArrStr", "string", NULL, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_cleanup_func(t, "ArrObj", "object", "MyObj", &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_to_json_func(t, "ArrObj", "object", "MyObj", &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_from_json_func(t, "ArrObj", "object", "MyObj",
                                         &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_cleanup_func(t, "ArrInt", "integer", NULL, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_to_json_func(t, "ArrInt", "integer", NULL, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_from_json_func(t, "ArrInt", "integer", NULL, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_cleanup_func(t, "ArrBool", "boolean", NULL, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_to_json_func(t, "ArrBool", "boolean", NULL, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 50; ++i) {
    FILE *t;
#if defined(_MSC_VER)
    if (((t = cdd_test_tmpfile_global()) == NULL))
      t = NULL;
#else
    t = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc =
        write_root_array_from_json_func(t, "ArrBool", "boolean", NULL, &config);
    if (t)
      fclose(t);
    if (rc == 0)
      break;
  }
  /* 6. Exhaustive IO for write_union_from_json_func with single array of
   * various types */
  {
    int t_idx;
    const char *arr_types[] = {"integer", "number",  "string", "object",
                               "boolean", "unknown", "enum"};
    const char *arr_refs[] = {(char *)(size_t)NULL,
                              (char *)(size_t)NULL,
                              (char *)(size_t)NULL,
                              "ObjType",
                              (char *)(size_t)NULL,
                              (char *)(size_t)NULL,
                              "MyEnum"};
    for (t_idx = 0; t_idx < 7; ++t_idx) {
      struct_fields_init(&sf);
      struct_fields_add(&sf, "arr", "array", arr_types[t_idx], arr_refs[t_idx],
                        NULL);

      for (i = 0; i < 50; ++i) {
        FILE *t;
#if defined(_MSC_VER)
        if (((t = cdd_test_tmpfile_global()) == NULL))
          t = NULL;
#else
        t = cdd_test_tmpfile_global();
#endif
        g_fail_io_after = i;
        g_io_calls = 0;
        rc = write_union_from_json_func(t, "UnionArr", &sf, &config);
        if (t)
          fclose(t);
        if (rc == 0)
          break;
      }
      struct_fields_free(&sf);
    }
  }

  g_fail_io_after = -1;
  if (tmp)
    fclose(tmp);
  PASS();
}

SUITE(codegen_types_uncovered_suite) { RUN_TEST(test_types_uncovered); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_TYPES_UNCOVERED_H */
