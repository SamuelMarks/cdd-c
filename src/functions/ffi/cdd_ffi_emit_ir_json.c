/**
 * @file cdd_ffi_emit_ir_json.c
 * @brief FFI binding emitter for Ir Json.
 */
/* clang-format off */
#include "../../include/ffi/cdd_ffi_ir.h"
#include "../../include/cdd_c_error.h"
#include "../../include/c_cdd/memory.h"
#include "../../include/c_cdd/safe_crt.h"
#include "../../src/cdd_api.h"
#include "../parse/fs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_ffi_emit_ir_json.h"
/* clang-format on */

/*
 * @brief Emits the FFI IR as a serialized JSON file (`ir_dump.json`).
 *
 * @param[in] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t
cdd_ffi_emit_ir_json(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  char *json_str = NULL;
  char path[1024];
  cdd_c_error_t rc;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_ffi_ir_to_json(ir, &json_str);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  CDD_SNPRINTF(path, sizeof(path), "%s/ir_dump.json", config->output_dir);

  rc = fs_write_to_file(path, json_str);
  C_CDD_FREE(json_str);
  return rc;
}
