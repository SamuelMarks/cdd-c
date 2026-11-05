/**
 * @file struct_internal.h
 * @brief Internal shared helpers for struct emission and runtime reflection.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CLASSES_EMIT_STRUCT_INTERNAL_H
#define C_CDD_CLASSES_EMIT_STRUCT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/reflect.h"
#include "classes/emit/struct.h"
/* clang-format on */

/**
 * @brief Evaluates a bit-field width string (such as `"3"`, `"(8)"`,
 * `"(1 + 2 * 3)"`, `"1 << 2"`, `"0x4"`, or a symbolic constant `"MAX_BITS"`)
 * into a non-zero unsigned integer width with full C operator precedence.
 *
 * @param[in] bw_str Bit-width expression string.
 * @param[out] out_bw Output evaluated bit-width (`0` if empty or `"0"`).
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_struct_eval_bitwidth_str(const char *bw_str,
                                                  unsigned int *out_bw);

/**
 * @brief Determines the integer/float pointee byte width and signedness of a
 * reflected field from `field->c_type` and `field->size`.
 *
 * @param[in] field Reflected field descriptor.
 * @param[out] out_width Effective scalar byte width (1, 2, 4, or 8).
 * @param[out] out_unsigned Non-zero if the type is unsigned.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t
cdd_reflect_resolve_field_scalar_width(const struct cdd_field_info *field,
                                       size_t *out_width, int *out_unsigned);

/**
 * @brief Cleans up truncated elements `[new_len .. old_len - 1]` of a dynamic
 * array buffer before shrinking or freeing.
 *
 * @param[in,out] buf Raw array buffer pointer.
 * @param[in] field Reflected array field descriptor.
 * @param[in] new_len New target element count.
 * @param[in] old_len Previous element count.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t
cdd_reflect_cleanup_shrunk_array_elements(void *buf,
                                          const struct cdd_field_info *field,
                                          size_t new_len, size_t old_len);

/**
 * @brief Cleans up a single field on `obj` (used by
 * `cdd_reflect_struct_cleanup` and JSON deserialization).
 *
 * @param[in,out] obj Mutable struct instance.
 * @param[in] f Field descriptor to clean up.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t
cdd_reflect_cleanup_single_field(void *obj, const struct cdd_field_info *f);

/**
 * @brief Compares a single field `f` across `a` and `b` for deep equality.
 *
 * @param[in] a First struct instance.
 * @param[in] b Second struct instance.
 * @param[in] f Field descriptor.
 * @param[out] out_eq Set to `1` if equal, `0` otherwise.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern cdd_c_error_t cdd_reflect_eq_single_field(const void *a, const void *b,
                                                 const struct cdd_field_info *f,
                                                 int *out_eq);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CLASSES_EMIT_STRUCT_INTERNAL_H */
