/* clang-format off */
#include "cdd_ffi_emit_fortran.h"

#include "../../cdd_api.h"
#include "../../win_compat_sym.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static void map_fortran_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz,
                             int is_return) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      CDD_SNPRINTF(out_type, out_sz,
                   "type(c_ptr)"); /* C String as opaque pointer */
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, "type(c_ptr)");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    if (!is_return) {
      CDD_SNPRINTF(out_type, out_sz,
                   "type(c_ptr)"); /* Generic placeholder if needed */
    } else {
      if (out_sz > 0) {
        out_type[0] = '\0'; /* Handled by SUBROUTINE instead of FUNCTION */
      }
    }
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz, "logical(c_bool)");
    break;
  case CDD_FFI_KIND_INT8:
    CDD_SNPRINTF(out_type, out_sz, "integer(c_int8_t)");
    break;
  case CDD_FFI_KIND_UINT8:
    CDD_SNPRINTF(
        out_type, out_sz,
        "integer(c_int8_t)"); /* Fortran doesn't really have unsigned */
    break;
  case CDD_FFI_KIND_INT16:
    CDD_SNPRINTF(out_type, out_sz, "integer(c_int16_t)");
    break;
  case CDD_FFI_KIND_UINT16:
    CDD_SNPRINTF(out_type, out_sz, "integer(c_int16_t)");
    break;
  case CDD_FFI_KIND_INT32:
    CDD_SNPRINTF(out_type, out_sz, "integer(c_int32_t)");
    break;
  case CDD_FFI_KIND_UINT32:
    CDD_SNPRINTF(out_type, out_sz, "integer(c_int32_t)");
    break;
  case CDD_FFI_KIND_INT64:
    CDD_SNPRINTF(out_type, out_sz, "integer(c_int64_t)");
    break;
  case CDD_FFI_KIND_UINT64:
    CDD_SNPRINTF(out_type, out_sz, "integer(c_int64_t)");
    break;
  case CDD_FFI_KIND_FLOAT32:
    CDD_SNPRINTF(out_type, out_sz, "real(c_float)");
    break;
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, "real(c_double)");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "type(%s)", t->ref_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "type(c_ptr)");
    }
    break;
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz,
                   "integer(c_int)"); /* Enums usually map to C int */
    } else {
      CDD_SNPRINTF(out_type, out_sz, "type(c_ptr)");
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, "type(c_ptr)");
    break;
  }
}

int cdd_ffi_emit_fortran(cdd_ffi_ir_t *ir,
                         const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name;
  const char *module_name;
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_enum_variant_t *var;
  char type_str[256];
  char ret_type_str[256];
  int is_void;

  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  lib_name = config->library_name ? config->library_name : "mylib";
  module_name = config->module_name ? config->module_name : "mylib_bindings";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.f90", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.f90", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "! Auto-generated Fortran (ISO_C_BINDING) bindings for %s\n",
          lib_name);
  fprintf(f, "MODULE %s\n", module_name);
  fprintf(f, "  USE, INTRINSIC :: ISO_C_BINDING\n");
  fprintf(f, "  IMPLICIT NONE\n\n");

  /* Structs, Unions, Enums, Typedefs */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "  TYPE, BIND(C) :: %s\n", node->name);
      if (node->kind == CDD_FFI_NODE_UNION) {
        /* Fortran doesn't have C-style unions, standard trick is a byte array
         * of max size, using c_ptr here for simplicity */
        fprintf(f, "    TYPE(c_ptr) :: data\n");
      } else {
        for (j = 0; j < node->fields_count; j++) {
          map_fortran_type(&node->fields[j].type, type_str, sizeof(type_str),
                           0);
          fprintf(f, "    %s :: %s\n", type_str, node->fields[j].name);
        }
      }
      fprintf(f, "  END TYPE %s\n\n", node->name);
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      /* Emit as integer parameters */
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        if (var->value) {
          fprintf(f, "  INTEGER(C_INT), PARAMETER :: %s = %s\n", var->name,
                  var->value);
        } else {
          fprintf(f,
                  "  INTEGER(C_INT), PARAMETER :: %s = %" CDD_SIZE_T_FMT "\n",
                  var->name, j);
        }
      }
      fprintf(f, "\n");
    }
  }

  fprintf(f, "  INTERFACE\n");

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      is_void = (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
                 node->return_or_base_type.pointer_depth == 0);

      if (is_void) {
        fprintf(f, "    SUBROUTINE %s(", node->name);
      } else {
        fprintf(f, "    FUNCTION %s(", node->name);
      }

      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s%s", node->fields[j].name,
                j + 1 < node->fields_count ? ", " : "");
      }
      fprintf(f, ") BIND(C, NAME=\"%s\")\n", node->name);
      fprintf(f, "      IMPORT\n");

      for (j = 0; j < node->fields_count; j++) {
        map_fortran_type(&node->fields[j].type, type_str, sizeof(type_str), 0);
        /* FFI generally implies pass by value unless it's a pointer in C */
        if (node->fields[j].type.pointer_depth > 0) {
          fprintf(f, "      %s, VALUE :: %s\n", type_str, node->fields[j].name);
        } else {
          fprintf(f, "      %s, VALUE :: %s\n", type_str, node->fields[j].name);
        }
      }

      if (!is_void) {
        map_fortran_type(&node->return_or_base_type, ret_type_str,
                         sizeof(ret_type_str), 1);
        fprintf(f, "      %s :: %s\n", ret_type_str, node->name);
      }

      if (is_void) {
        fprintf(f, "    END SUBROUTINE %s\n\n", node->name);
      } else {
        fprintf(f, "    END FUNCTION %s\n\n", node->name);
      }
    }
  }

  fprintf(f, "  END INTERFACE\n\n");
  fprintf(f, "END MODULE %s\n", module_name);

  fclose(f);
  return 0;
}
