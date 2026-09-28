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
  RUN_TEST(test_types_edge_cases_no_io);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_TYPES_H */
