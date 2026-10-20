#ifndef CDD_BUILD_TESTS
/**
 * @brief Macro CDD_BUILD_TESTS.
 */
#define CDD_BUILD_TESTS
#endif
#include "cdd_c_error.h"
/**
 * @file cdd_helpers.c
 * @brief Implementation of test helpers.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "c_cdd_export.h"
#include "cdd_helpers.h"
#include "cdd_test_helpers_export.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */
/** @brief Test definition g_io_calls. */
extern C_CDD_EXPORT int g_io_calls;
/** @brief Test definition g_fail_io_after. */
extern C_CDD_EXPORT int g_fail_io_after;
/** @brief Test definition g_cdd_helpers_fopen_err. */
CDD_TEST_HELPERS_EXPORT int g_cdd_helpers_fopen_err = 0;

#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
/* extern C_CDD_EXPORT int g_io_calls; (moved to global) */

/**
 * @brief Helper function mock_fopen.
 * @param[in] path Parameter path.
 * @param[in] mode Parameter mode.
 * @return Result status or value.
 */
static FILE *mock_fopen(const char *path, const char *mode) {
  if (g_fail_io_after >= 0 && ++g_io_calls == g_fail_io_after) {
    errno = g_cdd_helpers_fopen_err;
    return NULL;
  }
  return fopen(path, mode);
}

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
static int mock_fopen_s(FILE **fh, const char *path, const char *mode) {
  if (g_fail_io_after >= 0 && ++g_io_calls == g_fail_io_after) {
    *fh = NULL;
    return g_cdd_helpers_fopen_err;
  }
  return fopen_s(fh, path, mode);
}
#endif

/**
 * @brief Macro FOPEN_S.
 */
#define FOPEN_S mock_fopen_s
/**
 * @brief Macro FOPEN.
 */
#define FOPEN mock_fopen
/**
 * @brief Helper function mock_fputs.
 * @param[in] str Parameter str.
 * @param[in,out] stream Parameter stream.
 * @return Result status or value.
 */
static int mock_fputs(const char *str, FILE *stream) {
  if (g_fail_io_after >= 0 && ++g_io_calls == g_fail_io_after)
    return -1;
  return fputs(str, stream);
}
/**
 * @brief Helper function mock_fclose.
 * @param[in,out] stream Parameter stream.
 * @return Result status or value.
 */
static int mock_fclose(FILE *stream) {
  if (g_fail_io_after >= 0 && ++g_io_calls == g_fail_io_after) {
    if (stream)
      fclose(stream);
    return -1;
  }
  return fclose(stream);
}
/**
 * @brief Macro FPUTS.
 */
#define FPUTS mock_fputs
/**
 * @brief Macro FCLOSE.
 */
#define FCLOSE mock_fclose
#else
#define FOPEN_S fopen_s
#define FOPEN fopen
#define FPUTS fputs
#define FCLOSE fclose
#endif

/**
 * @brief Logs a precondition failure to stderr.
 */
void cdd_precondition_failed(void) {
  fputs("cdd_precondition_failed\n", stderr);
}

/**
 * @brief Writes contents to a file safely.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
cdd_c_error_t write_to_file(const char *filename, const char *contents) {
  FILE *fh;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (filename == NULL || contents == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err;
    err = FOPEN_S(&fh, filename, "w");
    if (err != 0 || fh == NULL) {
      if (err == ENOENT)
        return CDD_C_ERROR_NOT_FOUND;
      if (err == ENOMEM)
        return CDD_C_ERROR_MEMORY;
      if (err == EINVAL)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#else
#if defined(_MSC_VER)
  FOPEN_S(&fh, filename, "w");
#else
  fh = FOPEN(filename, "w");
#endif
  if (fh == NULL) {
    if (errno == ENOENT)
      return CDD_C_ERROR_NOT_FOUND;
    if (errno == ENOMEM)
      return CDD_C_ERROR_MEMORY;
    if (errno == EINVAL)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  if (FPUTS(contents, fh) < 0) {
    fprintf(stderr, "Failure to write to %s\n", filename);
    rc = CDD_C_ERROR_IO;
  }

  if (FCLOSE(fh) != 0) {
    rc = CDD_C_ERROR_IO;
  }

  return rc;
}

/**
 * @brief Helper function cdd_test_rmdir_recursive.
 * @param[in] path Parameter path.
 */
static void cdd_test_rmdir_recursive(const char *path) {
#if defined(_WIN32)
  struct _finddata_t file_info;
  intptr_t handle;
  char search_path[1024];
  char child_path[1024];

  if (!path || !*path)
    return;
#if defined(_MSC_VER)
  sprintf_s(search_path, sizeof(search_path), "%s\\*", path);
#else
  CDD_SNPRINTF(search_path, sizeof(search_path), "%s\\*", path);
#endif
  handle = _findfirst(search_path, &file_info);
  if (handle != -1) {
    do {
      if (strcmp(file_info.name, ".") == 0 || strcmp(file_info.name, "..") == 0)
        continue;
#if defined(_MSC_VER)
      sprintf_s(child_path, sizeof(child_path), "%s\\%s", path, file_info.name);
#else
      CDD_SNPRINTF(child_path, sizeof(child_path), "%s\\%s", path,
                   file_info.name);
#endif
      if (file_info.attrib & _A_SUBDIR) {
        cdd_test_rmdir_recursive(child_path);
      } else {
        remove(child_path);
      }
    } while (_findnext(handle, &file_info) == 0);
    _findclose(handle);
  }
  _rmdir(path);
#else
  DIR *d;
  struct dirent *entry;
  struct stat st;
  char child_path[4096];

  if (!path || !*path)
    return;
  chmod(path, 0755);
  d = opendir(path);
  if (d) {
    while ((entry = readdir(d)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      CDD_SNPRINTF(child_path, sizeof(child_path), "%s/%s", path,
                   entry->d_name);
      if (stat(child_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        cdd_test_rmdir_recursive(child_path);
      } else {
        remove(child_path);
      }
    }
    closedir(d);
  }
  rmdir(path);
#endif
}

/** @brief Test definition g_cdd_test_tmp_buf[1024][64]. */
char g_cdd_test_tmp_buf[1024][64];
/** @brief Test definition g_cdd_test_mut_pool[1024][512]. */
static char g_cdd_test_mut_pool[1024][512];
/** @brief Test definition g_cdd_test_mut_idx. */
static size_t g_cdd_test_mut_idx = 0;

/**
 * @brief Returns a mutable copy of a string in a rotating static test buffer.
 *
 * @param[in] str Input string.
 * @return Mutable pointer to a copy of str, or NULL if str is NULL.
 */
CDD_TEST_HELPERS_EXPORT char *cdd_test_mut_str(const char *str) {
  char *dst;
  size_t len;
  if (!str)
    return NULL;
  dst = g_cdd_test_mut_pool[g_cdd_test_mut_idx];
  g_cdd_test_mut_idx = (g_cdd_test_mut_idx + 1) & 1023;
  len = strlen(str);
  if (len >= sizeof(g_cdd_test_mut_pool[0]))
    len = sizeof(g_cdd_test_mut_pool[0]) - 1;
  memcpy(dst, str, len);
  dst[len] = '\0';
  return dst;
}

/**
 * @brief Create a temporary file in the test sandbox and return its stream.
 *
 * @return Open FILE pointer on success, or NULL on error.
 */
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void) {
  static int counter = 0;
  FILE *f;
  if (counter >= 1024)
    counter = 0;
#if defined(_MSC_VER)
  sprintf_s(g_cdd_test_tmp_buf[counter], sizeof(g_cdd_test_tmp_buf[counter]),
            "cdd_test_tmp_%d.txt", counter);
#else
  CDD_SNPRINTF(g_cdd_test_tmp_buf[counter], sizeof(g_cdd_test_tmp_buf[counter]),
               "cdd_test_tmp_%d.txt", counter);
#endif
  remove(g_cdd_test_tmp_buf[counter]);
#if defined(_MSC_VER)
  if (fopen_s(&f, g_cdd_test_tmp_buf[counter], "w+b") != 0)
    f = NULL;
#else
  f = fopen(g_cdd_test_tmp_buf[counter], "w+b");
#endif
  counter++;
  return f;
}

/**
 * @brief Clean up all temporary files and directories created by tests.
 */
CDD_TEST_HELPERS_EXPORT void cdd_test_tmpfile_cleanup(void) {
  int i;
  static const char *const k_transient_files[] = {
      "impl.c",
      "invalid_spec.json",
      "out.c",
      "out.json",
      "out2.json",
      "docs.json",
      "docs_out.json",
      "out_schema.json",
      "dummy_spec.json",
      "test_c2openapi_eof.c",
      "test_c2openapi_gui_only.c",
      "test_c2openapi_infer.c",
      "test_c2openapi_infer2.c",
      "test_c2openapi_infer3.c",
      "test_c2openapi_infer4.c",
      "test_c2openapi_infer5.c",
      "test_c2openapi_infer_fail.c",
      "test_c2openapi_last_tok.c",
      "test_c2openapi_no_brace.c",
      "test_c2openapi_only_infer.c",
      "test_c2openapi_oom_build.c",
      "test_c2openapi_render_extra.c",
      "test_codegen_schema_io.json.c",
      "test_codegen_schema_io.json.h",
      "test_empty.c",
      "test_empty.txt",
      "test_nif.c",
      "test_out_dsl.dsl",
      "test_router_edge.c",
      "test_tcl.c",
      "test_valid_infer.c",
      "union_array_schema.json.c",
      "union_array_schema.json.h",
      "cdd_test_tmp_union.txt",
      "cdd_test_tmp_hdr.h",
      "cdd_test_tmp_out.json",
      "cdd_test_tmp_unclosed.h",
      "cdd_test_tmp_nested_fail.h",
      "cdd_test_tmp_struct_fail.h",
      "cdd_test_tmp_enum_fail.h"};
  static const char *const k_transient_dirs[] = {"my_empty_dir",
                                                 "non_eexist_dir",
                                                 "out",
                                                 "out_dir",
                                                 "out_dir_io",
                                                 "out_sdk",
                                                 "out_sdk_cli",
                                                 "out_server",
                                                 "test_bindings_out",
                                                 "test_build_dir",
                                                 "test_build_dir_io",
                                                 "test_build_dir_oom",
                                                 "test_build_dir_readonly",
                                                 "test_build_dir_readonly2",
                                                 "test_build_dir_tests",
                                                 "test_cli_bind_out",
                                                 "test_cpp_out",
                                                 "test_csharp_out",
                                                 "test_dart_out",
                                                 "test_dummy_dir_for_makedirs",
                                                 "test_ffi_e2e_out",
                                                 "test_go_out",
                                                 "test_haskell_out",
                                                 "test_java_out",
                                                 "test_julia_out",
                                                 "test_kotlin_out",
                                                 "test_lua_out",
                                                 "test_m_dir",
                                                 "test_matlab_out",
                                                 "test_napi_dir",
                                                 "test_napi_out",
                                                 "test_ocaml_out",
                                                 "test_odin_out",
                                                 "test_out_swift_io",
                                                 "test_php_out",
                                                 "test_pom_dir",
                                                 "test_python_out",
                                                 "test_r_out",
                                                 "test_ruby_out",
                                                 "test_rust_out",
                                                 "test_swift_out",
                                                 "test_typescript_out",
                                                 "test_zig_out"};

  for (i = 0; i < 1024; i++) {
    char buf[64];
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "cdd_test_tmp_%d.txt", i);
#else
    CDD_SNPRINTF(buf, sizeof(buf), "cdd_test_tmp_%d.txt", i);
#endif
    remove(buf);
  }
  for (i = 0;
       i < (int)(sizeof(k_transient_files) / sizeof(k_transient_files[0]));
       i++) {
    remove(k_transient_files[i]);
  }
  for (i = 0; i < (int)(sizeof(k_transient_dirs) / sizeof(k_transient_dirs[0]));
       i++) {
    cdd_test_rmdir_recursive(k_transient_dirs[i]);
  }
}
