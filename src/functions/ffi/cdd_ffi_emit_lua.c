/* clang-format off */
#include "cdd_ffi_emit_lua.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_luajit_c_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return type.is_const ? "const char*" : "char*";
    }
    return "void*";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "bool";
  case CDD_FFI_KIND_INT8:
    return "int8_t";
  case CDD_FFI_KIND_UINT8:
    return "uint8_t";
  case CDD_FFI_KIND_INT16:
    return "int16_t";
  case CDD_FFI_KIND_UINT16:
    return "uint16_t";
  case CDD_FFI_KIND_INT32:
    return "int32_t";
  case CDD_FFI_KIND_UINT32:
    return "uint32_t";
  case CDD_FFI_KIND_INT64:
    return "int64_t";
  case CDD_FFI_KIND_UINT64:
    return "uint64_t";
  case CDD_FFI_KIND_FLOAT32:
    return "float";
  case CDD_FFI_KIND_FLOAT64:
    return "double";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "void*";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "void*";
  case CDD_FFI_KIND_ENUM_REF:
    return "int";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "void*";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "void*";
  default:
    return "void*";
  }
}

static int emit_lua_file(cdd_ffi_ir_t *ir,
                         const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.lua", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.lua", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "-- Auto-generated LuaJIT FFI bindings for %s\n\n", lib_name);
  fprintf(f, "local ffi = require(\"ffi\")\n\n");

  fprintf(f, "ffi.cdef[[\n");

  /* C definitions inside cdef block */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      fprintf(f, "  typedef struct %s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = get_luajit_c_type(node->fields[j].type);
        if (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(f, "    struct %s %s;\n", ftype, fname);
        } else {
          fprintf(f, "    %s %s;\n", ftype, fname);
        }
      }
      fprintf(f, "  } %s;\n\n", node->name);
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "  enum %s {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s", var->name, var->value);
        } else {
          fprintf(f, "    %s = " CDD_PRIz "", var->name, j);
        }
        if (j < node->variants_count - 1)
          fprintf(f, ",");
        fprintf(f, "\n");
      }
      fprintf(f, "  };\n\n");
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "  %s %s(", get_luajit_c_type(node->return_or_base_type),
              node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s %s", get_luajit_c_type(node->fields[j].type),
                node->fields[j].name ? node->fields[j].name : "arg");
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ");\n");
    }
  }

  fprintf(f, "]]\n\n");
  fprintf(f, "local lib = ffi.load(\"%s\")\n\n", lib_name);

  fprintf(f, "local M = {\n");
  fprintf(f, "  lib = lib,\n");
  fprintf(f, "}\n\n");

  /* Optional: generate wrappers */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->doc)
        fprintf(f, "-- %s\n", node->doc);
      fprintf(f, "function M.%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "function") == 0)
          arg_name = "func";
        fprintf(f, "%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")\n");

      fprintf(f, "  return lib.%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "function") == 0)
          arg_name = "func";
        fprintf(f, "%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")\n");
      fprintf(f, "end\n\n");
    }
  }

  fprintf(f, "return M\n");

  fclose(f);
  return 0;
}

int cdd_ffi_emit_lua(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  return emit_lua_file(ir, config);
}
