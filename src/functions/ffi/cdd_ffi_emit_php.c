/* clang-format off */
#include "cdd_ffi_emit_php.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_cdef_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "char*";
    }
    return "void*";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "bool";
  case CDD_FFI_KIND_INT8:
    return "int8_t";
  case CDD_FFI_KIND_UINT8:
    return "uint8_t";
  case CDD_FFI_KIND_INT16:
    return "int16_t";
  case CDD_FFI_KIND_UINT16:
    return "uint16_t";
  case CDD_FFI_KIND_INT32:
    return "int32_t";
  case CDD_FFI_KIND_UINT32:
    return "uint32_t";
  case CDD_FFI_KIND_INT64:
    return "int64_t";
  case CDD_FFI_KIND_UINT64:
    return "uint64_t";
  case CDD_FFI_KIND_FLOAT32:
    return "float";
  case CDD_FFI_KIND_FLOAT64:
    return "double";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "void*";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "void*";
  case CDD_FFI_KIND_ENUM_REF:
    return "int";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "void*";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "void*";
  default:
    return "void*";
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

static enum cdd_c_error
emit_php_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";
  char class_name[256];

  to_camel_case(lib_name, class_name, sizeof(class_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.php", config->output_dir,
               class_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.php", config->output_dir,
               class_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "<?php\n");
  fprintf(f, "// Auto-generated PHP FFI bindings for %s\n\n", lib_name);
  fprintf(f, "class %s {\n", class_name);
  fprintf(f, "    private \\FFI $ffi;\n\n");

  fprintf(f,
          "    public function __construct(string $libraryPath = '%s.so') {\n",
          lib_name);
  fprintf(f, "        $cdef = \"\n");

  /* C definitions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      fprintf(f, "            struct %s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = get_cdef_type(node->fields[j].type);
        if (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(f, "                struct %s %s;\n", ftype, fname);
        } else {
          fprintf(f, "                %s %s;\n", ftype, fname);
        }
      }
      fprintf(f, "            };\n");
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "            %s %s(", get_cdef_type(node->return_or_base_type),
              node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s %s", get_cdef_type(node->fields[j].type),
                node->fields[j].name ? node->fields[j].name : "arg");
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ");\n");
    }
  }

  fprintf(f, "        \";\n");
  fprintf(f, "        $this->ffi = \\FFI::cdef($cdef, $libraryPath);\n");
  fprintf(f, "    }\n\n");

  /* Function Wrappers */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->doc)
        fprintf(f, "    /**\n     * %s\n     */\n", node->doc);
      fprintf(f, "    public function %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        /* avoid php keywords */
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        fprintf(f, "$%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ") {\n");

      fprintf(f, "        return $this->ffi->%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        fprintf(f, "$%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ");\n");
      fprintf(f, "    }\n\n");
    }
  }

  fprintf(f, "}\n");
  fclose(f);
  return CDD_C_SUCCESS;
}

enum cdd_c_error
cdd_ffi_emit_php(cdd_ffi_ir_t *ir,
                 const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_php_file(ir, config);
}
