#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_codegen_jwt.h
 * @brief Unit tests for JWT parser generator.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_JWT_H
#define TEST_CODEGEN_JWT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/jwt.h"
#include "classes/emit/struct.h"
/* clang-format on */

/**
 * @brief Test for JWT generation functionality
 * @return TEST
 */
TEST test_jwt_generation(void) {
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
    ASSERT_EQ(0, struct_fields_init(&sf));

    /* Invalid arguments bounds */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_jwt_func(NULL, "JwtPayload", &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_jwt_func(tmp, NULL, &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_jwt_func(tmp, "JwtPayload", NULL));

    /* simulate struct { char* sub; int exp; } */
    {
      struct StructField f1;
      struct StructField f2;
      memset(&f1, 0, sizeof(f1));
      memset(&f2, 0, sizeof(f2));

#if defined(_MSC_VER)
      strcpy_s(f1.name, sizeof(f1.name), "sub");
#else
      strcpy(f1.name, "sub");
#endif
#if defined(_MSC_VER)
      strcpy_s(f1.type, sizeof(f1.type), "string");
#else
      strcpy(f1.type, "string");
#endif
      sf.fields[sf.size++] = f1;

#if defined(_MSC_VER)
      strcpy_s(f2.name, sizeof(f2.name), "exp");
#else
      strcpy(f2.name, "exp");
#endif
#if defined(_MSC_VER)
      strcpy_s(f2.type, sizeof(f2.type), "integer");
#else
      strcpy(f2.type, "integer");
#endif
      sf.fields[sf.size++] = f2;
    }

    ASSERT_EQ(0, write_struct_from_jwt_func(tmp, "JwtPayload", &sf));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "JwtPayload_from_jwt"));
    ASSERT(strstr(content, "cdd_c_base64url_decode"));
    ASSERT(strstr(content, " JwtPayload_from_json("));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Suite for codegen jwt
 */
SUITE(codegen_jwt_suite) { RUN_TEST(test_jwt_generation); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_JWT_H */
