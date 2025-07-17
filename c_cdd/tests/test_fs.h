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
  ASSERT_STR_EQ("", res);

  res = get_basename(NULL);
  ASSERT_EQ(NULL, res);

  PASS();
}

TEST test_c_read_file_error(void) {
  int err = 0;
  size_t size;
  const char *s = c_read_file("file_that_does_not_exist.xyz", &err, &size, "r");
  ASSERT_EQ(NULL, s);
  ASSERT_EQ(err, 1); // FILE_NOT_EXIST
  PASS();
}

TEST test_makedir_tmp(void) {
  const char tmp[] = "test_tmpdir42";
  int rc;
  rc = makedir(tmp);
  ASSERT(rc == 0 || rc == EXIT_FAILURE);
  rc = makedir(tmp);
  ASSERT(rc == 0 || rc == EXIT_FAILURE);
  PASS();
}

TEST test_fs_basename(void) {
  const char path1[] = PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt";
  const char path2[] = "baz.txt";
  ASSERT_STR_EQ("baz.txt", get_basename(path1));
  ASSERT_STR_EQ("baz.txt", get_basename(path2));
  PASS();
}

TEST test_fs_dirname(void) {
  char path3[] = PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt";
  ASSERT_STR_EQ(PATH_SEP "foo" PATH_SEP "bar", get_dirname(path3));
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
  FILE *fh;
  const char *const filename = "testfs.txt";
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fh, filename, "w");
    if (err != 0 || fh == NULL) {
      fprintf(stderr, "Failed to open %s for writing", filename);
      free(fh);
      return EXIT_FAILURE;
    }
  }
#else
  fh = fopen(filename, "w");
#endif
  fputs("Hello", fh);
  fclose(fh);
  {
    int err;
    size_t sz;
    char *data = c_read_file(filename, &err, &sz, "r");
    ASSERT(data != NULL && sz == 5);
    free(data);
  }
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
  ASSERT(makedirs("dir1" PATH_SEP "dir2" PATH_SEP "dir3") == 0 ||
         makedirs("dir1" PATH_SEP "dir2" PATH_SEP "dir3") == -1);
  ASSERT(makedir("dir1") == 0 || makedir("dir1") == EXIT_FAILURE);
  /* system("rm -rf dir1"); */
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

SUITE(fs_suite) {
  RUN_TEST(test_get_basename);
  RUN_TEST(test_c_read_file_error);
  RUN_TEST(test_makedir_tmp);
  RUN_TEST(test_fs_basename);
  RUN_TEST(test_fs_dirname);
  RUN_TEST(test_fs_c_read_file_error);
  RUN_TEST(test_fs_c_read_file_success);
  RUN_TEST(test_c_read_file_nulls);
  /* RUN_TEST(test_cp_errors); */
  RUN_TEST(test_makedirs_and_makedir_edge);
  RUN_TEST(test_get_dirname_edge_cases);
}

#endif /* !TEST_FS_H */
