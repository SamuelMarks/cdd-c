/**
 * @file cdd_ffi_emit_ir_json.h
 * @brief FFI binding emitter for Ir Json.
 */
#ifndef CDD_FFI_EMIT_IR_JSON_H
#define CDD_FFI_EMIT_IR_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "../../include/ffi/cdd_ffi_ir.h"
#include "../../src/cdd_api.h"
/* clang-format on */

/**
 * @brief Emits the FFI IR as a serialized JSON file (`ir_dump.json`).
 * @param[in] ir The FFI IR object.
 * @param[in] config Binding generation configuration containing output_dir.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_emit_ir_json(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_EMIT_IR_JSON_H */
