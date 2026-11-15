/**
 * @file cdd_ffi_emit_fortran.h
 * @brief Production ISO Fortran 2003/2008 (`ISO_C_BINDING`) FFI binding
 * emitter.
 */
#ifndef CDD_FFI_EMIT_FORTRAN_H
/** @brief Header guard for cdd_ffi_emit_fortran.h */
#define CDD_FFI_EMIT_FORTRAN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../cdd_api.h"
#include "c_cdd/no_discard.h"
#include "cdd_c_error.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include <stddef.h>
#include <stdio.h>
/* clang-format on */

/**
 * @brief Writes a single free-form Fortran code line to `fp`, automatically
 * wrapping lines exceeding 120 columns (well within the ISO Fortran 132-column
 * limit) using trailing `&` and leading `&` continuation markers.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] line Single Fortran source line (with or without trailing `\n`).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * or CDD_C_ERROR_IO on stream write failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_fortran_write_wrapped_line(FILE *fp, const char *line);

/**
 * @brief Sanitizes a C identifier into a valid Fortran identifier (`<= 63`
 * characters), generating deterministic fallback names (`arg_0`, `arg_1`, ...)
 * when `c_ident` is NULL/empty and escaping all Fortran reserved keywords.
 *
 * @param[in] c_ident Source C identifier (may be NULL or empty).
 * @param[in] fallback_idx Fallback parameter/field index when `c_ident` is
 * NULL/empty.
 * @param[out] out_buf Destination buffer receiving the sanitized identifier.
 * @param[in] out_cap Capacity of `out_buf` in bytes (must be >= 8).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_fortran_sanitize_ident(
    const char *c_ident, size_t fallback_idx, char *out_buf, size_t out_cap);

/**
 * @brief Emits Fortran 2003/2008 `ISO_C_BINDING` `.f90` module bindings,
 * `fpm.toml`, and optional `<module>_test.f90` for the given FFI IR.
 *
 * @param[in,out] ir Extracted and topologically sorted FFI IR.
 * @param[in] config Bindings generation configuration.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * CDD_C_ERROR_MEMORY on OOM/truncation, or CDD_C_ERROR_IO on file I/O failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_emit_fortran(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_EMIT_FORTRAN_H */
