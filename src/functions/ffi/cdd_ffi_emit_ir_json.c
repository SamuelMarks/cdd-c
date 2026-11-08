/**
 * @file cdd_ffi_emit_ir_json.c
 * @brief FFI binding emitter for Ir Json.
 */
/* clang-format off */
#include "../../include/ffi/cdd_ffi_ir.h"
#include "../../include/cdd_c_error.h"
#include "../../include/c_cdd/json_dom.h"
#include "../../include/c_cdd/memory.h"
#include "../../include/c_cdd/safe_crt.h"
#include "../../src/cdd_api.h"
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
  JSON_Value *root_val = NULL;
  cdd_c_error_t rc;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_ffi_ir_to_json(ir, &json_str);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  rc = cdd_json_parse_string(json_str, &root_val);
  C_CDD_FREE(json_str);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  CDD_SNPRINTF(path, sizeof(path), "%s/ir_dump.json", config->output_dir);

  rc = cdd_json_serialize_to_file_pretty(root_val, path);
  cdd_json_value_free(root_val);
  return rc;
}
