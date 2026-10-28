/**
 * @file cdd_ffi_emit_ocaml.c
 * @brief FFI binding emitter for Ocaml.
 */
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_ocaml.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding OCaml `Ctypes` type string.
 *
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved OCaml `Ctypes`
 * string.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t get_ocaml_type(cdd_ffi_type_t type,
                                    const char **out_type) {
  static const cdd_ffi_type_map_table_t k_ocaml_types = {
      "void",    "bool",       "int8_t",  "uint8_t",  "int16_t", "uint16_t",
      "int32_t", "uint32_t",   "int64_t", "uint64_t", "float",   "double",
      "string",  "(ptr void)", NULL,      "int",      1,         0,
      1};
  return cdd_ffi_map_type_from_table(&k_ocaml_types, type, out_type);
}

/**
 * @brief Emits an OCaml `.ml` source file containing `Ctypes` structure and
 * foreign function bindings for the given FFI IR.
 *
 * @param[in] ir The extracted and topologically sorted FFI IR.
 * @param[in] config The bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t
emit_ocaml_ml(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.ml", config->output_dir,
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
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.ml", config->output_dir,
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

  fprintf(f, "(* Auto-generated OCaml CTypes bindings for %s *)\n\n", lib_name);
  fprintf(f, "open Ctypes\n");
  fprintf(f, "open Foreign\n\n");

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "(* %s *)\n", node->doc);
      fprintf(f, "module %s = struct\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "  let %s = %s\n", var->name, var->value);
        } else {
          fprintf(f, "  let %s = %lu\n", var->name, (unsigned long)j);
        }
      }
      fprintf(f, "end\n\n");
    }
  }

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "(* %s *)\n", node->doc);
      fprintf(f, "type %s\n", node->name);
      fprintf(f, "let %s : %s structure typ = structure \"%s\"\n", node->name,
              node->name, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = NULL;
        get_ocaml_type(node->fields[j].type, &ftype);
        fprintf(f, "let %s_%s = field %s \"%s\" %s\n", node->name, fname,
                node->name, fname, ftype);
      }
      fprintf(f, "let () = seal %s\n\n", node->name);
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_ot = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "(* %s *)\n", node->doc);
      fprintf(f, "let %s = foreign \"%s\" (", node->name, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *ot = NULL;
        get_ocaml_type(node->fields[j].type, &ot);
        fprintf(f, "%s @-> ", ot);
      }
      get_ocaml_type(node->return_or_base_type, &ret_ot);
      fprintf(f, "returning %s)\n\n", ret_ot);
    }
  }

  fclose(f);
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits OCaml Ctypes FFI bindings for the given FFI IR.
 *
 * @param[in] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t cdd_ffi_emit_ocaml(cdd_ffi_ir_t *ir,
                                 const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_ocaml_ml(ir, config);
}
