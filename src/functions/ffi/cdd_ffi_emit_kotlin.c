/* clang-format off */
#include "cdd_ffi_emit_kotlin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static int emit_kotlin_def(cdd_ffi_ir_t *ir,
                           const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

  (void)ir; /* The def file just points to the header for cinterop to do the
               heavy lifting */

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.def", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.def", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "# Auto-generated Kotlin cinterop def file for %s\n\n", lib_name);

  if (config->input) {
    fprintf(f, "headers = %s\n", config->input);
  }

  fprintf(f, "headerFilter = *\n");
  fprintf(f, "package = %s\n\n", lib_name);

  /* Additional compiler/linker opts could be added here */
  fprintf(f, "compilerOpts = -I.\n");

  fclose(f);
  return 0;
}

int cdd_ffi_emit_kotlin(cdd_ffi_ir_t *ir,
                        const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  return emit_kotlin_def(ir, config);
}
