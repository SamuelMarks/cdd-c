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
/* LCOV_EXCL_START */
#include <errno.h>

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
enum cdd_c_error write_to_file(const char *const filename,
                               const char *const contents) {
  FILE *fh;
  int rc = CDD_C_SUCCESS;

  if (filename == NULL || contents == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err;
    err = fopen_s(&fh, filename, "w");
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
  fopen_s(&fh, filename, "w");
#else
  fh = fopen(filename, "w");
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

  if (fputs(contents, fh) < 0) {
    fprintf(stderr, "Failure to write to %s\n", filename);
    rc = CDD_C_ERROR_IO;
  }

  if (fclose(fh) != 0) {
    rc = CDD_C_ERROR_IO;
  }

  return rc;
}

/* LCOV_EXCL_STOP */
