/**
 * @file cdd_ffi_emit_lua.c
 * @brief FFI binding emitter for Lua.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_lua.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

static cdd_c_error_t get_luajit_c_type(cdd_ffi_type_t type,
                                       const char **out_type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      *out_type = type.is_const ? "const char*" : "char*";
      return CDD_C_SUCCESS;
    }
    *out_type = "void*";
    return CDD_C_SUCCESS;
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = "void";
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = "bool";
    break;
  case CDD_FFI_KIND_INT8:
    *out_type = "int8_t";
    break;
  case CDD_FFI_KIND_UINT8:
    *out_type = "uint8_t";
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = "int16_t";
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = "uint16_t";
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = "int32_t";
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = "uint32_t";
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = "int64_t";
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = "uint64_t";
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = "float";
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = "double";
    break;
  case CDD_FFI_KIND_FUNCTION_PTR:
    *out_type = "void*";
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    *out_type = type.ref_name ? type.ref_name : "void*";
    break;
  case CDD_FFI_KIND_ENUM_REF:
    *out_type = "int";
    break;
  case CDD_FFI_KIND_TYPEDEF_REF:
    *out_type = type.ref_name ? type.ref_name : "void*";
    break;
  case CDD_FFI_KIND_OPAQUE_PTR:
    *out_type = "void*";
    break;
  default:
    *out_type = "void*";
    break;
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
emit_lua_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name =
      (config->library_name) ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.lua", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.lua", config->output_dir,
               lib_name);
#if defined(_MSC_VER)
  if (fopen_s(&f, filepath, "w") != 0)
    f = NULL;
#else
  f = fopen(filepath, "w");
#endif
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "-- Auto-generated LuaJIT FFI bindings for %s\n\n", lib_name);
  fprintf(f, "local ffi = require(\"ffi\")\n\n");

  fprintf(f, "ffi.cdef[[\n");

  /* C definitions inside cdef block */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "  typedef struct %s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = NULL;
        get_luajit_c_type(node->fields[j].type, &ftype);
        if (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(f, "    struct %s %s;\n", ftype, fname);
        } else {
          fprintf(f, "    %s %s;\n", ftype, fname);
        }
      }
      fprintf(f, "  } %s;\n\n", node->name);
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "  enum %s {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s", var->name, var->value);
        } else {
          fprintf(f, "    %s = %lu", var->name, (unsigned long)j);
        }
        if (j < node->variants_count - 1)
          fprintf(f, ",");
        fprintf(f, "\n");
      }
      fprintf(f, "  };\n\n");
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_lt = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      get_luajit_c_type(node->return_or_base_type, &ret_lt);
      fprintf(f, "  %s %s(", ret_lt, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *lt = NULL;
        get_luajit_c_type(node->fields[j].type, &lt);
        fprintf(f, "%s %s", lt,
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
      cdd_ffi_emit_trivia(f, node->leading_trivia);
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
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_ffi_emit_lua(cdd_ffi_ir_t *ir,
                               const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_lua_file(ir, config);
}
