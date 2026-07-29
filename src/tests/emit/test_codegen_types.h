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
#include "c_cdd/memory.h"
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
  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_to_json_func(tmp, "MyUnion", &sf, &config);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
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

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);

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
  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_jsonObject_func(tmp, "ObjU", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "C_CDD_MALLOC(sizeof(struct ObjU))"));
  ASSERT(strstr(content, "match_count"));
  ASSERT(strstr(content, "json_object_get_count"));
  ASSERT(strstr(content, "ret->tag = ObjU_pet;"));
  ASSERT(strstr(content, "Pet_from_jsonObject"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);

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

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "MixU", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "json_parse_string"));
  ASSERT(strstr(content, "case JSONString"));
  ASSERT(strstr(content, "ret->tag = MixU_s;"));
  ASSERT(strstr(content, "case JSONNumber"));
  ASSERT(strstr(content, "ret->tag = MixU_i;"));
  ASSERT(strstr(content, "ret->data.i = (int)num;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);

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

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_to_json_func(tmp, "ArrU", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "case ArrU_vals:"));
  ASSERT(strstr(content, "obj->data.vals.n_vals"));
  ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"[\")"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);

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

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "ArrU", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "case JSONArray"));
  ASSERT(strstr(content, "json_array_get_count"));
  ASSERT(strstr(content, "ret->data.vals.n_vals"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);

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

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_cleanup_func(tmp, "ArrU", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "case ArrU_vals:"));
  ASSERT(strstr(content, "for (i = 0; i < obj->data.vals.n_vals"));
  ASSERT(strstr(content, "C_CDD_FREE(obj->data.vals.vals)"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);

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

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_cleanup_func(tmp, "U", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "switch (obj->tag)"));
  /* Integer should do nothing implicit */
  /* String should free */
  ASSERT(
      strstr(content, "case U_str:\n      C_CDD_FREE((void*)obj->data.str);"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);

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
  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_root_array_cleanup_func(tmp, "StrArr", "string", NULL, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content,
                "enum cdd_c_error StrArr_cleanup(char **in, size_t len)"));
  ASSERT(strstr(content, "C_CDD_FREE(in[i])"));
  ASSERT(strstr(content, "C_CDD_FREE(in)"));

  C_CDD_FREE(content);
  fclose(tmp);

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
  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc =
          write_root_array_from_json_func(tmp, "IntArr", "integer", NULL, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "enum cdd_c_error IntArr_from_json(const char *json, "
                         "int **out, size_t *len)"));
  ASSERT(strstr(content, "C_CDD_MALLOC(count * sizeof(int))"));
  ASSERT(strstr(content, "json_array_get_number"));

  C_CDD_FREE(content);
  fclose(tmp);

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
  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_root_array_to_json_func(tmp, "ObjArr", "object", "Obj", NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "Obj_to_json(in[i], &tmp)"));
  ASSERT(strstr(content, "c89stringutils_jasprintf(json_out, \"[\")"));

  C_CDD_FREE(content);
  fclose(tmp);

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

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_to_json_func(tmp, "GuardedU", &sf, &cfg);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }
  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "GuardedU", &sf, &cfg);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "#ifdef JSON_G"));
  ASSERT(strstr(content, "#endif /* JSON_G */"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);

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

  PASS();
}

#ifdef _WIN32
#else
#endif

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

  sf.union_discriminator = (char *)C_CDD_MALLOC(5);
  strcpy(sf.union_discriminator, "type");
  sf.union_variants = (struct UnionVariantMeta *)C_CDD_CALLOC(
      15, sizeof(struct UnionVariantMeta));
  sf.n_union_variants = 15;

  sf.union_variants[6].disc_value = (char *)C_CDD_MALLOC(5);
  strcpy(sf.union_variants[6].disc_value, "obj1");
  sf.union_variants[0].disc_value = (char *)C_CDD_MALLOC(3);
  strcpy(sf.union_variants[0].disc_value, "id");

  sf.union_variants[6].n_required_props = 3;
  sf.union_variants[6].required_props =
      (char **)C_CDD_CALLOC(3, sizeof(char *));
  sf.union_variants[6].required_props[0] = (char *)C_CDD_MALLOC(3);
  strcpy(sf.union_variants[6].required_props[0], "id");
  sf.union_variants[6].required_props[1] = NULL; /* trigger continue */
  sf.union_variants[6].required_props[2] = (char *)C_CDD_MALLOC(3);
  strcpy(sf.union_variants[6].required_props[2], "id");

  sf.union_variants[6].n_property_names = 3;
  sf.union_variants[6].property_names =
      (char **)C_CDD_CALLOC(3, sizeof(char *));
  sf.union_variants[6].property_names[0] = (char *)C_CDD_MALLOC(5);
  strcpy(sf.union_variants[6].property_names[0], "data");
  sf.union_variants[6].property_names[1] = NULL; /* trigger continue */
  sf.union_variants[6].property_names[2] = (char *)C_CDD_MALLOC(5);
  strcpy(sf.union_variants[6].property_names[2], "data");

  sf.union_variants[1].n_required_props = 3;
  sf.union_variants[1].required_props =
      (char **)C_CDD_CALLOC(3, sizeof(char *));
  sf.union_variants[1].required_props[0] = NULL;
  sf.union_variants[1].required_props[1] = (char *)C_CDD_MALLOC(5);
  strcpy(sf.union_variants[1].required_props[1], "bark");
  sf.union_variants[1].required_props[2] = (char *)C_CDD_MALLOC(5);
  strcpy(sf.union_variants[1].required_props[2], "bite");

  sf.union_variants[1].n_property_names = 0;
  config.json_guard = "ENABLE_JSON";
  config.utils_guard = "ENABLE_UTILS";

  sf.union_is_anyof = 0;

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_union_to_json_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_union_from_json_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_union_cleanup_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_root_array_to_json_func(tmp, "int", "integer", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_root_array_from_json_func(tmp, "int", "integer", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_root_array_from_json_func(tmp, "num", "number", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_root_array_from_json_func(tmp, "bool", "boolean", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_root_array_from_json_func(tmp, "str", "string", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_root_array_from_json_func(tmp, "obj", "object", "Obj", &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_root_array_from_json_func(tmp, "unk", "unknown", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_root_array_cleanup_func(tmp, "str", "string", NULL, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_root_array_cleanup_func(tmp, "obj", "object", "Obj", &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  sf.union_is_anyof = 1;
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_union_from_json_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  sf.union_discriminator[0] = '\0';
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  struct_fields_free(&sf);
  struct_fields_init(&sf);
  sf.union_discriminator = (char *)C_CDD_MALLOC(5);
  strcpy(sf.union_discriminator, "type");
  sf.union_variants = NULL;
  sf.n_union_variants = 0;
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  struct_fields_free(&sf);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, "0", NULL);
  sf.union_discriminator = (char *)C_CDD_MALLOC(5);
  strcpy(sf.union_discriminator, "type");
  sf.union_variants = NULL;
  sf.n_union_variants = 0;
  for (i = 0; i < 10; ++i) {
    FILE *tmp = tmpfile();

    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

  C_CDD_FREE(sf.union_discriminator);
  sf.union_discriminator = NULL;
  sf.union_variants = NULL;
  for (i = 0; i < 800; ++i) {
    FILE *tmp = tmpfile();

    rc = write_union_from_jsonObject_func(tmp, "MyUnion", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }

#endif
  struct_fields_free(&sf);

  PASS();
}

TEST test_union_from_json_ambiguous_types(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id1", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "id2", "integer", NULL, NULL, NULL);

  sf.union_is_anyof = 0; // Not anyOf -> ambiguous!

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "Amb", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "return CDD_C_ERROR_INVALID_ARGUMENT;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_ambiguous_types_num(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "num1", "number", NULL, NULL, NULL);
  struct_fields_add(&sf, "num2", "number", NULL, NULL, NULL);

  sf.union_is_anyof = 0; // Not anyOf -> ambiguous!

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "AmbN", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "return CDD_C_ERROR_INVALID_ARGUMENT;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_ambiguous_types_str(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "str1", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "str2", "string", NULL, NULL, NULL);

  sf.union_is_anyof = 0; // Not anyOf -> ambiguous!

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "AmbS", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "return CDD_C_ERROR_INVALID_ARGUMENT;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_ambiguous_types_bool(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "b1", "boolean", NULL, NULL, NULL);
  struct_fields_add(&sf, "b2", "boolean", NULL, NULL, NULL);

  sf.union_is_anyof = 0; // Not anyOf -> ambiguous!

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "AmbB", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "return CDD_C_ERROR_INVALID_ARGUMENT;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_ambiguous_types_array(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "a1", "array", "integer", NULL, NULL);
  struct_fields_add(&sf, "a2", "array", "integer", NULL, NULL);

  sf.union_is_anyof = 0; // Not anyOf -> ambiguous!

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "AmbA", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "return CDD_C_ERROR_INVALID_ARGUMENT;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_property_names(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "obj1", "object", "Obj1", NULL, NULL);
  struct_fields_add(&sf, "obj2", "object", "Obj2", NULL, NULL);

  sf.union_is_anyof = 0;

  sf.n_union_variants = 2;
  sf.union_variants = (struct UnionVariantMeta *)C_CDD_CALLOC(
      2, sizeof(struct UnionVariantMeta));

  // First variant has required properties
  sf.union_variants[0].n_required_props = 2;
  sf.union_variants[0].required_props =
      (char **)C_CDD_CALLOC(2, sizeof(char *));
  sf.union_variants[0].required_props[0] = strdup("req1");
  sf.union_variants[0].required_props[1] = strdup("req2");

  // Second variant has no required, but has optional properties
  sf.union_variants[1].n_property_names = 2;
  sf.union_variants[1].property_names =
      (char **)C_CDD_CALLOC(2, sizeof(char *));
  sf.union_variants[1].property_names[0] = strdup("opt1");
  sf.union_variants[1].property_names[1] = strdup("opt2");

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_jsonObject_func(tmp, "AmbO", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "json_object_has_value(jsonObject, \"opt1\") || "
                         "json_object_has_value(jsonObject, \"opt2\")"));
  ASSERT(strstr(content, "json_object_has_value(jsonObject, \"req1\") && "
                         "json_object_has_value(jsonObject, \"req2\")"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_number_only(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "n1", "number", NULL, NULL, NULL);
  struct_fields_add(&sf, "s1", "string", NULL, NULL, NULL);

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "UNum", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "ret->data.n1 = num;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_integer_only(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "i1", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "s1", "string", NULL, NULL, NULL);

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "UInt", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "ret->data.i1 = (int)num;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_int_and_num(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "i1", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "n1", "number", NULL, NULL, NULL);

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "UNumInt", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "if (num == (int)num)"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_no_bool_or_null(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "s1", "string", NULL, NULL, NULL);

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "UNo", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  // JSONBoolean and JSONNull should return error
  ASSERT(strstr(content, "case JSONBoolean:\n      json_value_free(val);\n     "
                         " return CDD_C_ERROR_INVALID_ARGUMENT;"));
  ASSERT(strstr(content, "case JSONNull:\n      json_value_free(val);\n      "
                         "return CDD_C_ERROR_INVALID_ARGUMENT;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_ambiguous_null(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "nl1", "null", NULL, NULL, NULL);
  struct_fields_add(&sf, "nl2", "null", NULL, NULL, NULL);
  sf.union_is_anyof = 0;

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "AmbNull", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "case JSONNull:\n      json_value_free(val);\n      "
                         "return CDD_C_ERROR_INVALID_ARGUMENT;"));

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_anyof_types(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id1", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "id2", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "b1", "boolean", NULL, NULL, NULL);
  struct_fields_add(&sf, "b2", "boolean", NULL, NULL, NULL);
  struct_fields_add(&sf, "nl1", "null", NULL, NULL, NULL);
  struct_fields_add(&sf, "nl2", "null", NULL, NULL, NULL);

  sf.union_is_anyof = 1; // anyOf -> ambiguous allowed

  {
    int _i;
    int _rc;
    for (_i = 0; _i < 200; ++_i) {

      rewind(tmp);
      _rc = write_union_from_json_func(tmp, "AnyOf", &sf, NULL);
      if (_rc == 0)
        break;
    }

    ASSERT_EQ(0, _rc);
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (content)
    fread(content, 1, sz, tmp);

  // If allowed, it should generate code checking values
  ASSERT(content != NULL);

  C_CDD_FREE(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_coverage_edge_cases(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "name", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr1", "array", "boolean", NULL, NULL);
  struct_fields_add(&sf, "arr2", "array", "string", NULL, NULL);
  struct_fields_add(&sf, "arr3", "array", "number", NULL, NULL);

  sf.union_discriminator = strdup("type");
  sf.union_variants = (struct UnionVariantMeta *)C_CDD_CALLOC(
      1, sizeof(struct UnionVariantMeta));
  sf.n_union_variants = 1;
  sf.union_variants[0].disc_value = NULL; // !disc_val check
  sf.union_variants[0].n_property_names = 1;
  sf.union_variants[0].property_names =
      (char **)C_CDD_CALLOC(1, sizeof(char *));
  sf.union_variants[0].property_names[0] = NULL; // !prop check

  write_union_to_json_func(tmp, "EdgeU", &sf, NULL);
  write_union_from_jsonObject_func(tmp, "EdgeU", &sf, NULL);
  write_union_from_json_func(tmp, "EdgeU", &sf, NULL);
  write_union_cleanup_func(tmp, "EdgeU", &sf, NULL);

  // also add a null type for the null check
  struct_fields_add(&sf, "nl", "null", NULL, NULL, NULL);
  write_union_to_json_func(tmp, "EdgeU", &sf, NULL);

  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_union_from_json_null_property(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  struct UnionVariantMeta meta;
  char *props[2] = {"valid", NULL};

  struct_fields_init(&sf);
  struct_fields_add(&sf, "a", "object", "Obj", NULL, NULL);

  sf.n_union_variants = 1;
  sf.union_variants = (struct UnionVariantMeta *)C_CDD_CALLOC(
      1, sizeof(struct UnionVariantMeta));

  memset(&meta, 0, sizeof(meta));
  meta.n_property_names = 2;
  meta.property_names = props;

  sf.union_variants[0] = meta;

  ASSERT_EQ(0, write_union_from_json_func(tmp, "MyUnion", &sf, NULL));

  C_CDD_FREE(sf.union_variants);
  sf.union_variants = NULL;
  struct_fields_free(&sf);
  fclose(tmp);

  PASS();
}

SUITE(codegen_types_suite) {
  RUN_TEST(test_union_from_json_null_property);
  RUN_TEST(test_union_coverage_edge_cases);
  RUN_TEST(test_union_from_json_anyof_types);
  RUN_TEST(test_union_from_json_int_and_num);
  RUN_TEST(test_union_from_json_no_bool_or_null);
  RUN_TEST(test_union_from_json_ambiguous_null);
  RUN_TEST(test_union_from_json_number_only);
  RUN_TEST(test_union_from_json_integer_only);
  RUN_TEST(test_union_from_json_property_names);
  RUN_TEST(test_union_from_json_ambiguous_types);
  RUN_TEST(test_union_from_json_ambiguous_types_num);
  RUN_TEST(test_union_from_json_ambiguous_types_str);
  RUN_TEST(test_union_from_json_ambiguous_types_bool);
  RUN_TEST(test_union_from_json_ambiguous_types_array);
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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_TYPES_H */
