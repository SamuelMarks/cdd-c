/**
 * @file cdd_ffi_emit_php.c
 * @brief FFI binding emitter for Php.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_php.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

static cdd_c_error_t get_cdef_type(cdd_ffi_type_t type, const char **out_type) {
  static const cdd_ffi_type_map_table_t k_php_types = {
      "void",     "bool",    "int8_t",   "uint8_t", "int16_t",
      "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t",
      "float",    "double",  "char*",    "void*",   NULL,
      "int",      1,         0,          1};
  return cdd_ffi_map_type_from_table(&k_php_types, type, out_type);
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
emit_php_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name =
      (config->library_name) ? config->library_name : "mylib";
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
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "            struct %s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = NULL;
        get_cdef_type(node->fields[j].type, &ftype);
        if (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(f, "                struct %s %s;\n", ftype, fname);
        } else {
          fprintf(f, "                %s %s;\n", ftype, fname);
        }
      }
      fprintf(f, "            };\n");
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_ct = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      get_cdef_type(node->return_or_base_type, &ret_ct);
      fprintf(f, "            %s %s(", ret_ct, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *ct = NULL;
        get_cdef_type(node->fields[j].type, &ct);
        fprintf(f, "%s %s", ct,
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
      cdd_ffi_emit_trivia(f, node->leading_trivia);
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

cdd_c_error_t cdd_ffi_emit_php(cdd_ffi_ir_t *ir,
                               const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_php_file(ir, config);
}
