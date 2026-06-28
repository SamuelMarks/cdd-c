/* clang-format off */
#include "cdd_ffi_emit_haskell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_haskell_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "CString";
    }
    return "Ptr ()";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "()";
  case CDD_FFI_KIND_BOOL:
    return "CBool";
  case CDD_FFI_KIND_INT8:
    return "CChar";
  case CDD_FFI_KIND_UINT8:
    return "CUChar";
  case CDD_FFI_KIND_INT16:
    return "CShort";
  case CDD_FFI_KIND_UINT16:
    return "CUShort";
  case CDD_FFI_KIND_INT32:
    return "CInt";
  case CDD_FFI_KIND_UINT32:
    return "CUInt";
  case CDD_FFI_KIND_INT64:
    return "CLLong";
  case CDD_FFI_KIND_UINT64:
    return "CULLong";
  case CDD_FFI_KIND_FLOAT32:
    return "CFloat";
  case CDD_FFI_KIND_FLOAT64:
    return "CDouble";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "FunPtr ()";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "Ptr ()";
  case CDD_FFI_KIND_ENUM_REF:
    return "CInt";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "Ptr ()";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "Ptr ()";
  default:
    return "Ptr ()";
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
emit_haskell_file(cdd_ffi_ir_t *ir,
                  const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "MyLib";
  char module_name[256];

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
  f = fopen(filepath, "w");
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
      if (node->doc)
        fprintf(f, "-- | %s\n", node->doc);
      fprintf(f, "data %s\n\n", node->name);
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->doc)
        fprintf(f, "-- | %s\n", node->doc);
      fprintf(f, "foreign import ccall unsafe \"%s\" c_%s\n", node->name,
              node->name);
      fprintf(f, "  :: ");
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s -> ", get_haskell_type(node->fields[j].type));
      }
      fprintf(f, "IO (%s)\n\n", get_haskell_type(node->return_or_base_type));
    }
  }

  fclose(f);
  return CDD_C_SUCCESS;
}

enum cdd_c_error
cdd_ffi_emit_haskell(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_haskell_file(ir, config);
}
