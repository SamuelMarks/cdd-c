/**
 * @file test_fs_coverage.h
 * @brief Comprehensive coverage tests for filesystem functions.
 */

#ifndef TEST_FS_COVERAGE_H
#define TEST_FS_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "c_cdd_export.h"
#include "functions/parse/fs.h"
#include <errno.h>
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <sys/stat.h>
#include <unistd.h>
#endif
/* clang-format on */

extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_strdup_fail;

#if defined(_WIN32)
#define test_unsetenv(var) _putenv(var "=")
static int test_setenv(const char *var, const char *val) {
  static char _buf[512];
#if defined(_MSC_VER)
  sprintf_s(_buf, sizeof(_buf), "%s=%s", var, val);
#else
  sprintf(_buf, "%s=%s", var, val);
#endif
  return _putenv(_buf);
}
#else
#define test_unsetenv(var) unsetenv(var)
#define test_setenv(var, val) setenv((var), (val), 1)
#endif

static cdd_c_error_t mock_walk_err_cb(const char *path, void *user_data) {
  (void)path;
  (void)user_data;
  return CDD_C_ERROR_IO;
}

static cdd_c_error_t mock_walk_count_cb(const char *path, void *user_data) {
  int *cnt = (int *)user_data;
  (void)path;
  (*cnt)++;
  return CDD_C_SUCCESS;
}

TEST test_fs_coverage_path_helpers(void) {
  char *path = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_path_join(NULL, "a", &path));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_path_join("a", NULL, &path));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_path_join("a", "b", NULL));

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, fs_path_join("a", "b", &path));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, fs_path_join("dir", "file", &path));
  ASSERT(path != NULL);
  C_CDD_FREE(path);
  path = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            format_tmp_filename(NULL, "pfx", 1, "sfx", &path));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            format_tmp_filename("dir", "pfx", 1, "sfx", NULL));

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            format_tmp_filename("dir", "pfx", 1, "sfx", &path));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, format_tmp_filename("dir", "pfx", 1, "sfx", &path));
  ASSERT(path != NULL);
  C_CDD_FREE(path);
  path = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, format_tmp_filename("dir", NULL, 2, NULL, &path));
  ASSERT(path != NULL);
  C_CDD_FREE(path);
  path = NULL;

  PASS();
}

TEST test_fs_coverage_errno_to_cdd_error(void) {
  ASSERT_EQ(CDD_C_SUCCESS, errno_to_cdd_error(0));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, errno_to_cdd_error(ENOENT));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, errno_to_cdd_error(ENOMEM));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, errno_to_cdd_error(EINVAL));
  ASSERT_EQ(CDD_C_ERROR_IO, errno_to_cdd_error(EIO));
  ASSERT_EQ(CDD_C_ERROR_IO, errno_to_cdd_error(EACCES));
  PASS();
}

TEST test_fs_coverage_is_directory(void) {
  int is_dir;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_is_directory(".", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, fs_is_directory(".", &is_dir));
  ASSERT(is_dir);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_is_directory(NULL, &is_dir));
  PASS();
}

TEST test_fs_coverage_fopen_error_from(void) {
  FopenError_t err;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fopen_error_from(0, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, fopen_error_from(0, &err));
  ASSERT_EQ(FOPEN_OK, err);
  ASSERT_EQ(CDD_C_SUCCESS, fopen_error_from(EINVAL, &err));
  ASSERT_EQ(FOPEN_INVALID_PARAMETER, err);
  ASSERT_EQ(CDD_C_SUCCESS, fopen_error_from(EMFILE, &err));
  ASSERT_EQ(FOPEN_TOO_MANY_OPEN_FILES, err);
  ASSERT_EQ(CDD_C_SUCCESS, fopen_error_from(ENOMEM, &err));
  ASSERT_EQ(FOPEN_OUT_OF_MEMORY, err);
  ASSERT_EQ(CDD_C_SUCCESS, fopen_error_from(ENOENT, &err));
  ASSERT_EQ(FOPEN_FILE_NOT_FOUND, err);
  ASSERT_EQ(CDD_C_SUCCESS, fopen_error_from(EACCES, &err));
  ASSERT_EQ(FOPEN_PERMISSION_DENIED, err);
  ASSERT_EQ(CDD_C_SUCCESS, fopen_error_from(ERANGE, &err));
  ASSERT_EQ(FOPEN_FILENAME_TOO_LONG, err);
  ASSERT_EQ(CDD_C_SUCCESS, fopen_error_from(-999, &err));
  ASSERT_EQ(FOPEN_UNKNOWN_ERROR, err);
  PASS();
}

TEST test_fs_coverage_basename_full(void) {
  char *out = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_basename(NULL, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_basename("foo", NULL));

  ASSERT_EQ(CDD_C_SUCCESS, get_basename("", &out));
  ASSERT_STR_EQ(".", out);
  C_CDD_FREE(out);
  out = NULL;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, get_basename("", &out));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, get_basename("///", &out));
  ASSERT_STR_EQ("/", out);
  C_CDD_FREE(out);
  out = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, get_basename("\\\\\\", &out));
  ASSERT_STR_EQ("/", out);
  C_CDD_FREE(out);
  out = NULL;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, get_basename("///", &out));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, get_basename("path/to/file.txt", &out));
  ASSERT_STR_EQ("file.txt", out);
  C_CDD_FREE(out);
  out = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, get_basename("path\\to\\file2.txt", &out));
  ASSERT_STR_EQ("file2.txt", out);
  C_CDD_FREE(out);
  out = NULL;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, get_basename("path/to/file.txt", &out));
  g_cdd_alloc_fail = 0;

  /* Single non-separator character to test branch 5 */
  ASSERT_EQ(CDD_C_SUCCESS, get_basename("a", &out));
  ASSERT_STR_EQ("a", out);
  C_CDD_FREE(out);
  out = NULL;

  PASS();
}

TEST test_fs_coverage_dirname_full(void) {
  char *out = NULL;
  const char bslash_path[] = {'\\', 'f', 'o', 'o', '\0'};

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_dirname(NULL, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_dirname("foo", NULL));

  ASSERT_EQ(CDD_C_SUCCESS, get_dirname("", &out));
  ASSERT_STR_EQ(".", out);
  C_CDD_FREE(out);
  out = NULL;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, get_dirname("", &out));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, get_dirname("/foo", &out));
  ASSERT_STR_EQ("/", out);
  C_CDD_FREE(out);
  out = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, get_dirname(bslash_path, &out));
  ASSERT_STR_EQ("/", out);
  C_CDD_FREE(out);
  out = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, get_dirname("\\\\foo", &out));
  ASSERT_STR_EQ("\\", out);
  C_CDD_FREE(out);
  out = NULL;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, get_dirname("/foo", &out));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, get_dirname("foo", &out));
  ASSERT_STR_EQ(".", out);
  C_CDD_FREE(out);
  out = NULL;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, get_dirname("foo", &out));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, get_dirname("a/b/c", &out));
  ASSERT_STR_EQ("a/b", out);
  C_CDD_FREE(out);
  out = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, get_dirname("a\\b\\c", &out));
  ASSERT_STR_EQ("a\\b", out);
  C_CDD_FREE(out);
  out = NULL;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, get_dirname("a/b/c", &out));
  g_cdd_alloc_fail = 0;

  PASS();
}

TEST test_fs_coverage_file_io(void) {
  char *data = NULL;
  size_t size = 0;
  char tmp_file[128];
  FILE *fh = NULL;

  CDD_SNPRINTF(tmp_file, sizeof(tmp_file), "fs_cov_io_%d.tmp", rand());

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_write_to_file(NULL, "data"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fs_write_to_file(tmp_file, NULL));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            fs_write_to_file("/does_not_exist_dir/foo.txt", "abc"));

  ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(tmp_file, "line1\nline2\n"));

  /* Write failure */
  g_fail_io_after = 88;
  ASSERT_EQ(CDD_C_ERROR_IO, fs_write_to_file(tmp_file, "fail_fputs"));
  g_fail_io_after = -1;

  /* Close failure */
  g_fail_io_after = 89;
  ASSERT_EQ(CDD_C_ERROR_IO, fs_write_to_file(tmp_file, "fail_close"));
  g_fail_io_after = -1;

  /* Both write and close failure */
  g_fail_io_after = 85;
  ASSERT_EQ(CDD_C_ERROR_IO, fs_write_to_file(tmp_file, "both_fail"));
  g_fail_io_after = -1;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            read_to_file(NULL, "r", &data, &size));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            read_to_file(tmp_file, NULL, &data, &size));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            read_to_file(tmp_file, "r", NULL, &size));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            read_to_file(tmp_file, "r", &data, NULL));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            read_to_file("no_such_file_12345.xyz", "r", &data, &size));

  ASSERT_EQ(CDD_C_SUCCESS, read_to_file(tmp_file, "r", &data, &size));
  ASSERT_STR_EQ("both_fail", data);
  C_CDD_FREE(data);
  data = NULL;

  /* Close error with successful read */
  g_fail_io_after = 86;
  ASSERT_EQ(CDD_C_ERROR_IO, read_to_file(tmp_file, "r", &data, &size));
  g_fail_io_after = -1;

  /* Close error with failed read */
  g_fail_io_after = 87;
  ASSERT_EQ(CDD_C_ERROR_IO, read_to_file(tmp_file, "r", &data, &size));
  g_fail_io_after = -1;

  fh = fopen(tmp_file, "r");
  ASSERT(fh != NULL);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, read_from_fh(NULL, &data, &size));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, read_from_fh(fh, NULL, &size));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, read_from_fh(fh, &data, NULL));

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, read_from_fh(fh, &data, &size));
  g_cdd_alloc_fail = 0;

  fclose(fh);

  /* Test read_from_fh on write-only stream to trigger ferror with errno != 0 */
  fh = fopen(tmp_file, "w");
  ASSERT(fh != NULL);
  g_fail_io_after = 98;
  ASSERT_EQ(CDD_C_ERROR_IO, read_from_fh(fh, &data, &size));
  g_fail_io_after = -1;

  /* Test read_from_fh on write-only stream with errno == 0 (g_fail_io_after ==
   * 99) */
  g_fail_io_after = 99;
  ASSERT_EQ(CDD_C_ERROR_IO, read_from_fh(fh, &data, &size));
  g_fail_io_after = -1;
  fclose(fh);

  remove(tmp_file);
  PASS();
}

TEST test_fs_coverage_cp(void) {
  char src_file[128];
  char dst_file[128];
  char dst_file2[128];
  char dst_file3[128];
  char dst_file4[128];
  char dst_file5[128];
  char dst_file6[128];

  CDD_SNPRINTF(src_file, sizeof(src_file), "fs_cov_cp_src_%d.tmp", rand());
  CDD_SNPRINTF(dst_file, sizeof(dst_file), "fs_cov_cp_dst_%d.tmp", rand());
  CDD_SNPRINTF(dst_file2, sizeof(dst_file2), "fs_cov_cp_dst2_%d.tmp", rand());
  CDD_SNPRINTF(dst_file3, sizeof(dst_file3), "fs_cov_cp_dst3_%d.tmp", rand());
  CDD_SNPRINTF(dst_file4, sizeof(dst_file4), "fs_cov_cp_dst4_%d.tmp", rand());
  CDD_SNPRINTF(dst_file5, sizeof(dst_file5), "fs_cov_cp_dst5_%d.tmp", rand());
  CDD_SNPRINTF(dst_file6, sizeof(dst_file6), "fs_cov_cp_dst6_%d.tmp", rand());

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cp(NULL, src_file));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cp(dst_file, NULL));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cp(dst_file, "does_not_exist_src.tmp"));

  ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(src_file, "copy content"));
  ASSERT_EQ(CDD_C_SUCCESS, cp(dst_file, src_file));
  ASSERT_NEQ(CDD_C_SUCCESS, cp(dst_file, src_file)); /* Already exists */

  /* Write error via 66 */
  g_fail_io_after = 66;
  ASSERT_EQ(CDD_C_ERROR_IO, cp(dst_file2, src_file));
  g_fail_io_after = -1;

  /* Read error in cp */
  g_fail_io_after = 77;
  ASSERT_EQ(CDD_C_ERROR_IO, cp(dst_file3, src_file));
  g_fail_io_after = -1;

  /* Close error in cp */
  g_fail_io_after = 64;
  ASSERT_EQ(CDD_C_ERROR_IO, cp(dst_file4, src_file));
  g_fail_io_after = -1;

  /* Multi-iteration write loop via partial write 63 */
  g_fail_io_after = 63;
  ASSERT_EQ(CDD_C_SUCCESS, cp(dst_file6, src_file));
  g_fail_io_after = -1;

  /* EINTR handling in cp write loop via hook 62 */
  g_fail_io_after = 62;
  ASSERT_EQ(CDD_C_SUCCESS, cp(dst_file5, src_file));
  g_fail_io_after = -1;

  remove(src_file);
  remove(dst_file);
  remove(dst_file2);
  remove(dst_file3);
  remove(dst_file4);
  remove(dst_file5);
  remove(dst_file6);
  PASS();
}

TEST test_fs_coverage_makedir_makedirs(void) {
  char dir_path[64];
  char file_path[256];

  CDD_SNPRINTF(dir_path, sizeof(dir_path), "fs_cov_dir_%d", rand());
  CDD_SNPRINTF(file_path, sizeof(file_path), "%s/file.txt", dir_path);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, makedir(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, makedir(""));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, makedirs(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, makedirs(""));
  ASSERT_EQ(CDD_C_SUCCESS, makedirs("/"));

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, makedirs("a/b/c"));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, makedir(dir_path));
  ASSERT_NEQ(CDD_C_SUCCESS, makedir(dir_path)); /* EEXIST */

  ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(file_path, "txt"));

  /* Test maybe_mkdir when path exists and is NOT a directory */
  ASSERT_EQ(CDD_C_ERROR_IO, makedirs(file_path));

  /* Test maybe_mkdir non-EEXIST error */
  g_fail_io_after = 55;
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, makedirs("non_eexist_dir/sub"));
  g_fail_io_after = -1;

  /* Test maybe_mkdir io failure early */
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO, makedirs("io_fail_dir/sub"));
  g_fail_io_after = -1;

  /* Test maybe_mkdir with stat error */
  g_fail_io_after = 53;
  ASSERT_EQ(CDD_C_ERROR_IO, makedirs(dir_path));
  g_fail_io_after = -1;

  /* Backslash in makedirs */
  ASSERT_EQ(CDD_C_SUCCESS, makedirs("fs_cov_b\\sub1\\sub2"));
  rmdir("fs_cov_b/sub1/sub2");
  rmdir("fs_cov_b/sub1");
  rmdir("fs_cov_b");

  remove(file_path);
  rmdir(dir_path);
  PASS();
}

TEST test_fs_coverage_tempdir(void) {
  char *path = NULL;
  char orig_tmpdir[512];
  char orig_tmp[512];
  char orig_temp[512];
  const char *env_val;

  orig_tmpdir[0] = '\0';
  orig_tmp[0] = '\0';
  orig_temp[0] = '\0';

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, tempdir(NULL));

  ASSERT_EQ(CDD_C_SUCCESS, tempdir(&path));
  ASSERT(path != NULL);
  C_CDD_FREE(path);
  path = NULL;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, tempdir(&path));
  g_cdd_strdup_fail = 0;

  /* Save environment variables */
  env_val = getenv("TMPDIR");
  if (env_val)
    CDD_STRCPY(orig_tmpdir, sizeof(orig_tmpdir), env_val);
  env_val = getenv("TMP");
  if (env_val)
    CDD_STRCPY(orig_tmp, sizeof(orig_tmp), env_val);
  env_val = getenv("TEMP");
  if (env_val)
    CDD_STRCPY(orig_temp, sizeof(orig_temp), env_val);

  /* Test fallback when TMPDIR is unset, but TMP is set */
  test_unsetenv("TMPDIR");
  test_setenv("TMP", "/tmp_test_tmp");
  test_unsetenv("TEMP");
  ASSERT_EQ(CDD_C_SUCCESS, tempdir(&path));
  ASSERT_STR_EQ("/tmp_test_tmp", path);
  C_CDD_FREE(path);
  path = NULL;

  /* Test fallback when TMPDIR and TMP are unset, but TEMP is set */
  test_unsetenv("TMPDIR");
  test_unsetenv("TMP");
  test_setenv("TEMP", "/tmp_test_temp");
  ASSERT_EQ(CDD_C_SUCCESS, tempdir(&path));
  ASSERT_STR_EQ("/tmp_test_temp", path);
  C_CDD_FREE(path);
  path = NULL;

  /* Test fallback when all are set to empty string "" */
  test_setenv("TMPDIR", "");
  test_setenv("TMP", "");
  test_setenv("TEMP", "");
  ASSERT_EQ(CDD_C_SUCCESS, tempdir(&path));
  ASSERT(path != NULL);
  C_CDD_FREE(path);
  path = NULL;

  /* Test fallback when all are unset */
  test_unsetenv("TMPDIR");
  test_unsetenv("TMP");
  test_unsetenv("TEMP");
  ASSERT_EQ(CDD_C_SUCCESS, tempdir(&path));
  ASSERT(path != NULL);
  C_CDD_FREE(path);
  path = NULL;

  /* Test OOM on fallback */
  g_cdd_strdup_fail = 1;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, tempdir(&path));
  g_cdd_strdup_fail = 0;
  g_cdd_alloc_fail = 0;

  /* Restore environment */
  if (orig_tmpdir[0])
    test_setenv("TMPDIR", orig_tmpdir);
  if (orig_tmp[0])
    test_setenv("TMP", orig_tmp);
  else
    test_unsetenv("TMP");
  if (orig_temp[0])
    test_setenv("TEMP", orig_temp);
  else
    test_unsetenv("TEMP");

  PASS();
}

TEST test_fs_coverage_filename_and_ptr(void) {
  struct FilenameAndPtr fap;
  char tmp_file[128];

  CDD_SNPRINTF(tmp_file, sizeof(tmp_file), "fs_cov_fap_%d.tmp", rand());

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FilenameAndPtr_cleanup(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            FilenameAndPtr_delete_and_cleanup(NULL));

  memset(&fap, 0, sizeof(fap));
  ASSERT_EQ(CDD_C_SUCCESS, FilenameAndPtr_cleanup(&fap));

  memset(&fap, 0, sizeof(fap));
  fap.filename = (char *)(size_t)C_CDD_MALLOC(128);
  CDD_STRCPY(fap.filename, 128, tmp_file);
  ASSERT_EQ(CDD_C_SUCCESS, FilenameAndPtr_cleanup(&fap));
  ASSERT(fap.filename == NULL);

  memset(&fap, 0, sizeof(fap));
  fap.fh = fopen(tmp_file, "w");
  ASSERT_EQ(CDD_C_SUCCESS, FilenameAndPtr_cleanup(&fap));
  ASSERT(fap.fh == NULL);

  memset(&fap, 0, sizeof(fap));
  fs_write_to_file(tmp_file, "test");
  fap.filename = (char *)(size_t)C_CDD_MALLOC(128);
  CDD_STRCPY(fap.filename, 128, tmp_file);
  ASSERT_EQ(CDD_C_SUCCESS, FilenameAndPtr_delete_and_cleanup(&fap));
  ASSERT(fap.filename == NULL);

  /* Cleanup with NULL filename */
  memset(&fap, 0, sizeof(fap));
  fap.filename = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, FilenameAndPtr_delete_and_cleanup(&fap));

  PASS();
}

TEST test_fs_coverage_mktmpfile(void) {
  struct FilenameAndPtr fap;

  memset(&fap, 0, sizeof(fap));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            mktmpfilegetnameandfile("p_", ".t", "w+", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            mktmpfilegetnameandfile("p_", ".t", NULL, &fap));

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            mktmpfilegetnameandfile("p_", ".t", "w+", &fap));
  g_cdd_strdup_fail = 0;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            mktmpfilegetnameandfile("p_", ".t", "w+", &fap));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, mktmpfilegetnameandfile("pfx_", ".sfx", "w+", &fap));
  ASSERT(fap.fh != NULL);
  ASSERT(fap.filename != NULL);
  ASSERT_EQ(CDD_C_SUCCESS, FilenameAndPtr_delete_and_cleanup(&fap));

  ASSERT_EQ(CDD_C_SUCCESS, mktmpfilegetnameandfile(NULL, NULL, "w+", &fap));
  ASSERT(fap.fh != NULL);
  ASSERT(fap.filename != NULL);
  ASSERT_EQ(CDD_C_SUCCESS, FilenameAndPtr_delete_and_cleanup(&fap));

  /* Test collision retry hook */
  g_fail_io_after = 44;
  ASSERT_EQ(CDD_C_SUCCESS,
            mktmpfilegetnameandfile("collide_", ".tmp", "w+", &fap));
  g_fail_io_after = -1;
  FilenameAndPtr_delete_and_cleanup(&fap);

  /* Exhaust iterations */
  g_fail_io_after = 45;
  ASSERT_EQ(CDD_C_ERROR_IO, mktmpfilegetnameandfile("p_", ".t", "w+", &fap));
  g_fail_io_after = -1;

  PASS();
}

TEST test_fs_coverage_walk_directory(void) {
  int count = 0;
  char root_dir[32];
  char sub_dir[64];
  char f1[128];
  char f2[128];

  CDD_SNPRINTF(root_dir, sizeof(root_dir), "fs_cov_walk_%d", rand());
  CDD_SNPRINTF(sub_dir, sizeof(sub_dir), "%s/sub", root_dir);
  CDD_SNPRINTF(f1, sizeof(f1), "%s/a.txt", root_dir);
  CDD_SNPRINTF(f2, sizeof(f2), "%s/b.txt", sub_dir);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            walk_directory(NULL, mock_walk_count_cb, &count));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, walk_directory(".", NULL, &count));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            walk_directory("nonexistent_dir_never_exists", mock_walk_count_cb,
                           &count));

  makedirs(sub_dir);
  fs_write_to_file(f1, "1");
  fs_write_to_file(f2, "2");

  /* Test walking single file */
  count = 0;
  ASSERT_EQ(CDD_C_SUCCESS, walk_directory(f1, mock_walk_count_cb, &count));
  ASSERT_EQ(1, count);

  /* Test walking tree */
  count = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            walk_directory(root_dir, mock_walk_count_cb, &count));
  ASSERT_EQ(2, count);

  /* Test callback abort */
  ASSERT_EQ(CDD_C_ERROR_IO, walk_directory(root_dir, mock_walk_err_cb, NULL));

  /* Test opendir failure */
  g_fail_io_after = 33;
  ASSERT_EQ(CDD_C_ERROR_IO,
            walk_directory(root_dir, mock_walk_count_cb, &count));
  g_fail_io_after = -1;

  /* Test stat failure on child */
  g_fail_io_after = 32;
  ASSERT_EQ(CDD_C_ERROR_IO,
            walk_directory(root_dir, mock_walk_count_cb, &count));
  g_fail_io_after = -1;

  /* Test asprintf failure inside walk_directory */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            walk_directory(root_dir, mock_walk_count_cb, &count));
  g_cdd_alloc_fail = 0;

  /* Test asprintf failure when alloc fail is 2 */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            walk_directory(root_dir, mock_walk_count_cb, &count));
  g_cdd_alloc_fail = 0;

  remove(f2);
  remove(f1);
  rmdir(sub_dir);
  rmdir(root_dir);

  PASS();
}

SUITE(fs_coverage_suite) {
  RUN_TEST(test_fs_coverage_path_helpers);
  RUN_TEST(test_fs_coverage_errno_to_cdd_error);
  RUN_TEST(test_fs_coverage_is_directory);
  RUN_TEST(test_fs_coverage_fopen_error_from);
  RUN_TEST(test_fs_coverage_basename_full);
  RUN_TEST(test_fs_coverage_dirname_full);
  RUN_TEST(test_fs_coverage_file_io);
  RUN_TEST(test_fs_coverage_cp);
  RUN_TEST(test_fs_coverage_makedir_makedirs);
  RUN_TEST(test_fs_coverage_tempdir);
  RUN_TEST(test_fs_coverage_filename_and_ptr);
  RUN_TEST(test_fs_coverage_mktmpfile);
  RUN_TEST(test_fs_coverage_walk_directory);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !TEST_FS_COVERAGE_H */
