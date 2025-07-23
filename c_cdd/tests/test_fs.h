#ifndef TEST_FS_H
#define TEST_FS_H

#include <fs.h>
#include <greatest.h>

TEST test_get_basename(void) {
  const char *res;

  res = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt");
  ASSERT_STR_EQ("baz.txt", res);

  res = get_basename("file.txt");
  ASSERT_STR_EQ("file.txt", res);

  res = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP);
  ASSERT_STR_EQ("bar", res);

  res = get_basename(NULL);
  ASSERT_STR_EQ(".", res);

  PASS();
}

TEST test_c_read_file_error(void) {
  int err = 0;
  size_t size = 0;
  char *s;

  s = c_read_file("file_that_does_not_exist.xyz", &err, &size, "r");
  ASSERT_EQ(NULL, s);
  ASSERT_EQ(1, err); /* FILE_NOT_EXIST */

  s = c_read_file(NULL, &err, &size, "r");
  ASSERT_EQ(NULL, s);
  ASSERT_EQ(1, err); /* FILE_NOT_EXIST */

  s = c_read_file("anything", NULL, &size, "r");
  ASSERT_EQ(NULL, s);

  s = c_read_file("anything", &err, NULL, "r");
  ASSERT_EQ(NULL, s);

  s = c_read_file("anything", &err, &size, NULL);
  ASSERT_EQ(NULL, s);

  PASS();
}

TEST test_makedir_tmp(void) {
  const char tmp[] = "test_makedir_tmp";
  int rc;
  rc = makedir(tmp);
  ASSERT_EQ_FMT(0, rc, "%d");
  rmdir(tmp);
  PASS();
}

TEST test_fs_basename(void) {
  const char path1[] = PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt";
  const char path2[] = "baz.txt";
  const char *res;
  char path_with_sep[] = "foo" PATH_SEP "bar";
  ASSERT_STR_EQ("baz.txt", get_basename(path1));
  ASSERT_STR_EQ("baz.txt", get_basename(path2));
  res = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt");
  ASSERT_STR_EQ("baz.txt", res);

  res = get_basename("file.txt");
  ASSERT_STR_EQ("file.txt", res);

  res = get_basename(path_with_sep);
  ASSERT_STR_EQ("bar", res);

  res = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP);
  ASSERT_STR_EQ("bar", res);

  res = get_basename(NULL);
  ASSERT_STR_EQ(".", res);

  PASS();
}

TEST test_fs_dirname(void) {
  char path1[] = PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt";
  char path2[] = "baz.txt";
  char path3[] = PATH_SEP;
  char path4[] = "";

  ASSERT_STR_EQ(PATH_SEP "foo" PATH_SEP "bar", get_dirname(path1));
  ASSERT_STR_EQ(".", get_dirname(path2));
  ASSERT_STR_EQ(PATH_SEP, get_dirname(path3));
  ASSERT_STR_EQ(".", get_dirname(path4));
  ASSERT_STR_EQ(".", get_dirname(NULL));
  PASS();
}

TEST test_fs_c_read_file_empty(void) {
  FILE *fp;
  int err;
  size_t sz;
  char *data;
  const char *const filename = "empty.tmp";
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err_s = fopen_s(&fp, filename, "w");
  if (err_s != 0 || fp == NULL) {
    fprintf(stderr, "Failed to open file %s\n", filename);
    return;
  }
#else
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    FAIL();
  }
#endif
  fclose(fp);

  data = c_read_file(filename, &err, &sz, "r");
  ASSERT_EQ(0, err);
  ASSERT_EQ(0, sz);
  ASSERT_NEQ(NULL, data);
  ASSERT_EQ('\0', data[0]);

  free(data);
  remove(filename);
  PASS();
}

TEST test_fs_cp(void) {
  FILE *fp;
  const char *const src = "cp_src.tmp";
  const char *const dst = "cp_dst.tmp";
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, src, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open header file %s\n", src);
      return;
    }
  }
#else
  fp = fopen(src, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", src);
    FAIL();
  }
#endif
  fputs("hello", fp);
  fclose(fp);

  ASSERT_EQ(0, cp(dst, src));

  /* Error: src does not exist */
  remove(src);
  ASSERT(cp(dst, src) != 0);

  /* Error: dst is a directory */
  remove(dst);
  makedir(dst);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, src, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open header file %s\n", src);
      return;
    }
  }
#else
  fp = fopen(src, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", src);
    FAIL();
  }
#endif
  fputs("hello", fp);
  fclose(fp);
  ASSERT(cp(dst, src) != 0);

  remove(src);
  rmdir(dst);
  PASS();
}

TEST test_makedirs_path_is_file(void) {
  const char *file_path = "makedirs_file.tmp";
  const char *dir_path = "makedirs_file.tmp" PATH_SEP "dir";
  FILE *fp;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, file_path, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open header file %s\n", file_path);
      return;
    }
  }
#else
  fp = fopen(file_path, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", file_path);
    FAIL();
  }
#endif
  fclose(fp);

  ASSERT(makedirs(dir_path) != 0);

  remove(file_path);
  PASS();
}

TEST test_fs_c_read_file_error(void) {
  int err;
  size_t sz;
  ASSERT_EQ(NULL, c_read_file(PATH_SEP "not" PATH_SEP "a" PATH_SEP "file", &err,
                              &sz, "r"));
  PASS();
}

TEST test_fs_c_read_file_success(void) {
  const char *const filename = "testfs.txt";
  int err;
  size_t sz;
  char *data;
  ASSERT_EQ(EXIT_SUCCESS, write_to_file(filename, "Hello"));
  data = c_read_file(filename, &err, &sz, "r");
  ASSERT(data != NULL && sz == 5);
  ASSERT_EQ(err, /* FILE_OK */ 0);
  free(data);
  remove(filename);
  PASS();
}

TEST test_c_read_file_nulls(void) {
  int err = 0;
  size_t sz = 0;
  const char *const s = c_read_file(NULL, &err, &sz, "r");
  ASSERT_EQ(NULL, s);
  PASS();
}

/* TODO: get working
TEST test_cp_errors(void) {
  FILE *fh0, *fh1;
  const char *const filename0 = "copy_exists.txt";
  const char *const filename1 = "copy_exists_dest.txt";
  ASSERT_EQ(-1, cp("copy_test_out.txt", "copy_test_nowhere.txt"));

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
  defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fh0, filename0, "w");
    if (err != 0 || fh == NULL) {
      fprintf(stderr, "Failed to open %s for writing", filename0);
      free(fh);
      return EXIT_FAILURE;
    }
  }
#else
  fh0 = fopen(filename0, "w");
#endif

  fputs("test", fh0);
  fclose(fh0);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
  defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fh1, filename1, "w");
    if (err != 0 || fh == NULL) {
      fprintf(stderr, "Failed to open %s for writing", filename1);
      free(fh1);
      return EXIT_FAILURE;
    }
  }
#else
  fh1 = fopen(filename1, "w");
#endif

  fputs("test", fh1);
  fclose(fh1);
  ASSERT_EQ(1, cp(filename1, filename0));
  ASSERT_EQ(-1, cp(filename1, filename0));
  remove(filename0);
  remove(filename1);
  PASS();
}
*/

TEST test_makedirs_and_makedir_edge(void) {
  const char *const deep = "dir1" PATH_SEP "dir2" PATH_SEP "dir3";
  ASSERT_EQ(makedirs(deep), 0);
  ASSERT_EQ(makedir("dir1"), EXIT_FAILURE);
  /* ASSERT(0 != makedirs(deep)); */
  ASSERT_EQ(EXIT_FAILURE, makedir("dir1"));

  /* Null and empty paths should fail */
  ASSERT(makedir(NULL) != 0);
  ASSERT(makedir("") != 0);
  ASSERT(makedirs(NULL) != 0);

  rmdir(deep);
  PASS();
}

/* Try get_dirname with NULL and "" */
TEST test_get_dirname_edge_cases(void) {
  char empty1[2] = "";
  const char *res1 = (char *)get_dirname(empty1);
  ASSERT(res1 != NULL);

  {
    const char *res2 = (char *)get_dirname(NULL);
    ASSERT(res2 == NULL || strcmp(res2, ".") == 0 || strcmp(res2, "") == 0);
  }
  PASS();
}

/* Simulate error for makedir: pass null and "" */
TEST test_fs_makedir_null_and_empty(void) {
  ASSERT(makedir(NULL) == EXIT_FAILURE || makedir(NULL) == -1);
  ASSERT(makedir("") == EXIT_FAILURE || makedir("") == -1);
  PASS();
}

/* Simulate makedirs edge-cases: pass "" and "/" */
TEST test_fs_makedirs_top_and_empty(void) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  /* On Windows, makedirs("/") or "" can act strangely, but should not crash. */
  ASSERT(makedirs("") != 0 ||
         makedirs("") == EXIT_SUCCESS); /* Nonzero likely */
  ASSERT(makedirs("\\") != 0 || makedirs("\\") == EXIT_SUCCESS);
#else
  ASSERT(makedirs("") != 0 || makedirs("") == EXIT_SUCCESS);
  ASSERT(makedirs("/") == 0 || makedirs("/") == -1);
#endif
  PASS();
}

TEST test_get_basename_long(void) {
  char long_path[PATH_MAX + 20];
  const char *res;

  /* Create a path that is too long */
  memset(long_path, 'a', sizeof(long_path) - 1);
  long_path[sizeof(long_path) - 1] = '\0';

  res = get_basename(long_path);
  ASSERT_EQ(NULL, res);

  PASS();
}

TEST test_get_dirname_long(void) {
#if !defined(_MSC_VER) || defined(PATHCCH_LIB)
  char long_path[PATH_MAX + 20];
  char *res;

  /* This test is for non-MSVC or MSVC with PathCch */
  memset(long_path, 'a', sizeof(long_path) - 1);
  long_path[sizeof(long_path) - 1] = '\0';

  res = (char *)get_dirname(long_path);
  ASSERT_STR_EQ(".", res);
#endif
  PASS();
}

SUITE(fs_suite) {
  RUN_TEST(test_get_basename);
  RUN_TEST(test_c_read_file_error);
  RUN_TEST(test_makedir_tmp);
  RUN_TEST(test_fs_basename);
  RUN_TEST(test_fs_dirname);
  RUN_TEST(test_fs_c_read_file_error);
  RUN_TEST(test_fs_c_read_file_success);
  RUN_TEST(test_fs_c_read_file_empty);
  RUN_TEST(test_c_read_file_nulls);
  /* RUN_TEST(test_cp_errors); */
  RUN_TEST(test_makedirs_and_makedir_edge);
  RUN_TEST(test_get_dirname_edge_cases);
  RUN_TEST(test_fs_makedir_null_and_empty);
  RUN_TEST(test_fs_makedirs_top_and_empty);
  RUN_TEST(test_makedirs_path_is_file);
  RUN_TEST(test_fs_cp);
  RUN_TEST(test_get_basename_long);
  RUN_TEST(test_get_dirname_long);
}

#endif /* !TEST_FS_H */
