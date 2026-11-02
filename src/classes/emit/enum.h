/**
 * @file enum.h
 * @brief Core Enum code generation definitions and implementation.
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
/* clang-format off */
#include <stddef.h>
#include <stdio.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "c_cdd_stdbool.h"
/* clang-format on */

/**
 * @brief Case transformation rule for Serde-style C89 struct and enum
 * reflection.
 */
enum CddSerdeCaseRule {
  CDD_SERDE_CASE_NONE = 0,            /**< Preserve original C identifier */
  CDD_SERDE_CASE_LOWERCASE,           /**< lowercase */
  CDD_SERDE_CASE_UPPERCASE,           /**< UPPERCASE */
  CDD_SERDE_CASE_PASCALCASE,          /**< PascalCase */
  CDD_SERDE_CASE_CAMELCASE,           /**< camelCase */
  CDD_SERDE_CASE_SNAKECASE,           /**< snake_case */
  CDD_SERDE_CASE_SCREAMING_SNAKECASE, /**< SCREAMING_SNAKE_CASE */
  CDD_SERDE_CASE_KEBABCASE,           /**< kebab-case */
  CDD_SERDE_CASE_SCREAMING_KEBABCASE, /**< SCREAMING-KEBAB-CASE */
  CDD_SERDE_CASE_TRAINCASE            /**< Train-Case */
};

/**
 * @brief Conditional serialization skip predicate kind (`skip_serializing_if`).
 */
enum CddSerdeSkipIfKind {
  CDD_SERDE_SKIP_IF_NONE = 0, /**< Always serialize (unless skipped) */
  CDD_SERDE_SKIP_IF_NULL,     /**< Skip if pointer is NULL */
  CDD_SERDE_SKIP_IF_EMPTY,    /**< Skip if NULL or empty string/array */
  CDD_SERDE_SKIP_IF_ZERO,     /**< Skip if numeric/bool value is zero */
  CDD_SERDE_SKIP_IF_FUNC      /**< Skip if custom C predicate returns nonzero */
};

/**
 * @brief Action behavior when a CLI flag or argument is matched (`structopt` /
 * `clap` parity).
 */
enum CddCliAction {
  CDD_CLI_ACTION_AUTO = 0,  /**< Infer action from C field type */
  CDD_CLI_ACTION_SET,       /**< Consume 1 value and assign to field */
  CDD_CLI_ACTION_SET_TRUE,  /**< Boolean flag sets field to 1 when present */
  CDD_CLI_ACTION_SET_FALSE, /**< Boolean flag sets field to 0 when present */
  CDD_CLI_ACTION_COUNT,     /**< Increment integer counter on each occurrence */
  CDD_CLI_ACTION_APPEND,    /**< Append each occurrence to dynamic array */
  CDD_CLI_ACTION_HELP,      /**< Print help text and return early */
  CDD_CLI_ACTION_VERSION    /**< Print version text and return early */
};

/**
 * @brief Shell completion and help value hint kind (`clap::ValueHint`).
 */
enum CddCliValueHint {
  CDD_CLI_HINT_NONE = 0,   /**< No specific value hint */
  CDD_CLI_HINT_FILE_PATH,  /**< File path completion hint */
  CDD_CLI_HINT_DIR_PATH,   /**< Directory path completion hint */
  CDD_CLI_HINT_ANY_PATH,   /**< Any filesystem path completion hint */
  CDD_CLI_HINT_EXECUTABLE, /**< Executable command completion hint */
  CDD_CLI_HINT_URL,        /**< URL value hint */
  CDD_CLI_HINT_HOSTNAME,   /**< Hostname value hint */
  CDD_CLI_HINT_USERNAME,   /**< Username value hint */
  CDD_CLI_HINT_EMAIL       /**< Email address value hint */
};

/**
 * @brief Container for enum members extracted from code or schema.
 * Stores a list of strings representing the enum constants.
 */
struct EnumMembers {
  size_t capacity;         /**< Allocated capacity of members array */
  size_t size;             /**< Number of members currently stored */
  char **members;          /**< Dynamic array of member names */
  char **serde_names;      /**< Optional explicit wire names per variant */
  int *serde_skip;         /**< Optional skip flags per variant */
  char ***serde_aliases;   /**< Optional per-variant alias string arrays */
  size_t *n_serde_aliases; /**< Count of aliases per variant */
  int serde_other_idx;     /**< Index of `@cdd other` fallback variant or -1 */
  enum CddSerdeCaseRule serde_rename_all; /**< Container-level case rule */
  char serde_replace_from[32]; /**< Container-level substring replace source */
  char serde_replace_to[32];   /**< Container-level substring replace target */
  char serde_tag[64];          /**< Optional container tag name */
  char serde_content[64];      /**< Optional container content name */
  int serde_untagged;          /**< 1 if untagged */

  /* CLI ValueEnum Reflection Metadata */
  enum CddSerdeCaseRule cli_rename_all; /**< Case rule for CLI enum choices */
  int cli_case_insensitive; /**< 1 if ignore_case is active for CLI parsing */
  char **cli_descriptions;  /**< Optional per-variant CLI help descriptions */
  int *cli_hidden;          /**< Optional per-variant CLI hide flags */
};

/**
 * @brief Configuration options for enum code generation.
 * Currently supports optional build guards.
 */
struct CodegenEnumConfig {
  /**
   * @brief Macro name to guard generated functions (e.g. "TO_ENUM").
   * If NULL, no \#ifdef/\#endif block is generated.
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
extern C_CDD_EXPORT cdd_c_error_t enum_members_init(struct EnumMembers *em);

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
extern C_CDD_EXPORT cdd_c_error_t enum_members_add(struct EnumMembers *em,
                                                   const char *name);

/**
 * @brief Add a member to the EnumMembers container with optional serde
 * metadata.
 *
 * @param[in,out] em Pointer to the container.
 * @param[in] name Name of the C enum member.
 * @param[in] serde_name Optional explicit wire name override (can be NULL).
 * @param[in] serde_skip Non-zero if this variant should be skipped in JSON/str
 * codecs.
 * @return 0 on success, ENOMEM on failure to allocate or resize.
 */
extern C_CDD_EXPORT cdd_c_error_t enum_members_add_ex(struct EnumMembers *em,
                                                      const char *name,
                                                      const char *serde_name,
                                                      int serde_skip);

/**
 * @brief Appends a deserialization alias to an enum variant at `member_idx`.
 *
 * @param[in,out] em Pointer to the EnumMembers container.
 * @param[in] member_idx Index of the enum variant.
 * @param[in] alias Alternative wire string to accept in `_from_str`.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t enum_member_add_alias(struct EnumMembers *em,
                                                        size_t member_idx,
                                                        const char *alias);

/**
 * @brief Sets CLI metadata (help description and hidden flag) for an enum
 * variant at `member_idx`.
 *
 * @param[in,out] em Pointer to the EnumMembers container.
 * @param[in] member_idx Index of the enum variant.
 * @param[in] description Optional CLI help text for this variant (can be NULL).
 * @param[in] hidden Non-zero to hide this variant from `--help` output.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
enum_member_set_cli_meta(struct EnumMembers *em, size_t member_idx,
                         const char *description, int hidden);

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
extern C_CDD_EXPORT cdd_c_error_t write_enum_from_str_func(
    FILE *fp, const char *enum_name, const struct EnumMembers *em,
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
extern C_CDD_EXPORT cdd_c_error_t write_enum_to_str_func(
    FILE *fp, const char *enum_name, const struct EnumMembers *em,
    const struct CodegenEnumConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_ENUM_H */
