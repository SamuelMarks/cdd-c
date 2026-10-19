/**
 * @file cdd_ffi_emit_r.c
 * @brief FFI binding emitter for R.
 */
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_r.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding R `.C()` coercion type
 * string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved R type string.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t get_r_type(cdd_ffi_type_t type, const char **out_type) {
  static const cdd_ffi_type_map_table_t k_r_types = {
      "void",    "logical", "integer",   "integer", "integer",
      "integer", "integer", "integer",   "integer", "integer",
      "double",  "double",  "character", "raw",     NULL,
      "integer", 1,         0,           1};
  return cdd_ffi_map_type_from_table(&k_r_types, type, out_type);
}

/**
 * @brief Emits an `.R` script containing `dyn.load` and `.C()` function
 * wrappers for the given FFI IR.
 *
 * @param[in] ir The extracted and topologically sorted FFI IR.
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t emit_r_file(cdd_ffi_ir_t *ir,
                                 const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.R", config->output_dir,
               lib_name);
  {
    int err = 0;
#ifdef CDD_BUILD_TESTS
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      err = 1;
    } else
#endif
    {
      err = fopen_s(&f, filepath, "w");
    }
    if (err != 0) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.R", config->output_dir,
               lib_name);
#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      f = NULL;
    } else
#endif
    {
#if defined(_MSC_VER)
      if (fopen_s(&f, filepath, "w") != 0)
        f = NULL;
#else
    f = fopen(filepath, "w");
#endif
    }
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "# Auto-generated R bindings for %s\n\n", lib_name);

  fprintf(f, "if (!is.loaded(\"%s\")) {\n", lib_name);
  fprintf(f, "  dyn.load(paste0(\"%s\", .Platform$dynlib.ext))\n", lib_name);
  fprintf(f, "}\n\n");

  /* Enums as global lists or variables */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "# %s\n", node->doc);
      fprintf(f, "%s <- list(\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "  %s = %s", var->name, var->value);
        } else {
          fprintf(f, "  %s = %lu", var->name, (unsigned long)j);
        }
        if (j < node->variants_count - 1)
          fprintf(f, ",");
        fprintf(f, "\n");
      }
      fprintf(f, ")\n\n");
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "# %s\n", node->doc);
      fprintf(f, "%s <- function(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        /* avoid R keywords */
        if (strcmp(arg_name, "function") == 0)
          arg_name = "func";
        if (strcmp(arg_name, "in") == 0)
          arg_name = "in_arg";
        fprintf(f, "%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ") {\n");

      fprintf(f, "  .C(\"%s\"", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        const char *rt = NULL;
        if (strcmp(arg_name, "function") == 0)
          arg_name = "func";
        if (strcmp(arg_name, "in") == 0)
          arg_name = "in_arg";
        get_r_type(node->fields[j].type, &rt);
        fprintf(f, ", \n    as.%s(%s)", rt, arg_name);
      }
      fprintf(f, "\n  )\n");
      fprintf(f, "}\n\n");
    }
  }

  fclose(f);
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits R FFI bindings for the given FFI IR.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t cdd_ffi_emit_r(cdd_ffi_ir_t *ir,
                             const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_r_file(ir, config);
}
