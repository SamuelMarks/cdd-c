/* clang-format off */
#include "cdd_ffi_emit_dart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_dart_ffi_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "Pointer<Utf8>";
    }
    return "Pointer<Void>";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "Void";
  case CDD_FFI_KIND_BOOL:
    return "Bool";
  case CDD_FFI_KIND_INT8:
    return "Int8";
  case CDD_FFI_KIND_UINT8:
    return "Uint8";
  case CDD_FFI_KIND_INT16:
    return "Int16";
  case CDD_FFI_KIND_UINT16:
    return "Uint16";
  case CDD_FFI_KIND_INT32:
    return "Int32";
  case CDD_FFI_KIND_UINT32:
    return "Uint32";
  case CDD_FFI_KIND_INT64:
    return "Int64";
  case CDD_FFI_KIND_UINT64:
    return "Uint64";
  case CDD_FFI_KIND_FLOAT32:
    return "Float";
  case CDD_FFI_KIND_FLOAT64:
    return "Double";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "Pointer<NativeFunction>";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "Struct";
  case CDD_FFI_KIND_ENUM_REF:
    return "Int32";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "Pointer<Void>";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "Pointer<Void>";
  default:
    return "Pointer<Void>";
  }
}

static const char *get_dart_native_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "Pointer<Utf8>";
    }
    return "Pointer<Void>";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "bool";
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
  case CDD_FFI_KIND_INT16:
  case CDD_FFI_KIND_UINT16:
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_UINT32:
  case CDD_FFI_KIND_INT64:
  case CDD_FFI_KIND_UINT64:
  case CDD_FFI_KIND_ENUM_REF:
    return "int";
  case CDD_FFI_KIND_FLOAT32:
  case CDD_FFI_KIND_FLOAT64:
    return "double";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "Pointer<NativeFunction>";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "Struct";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "Pointer<Void>";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "Pointer<Void>";
  default:
    return "Pointer<Void>";
  }
}

static enum cdd_c_error
emit_dart_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.dart", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.dart", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "// Auto-generated Dart FFI bindings for %s\n\n", lib_name);
  fprintf(f, "import 'dart:ffi';\n");
  fprintf(f, "import 'package:ffi/ffi.dart';\n\n");

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      if (node->doc)
        fprintf(f, "/// %s\n", node->doc);
      fprintf(f, "final class %s extends Struct {\n", node->name);

      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = get_dart_ffi_type(node->fields[j].type);
        const char *ntype = get_dart_native_type(node->fields[j].type);

        fprintf(f, "  @%s()\n", ftype);
        fprintf(f, "  external %s %s;\n", ntype, fname);
      }
      fprintf(f, "}\n\n");
    }
  }

  /* Library Class */
  fprintf(f, "class %sBindings {\n", lib_name);
  fprintf(f, "  final DynamicLibrary _dylib;\n\n");
  fprintf(f, "  %sBindings(this._dylib);\n\n", lib_name);

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->doc)
        fprintf(f, "  /// %s\n", node->doc);

      /* Typedefs for lookup */
      fprintf(f, "  late final _%sPtr = _dylib.lookup<NativeFunction<\n",
              node->name);
      fprintf(f, "      %s Function(",
              get_dart_ffi_type(node->return_or_base_type));
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s", get_dart_ffi_type(node->fields[j].type));
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")>>('%s');\n", node->name);

      fprintf(f, "  late final _%s = _%sPtr.asFunction<\n", node->name,
              node->name);
      fprintf(f, "      %s Function(",
              get_dart_native_type(node->return_or_base_type));
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s", get_dart_native_type(node->fields[j].type));
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")>();\n\n");

      /* Wrapper method */
      fprintf(f, "  %s %s(", get_dart_native_type(node->return_or_base_type),
              node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "in") == 0)
          arg_name = "in_";
        fprintf(f, "%s %s", get_dart_native_type(node->fields[j].type),
                arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ") {\n");

      fprintf(f, "    return _%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "in") == 0)
          arg_name = "in_";
        fprintf(f, "%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ");\n");
      fprintf(f, "  }\n\n");
    }
  }

  fprintf(f, "}\n");
  fclose(f);
  return CDD_C_SUCCESS;
}

enum cdd_c_error
cdd_ffi_emit_dart(cdd_ffi_ir_t *ir,
                  const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_dart_file(ir, config);
}
