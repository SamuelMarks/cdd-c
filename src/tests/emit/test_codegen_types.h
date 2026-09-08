#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_codegen_types.h
 * @brief Unit tests for Advanced Types (Unions/Arrays) generation.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_TYPES_H
#define TEST_CODEGEN_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/struct.h" /* For struct_fields init helpers */
#include "classes/emit/types.h"
/* clang-format on */

/* --- Union Tests --- */

/**
 * @brief test_write_union_to_json
 * @return TEST
 */
TEST test_write_union_to_json(void) {
  struct StructFields sf;
  struct CodegenTypesConfig config = {0};
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    struct_fields_init(&sf);
    struct_fields_add(&sf, "id", "integer", NULL, NULL, NULL);
    struct_fields_add(&sf, "name", "string", NULL, NULL, NULL);
    struct_fields_add(&sf, "obj", "object", "SomeObj", NULL, NULL);
    struct_fields_add(&sf, "b", "boolean", NULL, NULL, NULL);
    struct_fields_add(&sf, "n", "number", NULL, NULL, NULL);
    struct_fields_add(&sf, "e", "enum", "MyEnum", NULL, NULL);
    struct_fields_add(&sf, "a", "array", "integer", NULL, NULL);
    struct_fields_add(&sf, "nl", "null", NULL, NULL, NULL);

    /* Generate */
    ASSERT_EQ(0, write_union_to_json_func(tmp, "MyUnion", &sf, &config));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    /* Check for switch on tag */
    ASSERT(strstr(content, "switch (obj->tag)"));
    /* Check case for id */
    ASSERT(strstr(content, "case MyUnion_id:"));
    ASSERT(strstr(content, "obj->data.id"));
    /* Check case for name */
    ASSERT(strstr(content, "case MyUnion_name:"));
    ASSERT(strstr(content, "obj->data.name"));
    /* Check case for obj */
    ASSERT(strstr(content, "case MyUnion_obj:"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_write_union_from_json_object
 * @return TEST
 */
TEST test_write_union_from_json_object(void) {
  struct StructFields sf;
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    struct_fields_init(&sf);
    struct_fields_add(&sf, "pet", "object", "Pet", NULL, NULL);

    /* Generate */
    ASSERT_EQ(0, write_union_from_jsonObject_func(tmp, "ObjU", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "malloc(sizeof(struct ObjU))"));
    ASSERT(strstr(content, "match_count"));
    ASSERT(strstr(content, "json_object_get_count"));
    ASSERT(strstr(content, "ret->tag = ObjU_pet;"));
    ASSERT(strstr(content, "Pet_from_jsonObject"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_write_union_from_json
 * @return TEST
 */
TEST test_write_union_from_json(void) {
  struct StructFields sf;
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    struct_fields_init(&sf);
    struct_fields_add(&sf, "s", "string", NULL, NULL, NULL);
    struct_fields_add(&sf, "i", "integer", NULL, NULL, NULL);
    struct_fields_add(&sf, "b", "boolean", NULL, NULL, NULL);
    struct_fields_add(&sf, "n", "number", NULL, NULL, NULL);
    struct_fields_add(&sf, "e", "enum", "MyEnum", NULL, NULL);
    struct_fields_add(&sf, "a", "array", "integer", NULL, NULL);
    struct_fields_add(&sf, "nl", "null", NULL, NULL, NULL);

    sf.union_is_anyof = 1;

    ASSERT_EQ(0, write_union_from_json_func(tmp, "MixU", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "json_parse_string"));
    ASSERT(strstr(content, "case JSONString"));
    ASSERT(strstr(content, "ret->tag = MixU_s;"));
    ASSERT(strstr(content, "case JSONNumber"));
    ASSERT(strstr(content, "ret->tag = MixU_i;"));
    ASSERT(strstr(content, "ret->data.i = (int)num;"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_write_union_array_to_json
 * @return TEST
 */
TEST test_write_union_array_to_json(void) {
  struct StructFields sf;
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    struct_fields_init(&sf);
    struct_fields_add(&sf, "vals", "array", "string", NULL, NULL);

    ASSERT_EQ(0, write_union_to_json_func(tmp, "ArrU", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "case ArrU_vals:"));
    ASSERT(strstr(content, "obj->data.vals.n_vals"));
    ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"[\")"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_write_union_array_from_json
 * @return TEST
 */
TEST test_write_union_array_from_json(void) {
  struct StructFields sf;
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    struct_fields_init(&sf);
    struct_fields_add(&sf, "vals", "array", "string", NULL, NULL);

    ASSERT_EQ(0, write_union_from_json_func(tmp, "ArrU", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "case JSONArray"));
    ASSERT(strstr(content, "json_array_get_count"));
    ASSERT(strstr(content, "ret->data.vals.n_vals"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_write_union_array_cleanup
 * @return TEST
 */
TEST test_write_union_array_cleanup(void) {
  struct StructFields sf;
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    struct_fields_init(&sf);
    struct_fields_add(&sf, "vals", "array", "string", NULL, NULL);

    ASSERT_EQ(0, write_union_cleanup_func(tmp, "ArrU", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "case ArrU_vals:"));
    ASSERT(strstr(content, "for (i = 0; i < obj->data.vals.n_vals"));
    ASSERT(strstr(content, "free(obj->data.vals.vals)"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}
/**
 * @brief test_write_union_cleanup_switch
 * @return TEST
 */
TEST test_write_union_cleanup_switch(void) {
  struct StructFields sf;
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    struct_fields_init(&sf);
    struct_fields_add(&sf, "str", "string", NULL, NULL, NULL);
    struct_fields_add(&sf, "num", "integer", NULL, NULL, NULL);

    ASSERT_EQ(0, write_union_cleanup_func(tmp, "U", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "switch (obj->tag)"));
    /* Integer should do nothing implicit */
    /* String should free */
    ASSERT(strstr(content, "case U_str:\n      free((void*)obj->data.str);"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/* --- Root Array Tests --- */

/**
 * @brief test_root_array_string_cleanup
 * @return TEST
 */
TEST test_root_array_string_cleanup(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    ASSERT_EQ(
        0, write_root_array_cleanup_func(tmp, "StrArr", "string", NULL, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(
        strstr(content, "cdd_c_error_t StrArr_cleanup(char **in, size_t len)"));
    ASSERT(strstr(content, "free(in[i])"));
    ASSERT(strstr(content, "free(in)"));

    free(content);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_root_array_int_from_json
 * @return TEST
 */
TEST test_root_array_int_from_json(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    ASSERT_EQ(0, write_root_array_from_json_func(tmp, "IntArr", "integer", NULL,
                                                 NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "cdd_c_error_t IntArr_from_json(const char *json, "
                           "int **out, size_t *len)"));
    ASSERT(strstr(content, "malloc(count * sizeof(int))"));
    ASSERT(strstr(content, "json_array_get_number"));

    free(content);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_root_array_obj_to_json
 * @return TEST
 */
TEST test_root_array_obj_to_json(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    ASSERT_EQ(
        0, write_root_array_to_json_func(tmp, "ObjArr", "object", "Obj", NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "Obj_to_json(in[i], &tmp)"));
    ASSERT(strstr(content, "c89stringutils_jasprintf(json_out, \"[\")"));

    free(content);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/* Guard Logic */
TEST test_union_guards(void) {
  struct StructFields sf;
  struct CodegenTypesConfig cfg;
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    struct_fields_init(&sf);
    struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

    cfg.json_guard = (char *)(size_t)(size_t) "JSON_G";
    cfg.utils_guard = NULL;

    ASSERT_EQ(0, write_union_to_json_func(tmp, "GuardedU", &sf, &cfg));
    ASSERT_EQ(0, write_union_from_json_func(tmp, "GuardedU", &sf, &cfg));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "#ifdef JSON_G"));
    ASSERT(strstr(content, "#endif /* JSON_G */"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_types_null_args
 * @return TEST
 */
TEST test_types_null_args(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_cleanup_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_cleanup_func(tmp, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_cleanup_func(tmp, "U", NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_from_json_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_from_json_func(tmp, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_from_json_func(tmp, "U", NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_from_jsonObject_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_from_jsonObject_func(tmp, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_from_jsonObject_func(tmp, "U", NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_to_json_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_to_json_func(tmp, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_union_to_json_func(tmp, "U", NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_root_array_cleanup_func(NULL, "A", "T", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_root_array_cleanup_func(tmp, NULL, "T", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_root_array_cleanup_func(tmp, "A", NULL, NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_root_array_to_json_func(NULL, "A", "T", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_root_array_to_json_func(tmp, NULL, "T", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_root_array_to_json_func(tmp, "A", NULL, NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_root_array_from_json_func(NULL, "A", "T", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_root_array_from_json_func(tmp, NULL, "T", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_root_array_from_json_func(tmp, "A", NULL, NULL, NULL));

  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

#ifdef _WIN32
#else
#endif

TEST test_types_io_fail(void) {
  struct StructFields sf;
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  g_fail_io_after = 0;
  g_io_calls = 0;
  struct_fields_init(&sf);
  struct_fields_add(&sf, "string", "t", "s", 0, 0);

  ASSERT(tmp);
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO, write_union_to_json_func(tmp, "U", &sf, NULL));
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO, write_union_from_json_func(tmp, "U", &sf, NULL));
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO, write_union_cleanup_func(tmp, "U", &sf, NULL));

  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO,
            write_root_array_to_json_func(tmp, "A", "string", NULL, NULL));
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO,
            write_root_array_from_json_func(tmp, "A", "string", NULL, NULL));
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO,
            write_root_array_cleanup_func(tmp, "A", "string", NULL, NULL));

  struct_fields_free(&sf);
  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief codegen_types_suite
 */

#ifdef CDD_BUILD_TESTS
#endif

TEST test_types_exhaustive_io(void) {
#ifdef CDD_BUILD_TESTS
  int i, rc;
  struct StructFields sf;
  struct CodegenTypesConfig config = {0};

  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, "0", NULL);
  struct_fields_add(&sf, "data", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr_num", "array", "number", NULL, NULL);
  struct_fields_add(&sf, "arr_bool", "array", "boolean", NULL, NULL);
  struct_fields_add(&sf, "arr_str", "array", "string", NULL, NULL);
  struct_fields_add(&sf, "arr_obj", "array", "Object", NULL, NULL);
  struct_fields_add(&sf, "obj1", "object", "Object", NULL, NULL);
  struct_fields_add(&sf, "arr_null_ref", "array", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr_int", "array", "integer", NULL, NULL);
  struct_fields_add(&sf, "arr_enum", "array", "enum", "MyEnum", NULL);
  struct_fields_add(&sf, "arr_unk", "array", "unknown", NULL, NULL);
  struct_fields_add(&sf, "num1", "number", NULL, NULL, NULL);
  struct_fields_add(&sf, "bool1", "boolean", NULL, NULL, NULL);
  struct_fields_add(&sf, "enum1", "enum", "MyEnum", NULL, NULL);
  struct_fields_add(&sf, "null1", "null", NULL, NULL, NULL);

  sf.union_discriminator = (char *)(size_t)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_discriminator, 5, "type");
#else
  strcpy(sf.union_discriminator, "type");
#endif
  sf.union_variants =
      (struct UnionVariantMeta *)calloc(15, sizeof(struct UnionVariantMeta));
  sf.n_union_variants = 15;

  sf.union_variants[6].disc_value = (char *)(size_t)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].disc_value, 5, "obj1");
#else
  strcpy(sf.union_variants[6].disc_value, "obj1");
#endif
  sf.union_variants[0].disc_value = (char *)(size_t)malloc(3);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[0].disc_value, 3, "id");
#else
  strcpy(sf.union_variants[0].disc_value, "id");
#endif

  sf.union_variants[6].n_required_props = 3;
  sf.union_variants[6].required_props = (char **)calloc(3, sizeof(char *));
  sf.union_variants[6].required_props[0] = (char *)(size_t)malloc(3);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].required_props[0], 3, "id");
#else
  strcpy(sf.union_variants[6].required_props[0], "id");
#endif
  sf.union_variants[6].required_props[1] = NULL; /* trigger continue */
  sf.union_variants[6].required_props[2] = (char *)(size_t)malloc(3);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].required_props[2], 3, "id");
#else
  strcpy(sf.union_variants[6].required_props[2], "id");
#endif

  sf.union_variants[6].n_property_names = 3;
  sf.union_variants[6].property_names = (char **)calloc(3, sizeof(char *));
  sf.union_variants[6].property_names[0] = (char *)(size_t)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].property_names[0], 5, "data");
#else
  strcpy(sf.union_variants[6].property_names[0], "data");
#endif
  sf.union_variants[6].property_names[1] = NULL; /* trigger continue */
  sf.union_variants[6].property_names[2] = (char *)(size_t)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[6].property_names[2], 5, "data");
#else
  strcpy(sf.union_variants[6].property_names[2], "data");
#endif

  sf.union_variants[1].n_required_props = 3;
  sf.union_variants[1].required_props = (char **)calloc(3, sizeof(char *));
  sf.union_variants[1].required_props[0] = NULL;
  sf.union_variants[1].required_props[1] = (char *)(size_t)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[1].required_props[1], 5, "bark");
#else
  strcpy(sf.union_variants[1].required_props[1], "bark");
#endif
  sf.union_variants[1].required_props[2] = (char *)(size_t)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_variants[1].required_props[2], 5, "bite");
#else
  strcpy(sf.union_variants[1].required_props[2], "bite");
#endif

  sf.union_variants[1].n_property_names = 0;
  config.json_guard = (char *)(size_t)(size_t) "ENABLE_JSON";
  config.utils_guard = (char *)(size_t)(size_t) "ENABLE_UTILS";

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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
  }

  struct_fields_free(&sf);
  struct_fields_init(&sf);
  sf.union_discriminator = (char *)(size_t)malloc(5);
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
  }

  struct_fields_free(&sf);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, "0", NULL);
  sf.union_discriminator = (char *)(size_t)malloc(5);
#if defined(_MSC_VER)
  strcpy_s(sf.union_discriminator, 5, "type");
#else
  strcpy(sf.union_discriminator, "type");
#endif
  sf.union_variants = NULL;
  sf.n_union_variants = 0;
  for (i = 0; i < 10; ++i) {
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
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
    if (i == 1999)
      printf("WARNING: Loop reached 2000!\n");
  }

  g_fail_io_after = -1;
#endif
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

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

TEST test_types_edge_cases_no_io(void) {
  struct StructFields sf;
  struct CodegenTypesConfig config = {0};
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  /* Cover sf.union_variants && i < sf->n_union_variants where i >=
   * n_union_variants */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "obj1", "object", "Obj1", NULL, NULL);
  struct_fields_add(&sf, "obj2", "object", "Obj2", NULL, NULL);

  sf.union_discriminator = strdup("type");
  sf.union_variants =
      (struct UnionVariantMeta *)calloc(1, sizeof(struct UnionVariantMeta));
  sf.n_union_variants =
      1; /* sf.size is 2, so i=1 will fail i < sf->n_union_variants */
  sf.union_variants[0].disc_value = strdup("obj1");

  write_union_from_jsonObject_func(tmp, "Union1", &sf, &config);

  struct_fields_free(&sf);

  /* Cover jtype == UNION_JSON_UNKNOWN && t where t == NULL */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "null_type", NULL, NULL, NULL, NULL);
  sf.union_is_anyof = 0;
  write_union_from_jsonObject_func(tmp, "Union1", &sf, &config);
  struct_fields_free(&sf);

  /* Cover num_count > 1 when int_count <= 1 AND !union_is_anyof */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "n1", "number", NULL, NULL, NULL);
  struct_fields_add(&sf, "n2", "number", NULL, NULL, NULL);
  sf.union_is_anyof = 0;
  write_union_from_jsonObject_func(tmp, "Union1", &sf, &config);
  struct_fields_free(&sf);

  /* Cover string_count > 1 false when !union_is_anyof */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "s1", "string", NULL, NULL, NULL);
  sf.union_is_anyof = 0;
  write_union_from_jsonObject_func(tmp, "Union1", &sf, &config);
  struct_fields_free(&sf);

  /* Cover int_count == 0 && num_count == 0 fallback */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "b1", "boolean", NULL, NULL, NULL);
  sf.union_is_anyof = 0;
  write_union_from_jsonObject_func(tmp, "Union1", &sf, &config);
  struct_fields_free(&sf);

  /* Cover strcmp(type, "null") == 0 false branch */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "unk1", "unknown", NULL, NULL, NULL);
  write_union_to_json_func(tmp, "Union1", &sf, &config);
  struct_fields_free(&sf);

  if (tmp)
    fclose(tmp);
  PASS();
}

SUITE(codegen_types_suite) {
  RUN_TEST(test_types_exhaustive_io);
  RUN_TEST(test_write_union_to_json);
  RUN_TEST(test_write_union_from_json_object);
  RUN_TEST(test_write_union_from_json);
  RUN_TEST(test_write_union_array_to_json);
  RUN_TEST(test_write_union_array_from_json);
  RUN_TEST(test_write_union_array_cleanup);
  RUN_TEST(test_write_union_cleanup_switch);
  RUN_TEST(test_root_array_string_cleanup);
  RUN_TEST(test_root_array_int_from_json);
  RUN_TEST(test_root_array_obj_to_json);
  RUN_TEST(test_union_guards);
  RUN_TEST(test_types_null_args);
  RUN_TEST(test_types_io_fail);
  RUN_TEST(test_types_uncovered);
  RUN_TEST(test_types_edge_cases_no_io);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_TYPES_H */
