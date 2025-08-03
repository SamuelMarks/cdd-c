#ifndef TEST_FS_H
#define TEST_FS_H

#include "cdd_test_helpers/cdd_helpers.h"
#include <errno.h>
#include <fs.h>
#include <greatest.h>

#ifndef _MSC_VER
#include <sys/stat.h>
#endif /* !_MSC_VER */

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

TEST test_fs_dirname_windows_specific(void) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  char path_drive_rel[] = "C:foo";
  char path_drive_abs[] = "C:\\foo";

  ASSERT_STR_EQ("C:", get_dirname(path_drive_rel));
  ASSERT_STR_EQ("C:\\", get_dirname(path_drive_abs));
#endif
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
    FAIL();
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
  const char *src = "cp_src.tmp";
  const char *dst = "cp_dst.tmp";
  int rc;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err_s = fopen_s(&fp, src, "w");
    if (err_s != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open header file %s\n", src);
      FAIL();
    }
  }
#else
  fp = fopen(src, "w");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open file: %s\n", src);
    FAIL();
  }
#endif
  fputs("hello", fp);
  fclose(fp);
  rc = cp(dst, src);
  ASSERT_EQ_FMT(0, rc, "%d");
  {
    int err;
    size_t size;
    char *content = c_read_file(dst, &err, &size, "r");
    ASSERT(content != NULL && strcmp(content, "hello") == 0);
    free(content);
  }

  /* Error: src does not exist */
  remove(src);
  ASSERT(cp(dst, src) != 0);

  /* Error: dst is a directory */
  remove(dst);
  makedir(dst);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err_s = fopen_s(&fp, src, "w");
    if (err_s != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open header file %s\n", src);
      FAIL();
    }
  }
#else
  fp = fopen(src, "w");
  if (fp == NULL) {
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

TEST test_fs_c_read_file_error(void) {
  int err;
  size_t sz;
  const char path[] = PATH_SEP "not" PATH_SEP "a" PATH_SEP "file";
  ASSERT_EQ(NULL, c_read_file(path, &err, &sz, "r"));
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

TEST test_makedirs_and_makedir_edge(void) {
  const char *const deep = "dir1" PATH_SEP "dir2" PATH_SEP "dir3";
  ASSERT_EQ(0, makedirs(deep));
  ASSERT_EQ(EXIT_FAILURE, makedir("dir1"));
  ASSERT_EQ(EXIT_FAILURE, makedir("dir1"));

  /* Null and empty paths should fail */
  ASSERT(makedir(NULL) != 0);
  ASSERT(makedir("") != 0);
  ASSERT(makedirs(NULL) != 0);

  rmdir(deep);
  rmdir("dir1" PATH_SEP "dir2");
  rmdir("dir1");
  PASS();
}

/* Try get_dirname with NULL and "" */
TEST test_get_dirname_edge_cases(void) {
  char empty1[2] = "";
  const char *res1 = (char *)get_dirname(empty1);
  ASSERT(res1 != NULL);

  {
    const char *res2 = (char *)get_dirname(NULL);
    ASSERT(res2 != NULL && (strcmp(res2, ".") == 0 || strcmp(res2, "") == 0));
  }
  PASS();
}

TEST test_write_to_file_fail(void) {
  const char *dir = "test_dir_for_write";
  ASSERT_EQ(0, makedir(dir));
  ASSERT_NEQ(0, write_to_file(dir, "some content"));
  rmdir(dir);
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
  ASSERT(makedirs("") != 0 || makedirs("") == EXIT_SUCCESS);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  ASSERT_EQ(0, makedirs("\\"));
#else
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

TEST test_get_dirname_long_filename_no_path(void) {
  char long_path[PATH_MAX + 20];
  char *res;

  memset(long_path, 'a', sizeof(long_path) - 1);
  long_path[sizeof(long_path) - 1] = '\0';

  res = (char *)get_dirname(long_path);
  ASSERT_STR_EQ(".", res);
  PASS();
}

TEST test_get_dirname_long_path(void) {
#if !defined(_MSC_VER)
  char long_path[PATH_MAX + 20];
  const char *res;

  memset(long_path, 'a', sizeof(long_path) - 1);
  long_path[sizeof(long_path) - 1] = '\0';

  /* Make it a path by adding a slash */
  long_path[PATH_MAX + 5] = '/';
  long_path[PATH_MAX + 6] = 'b';

  res = get_dirname(long_path);
  ASSERT_EQ(NULL, res);
  ASSERT_EQ(ENAMETOOLONG, errno);
#endif
  PASS();
}

TEST test_dirname_msvc_root(void) {
#if defined(_MSC_VER)
  char path_drive[] = "C:\\";
  char path_server_share[] = "\\\\server\\share";

  ASSERT_STR_EQ("C:\\", get_dirname(path_drive));
  ASSERT_STR_EQ("\\\\server\\share", get_dirname(path_server_share));
#endif
  PASS();
}

TEST test_get_basename_root_path(void) {
  ASSERT_STR_EQ(PATH_SEP, get_basename(PATH_SEP));
  ASSERT_STR_EQ(PATH_SEP, get_basename(PATH_SEP PATH_SEP PATH_SEP));
  PASS();
}

TEST test_cp_dest_exists(void) {
#if !defined(_MSC_VER)
  const char *src = "cp_src_exist.tmp";
  const char *dst = "cp_dst_exist.tmp";

  write_to_file(src, "src content");
  write_to_file(dst, "dst content");

  /* cp should fail if destination exists and O_EXCL is used */
  ASSERT_NEQ(0, cp(dst, src));

  remove(src);
  remove(dst);
#endif
  PASS();
}

TEST test_makedirs_stat_fail(void) {
#ifndef _MSC_VER
  const char *path = "perm_dir/sub";
  ASSERT_EQ(0, makedir("perm_dir"));

  /* Make perm_dir non-searchable */
  ASSERT_EQ(0, chmod("perm_dir", 0666)); /* no x bit */

  errno = 0;
  ASSERT_NEQ(0, makedirs(path));
  ASSERT(errno == EACCES || errno == ENOTDIR); /* EACCES is more likely */

  /* Cleanup */
  chmod("perm_dir", 0777);
  rmdir("perm_dir");
#endif
  PASS();
}

TEST test_write_to_file_null_args(void) {
  ASSERT_NEQ(0, write_to_file(NULL, "content"));
  ASSERT_NEQ(0, write_to_file("filename.txt", NULL));
  PASS();
}

TEST test_get_dirname_multiple_separators(void) {
  char path2[] = PATH_SEP PATH_SEP "foo" PATH_SEP;
  char path3[] = PATH_SEP PATH_SEP PATH_SEP;
  ASSERT_STR_EQ(PATH_SEP, get_dirname(path2));
  ASSERT_STR_EQ(PATH_SEP, get_dirname(path3));
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
  RUN_TEST(test_makedirs_and_makedir_edge);
  RUN_TEST(test_get_dirname_edge_cases);
  RUN_TEST(test_fs_makedir_null_and_empty);
  RUN_TEST(test_fs_makedirs_top_and_empty);
  RUN_TEST(test_fs_cp);
  RUN_TEST(test_get_basename_long);
  RUN_TEST(test_get_dirname_long_filename_no_path);
  RUN_TEST(test_get_dirname_long_path);
  RUN_TEST(test_dirname_msvc_root);
  RUN_TEST(test_write_to_file_fail);
  RUN_TEST(test_get_basename_root_path);
  RUN_TEST(test_cp_dest_exists);
  RUN_TEST(test_makedirs_stat_fail);
  RUN_TEST(test_get_dirname_multiple_separators);
  RUN_TEST(test_write_to_file_null_args);
  RUN_TEST(test_fs_dirname_windows_specific);
}

#endif /* !TEST_FS_H */
