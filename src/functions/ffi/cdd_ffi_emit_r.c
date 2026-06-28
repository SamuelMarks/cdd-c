/* clang-format off */
#include "cdd_ffi_emit_r.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_r_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "character";
    }
    return "raw";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "logical";
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
  case CDD_FFI_KIND_INT16:
  case CDD_FFI_KIND_UINT16:
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_UINT32:
  case CDD_FFI_KIND_INT64:
  case CDD_FFI_KIND_UINT64:
  case CDD_FFI_KIND_ENUM_REF:
    return "integer";
  case CDD_FFI_KIND_FLOAT32:
  case CDD_FFI_KIND_FLOAT64:
    return "double";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "raw";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "raw";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "raw";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "raw";
  default:
    return "raw";
  }
}

static enum cdd_c_error
emit_r_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.R", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.R", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
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
      if (node->doc)
        fprintf(f, "# %s\n", node->doc);
      fprintf(f, "%s <- list(\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "  %s = %s", var->name, var->value);
        } else {
          fprintf(f, "  %s = %" CDD_PRIz "", var->name, j);
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
        if (strcmp(arg_name, "function") == 0)
          arg_name = "func";
        if (strcmp(arg_name, "in") == 0)
          arg_name = "in_arg";
        fprintf(f, ", \n    as.%s(%s)", get_r_type(node->fields[j].type),
                arg_name);
      }
      fprintf(f, "\n  )\n");
      fprintf(f, "}\n\n");
    }
  }

  fclose(f);
  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_ffi_emit_r(cdd_ffi_ir_t *ir,
                                const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_r_file(ir, config);
}
