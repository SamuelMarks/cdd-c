/**
 * @file cdd_ffi_emit_odin.c
 * @brief FFI binding emitter for Odin.
 */
#ifdef CDD_BUILD_TESTS
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_odin.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

static cdd_c_error_t get_odin_type(cdd_ffi_type_t type, const char **out_type) {
  static const cdd_ffi_type_map_table_t k_odin_types = {
      "---", "bool",  "i8",  "u8",  "i16", "u16",     "i32",
      "u32", "i64",   "u64", "f32", "f64", "cstring", "rawptr",
      NULL,  "c.int", 1,     0,     1};
  return cdd_ffi_map_type_from_table(&k_odin_types, type, out_type);
}

static cdd_c_error_t
emit_odin_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name =
      (config->library_name) ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.odin", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.odin", config->output_dir,
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

#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after == 1) {
      fclose(f);
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#endif

  fprintf(f, "// Auto-generated Odin bindings for %s\n\n", lib_name);
  fprintf(f, "package %s\n\n", lib_name);
  fprintf(f, "import \"core:c\"\n\n");

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "// %s\n", node->doc);
      fprintf(f, "%s :: struct #c {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *otype = NULL;
        get_odin_type(node->fields[j].type, &otype);
        fprintf(f, "    %s: %s,\n", fname, otype);
      }
      fprintf(f, "}\n\n");
    }
  }

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "// %s\n", node->doc);
      fprintf(f, "%s :: enum c.int {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s,\n", var->name, var->value);
        } else {
          fprintf(f, "    %s = %lu,\n", var->name, (unsigned long)j);
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
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "    // %s\n", node->doc);
      fprintf(f, "    %s :: proc(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        const char *ot = NULL;
        if (strcmp(arg_name, "context") == 0)
          arg_name = "ctx";
        if (strcmp(arg_name, "in") == 0)
          arg_name = "in_";
        get_odin_type(node->fields[j].type, &ot);
        fprintf(f, "%s: %s", arg_name, ot);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")");

      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        const char *ret_ot = NULL;
        get_odin_type(node->return_or_base_type, &ret_ot);
        fprintf(f, " -> %s", ret_ot);
      }
      fprintf(f, " ---\n");
    }
  }
  fprintf(f, "}\n");

  fclose(f);
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_ffi_emit_odin(cdd_ffi_ir_t *ir,
                                const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_odin_file(ir, config);
}
