/* clang-format off */
#include "cdd_ffi_emit_ada.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static const char *map_ada_type(cdd_ffi_type_t *t) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      return "Interfaces.C.Strings.chars_ptr";
    }
    return "System.Address";
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    return "System.Address";
  case CDD_FFI_KIND_BOOL:
    return "Boolean";
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
    return "Interfaces.C.char";
  case CDD_FFI_KIND_INT16:
    return "Interfaces.C.short";
  case CDD_FFI_KIND_UINT16:
    return "Interfaces.C.unsigned_short";
  case CDD_FFI_KIND_INT32:
    return "Interfaces.C.int";
  case CDD_FFI_KIND_UINT32:
    return "Interfaces.C.unsigned";
  case CDD_FFI_KIND_INT64:
    return "Interfaces.C.long";
  case CDD_FFI_KIND_UINT64:
    return "Interfaces.C.unsigned_long";
  case CDD_FFI_KIND_FLOAT32:
    return "Interfaces.C.C_float";
  case CDD_FFI_KIND_FLOAT64:
    return "Interfaces.C.double";
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      return t->ref_name;
    }
    return "System.Address";
  default:
    return "System.Address";
  }
}

enum cdd_c_error
cdd_ffi_emit_ada(cdd_ffi_ir_t *ir,
                 const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  FILE *gpr_f = NULL;
  char filepath[1024];
  char gpr_filepath[1024];
  const char *lib_name;
  const char *module_name;
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_enum_variant_t *var;
  int is_void;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  lib_name = config->library_name ? config->library_name : "mylib";
  module_name = config->module_name ? config->module_name : "MyLib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.ads", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(gpr_filepath, sizeof(gpr_filepath), "%s\\%s.gpr",
               config->output_dir, lib_name);
  if (fopen_s(&gpr_f, gpr_filepath, "w") != 0) {
    fclose(f);
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.ads", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(gpr_filepath, sizeof(gpr_filepath), "%s/%s.gpr",
               config->output_dir, lib_name);
  gpr_f = fopen(gpr_filepath, "w");
#ifdef CDD_BUILD_TESTS
  extern volatile int g_fail_io_after;
  if (g_fail_io_after == 555) {
    if (gpr_f)
      fclose(gpr_f);
    gpr_f = NULL;
  }
#endif
  if (!gpr_f) {
    fclose(f);
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "-- Auto-generated Ada lib bindings for %s\n\n", lib_name);
  fprintf(f, "with Interfaces.C;\n");
  fprintf(f, "with Interfaces.C.Strings;\n");
  fprintf(f, "with System;\n\n");
  fprintf(f, "package %s is\n\n", module_name);

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];

    if (node->kind == CDD_FFI_NODE_STRUCT) {
      fprintf(f, "   type %s is record\n", node->name);
      if (node->fields_count == 0) {
        fprintf(f, "      null;\n");
      }
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "      %s : %s;\n", node->fields[j].name,
                map_ada_type(&node->fields[j].type));
      }
      fprintf(f, "   end record;\n");
      fprintf(f, "   pragma Convention (C, %s);\n\n", node->name);
    } else if (node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f,
              "   -- Union %s (Mapped to largest size or unchecked union if "
              "available)\n",
              node->name);
      fprintf(f, "   type %s is record\n", node->name);
      fprintf(f, "      Data : System.Address; -- Placeholder for union\n");
      fprintf(f, "   end record;\n");
      fprintf(f, "   pragma Convention (C, %s);\n\n", node->name);
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "   type %s is (\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        fprintf(f, "      %s%s", var->name,
                j + 1 < node->variants_count ? ",\n" : "\n");
      }
      fprintf(f, "   );\n");
      fprintf(f, "   pragma Convention (C, %s);\n\n", node->name);
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      is_void = (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
                 node->return_or_base_type.pointer_depth == 0);

      if (is_void) {
        fprintf(f, "   procedure %s (\n", node->name);
      } else {
        fprintf(f, "   function %s (\n", node->name);
      }

      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "      %s : %s%s\n", node->fields[j].name,
                map_ada_type(&node->fields[j].type),
                j + 1 < node->fields_count ? ";" : "");
      }

      if (is_void) {
        if (node->fields_count == 0) {
          fprintf(f, "   pragma Import (C, %s, \"%s\");\n\n", node->name,
                  node->name);
        } else {
          fprintf(f, "   );\n");
          fprintf(f, "   pragma Import (C, %s, \"%s\");\n\n", node->name,
                  node->name);
        }
      } else {
        if (node->fields_count == 0) {
          fprintf(f, "   return %s;\n",
                  map_ada_type(&node->return_or_base_type));
          fprintf(f, "   pragma Import (C, %s, \"%s\");\n\n", node->name,
                  node->name);
        } else {
          fprintf(f, "   ) return %s;\n",
                  map_ada_type(&node->return_or_base_type));
          fprintf(f, "   pragma Import (C, %s, \"%s\");\n\n", node->name,
                  node->name);
        }
      }
    }
  }

  fprintf(f, "end %s;\n", module_name);
  fclose(f);

  fprintf(gpr_f, "project %s is\n", lib_name);
  fprintf(gpr_f, "   for Source_Dirs use (\".\");\n");
  fprintf(gpr_f, "   for Object_Dir use \"obj\";\n");
  fprintf(gpr_f, "   for Exec_Dir use \"bin\";\n");
  fprintf(gpr_f, "   for Main use (\"%s_main.adb\");\n", lib_name);
  fprintf(gpr_f, "end %s;\n", lib_name);
  fclose(gpr_f);

  return CDD_C_SUCCESS;
}
