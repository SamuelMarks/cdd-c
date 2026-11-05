/**
 * @file cli_struct_internal.h
 * @brief Internal shared helpers for declarative C89 CLI code generation.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CLASSES_EMIT_CLI_STRUCT_INTERNAL_H
#define C_CDD_CLASSES_EMIT_CLI_STRUCT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
#include <stdio.h>

#include "c_cdd/code_writer.h"
#include "classes/emit/cli_struct.h"
#include "classes/parse/inspector.h"
/* clang-format on */

/** @brief Redirects fprintf through the testable I/O hook */
#define FPRINTF_HOOK cdd_io_fprintf_hook

/** @brief Checks fprintf return value and percolates CDD_C_ERROR_IO */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

/**
 * @brief Copies `src` into `dst` with bounds checking.
 *
 * @param[out] dst Destination buffer.
 * @param[in] dst_sz Capacity of `dst`.
 * @param[in] src Source NUL-terminated string.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_cli_copy_str(char *dst, size_t dst_sz,
                                      const char *src);

/**
 * @brief Determines the effective `enum CddCliAction` for a field.
 *
 * @param[in] f Field descriptor.
 * @param[out] out_action Receives the effective `enum CddCliAction`.
 * @return `CDD_C_SUCCESS`.
 */
extern cdd_c_error_t cdd_cli_effective_action(const struct StructField *f,
                                              enum CddCliAction *out_action);

/**
 * @brief Looks up a `TypeDefinition` by name in an optional `TypeDefList`.
 *
 * @param[in] reg Type registry (may be NULL).
 * @param[in] name Type name to search for.
 * @param[in] kind Expected `TypeDefinitionKind` (`KIND_ENUM` or `KIND_STRUCT`).
 * @param[out] out_def Matching `TypeDefinition` pointer, or NULL if not found.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t
cdd_cli_find_type_def(const struct TypeDefList *reg, const char *name,
                      enum TypeDefinitionKind kind,
                      const struct TypeDefinition **out_def);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CLASSES_EMIT_CLI_STRUCT_INTERNAL_H */
