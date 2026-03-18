/**
 * @file test_codegen_form.h
 * @brief Unit tests for Form Urlencoded generator.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_FORM_H
#define TEST_CODEGEN_FORM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/form.h"
#include "classes/emit/struct.h"

/* clang-format on */
TEST test_form_urlencoded_generation(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(0, struct_fields_init(&sf));

  /* simulate struct { char* user; char* pass; } */
  {
    struct StructField f1;
    struct StructField f2;
    memset(&f1, 0, sizeof(f1));
    memset(&f2, 0, sizeof(f2));

    strcpy(f1.name, "username");
    strcpy(f1.type, "string");
    sf.fields[sf.size++] = f1;

    strcpy(f2.name, "password");
    strcpy(f2.type, "string");
    sf.fields[sf.size++] = f2;
  }

  ASSERT_EQ(0, write_struct_to_form_urlencoded_func(tmp, "LoginRequest", &sf));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  fread(content, 1, (size_t)sz, tmp);

  ASSERT(strstr(content, "LoginRequest_to_form_urlencoded"));
  ASSERT(strstr(content, "LoginRequest_url_encode_form"));
  ASSERT(strstr(content, "obj->username"));
  ASSERT(strstr(content, "obj->password"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

SUITE(codegen_form_suite) { RUN_TEST(test_form_urlencoded_generation); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_FORM_H */