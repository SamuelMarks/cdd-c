extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_codegen_form.h
 * @brief Unit tests for generating URL encoded form functions.
 */

#ifndef TEST_CODEGEN_FORM_H
#define TEST_CODEGEN_FORM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/form.h"
#include "classes/emit/struct.h"
/* clang-format on */

/**
 * @brief test_form_generation_basic
 * @return TEST
 */
TEST test_form_generation_basic(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(0, struct_fields_init(&sf));

  ASSERT_EQ(EINVAL, write_struct_to_form_urlencoded_func(NULL, "Form", &sf));
  ASSERT_EQ(EINVAL, write_struct_to_form_urlencoded_func(tmp, NULL, &sf));
  ASSERT_EQ(EINVAL, write_struct_to_form_urlencoded_func(tmp, "Form", NULL));

  /* add fields */
  {
    struct StructField f1, f2, f3;
    memset(&f1, 0, sizeof(f1));
    memset(&f2, 0, sizeof(f2));
    memset(&f3, 0, sizeof(f3));

    strcpy(f1.name, "grant_type");
    strcpy(f1.type, "string");
    sf.fields[sf.size++] = f1;

    strcpy(f2.name, "count");
    strcpy(f2.type, "integer");
    sf.fields[sf.size++] = f2;

    strcpy(f3.name, "active");
    strcpy(f3.type, "boolean");
    sf.fields[sf.size++] = f3;
  }

  ASSERT_EQ(0, write_struct_to_form_urlencoded_func(tmp, "AuthRequest", &sf));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, (size_t)sz + 1);
  fread(content, 1, (size_t)sz, tmp);

  ASSERT(strstr(content, "AuthRequest_url_encode_form"));
  ASSERT(strstr(content, "AuthRequest_to_form_urlencoded"));
  ASSERT(strstr(content, "grant_type"));
  ASSERT(strstr(content, "count"));
  ASSERT(strstr(content, "active"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief codegen_form_suite
 */
SUITE(codegen_form_suite) { RUN_TEST(test_form_generation_basic); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_FORM_H */
