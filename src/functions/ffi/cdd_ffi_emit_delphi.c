/* clang-format off */
#include "cdd_ffi_emit_delphi.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static void map_delphi_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      CDD_SNPRINTF(out_type, out_sz, "PAnsiChar");
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, "Pointer");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    CDD_SNPRINTF(out_type, out_sz, "Pointer");
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz, "Boolean");
    break;
  case CDD_FFI_KIND_INT8:
    CDD_SNPRINTF(out_type, out_sz, "ShortInt");
    break;
  case CDD_FFI_KIND_UINT8:
    CDD_SNPRINTF(out_type, out_sz, "Byte");
    break;
  case CDD_FFI_KIND_INT16:
    CDD_SNPRINTF(out_type, out_sz, "SmallInt");
    break;
  case CDD_FFI_KIND_UINT16:
    CDD_SNPRINTF(out_type, out_sz, "Word");
    break;
  case CDD_FFI_KIND_INT32:
    CDD_SNPRINTF(out_type, out_sz, "Integer");
    break;
  case CDD_FFI_KIND_UINT32:
    CDD_SNPRINTF(out_type, out_sz, "Cardinal");
    break;
  case CDD_FFI_KIND_INT64:
    CDD_SNPRINTF(out_type, out_sz, "Int64");
    break;
  case CDD_FFI_KIND_UINT64:
    CDD_SNPRINTF(out_type, out_sz, "UInt64");
    break;
  case CDD_FFI_KIND_FLOAT32:
    CDD_SNPRINTF(out_type, out_sz, "Single");
    break;
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, "Double");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "%s", t->ref_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "Pointer");
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, "Pointer");
    break;
  }
}

int cdd_ffi_emit_delphi(cdd_ffi_ir_t *ir,
                        const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name;
  const char *module_name;
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_enum_variant_t *var;
  char type_str[256];
  char ret_type_str[256];
  int is_void;

  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  lib_name = config->library_name ? config->library_name : "mylib";
  module_name = config->module_name ? config->module_name : "MyLibBindings";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.pas", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.pas", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "// Auto-generated Delphi / Object Pascal bindings for %s\n",
          lib_name);
  fprintf(f, "unit %s;\n\n", module_name);
  fprintf(f, "interface\n\n");
  fprintf(f, "const\n");
  fprintf(f,
          "  LIB_NAME = '%s.dll'; {$IFDEF LINUX} LIB_NAME = 'lib%s.so'; "
          "{$ENDIF} {$IFDEF MACOS} LIB_NAME = 'lib%s.dylib'; {$ENDIF}\n\n",
          lib_name, lib_name, lib_name);

  fprintf(f, "type\n");

  /* First pass: Types, Enums, Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      /* Need to forward declare pointer type if needed, but for now simple
       * records */
      fprintf(f, "  P%s = ^%s;\n", node->name, node->name);
      fprintf(f, "  %s = record\n", node->name);
      if (node->kind == CDD_FFI_NODE_UNION) {
        fprintf(f, "    case Integer of\n");
        fprintf(f, "      0: (Data: Pointer); // Union mapped as opaque "
                   "variant to be safe\n");
        /* Real variant records would require mapping C unions closely */
      } else {
        for (j = 0; j < node->fields_count; j++) {
          map_delphi_type(&node->fields[j].type, type_str, sizeof(type_str));
          fprintf(f, "    %s: %s;\n", node->fields[j].name, type_str);
        }
      }
      fprintf(f, "  end;\n\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "  %s = (\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        fprintf(f, "    %s%s", var->name,
                j + 1 < node->variants_count ? ",\n" : "\n");
      }
      fprintf(f, "  );\n\n");
    } else if (node->kind == CDD_FFI_NODE_TYPEDEF) {
      map_delphi_type(&node->return_or_base_type, type_str, sizeof(type_str));
      fprintf(f, "  %s = %s;\n\n", node->name, type_str);
    }
  }

  /* Second pass: Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      is_void = (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
                 node->return_or_base_type.pointer_depth == 0);
      if (is_void) {
        fprintf(f, "procedure %s(", node->name);
      } else {
        fprintf(f, "function %s(", node->name);
      }
      for (j = 0; j < node->fields_count; j++) {
        map_delphi_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(f, "%s%s: %s", j > 0 ? "; " : "", node->fields[j].name,
                type_str);
      }
      if (is_void) {
        fprintf(f, "); cdecl; external LIB_NAME;\n");
      } else {
        map_delphi_type(&node->return_or_base_type, ret_type_str,
                        sizeof(ret_type_str));
        fprintf(f, "): %s; cdecl; external LIB_NAME;\n", ret_type_str);
      }
    }
  }

  fprintf(f, "\nimplementation\n\n");
  fprintf(f, "end.\n");

  fclose(f);
  return 0;
}
