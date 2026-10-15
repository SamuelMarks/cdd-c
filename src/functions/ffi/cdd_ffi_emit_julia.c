/**
 * @file cdd_ffi_emit_julia.c
 * @brief FFI binding emitter for Julia.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_julia.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

static cdd_c_error_t get_julia_type(cdd_ffi_type_t type,
                                    const char **out_type) {
  static const cdd_ffi_type_map_table_t k_julia_types = {
      "Cvoid",   "Cint",       "Int8",  "UInt8",  "Int16",   "UInt16",
      "Int32",   "UInt32",     "Int64", "UInt64", "Float32", "Float64",
      "Cstring", "Ptr{Cvoid}", NULL,    "Cint",   1,         0,
      1};
  return cdd_ffi_map_type_from_table(&k_julia_types, type, out_type);
}

static cdd_c_error_t to_camel_case(const char *snake, char *out,
                                   size_t out_size) {
  size_t i, j = 0;
  int up = 1;
  for (i = 0; i < strlen(snake) && j < out_size - 1; i++) {
    if (snake[i] == '_') {
      up = 1;
    } else {
      if (up) {
        out[j++] = (char)toupper(snake[i]);
        up = 0;
      } else {
        out[j++] = snake[i];
      }
    }
  }
  out[j] = '\0';
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
emit_julia_file(cdd_ffi_ir_t *ir,
                const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = "mylib";
  char module_name[256];

  if (config->library_name) {
    lib_name = config->library_name;
  }

  to_camel_case(lib_name, module_name, sizeof(module_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.jl", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.jl", config->output_dir,
               module_name);
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

  fprintf(f, "# Auto-generated Julia bindings for %s\n\n", lib_name);
  fprintf(f, "module %s\n\n", module_name);

  fprintf(f, "const lib_path = \"%s\"\n\n", lib_name);

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "# %s\n", node->doc);
      fprintf(f, "@enum %s::Cint begin\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s\n", var->name, var->value);
        } else {
          fprintf(f, "    %s = %lu\n", var->name, (unsigned long)j);
        }
      }
      fprintf(f, "end\n\n");
    }
  }

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "# %s\n", node->doc);
      fprintf(f, "mutable struct %s\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *jt = NULL;
        get_julia_type(node->fields[j].type, &jt);
        fprintf(f, "    %s::%s\n", fname, jt);
      }
      fprintf(f, "    %s() = new()\n", node->name);
      fprintf(f, "end\n\n");
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_jt = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "# %s\n", node->doc);
      fprintf(f, "function %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        /* avoid julia keywords */
        if (strcmp(arg_name, "function") == 0)
          arg_name = "func";
        fprintf(f, "%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")\n");

      get_julia_type(node->return_or_base_type, &ret_jt);
      fprintf(f, "    ccall((:%s, lib_path), %s, (", node->name, ret_jt);
      for (j = 0; j < node->fields_count; j++) {
        const char *jt = NULL;
        get_julia_type(node->fields[j].type, &jt);
        fprintf(f, "%s", jt);
        if (j < node->fields_count - 1 || node->fields_count == 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")");

      if (node->fields_count > 0) {
        fprintf(f, ", ");
        for (j = 0; j < node->fields_count; j++) {
          const char *arg_name =
              node->fields[j].name ? node->fields[j].name : "arg";
          if (strcmp(arg_name, "function") == 0)
            arg_name = "func";
          fprintf(f, "%s", arg_name);
          if (j < node->fields_count - 1)
            fprintf(f, ", ");
        }
      }

      fprintf(f, ")\n");
      fprintf(f, "end\n\n");
      fprintf(f, "export %s\n\n", node->name);
    }
  }

  fprintf(f, "end # module\n");
  fclose(f);
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_ffi_emit_julia(cdd_ffi_ir_t *ir,
                                 const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_julia_file(ir, config);
}
