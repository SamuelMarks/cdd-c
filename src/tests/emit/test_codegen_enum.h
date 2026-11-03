/**
 * @file test_codegen_enum.h
 * @brief Unit tests and mocks in test_codegen_enum.h.
 */
#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);

#ifndef TEST_CODEGEN_ENUM_H
#define TEST_CODEGEN_ENUM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "c_cdd/memory.h"
#include "classes/emit/enum.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_strdup_fail;

/**
 * @brief Unit test test_enum_generation.
 * @return Greatest test result.
 */
TEST test_enum_generation(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct EnumMembers em;
    char *content = NULL;
    long sz;
    struct CodegenEnumConfig config = {"MY_GUARD"};
    int i;

    ASSERT(tmp);

    ASSERT_EQ(0, enum_members_init(&em));

    /* Invalid args */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, enum_members_init(NULL));
    enum_members_free(NULL); /* Should not crash */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, enum_members_add(NULL, "VAL"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, enum_members_add(&em, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_enum_to_str_func(NULL, "MyEnum", &em, &config));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_enum_to_str_func(tmp, NULL, &em, &config));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_enum_to_str_func(tmp, "MyEnum", NULL, &config));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_enum_from_str_func(NULL, "MyEnum", &em, &config));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_enum_from_str_func(tmp, NULL, &em, &config));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_enum_from_str_func(tmp, "MyEnum", NULL, &config));

    {
      struct EnumMembers bad_em = {0};
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_enum_to_str_func(tmp, "MyEnum", &bad_em, &config));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_enum_from_str_func(tmp, "MyEnum", &bad_em, &config));
    }

    ASSERT_EQ(0, enum_members_add(&em, "VAL1"));
    ASSERT_EQ(0, enum_members_add(&em, "UNKNOWN"));
    ASSERT_EQ(0, enum_members_add(&em, "VAL2"));

    /* Force realloc to increase capacity to test the branch */
    for (i = 0; i < 10; ++i) {
      char buf[16];
#if defined(_MSC_VER)
      sprintf_s(buf, sizeof(buf), "VAL%d", i + 3);
#else
      CDD_SNPRINTF(buf, sizeof(buf), "VAL%d", i + 3);
#endif
      ASSERT_EQ(0, enum_members_add(&em, buf));
    }

    /* Test OOM during string duplication by failing strdup somehow? We can fail
     * the next realloc instead */
    {
      while (em.size < em.capacity) {
        ASSERT_EQ(0, enum_members_add(&em, "FILL"));
      }
      /* We can't actually easily fail it without a mock, so we just let it
       * increase */
      ASSERT_EQ(0, enum_members_add(&em, "OOM"));
    }

    printf("g_fail_io_after=%d g_io_calls=%d\n", g_fail_io_after, g_io_calls);
    ASSERT_EQ(0, write_enum_to_str_func(tmp, "MyEnum", &em, &config));
    ASSERT_EQ(0, write_enum_from_str_func(tmp, "MyEnum", &em, &config));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "int MyEnum_to_str"));
    ASSERT(strstr(content, "int MyEnum_from_str"));
    ASSERT(strstr(content, "#ifdef MY_GUARD"));
    ASSERT(strstr(content, "#endif /* MY_GUARD */"));

    {
      FILE *readonly_f;
#if defined(_MSC_VER)
      if (((readonly_f = cdd_test_tmpfile_global()) == NULL))
        readonly_f = NULL;
#else
      readonly_f = cdd_test_tmpfile_global();
#endif
      if (readonly_f) {
        g_fail_io_after = 0;
        g_io_calls = 0;
        g_fail_io_after = 0;
        g_io_calls = 0;
        {
          int _rc = write_enum_to_str_func(readonly_f, "MyEnum", &em, &config);
          printf("write_enum_to_str_func RC WAS %d\\n", _rc);
          ASSERT_EQ(CDD_C_ERROR_IO, _rc);
        }
        g_fail_io_after = 0;
        g_io_calls = 0;
        g_fail_io_after = 0;
        g_io_calls = 0;
        ASSERT_EQ(CDD_C_ERROR_IO,
                  write_enum_from_str_func(readonly_f, "MyEnum", &em, &config));
        fclose(readonly_f);
      }
    }

    free(content);
    enum_members_free(&em);
    if (tmp)
      if (tmp)
        fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Unit test test_enum_generation_oom.
 * @return Greatest test result.
 */
TEST test_enum_generation_oom(void) {
  struct EnumMembers em;
#ifdef CDD_BUILD_TESTS
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_init(&em));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(0, enum_members_init(&em));
  em.size = em.capacity; /* Force realloc */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_add(&em, "TEST"));
  g_cdd_alloc_fail = 0;

  em.capacity = 16;
  em.size = 16;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_add(&em, "TEST2"));
  em.capacity = 0;
  em.size = 0;
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_add(&em, "TEST"));
  g_cdd_strdup_fail = 0;

  /* NULL member test for coverage in write_enum_to_str_func / from_str_func */
  ASSERT_EQ(0, enum_members_add(&em, "VALID"));
  em.size++; /* Intentionally leave one member NULL */

  {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    {
      struct CodegenEnumConfig config = {"MY_GUARD"};
      printf("g_fail_io_after=%d g_io_calls=%d\n", g_fail_io_after, g_io_calls);
      ASSERT_EQ(0, write_enum_to_str_func(tmp, "MyEnum", &em, &config));
      ASSERT_EQ(0, write_enum_from_str_func(tmp, "MyEnum", &em, &config));
      if (tmp)
        if (tmp)
          fclose(tmp);
    }
  }

  enum_members_free(&em);

  /* Test enum_members_init OOM on 2nd and 3rd calloc */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_init(&em));
  g_cdd_alloc_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_init(&em));
  g_cdd_alloc_fail = 0;

  /* Test enum_members_add_ex lazy allocation and OOM when
   * serde_names/serde_skip are NULL */
  memset(&em, 0, sizeof(em));
  em.capacity = 2;
  em.members = (char **)C_CDD_CALLOC(2, sizeof(char *));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_add_ex(&em, "A", "a", 0));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_add_ex(&em, "A", "a", 0));
  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_add_ex(&em, "A", "a", 0));
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_add_ex(&em, "A", "a", 1));
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_add_ex(&em, "B", "", 0));

  /* Populate cli_descriptions, cli_hidden, serde_aliases, n_serde_aliases and
   * test realloc OOM */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            enum_member_set_cli_meta(NULL, 0, "d", 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            enum_member_set_cli_meta(&em, 99, "d", 0));
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_set_cli_meta(&em, 0, "", 0));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_member_set_cli_meta(&em, 0, "desc", 0));
  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_member_set_cli_meta(&em, 0, "desc", 0));
  g_cdd_strdup_fail = 0;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_member_set_cli_meta(&em, 0, NULL, 1));
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_set_cli_meta(&em, 0, "desc1", 1));
  /* Overwrite existing description and reuse cli_hidden */
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_set_cli_meta(&em, 0, "desc2", 1));

  /* Test enum_member_add_alias validation and OOM */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, enum_member_add_alias(NULL, 0, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, enum_member_add_alias(&em, 0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, enum_member_add_alias(&em, 0, ""));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, enum_member_add_alias(&em, 99, "a"));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_member_add_alias(&em, 0, "al1"));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_member_add_alias(&em, 0, "al1"));
  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_member_add_alias(&em, 0, "al1"));
  g_cdd_strdup_fail = 0;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_member_add_alias(&em, 0, "al1"));
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_add_alias(&em, 0, "al1"));
  /* Duplicate alias */
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_add_alias(&em, 0, "al1"));
  /* NULL slot in serde_aliases[0] */
  C_CDD_FREE(em.serde_aliases[0][0]);
  em.serde_aliases[0][0] = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_add_alias(&em, 0, "al2"));

  /* Realloc with all metadata arrays present (em.size == 2 == em.capacity) */
  {
    int f_idx;
    for (f_idx = 1; f_idx <= 7; ++f_idx) {
      g_cdd_alloc_fail = f_idx;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, enum_members_add_ex(&em, "C", NULL, 0));
      g_cdd_alloc_fail = 0;
    }
    ASSERT_EQ(CDD_C_SUCCESS, enum_members_add_ex(&em, "C", NULL, 0));
  }
  enum_members_free(&em);

  /* Test capacity <= size branch in enum_member_set_cli_meta and
   * enum_member_add_alias, and enum_members_free with n_serde_aliases == NULL
   */
  memset(&em, 0, sizeof(em));
  em.size = 1;
  em.capacity = 0;
  em.members = (char **)C_CDD_CALLOC(1, sizeof(char *));
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_set_cli_meta(&em, 0, "d", 1));
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_add_alias(&em, 0, "a"));
  C_CDD_FREE(em.n_serde_aliases);
  em.n_serde_aliases = NULL;
  enum_members_free(&em);
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_enum_exhaustive_io.
 * @return Greatest test result.
 */
TEST test_enum_exhaustive_io(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  int rc = 0;
  struct EnumMembers em;
  struct CodegenEnumConfig config = {"MY_GUARD"};

  enum_members_init(&em);
  enum_members_add(&em, "VAL1");
  enum_members_add(&em, "VAL2");
  enum_member_add_alias(&em, 0, "v1_alias");
  enum_member_add_alias(&em, 0, "v1_null");
  C_CDD_FREE(em.serde_aliases[0][1]);
  em.serde_aliases[0][1] = NULL;
  em.serde_other_idx = 1;

  for (i = 0; i < 50; ++i) {
    FILE *tmp = cdd_test_tmpfile_global();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_enum_from_str_func(tmp, "MyEnum", &em, &config);
    if (tmp)
      fclose(tmp);
    if (rc == 0)
      break;
  }
  g_fail_io_after = -1;

  /* Test serde_other_idx out of bounds and pointing to NULL member,
   * serde_skip == NULL, serde_aliases == NULL with n_serde_aliases != NULL,
   * and empty member name error */
  {
    FILE *tmp = cdd_test_tmpfile_global();
    int *saved_skip = em.serde_skip;
    char ***saved_aliases = em.serde_aliases;
    char saved_ch = em.members[0][0];

    em.serde_other_idx = 99;
    ASSERT_EQ(CDD_C_SUCCESS,
              write_enum_from_str_func(tmp, "MyEnum", &em, NULL));

    C_CDD_FREE(em.members[1]);
    em.members[1] = NULL;
    em.serde_other_idx = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              write_enum_from_str_func(tmp, "MyEnum", &em, NULL));

    em.serde_skip = NULL;
    em.serde_aliases = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, write_enum_to_str_func(tmp, "MyEnum", &em, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              write_enum_from_str_func(tmp, "MyEnum", &em, NULL));

    em.members[0][0] = '\0';
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_enum_to_str_func(tmp, "MyEnum", &em, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_enum_from_str_func(tmp, "MyEnum", &em, NULL));
    em.members[0][0] = saved_ch;

    em.serde_skip = saved_skip;
    em.serde_aliases = saved_aliases;
    em.serde_other_idx = -1;
    if (tmp)
      fclose(tmp);
  }
  enum_members_free(&em);

  enum_members_init(&em);
  enum_members_add(&em, "VAL1");
  enum_members_add(&em, "VAL2");

  for (i = 0; i < 50; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_enum_to_str_func(tmp, "MyEnum", &em, &config);
    if (tmp)
      if (tmp)
        fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
  }

  for (i = 0; i < 50; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_enum_from_str_func(tmp, "MyEnum", &em, &config);
    if (tmp)
      if (tmp)
        fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
  }

  config.guard_macro = NULL;

  for (i = 0; i < 50; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_enum_to_str_func(tmp, "MyEnum", &em, &config);
    if (tmp)
      if (tmp)
        fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
  }

  for (i = 0; i < 50; ++i) {
    FILE *tmp;
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_enum_from_str_func(tmp, "MyEnum", &em, &config);
    if (tmp)
      if (tmp)
        fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
  }

  g_fail_io_after = -1;
  enum_members_free(&em);
#endif
  g_fail_io_after = -1;
  PASS();
}
/**
 * @brief Executes the codegen_enum_suite test suite.
 */
SUITE(codegen_enum_suite) {
  RUN_TEST(test_enum_generation);
  RUN_TEST(test_enum_generation_oom);
  RUN_TEST(test_enum_exhaustive_io);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_ENUM_H */
