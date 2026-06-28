#ifndef CDD_FFI_EMIT_RUBY_H
#define CDD_FFI_EMIT_RUBY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../cdd_api.h"
#include "cdd_c_error.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
/* clang-format on */

/**
 * @brief Emits Ruby (Fiddle) bindings for the given FFI IR.
 * @param ir The extracted and topologically sorted FFI IR.
 * @param config The generation config.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT enum cdd_c_error
cdd_ffi_emit_ruby(cdd_ffi_ir_t *ir,
                  const cdd_generate_bindings_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_EMIT_RUBY_H */
