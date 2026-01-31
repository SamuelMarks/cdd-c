/**
 * @file codegen_struct.h
 * @brief Struct Lifecycle generation module.
 *
 * Provides functionality to generate C "Data Class" utilities:
 * - `_cleanup`: Recursive memory freeing.
 * - `_deepcopy`: Recursive independent copying.
 * - `_eq`: Deep equality comparison.
 * - `_default`: Initialization with default values.
 * - `_debug` / `_display`: Inspection utilities.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_STRUCT_H
#define C_CDD_CODEGEN_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdio.h>

#include "c_cdd_export.h"
#include "c_cdd_stdbool.h"

/**
 * @brief Represents a single field within a struct.
 * Used to drive generation logic based on type traits.
 */
struct StructField {
  char name[64]; /**< Field identifier */
  char type[64]; /**< C or Logical Field type (e.g. "string", "integer",
                    "object") */
  char ref[64];  /**< Reference type name (for objects/enums) or item type (for
                    arrays) */
  char default_val[64]; /**< Default value literal (e.g. "5", "\"foo\"") or
                           empty */

  /* Validation Constraints */
  int has_min;
  double min_val;
  int exclusive_min;
  int has_max;
  double max_val;
  int exclusive_max;
  int has_min_len;
  size_t min_len;
  int has_max_len;
  size_t max_len;
  char pattern[256]; /**< Regex pattern */
};

/**
 * @brief Container for fields of a struct.
 */
struct StructFields {
  size_t capacity;            /**< allocated capacity */
  size_t size;                /**< number of fields */
  struct StructField *fields; /**< dynamic array of fields */
};

/**
 * @brief Configuration options for struct code generation.
 */
struct CodegenStructConfig {
  /**
   * @brief Macro name to guard generated functions (e.g. "DATA_UTILS").
   * If NULL, no #ifdef/#endif block is generated.
   */
  const char *guard_macro;
};

/**
 * @brief Initialize a StructFields container.
 * @param[out] sf Pointer to container.
 * @return 0 on success, EINVAL if NULL, ENOMEM if alloc fails.
 */
extern C_CDD_EXPORT int struct_fields_init(struct StructFields *sf);

/**
 * @brief Free memory within a StructFields container.
 * @param[in] sf Pointer to container.
 */
extern C_CDD_EXPORT void struct_fields_free(struct StructFields *sf);

/**
 * @brief Add a field to the container.
 *
 * @param[in,out] sf Pointer to the container.
 * @param[in] name Field name.
 * @param[in] type Field type ("integer", "string", "object", etc.)
 * @param[in] ref Reference type (nullable).
 * @param[in] default_val Default value literal (nullable).
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int struct_fields_add(struct StructFields *sf,
                                          const char *name, const char *type,
                                          const char *ref,
                                          const char *default_val);

/**
 * @brief Search for a field by name.
 *
 * @param[in] sf Pointer to the container.
 * @param[in] name Field name to find.
 * @return Pointer to the field if found, NULL otherwise.
 */
extern C_CDD_EXPORT struct StructField *
struct_fields_get(const struct StructFields *sf, const char *name);

/* --- Generator Functions --- */

extern C_CDD_EXPORT int
write_struct_cleanup_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config);

extern C_CDD_EXPORT int
write_struct_deepcopy_func(FILE *fp, const char *struct_name,
                           const struct StructFields *sf,
                           const struct CodegenStructConfig *config);

extern C_CDD_EXPORT int
write_struct_eq_func(FILE *fp, const char *struct_name,
                     const struct StructFields *sf,
                     const struct CodegenStructConfig *config);

extern C_CDD_EXPORT int
write_struct_default_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config);

extern C_CDD_EXPORT int
write_struct_debug_func(FILE *fp, const char *struct_name,
                        const struct StructFields *sf,
                        const struct CodegenStructConfig *config);

extern C_CDD_EXPORT int
write_struct_display_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config);

extern C_CDD_EXPORT const char *get_type_from_ref(const char *ref);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_STRUCT_H */
