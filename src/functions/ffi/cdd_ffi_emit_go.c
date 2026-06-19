/* clang-format off */
#include "cdd_ffi_emit_go.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_go_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "*C.char";
    }
    return "unsafe.Pointer";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return ""; /* Go uses no return type for void */
  case CDD_FFI_KIND_BOOL:
    return "bool";
  case CDD_FFI_KIND_INT8:
    return "int8";
  case CDD_FFI_KIND_UINT8:
    return "uint8";
  case CDD_FFI_KIND_INT16:
    return "int16";
  case CDD_FFI_KIND_UINT16:
    return "uint16";
  case CDD_FFI_KIND_INT32:
    return "int32";
  case CDD_FFI_KIND_UINT32:
    return "uint32";
  case CDD_FFI_KIND_INT64:
    return "int64";
  case CDD_FFI_KIND_UINT64:
    return "uint64";
  case CDD_FFI_KIND_FLOAT32:
    return "float32";
  case CDD_FFI_KIND_FLOAT64:
    return "float64";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "unsafe.Pointer";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "unsafe.Pointer";
  case CDD_FFI_KIND_ENUM_REF:
    return "int32"; /* Default C enum size in Go */
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "unsafe.Pointer";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "unsafe.Pointer";
  default:
    return "unsafe.Pointer";
  }
}

static const char *get_go_c_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "*C.char";
    }
    return "unsafe.Pointer";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "";
  case CDD_FFI_KIND_BOOL:
    return "C.bool";
  case CDD_FFI_KIND_INT8:
    return "C.int8_t";
  case CDD_FFI_KIND_UINT8:
    return "C.uint8_t";
  case CDD_FFI_KIND_INT16:
    return "C.int16_t";
  case CDD_FFI_KIND_UINT16:
    return "C.uint16_t";
  case CDD_FFI_KIND_INT32:
    return "C.int32_t";
  case CDD_FFI_KIND_UINT32:
    return "C.uint32_t";
  case CDD_FFI_KIND_INT64:
    return "C.int64_t";
  case CDD_FFI_KIND_UINT64:
    return "C.uint64_t";
  case CDD_FFI_KIND_FLOAT32:
    return "C.float";
  case CDD_FFI_KIND_FLOAT64:
    return "C.double";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "unsafe.Pointer";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "unsafe.Pointer";
  case CDD_FFI_KIND_ENUM_REF:
    return "C.int";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "unsafe.Pointer";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "unsafe.Pointer";
  default:
    return "unsafe.Pointer";
  }
}

static int emit_go_file(cdd_ffi_ir_t *ir,
                        const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.go", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.go", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "// Auto-generated Go bindings for %s\n", lib_name);
  fprintf(f, "package %s\n\n", lib_name);

  fprintf(f, "/*\n");
  if (config->input) {
    fprintf(f, "#include \"%s\"\n", config->input);
  }
  fprintf(f, "*/\n");
  fprintf(f, "import \"C\"\n");
  fprintf(f, "import \"unsafe\"\n\n");

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      if (node->doc)
        fprintf(f, "// %s\n", node->doc);
      fprintf(f, "type %s int32\n", node->name);
      fprintf(f, "const (\n");
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->doc)
          fprintf(f, "    // %s\n", var->doc);
        if (var->value) {
          fprintf(f, "    %s %s = %s\n", var->name, node->name, var->value);
        } else {
          fprintf(f, "    %s %s = " CDD_PRIz "\n", var->name, node->name, j);
        }
      }
      fprintf(f, ")\n\n");
    }
  }

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      if (node->doc)
        fprintf(f, "// %s\n", node->doc);
      fprintf(f, "type %s struct {\n", node->name);
      fprintf(f, "    inner C.struct_%s\n", node->name);
      fprintf(f, "}\n\n");

      /* Accessors */
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *gtype = get_go_type(node->fields[j].type);
        fprintf(f, "func (s *%s) Get%s() %s {\n", node->name, fname, gtype);
        fprintf(f, "    return (%s)(s.inner.%s)\n", gtype, fname);
        fprintf(f, "}\n\n");
      }
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->doc)
        fprintf(f, "// %s\n", node->doc);
      fprintf(f, "func %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "type") == 0)
          arg_name = "type_";
        if (strcmp(arg_name, "func") == 0)
          arg_name = "func_";
        fprintf(f, "%s %s", arg_name, get_go_type(node->fields[j].type));
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ") %s {\n", get_go_type(node->return_or_base_type));

      fprintf(f, "    ");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "return (%s)(", get_go_type(node->return_or_base_type));
      }
      fprintf(f, "C.%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "type") == 0)
          arg_name = "type_";
        if (strcmp(arg_name, "func") == 0)
          arg_name = "func_";
        fprintf(f, "(%s)(%s)", get_go_c_type(node->fields[j].type), arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "))\n");
      } else {
        fprintf(f, ")\n");
      }
      fprintf(f, "}\n\n");
    }
  }

  fclose(f);
  return 0;
}

static int emit_go_mod(const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\go.mod", config->output_dir);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/go.mod", config->output_dir);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "module %s\n\n",
          config->library_name ? config->library_name : "mylib");
  fprintf(f, "go 1.20\n");

  fclose(f);
  return 0;
}

int cdd_ffi_emit_go(cdd_ffi_ir_t *ir,
                    const cdd_generate_bindings_config_t *config) {
  int rc;
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  rc = emit_go_file(ir, config);
  if (rc != 0)
    return rc;

  return emit_go_mod(config);
}
