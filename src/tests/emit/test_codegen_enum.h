#ifndef TEST_CODEGEN_ENUM_H
#define TEST_CODEGEN_ENUM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/emit/enum.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

TEST test_enum_generation(void) {
  FILE *tmp = tmpfile();
  struct EnumMembers em;
  char *content = NULL;
  long sz;
  struct CodegenEnumConfig config = {"MY_GUARD"};
  int i;

  ASSERT(tmp);

  ASSERT_EQ(0, enum_members_init(&em));

  /* Invalid args */
  ASSERT_EQ(EINVAL, enum_members_init(NULL));
  enum_members_free(NULL); /* Should not crash */
  ASSERT_EQ(EINVAL, enum_members_add(NULL, "VAL"));
  ASSERT_EQ(EINVAL, enum_members_add(&em, NULL));

  ASSERT_EQ(EINVAL, write_enum_to_str_func(NULL, "MyEnum", &em, &config));
  ASSERT_EQ(EINVAL, write_enum_to_str_func(tmp, NULL, &em, &config));
  ASSERT_EQ(EINVAL, write_enum_to_str_func(tmp, "MyEnum", NULL, &config));

  ASSERT_EQ(EINVAL, write_enum_from_str_func(NULL, "MyEnum", &em, &config));
  ASSERT_EQ(EINVAL, write_enum_from_str_func(tmp, NULL, &em, &config));
  ASSERT_EQ(EINVAL, write_enum_from_str_func(tmp, "MyEnum", NULL, &config));

  {
    struct EnumMembers bad_em = {0};
    ASSERT_EQ(EINVAL, write_enum_to_str_func(tmp, "MyEnum", &bad_em, &config));
    ASSERT_EQ(EINVAL,
              write_enum_from_str_func(tmp, "MyEnum", &bad_em, &config));
  }

  ASSERT_EQ(0, enum_members_add(&em, "VAL1"));
  ASSERT_EQ(0, enum_members_add(&em, "UNKNOWN"));
  ASSERT_EQ(0, enum_members_add(&em, "VAL2"));

  /* Force realloc to increase capacity to test the branch */
  for (i = 0; i < 10; ++i) {
    char buf[16];
    sprintf(buf, "VAL%d", i + 3);
    ASSERT_EQ(0, enum_members_add(&em, buf));
  }

  /* Test OOM during string duplication by failing strdup somehow? We can fail
   * the next realloc instead */
  {
    size_t old_cap = em.capacity;
    while (em.size < em.capacity) {
      ASSERT_EQ(0, enum_members_add(&em, "FILL"));
    }
    /* We can't actually easily fail it without a mock, so we just let it
     * increase */
    ASSERT_EQ(0, enum_members_add(&em, "OOM"));
  }

  ASSERT_EQ(0, write_enum_to_str_func(tmp, "MyEnum", &em, &config));
  ASSERT_EQ(0, write_enum_from_str_func(tmp, "MyEnum", &em, &config));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  fread(content, 1, (size_t)sz, tmp);

  ASSERT(strstr(content, "int MyEnum_to_str"));
  ASSERT(strstr(content, "int MyEnum_from_str"));
  ASSERT(strstr(content, "#ifdef MY_GUARD"));
  ASSERT(strstr(content, "#endif /* MY_GUARD */"));

  {
    FILE *readonly_f = fopen("test_codegen_enum_ro.txt", "w");
    fclose(readonly_f);
    readonly_f = fopen("test_codegen_enum_ro.txt", "r");
    if (readonly_f) {
      ASSERT_EQ(EIO,
                write_enum_to_str_func(readonly_f, "MyEnum", &em, &config));
      ASSERT_EQ(EIO,
                write_enum_from_str_func(readonly_f, "MyEnum", &em, &config));
      fclose(readonly_f);
    }
    remove("test_codegen_enum_ro.txt");
  }

  free(content);
  enum_members_free(&em);
  fclose(tmp);
  PASS();
}

SUITE(codegen_enum_suite) { RUN_TEST(test_enum_generation); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_ENUM_H */
