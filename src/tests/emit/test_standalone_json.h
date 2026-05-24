#ifndef TEST_STANDALONE_JSON_H
#define TEST_STANDALONE_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "classes/emit/json.h"
#include "classes/emit/struct.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_standalone_json_gen(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(0, struct_fields_init(&sf));

  /* Invalid args */
  ASSERT_EQ(EINVAL,
            write_struct_from_json_standalone_func(NULL, "MyStruct", &sf));
  ASSERT_EQ(EINVAL, write_struct_from_json_standalone_func(tmp, NULL, &sf));
  ASSERT_EQ(EINVAL,
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

    strcpy(f1.name, "my_str");
    strcpy(f1.type, "string");
    f1.required = 1;
    f1.has_min_len = 1;
    f1.min_len = 1;
    f1.has_max_len = 1;
    f1.max_len = 10;
    sf.fields[sf.size++] = f1;

    strcpy(f2.name, "my_int");
    strcpy(f2.type, "integer");
    f2.required = 1;
    sf.fields[sf.size++] = f2;

    strcpy(f3.name, "my_bool");
    strcpy(f3.type, "boolean");
    f3.required = 1;
    sf.fields[sf.size++] = f3;

    {
      struct StructField f_opt_bool = {0};
      strcpy(f_opt_bool.name, "opt_bool");
      strcpy(f_opt_bool.type, "boolean");
      f_opt_bool.required = 0;
      sf.fields[sf.size++] = f_opt_bool;
    }

    strcpy(f4.name, "my_num");
    strcpy(f4.type, "number");
    f4.required = 1;
    sf.fields[sf.size++] = f4;

    strcpy(f5.name, "my_arr");
    strcpy(f5.type, "array");
    sf.fields[sf.size++] = f5;
  }

  ASSERT_EQ(0, write_struct_from_json_standalone_func(tmp, "MyStruct", &sf));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  fread(content, 1, (size_t)sz, tmp);

  ASSERT(strstr(content, "MyStruct_parse_json"));
  ASSERT(strstr(content, "my_str"));
  ASSERT(strstr(content, "my_int"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

SUITE(standalone_json_suite) { RUN_TEST(test_standalone_json_gen); }

#ifdef __cplusplus
}
#endif

#endif
