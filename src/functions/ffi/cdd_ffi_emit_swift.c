/**
 * @file cdd_ffi_emit_swift.c
 * @brief FFI binding emitter for Swift.
 */
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_swift.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding Swift C-interop type string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved Swift type string.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t get_swift_type(cdd_ffi_type_t type,
                                    const char **out_type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      *out_type = type.is_const ? "UnsafePointer<CChar>?"
                                : "UnsafeMutablePointer<CChar>?";
      return CDD_C_SUCCESS;
    }
    *out_type = "UnsafeMutableRawPointer?";
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
    *out_type = "UInt8";
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = "Int16";
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = "UInt16";
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = "Int32";
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = "UInt32";
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = "Int64";
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = "UInt64";
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = "Float";
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = "Double";
    break;
  case CDD_FFI_KIND_FUNCTION_PTR:
    *out_type = "UnsafeMutableRawPointer?";
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    *out_type = type.ref_name ? type.ref_name : "UnsafeMutableRawPointer?";
    break;
  case CDD_FFI_KIND_ENUM_REF:
    *out_type = type.ref_name ? type.ref_name : "Int32";
    break;
  case CDD_FFI_KIND_TYPEDEF_REF:
    *out_type = type.ref_name ? type.ref_name : "UnsafeMutableRawPointer?";
    break;
  case CDD_FFI_KIND_OPAQUE_PTR:
    *out_type = "OpaquePointer?";
    break;
  default:
    *out_type = "UnsafeMutableRawPointer?";
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a `.swift` source file containing struct wrapper classes and
 * static C interop methods for the given FFI IR.
 *
 * @param[in] ir The extracted and topologically sorted FFI IR.
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t
emit_swift_file(cdd_ffi_ir_t *ir,
                const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name =
      (config->library_name) ? config->library_name : "MyLib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.swift", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0)
    f = NULL;
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.swift", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
#endif
#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      if (f)
        fclose(f);
      f = NULL;
    }
  }
#endif
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }

  fprintf(f, "// Auto-generated Swift bindings for %s\n\n", lib_name);
  fprintf(f, "import Foundation\n");
  /* In a real project we might import the Clang module: import CMyLib */
  fprintf(f, "// import C%s\n\n", lib_name);

  /* Struct Wrappers */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "/// %s\n", node->doc);
      fprintf(f, "public class %sWrapper {\n", node->name);
      fprintf(f, "    public var inner: %s\n\n", node->name);

      fprintf(f, "    public init(inner: %s) {\n", node->name);
      fprintf(f, "        self.inner = inner\n");
      fprintf(f, "    }\n\n");

      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *stype = NULL;
        get_swift_type(node->fields[j].type, &stype);
        fprintf(f, "    public var %s: %s {\n", fname, stype);
        fprintf(f, "        get { return inner.%s }\n", fname);
        fprintf(f, "        set { inner.%s = newValue }\n", fname);
        fprintf(f, "    }\n");
      }
      fprintf(f, "}\n\n");
    }
  }

  /* Function Wrappers */
  fprintf(f, "public class %s {\n", lib_name);
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "    /// %s\n", node->doc);
      fprintf(f, "    public static func %s(", node->name);

      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        const char *st = NULL;
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "inout") == 0)
          arg_name = "inout_";
        get_swift_type(node->fields[j].type, &st);
        fprintf(f, "_ %s: %s", arg_name, st);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")");

      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        const char *ret_st = NULL;
        get_swift_type(node->return_or_base_type, &ret_st);
        fprintf(f, " -> %s", ret_st);
      }
      fprintf(f, " {\n");

      fprintf(f, "        ");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "return ");
      }

      /* In a real environment, we'd prefix with the C module name or assume
       * it's global */
      fprintf(f, "::%s(", node->name); /* Just calling the C function */
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "inout") == 0)
          arg_name = "inout_";
        fprintf(f, "%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")\n");
      fprintf(f, "    }\n\n");
    }
  }
  fprintf(f, "}\n");

  fclose(f);
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a Clang `module.modulemap` file exposing the C header to Swift.
 *
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t
emit_module_map(const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  const char *lib_name =
      (config->library_name) ? config->library_name : "MyLib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\module.modulemap",
               config->output_dir);
  if (fopen_s(&f, filepath, "w") != 0)
    f = NULL;
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/module.modulemap",
               config->output_dir);
  f = fopen(filepath, "w");
#endif
#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      if (f)
        fclose(f);
      f = NULL;
    }
  }
#endif
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }

  fprintf(f, "module C%s {\n", lib_name);
  if (config->input) {
    fprintf(f, "    header \"%s\"\n", config->input);
  }
  fprintf(f, "    export *\n");
  fprintf(f, "}\n");

  fclose(f);
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Swift `.swift` bindings and `module.modulemap` files for the
 * given FFI IR.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t cdd_ffi_emit_swift(cdd_ffi_ir_t *ir,
                                 const cdd_generate_bindings_config_t *config) {
  cdd_c_error_t rc;
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  rc = emit_swift_file(ir, config);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return emit_module_map(config);
}
