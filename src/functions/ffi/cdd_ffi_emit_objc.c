/* clang-format off */
#include "cdd_ffi_emit_objc.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

static const char *map_objc_type(cdd_ffi_type_t *t, int is_return) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      if (t->is_const) {
        return is_return ? "NSString *" : "NSString * _Nonnull";
      }
      return "char *";
    }
    if (t->kind == CDD_FFI_KIND_STRUCT_REF ||
        t->kind == CDD_FFI_KIND_TYPEDEF_REF ||
        t->kind == CDD_FFI_KIND_ENUM_REF) {
      if (t->ref_name) {
        return t->ref_name; /* assume it maps to an ObjC class pointer, usually
                               needs '*' added later */
      }
    }
    return "void *";
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "BOOL";
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
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      return t->ref_name;
    }
    return "void *";
  default:
    return "void *";
  }
}

static void capitalize_first(char *str) {
  if (str && str[0]) {
    str[0] = (char)toupper((unsigned char)str[0]);
  }
}

enum cdd_c_error
cdd_ffi_emit_objc(cdd_ffi_ir_t *ir,
                  const cdd_generate_bindings_config_t *config) {
  FILE *h_file = NULL;
  FILE *m_file = NULL;
  char h_filepath[1024];
  char m_filepath[1024];
  const char *lib_name = config->library_name ? config->library_name : "MyLib";
  const char *module_name =
      config->module_name ? config->module_name : "Bindings";
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(h_filepath, sizeof(h_filepath), "%s\\%s.h", config->output_dir,
               module_name);
  CDD_SNPRINTF(m_filepath, sizeof(m_filepath), "%s\\%s.m", config->output_dir,
               module_name);
  if (fopen_s(&h_file, h_filepath, "w") != 0)
    return CDD_C_ERROR_UNKNOWN;
  if (fopen_s(&m_file, m_filepath, "w") != 0) {
    fclose(h_file);
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(h_filepath, sizeof(h_filepath), "%s/%s.h", config->output_dir,
               module_name);
  CDD_SNPRINTF(m_filepath, sizeof(m_filepath), "%s/%s.m", config->output_dir,
               module_name);
  h_file = fopen(h_filepath, "w");
  if (!h_file)
    return CDD_C_ERROR_UNKNOWN;
  m_file = fopen(m_filepath, "w");
  if (!m_file) {
    fclose(h_file);
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  /* Header File (.h) */
  fprintf(h_file, "// Auto-generated Objective-C bindings for %s\n\n",
          lib_name);
  fprintf(h_file, "#import <Foundation/Foundation.h>\n\n");
  fprintf(h_file, "NS_ASSUME_NONNULL_BEGIN\n\n");

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(h_file, "typedef NS_ENUM(NSInteger, %s) {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(h_file, "    %s = %s,\n", var->name, var->value);
        } else {
          fprintf(h_file, "    %s,\n", var->name);
        }
      }
      fprintf(h_file, "};\n\n");
    }
  }

  /* Struct Wrappers (Interfaces) */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      fprintf(h_file, "@interface %s : NSObject\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *t_str = map_objc_type(&node->fields[j].type, 1);
        /* simplistic pointer handling for class refs */
        int is_obj = (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF ||
                      strstr(t_str, "NSString") != NULL);
        fprintf(h_file, "@property (nonatomic%s) %s%s%s;\n",
                is_obj ? ", strong" : ", assign", t_str,
                is_obj && !strstr(t_str, "*") ? " *" : " ",
                node->fields[j].name);
      }
      fprintf(h_file, "@end\n\n");
    }
  }

  /* Main Static Methods Interface */
  fprintf(h_file, "@interface %s : NSObject\n", module_name);
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_type = map_objc_type(&node->return_or_base_type, 1);
      int ret_is_obj =
          (node->return_or_base_type.kind == CDD_FFI_KIND_STRUCT_REF ||
           strstr(ret_type, "NSString") != NULL);

      fprintf(h_file, "+ (%s%s)%s", ret_type,
              ret_is_obj && !strstr(ret_type, "*") ? " *" : "", node->name);

      if (node->fields_count > 0) {
        for (j = 0; j < node->fields_count; j++) {
          const char *arg_type = map_objc_type(&node->fields[j].type, 0);
          int arg_is_obj =
              (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF ||
               strstr(arg_type, "NSString") != NULL);

          char arg_name_cap[256];
          CDD_STRNCPY(arg_name_cap, sizeof(arg_name_cap), node->fields[j].name,
                      sizeof(arg_name_cap) - 1);
          arg_name_cap[sizeof(arg_name_cap) - 1] = '\0';

          if (j > 0) {
            capitalize_first(arg_name_cap);
            fprintf(h_file, " %s:(%s%s)%s", node->fields[j].name, arg_type,
                    arg_is_obj && !strstr(arg_type, "*") ? " *" : "",
                    node->fields[j].name);
          } else {
            /* First arg format: + (void)doSomethingWithArg:(int)arg; */
            fprintf(h_file, "%s%s:(%s%s)%s",
                    node->fields_count > 1
                        ? "With"
                        : (strlen(node->name) > 0 ? "With" : ""),
                    node->fields_count > 1 ? arg_name_cap : arg_name_cap,
                    arg_type, arg_is_obj && !strstr(arg_type, "*") ? " *" : "",
                    node->fields[j].name);
          }
        }
      }
      fprintf(h_file, ";\n");
    }
  }
  fprintf(h_file, "@end\n\n");
  fprintf(h_file, "NS_ASSUME_NONNULL_END\n");
  fclose(h_file);

  /* Implementation File (.m) */
  fprintf(m_file,
          "// Auto-generated Objective-C bindings implementation for %s\n\n",
          lib_name);
  fprintf(m_file, "#import \"%s.h\"\n", module_name);
  /* we'd normally import the underlying C header here */
  fprintf(m_file, "// #import \"%s_internal.h\"\n\n", lib_name);

  /* Struct Impls */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      fprintf(m_file, "@implementation %s\n@end\n\n", node->name);
    }
  }

  /* Main Static Methods Impl */
  fprintf(m_file, "@implementation %s\n\n", module_name);
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_type = map_objc_type(&node->return_or_base_type, 1);
      int ret_is_obj =
          (node->return_or_base_type.kind == CDD_FFI_KIND_STRUCT_REF ||
           strstr(ret_type, "NSString") != NULL);

      fprintf(m_file, "+ (%s%s)%s", ret_type,
              ret_is_obj && !strstr(ret_type, "*") ? " *" : "", node->name);

      if (node->fields_count > 0) {
        for (j = 0; j < node->fields_count; j++) {
          const char *arg_type = map_objc_type(&node->fields[j].type, 0);
          int arg_is_obj =
              (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF ||
               strstr(arg_type, "NSString") != NULL);

          char arg_name_cap[256];
          CDD_STRNCPY(arg_name_cap, sizeof(arg_name_cap), node->fields[j].name,
                      sizeof(arg_name_cap) - 1);
          arg_name_cap[sizeof(arg_name_cap) - 1] = '\0';

          if (j > 0) {
            capitalize_first(arg_name_cap);
            fprintf(m_file, " %s:(%s%s)%s", node->fields[j].name, arg_type,
                    arg_is_obj && !strstr(arg_type, "*") ? " *" : "",
                    node->fields[j].name);
          } else {
            fprintf(m_file, "%s%s:(%s%s)%s",
                    node->fields_count > 1
                        ? "With"
                        : (strlen(node->name) > 0 ? "With" : ""),
                    node->fields_count > 1 ? arg_name_cap : arg_name_cap,
                    arg_type, arg_is_obj && !strstr(arg_type, "*") ? " *" : "",
                    node->fields[j].name);
          }
        }
      }
      fprintf(m_file, " {\n");
      fprintf(m_file,
              "    // TODO: Implement actual C call and ARC conversion\n");
      /* Stub return */
      if (strcmp(ret_type, "void") != 0) {
        if (ret_is_obj) {
          fprintf(m_file, "    return nil;\n");
        } else {
          fprintf(m_file, "    return CDD_C_SUCCESS;\n");
        }
      }
      fprintf(m_file, "}\n\n");
    }
  }
  fprintf(m_file, "@end\n");
  fclose(m_file);

  return CDD_C_SUCCESS;
}
