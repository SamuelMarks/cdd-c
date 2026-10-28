/**
 * @file cdd_ffi_emit_ruby.c
 * @brief FFI binding emitter for Ruby.
 */
#ifdef CDD_BUILD_TESTS
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_ruby.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/safe_crt.h"
#include "cdd_ffi_emit_common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding Ruby `Fiddle` type constant
 * string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved `Fiddle` type
 * string.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t get_ruby_type(cdd_ffi_type_t type, const char **out_type) {
  static const cdd_ffi_type_map_table_t k_ruby_types = {
      "Fiddle::TYPE_VOID",
      "Fiddle::TYPE_CHAR",
      "Fiddle::TYPE_CHAR",
      "-Fiddle::TYPE_CHAR",
      "Fiddle::TYPE_SHORT",
      "-Fiddle::TYPE_SHORT",
      "Fiddle::TYPE_INT",
      "-Fiddle::TYPE_INT",
      "Fiddle::TYPE_LONG_LONG",
      "-Fiddle::TYPE_LONG_LONG",
      "Fiddle::TYPE_FLOAT",
      "Fiddle::TYPE_DOUBLE",
      "Fiddle::TYPE_VOIDP",
      "Fiddle::TYPE_VOIDP",
      NULL,
      "Fiddle::TYPE_INT",
      0,
      0,
      0};
  return cdd_ffi_map_type_from_table(&k_ruby_types, type, out_type);
}

/**
 * @brief Converts a `snake_case` C identifier into a `PascalCase` Ruby module
 * name.
 *
 * @param[in] snake Source `snake_case` string.
 * @param[out] out Destination buffer for the PascalCase string.
 * @param[in] out_size Capacity of `out` in bytes.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t to_camel_case(const char *snake, char *out,
                                   size_t out_size) {
  size_t i, j = 0;
  int up = 1;
  for (i = 0; i < strlen(snake) && j < out_size - 1; i++) {
    if (snake[i] == '_') {
      up = 1;
    } else {
      if (up) {
        out[j++] = (char)toupper((unsigned char)snake[i]);
        up = 0;
      } else {
        out[j++] = snake[i];
      }
    }
  }
  out[j] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a `.rb` Ruby `Fiddle` bindings module, C GVL extension wrapper,
 * and optional test suite for the given FFI IR.
 *
 * @param[in] ir The extracted and topologically sorted FFI IR.
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t
emit_ruby_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name =
      (config->library_name) ? config->library_name : "mylib";
  char module_name[256];

  to_camel_case(lib_name, module_name, sizeof(module_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.rb", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.rb", config->output_dir,
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
        const char *ftype = NULL;
        get_ruby_type(node->fields[j].type, &ftype);
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
          fprintf(f, "    %s = %lu\n", var->name, (unsigned long)j);
        }
      }
      fprintf(f, "  end\n\n");
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_rt = NULL;
      if (node->doc)
        fprintf(f, "  # %s\n", node->doc);
      get_ruby_type(node->return_or_base_type, &ret_rt);
      fprintf(f, "  extern '%s %s(", ret_rt, node->name);

      for (j = 0; j < node->fields_count; j++) {
        const char *rt = NULL;
        get_ruby_type(node->fields[j].type, &rt);
        fprintf(f, "%s", rt);
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
    FILE *fc = NULL;
    char filepath_c[1024];
#if defined(_MSC_VER)
    sprintf_s(filepath_c, sizeof(filepath_c), "%s\\cdd_ruby_wrap.c",
              config->output_dir);
    fopen_s(&fc, filepath_c, "w");
#else
    CDD_SNPRINTF(filepath_c, sizeof(filepath_c), "%s/cdd_ruby_wrap.c",
                 config->output_dir);
#if defined(_MSC_VER)
    if (fopen_s(&fc, filepath_c, "w") != 0)
      fc = NULL;
#else
    fc = fopen(filepath_c, "w");
#endif
#endif
#ifdef CDD_BUILD_TESTS
    {
      if (g_fail_io_after == 2) {
        fclose(fc);
        fc = NULL;
      }
    }
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
#if defined(_MSC_VER)
    if (fopen_s(&f, filepath_test, "w") != 0)
      f = NULL;
#else
    f = fopen(filepath_test, "w");
#endif
#endif
#ifdef CDD_BUILD_TESTS
    {
      if (g_fail_io_after == 3) {
        fclose(f);
        f = NULL;
      }
    }
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

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Ruby Fiddle FFI bindings for the given FFI IR.
 *
 * @param[in] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t cdd_ffi_emit_ruby(cdd_ffi_ir_t *ir,
                                const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_ruby_file(ir, config);
}
