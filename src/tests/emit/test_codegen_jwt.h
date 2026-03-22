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
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/jwt.h"
#include "classes/emit/struct.h"
/* clang-format on */

TEST test_jwt_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(0, struct_fields_init(&sf));

  /* simulate struct { char* sub; int exp; } */
  {
    struct StructField f1;
    struct StructField f2;
    memset(&f1, 0, sizeof(f1));
    memset(&f2, 0, sizeof(f2));

    strcpy(f1.name, "sub");
    strcpy(f1.type, "string");
    sf.fields[sf.size++] = f1;

    strcpy(f2.name, "exp");
    strcpy(f2.type, "integer");
    sf.fields[sf.size++] = f2;
  }

  ASSERT_EQ(0, write_struct_from_jwt_func(tmp, "JwtPayload", &sf));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  fread(content, 1, (size_t)sz, tmp);

  ASSERT(strstr(content, "JwtPayload_from_jwt"));
  ASSERT(strstr(content, "cdd_c_base64url_decode"));
  ASSERT(strstr(content, " JwtPayload_from_json("));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

SUITE(codegen_jwt_suite) { RUN_TEST(test_jwt_generation); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_JWT_H */