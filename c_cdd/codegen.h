/**
 * @file codegen.h
 * @brief Code generation utility functions for C structs and enums.
 * @author Samuel Marks
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include <c_cdd_export.h>

/**
 * @brief Container for enum members extracted from code or schema.
 */
struct EnumMembers {
  size_t capacity; /**< allocated capacity of members array */
  size_t size;     /**< number of members currently stored */
  char **members;  /**< dynamic array of member names (strings) */
};

/**
 * @brief Represents a single field within a struct.
 */
struct StructField {
  char name[64]; /**< Field name */
  char type[64]; /**< Field type (e.g. "string", "integer", "object") */
  char ref[64];  /**< Reference type name if type is "object" or "enum" */
};

/**
 * @brief Container for fields of a struct.
 */
struct StructFields {
  size_t capacity;            /**< allocated capacity of fields array */
  size_t size;                /**< number of fields currently stored */
  struct StructField *fields; /**< dynamic array of fields */
};

/**
 * @brief Write implementation of enum_from_str function to file.
 *
 * @param[in] fp Output file stream.
 * @param[in] enum_name Name of the enum.
 * @param[in] em Container of enum members.
 * @return 0 on success, error code (EINVAL, EIO) on failure.
 */
extern C_CDD_EXPORT int write_enum_from_str_func(FILE *fp,
                                                 const char *enum_name,
                                                 const struct EnumMembers *em);

/**
 * @brief Write implementation of enum_to_str function to file.
 *
 * @param[in] fp Output file stream.
 * @param[in] enum_name Name of the enum.
 * @param[in] em Container of enum members.
 * @return 0 on success, error code (EINVAL, EIO) on failure.
 */
extern C_CDD_EXPORT int write_enum_to_str_func(FILE *fp, const char *enum_name,
                                               const struct EnumMembers *em);

/**
 * @brief Write implementation of struct_cleanup function.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Container of struct fields.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_cleanup_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf);

/**
 * @brief Write implementation of struct_debug function.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Container of struct fields.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int write_struct_debug_func(FILE *fp,
                                                const char *struct_name,
                                                const struct StructFields *sf);

/**
 * @brief Write implementation of struct_deepcopy function.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Container of struct fields.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_deepcopy_func(FILE *fp, const char *struct_name,
                           const struct StructFields *sf);

/**
 * @brief Write implementation of struct_default function.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Container of struct fields.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_default_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf);

/**
 * @brief Write implementation of struct_display function.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Container of struct fields.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_display_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf);

/**
 * @brief Write implementation of struct_eq function.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Container of struct fields.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int write_struct_eq_func(FILE *fp, const char *struct_name,
                                             const struct StructFields *sf);

/**
 * @brief Write implementation of struct_from_jsonObject function.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Container of struct fields.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_from_jsonObject_func(FILE *fp, const char *struct_name,
                                  const struct StructFields *sf);

/**
 * @brief Write implementation of struct_from_json function string wrapper.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int write_struct_from_json_func(FILE *fp,
                                                    const char *struct_name);

/**
 * @brief Write implementation of struct_to_json function.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Container of struct fields.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_to_json_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf);

/* helpers */

/**
 * @brief Extract just the type name from a JSON reference string.
 *
 * @param[in] ref The full reference string.
 * @return Pointer to the start of the type name within ref, or ref itself.
 */
extern C_CDD_EXPORT const char *get_type_from_ref(const char *ref);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CODEGEN_H */
