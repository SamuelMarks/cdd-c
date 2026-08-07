#ifndef CDD_BUILD_TESTS
#define CDD_BUILD_TESTS
#endif
#include "cdd_c_error.h"
/**
 * @file cdd_helpers.c
 * @brief Implementation of test helpers.
 * @author Samuel Marks
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>

#include "cdd_helpers.h"
/* clang-format on */
#include <errno.h>

#include "c_cdd_export.h"
C_CDD_EXPORT int g_cdd_helpers_fopen_err = 0;

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;

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

#define FOPEN_S mock_fopen_s
#define FOPEN mock_fopen
#define FPUTS(str, stream)                                                     \
  ((g_fail_io_after >= 0 && ++g_io_calls == g_fail_io_after)                   \
       ? -1                                                                    \
       : fputs(str, stream))
#define FCLOSE(stream)                                                         \
  ((g_fail_io_after >= 0 && ++g_io_calls == g_fail_io_after) ? -1              \
                                                             : fclose(stream))
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
 * @param[in] filename The name of the file to write to.
 * @param[in] contents The contents to write.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
cdd_c_error_t write_to_file(const char *const filename,
                            const char *const contents) {
  FILE *fh;
  int rc = CDD_C_SUCCESS;

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
