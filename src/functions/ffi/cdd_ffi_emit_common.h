/**
 * @file cdd_ffi_emit_common.h
 * @brief Shared FFI type-mapping, trivia emission, and file-scaffolding
 * utilities across language emitters.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_FFI_EMIT_COMMON_H
/** @brief Header guard for cdd_ffi_emit_common.h */
#define C_CDD_FFI_EMIT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "ffi/cdd_ffi_ir.h"
#include <stddef.h>
#include <stdio.h>
/* clang-format on */

/**
 * @brief Table of target-language type names for C FFI primitives and pointers.
 */
typedef struct cdd_ffi_type_map_table_t {
  const char *void_type;         /**< Void type representation */
  const char *bool_type;         /**< Boolean type representation */
  const char *int8_type;         /**< 8-bit signed integer representation */
  const char *uint8_type;        /**< 8-bit unsigned integer representation */
  const char *int16_type;        /**< 16-bit signed integer representation */
  const char *uint16_type;       /**< 16-bit unsigned integer representation */
  const char *int32_type;        /**< 32-bit signed integer representation */
  const char *uint32_type;       /**< 32-bit unsigned integer representation */
  const char *int64_type;        /**< 64-bit signed integer representation */
  const char *uint64_type;       /**< 64-bit unsigned integer representation */
  const char *float32_type;      /**< 32-bit float representation */
  const char *float64_type;      /**< 64-bit double representation */
  const char *char_ptr_type;     /**< String / char pointer representation */
  const char *void_ptr_type;     /**< Generic / opaque pointer representation */
  const char *function_ptr_type; /**< Optional function pointer representation
                                  */
  const char *enum_type;         /**< Default enum representation */
  int use_ref_name_for_struct; /**< Non-zero to use type.ref_name for structs */
  int use_ref_name_for_enum;   /**< Non-zero to use type.ref_name for enums */
  int use_ref_name_for_typedef; /**< Non-zero to use type.ref_name for typedefs
                                 */
} cdd_ffi_type_map_table_t;

/**
 * @brief Emit a linked list of FFI trivia tokens to an open file stream.
 *
 * @param[in,out] f Output file stream.
 * @param[in] trivia Head of trivia linked list (may be NULL).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if f is
 * NULL.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_emit_trivia(FILE *f,
                                               const cdd_ffi_trivia_t *trivia);

/**
 * @brief Map a `cdd_ffi_type_t` to a target-language type string using a lookup
 * table.
 *
 * @param[in] table Language type mapping table.
 * @param[in] type FFI IR type descriptor.
 * @param[out] out_type Output pointer receiving the resolved type string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_ffi_map_type_from_table(const cdd_ffi_type_map_table_t *table,
                            cdd_ffi_type_t type, const char **out_type);

/**
 * @brief Open an output file inside `output_dir` for FFI code emission.
 *
 * @param[in] output_dir Target directory path.
 * @param[in] filename Relative filename inside `output_dir`.
 * @param[out] out_fp Pointer to receive the opened FILE stream.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO /
 * CDD_C_ERROR_INVALID_ARGUMENT on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_open_output_file(const char *output_dir,
                                                    const char *filename,
                                                    FILE **out_fp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_FFI_EMIT_COMMON_H */
