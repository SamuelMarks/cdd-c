/**
 * @file cdd_ffi_emit_ocaml.h
 * @brief Production OCaml (`Ctypes`, `.mli`, `dune`, and C89 stubs) FFI binding
 * emitter.
 */
#ifndef CDD_FFI_EMIT_OCAML_H
/** @brief Header guard for cdd_ffi_emit_ocaml.h */
#define CDD_FFI_EMIT_OCAML_H

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
 * @brief Sanitizes an arbitrary C identifier into a valid lowercase OCaml
 * value/type identifier (`[a-z_][a-zA-Z0-9_']*`), escaping all OCaml reserved
 * keywords by appending `_`.
 *
 * @param[in] c_ident Source C identifier (may be NULL or empty).
 * @param[out] out_buf Destination buffer for sanitized identifier.
 * @param[in] out_cap Capacity of `out_buf` in bytes (must be >= 4).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_buf` is NULL or `out_cap < 4`.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ocaml_sanitize_lid(const char *c_ident, char *out_buf, size_t out_cap);

/**
 * @brief Sanitizes an arbitrary C identifier into a valid capitalized OCaml
 * module or constructor identifier (`[A-Z][a-zA-Z0-9_']*`).
 *
 * @param[in] c_ident Source C identifier (may be NULL or empty).
 * @param[out] out_buf Destination buffer for capitalized identifier.
 * @param[in] out_cap Capacity of `out_buf` in bytes (must be >= 4).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_buf` is NULL or `out_cap < 4`.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ocaml_sanitize_uid(const char *c_ident, char *out_buf, size_t out_cap);

/**
 * @brief Emits OCaml (`Ctypes` `.ml`, `.mli`, `dune`, `dune-project`, C89
 * `_stubs.c`, and optional `_test.ml`) bindings for the given FFI IR.
 *
 * @param[in,out] ir Extracted and topologically sorted FFI IR.
 * @param[in] config Bindings generation configuration.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * CDD_C_ERROR_MEMORY on OOM/truncation, or CDD_C_ERROR_IO on file I/O failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_emit_ocaml(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_EMIT_OCAML_H */
