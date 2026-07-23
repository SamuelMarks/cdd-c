/**
 * @file test_codegen_struct.h
 * @brief Unit tests for Struct Lifecycle generation logic.
 *
 * Verifies that C utility string generation includes necessary null checks,
 * memory allocations, and recursion.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_H
#define TEST_CODEGEN_STRUCT_H

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
/* clang-format on */

static void setup_struct_fields(struct StructFields *sf) {
  struct_fields_init(sf);
  struct_fields_add(sf, "id", "integer", NULL, "0", NULL);
  struct_fields_add(sf, "name", "string", NULL, "\"test\"", NULL);
}

/**
 * @brief test_cleanup_generation
 * @return TEST
 */
TEST test_cleanup_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  long sz;
  char *content = NULL;

  ASSERT(tmp);
  setup_struct_fields(&sf);

  ASSERT_EQ(0, write_struct_cleanup_func(tmp, "User", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "enum cdd_c_error User_cleanup(struct User *obj)"));
  ASSERT(strstr(content, "if (!obj) return;"));
  ASSERT(strstr(content, "if (obj->name) free((void*)obj->name);"));
  ASSERT(strstr(content, "free(obj);"));

  free(content);
  struct_fields_free(NULL);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_default_generation
 * @return TEST
 */
TEST test_default_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  setup_struct_fields(&sf);

  ASSERT_EQ(0, write_struct_default_func(tmp, "User", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "*out = calloc(1, sizeof(**out));"));
  /* Check literals injected */
  ASSERT(strstr(content, "(*out)->id = 0;"));
  /* Check string duplication */
  ASSERT(strstr(content, "strdup(\"test\");"));

  free(content);
  struct_fields_free(NULL);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_deepcopy_generation
 * @return TEST
 */
TEST test_deepcopy_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  setup_struct_fields(&sf);

  ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "User", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "memcpy(*dest, src, sizeof(struct User));"));
  ASSERT(strstr(content, "if (src->name) {"));
#if defined(_MSC_VER)
  ASSERT(strstr(content, "(*dest)->name = _strdup(src->name);"));
#else
  ASSERT(strstr(content, "(*dest)->name = strdup(src->name);"));
#endif

  free(content);
  struct_fields_free(NULL);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_eq_generation
 * @return TEST
 */
TEST test_eq_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  setup_struct_fields(&sf);

  ASSERT_EQ(0, write_struct_eq_func(tmp, "User", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "if (a == b) { *out_eq = 1; return CDD_C_SUCCESS; }"));
  ASSERT(strstr(content, "a->id != b->id"));
  ASSERT(strstr(content, "strcmp(a->name, b->name)"));

  free(content);
  struct_fields_free(NULL);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_guards_injection
 * @return TEST
 */
TEST test_guards_injection(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  struct CodegenStructConfig cfg;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  setup_struct_fields(&sf);
  cfg.guard_macro = "MY_GUARD";

  ASSERT_EQ(0, write_struct_cleanup_func(tmp, "User", &sf, &cfg));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "#ifdef MY_GUARD"));
  ASSERT(strstr(content, "#endif /* MY_GUARD */"));

  free(content);
  struct_fields_free(NULL);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_null_args
 * @return TEST
 */
TEST test_null_args(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_cleanup_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_default_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_deepcopy_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_eq_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_debug_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_display_func(NULL, "U", NULL, NULL));
  g_fail_io_after = -1;
  PASS();
}

TEST test_struct_debug_func(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, "0", NULL);
  struct_fields_add(&sf, "name", "string", NULL, "\"test\"", NULL);
  struct_fields_add(&sf, "val", "number", NULL, "1.0", NULL);
  struct_fields_add(&sf, "is_active", "boolean", NULL, "1", NULL);
  struct_fields_add(&sf, "status", "enum", "Status", "1", NULL);
  struct_fields_add(&sf, "items", "array", "string", NULL, NULL);
  struct_fields_add(&sf, "arr_int", "array", "integer", NULL, NULL);
  struct_fields_add(&sf, "arr_num", "array", "number", NULL, NULL);
  struct_fields_add(&sf, "arr_enum", "array", "enum", "Enum", NULL);
  struct_fields_add(&sf, "arr_bool", "array", "boolean", NULL, NULL);
  struct_fields_add(&sf, "def_123", "integer", NULL, "123", NULL);
  struct_fields_add(&sf, "def_hex", "integer", NULL, "0x10", NULL);
  struct_fields_add(&sf, "arr_obj", "array", "object", NULL, NULL);
  struct_fields_add(&sf, "user", "object", "User", NULL, NULL);
  struct_fields_add(&sf, "unknown", "unknown", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr_unknown", "array", "unknown", NULL, NULL);

  ASSERT_EQ(0, write_struct_debug_func(tmp, "TestStruct", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "TestStruct_debug"));
  ASSERT(strstr(content, "obj->id"));
  ASSERT(strstr(content, "obj->name"));
  ASSERT(strstr(content, "obj->val"));
  ASSERT(strstr(content, "obj->items[i]"));
  ASSERT(strstr(content, "(unknown)"));

  free(content);
  struct_fields_free(NULL);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_struct_invalid_args
 * @return TEST
 */
TEST test_struct_invalid_args(void) {
  struct StructFields sf;
  struct_fields_init(&sf);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_debug_func(NULL, "S", &sf, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_debug_func(stdout, NULL, &sf, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_debug_func(stdout, "S", NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_cleanup_func(NULL, "S", &sf, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_default_func(NULL, "S", &sf, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_deepcopy_func(NULL, "S", &sf, NULL));

  /* also trigger deepcopy array path */
  {
    FILE *tmp = tmpfile();
    struct_fields_add(&sf, "items", "array", "string", NULL, NULL);
    struct_fields_add(&sf, "arr_int", "array", "integer", NULL, NULL);
    struct_fields_add(&sf, "arr_num", "array", "number", NULL, NULL);
    struct_fields_add(&sf, "arr_enum", "array", "enum", "Enum", NULL);
    struct_fields_add(&sf, "arr_bool", "array", "boolean", NULL, NULL);
    struct_fields_add(&sf, "def_123", "integer", NULL, "123", NULL);
    struct_fields_add(&sf, "def_hex", "integer", NULL, "0x10", NULL);
    struct_fields_add(&sf, "arr_obj", "array", "object", NULL, NULL);
    struct_fields_add(&sf, "arr_unknown", "array", "unknown", NULL, NULL);
    ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "S", &sf, NULL));
    fclose(tmp);
  }

  struct_fields_free(NULL);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}
TEST test_struct_fields_add_bitwidth(void) {
  struct StructFields sf;
  struct_fields_init(&sf);

  /* Add bitfield */
  ASSERT_EQ(0, struct_fields_add(&sf, "flag", "integer", NULL, NULL, "3"));
  ASSERT_EQ(1, sf.size);
  ASSERT_STR_EQ("flag", sf.fields[0].name);
  ASSERT_STR_EQ("3", sf.fields[0].bit_width);

  /* Add normal */
  ASSERT_EQ(0, struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL));
  ASSERT_EQ(2, sf.size);
  ASSERT_STR_EQ("\0", sf.fields[1].bit_width);

  struct_fields_free(NULL);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief codegen_struct_suite
 */

TEST test_struct_io_errors(void) {
  FILE *readonly_f = tmpfile();
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
      printf("write_struct_cleanup_func RC %d\\n", _rc);
      ASSERT_EQ(CDD_C_ERROR_IO, _rc);
    }
    g_fail_io_after = 0;
    g_io_calls = 0;
    {
      int _rc = write_struct_default_func(readonly_f, "Test", &sf, &config);
      printf("write_struct_default_func RC %d\\n", _rc);
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

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_struct_fields_init_fail;
extern C_CDD_EXPORT int g_struct_fields_add_fail;
#endif

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
  sf.union_discriminator = (char *)malloc(5);
  strcpy(sf.union_discriminator, "type");
  sf.n_union_variants = 1;
  sf.union_variants =
      (struct UnionVariantMeta *)calloc(1, sizeof(struct UnionVariantMeta));
  sf.union_variants[0].n_property_names = 1;
  sf.union_variants[0].property_names = (char **)calloc(1, sizeof(char *));
  sf.union_variants[0].property_names[0] = (char *)malloc(2);
  strcpy(sf.union_variants[0].property_names[0], "a");
  sf.union_variants[0].n_required_props = 1;
  sf.union_variants[0].required_props = (char **)calloc(1, sizeof(char *));
  sf.union_variants[0].required_props[0] = (char *)malloc(2);
  strcpy(sf.union_variants[0].required_props[0], "a");
  sf.union_variants[0].disc_value = (char *)malloc(5);
  strcpy(sf.union_variants[0].disc_value, "val1");

  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_cleanup_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_display_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_default_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_deepcopy_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_eq_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_debug_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  config.guard_macro = "MY_GUARD";

  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_default_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_deepcopy_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_eq_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_debug_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
  }
  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_display_func(tmp, "Test", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 600; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_cleanup_func(tmp, "Test", &sf, &config);
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

TEST test_struct_fields_init_oom(void) {
  struct StructFields sf = {0};
  struct StructField *f_ptr = NULL;
  char *val = NULL;
#ifdef CDD_BUILD_TESTS
  g_struct_fields_init_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, struct_fields_init(&sf));
  struct_fields_free(&sf); /* hit sf->fields == NULL branch */
  g_struct_fields_init_fail = 0;

  g_struct_fields_add_fail = 2;
  ASSERT_EQ(0, struct_fields_init(&sf));

  sf.capacity = 1;
  sf.size = 1;
  ASSERT_EQ(0, struct_fields_add(&sf, "a", "b", NULL, NULL, NULL));

  sf.capacity = sf.size;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            struct_fields_add(&sf, "c", "d", NULL, NULL, NULL));

  g_struct_fields_add_fail = 0;
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

  /* Free string array null */
  sf.is_union = 1;
  sf.union_is_anyof = 0;
  sf.union_discriminator = (char *)malloc(5);
  strcpy(sf.union_discriminator, "type");
  sf.n_union_variants = 1;
  sf.union_variants =
      (struct UnionVariantMeta *)calloc(1, sizeof(struct UnionVariantMeta));
  /* We leave required_props and property_names NULL */

  /* Force capacity expansion */
  sf.capacity = 1;
  sf.size = 1;
#ifdef CDD_BUILD_TESTS
  g_struct_fields_add_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            struct_fields_add(&sf, "new_field", "string", NULL, NULL, NULL));
  g_struct_fields_add_fail = 0;
#endif

  struct_fields_free(NULL);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

SUITE(codegen_struct_suite) {
  RUN_TEST(test_struct_io_errors);
  RUN_TEST(test_struct_exhaustive_io);
  RUN_TEST(test_struct_fields_init_oom);
  RUN_TEST(test_cleanup_generation);
  RUN_TEST(test_default_generation);
  RUN_TEST(test_deepcopy_generation);
  RUN_TEST(test_eq_generation);
  RUN_TEST(test_guards_injection);
  RUN_TEST(test_null_args);
  RUN_TEST(test_struct_fields_add_bitwidth);
  RUN_TEST(test_struct_debug_func);
  RUN_TEST(test_struct_invalid_args);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_STRUCT_H */
