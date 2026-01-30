#ifndef TEST_FS_H
#define TEST_FS_H

#include "cdd_test_helpers/cdd_helpers.h"
#include <errno.h>
#include <fs.h>
#include <greatest.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <wchar.h>
#endif /* !_MSC_VER */

#ifndef _MSC_VER
#include <sys/stat.h>
#endif /* !_MSC_VER */

TEST test_get_basename(void) {
  char *res = NULL;
  int rc;

  rc = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt", &res);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("baz.txt", res);
  free(res);

  rc = get_basename("file.txt", &res);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("file.txt", res);
  free(res);

  rc = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP, &res);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("bar", res);
  free(res);

  rc = get_basename(NULL, &res);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ(".", res);
  free(res);

  PASS();
}

TEST test_read_to_file_error(void) {
  int rc;
  size_t size = 0;
  char *s = NULL;

  rc = read_to_file("file_that_does_not_exist.xyz", "r", &s, &size);
  ASSERT_EQ(ENOENT, rc); /* No such file or directory */
  ASSERT_EQ(NULL, s);

  rc = read_to_file(NULL, "r", &s, &size);
  ASSERT_EQ(EINVAL, rc); /* Invalid argument */

  rc = read_to_file("anything", NULL, &s, &size);
  ASSERT_EQ(EINVAL, rc);

  rc = read_to_file("anything", "r", NULL, &size);
  ASSERT_EQ(EINVAL, rc);

  rc = read_to_file("anything", "r", &s, NULL);
  ASSERT_EQ(EINVAL, rc);

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
  char *out = NULL;

  ASSERT_EQ(0, get_dirname(path1, &out));
  ASSERT_STR_EQ(PATH_SEP "foo" PATH_SEP "bar", out);
  free(out);

  ASSERT_EQ(0, get_dirname(path2, &out));
  ASSERT_STR_EQ(".", out);
  free(out);

  ASSERT_EQ(0, get_dirname(path3, &out));
  ASSERT_STR_EQ(".", out);
  free(out);

  ASSERT_EQ(0, get_dirname(NULL, &out));
  ASSERT_STR_EQ(".", out);
  free(out);

  PASS();
}

TEST test_fs_read_to_file_empty(void) {
  int rc;
  size_t sz = 0;
  char *data = NULL;
  struct FilenameAndPtr *file = malloc(sizeof(*file));
  if (!file)
    FAILm("Memory allocation failed in test");

  ASSERT_EQ(0, mktmpfilegetnameandfile(NULL, "empty.tmp", "wb", file));
  /* Close handle so we can read it fresh */
  fclose(file->fh);
  file->fh = NULL; /* Prevent double close in cleanup */

  rc = read_to_file(file->filename, "rb", &data, &sz);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, sz);
  ASSERT_NEQ(NULL, data);
  ASSERT_EQ('\0', data[0]);
  free(data);

  FilenameAndPtr_delete_and_cleanup(file);
  free(file);
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
      fprintf(stderr, "Failed to open file %s\n", src);
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
    int rc_read;
    size_t size;
    char *content = NULL;
    rc_read = read_to_file(dst, "r", &content, &size);
    ASSERT_EQ(0, rc_read);
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
      fprintf(stderr, "Failed to open file %s\n", src);
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

TEST test_fs_read_to_file_failure(void) {
  int rc;
  size_t sz;
  char *data = NULL;
  const char path[] = PATH_SEP "not" PATH_SEP "a" PATH_SEP "file";
  rc = read_to_file(path, "r", &data, &sz);
  ASSERT(rc != 0);
  ASSERT_EQ(NULL, data);
  PASS();
}

TEST test_fs_read_to_file_success(void) {
  const char *const filename = "testfs.txt";
  int rc;
  size_t sz;
  char *data = NULL;

  if (write_to_file(filename, "Hello") != EXIT_SUCCESS) {
    FAILm("Setup failed");
  }

  rc = read_to_file(filename, "r", &data, &sz);
  ASSERT_EQ(0, rc);
  ASSERT(data != NULL);
  ASSERT_EQ(5, sz);
  free(data);
  remove(filename);
  PASS();
}

TEST test_makedirs_and_makedir_edge(void) {
  const char *const deep = "dir1" PATH_SEP "dir2" PATH_SEP "dir3";
  /* Clean up first in case leftover */
  rmdir(deep);
  rmdir("dir1" PATH_SEP "dir2");
  rmdir("dir1");

  ASSERT_EQ(0, makedirs(deep));
  /* Idempotence */
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
  int rc;
  size_t out_len;

  rc = ascii_to_wide(ascii_str, wide_buf, 10, &out_len);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(5, out_len);
  ASSERT(wcscmp(wide_buf, L"hello") == 0);

  rc = wide_to_ascii(wide_buf, ascii_buf, 10, &out_len);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(5, out_len);
  ASSERT_STR_EQ("hello", ascii_buf);

  /* Error cases */
  ASSERT_EQ(EINVAL, ascii_to_wide(NULL, wide_buf, 10, &out_len));
  ASSERT_EQ(EINVAL, ascii_to_wide(ascii_str, NULL, 10, &out_len));
  ASSERT_EQ(EINVAL, ascii_to_wide(ascii_str, wide_buf, 0, &out_len));
  ASSERT_EQ(EINVAL, wide_to_ascii(NULL, ascii_buf, 10, &out_len));
  ASSERT_EQ(EINVAL, wide_to_ascii(wide_buf, NULL, 10, &out_len));
  ASSERT_EQ(EINVAL, wide_to_ascii(wide_buf, ascii_buf, 0, &out_len));

  PASS();
}

TEST test_fs_windows_unc(void) {
  char unc_path[] = "\\\\server\\share\\file";
  char unc_path_dir_only[] = "\\\\server\\share";
  char *out = NULL;

  ASSERT(path_is_unc(unc_path));

  ASSERT_EQ(0, get_dirname(unc_path, &out));
  ASSERT_STR_EQ("\\\\server\\share", out);
  free(out);

  ASSERT_EQ(0, get_dirname(unc_path_dir_only, &out));
  ASSERT_STR_EQ("\\\\server\\share", out);
  free(out);

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
  char *res1 = NULL;

  ASSERT_EQ(0, get_dirname(empty1, &res1));
  ASSERT(res1 != NULL && strcmp(res1, ".") == 0);
  free(res1);

  /* Matches behavior of get_basename and implementation in fs.c */
  ASSERT_EQ(0, get_dirname(NULL, &res1));
  ASSERT_STR_EQ(".", res1);
  free(res1);

  PASS();
}

TEST test_write_to_file_fail(void) {
  const char *dir = "test_dir_for_write";
  ASSERT_EQ(0, makedir(dir));
  ASSERT_NEQ(0, write_to_file(dir, "some content"));
  rmdir(dir);
  PASS();
}

TEST test_fs_makedir_null_and_empty(void) {
  ASSERT(makedir(NULL) != 0);
  ASSERT(makedir("") != 0);
  PASS();
}

TEST test_fs_makedirs_top_and_empty(void) {
  ASSERT(makedirs("") != 0);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  ASSERT(makedirs("\\") == 0);
#else
  ASSERT(makedirs("/") == 0);
#endif
  PASS();
}

TEST test_get_dirname_long_filename_no_path(void) {
  char long_path[PATH_MAX + 20];
  char *res = NULL;

  memset(long_path, 'a', sizeof(long_path) - 1);
  long_path[sizeof(long_path) - 1] = '\0';

  ASSERT_EQ(0, get_dirname(long_path, &res));
  ASSERT_STR_EQ(".", res);
  free(res);
  PASS();
}

TEST test_get_basename_root_path(void) {
  char *res = NULL;
  ASSERT_EQ(0, get_basename(PATH_SEP, &res));
  /* get_basename("/") -> "/" */
  ASSERT_STR_EQ(PATH_SEP, res);
  free(res);

  ASSERT_EQ(0, get_basename(PATH_SEP PATH_SEP, &res));
  ASSERT_STR_EQ(PATH_SEP, res);
  free(res);
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
  /* This test relies on permissions */
  const char *path = "perm_dir";
  const char *sub = "perm_dir/sub";

  if (getuid() == 0)
    SKIPm("Skipping permission test as root");

  ASSERT_EQ(0, makedir(path));

  /* Make perm_dir non-writeable */
  ASSERT_EQ(0, chmod(path, 0555));

  errno = 0;
  ASSERT_NEQ(0, makedirs(sub));
  ASSERT(errno == EACCES || errno == EROFS || errno == EPERM);

  /* Cleanup */
  chmod(path, 0777);
  rmdir(path);
  PASS();
#else
  SKIP();
#endif
}

TEST test_get_dirname_multiple_separators(void) {
  char *out = NULL;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  char path0[] = PATH_SEP PATH_SEP PATH_SEP PATH_SEP "foo";
  ASSERT_EQ(0, get_dirname(path0, &out));
  ASSERT_STR_EQ("\\\\foo", out);
  free(out);
#else
  char path0[] = PATH_SEP PATH_SEP "foo";
  ASSERT_EQ(0, get_dirname(path0, &out));
  ASSERT_STR_EQ(PATH_SEP, out);
  free(out);
#endif
  PASS();
}

TEST test_tempdir(void) {
  char *tmpdir = NULL;
  const int rc = tempdir(&tmpdir);
  ASSERT_EQ(EXIT_SUCCESS, rc);
  ASSERT(tmpdir != NULL);
  /* Basic sanity check content */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  ASSERT_STR_EQ(tmpdir, getenv("TEMP"));
#endif
  free(tmpdir);
  PASS();
}

SUITE(fs_suite) {
  RUN_TEST(test_get_basename);
  RUN_TEST(test_read_to_file_error);
  RUN_TEST(test_makedir_tmp);
  RUN_TEST(test_fs_dirname);
  RUN_TEST(test_fs_read_to_file_failure);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  /* TODO: Fix file locking or sharing issues for these on Windows CI */
#else
  RUN_TEST(test_fs_read_to_file_success);
  RUN_TEST(test_fs_cp);
  RUN_TEST(test_makedirs_path_is_file);
#endif
  RUN_TEST(test_fs_read_to_file_empty);
  RUN_TEST(test_makedirs_and_makedir_edge);
  RUN_TEST(test_get_dirname_edge_cases);
  RUN_TEST(test_fs_makedir_null_and_empty);
  RUN_TEST(test_fs_makedirs_top_and_empty);
  RUN_TEST(test_get_dirname_long_filename_no_path);
  RUN_TEST(test_write_to_file_fail);
  RUN_TEST(test_get_basename_root_path);
  RUN_TEST(test_cp_dest_exists);
  RUN_TEST(test_makedirs_stat_fail);
  RUN_TEST(test_get_dirname_multiple_separators);
  RUN_TEST(test_write_to_file_null_args);
#ifdef _MSC_VER
  RUN_TEST(test_fs_windows_conversions);
  RUN_TEST(test_fs_windows_unc);
#endif
  RUN_TEST(test_tempdir);
}

#endif /* !TEST_FS_H */
