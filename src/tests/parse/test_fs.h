extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_fs.h
 * @brief Unit tests for filesystem utilities.
 */

#ifndef TEST_FS_H
#define TEST_FS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include <errno.h>
#include <greatest.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <wchar.h>
#endif /* !_MSC_VER */

#ifndef _MSC_VER
#include <sys/stat.h>
#endif /* !_MSC_VER */
/* clang-format on */
/* LCOV_EXCL_START */

TEST test_get_basename(void) {
  char *res = NULL;
  int is_dir = 0;
  int rc;

  rc = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt", &res);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("baz.txt", res);
  free(res);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_is_directory(NULL, &is_dir));
  ASSERT_EQ(
      0, fs_is_directory("/path/that/does/not/exist/ever/ever/ever", &is_dir));
  ASSERT_EQ(0, is_dir);

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
  g_fail_io_after = -1;

  PASS();
}

TEST test_read_to_file_error(void) {
  int rc;
  size_t size = 0;
  char *s = NULL;

  rc = read_to_file("file_that_does_not_exist.xyz", "r", &s, &size);
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);
  ASSERT_EQ(NULL, s);

  rc = read_to_file(NULL, "r", &s, &size);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;

  PASS();
}

static enum cdd_c_error mock_walk_cb(const char *path, void *user_data) {
  int *count = (int *)user_data;
  (*count)++;
  (void)path;
  return CDD_C_SUCCESS;
}

TEST test_walk_directory(void) {
  /* Setup structure:
     walk_test_root/
       a.txt
       sub/
         b.txt
  */
  char *sys_tmp = NULL;
  char *root = NULL;
  int count = 0;
  int rc;

  /* tempdir returns system temp path usually */
  rc = tempdir(&sys_tmp);
  ASSERT_EQ(0, rc);

  /* Create a unique subdirectory to avoid counting other temp files */
  if (asprintf(&root, "%s%swalk_test_%d", sys_tmp, PATH_SEP, rand()) == -1) {
    free(sys_tmp);
    FAILm("asprintf failed");
  }
  free(sys_tmp);

  rc = makedir(root);
  ASSERT_EQ(0, rc);

  {
    char *sub = NULL;
    char *f1 = NULL;
    char *f2 = NULL;

    asprintf(&sub, "%s%ssub", root, PATH_SEP);
    makedir(sub);

    asprintf(&f1, "%s%sa.txt", root, PATH_SEP);
    write_to_file(f1, "a");

    asprintf(&f2, "%s%sb.txt", sub, PATH_SEP);
    write_to_file(f2, "b");

    rc = walk_directory(root, mock_walk_cb, &count);
    ASSERT_EQ(0, rc);
    /* Expect 2 files */
    ASSERT_EQ(2, count);

    remove(f2);
    remove(f1);
    rmdir(sub);
    rmdir(root);

    free(sub);
    free(f1);
    free(f2);
  }
  free(root);
  g_fail_io_after = -1;
  PASS();
}

TEST test_makedir_check(void) {
  char *t = NULL;
  char *p = NULL;
  tempdir(&t);
  asprintf(&p, "%s%snewdir", t, PATH_SEP);
  ASSERT_EQ(0, makedir(p));
  rmdir(p);
  /* Don't try to remove system temp dir (t) */
  /* rmdir(t); */
  free(p);
  free(t);
  g_fail_io_after = -1;
  PASS();
}

TEST test_fs_fopen_error_from(void) {
  enum FopenError err;

  fopen_error_from(0, &err);
  ASSERT_EQ(FOPEN_OK, err);
  fopen_error_from(EINVAL, &err);
  ASSERT_EQ(FOPEN_INVALID_PARAMETER, err);
  fopen_error_from(EMFILE, &err);
  ASSERT_EQ(FOPEN_TOO_MANY_OPEN_FILES, err);
  fopen_error_from(ENOMEM, &err);
  ASSERT_EQ(FOPEN_OUT_OF_MEMORY, err);
  fopen_error_from(ENOENT, &err);
  ASSERT_EQ(FOPEN_FILE_NOT_FOUND, err);
  fopen_error_from(EACCES, &err);
  ASSERT_EQ(FOPEN_PERMISSION_DENIED, err);
  fopen_error_from(EIO, &err);
  ASSERT_EQ(FOPEN_UNKNOWN_ERROR, err);
  g_fail_io_after = -1;

  PASS();
}

TEST test_fs_cp(void) {
  FILE *f;
  int rc;

  f = fopen("test_cp_src.txt", "w");
  fprintf(f, "test");
  fclose(f);

  rc = cp("test_cp_dst.txt", "test_cp_src.txt");
  ASSERT_EQ(0, rc);

  remove("test_cp_src.txt");
  remove("test_cp_dst.txt");

  /* Test error */
  rc = cp("invalid/dst/path", "invalid_src.txt");

  ASSERT(rc != 0);
  g_fail_io_after = -1;

  PASS();
}

TEST test_fs_basename_dirname_edge_cases(void) {
  char *out = NULL;

  /* get_basename edges */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_basename("foo", NULL));

  ASSERT_EQ(0, get_basename("///", &out));
  ASSERT_STR_EQ("/", out);
  free(out);
  out = NULL;

  /* get_dirname edges */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_dirname("foo", NULL));

  ASSERT_EQ(0, get_dirname("foo///", &out));
  ASSERT_STR_EQ(".", out);
  free(out);
  out = NULL;

  ASSERT_EQ(0, get_dirname("///", &out));
  ASSERT_STR_EQ("/", out);
  free(out);
  out = NULL;
  g_fail_io_after = -1;

  PASS();
}

TEST test_fs_dirname_more_edge_cases(void) {
  char *out = NULL;

  ASSERT_EQ(0, get_dirname("", &out));
  ASSERT_STR_EQ(".", out);
  free(out);
  out = NULL;

  ASSERT_EQ(0, get_dirname("foo//bar", &out));
  ASSERT_STR_EQ("foo", out);
  free(out);
  out = NULL;

  ASSERT_EQ(0, get_dirname("foo/bar///", &out));
  ASSERT_STR_EQ("foo", out);
  free(out);
  out = NULL;
  g_fail_io_after = -1;

  PASS();
}

TEST test_fs_write_to_file(void) {
  char *tmp_dir = NULL;
  char *file_path = NULL;
  char *out_data = NULL;
  size_t sz = 0;

  tempdir(&tmp_dir);
  asprintf(&file_path, "%s%ctest_write.txt", tmp_dir, PATH_SEP_C);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_write_to_file(NULL, "data"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_write_to_file(file_path, NULL));

  ASSERT_EQ(0, fs_write_to_file(file_path, "hello world"));

  read_to_file(file_path, "r", &out_data, &sz);
  ASSERT_STR_EQ("hello world", out_data);

  free(out_data);
  remove(file_path);
  rmdir(tmp_dir);
  free(file_path);
  free(tmp_dir);
  g_fail_io_after = -1;

  PASS();
}

TEST test_fs_dirname_foo(void) {
  char *out = NULL;

  ASSERT_EQ(0, get_dirname("foo", &out));
  ASSERT_STR_EQ(".", out);
  free(out);
  out = NULL;
  g_fail_io_after = -1;

  PASS();
}

TEST test_fs_cdd_fopen_too_long(void) {
  enum FopenError err;

  ASSERT_EQ(0, fopen_error_from(ERANGE, &err));
  ASSERT_EQ(FOPEN_FILENAME_TOO_LONG, err);
  g_fail_io_after = -1;

  PASS();
}

TEST test_fs_write_to_file_errors(void) {
  ASSERT_NEQ(
      0, fs_write_to_file("/invalid/path/that/cannot/exist/ever.txt", "hello"));
  g_fail_io_after = -1;
  PASS();
}

TEST test_read_from_fh_errors(void) {
  FILE *f = tmpfile();
  char *data = NULL;
  size_t sz = 0;

  /* Test invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, read_from_fh(NULL, &data, &sz));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, read_from_fh(f, NULL, &sz));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, read_from_fh(f, &data, NULL));

  fclose(f);
  g_fail_io_after = -1;
  PASS();
}

#ifdef _MSC_VER
TEST test_ascii_wide_conversion(void) {
  wchar_t wbuf[32];
  char abuf[32];
  size_t wlen = 0;
  size_t alen = 0;
  int rc;

  /* ascii_to_wide */
  rc = ascii_to_wide("hello", wbuf, 32, &wlen);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(5, wlen);

  /* wide_to_ascii */
  rc = wide_to_ascii(wbuf, abuf, 32, &alen);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(5, alen);
  ASSERT_STR_EQ("hello", abuf);

  /* errors */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, ascii_to_wide(NULL, wbuf, 32, &wlen));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            ascii_to_wide("test", NULL, 32, &wlen));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            ascii_to_wide("test", wbuf, 0, &wlen));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            ascii_to_wide("test", wbuf, 32, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, wide_to_ascii(NULL, abuf, 32, &alen));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, wide_to_ascii(wbuf, NULL, 32, &alen));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, wide_to_ascii(wbuf, abuf, 0, &alen));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, wide_to_ascii(wbuf, abuf, 32, NULL));
  g_fail_io_after = -1;

  PASS();
}
#endif /* _MSC_VER */

SUITE(fs_suite) {
  RUN_TEST(test_fs_fopen_error_from);
  RUN_TEST(test_fs_cp);
  RUN_TEST(test_get_basename);
  RUN_TEST(test_read_to_file_error);
  RUN_TEST(test_read_from_fh_errors);
  RUN_TEST(test_walk_directory);
  RUN_TEST(test_makedir_check);
  RUN_TEST(test_fs_basename_dirname_edge_cases);
  RUN_TEST(test_fs_dirname_more_edge_cases);
  RUN_TEST(test_fs_write_to_file);
  RUN_TEST(test_fs_write_to_file_errors);
  RUN_TEST(test_fs_dirname_foo);
  RUN_TEST(test_fs_cdd_fopen_too_long);
#ifdef _MSC_VER
  RUN_TEST(test_ascii_wide_conversion);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_FS_H */

/* LCOV_EXCL_STOP */
