/**
 * @file cdd_ffi_ir_templates.c
 * @brief Template instantiation routines for FFI IR.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/format_specifiers.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "functions/ffi/cdd_ffi_ir_internal.h"
#include "ffi/cdd_ffi_ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Instantiates templated types discovered in the FFI IR.
 *
 * @param ir Pointer to FFI IR structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
cdd_c_error_t instantiate_templates(cdd_ffi_ir_t *ir) {
  size_t i, j;
  size_t initial_count;
  if (!ir)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  initial_count = ir->nodes_count;
  for (i = 0; i < initial_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if ((node->kind == CDD_FFI_NODE_STRUCT ||
         node->kind == CDD_FFI_NODE_FUNCTION) &&
        node->fields) {
      for (j = 0; j < node->fields_count; j++) {
        if (node->fields[j].type.kind == CDD_FFI_KIND_TEMPLATE_STRUCT_REF) {
          size_t k;
          const char *base_name = node->fields[j].type.ref_name;
          if (!base_name)
            continue;
          if (strncmp(base_name, "struct ", 7) == 0)
            base_name += 7;
          else if (strncmp(base_name, "class ", 6) == 0)
            base_name += 6;

          for (k = 0; k < initial_count; k++) {
            if (strcmp(ir->nodes[k].name, base_name) == 0) {
              char inst_name[256];
              const char *arg_type_name = "unknown";
              size_t m;
              int already_inst = 0;
              cdd_ffi_ir_node_t *new_node = NULL;
              cdd_ffi_ir_node_t *base_struct;

              if (node->fields[j].type.template_args_count > 0) {
                if (node->fields[j].type.template_args[0].kind ==
                    CDD_FFI_KIND_INT32) {
                  arg_type_name = "int";
                } else if (node->fields[j].type.template_args[0].kind ==
                           CDD_FFI_KIND_FLOAT64) {
                  arg_type_name = "double";
                } else if (node->fields[j].type.template_args[0].kind ==
                           CDD_FFI_KIND_FLOAT32) {
                  arg_type_name = "float";
                } else if (node->fields[j].type.template_args[0].ref_name) {
                  arg_type_name =
                      node->fields[j].type.template_args[0].ref_name;
                }
              }
#if defined(_MSC_VER)
              sprintf_s(inst_name, sizeof(inst_name), "%s_%s", base_name,
                        arg_type_name);
#else
              CDD_SNPRINTF(inst_name, sizeof(inst_name), "%s_%s", base_name,
                           arg_type_name);
#endif

              for (m = 0; m < ir->nodes_count; m++) {
                if (strcmp(ir->nodes[m].name, inst_name) == 0) {
                  already_inst = 1;
                  break;
                }
              }

              if (!already_inst) {
                cdd_c_error_t rc_ex =
                    ir_add_node(ir, CDD_FFI_NODE_STRUCT, inst_name, &new_node);
                if (rc_ex != CDD_C_SUCCESS)
                  return rc_ex;

                base_struct = &ir->nodes[k];

                new_node->fields_count = base_struct->fields_count;
                if (new_node->fields_count > 0) {
                  new_node->fields = (cdd_ffi_field_t *)CDD_CALLOC(
                      new_node->fields_count, sizeof(cdd_ffi_field_t));
                  if (!new_node->fields) {
                    return CDD_C_ERROR_MEMORY;
                  }
                  {
                    size_t f;
                    for (f = 0; f < new_node->fields_count; f++) {
                      new_node->fields[f].name =
                          CDD_STRDUP(base_struct->fields[f].name);
                      if (base_struct->fields[f].type.ref_name &&
                          strlen(base_struct->fields[f].type.ref_name) == 1) {
                        new_node->fields[f].type.kind =
                            ir->nodes[i].fields[j].type.template_args[0].kind;
                      } else {
                        new_node->fields[f].type.kind =
                            base_struct->fields[f].type.kind;
                        if (base_struct->fields[f].type.ref_name)
                          new_node->fields[f].type.ref_name =
                              CDD_STRDUP(base_struct->fields[f].type.ref_name);
                      }
                    }
                  }
                }
              }

              node = &ir->nodes[i];
              node->fields[j].type.kind = CDD_FFI_KIND_STRUCT_REF;
              free(node->fields[j].type.ref_name);
              node->fields[j].type.ref_name = CDD_STRDUP(inst_name);
              break;
            }
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT
cdd_c_error_t cdd_ffi_instantiate_templates_test(cdd_ffi_ir_t *ir) {
  return instantiate_templates(ir);
}
#endif /* CDD_BUILD_TESTS */
