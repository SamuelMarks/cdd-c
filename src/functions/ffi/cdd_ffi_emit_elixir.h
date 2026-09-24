#ifndef CDD_FFI_EMIT_ELIXIR_H
#define CDD_FFI_EMIT_ELIXIR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../cdd_api.h"
#include "cdd_c_error.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
/* clang-format on */

/**
 * @brief Emits Elixir NIF C89 boilerplate for the given FFI IR.
 * @param ir The extracted and topologically sorted FFI IR.
 * @param config The generation config.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_emit_elixir(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config);

#ifdef CDD_BUILD_TESTS
/**
 * @brief Exposes internal edge cases for code coverage.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t test_cdd_ffi_emit_elixir_internals(void);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_EMIT_ELIXIR_H */
