/* clang-format off */
#include "cdd_ffi_emit_crystal.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static const char *map_crystal_type(cdd_ffi_type_t *t) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      if (t->is_const) {
        return "Pointer(LibC::Char)"; /* or Pointer(UInt8) */
      }
    }
    return "Pointer(Void)";
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    return "Void";
  case CDD_FFI_KIND_BOOL:
    return "Bool";
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
    return "LibC::Char"; /* or Int8/UInt8 */
  case CDD_FFI_KIND_INT16:
    return "LibC::Short";
  case CDD_FFI_KIND_UINT16:
    return "LibC::UShort";
  case CDD_FFI_KIND_INT32:
    return "LibC::Int";
  case CDD_FFI_KIND_UINT32:
    return "LibC::UInt";
  case CDD_FFI_KIND_INT64:
    return "LibC::LongLong";
  case CDD_FFI_KIND_UINT64:
    return "LibC::ULongLong";
  case CDD_FFI_KIND_FLOAT32:
    return "LibC::Float";
  case CDD_FFI_KIND_FLOAT64:
    return "LibC::Double";
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      return t->ref_name;
    }
    return "Pointer(Void)";
  default:
    return "Pointer(Void)";
  }
}

enum cdd_c_error
cdd_ffi_emit_crystal(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name = config->library_name ? config->library_name : "mylib";
  const char *module_name = config->module_name ? config->module_name : "MyLib";
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.cr", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.cr", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "# Auto-generated Crystal lib bindings for %s\n\n", lib_name);
  fprintf(f, "@[Link(\"%s\")]\n", lib_name);
  fprintf(f, "lib %s\n", module_name);

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    if (node->kind == CDD_FFI_NODE_STRUCT) {
      fprintf(f, "  struct %s\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "    %s : %s\n", node->fields[j].name,
                map_crystal_type(&node->fields[j].type));
      }
      fprintf(f, "  end\n\n");
    } else if (node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "  union %s\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "    %s : %s\n", node->fields[j].name,
                map_crystal_type(&node->fields[j].type));
      }
      fprintf(f, "  end\n\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "  enum %s\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s\n", var->name, var->value);
        } else {
          fprintf(f, "    %s\n", var->name);
        }
      }
      fprintf(f, "  end\n\n");
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "  fun %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s%s : %s", j > 0 ? ", " : "", node->fields[j].name,
                map_crystal_type(&node->fields[j].type));
      }
      fprintf(f, ") : %s\n", map_crystal_type(&node->return_or_base_type));
    }
  }

  fprintf(f, "end\n\n");

  /* Optional idiomatic wrapper generator logic could go here */

  fclose(f);
  return CDD_C_SUCCESS;
}
