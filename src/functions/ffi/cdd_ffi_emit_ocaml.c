/* clang-format off */
#include "cdd_ffi_emit_ocaml.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_ocaml_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "string";
    }
    return "(ptr void)";
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
    return "(ptr void)";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "(ptr void)";
  case CDD_FFI_KIND_ENUM_REF:
    return "int";
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "(ptr void)";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "(ptr void)";
  default:
    return "(ptr void)";
  }
}

static int emit_ocaml_ml(cdd_ffi_ir_t *ir,
                         const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.ml", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.ml", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "(* Auto-generated OCaml CTypes bindings for %s *)\n\n", lib_name);
  fprintf(f, "open Ctypes\n");
  fprintf(f, "open Foreign\n\n");

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      if (node->doc)
        fprintf(f, "(* %s *)\n", node->doc);
      fprintf(f, "module %s = struct\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "  let %s = %s\n", var->name, var->value);
        } else {
          fprintf(f, "  let %s = " CDD_PRIz "\n", var->name, j);
        }
      }
      fprintf(f, "end\n\n");
    }
  }

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      if (node->doc)
        fprintf(f, "(* %s *)\n", node->doc);
      fprintf(f, "type %s\n", node->name);
      fprintf(f, "let %s : %s structure typ = structure \"%s\"\n", node->name,
              node->name, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = get_ocaml_type(node->fields[j].type);
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
      if (node->doc)
        fprintf(f, "(* %s *)\n", node->doc);
      fprintf(f, "let %s = foreign \"%s\" (", node->name, node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s @-> ", get_ocaml_type(node->fields[j].type));
      }
      fprintf(f, "returning %s)\n\n",
              get_ocaml_type(node->return_or_base_type));
    }
  }

  fclose(f);
  return 0;
}

int cdd_ffi_emit_ocaml(cdd_ffi_ir_t *ir,
                       const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  return emit_ocaml_ml(ir, config);
}
