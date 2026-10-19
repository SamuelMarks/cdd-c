/**
 * @file cdd_ffi_emit_ada.c
 * @brief FFI binding emitter for Ada.
 */
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
#ifdef CDD_BUILD_TESTS
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_ada.h"

#include "../../../include/c_cdd/safe_crt.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../cdd_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding Ada `Interfaces.C` type name.
 *
 * @param[in] t FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved Ada type string.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t map_ada_type(cdd_ffi_type_t *t, const char **out_type) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      *out_type = "Interfaces.C.Strings.chars_ptr";
      return CDD_C_SUCCESS;
    }
    *out_type = "System.Address";
    return CDD_C_SUCCESS;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = "System.Address";
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = "Boolean";
    break;
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
    *out_type = "Interfaces.C.char";
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = "Interfaces.C.short";
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = "Interfaces.C.unsigned_short";
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = "Interfaces.C.int";
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = "Interfaces.C.unsigned";
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = "Interfaces.C.long";
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = "Interfaces.C.unsigned_long";
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = "Interfaces.C.C_float";
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = "Interfaces.C.double";
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      *out_type = t->ref_name;
    } else {
      *out_type = "System.Address";
    }
    break;
  default:
    *out_type = "System.Address";
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Ada `Interfaces.C` package specifications and GPR project files
 * for the given FFI IR.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t cdd_ffi_emit_ada(cdd_ffi_ir_t *ir,
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
#if defined(_MSC_VER)
  if (fopen_s(&f, filepath, "w") != 0)
    f = NULL;
#else
  f = fopen(filepath, "w");
#endif
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(gpr_filepath, sizeof(gpr_filepath), "%s/%s.gpr",
               config->output_dir, lib_name);
#if defined(_MSC_VER)
  if (fopen_s(&gpr_f, gpr_filepath, "w") != 0)
    gpr_f = NULL;
#else
  gpr_f = fopen(gpr_filepath, "w");
#endif
#ifdef CDD_BUILD_TESTS

  if (g_fail_io_after == 555) {
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
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "   type %s is record\n", node->name);
      if (node->fields_count == 0) {
        fprintf(f, "      null;\n");
      }
      for (j = 0; j < node->fields_count; j++) {
        const char *atype = NULL;
        map_ada_type(&node->fields[j].type, &atype);
        fprintf(f, "      %s : %s;\n", node->fields[j].name, atype);
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
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "   type %s is (\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        fprintf(f, "      %s%s", var->name,
                j + 1 < node->variants_count ? ",\n" : "\n");
      }
      fprintf(f, "   );\n");
      fprintf(f, "   pragma Convention (C, %s);\n\n", node->name);
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      is_void = (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
                 node->return_or_base_type.pointer_depth == 0);

      if (is_void) {
        fprintf(f, "   procedure %s (\n", node->name);
      } else {
        fprintf(f, "   function %s (\n", node->name);
      }

      for (j = 0; j < node->fields_count; j++) {
        const char *atype = NULL;
        map_ada_type(&node->fields[j].type, &atype);
        fprintf(f, "      %s : %s%s\n", node->fields[j].name, atype,
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
        const char *ret_atype = NULL;
        map_ada_type(&node->return_or_base_type, &ret_atype);
        if (node->fields_count == 0) {
          fprintf(f, "   return %s;\n", ret_atype);
          fprintf(f, "   pragma Import (C, %s, \"%s\");\n\n", node->name,
                  node->name);
        } else {
          fprintf(f, "   ) return %s;\n", ret_atype);
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
