/* clang-format off */
#include "cdd_ffi_emit_odin.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_odin_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "cstring";
    }
    return "rawptr";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "---"; /* Odin functions returning void just have no return type decl
                   */
  case CDD_FFI_KIND_BOOL:
    return "bool";
  case CDD_FFI_KIND_INT8:
    return "i8";
  case CDD_FFI_KIND_UINT8:
    return "u8";
  case CDD_FFI_KIND_INT16:
    return "i16";
  case CDD_FFI_KIND_UINT16:
    return "u16";
  case CDD_FFI_KIND_INT32:
    return "i32";
  case CDD_FFI_KIND_UINT32:
    return "u32";
  case CDD_FFI_KIND_INT64:
    return "i64";
  case CDD_FFI_KIND_UINT64:
    return "u64";
  case CDD_FFI_KIND_FLOAT32:
    return "f32";
  case CDD_FFI_KIND_FLOAT64:
    return "f64";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "rawptr";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "rawptr";
  case CDD_FFI_KIND_ENUM_REF:
    return "c.int";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "rawptr";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "rawptr";
  default:
    return "rawptr";
  }
}

static int emit_odin_file(cdd_ffi_ir_t *ir,
                          const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.odin", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.odin", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "// Auto-generated Odin bindings for %s\n\n", lib_name);
  fprintf(f, "package %s\n\n", lib_name);
  fprintf(f, "import \"core:c\"\n\n");

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      if (node->doc)
        fprintf(f, "// %s\n", node->doc);
      fprintf(f, "%s :: struct #c {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *otype = get_odin_type(node->fields[j].type);
        fprintf(f, "    %s: %s,\n", fname, otype);
      }
      fprintf(f, "}\n\n");
    }
  }

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      if (node->doc)
        fprintf(f, "// %s\n", node->doc);
      fprintf(f, "%s :: enum c.int {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s,\n", var->name, var->value);
        } else {
          fprintf(f, "    %s = %" CDD_PRIz ",\n", var->name, j);
        }
      }
      fprintf(f, "}\n\n");
    }
  }

  /* Foreign block for functions */
  fprintf(f, "when ODIN_OS == .Windows {\n");
  fprintf(f, "    foreign import lib \"%s.lib\"\n", lib_name);
  fprintf(f, "} else when ODIN_OS == .Darwin {\n");
  fprintf(f, "    foreign import lib \"%s.dylib\"\n", lib_name);
  fprintf(f, "} else {\n");
  fprintf(f, "    foreign import lib \"%s.so\"\n", lib_name);
  fprintf(f, "}\n\n");

  fprintf(f, "@(default_calling_convention=\"c\")\n");
  fprintf(f, "foreign lib {\n");

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->doc)
        fprintf(f, "    // %s\n", node->doc);
      fprintf(f, "    %s :: proc(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "context") == 0)
          arg_name = "ctx";
        if (strcmp(arg_name, "in") == 0)
          arg_name = "in_";
        fprintf(f, "%s: %s", arg_name, get_odin_type(node->fields[j].type));
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")");

      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, " -> %s", get_odin_type(node->return_or_base_type));
      }
      fprintf(f, " ---\n");
    }
  }
  fprintf(f, "}\n");

  fclose(f);
  return 0;
}

int cdd_ffi_emit_odin(cdd_ffi_ir_t *ir,
                      const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  return emit_odin_file(ir, config);
}
