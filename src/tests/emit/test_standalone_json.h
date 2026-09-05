#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
#ifndef TEST_STANDALONE_JSON_H
#define TEST_STANDALONE_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd_export.h"
#include "classes/emit/json.h"
#include "classes/emit/struct.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_standalone_json_gen(void) {
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

    /* Invalid args */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_json_standalone_func(NULL, "MyStruct", &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_json_standalone_func(tmp, NULL, &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_json_standalone_func(tmp, "MyStruct", NULL));

    /* empty struct */
    ASSERT_EQ(0, write_struct_from_json_standalone_func(tmp, "MyStruct", &sf));

    /* with string and primitive */
    {
      struct StructField f1 = {0};
      struct StructField f2 = {0};
      struct StructField f3 = {0};
      struct StructField f4 = {0};
      struct StructField f5 = {0};

#if defined(_MSC_VER)
      strcpy_s(f1.name, sizeof(f1.name), "my_str");
#else
      strcpy(f1.name, "my_str");
#endif
#if defined(_MSC_VER)
      strcpy_s(f1.type, sizeof(f1.type), "string");
#else
      strcpy(f1.type, "string");
#endif
      f1.required = 1;
      f1.has_min_len = 1;
      f1.min_len = 1;
      f1.has_max_len = 1;
      f1.max_len = 10;
      sf.fields[sf.size++] = f1;

#if defined(_MSC_VER)
      strcpy_s(f2.name, sizeof(f2.name), "my_int");
#else
      strcpy(f2.name, "my_int");
#endif
#if defined(_MSC_VER)
      strcpy_s(f2.type, sizeof(f2.type), "integer");
#else
      strcpy(f2.type, "integer");
#endif
      f2.required = 1;
      sf.fields[sf.size++] = f2;

#if defined(_MSC_VER)
      strcpy_s(f3.name, sizeof(f3.name), "my_bool");
#else
      strcpy(f3.name, "my_bool");
#endif
#if defined(_MSC_VER)
      strcpy_s(f3.type, sizeof(f3.type), "boolean");
#else
      strcpy(f3.type, "boolean");
#endif
      f3.required = 1;
      sf.fields[sf.size++] = f3;

      {
        struct StructField f_opt_bool = {0};
#if defined(_MSC_VER)
        strcpy_s(f_opt_bool.name, sizeof(f_opt_bool.name), "opt_bool");
#else
        strcpy(f_opt_bool.name, "opt_bool");
#endif
#if defined(_MSC_VER)
        strcpy_s(f_opt_bool.type, sizeof(f_opt_bool.type), "boolean");
#else
        strcpy(f_opt_bool.type, "boolean");
#endif
        f_opt_bool.required = 0;
        sf.fields[sf.size++] = f_opt_bool;
      }

#if defined(_MSC_VER)
      strcpy_s(f4.name, sizeof(f4.name), "my_num");
#else
      strcpy(f4.name, "my_num");
#endif
#if defined(_MSC_VER)
      strcpy_s(f4.type, sizeof(f4.type), "number");
#else
      strcpy(f4.type, "number");
#endif
      f4.required = 1;
      sf.fields[sf.size++] = f4;

#if defined(_MSC_VER)
      strcpy_s(f5.name, sizeof(f5.name), "my_arr");
#else
      strcpy(f5.name, "my_arr");
#endif
#if defined(_MSC_VER)
      strcpy_s(f5.type, sizeof(f5.type), "array");
#else
      strcpy(f5.type, "array");
#endif
      sf.fields[sf.size++] = f5;
    }

    ASSERT_EQ(0, write_struct_from_json_standalone_func(tmp, "MyStruct", &sf));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)(size_t)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "MyStruct_parse_json"));
    ASSERT(strstr(content, "my_str"));
    ASSERT(strstr(content, "my_int"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

SUITE(standalone_json_suite) { RUN_TEST(test_standalone_json_gen); }

#ifdef __cplusplus
}
#endif

#endif
