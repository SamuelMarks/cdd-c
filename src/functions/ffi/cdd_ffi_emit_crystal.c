/**
 * @file cdd_ffi_emit_crystal.c
 * @brief FFI binding emitter for Crystal.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_crystal.h"

#include "../../../include/c_cdd/safe_crt.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../cdd_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding Crystal `LibC` type string.
 *
 * @param[in] t FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved Crystal type
 * string.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t map_crystal_type(cdd_ffi_type_t *t,
                                      const char **out_type) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      if (t->is_const) {
        *out_type = "Pointer(LibC::Char)"; /* or Pointer(UInt8) */
        return CDD_C_SUCCESS;
      }
    }
    *out_type = "Pointer(Void)";
    return CDD_C_SUCCESS;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = "Void";
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = "Bool";
    break;
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
    *out_type = "LibC::Char"; /* or Int8/UInt8 */
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = "LibC::Short";
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = "LibC::UShort";
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = "LibC::Int";
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = "LibC::UInt";
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = "LibC::LongLong";
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = "LibC::ULongLong";
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = "LibC::Float";
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = "LibC::Double";
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      *out_type = t->ref_name;
    } else {
      *out_type = "Pointer(Void)";
    }
    break;
  default:
    *out_type = "Pointer(Void)";
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Crystal `lib` bindings for the given FFI IR.
 *
 * @param[in] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t
cdd_ffi_emit_crystal(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name =
      config ? ((config->library_name) ? config->library_name : "mylib")
             : "mylib";
  const char *module_name =
      config ? ((config->module_name) ? config->module_name : "MyLib")
             : "MyLib";
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.cr", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.cr", config->output_dir,
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

  fprintf(f, "# Auto-generated Crystal lib bindings for %s\n\n", lib_name);
  fprintf(f, "@[Link(\"%s\")]\n", lib_name);
  fprintf(f, "lib %s\n", module_name);

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "  struct %s\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *ct = NULL;
        map_crystal_type(&node->fields[j].type, &ct);
        fprintf(f, "    %s : %s\n", node->fields[j].name, ct);
      }
      fprintf(f, "  end\n\n");
    } else if (node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "  union %s\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *ct = NULL;
        map_crystal_type(&node->fields[j].type, &ct);
        fprintf(f, "    %s : %s\n", node->fields[j].name, ct);
      }
      fprintf(f, "  end\n\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "  enum %s\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s\n", var->name, var->value);
        } else {
          fprintf(f, "    %s\n", var->name);
        }
      }
      fprintf(f, "  end\n\n");
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_ct = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "  fun %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *ct = NULL;
        map_crystal_type(&node->fields[j].type, &ct);
        fprintf(f, "%s%s : %s", j > 0 ? ", " : "", node->fields[j].name, ct);
      }
      map_crystal_type(&node->return_or_base_type, &ret_ct);
      fprintf(f, ") : %s\n", ret_ct);
    }
  }

  fprintf(f, "end\n\n");

  /* Optional idiomatic wrapper generator logic could go here */

  fclose(f);
  return CDD_C_SUCCESS;
}
