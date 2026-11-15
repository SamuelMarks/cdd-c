/**
 * @file cdd_ffi_emit_cobol.h
 * @brief Production COBOL 2002+ / GnuCOBOL FFI binding emitter (`.cpy`
 * Copybook, `.cbl` wrapper subprograms, strict C89 ABI shim, and `.cbl` tests).
 */
#ifndef CDD_FFI_EMIT_COBOL_H
/** @brief Header guard for cdd_ffi_emit_cobol.h */
#define CDD_FFI_EMIT_COBOL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../cdd_api.h"
#include "c_cdd/no_discard.h"
#include "cdd_c_error.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Normalizes a C identifier (`snake_case` or `camelCase`) into a valid
 * uppercase hyphenated COBOL user-defined word (`<= 30` characters), escaping
 * leading digits and COBOL reserved words with `CDD-` (or `prefix` if supplied)
 * and disambiguating truncated identifiers deterministically.
 *
 * @param[in] c_ident Source C identifier (may be NULL or empty).
 * @param[in] prefix Optional explicit prefix (e.g. `"LS-"` or `"WS-"`), or
 * NULL.
 * @param[in] disambig_idx Disambiguation index (`0` for primary name, `>= 1` on
 * collision).
 * @param[out] out_buf Destination buffer receiving the sanitized COBOL word.
 * @param[in] out_cap Capacity of `out_buf` in bytes (must be >= 8).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_cobol_sanitize_word(const char *c_ident, const char *prefix,
                        size_t disambig_idx, char *out_buf, size_t out_cap);

/**
 * @brief Emits COBOL FFI bindings (`<lib>.cpy` Copybook, `<module>.cbl` wrapper
 * subprograms, `<lib>_cobol_shim.h/.c` C89 ABI shim, and optional
 * `<module>_test.cbl`) for the given FFI IR.
 *
 * @param[in,out] ir Extracted and topologically sorted FFI IR.
 * @param[in] config Bindings generation configuration.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * CDD_C_ERROR_MEMORY on OOM/truncation, or CDD_C_ERROR_IO on file I/O failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_emit_cobol(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_EMIT_COBOL_H */
