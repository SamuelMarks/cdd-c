/* clang-format off */
#include "cdd_ffi_emit_ruby.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_ruby_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "Fiddle::TYPE_VOIDP"; /* Usually treated as string or ptr in fiddle
                                    */
    }
    return "Fiddle::TYPE_VOIDP";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "Fiddle::TYPE_VOID";
  case CDD_FFI_KIND_BOOL:
    return "Fiddle::TYPE_CHAR"; /* Ruby usually treats bool as char/int */
  case CDD_FFI_KIND_INT8:
    return "Fiddle::TYPE_CHAR";
  case CDD_FFI_KIND_UINT8:
    return "-Fiddle::TYPE_CHAR";
  case CDD_FFI_KIND_INT16:
    return "Fiddle::TYPE_SHORT";
  case CDD_FFI_KIND_UINT16:
    return "-Fiddle::TYPE_SHORT";
  case CDD_FFI_KIND_INT32:
    return "Fiddle::TYPE_INT";
  case CDD_FFI_KIND_UINT32:
    return "-Fiddle::TYPE_INT";
  case CDD_FFI_KIND_INT64:
    return "Fiddle::TYPE_LONG_LONG";
  case CDD_FFI_KIND_UINT64:
    return "-Fiddle::TYPE_LONG_LONG";
  case CDD_FFI_KIND_FLOAT32:
    return "Fiddle::TYPE_FLOAT";
  case CDD_FFI_KIND_FLOAT64:
    return "Fiddle::TYPE_DOUBLE";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "Fiddle::TYPE_VOIDP";
  case CDD_FFI_KIND_STRUCT_REF:
    return "Fiddle::TYPE_VOIDP"; /* Structs by value are complex in Fiddle,
                                    fallback to ptr for now */
  case CDD_FFI_KIND_ENUM_REF:
    return "Fiddle::TYPE_INT";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return "Fiddle::TYPE_VOIDP";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "Fiddle::TYPE_VOIDP";
  default:
    return "Fiddle::TYPE_VOIDP";
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

static int emit_ruby_file(cdd_ffi_ir_t *ir,
                          const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";
  char module_name[256];

  to_camel_case(lib_name, module_name, sizeof(module_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.rb", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.rb", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "# Auto-generated Ruby Fiddle bindings for %s\n\n", lib_name);
  fprintf(f, "require 'fiddle'\n");
  fprintf(f, "require 'fiddle/import'\n\n");

  fprintf(f, "module %s\n", module_name);
  fprintf(f, "  extend Fiddle::Importer\n");
  fprintf(f,
          "  dlload '%s.so' # Adjust extension for platform (.dll/.dylib)\n\n",
          lib_name);

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      if (node->doc)
        fprintf(f, "  # %s\n", node->doc);
      fprintf(f, "  %s = struct([\n", node->name);

      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = get_ruby_type(node->fields[j].type);
        fprintf(f, "    \"%s %s\"", ftype, fname);
        if (j < node->fields_count - 1)
          fprintf(f, ",");
        fprintf(f, "\n");
      }
      fprintf(f, "  ])\n\n");
    }
  }

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      if (node->doc)
        fprintf(f, "  # %s\n", node->doc);
      fprintf(f, "  module %s\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->doc)
          fprintf(f, "    # %s\n", var->doc);
        if (var->value) {
          fprintf(f, "    %s = %s\n", var->name, var->value);
        } else {
          fprintf(f, "    %s = " CDD_PRIz "\n", var->name, j);
        }
      }
      fprintf(f, "  end\n\n");
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->doc)
        fprintf(f, "  # %s\n", node->doc);
      fprintf(f, "  extern '%s %s(", get_ruby_type(node->return_or_base_type),
              node->name);

      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s", get_ruby_type(node->fields[j].type));
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")'\n");
    }
  }

  fprintf(f, "end\n");
  fclose(f);

  /* Generate Ruby C Extension wrapper (cdd_ruby_wrap.c) for GVL handling */
  {
    FILE *fc;
    char filepath_c[1024];
#if defined(_MSC_VER)
    sprintf_s(filepath_c, sizeof(filepath_c), "%s\\cdd_ruby_wrap.c",
              config->output_dir);
    fopen_s(&fc, filepath_c, "w");
#else
    CDD_SNPRINTF(filepath_c, sizeof(filepath_c), "%s/cdd_ruby_wrap.c",
                 config->output_dir);
    fc = fopen(filepath_c, "w");
#endif
    if (fc) {
      fprintf(fc, "/* Auto-generated Ruby C Extension Wrapper */\n");
      fprintf(fc, "#include <ruby.h>\n");
      fprintf(fc, "#include <ruby/thread.h>\n\n");

      for (i = 0; i < ir->nodes_count; i++) {
        cdd_ffi_ir_node_t *node = &ir->nodes[i];
        if (node->kind == CDD_FFI_NODE_FUNCTION && node->requires_gil_release) {
          fprintf(fc, "static void* no_gvl_%s(void *data) {\n", node->name);
          fprintf(fc, "    /* %s(); */\n", node->name);
          fprintf(fc, "    return NULL;\n");
          fprintf(fc, "}\n\n");

          fprintf(fc, "static VALUE wrap_%s(VALUE self) {\n", node->name);
          fprintf(fc,
                  "    /* rb_thread_call_without_gvl(no_gvl_%s, NULL, "
                  "RUBY_UBF_IO, NULL); */\n",
                  node->name);
          fprintf(fc, "    return Qnil;\n");
          fprintf(fc, "}\n\n");
        }
      }
      fclose(fc);
    }
  }

  if (config->generate_tests) {
    char filepath_test[1024];
#if defined(_MSC_VER)
    sprintf_s(filepath_test, sizeof(filepath_test), "%s\\test_%s.rb",
              config->output_dir, lib_name);
    fopen_s(&f, filepath_test, "w");
#else
    CDD_SNPRINTF(filepath_test, sizeof(filepath_test), "%s/test_%s.rb",
                 config->output_dir, lib_name);
    f = fopen(filepath_test, "w");
#endif
    if (f) {
      fprintf(f, "# Auto-generated tests for %s\n", lib_name);
      fprintf(f, "require_relative '%s'\n\n", lib_name);
      for (i = 0; i < ir->nodes_count; i++) {
        cdd_ffi_ir_node_t *node = &ir->nodes[i];
        if (node->kind == CDD_FFI_NODE_FUNCTION) {
          int out_count = 0;
          for (j = 0; j < node->fields_count; j++) {
            if (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
                node->fields[j].intent == CDD_FFI_INTENT_INOUT)
              out_count++;
          }
          if (out_count > 0) {
            fprintf(f, "# Multiple return values test placeholder\n");
            fprintf(f, "# res = %s::%s(...)\n", module_name, node->name);
            fprintf(f,
                    "# raise \"Expected array\" unless res.is_a?(Array)\n\n");
          }
        }
      }
      fclose(f);
    }
  }

  return 0;
}

int cdd_ffi_emit_ruby(cdd_ffi_ir_t *ir,
                      const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  return emit_ruby_file(ir, config);
}
