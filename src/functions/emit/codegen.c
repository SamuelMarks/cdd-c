/**
 * @file codegen.c
 * @brief Implementation of shared code generation utilities.
 *
 * Contains legacy or shared functionality not yet migrated to specific modules.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>

#include "functions/emit/codegen.h"
/* clang-format on */

/**
 * @brief Generates C code for write forward decl.
 */
int write_forward_decl(FILE *fp, const char *struct_name) {
  if (!fp || !struct_name) {
    return EINVAL;
  }
  if (fprintf(fp, "struct %s;\n", struct_name) < 0) {
    return EIO;
  }
  return 0;
}
