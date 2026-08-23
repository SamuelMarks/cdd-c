#ifndef TEST_FS_COVERAGE_H
#define TEST_FS_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "functions/parse/fs.h"
#include <greatest.h>
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern int g_cdd_strdup_fail;
extern int g_cdd_alloc_fail;

TEST test_fs_coverage_is_directory(void) {
  int is_dir;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_is_directory(".", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, fs_is_directory(".", &is_dir));
  ASSERT(is_dir);
  PASS();
}

TEST test_fs_coverage_write_errors(void) {
  /* Write to a nonexistent directory should fail with ENOENT */
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            fs_write_to_file("/does/not/exist/ever.txt", "abc"));
  PASS();
}

TEST test_fs_coverage_dirname(void) {
  char *out = NULL;
  /* Test dirname on simple files with no slash */
  ASSERT_EQ(CDD_C_SUCCESS, get_dirname("file.txt", &out));
  ASSERT_STR_EQ(".", out);
  C_CDD_FREE(out);
  PASS();
}

TEST test_fs_coverage_basename(void) {
  char *out = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, get_basename("file", &out));
  ASSERT_STR_EQ("file", out);
  C_CDD_FREE(out);
  PASS();
}

TEST test_fs_coverage_edge_cases(void) {
  char *out = NULL;
  ASSERT_EQ(0, get_basename("", &out));
  ASSERT_STR_EQ(".", out);
  C_CDD_FREE(out);
  out = NULL;

  ASSERT_EQ(0, get_dirname("", &out));
  ASSERT_STR_EQ(".", out);
  C_CDD_FREE(out);
  out = NULL;
  PASS();
}

/* extern int g_cdd_alloc_fail; (moved to global) */
/* extern int g_cdd_strdup_fail; (moved to global) */

TEST test_fs_coverage_oom(void) {
  int i;
  char *out = NULL;

  for (i = 1; i < 5; ++i) {
    g_cdd_alloc_fail = i;
    get_basename("path/to/file", &out);
    C_CDD_FREE(out);
    out = NULL;
    g_cdd_alloc_fail = 0;

    g_cdd_strdup_fail = i;
    get_basename("path/to/file", &out);
    C_CDD_FREE(out);
    out = NULL;
    g_cdd_strdup_fail = 0;

    g_cdd_alloc_fail = i;
    get_dirname("path/to/file", &out);
    C_CDD_FREE(out);
    out = NULL;
    g_cdd_alloc_fail = 0;

    g_cdd_strdup_fail = i;
    get_dirname("path/to/file", &out);
    C_CDD_FREE(out);
    out = NULL;
    g_cdd_strdup_fail = 0;
  }
  PASS();
}

SUITE(fs_coverage_suite) {
  RUN_TEST(test_fs_coverage_is_directory);
  RUN_TEST(test_fs_coverage_write_errors);
  RUN_TEST(test_fs_coverage_dirname);
  RUN_TEST(test_fs_coverage_basename);
  RUN_TEST(test_fs_coverage_edge_cases);
  RUN_TEST(test_fs_coverage_oom);
}

#ifdef __cplusplus
}
#endif
#endif
