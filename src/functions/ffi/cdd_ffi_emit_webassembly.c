/* clang-format off */
#include "cdd_ffi_emit_webassembly.h"

#include "../../cdd_api.h"
#include "../../win_compat_sym.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

static void map_idl_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      CDD_SNPRINTF(out_type, out_sz, "DOMString");
      return;
    }
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "%s", t->ref_name);
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, "any");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    CDD_SNPRINTF(out_type, out_sz, "void");
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz, "boolean");
    break;
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
    CDD_SNPRINTF(out_type, out_sz, "byte");
    break;
  case CDD_FFI_KIND_INT16:
  case CDD_FFI_KIND_UINT16:
    CDD_SNPRINTF(out_type, out_sz, "short");
    break;
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_UINT32:
  case CDD_FFI_KIND_INT64:
  case CDD_FFI_KIND_UINT64:
    CDD_SNPRINTF(out_type, out_sz, "long"); /* WebIDL maps JS numbers */
    break;
  case CDD_FFI_KIND_FLOAT32:
    CDD_SNPRINTF(out_type, out_sz, "float");
    break;
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, "double");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "%s", t->ref_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "long"); /* Opaque handle */
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, "long");
    break;
  }
}

static void map_ts_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      CDD_SNPRINTF(out_type, out_sz, "string");
      return;
    }
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      CDD_SNPRINTF(out_type, out_sz, "Uint8Array | number");
      return;
    }
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "%s", t->ref_name);
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, "number");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    CDD_SNPRINTF(out_type, out_sz, "void");
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz, "boolean");
    break;
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
  case CDD_FFI_KIND_INT16:
  case CDD_FFI_KIND_UINT16:
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_UINT32:
  case CDD_FFI_KIND_INT64:
  case CDD_FFI_KIND_UINT64:
  case CDD_FFI_KIND_FLOAT32:
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, "number");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "%s", t->ref_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "number"); /* Opaque handle */
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, "number");
    break;
  }
}

enum cdd_c_error
cdd_ffi_emit_webassembly(cdd_ffi_ir_t *ir,
                         const cdd_generate_bindings_config_t *config) {
  FILE *idl_f = NULL;
  FILE *cpp_f = NULL;
  FILE *ts_f = NULL;
  char idl_filepath[1024];
  char cpp_filepath[1024];
  char ts_filepath[1024];
  const char *lib_name;
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_enum_variant_t *var;
  char type_str[256];
  char ret_type_str[256];

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(idl_filepath, sizeof(idl_filepath), "%s\\%s.idl",
               config->output_dir, lib_name);
  if (fopen_s(&idl_f, idl_filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(cpp_filepath, sizeof(cpp_filepath), "%s\\%s_glue.cpp",
               config->output_dir, lib_name);
  if (fopen_s(&cpp_f, cpp_filepath, "w") != 0) {
    fclose(idl_f);
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(ts_filepath, sizeof(ts_filepath), "%s\\%s.d.ts",
               config->output_dir, lib_name);
  if (fopen_s(&ts_f, ts_filepath, "w") != 0) {
    fclose(idl_f);
    fclose(cpp_f);
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(idl_filepath, sizeof(idl_filepath), "%s/%s.idl",
               config->output_dir, lib_name);
  idl_f = fopen(idl_filepath, "w");
  if (!idl_f) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(cpp_filepath, sizeof(cpp_filepath), "%s/%s_glue.cpp",
               config->output_dir, lib_name);
  cpp_f = fopen(cpp_filepath, "w");
  if (!cpp_f) {
    /* LCOV_EXCL_START */
    fclose(idl_f);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }
  CDD_SNPRINTF(ts_filepath, sizeof(ts_filepath), "%s/%s.d.ts",
               config->output_dir, lib_name);
  ts_f = fopen(ts_filepath, "w");
  if (!ts_f) {
    /* LCOV_EXCL_START */
    fclose(idl_f);
    fclose(cpp_f);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }
#endif

  /* WebIDL File */
  fprintf(idl_f, "/* Auto-generated Emscripten WebIDL for %s */\n\n", lib_name);
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      fprintf(idl_f, "interface %s {\n", node->name);
      fprintf(idl_f, "  void %s();\n", node->name); /* Constructor */
      for (j = 0; j < (unsigned long)node->fields_count; j++) {
        map_idl_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(idl_f, "  attribute %s %s;\n", type_str, node->fields[j].name);
      }
      fprintf(idl_f, "};\n\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(idl_f, "enum %s {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        fprintf(idl_f, "  \"%s\"%s\n", var->name,
                (unsigned long)(j + 1) < node->variants_count ? "," : "");
      }
      fprintf(idl_f, "};\n\n");
    }
  }
  /* WebIDL functions go on a global or namespace. Emscripten uses a flat
   * namespace often, or an interface. */
  fprintf(idl_f, "interface %s_Module {\n", lib_name);
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      map_idl_type(&node->return_or_base_type, ret_type_str,
                   sizeof(ret_type_str));
      fprintf(idl_f, "  %s %s(", ret_type_str, node->name);
      for (j = 0; j < (unsigned long)node->fields_count; j++) {
        map_idl_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(idl_f, "%s %s%s", type_str, node->fields[j].name,
                (unsigned long)(j + 1) < (unsigned long)node->fields_count
                    ? ", "
                    : "");
      }
      fprintf(idl_f, ");\n");
    }
  }
  fprintf(idl_f, "};\n");
  fclose(idl_f);

  /* C++ Glue (EMSCRIPTEN_KEEPALIVE alternative to WebIDL binder) */
  fprintf(cpp_f, "/* Auto-generated Emscripten C++ Glue for %s */\n", lib_name);
  fprintf(cpp_f, "#include <emscripten.h>\n");
  fprintf(cpp_f, "#include \"%s.h\"\n\n", lib_name);
  fprintf(cpp_f, "extern \"C\" {\n\n");

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(cpp_f, "EMSCRIPTEN_KEEPALIVE\n");
      /* Standard C signature */
      if (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
          node->return_or_base_type.pointer_depth == 0) {
        fprintf(cpp_f, "void ");
      } else {
        fprintf(cpp_f,
                "/* returning type */ void* "); /* Oversimplified for stub */
      }
      fprintf(cpp_f, "wasm_%s(", node->name);
      for (j = 0; j < (unsigned long)node->fields_count; j++) {
        /* cppcheck-suppress invalidPrintfArgType_uint */
        fprintf(cpp_f, "void* arg%lu%s", (unsigned long)j,
                (unsigned long)(j + 1) < (unsigned long)node->fields_count
                    ? ", "
                    : "");
      }
      fprintf(cpp_f, ") {\n");
      fprintf(cpp_f, "    /* Call %s(...) */\n", node->name);
      fprintf(cpp_f, "    return;\n");
      fprintf(cpp_f, "}\n\n");
    }
  }
  fprintf(cpp_f, "} /* extern \"C\" */\n");
  fclose(cpp_f);

  /* TypeScript .d.ts */
  fprintf(ts_f, "// Auto-generated TypeScript definitions for %s\n\n",
          lib_name);
  fprintf(ts_f, "export interface %sModule extends EmscriptenModule {\n",
          lib_name);

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      map_ts_type(&node->return_or_base_type, ret_type_str,
                  sizeof(ret_type_str));
      fprintf(ts_f, "  _%s(", node->name);
      for (j = 0; j < (unsigned long)node->fields_count; j++) {
        map_ts_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(ts_f, "%s: %s%s", node->fields[j].name, type_str,
                (unsigned long)(j + 1) < (unsigned long)node->fields_count
                    ? ", "
                    : "");
      }
      fprintf(ts_f, "): %s;\n", ret_type_str);
    } else if (node->kind == CDD_FFI_NODE_STRUCT ||
               node->kind == CDD_FFI_NODE_UNION) {
      fprintf(ts_f, "  %s: { new(): %s };\n", node->name, node->name);
    }
  }

  fprintf(ts_f, "}\n\n");

  /* Classes for structs */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      fprintf(ts_f, "export declare class %s {\n", node->name);
      for (j = 0; j < (unsigned long)node->fields_count; j++) {
        map_ts_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(ts_f, "  %s: %s;\n", node->fields[j].name, type_str);
      }
      fprintf(ts_f, "}\n\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(ts_f, "export enum %s {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        fprintf(ts_f, "  %s%s\n", var->name,
                (unsigned long)(j + 1) < node->variants_count ? "," : "");
      }
      fprintf(ts_f, "}\n\n");
    }
  }

  fprintf(
      ts_f,
      "export default function Module(moduleArg?: any): Promise<%sModule>;\n",
      lib_name);
  fclose(ts_f);

  return CDD_C_SUCCESS;
}
