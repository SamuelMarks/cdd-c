/**
 * @file cdd_ffi_emit_go.c
 * @brief FFI binding emitter for Go.
 */
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_go.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding Go language type string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved Go type string.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t get_go_type(cdd_ffi_type_t type, const char **out_type) {
  static const cdd_ffi_type_map_table_t k_go_types = {
      "",        "bool",           "int8",  "uint8",  "int16",   "uint16",
      "int32",   "uint32",         "int64", "uint64", "float32", "float64",
      "*C.char", "unsafe.Pointer", NULL,    "int32",  1,         0,
      1};
  return cdd_ffi_map_type_from_table(&k_go_types, type, out_type);
}

/**
 * @brief Maps an FFI IR type to its corresponding cgo (`C.*`) type string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved cgo type string.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t get_go_c_type(cdd_ffi_type_t type, const char **out_type) {
  static const cdd_ffi_type_map_table_t k_go_c_types = {"",
                                                        "C.bool",
                                                        "C.int8_t",
                                                        "C.uint8_t",
                                                        "C.int16_t",
                                                        "C.uint16_t",
                                                        "C.int32_t",
                                                        "C.uint32_t",
                                                        "C.int64_t",
                                                        "C.uint64_t",
                                                        "C.float",
                                                        "C.double",
                                                        "*C.char",
                                                        "unsafe.Pointer",
                                                        NULL,
                                                        "C.int",
                                                        1,
                                                        0,
                                                        1};
  return cdd_ffi_map_type_from_table(&k_go_c_types, type, out_type);
}

/**
 * @brief Emits a `.go` source file containing cgo wrappers for the given FFI
 * IR.
 *
 * @param[in] ir The extracted and topologically sorted FFI IR.
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t
emit_go_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.go", config->output_dir,
               lib_name);
  {
    int err = 0;
#ifdef CDD_BUILD_TESTS
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      err = 1;
    } else
#endif
    {
      err = fopen_s(&f, filepath, "w");
    }
    if (err != 0) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.go", config->output_dir,
               lib_name);
#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      f = NULL;
    } else
#endif
    {
#if defined(_MSC_VER)
      if (fopen_s(&f, filepath, "w") != 0)
        f = NULL;
#else
    f = fopen(filepath, "w");
#endif
    }
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
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
      cdd_ffi_emit_trivia(f, node->leading_trivia);
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
          fprintf(f, "    %s %s = %lu\n", var->name, node->name,
                  (unsigned long)j);
        }
      }
      fprintf(f, ")\n\n");
    }
  }

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "// %s\n", node->doc);
      fprintf(f, "type %s struct {\n", node->name);
      fprintf(f, "    inner C.struct_%s\n", node->name);
      fprintf(f, "}\n\n");

      /* Accessors */
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *gtype = NULL;
        get_go_type(node->fields[j].type, &gtype);
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
      const char *ret_gtype = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "// %s\n", node->doc);
      fprintf(f, "func %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        const char *gt = NULL;
        if (strcmp(arg_name, "type") == 0)
          arg_name = "type_";
        if (strcmp(arg_name, "func") == 0)
          arg_name = "func_";
        get_go_type(node->fields[j].type, &gt);
        fprintf(f, "%s %s", arg_name, gt);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      get_go_type(node->return_or_base_type, &ret_gtype);
      fprintf(f, ") %s {\n", ret_gtype);

      fprintf(f, "    ");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "return (%s)(", ret_gtype);
      }
      fprintf(f, "C.%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        const char *gct = NULL;
        if (strcmp(arg_name, "type") == 0)
          arg_name = "type_";
        if (strcmp(arg_name, "func") == 0)
          arg_name = "func_";
        get_go_c_type(node->fields[j].type, &gct);
        fprintf(f, "(%s)(%s)", gct, arg_name);
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
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a `go.mod` module manifest file in `config->output_dir`.
 *
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t emit_go_mod(const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\go.mod", config->output_dir);
  {
    int err = 0;
#ifdef CDD_BUILD_TESTS
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      err = 1;
    } else
#endif
    {
      err = fopen_s(&f, filepath, "w");
    }
    if (err != 0) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/go.mod", config->output_dir);
#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      f = NULL;
    } else
#endif
    {
#if defined(_MSC_VER)
      if (fopen_s(&f, filepath, "w") != 0)
        f = NULL;
#else
    f = fopen(filepath, "w");
#endif
    }
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "module %s\n\n",
          config->library_name ? config->library_name : "mylib");
  fprintf(f, "go 1.20\n");

  fclose(f);
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Go (cgo) bindings and `go.mod` manifest for the given FFI IR.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t cdd_ffi_emit_go(cdd_ffi_ir_t *ir,
                              const cdd_generate_bindings_config_t *config) {
  cdd_c_error_t rc;
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  rc = emit_go_file(ir, config);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return emit_go_mod(config);
}
