/* clang-format off */
#include "../../include/ffi/cdd_ffi_ir.h"
#include "../../include/cdd_c_error.h"
#include "../../include/c_cdd/safe_crt.h"
#include "../../src/cdd_api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_ffi_emit_ir_json.h"
/* clang-format on */

C_CDD_EXPORT cdd_c_error_t cdd_ffi_emit_ir_json(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char *json_str = NULL;
  cdd_c_error_t rc;
  FILE *fp;
  char path[1024];

  if (!config || !config->output_dir) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_ffi_ir_to_json(ir, &json_str);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  CDD_SNPRINTF(path, sizeof(path), "%s/ir_dump.json", config->output_dir);

#if defined(_MSC_VER)
  if (fopen_s(&fp, path, "w") != 0) {
    free(json_str);
    return CDD_C_ERROR_IO;
  }
#else
  fp = fopen(path, "w");
  if (!fp) {
    free(json_str);
    return CDD_C_ERROR_IO;
  }
#endif

  fprintf(fp, "%s\n", json_str);
  fclose(fp);
  free(json_str);

  return CDD_C_SUCCESS;
}
