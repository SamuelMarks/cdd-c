#ifndef CDD_FFI_EMIT_DSL_H
#define CDD_FFI_EMIT_DSL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "ffi/cdd_ffi_ir.h"
/* #include "cdd_generate_bindings.h" */
/* clang-format on */

/**
 * @brief Emits a Human IR DSL representation from an FFI IR AST.
 *
 * @param ir The FFI IR object.
 * @param config Binding generation configuration containing output paths.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
struct cdd_generate_bindings_config_t;
C_CDD_EXPORT cdd_c_error_t cdd_ffi_emit_dsl(cdd_ffi_ir_t *ir,
                                            const void *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_EMIT_DSL_H */
