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

/* clang-format off */
#include "c_cdd_export.h"
#include "classes/emit/enum.h"
#include "classes/emit/json.h"
#include "classes/emit/struct.h"
#include "classes/emit/types.h"

#include <stdio.h>
/* clang-format on */

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
extern C_CDD_EXPORT /**
                     * @brief Generates C code for write forward decl.
                     */
    int
    write_forward_decl(FILE *fp, const char *struct_name);

/**
 * @brief Generates C code for write enum declaration.
 *
 * @param[in] hfile Output header stream.
 * @param[in] enum_name The enum name.
 * @param[in] sf The struct fields representing the enum.
 * @param[in] config The codegen configuration.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_enum_declaration_h(FILE *hfile, const char *enum_name,
                         const struct StructFields *sf,
                         const struct CodegenConfig *config);

/**
 * @brief Generates C code for write union declaration.
 *
 * @param[in] hfile Output header stream.
 * @param[in] union_name The union name.
 * @param[in] sf The struct fields representing the union.
 * @param[in] config The codegen configuration.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_union_declaration_h(FILE *hfile, const char *union_name,
                          const struct StructFields *sf,
                          const struct CodegenConfig *config);

/**
 * @brief Generates C code for write struct declaration.
 *
 * @param[in] hfile Output header stream.
 * @param[in] struct_name The struct name.
 * @param[in] sf The struct fields.
 * @param[in] config The codegen configuration.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_declaration_h(FILE *hfile, const char *struct_name,
                           const struct StructFields *sf,
                           const struct CodegenConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_H */
