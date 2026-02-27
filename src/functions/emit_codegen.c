/**
 * @file codegen.c
 * @brief Implementation of shared code generation utilities.
 *
 * Contains legacy or shared functionality not yet migrated to specific modules.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>

#include "functions/emit_codegen.h"

int write_forward_decl(FILE *const fp, const char *const struct_name) {
  if (!fp || !struct_name) {
    return EINVAL;
  }
  if (fprintf(fp, "struct %s;\n", struct_name) < 0) {
    return EIO;
  }
  return 0;
}
