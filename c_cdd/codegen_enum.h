/**
 * @file codegen_enum.h
 * @brief Enum code generation module.
 *
 * Provides functionality to extract enum definitions from C code and generate:
 * - `_to_str` functions (String serialization)
 * - `_from_str` functions (String deserialization with error handling)
 *
 * This module is designed to be independent of JSON logic, focusing strictly
 * on C Enum <-> C String conversion.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_ENUM_H
#define C_CDD_CODEGEN_ENUM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdio.h>

#include "c_cdd_export.h"
#include "c_cdd_stdbool.h"

/**
 * @brief Container for enum members extracted from code or schema.
 * Stores a list of strings representing the enum constants.
 */
struct EnumMembers {
  size_t capacity; /**< Allocated capacity of members array */
  size_t size;     /**< Number of members currently stored */
  char **members;  /**< Dynamic array of member names */
};

/**
 * @brief Configuration options for enum code generation.
 * Currently supports optional build guards.
 */
struct CodegenEnumConfig {
  /**
   * @brief Macro name to guard generated functions (e.g. "TO_ENUM").
   * If NULL, no #ifdef/#endif block is generated.
   */
  const char *guard_macro;
};

/**
 * @brief Initialize an EnumMembers container.
 * Must be called before use.
 *
 * @param[out] em Pointer to the container to initialize.
 * @return 0 on success, EINVAL if em is NULL, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT int enum_members_init(struct EnumMembers *em);

/**
 * @brief Free memory associated with an EnumMembers container.
 * Frees all member strings and the array itself.
 *
 * @param[in] em Pointer to the container. Safe to call on NULL or zeroed
 * structs.
 */
extern C_CDD_EXPORT void enum_members_free(struct EnumMembers *em);

/**
 * @brief Add a member to the EnumMembers container.
 * Resizes internal storage if necessary. copies the name string.
 *
 * @param[in,out] em Pointer to the container.
 * @param[in] name Name of the enum member.
 * @return 0 on success, ENOMEM on failure to allocate or resize.
 */
extern C_CDD_EXPORT int enum_members_add(struct EnumMembers *em,
                                         const char *name);

/**
 * @brief Generate the `_from_str` implementation for an enum.
 *
 * Generates a C function `int Name_from_str(const char *str, enum Name *out)`
 * that performs string comparisons to match enum values.
 * Handles validation: returns 0 on success, EINVAL on error.
 * Sets `*out` to `Name_UNKNOWN` if no match found.
 *
 * @param[in] fp Output file stream.
 * @param[in] enum_name Name of the C enum type.
 * @param[in] em Container of enum member strings.
 * @param[in] config Optional configuration for guards (can be NULL).
 * @return 0 on success, error code (EINVAL/EIO) on failure.
 */
extern C_CDD_EXPORT int
write_enum_from_str_func(FILE *fp, const char *enum_name,
                         const struct EnumMembers *em,
                         const struct CodegenEnumConfig *config);

/**
 * @brief Generate the `_to_str` implementation for an enum.
 *
 * Generates a C function `int Name_to_str(enum Name val, char **out)`
 * that switches on the enum value and returns a malloc'd string copy.
 *
 * @param[in] fp Output file stream.
 * @param[in] enum_name Name of the C enum type.
 * @param[in] em Container of enum member strings.
 * @param[in] config Optional configuration for guards (can be NULL).
 * @return 0 on success, error code (EINVAL/EIO) on failure.
 */
extern C_CDD_EXPORT int
write_enum_to_str_func(FILE *fp, const char *enum_name,
                       const struct EnumMembers *em,
                       const struct CodegenEnumConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_ENUM_H */
