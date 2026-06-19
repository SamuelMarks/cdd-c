/* clang-format off */
#include "cdd_ffi_emit_julia.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_julia_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "Cstring";
    }
    return "Ptr{Cvoid}";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "Cvoid";
  case CDD_FFI_KIND_BOOL:
    return "Cint"; /* Often bools are passed as ints in C FFI */
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
    return "Float32";
  case CDD_FFI_KIND_FLOAT64:
    return "Float64";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "Ptr{Cvoid}";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "Ptr{Cvoid}";
  case CDD_FFI_KIND_ENUM_REF:
    return "Cint";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "Ptr{Cvoid}";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "Ptr{Cvoid}";
  default:
    return "Ptr{Cvoid}";
  }
}

static void to_camel_case(const char *snake, char *out, size_t out_size) {
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
}

static int emit_julia_file(cdd_ffi_ir_t *ir,
                           const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";
  char module_name[256];

  to_camel_case(lib_name, module_name, sizeof(module_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.jl", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.jl", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "# Auto-generated Julia bindings for %s\n\n", lib_name);
  fprintf(f, "module %s\n\n", module_name);

  fprintf(f, "const lib_path = \"%s\"\n\n", lib_name);

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      if (node->doc)
        fprintf(f, "# %s\n", node->doc);
      fprintf(f, "@enum %s::Cint begin\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s\n", var->name, var->value);
        } else {
          fprintf(f, "    %s = " CDD_PRIz "\n", var->name, j);
        }
      }
      fprintf(f, "end\n\n");
    }
  }

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      if (node->doc)
        fprintf(f, "# %s\n", node->doc);
      fprintf(f, "mutable struct %s\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        fprintf(f, "    %s::%s\n", fname, get_julia_type(node->fields[j].type));
      }
      fprintf(f, "    %s() = new()\n", node->name);
      fprintf(f, "end\n\n");
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
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

      fprintf(f, "    ccall((:%s, lib_path), %s, (", node->name,
              get_julia_type(node->return_or_base_type));
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s", get_julia_type(node->fields[j].type));
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
  return 0;
}

int cdd_ffi_emit_julia(cdd_ffi_ir_t *ir,
                       const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  return emit_julia_file(ir, config);
}
