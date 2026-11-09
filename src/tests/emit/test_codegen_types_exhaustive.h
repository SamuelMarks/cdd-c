#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_codegen_types_exhaustive.h
 * @brief Exhaustive IO unit tests for Advanced Types generation.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_TYPES_EXHAUSTIVE_H
#define TEST_CODEGEN_TYPES_EXHAUSTIVE_H

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

/**
 * @brief Unit test test_types_exhaustive_io.
 * @return Greatest test result.
 */
TEST test_types_exhaustive_io(void) {
#ifdef CDD_BUILD_TESTS
  int i, rc;
  struct StructFields sf;
  struct CodegenTypesConfig config = {0};

  g_cdd_last_alloc_err = struct_fields_init(&sf);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "id", "integer", NULL, "0", NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "data", "string", NULL, NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "arr_num", "array", "number", NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "arr_bool", "array", "boolean", NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "arr_str", "array", "string", NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "arr_obj", "array", "Object", NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "obj1", "object", "Object", NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "arr_null_ref", "array", NULL, NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "arr_int", "array", "integer", NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "arr_enum", "array", "enum", "MyEnum", NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "arr_unk", "array", "unknown", NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "num1", "number", NULL, NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "bool1", "boolean", NULL, NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "enum1", "enum", "MyEnum", NULL, NULL);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "null1", "null", NULL, NULL, NULL);

  sf.union_discriminator = (char *)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_discriminator, 5, "type");
#else
  strcpy(sf.union_discriminator, "type");
#endif
  sf.union_variants =
      (struct UnionVariantMeta *)calloc(15, sizeof(struct UnionVariantMeta));
  sf.n_union_variants = 15;

  sf.union_variants[6].disc_value = (char *)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].disc_value, 5, "obj1");
#else
  strcpy(sf.union_variants[6].disc_value, "obj1");
#endif
  sf.union_variants[0].disc_value = (char *)malloc(3);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[0].disc_value, 3, "id");
#else
  strcpy(sf.union_variants[0].disc_value, "id");
#endif

  sf.union_variants[6].n_required_props = 3;
  sf.union_variants[6].required_props = (char **)calloc(3, sizeof(char *));
  sf.union_variants[6].required_props[0] = (char *)malloc(3);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].required_props[0], 3, "id");
#else
  strcpy(sf.union_variants[6].required_props[0], "id");
#endif
  sf.union_variants[6].required_props[1] = NULL; /* trigger continue */
  sf.union_variants[6].required_props[2] = (char *)malloc(3);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].required_props[2], 3, "id");
#else
  strcpy(sf.union_variants[6].required_props[2], "id");
#endif

  sf.union_variants[6].n_property_names = 3;
  sf.union_variants[6].property_names = (char **)calloc(3, sizeof(char *));
  sf.union_variants[6].property_names[0] = (char *)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].property_names[0], 5, "data");
#else
  strcpy(sf.union_variants[6].property_names[0], "data");
#endif
  sf.union_variants[6].property_names[1] = NULL; /* trigger continue */
  sf.union_variants[6].property_names[2] = (char *)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].property_names[2], 5, "data");
#else
  strcpy(sf.union_variants[6].property_names[2], "data");
#endif

  sf.union_variants[1].n_required_props = 3;
  sf.union_variants[1].required_props = (char **)calloc(3, sizeof(char *));
  sf.union_variants[1].required_props[0] = NULL;
  sf.union_variants[1].required_props[1] = (char *)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[1].required_props[1], 5, "bark");
#else
  strcpy(sf.union_variants[1].required_props[1], "bark");
#endif
  sf.union_variants[1].required_props[2] = (char *)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[1].required_props[2], 5, "bite");
#else
  strcpy(sf.union_variants[1].required_props[2], "bite");
#endif

  sf.union_variants[1].n_property_names = 0;
  config.json_guard = cdd_test_mut_str("ENABLE_JSON");
  config.utils_guard = cdd_test_mut_str("ENABLE_UTILS");

  sf.union_is_anyof = 0;

  for (i = 0; i < 150; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_to_json_func(tmp, "MyUnion", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 150; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 150; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(tmp, "MyUnion", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 150; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_cleanup_func(tmp, "MyUnion", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
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
    rc = write_root_array_to_json_func(tmp, "int", "integer", NULL, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
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
    rc = write_root_array_from_json_func(tmp, "int", "integer", NULL, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
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
    rc = write_root_array_from_json_func(tmp, "num", "number", NULL, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
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
    rc = write_root_array_from_json_func(tmp, "bool", "boolean", NULL, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
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
    rc = write_root_array_from_json_func(tmp, "str", "string", NULL, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
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
    rc = write_root_array_from_json_func(tmp, "obj", "object", "Obj", &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
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
    rc = write_root_array_from_json_func(tmp, "unk", "unknown", NULL, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
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
    rc = write_root_array_cleanup_func(tmp, "str", "string", NULL, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
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
    rc = write_root_array_cleanup_func(tmp, "obj", "object", "Obj", &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }

  sf.union_is_anyof = 1;
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
    rc = write_union_from_json_func(tmp, "MyUnion", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }

  sf.union_discriminator[0] = '\0';
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
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }

  g_cdd_last_alloc_err = struct_fields_free(&sf);
  g_cdd_last_alloc_err = struct_fields_init(&sf);
  sf.union_discriminator = (char *)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_discriminator, 5, "type");
#else
  strcpy(sf.union_discriminator, "type");
#endif
  sf.union_variants = NULL;
  sf.n_union_variants = 0;
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
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }

  g_cdd_last_alloc_err = struct_fields_free(&sf);
  g_cdd_last_alloc_err = struct_fields_init(&sf);
  g_cdd_last_alloc_err =
      struct_fields_add(&sf, "id", "integer", NULL, "0", NULL);
  sf.union_discriminator = (char *)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_discriminator, 5, "type");
#else
  strcpy(sf.union_discriminator, "type");
#endif
  sf.union_variants = NULL;
  sf.n_union_variants = 0;
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
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }

  free(sf.union_discriminator);
  sf.union_discriminator = NULL;
  sf.union_variants = NULL;
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
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }

  g_fail_io_after = -1;

  /* Test internally tagged, adjacently tagged, and untagged union variants */
  {
    struct StructFields sf_tag;
    char *al_arr[2];
    g_cdd_last_alloc_err = struct_fields_init(&sf_tag);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf_tag, "v_obj", "object", "SubObj", NULL, NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf_tag, "v_int", "integer", NULL, NULL, NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf_tag, "v_num", "number", NULL, NULL, NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf_tag, "v_bool", "boolean", NULL, NULL, NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf_tag, "v_str", "string", NULL, NULL, NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf_tag, "v_null", "null", NULL, NULL, NULL);

    al_arr[0] = cdd_test_mut_str("alias_obj");
    al_arr[1] = NULL;
    sf_tag.fields[0].serde_aliases = al_arr;
    sf_tag.fields[0].n_serde_aliases = 2;

    sf_tag.union_variants =
        (struct UnionVariantMeta *)calloc(1, sizeof(struct UnionVariantMeta));
    sf_tag.n_union_variants = 1;

    /* 1. Empty union_discriminator in write_union_to_json_func */
    sf_tag.union_discriminator = (char *)calloc(1, 1);
    {
      FILE *tmp = cdd_test_tmpfile_global();
      g_cdd_last_alloc_err =
          write_union_to_json_func(tmp, "TagU", &sf_tag, &config);
      if (tmp)
        fclose(tmp);
    }

    /* 2. Internally tagged (serde_tag set, serde_content empty) */
#if defined(_MSC_VER)
    strcpy_s(sf_tag.serde_tag, sizeof(sf_tag.serde_tag), "kind");
#else
    strcpy(sf_tag.serde_tag, "kind");
#endif
    sf_tag.serde_content[0] = '\0';
    sf_tag.serde_untagged = 0;
    for (i = 0; i < 60; ++i) {
      FILE *tmp = cdd_test_tmpfile_global();
      g_fail_io_after = i;
      g_io_calls = 0;
      rc = write_union_to_json_func(tmp, "TagU", &sf_tag, &config);
      if (tmp)
        fclose(tmp);
      if (rc == 0)
        break;
    }
    sf_tag.serde_untagged = 1;
    {
      FILE *tmp = cdd_test_tmpfile_global();
      g_fail_io_after = -1;
      g_cdd_last_alloc_err =
          write_union_to_json_func(tmp, "TagU", &sf_tag, &config);
      if (tmp)
        fclose(tmp);
    }

    /* 3. Adjacently tagged (serde_tag + serde_content) */
#if defined(_MSC_VER)
    strcpy_s(sf_tag.serde_content, sizeof(sf_tag.serde_content), "val");
#else
    strcpy(sf_tag.serde_content, "val");
#endif
    sf_tag.serde_untagged = 0;
    for (i = 0; i < 80; ++i) {
      FILE *tmp = cdd_test_tmpfile_global();
      g_fail_io_after = i;
      g_io_calls = 0;
      rc = write_union_to_json_func(tmp, "TagU", &sf_tag, &config);
      if (tmp)
        fclose(tmp);
      if (rc == 0)
        break;
    }
    for (i = 0; i < 80; ++i) {
      FILE *tmp = cdd_test_tmpfile_global();
      g_fail_io_after = i;
      g_io_calls = 0;
      rc = write_union_from_jsonObject_func(tmp, "TagU", &sf_tag, &config);
      if (tmp)
        fclose(tmp);
      if (rc == 0)
        break;
    }

    /* 4. Untagged union with and without config->json_guard */
    sf_tag.serde_untagged = 1;
    {
      FILE *tmp = cdd_test_tmpfile_global();
      g_fail_io_after = -1;
      g_cdd_last_alloc_err =
          write_union_to_json_func(tmp, "TagU", &sf_tag, &config);
      if (tmp)
        fclose(tmp);
    }
    for (i = 0; i < 40; ++i) {
      FILE *tmp = cdd_test_tmpfile_global();
      g_fail_io_after = i;
      g_io_calls = 0;
      rc = write_union_from_jsonObject_func(tmp, "TagU", &sf_tag, &config);
      if (tmp)
        fclose(tmp);
      if (rc == 0)
        break;
    }
    {
      struct CodegenTypesConfig no_guard_cfg = {0};
      FILE *tmp = cdd_test_tmpfile_global();
      g_fail_io_after = -1;
      g_cdd_last_alloc_err =
          write_union_from_jsonObject_func(tmp, "TagU", &sf_tag, &no_guard_cfg);
      if (tmp)
        fclose(tmp);
    }

    sf_tag.fields[0].serde_aliases = NULL;
    sf_tag.fields[0].n_serde_aliases = 0;
    g_cdd_last_alloc_err = struct_fields_free(&sf_tag);
    g_fail_io_after = -1;
    g_io_calls = 0;
  }
#endif
  g_cdd_last_alloc_err = struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the codegen_types_exhaustive_suite test suite.
 */
SUITE(codegen_types_exhaustive_suite) { RUN_TEST(test_types_exhaustive_io); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_TYPES_EXHAUSTIVE_H */
