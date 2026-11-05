/**
 * @file test_codegen_struct_fields.h
 * @brief Unit tests for StructFields allocation, OOM paths, ref resolution,
 * and IO failure loops across struct emitters.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_FIELDS_H
#define TEST_CODEGEN_STRUCT_FIELDS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_struct_common.h"
/* clang-format on */

/**
 * @brief Unit test test_struct_fields_init_oom.
 * @return Greatest test result.
 */
TEST test_struct_fields_init_oom(void) {
  struct StructFields sf = {0};
  struct StructField *f_ptr = NULL;
  const char *val = NULL;
#ifdef CDD_BUILD_TESTS
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, struct_fields_init(&sf));
  struct_fields_free(&sf); /* hit sf->fields == NULL branch */
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(0, struct_fields_init(&sf));

  sf.capacity = 1;
  sf.size = 1;
  ASSERT_EQ(0, struct_fields_add(&sf, "a", "b", NULL, NULL, NULL));

  sf.capacity = sf.size;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            struct_fields_add(&sf, "c", "d", NULL, NULL, NULL));
  g_cdd_alloc_fail = 0;

  struct_fields_free(&sf);
#endif

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, struct_fields_init(NULL));
  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            struct_fields_add(NULL, "a", "b", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            struct_fields_add(&sf, NULL, "b", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            struct_fields_add(&sf, "a", NULL, NULL, NULL, NULL));

  ASSERT_EQ(0, struct_fields_get(NULL, "a", &f_ptr));
  ASSERT_EQ(0, struct_fields_get(&sf, NULL, &f_ptr));

  /* Test get_type_from_ref null */
  ASSERT_EQ(0, get_type_from_ref(NULL, &val));
  ASSERT_STR_EQ("", val);
  ASSERT_EQ(0, get_type_from_ref("#/components/schemas/My-Type", &val));
  ASSERT_STR_EQ("My_Type", val);
#ifdef CDD_BUILD_TESTS
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(0, get_type_from_ref("#/components/schemas/My-Type", &val));
  ASSERT_STR_EQ("My-Type", val);
  g_cdd_alloc_fail = 0;
#endif

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, struct_field_add_alias(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            struct_fields_add_cli_alias(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            struct_field_add_cli_conflict(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            struct_field_add_cli_requires(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            struct_field_add_cli_required_unless(NULL, "a"));
  ASSERT_EQ(0, struct_fields_add(&sf, "f0", "string", NULL, NULL, NULL));
  sf.fields[0].cli_long_help = (char *)C_CDD_MALLOC(4);
  if (sf.fields[0].cli_long_help) {
    sf.fields[0].cli_long_help[0] = 'h';
    sf.fields[0].cli_long_help[1] = '\0';
  }
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            struct_field_add_alias(&sf.fields[0], NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            struct_field_add_alias(&sf.fields[0], ""));
  ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_alias(&sf.fields[0], "al1"));
#ifdef CDD_BUILD_TESTS
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, struct_field_add_alias(&sf.fields[0], "al2"));
  g_cdd_strdup_fail = 0;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, struct_field_add_alias(&sf.fields[0], "al2"));
  g_cdd_alloc_fail = 0;
#endif
  C_CDD_FREE(sf.fields[0].serde_aliases[0]);
  sf.fields[0].serde_aliases[0] = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_alias(&sf.fields[0], "al3"));

  /* Free string array null */
  sf.is_union = 1;
  sf.union_is_anyof = 0;
  sf.union_discriminator = (char *)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_discriminator, 5, "type");
#else
  strcpy(sf.union_discriminator, "type");
#endif
  sf.n_union_variants = 1;
  sf.union_variants =
      (struct UnionVariantMeta *)calloc(1, sizeof(struct UnionVariantMeta));
  /* We leave required_props and property_names NULL */

  /* Force capacity expansion */
  sf.capacity = 1;
  sf.size = 1;
#ifdef CDD_BUILD_TESTS
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            struct_fields_add(&sf, "new_field", "string", NULL, NULL, NULL));
  g_cdd_alloc_fail = 0;
#endif

  struct_fields_free(NULL);
  struct_fields_free(&sf);
  g_fail_io_after = -1;

  /* Test get_type_from_ref failure via g_cdd_fail_str_after_last */
  {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    if (tmp) {
      struct StructFields sf_ref;
      struct CodegenStructConfig cfg;
      struct StructFields sf_arr;
      struct StructFields sf_empty;
      struct StructFields sf_arr_prim;
      struct StructFields sf_str;
      struct StructFields sf_obj;
      struct StructFields sf_enum;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_ref));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_ref, "item", "object",
                                  "#/definitions/Item", "{}", NULL));

      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_arr));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_arr, "items", "array",
                                  "#/definitions/Item", NULL, NULL));

      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_arr_prim));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_arr_prim, "arr_int", "array", "integer",
                                  NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_arr_prim, "arr_enum",
                                                 "array", "enum", NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_arr_prim, "arr_str", "array", "string",
                                  NULL, NULL));

      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_empty));

      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_str));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_str, "str_f", "string",
                                                 NULL, "\"val\"", NULL));

      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_obj));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_obj, "obj_f", "object",
                                  "#/definitions/Item", NULL, NULL));

      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_enum));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_enum, "enum_f", "enum",
                                  "#/definitions/MyEnum", "\"FOO\"", NULL));

      cfg.guard_macro = cdd_test_mut_str("MY_GUARD");

      /* cleanup with array of ref objects failing get_type_from_ref */
      g_cdd_fail_str_after_last = 1;
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
                write_struct_cleanup_func(tmp, "Test", &sf_arr, NULL));
      g_cdd_fail_str_after_last = 0;

      /* eq with array of ref objects failing get_type_from_ref */
      g_cdd_fail_str_after_last = 1;
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
                write_struct_eq_func(tmp, "Test", &sf_arr, NULL));
      g_cdd_fail_str_after_last = 0;

      /* default with ref object failing get_type_from_ref */
      g_cdd_fail_str_after_last = 1;
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
                write_struct_default_func(tmp, "Test", &sf_ref, NULL));
      g_cdd_fail_str_after_last = 0;

      /* debug with ref object failing get_type_from_ref */
      g_cdd_fail_str_after_last = 1;
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
                write_struct_debug_func(tmp, "Test", &sf_ref, NULL));
      g_cdd_fail_str_after_last = 0;

      /* eq with array but empty array fields so iter_needed is true */
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_eq_func(tmp, "TestArr", &sf_arr_prim, &cfg));
      /* eq with non-array struct with guard to hit iter_needed false and guard
       * branch */
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_eq_func(tmp, "TestNoArr", &sf_str, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_eq_func(tmp, "TestNoArrNoGuard", &sf_str, NULL));

      /* default with enum */
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_default_func(tmp, "TestEnum", &sf_enum, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, write_struct_default_func(tmp, "TestEnumNoGuard",
                                                         &sf_enum, NULL));

      /* debug with array and guard */
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_debug_func(tmp, "TestArrDbg", &sf_arr_prim, &cfg));
      /* debug with non-array struct without guard */
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_debug_func(tmp, "TestNoArrDbg", &sf_str, NULL));
      /* debug with non-array struct with guard */
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_debug_func(tmp, "TestNoArrDbgG", &sf_str, &cfg));
      /* test failing at every step in write_struct_debug_func including final
       * #endif */
      {
        int fail_step;
        cdd_c_error_t rc_step;
        for (fail_step = 0; fail_step < 20; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step =
              write_struct_debug_func(tmp, "TestNoArrDbgG", &sf_str, &cfg);
          if (rc_step == CDD_C_SUCCESS) {
            break;
          }
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        for (fail_step = 0; fail_step < 50; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step =
              write_struct_debug_func(tmp, "TestArrDbgG", &sf_arr_prim, &cfg);
          if (rc_step == CDD_C_SUCCESS) {
            break;
          }
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        for (fail_step = 0; fail_step < 20; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step = write_struct_eq_func(tmp, "TestArrEqG", &sf_arr_prim, &cfg);
          if (rc_step == CDD_C_SUCCESS) {
            break;
          }
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        for (fail_step = 0; fail_step < 20; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step =
              write_struct_cleanup_func(tmp, "TestNoArrCleanG", &sf_str, &cfg);
          if (rc_step == CDD_C_SUCCESS) {
            break;
          }
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        for (fail_step = 0; fail_step < 20; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step =
              write_struct_deepcopy_func(tmp, "TestNoArrDeepG", &sf_str, &cfg);
          if (rc_step == CDD_C_SUCCESS) {
            break;
          }
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        for (fail_step = 0; fail_step < 20; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step = write_struct_eq_func(tmp, "TestNoArrEqG", &sf_str, &cfg);
          if (rc_step == CDD_C_SUCCESS) {
            break;
          }
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        for (fail_step = 0; fail_step < 20; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step =
              write_struct_default_func(tmp, "TestNoArrDefG", &sf_str, &cfg);
          if (rc_step == CDD_C_SUCCESS) {
            break;
          }
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        for (fail_step = 0; fail_step < 20; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step =
              write_struct_display_func(tmp, "TestNoArrDispG", &sf_str, &cfg);
          if (rc_step == CDD_C_SUCCESS) {
            break;
          }
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
      }
      g_fail_io_after = -1;
      g_io_calls = 0;

      {
        struct StructFields sf_dc;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_dc));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_dc, "obj_empty",
                                                   "object", NULL, NULL, NULL));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_dc, "arr_empty", "array",
                                                   NULL, NULL, NULL));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_dc, "arr_slash", "array",
                                                   "#/", NULL, NULL));
        ASSERT_EQ(CDD_C_SUCCESS,
                  write_struct_deepcopy_func(tmp, "TestDcEdge", &sf_dc, NULL));
        struct_fields_free(&sf_dc);
      }

      /* Test serde_default_fn in write_struct_default_func and serde_skip in
       * write_struct_debug_func */
      {
        struct StructFields sf_df;
        int fail_step;
        cdd_c_error_t rc_step;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_df));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_df, "fn_val", "integer",
                                                   NULL, NULL, NULL));
        CDD_STRCPY(sf_df.fields[0].serde_default_fn,
                   sizeof(sf_df.fields[0].serde_default_fn), "init_fn_val");
        sf_df.fields[0].serde_skip = 1;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_df, "enum_no_ref",
                                                   "enum", NULL, NULL, NULL));
        ASSERT_EQ(CDD_C_SUCCESS,
                  write_struct_default_func(tmp, "TestDefFn", &sf_df, NULL));
        ASSERT_EQ(CDD_C_SUCCESS,
                  write_struct_debug_func(tmp, "TestDefFn", &sf_df, NULL));
        for (fail_step = 0; fail_step < 20; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step = write_struct_default_func(tmp, "TestDefFn", &sf_df, NULL);
          if (rc_step == CDD_C_SUCCESS)
            break;
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        g_fail_io_after = -1;
        struct_fields_free(&sf_df);
      }

      struct_fields_free(&sf_ref);
      struct_fields_free(&sf_arr);
      struct_fields_free(&sf_arr_prim);
      struct_fields_free(&sf_empty);
      struct_fields_free(&sf_str);
      struct_fields_free(&sf_obj);
      struct_fields_free(&sf_enum);
      fclose(tmp);
    }
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_STRUCT_FIELDS_H */
