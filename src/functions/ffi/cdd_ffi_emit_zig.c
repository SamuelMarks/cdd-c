/* clang-format off */
#include "cdd_ffi_emit_zig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_zig_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return type.is_const ? "[*c]const u8" : "[*c]u8";
    }
    return "?*anyopaque";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
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
    return "?*anyopaque";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "?*anyopaque";
  case CDD_FFI_KIND_ENUM_REF:
    return "c_int";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "?*anyopaque";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "?*anyopaque";
  default:
    return "?*anyopaque";
  }
}

static int emit_zig_file(cdd_ffi_ir_t *ir,
                         const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.zig", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.zig", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "// Auto-generated Zig bindings for %s\n\n", lib_name);

  if (config->input) {
    fprintf(f, "pub const c = @cImport({\n");
    fprintf(f, "    @cInclude(\"%s\");\n", config->input);
    fprintf(f, "});\n\n");

    /* Just re-export if using cImport */
    fprintf(f, "// By using @cImport, Zig automatically parses the header.\n");
    fprintf(f, "// We re-export symbols for convenience.\n\n");

    for (i = 0; i < ir->nodes_count; i++) {
      cdd_ffi_ir_node_t *node = &ir->nodes[i];
      if (node->kind == CDD_FFI_NODE_FUNCTION) {
        fprintf(f, "pub const %s = c.%s;\n", node->name, node->name);
      } else if (node->kind == CDD_FFI_NODE_STRUCT) {
        fprintf(f, "pub const %s = c.%s;\n", node->name, node->name);
        fprintf(f, "pub const struct_%s = c.struct_%s;\n", node->name,
                node->name);
      } else if (node->kind == CDD_FFI_NODE_ENUM) {
        fprintf(f, "pub const %s = c.%s;\n", node->name, node->name);
      }
    }
  } else {
    /* Manual extern declarations if no header provided to parse */
    fprintf(f, "const std = @import(\"std\");\n\n");

    /* Structs */
    for (i = 0; i < ir->nodes_count; i++) {
      cdd_ffi_ir_node_t *node = &ir->nodes[i];
      if (node->kind == CDD_FFI_NODE_STRUCT) {
        if (node->doc)
          fprintf(f, "/// %s\n", node->doc);
        fprintf(f, "pub const %s = extern struct {\n", node->name);
        for (j = 0; j < node->fields_count; j++) {
          const char *fname =
              node->fields[j].name ? node->fields[j].name : "field";
          fprintf(f, "    %s: %s,\n", fname,
                  get_zig_type(node->fields[j].type));
        }
        fprintf(f, "};\n\n");
      }
    }

    /* Functions */
    for (i = 0; i < ir->nodes_count; i++) {
      cdd_ffi_ir_node_t *node = &ir->nodes[i];
      if (node->kind == CDD_FFI_NODE_FUNCTION) {
        if (node->doc)
          fprintf(f, "/// %s\n", node->doc);
        fprintf(f, "pub extern fn %s(", node->name);
        for (j = 0; j < node->fields_count; j++) {
          const char *arg_name =
              node->fields[j].name ? node->fields[j].name : "arg";
          if (strcmp(arg_name, "error") == 0)
            arg_name = "err";
          if (strcmp(arg_name, "type") == 0)
            arg_name = "type_";
          fprintf(f, "%s: %s", arg_name, get_zig_type(node->fields[j].type));
          if (j < node->fields_count - 1)
            fprintf(f, ", ");
        }
        fprintf(f, ") %s;\n\n", get_zig_type(node->return_or_base_type));
      }
    }
  }

  fclose(f);
  return 0;
}

int cdd_ffi_emit_zig(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  return emit_zig_file(ir, config);
}
