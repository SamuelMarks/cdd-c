/**
 * @file codegen.h
 * @brief Facade definition for code generation modules.
 *
 * Aggregates specific generation modules (struct, enum, json, types) and
 * defines shared configuration structures.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_H
#define C_CDD_CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "codegen_enum.h"
#include "codegen_json.h"
#include "codegen_struct.h"
#include "codegen_types.h"

#include <stdio.h>

/**
 * @brief Aggregated configuration for code generation.
 */
struct CodegenConfig {
  const char *enum_guard;  /**< Guard for enum functions */
  const char *json_guard;  /**< Guard for JSON functions */
  const char *utils_guard; /**< Guard for utility functions (cleanup, etc) */
};

/**
 * @brief Write a forward declaration for a struct.
 *
 * @param[in] fp Output file pointer.
 * @param[in] struct_name Name of the struct.
 * @return 0 on success, EIO on error.
 */
extern C_CDD_EXPORT int write_forward_decl(FILE *fp, const char *struct_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_H */
