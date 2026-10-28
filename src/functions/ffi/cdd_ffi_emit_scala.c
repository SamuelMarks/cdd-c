/**
 * @file cdd_ffi_emit_scala.c
 * @brief FFI binding emitter for Scala.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_scala.h"

#include "../../../include/c_cdd/safe_crt.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../cdd_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding Scala Native `unsafe` type
 * string.
 *
 * @param[in] t FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved Scala Native type
 * string.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t map_scala_type(cdd_ffi_type_t *t, const char **out_type) {
  static const cdd_ffi_type_map_table_t k_scala_types = {"Unit",
                                                         "CBool",
                                                         "CSignedChar",
                                                         "CUnsignedChar",
                                                         "CShort",
                                                         "CUnsignedShort",
                                                         "CInt",
                                                         "CUnsignedInt",
                                                         "CLongLong",
                                                         "CUnsignedLongLong",
                                                         "CFloat",
                                                         "CDouble",
                                                         "CString",
                                                         "Ptr[Byte]",
                                                         NULL,
                                                         "Ptr[Byte]",
                                                         1,
                                                         1,
                                                         1};
  return cdd_ffi_map_type_from_table(&k_scala_types, *t, out_type);
}

/**
 * @brief Emits Scala Native `.scala` `@extern` bindings for the given FFI IR.
 *
 * @param[in] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t cdd_ffi_emit_scala(cdd_ffi_ir_t *ir,
                                 const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name;
  const char *module_name;
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }
  lib_name = (config->library_name) ? config->library_name : "mylib";
  module_name = (config->module_name) ? config->module_name : "Bindings";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.scala", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.scala", config->output_dir,
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

  fprintf(f, "// Auto-generated Scala Native bindings for %s\n\n", lib_name);
  fprintf(f, "import scalanative.unsafe._\n\n");

  /* Create an object to hold the types */
  fprintf(f, "object %sTypes {\n", module_name);

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "  type %s = CInt\n", node->name);
      fprintf(f, "  object %s {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    final val %s: %s = %s\n", var->name, node->name,
                  var->value);
        } else {
          fprintf(f, "    final val %s: %s = %lu\n", var->name, node->name,
                  (unsigned long)j);
        }
      }
      fprintf(f, "  }\n\n");
    }
  }

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      /* Scala Native supports up to CStruct22 */
      if (node->fields_count > 0 && node->fields_count <= 22) {
        fprintf(f, "  type %s = CStruct%lu[", node->name,
                (unsigned long)node->fields_count);
        for (j = 0; j < node->fields_count; j++) {
          const char *st = NULL;
          map_scala_type(&node->fields[j].type, &st);
          fprintf(f, "%s%s", j > 0 ? ", " : "", st);
        }
        fprintf(f, "]\n");
      } else if (node->fields_count > 22) {
        fprintf(f,
                "  // Struct %s has more than 22 fields, Scala Native CStructN "
                "only supports up to 22.\n",
                node->name);
        fprintf(f, "  type %s = CArray[Byte, Nat._0] // Fallback mapping\n",
                node->name);
      } else {
        fprintf(f, "  type %s = CStruct0\n", node->name);
      }
    }
  }
  fprintf(f, "}\n\n");

  /* The actual extern object */
  fprintf(f, "import %sTypes._\n\n", module_name);
  fprintf(f, "@link(\"%s\")\n", lib_name);
  fprintf(f, "@extern\n");
  fprintf(f, "object %s {\n", module_name);

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_st = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "  def %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *st = NULL;
        map_scala_type(&node->fields[j].type, &st);
        fprintf(f, "%s%s: %s", j > 0 ? ", " : "", node->fields[j].name, st);
      }
      map_scala_type(&node->return_or_base_type, &ret_st);
      fprintf(f, "): %s = extern\n", ret_st);
    }
  }

  fprintf(f, "}\n");

  fclose(f);
  return CDD_C_SUCCESS;
}
