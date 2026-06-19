/* clang-format off */
#include "cdd_ffi_emit_swift.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_swift_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return type.is_const ? "UnsafePointer<CChar>?"
                           : "UnsafeMutablePointer<CChar>?";
    }
    return "UnsafeMutableRawPointer?";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "Void";
  case CDD_FFI_KIND_BOOL:
    return "Bool";
  case CDD_FFI_KIND_INT8:
    return "Int8";
  case CDD_FFI_KIND_UINT8:
    return "UInt8";
  case CDD_FFI_KIND_INT16:
    return "Int16";
  case CDD_FFI_KIND_UINT16:
    return "UInt16";
  case CDD_FFI_KIND_INT32:
    return "Int32";
  case CDD_FFI_KIND_UINT32:
    return "UInt32";
  case CDD_FFI_KIND_INT64:
    return "Int64";
  case CDD_FFI_KIND_UINT64:
    return "UInt64";
  case CDD_FFI_KIND_FLOAT32:
    return "Float";
  case CDD_FFI_KIND_FLOAT64:
    return "Double";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "UnsafeMutableRawPointer?";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "UnsafeMutableRawPointer?";
  case CDD_FFI_KIND_ENUM_REF:
    return type.ref_name ? type.ref_name : "Int32";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "UnsafeMutableRawPointer?";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "OpaquePointer?";
  default:
    return "UnsafeMutableRawPointer?";
  }
}

static int emit_swift_file(cdd_ffi_ir_t *ir,
                           const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "MyLib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.swift", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.swift", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "// Auto-generated Swift bindings for %s\n\n", lib_name);
  fprintf(f, "import Foundation\n");
  /* In a real project we might import the Clang module: import CMyLib */
  fprintf(f, "// import C%s\n\n", lib_name);

  /* Struct Wrappers */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
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
        const char *stype = get_swift_type(node->fields[j].type);
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
      if (node->doc)
        fprintf(f, "    /// %s\n", node->doc);
      fprintf(f, "    public static func %s(", node->name);

      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "inout") == 0)
          arg_name = "inout_";
        fprintf(f, "_ %s: %s", arg_name, get_swift_type(node->fields[j].type));
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")");

      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, " -> %s", get_swift_type(node->return_or_base_type));
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
  return 0;
}

static int emit_module_map(const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  const char *lib_name = config->library_name ? config->library_name : "MyLib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\module.modulemap",
               config->output_dir);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/module.modulemap",
               config->output_dir);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "module C%s {\n", lib_name);
  if (config->input) {
    fprintf(f, "    header \"%s\"\n", config->input);
  }
  fprintf(f, "    export *\n");
  fprintf(f, "}\n");

  fclose(f);
  return 0;
}

int cdd_ffi_emit_swift(cdd_ffi_ir_t *ir,
                       const cdd_generate_bindings_config_t *config) {
  int rc;
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  rc = emit_swift_file(ir, config);
  if (rc != 0)
    return rc;

  return emit_module_map(config);
}
