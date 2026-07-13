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
  FILE *tmp = tmpfile();
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
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

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
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_write_union_from_json_object
 * @return TEST
 */
TEST test_write_union_from_json_object(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
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
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "malloc(sizeof(struct ObjU))"));
  ASSERT(strstr(content, "match_count"));
  ASSERT(strstr(content, "json_object_get_count"));
  ASSERT(strstr(content, "ret->tag = ObjU_pet;"));
  ASSERT(strstr(content, "Pet_from_jsonObject"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_write_union_from_json
 * @return TEST
 */
TEST test_write_union_from_json(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
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
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "json_parse_string"));
  ASSERT(strstr(content, "case JSONString"));
  ASSERT(strstr(content, "ret->tag = MixU_s;"));
  ASSERT(strstr(content, "case JSONNumber"));
  ASSERT(strstr(content, "ret->tag = MixU_i;"));
  ASSERT(strstr(content, "ret->data.i = (int)num;"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_write_union_array_to_json
 * @return TEST
 */
TEST test_write_union_array_to_json(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "vals", "array", "string", NULL, NULL);

  ASSERT_EQ(0, write_union_to_json_func(tmp, "ArrU", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "case ArrU_vals:"));
  ASSERT(strstr(content, "obj->data.vals.n_vals"));
  ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"[\")"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_write_union_array_from_json
 * @return TEST
 */
TEST test_write_union_array_from_json(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "vals", "array", "string", NULL, NULL);

  ASSERT_EQ(0, write_union_from_json_func(tmp, "ArrU", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "case JSONArray"));
  ASSERT(strstr(content, "json_array_get_count"));
  ASSERT(strstr(content, "ret->data.vals.n_vals"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_write_union_array_cleanup
 * @return TEST
 */
TEST test_write_union_array_cleanup(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "vals", "array", "string", NULL, NULL);

  ASSERT_EQ(0, write_union_cleanup_func(tmp, "ArrU", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "case ArrU_vals:"));
  ASSERT(strstr(content, "for (i = 0; i < obj->data.vals.n_vals"));
  ASSERT(strstr(content, "free(obj->data.vals.vals)"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}
/**
 * @brief test_write_union_cleanup_switch
 * @return TEST
 */
TEST test_write_union_cleanup_switch(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
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
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "switch (obj->tag)"));
  /* Integer should do nothing implicit */
  /* String should free */
  ASSERT(strstr(content, "case U_str:\n      free((void*)obj->data.str);"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/* --- Root Array Tests --- */

/**
 * @brief test_root_array_string_cleanup
 * @return TEST
 */
TEST test_root_array_string_cleanup(void) {
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(0,
            write_root_array_cleanup_func(tmp, "StrArr", "string", NULL, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content,
                "enum cdd_c_error StrArr_cleanup(char **in, size_t len)"));
  ASSERT(strstr(content, "free(in[i])"));
  ASSERT(strstr(content, "free(in)"));

  free(content);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_root_array_int_from_json
 * @return TEST
 */
TEST test_root_array_int_from_json(void) {
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(
      0, write_root_array_from_json_func(tmp, "IntArr", "integer", NULL, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "enum cdd_c_error IntArr_from_json(const char *json, "
                         "int **out, size_t *len)"));
  ASSERT(strstr(content, "malloc(count * sizeof(int))"));
  ASSERT(strstr(content, "json_array_get_number"));

  free(content);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_root_array_obj_to_json
 * @return TEST
 */
TEST test_root_array_obj_to_json(void) {
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(
      0, write_root_array_to_json_func(tmp, "ObjArr", "object", "Obj", NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "Obj_to_json(in[i], &tmp)"));
  ASSERT(strstr(content, "c89stringutils_jasprintf(json_out, \"[\")"));

  free(content);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/* Guard Logic */
TEST test_union_guards(void) {
  struct StructFields sf;
  struct CodegenTypesConfig cfg;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

  cfg.json_guard = "JSON_G";
  cfg.utils_guard = NULL;

  ASSERT_EQ(0, write_union_to_json_func(tmp, "GuardedU", &sf, &cfg));
  ASSERT_EQ(0, write_union_from_json_func(tmp, "GuardedU", &sf, &cfg));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "#ifdef JSON_G"));
  ASSERT(strstr(content, "#endif /* JSON_G */"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_types_null_args
 * @return TEST
 */
TEST test_types_null_args(void) {
  FILE *tmp = tmpfile();
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

  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

#ifdef _WIN32
#else
#endif

TEST test_types_io_fail(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
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

  sf.union_discriminator = (char *)malloc(5);
  strcpy(sf.union_discriminator, "type");
  sf.union_variants =
      (struct UnionVariantMeta *)calloc(15, sizeof(struct UnionVariantMeta));
  sf.n_union_variants = 15;

  sf.union_variants[6].disc_value = (char *)malloc(5);
  strcpy(sf.union_variants[6].disc_value, "obj1");
  sf.union_variants[0].disc_value = (char *)malloc(3);
  strcpy(sf.union_variants[0].disc_value, "id");

  sf.union_variants[6].n_required_props = 3;
  sf.union_variants[6].required_props = (char **)calloc(3, sizeof(char *));
  sf.union_variants[6].required_props[0] = (char *)malloc(3);
  strcpy(sf.union_variants[6].required_props[0], "id");
  sf.union_variants[6].required_props[1] = NULL; /* trigger continue */
  sf.union_variants[6].required_props[2] = (char *)malloc(3);
  strcpy(sf.union_variants[6].required_props[2], "id");

  sf.union_variants[6].n_property_names = 3;
  sf.union_variants[6].property_names = (char **)calloc(3, sizeof(char *));
  sf.union_variants[6].property_names[0] = (char *)malloc(5);
  strcpy(sf.union_variants[6].property_names[0], "data");
  sf.union_variants[6].property_names[1] = NULL; /* trigger continue */
  sf.union_variants[6].property_names[2] = (char *)malloc(5);
  strcpy(sf.union_variants[6].property_names[2], "data");

  sf.union_variants[1].n_required_props = 3;
  sf.union_variants[1].required_props = (char **)calloc(3, sizeof(char *));
  sf.union_variants[1].required_props[0] = NULL;
  sf.union_variants[1].required_props[1] = (char *)malloc(5);
  strcpy(sf.union_variants[1].required_props[1], "bark");
  sf.union_variants[1].required_props[2] = (char *)malloc(5);
  strcpy(sf.union_variants[1].required_props[2], "bite");

  sf.union_variants[1].n_property_names = 0;
  config.json_guard = "ENABLE_JSON";
  config.utils_guard = "ENABLE_UTILS";

  sf.union_is_anyof = 0;

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_to_json_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_cleanup_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_to_json_func(tmp, "int", "integer", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_from_json_func(tmp, "int", "integer", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_from_json_func(tmp, "num", "number", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_from_json_func(tmp, "bool", "boolean", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_from_json_func(tmp, "str", "string", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_from_json_func(tmp, "obj", "object", "Obj", &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_from_json_func(tmp, "unk", "unknown", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_cleanup_func(tmp, "str", "string", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_root_array_cleanup_func(tmp, "obj", "object", "Obj", &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  sf.union_is_anyof = 1;
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_json_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  sf.union_discriminator[0] = '\0';
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  struct_fields_free(&sf);
  struct_fields_init(&sf);
  sf.union_discriminator = (char *)malloc(5);
  strcpy(sf.union_discriminator, "type");
  sf.union_variants = NULL;
  sf.n_union_variants = 0;
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  struct_fields_free(&sf);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, "0", NULL);
  sf.union_discriminator = (char *)malloc(5);
  strcpy(sf.union_discriminator, "type");
  sf.union_variants = NULL;
  sf.n_union_variants = 0;
  for (i = 0; i < 10; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      /* LCOV_EXCL_START */
      break;
    /* LCOV_EXCL_STOP */
  }

  free(sf.union_discriminator);
  sf.union_discriminator = NULL;
  sf.union_variants = NULL;
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  g_fail_io_after = -1;
#endif
  struct_fields_free(&sf);
  g_fail_io_after = -1;
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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_TYPES_H */
