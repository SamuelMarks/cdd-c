/**
 * @file cdd_ffi_emit_zig.c
 * @brief FFI binding emitter for Zig.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_zig.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding Zig `extern` C interop type
 * string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved Zig type string.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t get_zig_type(cdd_ffi_type_t type, const char **out_type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      *out_type = type.is_const ? "[*c]const u8" : "[*c]u8";
      return CDD_C_SUCCESS;
    }
    *out_type = "?*anyopaque";
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
    *out_type = "i8";
    break;
  case CDD_FFI_KIND_UINT8:
    *out_type = "u8";
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = "i16";
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = "u16";
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = "i32";
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = "u32";
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = "i64";
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = "u64";
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = "f32";
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = "f64";
    break;
  case CDD_FFI_KIND_FUNCTION_PTR:
    *out_type = "?*anyopaque";
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    *out_type = type.ref_name ? type.ref_name : "?*anyopaque";
    break;
  case CDD_FFI_KIND_ENUM_REF:
    *out_type = "c_int";
    break;
  case CDD_FFI_KIND_TYPEDEF_REF:
    *out_type = type.ref_name ? type.ref_name : "?*anyopaque";
    break;
  case CDD_FFI_KIND_OPAQUE_PTR:
    *out_type = "?*anyopaque";
    break;
  default:
    *out_type = "?*anyopaque";
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a `.zig` source file containing `@cImport` re-exports or manual
 * `extern` declarations for the given FFI IR.
 *
 * @param[in] ir The extracted and topologically sorted FFI IR.
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t
emit_zig_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name =
      (config->library_name) ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.zig", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.zig", config->output_dir,
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
        cdd_ffi_emit_trivia(f, node->leading_trivia);
        fprintf(f, "pub const %s = c.%s;\n", node->name, node->name);
      } else if (node->kind == CDD_FFI_NODE_STRUCT) {
        cdd_ffi_emit_trivia(f, node->leading_trivia);
        fprintf(f, "pub const %s = c.%s;\n", node->name, node->name);
        fprintf(f, "pub const struct_%s = c.struct_%s;\n", node->name,
                node->name);
      } else if (node->kind == CDD_FFI_NODE_ENUM) {
        cdd_ffi_emit_trivia(f, node->leading_trivia);
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
        cdd_ffi_emit_trivia(f, node->leading_trivia);
        if (node->doc)
          fprintf(f, "/// %s\n", node->doc);
        fprintf(f, "pub const %s = extern struct {\n", node->name);
        for (j = 0; j < node->fields_count; j++) {
          const char *fname =
              node->fields[j].name ? node->fields[j].name : "field";
          const char *zt = NULL;
          get_zig_type(node->fields[j].type, &zt);
          fprintf(f, "    %s: %s,\n", fname, zt);
        }
        fprintf(f, "};\n\n");
      }
    }

    /* Functions */
    for (i = 0; i < ir->nodes_count; i++) {
      cdd_ffi_ir_node_t *node = &ir->nodes[i];
      if (node->kind == CDD_FFI_NODE_FUNCTION) {
        const char *ret_zt = NULL;
        cdd_ffi_emit_trivia(f, node->leading_trivia);
        if (node->doc)
          fprintf(f, "/// %s\n", node->doc);
        fprintf(f, "pub extern fn %s(", node->name);
        for (j = 0; j < node->fields_count; j++) {
          const char *arg_name =
              node->fields[j].name ? node->fields[j].name : "arg";
          const char *zt = NULL;
          if (strcmp(arg_name, "error") == 0)
            arg_name = "err";
          if (strcmp(arg_name, "type") == 0)
            arg_name = "type_";
          get_zig_type(node->fields[j].type, &zt);
          fprintf(f, "%s: %s", arg_name, zt);
          if (j < node->fields_count - 1)
            fprintf(f, ", ");
        }
        get_zig_type(node->return_or_base_type, &ret_zt);
        fprintf(f, ") %s;\n\n", ret_zt);
      }
    }
  }

  fclose(f);
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Zig FFI bindings for the given FFI IR.
 *
 * @param[in] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t cdd_ffi_emit_zig(cdd_ffi_ir_t *ir,
                               const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_zig_file(ir, config);
}
