/**
 * @file cdd_ffi_emit_dart.c
 * @brief FFI binding emitter for Dart.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_dart.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding `dart:ffi` native C type
 * annotation string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved `dart:ffi` type
 * string.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t get_dart_ffi_type(cdd_ffi_type_t type,
                                       const char **out_type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      *out_type = "Pointer<Utf8>";
      return CDD_C_SUCCESS;
    }
    *out_type = "Pointer<Void>";
    return CDD_C_SUCCESS;
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = "Void";
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = "Bool";
    break;
  case CDD_FFI_KIND_INT8:
    *out_type = "Int8";
    break;
  case CDD_FFI_KIND_UINT8:
    *out_type = "Uint8";
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = "Int16";
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = "Uint16";
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = "Int32";
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = "Uint32";
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = "Int64";
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = "Uint64";
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = "Float";
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = "Double";
    break;
  case CDD_FFI_KIND_FUNCTION_PTR:
    *out_type = "Pointer<NativeFunction>";
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    *out_type = type.ref_name ? type.ref_name : "Struct";
    break;
  case CDD_FFI_KIND_ENUM_REF:
    *out_type = "Int32";
    break;
  case CDD_FFI_KIND_TYPEDEF_REF:
    *out_type = type.ref_name ? type.ref_name : "Pointer<Void>";
    break;
  case CDD_FFI_KIND_OPAQUE_PTR:
    *out_type = "Pointer<Void>";
    break;
  default:
    *out_type = "Pointer<Void>";
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Maps an FFI IR type to its corresponding Dart language runtime type
 * string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved Dart runtime type
 * string.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t get_dart_native_type(cdd_ffi_type_t type,
                                          const char **out_type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      *out_type = "Pointer<Utf8>";
      return CDD_C_SUCCESS;
    }
    *out_type = "Pointer<Void>";
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
  case CDD_FFI_KIND_UINT8:
  case CDD_FFI_KIND_INT16:
  case CDD_FFI_KIND_UINT16:
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_UINT32:
  case CDD_FFI_KIND_INT64:
  case CDD_FFI_KIND_UINT64:
  case CDD_FFI_KIND_ENUM_REF:
    *out_type = "int";
    break;
  case CDD_FFI_KIND_FLOAT32:
  case CDD_FFI_KIND_FLOAT64:
    *out_type = "double";
    break;
  case CDD_FFI_KIND_FUNCTION_PTR:
    *out_type = "Pointer<NativeFunction>";
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    *out_type = type.ref_name ? type.ref_name : "Struct";
    break;
  case CDD_FFI_KIND_TYPEDEF_REF:
    *out_type = type.ref_name ? type.ref_name : "Pointer<Void>";
    break;
  case CDD_FFI_KIND_OPAQUE_PTR:
    *out_type = "Pointer<Void>";
    break;
  default:
    *out_type = "Pointer<Void>";
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a `.dart` source file containing `dart:ffi` struct and dynamic
 * library bindings for the given FFI IR.
 *
 * @param[in] ir The extracted and topologically sorted FFI IR.
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t
emit_dart_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name =
      (config->library_name) ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.dart", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.dart", config->output_dir,
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

  fprintf(f, "// Auto-generated Dart FFI bindings for %s\n\n", lib_name);
  fprintf(f, "import 'dart:ffi';\n");
  fprintf(f, "import 'package:ffi/ffi.dart';\n\n");

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "/// %s\n", node->doc);
      fprintf(f, "final class %s extends Struct {\n", node->name);

      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = NULL;
        const char *ntype = NULL;
        get_dart_ffi_type(node->fields[j].type, &ftype);
        get_dart_native_type(node->fields[j].type, &ntype);

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
      const char *ret_ftype = NULL;
      const char *ret_ntype = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "  /// %s\n", node->doc);

      get_dart_ffi_type(node->return_or_base_type, &ret_ftype);
      get_dart_native_type(node->return_or_base_type, &ret_ntype);

      /* Typedefs for lookup */
      fprintf(f, "  late final _%sPtr = _dylib.lookup<NativeFunction<\n",
              node->name);
      fprintf(f, "      %s Function(", ret_ftype);
      for (j = 0; j < node->fields_count; j++) {
        const char *ft = NULL;
        get_dart_ffi_type(node->fields[j].type, &ft);
        fprintf(f, "%s", ft);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")>>('%s');\n", node->name);

      fprintf(f, "  late final _%s = _%sPtr.asFunction<\n", node->name,
              node->name);
      fprintf(f, "      %s Function(", ret_ntype);
      for (j = 0; j < node->fields_count; j++) {
        const char *nt = NULL;
        get_dart_native_type(node->fields[j].type, &nt);
        fprintf(f, "%s", nt);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")>();\n\n");

      /* Wrapper method */
      fprintf(f, "  %s %s(", ret_ntype, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        const char *nt = NULL;
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "in") == 0)
          arg_name = "in_";
        get_dart_native_type(node->fields[j].type, &nt);
        fprintf(f, "%s %s", nt, arg_name);
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

/**
 * @brief Emits Dart FFI bindings for the given FFI IR.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t cdd_ffi_emit_dart(cdd_ffi_ir_t *ir,
                                const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_dart_file(ir, config);
}
