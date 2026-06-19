/* clang-format off */
#include "cdd_ffi_emit_nim.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static const char *map_nim_type(cdd_ffi_type_t *t) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      return "cstring";
    }
    if (t->kind == CDD_FFI_KIND_VOID) {
      return "pointer";
    }
    if (t->kind == CDD_FFI_KIND_STRUCT_REF ||
        t->kind == CDD_FFI_KIND_TYPEDEF_REF ||
        t->kind == CDD_FFI_KIND_ENUM_REF) {
      if (t->ref_name) {
        /* Return pointer type for known structs, but string manipulation is
           hard here, so we just return ptr type. A better way would be
           returning "ptr structName" but for simplicity, we map to pointer. */
        return "pointer";
      }
    }
    return "pointer";
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "bool";
  case CDD_FFI_KIND_INT8:
    return "int8";
  case CDD_FFI_KIND_UINT8:
    return "uint8";
  case CDD_FFI_KIND_INT16:
    return "int16";
  case CDD_FFI_KIND_UINT16:
    return "uint16";
  case CDD_FFI_KIND_INT32:
    return "cint";
  case CDD_FFI_KIND_UINT32:
    return "cuint";
  case CDD_FFI_KIND_INT64:
    return "clonglong";
  case CDD_FFI_KIND_UINT64:
    return "culonglong";
  case CDD_FFI_KIND_FLOAT32:
    return "cfloat";
  case CDD_FFI_KIND_FLOAT64:
    return "cdouble";
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      return t->ref_name;
    }
    return "pointer";
  default:
    return "pointer";
  }
}

int cdd_ffi_emit_nim(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name = config->library_name ? config->library_name : "mylib";
  const char *module_name =
      config->module_name ? config->module_name : "bindings";
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return 1;
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.nim", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.nim", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "# Auto-generated Nim bindings for %s\n\n", lib_name);
  fprintf(f, "{.passL: \"-l%s\".}\n", lib_name);
  /* Assuming the original header name matches the module_name or is accessible
   */
  fprintf(f, "## {.header: \"%s.h\".}\n\n", module_name);

  /* Forward declarations for structs if any cyclic dependency, but Nim allows
   * types block */
  fprintf(f, "type\n");

  /* Structures and Unions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "  %s* {.importc: \"%s\", bycopy.} = object\n", node->name,
              node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "    %s*: %s\n", node->fields[j].name,
                map_nim_type(&node->fields[j].type));
      }
      fprintf(f, "\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "  %s* {.size: sizeof(cint).} = enum\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s,\n", var->name, var->value);
        } else {
          fprintf(f, "    %s,\n", var->name);
        }
      }
      fprintf(f, "\n");
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "proc %s*(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s%s: %s", j > 0 ? ", " : "", node->fields[j].name,
                map_nim_type(&node->fields[j].type));
      }
      fprintf(f, "): %s {.importc: \"%s\", cdecl.}\n\n",
              map_nim_type(&node->return_or_base_type), node->name);
    }
  }

  fclose(f);

  /* Generate .nimble file */
#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.nimble", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") == 0) {
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.nimble", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (f) {
#endif
    fprintf(f, "version       = \"0.1.0\"\n");
    fprintf(f, "author        = \"Auto-generated\"\n");
    fprintf(f, "description   = \"Nim bindings for %s\"\n", lib_name);
    fprintf(f, "license       = \"MIT\"\n");
    fprintf(f, "srcDir        = \".\"\n");
    fprintf(f, "requires \"nim >= 1.6.0\"\n");
    fclose(f);
  }

  return 0;
}
