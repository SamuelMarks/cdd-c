/**
 * @file cdd_ffi_emit_objc.c
 * @brief FFI binding emitter for Objc.
 */
#ifdef CDD_BUILD_TESTS
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_objc.h"

#include "../../../include/c_cdd/safe_crt.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../cdd_api.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static cdd_c_error_t map_objc_type(cdd_ffi_type_t *t, int is_return,
                                   const char **out_type) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      if (t->is_const) {
        *out_type = is_return ? "NSString *" : "NSString * _Nonnull";
        return CDD_C_SUCCESS;
      }
      *out_type = "char *";
      return CDD_C_SUCCESS;
    }
    if (t->kind == CDD_FFI_KIND_STRUCT_REF ||
        t->kind == CDD_FFI_KIND_TYPEDEF_REF ||
        t->kind == CDD_FFI_KIND_ENUM_REF) {
      if (t->ref_name) {
        *out_type = t->ref_name; /* assume it maps to an ObjC class pointer,
                                    usually needs '*' added later */
        return CDD_C_SUCCESS;
      }
    }
    *out_type = "void *";
    return CDD_C_SUCCESS;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = "void";
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = "BOOL";
    break;
  case CDD_FFI_KIND_INT8:
    *out_type = "int8_t";
    break;
  case CDD_FFI_KIND_UINT8:
    *out_type = "uint8_t";
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = "int16_t";
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = "uint16_t";
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = "int32_t";
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = "uint32_t";
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = "int64_t";
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = "uint64_t";
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = "float";
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = "double";
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      *out_type = t->ref_name;
    } else {
      *out_type = "void *";
    }
    break;
  default:
    *out_type = "void *";
    break;
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t capitalize_first(char *str) {
  str[0] = (char)toupper((unsigned char)str[0]);
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_ffi_emit_objc(cdd_ffi_ir_t *ir,
                                const cdd_generate_bindings_config_t *config) {
  FILE *h_file = NULL;
  FILE *m_file = NULL;
  char h_filepath[1024];
  char m_filepath[1024];
  const char *lib_name;
  const char *module_name;
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  lib_name = (config->library_name) ? config->library_name : "MyLib";
  module_name = (config->module_name) ? config->module_name : "Bindings";

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
#if defined(_MSC_VER)
  if (fopen_s(&h_file, h_filepath, "w") != 0)
    h_file = NULL;
#else
  h_file = fopen(h_filepath, "w");
#endif
  if (!h_file)
    return CDD_C_ERROR_UNKNOWN;
#if defined(_MSC_VER)
  if (fopen_s(&m_file, m_filepath, "w") != 0)
    m_file = NULL;
#else
  m_file = fopen(m_filepath, "w");
#endif
  if (!m_file) {
    printf("Failed to open %s\n", m_filepath);
    fclose(h_file);
    return CDD_C_ERROR_UNKNOWN;
  } else {
    printf("Successfully opened %s (which is weird if it's a directory)\n",
           m_filepath);
  }
#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after == 2) {
      fclose(m_file);
      fclose(h_file);
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#endif
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
        const char *t_str = NULL;
        int is_obj;
        map_objc_type(&node->fields[j].type, 1, &t_str);
        /* simplistic pointer handling for class refs */
        is_obj = (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF ||
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
      const char *ret_type = NULL;
      int ret_is_obj;
      map_objc_type(&node->return_or_base_type, 1, &ret_type);
      ret_is_obj = (node->return_or_base_type.kind == CDD_FFI_KIND_STRUCT_REF ||
                    strstr(ret_type, "NSString") != NULL);

      fprintf(h_file, "+ (%s%s)%s", ret_type,
              ret_is_obj && !strstr(ret_type, "*") ? " *" : "", node->name);

      if (node->fields_count > 0) {
        for (j = 0; j < node->fields_count; j++) {
          const char *arg_type = NULL;
          int arg_is_obj;
          const char *arg_name =
              node->fields[j].name ? node->fields[j].name : "arg";
          char arg_name_cap[256];
          map_objc_type(&node->fields[j].type, 0, &arg_type);
          arg_is_obj = (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF ||
                        strstr(arg_type, "NSString") != NULL);
          CDD_STRNCPY(arg_name_cap, sizeof(arg_name_cap), arg_name,
                      sizeof(arg_name_cap) - 1);
          arg_name_cap[sizeof(arg_name_cap) - 1] = '\0';

          if (j > 0) {
            capitalize_first(arg_name_cap);
            fprintf(h_file, " %s:(%s%s)%s", node->fields[j].name, arg_type,
                    arg_is_obj && !strstr(arg_type, "*") ? " *" : "",
                    node->fields[j].name);
          } else {
            /* First arg format: + (void)doSomethingWithArg:(int)arg; */
            fprintf(h_file, "With%s:(%s%s)%s", arg_name_cap, arg_type,
                    arg_is_obj && !strstr(arg_type, "*") ? " *" : "",
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
      const char *ret_type = NULL;
      int ret_is_obj;
      map_objc_type(&node->return_or_base_type, 1, &ret_type);
      ret_is_obj = (node->return_or_base_type.kind == CDD_FFI_KIND_STRUCT_REF ||
                    strstr(ret_type, "NSString") != NULL);

      fprintf(m_file, "+ (%s%s)%s", ret_type,
              ret_is_obj && !strstr(ret_type, "*") ? " *" : "", node->name);

      if (node->fields_count > 0) {
        for (j = 0; j < node->fields_count; j++) {
          const char *arg_type = NULL;
          int arg_is_obj;
          const char *arg_name =
              node->fields[j].name ? node->fields[j].name : "arg";
          char arg_name_cap[256];
          map_objc_type(&node->fields[j].type, 0, &arg_type);
          arg_is_obj = (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF ||
                        strstr(arg_type, "NSString") != NULL);
          CDD_STRNCPY(arg_name_cap, sizeof(arg_name_cap), arg_name,
                      sizeof(arg_name_cap) - 1);
          arg_name_cap[sizeof(arg_name_cap) - 1] = '\0';

          if (j > 0) {
            capitalize_first(arg_name_cap);
            fprintf(m_file, " %s:(%s%s)%s", node->fields[j].name, arg_type,
                    arg_is_obj && !strstr(arg_type, "*") ? " *" : "",
                    node->fields[j].name);
          } else {
            fprintf(m_file, "With%s:(%s%s)%s", arg_name_cap, arg_type,
                    arg_is_obj && !strstr(arg_type, "*") ? " *" : "",
                    node->fields[j].name);
          }
        }
      }
      fprintf(m_file, " {\n");

      /* Argument marshaling */
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_type = NULL;
        map_objc_type(&node->fields[j].type, 0, &arg_type);
        if (strstr(arg_type, "NSString") != NULL) {
          fprintf(m_file, "    const char * _c_%s = [%s UTF8String];\n",
                  node->fields[j].name, node->fields[j].name);
        }
      }

      /* Call C function */
      fprintf(m_file, "    ");
      if (strcmp(ret_type, "void") != 0) {
        if (strstr(ret_type, "NSString") != NULL) {
          fprintf(m_file, "const char* _c_ret = ");
        } else if (ret_is_obj) {
          fprintf(m_file, "void* _c_ret = (void*)");
        } else {
          fprintf(m_file, "%s _c_ret = ", ret_type);
        }
      }

      fprintf(m_file, "%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_type = NULL;
        map_objc_type(&node->fields[j].type, 0, &arg_type);
        if (j > 0)
          fprintf(m_file, ", ");

        if (strstr(arg_type, "NSString") != NULL) {
          fprintf(m_file, "_c_%s", node->fields[j].name);
        } else if (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(m_file, "(__bridge void *)%s", node->fields[j].name);
        } else {
          fprintf(m_file, "%s", node->fields[j].name);
        }
      }
      fprintf(m_file, ");\n");

      /* Return conversion */
      if (strcmp(ret_type, "void") != 0) {
        if (strstr(ret_type, "NSString") != NULL) {
          fprintf(m_file, "    return _c_ret ? [NSString "
                          "stringWithUTF8String:_c_ret] : nil;\n");
        } else if (ret_is_obj) {
          fprintf(m_file, "    return (__bridge %s *)_c_ret;\n", ret_type);
        } else {
          fprintf(m_file, "    return _c_ret;\n");
        }
      }

      fprintf(m_file, "}\n\n");
    }
  }
  fprintf(m_file, "@end\n");
  fclose(m_file);

  return CDD_C_SUCCESS;
}
