#ifndef TEST_FS_H
#define TEST_FS_H

#include "cdd_test_helpers/cdd_helpers.h"
#include <errno.h>
#include <fs.h>
#include <greatest.h>

#ifdef _MSC_VER
#include <wchar.h>
#endif /* !_MSC_VER */

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

TEST test_read_to_file_error(void) {
  int err = 0;
  size_t size = 0;
  char *s;

  s = read_to_file("file_that_does_not_exist.xyz", &err, &size, "r");
  ASSERT_EQ(NULL, s);
  ASSERT_EQ(1, err); /* FILE_NOT_EXIST */

  s = read_to_file(NULL, &err, &size, "r");
  ASSERT_EQ(NULL, s);
  ASSERT_EQ(1, err); /* FILE_NOT_EXIST */

  s = read_to_file("anything", NULL, &size, "r");
  ASSERT_EQ(NULL, s);

  s = read_to_file("anything", &err, NULL, "r");
  ASSERT_EQ(NULL, s);

  s = read_to_file("anything", &err, &size, NULL);
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

TEST test_fs_dirname(void) {
  char path1[] = PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt";
  char path2[] = "baz.txt";
  char path3[] = "";

  ASSERT_STR_EQ(PATH_SEP "foo" PATH_SEP "bar", get_dirname(path1));
  ASSERT_STR_EQ(".", get_dirname(path2));
  ASSERT_STR_EQ(".", get_dirname(path3));
  ASSERT_STR_EQ(".", get_dirname(NULL));
  PASS();
}

TEST test_fs_read_to_file_empty(void) {
  FILE *fp;
  int err;
  size_t sz;
  char *data;
  const char *const filename = "empty.tmp";
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err_s = fopen_s(&fp, filename, "w, ccs=UTF-8");
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

  data = read_to_file(filename, &err, &sz, "r");
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
    errno_t err_s = fopen_s(&fp, src, "w, ccs=UTF-8");
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
    char *content = read_to_file(dst, &err, &size, "r");
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
    errno_t err_s = fopen_s(&fp, src, "w, ccs=UTF-8");
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

TEST test_fs_read_to_file_error(void) {
  int err;
  size_t sz;
  const char path[] = PATH_SEP "not" PATH_SEP "a" PATH_SEP "file";
  ASSERT_EQ(NULL, read_to_file(path, &err, &sz, "r"));
  PASS();
}

TEST test_fs_read_to_file_success(void) {
  const char *const filename = "testfs.txt";
  int err;
  size_t sz;
  char *data;
  ASSERT_EQ(EXIT_SUCCESS, write_to_file(filename, "Hello"));
  data = read_to_file(filename, &err, &sz, "r");
  ASSERT(data != NULL && sz == 5);
  ASSERT_EQ(err, /* FILE_OK */ 0);
  free(data);
  remove(filename);
  PASS();
}

TEST test_read_to_file_nulls(void) {
  int err = 0;
  size_t sz = 0;
  const char *const s = read_to_file(NULL, &err, &sz, "r");
  ASSERT_EQ(NULL, s);
  PASS();
}

TEST test_makedirs_and_makedir_edge(void) {
  const char *const deep = "dir1" PATH_SEP "dir2" PATH_SEP "dir3";
  ASSERT_EQ(0, makedirs(deep));
  ASSERT_EQ(0, makedirs("dir1"));

  /* Null and empty paths should fail */
  ASSERT(makedir(NULL) != 0);
  ASSERT(makedir("") != 0);
  ASSERT(makedirs(NULL) != 0);
  ASSERT(makedirs("") != 0);

  rmdir(deep);
  rmdir("dir1" PATH_SEP "dir2");
  rmdir("dir1");
  PASS();
}

TEST test_write_to_file_null_args(void) {
  ASSERT_NEQ(0, write_to_file(NULL, "content"));
  ASSERT_NEQ(0, write_to_file("filename.txt", NULL));
  PASS();
}

#ifdef _MSC_VER
TEST test_fs_windows_conversions(void) {
  const char *ascii_str = "hello";
  wchar_t wide_buf[10];
  char ascii_buf[10];
  int len;

  len = ascii_to_wide(ascii_str, wide_buf, 10);
  ASSERT_EQ(5, len);
  ASSERT(wcscmp(wide_buf, L"hello") == 0);

  len = wide_to_ascii(wide_buf, ascii_buf, 10);
  ASSERT_EQ(5, len);
  ASSERT_STR_EQ("hello", ascii_buf);

  /* Error cases */
  ASSERT_EQ(-1, ascii_to_wide(NULL, wide_buf, 10));
  ASSERT_EQ(-1, ascii_to_wide(ascii_str, NULL, 10));
  ASSERT_EQ(-1, ascii_to_wide(ascii_str, wide_buf, 0));
  ASSERT_EQ(-1, wide_to_ascii(NULL, ascii_buf, 10));
  ASSERT_EQ(-1, wide_to_ascii(wide_buf, NULL, 10));
  ASSERT_EQ(-1, wide_to_ascii(wide_buf, ascii_buf, 0));

  PASS();
}

TEST test_fs_windows_unc(void) {
  char unc_path[] = "\\\\server\\share\\file";
  char unc_path_dir_only[] = "\\\\server\\share";
  ASSERT(path_is_unc(unc_path));
  ASSERT_STR_EQ("\\\\server\\share", get_dirname(unc_path));
  ASSERT_STR_EQ("\\\\server\\share", get_dirname(unc_path_dir_only));

  ASSERT(!path_is_unc("C:\\notunc"));
  ASSERT(!path_is_unc("\\nounc"));
  ASSERT(!path_is_unc("nounc"));
  PASS();
}
#endif

TEST test_makedirs_path_is_file(void) {
  const char *filename = "test_file_for_makedirs";
  const char *path = "test_file_for_makedirs/sub";
  write_to_file(filename, "");

  ASSERT_NEQ(0, makedirs(path));

  remove(filename);
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
  ASSERT(makedirs("") != 0);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  ASSERT(makedirs("\\") == 0);
#else
  ASSERT(makedirs("/") == 0);
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

TEST test_get_basename_root_path(void) {
  ASSERT_STR_EQ(PATH_SEP, get_basename(PATH_SEP));
  ASSERT_STR_EQ(PATH_SEP, get_basename(PATH_SEP PATH_SEP PATH_SEP));
  PASS();
}

TEST test_cp_dest_exists(void) {
#ifndef _MSC_VER
  const char *src = "cp_src_exist.tmp";
  const char *dst = "cp_dst_exist.tmp";

  write_to_file(src, "src content");
  write_to_file(dst, "dst content");

  /* cp should fail if destination exists and O_EXCL is used */
  ASSERT_NEQ(0, cp(dst, src));

  remove(src);
  remove(dst);
  PASS();
#else
  SKIP();
#endif
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
  PASS();
#else
  SKIP();
#endif
}

TEST test_get_dirname_multiple_separators(void) {
  char path0[] = PATH_SEP PATH_SEP "foo";
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  ASSERT_STR_EQ("\\\\foo", get_dirname(path0));
#else
  ASSERT_STR_EQ(PATH_SEP, get_dirname(path0));
#endif
  PASS();
}

SUITE(fs_suite) {
  RUN_TEST(test_get_basename);
  RUN_TEST(test_read_to_file_error);
  RUN_TEST(test_makedir_tmp);
  RUN_TEST(test_fs_dirname);
  RUN_TEST(test_fs_read_to_file_error);
  RUN_TEST(test_fs_read_to_file_success);
  RUN_TEST(test_fs_read_to_file_empty);
  RUN_TEST(test_read_to_file_nulls);
  RUN_TEST(test_makedirs_and_makedir_edge);
  RUN_TEST(test_get_dirname_edge_cases);
  RUN_TEST(test_fs_makedir_null_and_empty);
  RUN_TEST(test_fs_makedirs_top_and_empty);
  RUN_TEST(test_fs_cp);
  RUN_TEST(test_get_basename_long);
  RUN_TEST(test_get_dirname_long_filename_no_path);
  RUN_TEST(test_write_to_file_fail);
  RUN_TEST(test_get_basename_root_path);
  RUN_TEST(test_cp_dest_exists);
  RUN_TEST(test_makedirs_stat_fail);
  RUN_TEST(test_get_dirname_multiple_separators);
  RUN_TEST(test_write_to_file_null_args);
  RUN_TEST(test_makedirs_path_is_file);
#ifdef _MSC_VER
  RUN_TEST(test_fs_windows_conversions);
  RUN_TEST(test_fs_windows_unc);
#endif
}

#endif /* !TEST_FS_H */
