/**
 * @file test_codegen_struct_lifecycle.h
 * @brief Unit tests for basic struct lifecycle code generation (cleanup,
 * default, deepcopy, eq, debug, guards, and invalid arguments).
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_LIFECYCLE_H
#define TEST_CODEGEN_STRUCT_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_struct_common.h"
/* clang-format on */

/**
 * @brief test_cleanup_generation
 * @return TEST
 */
TEST test_cleanup_generation(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    long sz;
    char *content = NULL;

    ASSERT(tmp);
    ASSERT_EQ(CDD_C_SUCCESS, setup_struct_fields(&sf));

    ASSERT_EQ(0, write_struct_cleanup_func(tmp, "User", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);

    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "cdd_c_error_t User_cleanup(struct User *obj)"));
    ASSERT(strstr(content, "if (!obj) return CDD_C_SUCCESS;"));
    ASSERT(strstr(content, "if (obj->name) free((void*)obj->name);"));
    ASSERT(strstr(content, "free(obj);"));

    free(content);
    struct_fields_free(NULL);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_default_generation
 * @return TEST
 */
TEST test_default_generation(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    ASSERT_EQ(CDD_C_SUCCESS, setup_struct_fields(&sf));

    ASSERT_EQ(0, write_struct_default_func(tmp, "User", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "*out = calloc(1, sizeof(**out));"));
    /* Check literals injected */
    ASSERT(strstr(content, "(*out)->id = 0;"));
    /* Check string duplication */
    ASSERT(strstr(content, "strdup(\"test\");"));

    free(content);
    struct_fields_free(NULL);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_deepcopy_generation
 * @return TEST
 */
TEST test_deepcopy_generation(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    ASSERT_EQ(CDD_C_SUCCESS, setup_struct_fields(&sf));

    ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "User", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

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
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_eq_generation
 * @return TEST
 */
TEST test_eq_generation(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    ASSERT_EQ(CDD_C_SUCCESS, setup_struct_fields(&sf));

    ASSERT_EQ(0, write_struct_eq_func(tmp, "User", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(
        strstr(content, "if (a == b) { *out_eq = 1; return CDD_C_SUCCESS; }"));
    ASSERT(strstr(content, "a->id != b->id"));
    ASSERT(strstr(content, "strcmp(a->name, b->name)"));

    free(content);
    struct_fields_free(NULL);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_guards_injection
 * @return TEST
 */
TEST test_guards_injection(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    struct CodegenStructConfig cfg;
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    ASSERT_EQ(CDD_C_SUCCESS, setup_struct_fields(&sf));
    cfg.guard_macro = cdd_test_mut_str("MY_GUARD");

    ASSERT_EQ(0, write_struct_cleanup_func(tmp, "User", &sf, &cfg));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "#ifdef MY_GUARD"));
    ASSERT(strstr(content, "#endif /* MY_GUARD */"));

    free(content);
    struct_fields_free(NULL);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
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

/**
 * @brief Unit test test_struct_debug_func.
 * @return Greatest test result.
 */
TEST test_struct_debug_func(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    char *content = NULL;
    long sz;

    ASSERT(tmp);
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "id", "integer", NULL, "0", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "name", "string", NULL, "\"test\"", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "val", "number", NULL, "1.0", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "is_active", "boolean", NULL, "1", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "status", "enum", "Status", "1", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "items", "array", "string", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_int", "array",
                                               "integer", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "arr_num", "array", "number", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_enum", "array", "enum",
                                               "Enum", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_bool", "array",
                                               "boolean", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "def_123", "integer", NULL, "123", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "def_hex", "integer", NULL, "0x10", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "arr_obj", "array", "object", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "user", "object", "User", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "unknown", "unknown", NULL, NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_unknown", "array",
                                               "unknown", NULL, NULL));

    ASSERT_EQ(0, write_struct_debug_func(tmp, "TestStruct", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);

    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "TestStruct_debug"));
    ASSERT(strstr(content, "obj->id"));
    ASSERT(strstr(content, "obj->name"));
    ASSERT(strstr(content, "obj->val"));
    ASSERT(strstr(content, "obj->items[i]"));
    ASSERT(strstr(content, "(unknown)"));

    free(content);
    struct_fields_free(NULL);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_struct_invalid_args
 * @return TEST
 */
TEST test_struct_invalid_args(void) {
  struct StructFields sf;
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));

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
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "items", "array", "string", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_int", "array",
                                               "integer", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "arr_num", "array", "number", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_enum", "array", "enum",
                                               "Enum", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_bool", "array",
                                               "boolean", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "def_123", "integer", NULL, "123", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "def_hex", "integer", NULL, "0x10", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "arr_obj", "array", "object", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_unknown", "array",
                                               "unknown", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "fix_str_arr", "array",
                                               "string", NULL, NULL));
    sf.fields[sf.size - 1].fixed_array_len = 3;
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf, "fix_obj_arr", "array",
                                "#/definitions/SubObj", NULL, NULL));
    sf.fields[sf.size - 1].fixed_array_len = 2;
    ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "S", &sf, NULL));
    if (tmp)
      fclose(tmp);
  }

  struct_fields_free(NULL);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_struct_fields_add_bitwidth.
 * @return Greatest test result.
 */
TEST test_struct_fields_add_bitwidth(void) {
  struct StructFields sf;
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));

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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_STRUCT_LIFECYCLE_H */
