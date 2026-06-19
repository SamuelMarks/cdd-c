#ifndef CDD_FFI_EMIT_WEBASSEMBLY_H
#define CDD_FFI_EMIT_WEBASSEMBLY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
/* clang-format on */

/**
 * @brief Emits WebAssembly Emscripten IDL & JavaScript glue for the given FFI
 * IR.
 * @param ir The extracted and topologically sorted FFI IR.
 * @param config The generation config.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT int
cdd_ffi_emit_webassembly(cdd_ffi_ir_t *ir,
                         const cdd_generate_bindings_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_EMIT_WEBASSEMBLY_H */
