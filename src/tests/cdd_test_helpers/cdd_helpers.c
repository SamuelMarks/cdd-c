/**
 * @file cdd_helpers.c
 * @brief Implementation of test helpers.
 * @author Samuel Marks
 */

#include <stdio.h>
#include <stdlib.h>

#include "cdd_helpers.h"

void cdd_precondition_failed(void) {
  fputs("cdd_precondition_failed\n", stderr);
}

int write_to_file(const char *const filename, const char *const contents) {
  FILE *fh;
  int rc = 0;

  if (filename == NULL || contents == NULL)
    return EXIT_FAILURE;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err;
    err = fopen_s(&fh, filename, "w, ccs=UTF-8");
    if (err != 0 || fh == NULL) {
      fprintf(stderr, "Failed to open for writing %s\n", filename);
      return EXIT_FAILURE;
    }
  }
#else
  fh = fopen(filename, "w");
  if (fh == NULL) {
    fprintf(stderr, "Failed to open for writing %s\n", filename);
    return EXIT_FAILURE;
  }
#endif

  if (fputs(contents, fh) < 0) {
    fprintf(stderr, "Failure to write to %s\n", filename);
    rc = EXIT_FAILURE;
  }

  if (fclose(fh) != 0) {
    rc = EXIT_FAILURE;
  }

  return rc;
}
