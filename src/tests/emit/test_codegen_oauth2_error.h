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
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/oauth2_error.h"
#include "classes/emit/struct.h"
/* clang-format on */

TEST test_oauth2_error_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(0, struct_fields_init(&sf));

  /* simulate struct { char* error; char* error_description; } */
  {
    struct StructField f1;
    struct StructField f2;
    memset(&f1, 0, sizeof(f1));
    memset(&f2, 0, sizeof(f2));

    strcpy(f1.name, "error");
    strcpy(f1.type, "string");
    sf.fields[sf.size++] = f1;

    strcpy(f2.name, "error_description");
    strcpy(f2.type, "string");
    sf.fields[sf.size++] = f2;
  }

  ASSERT_EQ(0, write_oauth2_error_parser_func(tmp, "OAuth2Error", &sf));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  fread(content, 1, (size_t)sz, tmp);

  ASSERT(strstr(content, "enum OAuth2ErrorErrorType"));
  ASSERT(strstr(content, "OAuth2Error_parse_oauth2_error"));
  ASSERT(strstr(content, "invalid_grant"));
  ASSERT(strstr(content, "OAuth2Error_ERROR_INVALID_GRANT"));
  ASSERT(strstr(content, "ret->error_description = val;"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

SUITE(codegen_oauth2_error_suite) { RUN_TEST(test_oauth2_error_generation); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_OAUTH2_ERROR_H */