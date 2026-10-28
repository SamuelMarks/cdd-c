/**
 * @file cdd_ffi_emit_haskell.c
 * @brief FFI binding emitter for Haskell.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_haskell.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding Haskell `Foreign.C.Types`
 * type string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved Haskell type
 * string.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t get_haskell_type(cdd_ffi_type_t type,
                                      const char **out_type) {
  static const cdd_ffi_type_map_table_t k_haskell_types = {
      "()",      "CBool",   "CChar",   "CUChar", "CShort",
      "CUShort", "CInt",    "CUInt",   "CLLong", "CULLong",
      "CFloat",  "CDouble", "CString", "Ptr ()", "FunPtr ()",
      "CInt",    1,         0,         1};
  return cdd_ffi_map_type_from_table(&k_haskell_types, type, out_type);
}

/**
 * @brief Converts a `snake_case` C identifier into a `PascalCase` Haskell
 * module name.
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
 * @brief Emits a `.hs` Haskell source file containing `foreign import ccall`
 * declarations for the given FFI IR.
 *
 * @param[in] ir The extracted and topologically sorted FFI IR.
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t
emit_haskell_file(cdd_ffi_ir_t *ir,
                  const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = "MyLib";
  char module_name[256];

  if (config->library_name) {
    lib_name = config->library_name;
  }

  to_camel_case(lib_name, module_name, sizeof(module_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.hs", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.hs", config->output_dir,
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

  fprintf(f, "-- Auto-generated Haskell bindings for %s\n\n", lib_name);
  fprintf(f, "{-# LANGUAGE ForeignFunctionInterface #-}\n\n");
  fprintf(f, "module %s where\n\n", module_name);
  fprintf(f, "import Foreign\n");
  fprintf(f, "import Foreign.C.Types\n");
  fprintf(f, "import Foreign.C.String\n\n");

  /* Structs - Haskell usually treats structs as opaque pointers unless Storable
   * is implemented. For this generator, we will define empty data decls for
   * type safety. */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "-- | %s\n", node->doc);
      fprintf(f, "data %s\n\n", node->name);
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_ht = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "-- | %s\n", node->doc);
      fprintf(f, "foreign import ccall unsafe \"%s\" c_%s\n", node->name,
              node->name);
      fprintf(f, "  :: ");
      for (j = 0; j < node->fields_count; j++) {
        const char *ht = NULL;
        get_haskell_type(node->fields[j].type, &ht);
        fprintf(f, "%s -> ", ht);
      }
      get_haskell_type(node->return_or_base_type, &ret_ht);
      fprintf(f, "IO (%s)\n\n", ret_ht);
    }
  }

  fclose(f);
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Haskell FFI bindings for the given FFI IR.
 *
 * @param[in] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t
cdd_ffi_emit_haskell(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_haskell_file(ir, config);
}
