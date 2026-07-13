/**
 * @file test_codegen_oauth2_error.h
 * @brief Unit tests for OAuth2 Error generator.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_OAUTH2_ERROR_H
#define TEST_CODEGEN_OAUTH2_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/oauth2_error.h"
#include "classes/emit/struct.h"
/* clang-format on */

/**
 * @brief Test for OAuth2 error generation
 * @return TEST
 */
/* LCOV_EXCL_START */
TEST test_oauth2_error_generation(void) {
  FILE *tmp = tmpfile();
  /* LCOV_EXCL_STOP */
  struct StructFields sf;
  /* LCOV_EXCL_START */
  char *content = NULL;
  /* LCOV_EXCL_STOP */
  long sz;
  /* LCOV_EXCL_START */
  ASSERT(tmp);
  ASSERT_EQ(0, struct_fields_init(&sf));
  /* LCOV_EXCL_STOP */

  /* Invalid args */
  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            /* LCOV_EXCL_STOP */
            write_oauth2_error_parser_func(NULL, "OAuth2Error", &sf));
  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            /* LCOV_EXCL_STOP */
            write_oauth2_error_parser_func(tmp, NULL, &sf));
  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            /* LCOV_EXCL_STOP */
            write_oauth2_error_parser_func(tmp, "OAuth2Error", NULL));

  /* simulate struct { char* error; char* error_description; } */
  {
    struct StructField f1;
    struct StructField f2;
    /* LCOV_EXCL_START */
    memset(&f1, 0, sizeof(f1));
    memset(&f2, 0, sizeof(f2));
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    strcpy(f1.name, "error");
    strcpy(f1.type, "string");
    sf.fields[sf.size++] = f1;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    strcpy(f2.name, "error_description");
    strcpy(f2.type, "string");
    sf.fields[sf.size++] = f2;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, write_oauth2_error_parser_func(tmp, "OAuth2Error", &sf));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  fread(content, 1, (size_t)sz, tmp);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT(strstr(content, "enum OAuth2ErrorErrorType"));
  ASSERT(strstr(content, "OAuth2Error_parse_oauth2_error"));
  ASSERT(strstr(content, "invalid_grant"));
  ASSERT(strstr(content, "OAuth2Error_ERROR_INVALID_GRANT"));
  ASSERT(strstr(content, "ret->error_description = val;"));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_oauth2_error_generation_non_string(void) {
  FILE *tmp = tmpfile();
  /* LCOV_EXCL_STOP */
  struct StructFields sf;
  /* LCOV_EXCL_START */
  char *content = NULL;
  /* LCOV_EXCL_STOP */
  long sz;
  /* LCOV_EXCL_START */
  ASSERT(tmp);
  ASSERT_EQ(0, struct_fields_init(&sf));
  /* LCOV_EXCL_STOP */

  /* simulate struct with non-string fields */
  {
    struct StructField f1;
    struct StructField f2;
    /* LCOV_EXCL_START */
    memset(&f1, 0, sizeof(f1));
    memset(&f2, 0, sizeof(f2));
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    strcpy(f1.name, "error");
    strcpy(f1.type, "integer");
    sf.fields[sf.size++] = f1;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    strcpy(f2.name, "error_description");
    strcpy(f2.type, "integer");
    sf.fields[sf.size++] = f2;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  ASSERT_EQ(0,
            /* LCOV_EXCL_STOP */
            write_oauth2_error_parser_func(tmp, "OAuth2ErrorNonString", &sf));

  /* LCOV_EXCL_START */
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  fread(content, 1, (size_t)sz, tmp);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT(strstr(content, "enum OAuth2ErrorNonStringErrorType"));
  ASSERT(strstr(content, "OAuth2ErrorNonString_parse_oauth2_error"));
  ASSERT(strstr(content, "invalid_grant"));
  ASSERT(strstr(content, "ret->error = val;") == NULL);
  ASSERT(strstr(content, "ret->error_description = val;") == NULL);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Suite for codegen oauth2 error
 */
/* LCOV_EXCL_START */
SUITE(codegen_oauth2_error_suite) {
  RUN_TEST(test_oauth2_error_generation);
  RUN_TEST(test_oauth2_error_generation_non_string);
}
/* LCOV_EXCL_STOP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_OAUTH2_ERROR_H */
