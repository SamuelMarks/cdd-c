/* clang-format off */
#include "cdd_ffi_emit_fsharp.h"

#include "../../cdd_api.h"
#include "../../win_compat_sym.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static void map_fsharp_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      CDD_SNPRINTF(out_type, out_sz, "string");
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, "nativeint");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    CDD_SNPRINTF(out_type, out_sz, "unit");
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz, "bool");
    break;
  case CDD_FFI_KIND_INT8:
    CDD_SNPRINTF(out_type, out_sz, "sbyte");
    break;
  case CDD_FFI_KIND_UINT8:
    CDD_SNPRINTF(out_type, out_sz, "byte");
    break;
  case CDD_FFI_KIND_INT16:
    CDD_SNPRINTF(out_type, out_sz, "int16");
    break;
  case CDD_FFI_KIND_UINT16:
    CDD_SNPRINTF(out_type, out_sz, "uint16");
    break;
  case CDD_FFI_KIND_INT32:
    CDD_SNPRINTF(out_type, out_sz, "int");
    break;
  case CDD_FFI_KIND_UINT32:
    CDD_SNPRINTF(out_type, out_sz, "uint32");
    break;
  case CDD_FFI_KIND_INT64:
    CDD_SNPRINTF(out_type, out_sz, "int64");
    break;
  case CDD_FFI_KIND_UINT64:
    CDD_SNPRINTF(out_type, out_sz, "uint64");
    break;
  case CDD_FFI_KIND_FLOAT32:
    CDD_SNPRINTF(out_type, out_sz, "float32");
    break;
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, "float");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "%s", t->ref_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "nativeint");
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, "nativeint");
    break;
  }
}

int cdd_ffi_emit_fsharp(cdd_ffi_ir_t *ir,
                        const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  FILE *proj_f = NULL;
  char filepath[1024];
  char proj_filepath[1024];
  const char *lib_name;
  const char *module_name;
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_enum_variant_t *var;
  char type_str[256];
  char ret_type_str[256];

  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  lib_name = config->library_name ? config->library_name : "mylib";
  module_name = config->module_name ? config->module_name : "MyLibBindings";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\Bindings.fs",
               config->output_dir);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
  CDD_SNPRINTF(proj_filepath, sizeof(proj_filepath), "%s\\%s.fsproj",
               config->output_dir, module_name);
  if (fopen_s(&proj_f, proj_filepath, "w") != 0) {
    fclose(f);
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/Bindings.fs",
               config->output_dir);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
  CDD_SNPRINTF(proj_filepath, sizeof(proj_filepath), "%s/%s.fsproj",
               config->output_dir, module_name);
  proj_f = fopen(proj_filepath, "w");
  if (!proj_f) {
    fclose(f);
    return 1;
  }
#endif

  fprintf(proj_f, "<Project Sdk=\"Microsoft.NET.Sdk\">\n");
  fprintf(proj_f, "  <PropertyGroup>\n");
  fprintf(proj_f, "    <TargetFramework>net8.0</TargetFramework>\n");
  fprintf(proj_f, "  </PropertyGroup>\n");
  fprintf(proj_f, "  <ItemGroup>\n");
  fprintf(proj_f, "    <Compile Include=\"Bindings.fs\" />\n");
  fprintf(proj_f, "  </ItemGroup>\n");
  fprintf(proj_f, "</Project>\n");
  fclose(proj_f);

  fprintf(f, "// Auto-generated F# bindings for %s\n", lib_name);
  fprintf(f, "namespace %s\n\n", module_name);
  fprintf(f, "open System\n");
  fprintf(f, "open System.Runtime.InteropServices\n\n");

  fprintf(f, "module Types =\n");

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "    [<Struct; StructLayout(LayoutKind.Sequential)>]\n");
      fprintf(f, "    type %s =\n", node->name);
      if (node->kind == CDD_FFI_NODE_UNION) {
        fprintf(f, "        val mutable Data: nativeint\n");
      } else {
        if (node->fields_count == 0) {
          fprintf(f, "        struct end\n");
        }
        for (j = 0; j < node->fields_count; j++) {
          map_fsharp_type(&node->fields[j].type, type_str, sizeof(type_str));
          fprintf(f, "        val mutable %s: %s\n", node->fields[j].name,
                  type_str);
        }
      }
      fprintf(f, "\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "    type %s =\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        if (var->value) {
          fprintf(f, "        | %s = %s\n", var->name, var->value);
        } else {
          fprintf(f, "        | %s = %" CDD_SIZE_T_FMT "\n", var->name, j);
        }
      }
      fprintf(f, "\n");
    } else if (node->kind == CDD_FFI_NODE_TYPEDEF) {
      map_fsharp_type(&node->return_or_base_type, type_str, sizeof(type_str));
      fprintf(f, "    type %s = %s\n\n", node->name, type_str);
    }
  }

  fprintf(f, "module Native =\n");
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f,
              "    [<DllImport(\"%s\", CallingConvention = "
              "CallingConvention.Cdecl)>]\n",
              lib_name);
      fprintf(f, "    extern ");
      map_fsharp_type(&node->return_or_base_type, ret_type_str,
                      sizeof(ret_type_str));
      fprintf(f, "%s %s(", ret_type_str, node->name);
      for (j = 0; j < node->fields_count; j++) {
        map_fsharp_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(f, "%s%s %s", j > 0 ? ", " : "", type_str,
                node->fields[j].name);
      }
      fprintf(f, ")\n\n");
    }
  }

  fclose(f);
  return 0;
}
