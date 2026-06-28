/* clang-format off */
#include "cdd_ffi_emit_common_lisp.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

static void lispify_name(const char *c_name, char *out_name, size_t out_sz) {
  size_t i = 0, j = 0;
  if (!c_name || !out_name || out_sz == 0)
    return;
  while (c_name[i] && j < out_sz - 1) {
    if (c_name[i] == '_') {
      out_name[j++] = '-';
    } else {
      out_name[j++] = (char)tolower((unsigned char)c_name[i]);
    }
    i++;
  }
  out_name[j] = '\0';
}

static void map_lisp_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      CDD_SNPRINTF(out_type, out_sz, ":string");
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, ":pointer");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    CDD_SNPRINTF(out_type, out_sz, ":void");
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz, ":boolean");
    break;
  case CDD_FFI_KIND_INT8:
    CDD_SNPRINTF(out_type, out_sz, ":int8");
    break;
  case CDD_FFI_KIND_UINT8:
    CDD_SNPRINTF(out_type, out_sz, ":uint8");
    break;
  case CDD_FFI_KIND_INT16:
    CDD_SNPRINTF(out_type, out_sz, ":int16");
    break;
  case CDD_FFI_KIND_UINT16:
    CDD_SNPRINTF(out_type, out_sz, ":uint16");
    break;
  case CDD_FFI_KIND_INT32:
    CDD_SNPRINTF(out_type, out_sz, ":int32");
    break;
  case CDD_FFI_KIND_UINT32:
    CDD_SNPRINTF(out_type, out_sz, ":uint32");
    break;
  case CDD_FFI_KIND_INT64:
    CDD_SNPRINTF(out_type, out_sz, ":int64");
    break;
  case CDD_FFI_KIND_UINT64:
    CDD_SNPRINTF(out_type, out_sz, ":uint64");
    break;
  case CDD_FFI_KIND_FLOAT32:
    CDD_SNPRINTF(out_type, out_sz, ":float");
    break;
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, ":double");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    if (t->ref_name) {
      char lisp_name[256];
      lispify_name(t->ref_name, lisp_name, sizeof(lisp_name));
      CDD_SNPRINTF(out_type, out_sz, "(:struct %s)", lisp_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, ":pointer");
    }
    break;
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      char lisp_name[256];
      lispify_name(t->ref_name, lisp_name, sizeof(lisp_name));
      CDD_SNPRINTF(out_type, out_sz, "%s", lisp_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, ":pointer");
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, ":pointer");
    break;
  }
}

enum cdd_c_error
cdd_ffi_emit_common_lisp(cdd_ffi_ir_t *ir,
                         const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  FILE *asd_f = NULL;
  char filepath[1024];
  char asd_filepath[1024];
  const char *lib_name;
  char lisp_lib_name[256];
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_enum_variant_t *var;
  char lisp_node_name[256];
  char field_name[256];
  char type_str[256];
  char var_name[256];
  char arg_name[256];
  char ret_type_str[256];

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  lib_name = config->library_name ? config->library_name : "mylib";
  lispify_name(lib_name, lisp_lib_name, sizeof(lisp_lib_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.lisp", config->output_dir,
               lisp_lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(asd_filepath, sizeof(asd_filepath), "%s\\%s.asd",
               config->output_dir, lisp_lib_name);
  if (fopen_s(&asd_f, asd_filepath, "w") != 0) {
    fclose(f);
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.lisp", config->output_dir,
               lisp_lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(asd_filepath, sizeof(asd_filepath), "%s/%s.asd",
               config->output_dir, lisp_lib_name);
  asd_f = fopen(asd_filepath, "w");
  if (!asd_f) {
    fclose(f);
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(asd_f, ";;;; Auto-generated ASDF system definition for %s\n\n",
          lisp_lib_name);
  fprintf(asd_f, "(asdf:defsystem #:%s\n", lisp_lib_name);
  fprintf(asd_f, "  :description \"CFFI bindings for %s\"\n", lib_name);
  fprintf(asd_f, "  :depends-on (#:cffi)\n");
  fprintf(asd_f, "  :components ((:file \"%s\")))\n", lisp_lib_name);
  fclose(asd_f);

  fprintf(f, ";;;; Auto-generated Common Lisp CFFI bindings for %s\n\n",
          lib_name);
  fprintf(f, "(defpackage #:%s\n", lisp_lib_name);
  fprintf(f, "  (:use #:cl #:cffi))\n\n");
  fprintf(f, "(in-package #:%s)\n\n", lisp_lib_name);

  fprintf(f, "(define-foreign-library %s-lib\n", lisp_lib_name);
  fprintf(f, "  (:windows \"%s.dll\")\n", lib_name);
  fprintf(f, "  (:darwin \"lib%s.dylib\")\n", lib_name);
  fprintf(f, "  (:unix \"lib%s.so\")\n", lib_name);
  fprintf(f, "  (t (:default \"%s\")))\n\n", lib_name);
  fprintf(f, "(use-foreign-library %s-lib)\n\n", lisp_lib_name);

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    lispify_name(node->name, lisp_node_name, sizeof(lisp_node_name));

    if (node->kind == CDD_FFI_NODE_STRUCT) {
      fprintf(f, "(defcstruct %s\n", lisp_node_name);
      for (j = 0; j < node->fields_count; j++) {
        lispify_name(node->fields[j].name, field_name, sizeof(field_name));
        map_lisp_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(f, "  (%s %s)\n", field_name, type_str);
      }
      fprintf(f, ")\n\n");
    } else if (node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "(defcunion %s\n", lisp_node_name);
      for (j = 0; j < node->fields_count; j++) {
        lispify_name(node->fields[j].name, field_name, sizeof(field_name));
        map_lisp_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(f, "  (%s %s)\n", field_name, type_str);
      }
      fprintf(f, ")\n\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "(defcenum %s\n", lisp_node_name);
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        lispify_name(var->name, var_name, sizeof(var_name));
        if (var->value) {
          fprintf(f, "  (:%s %s)\n", var_name, var->value);
        } else {
          fprintf(f, "  :%s\n", var_name);
        }
      }
      fprintf(f, ")\n\n");
    } else if (node->kind == CDD_FFI_NODE_TYPEDEF) {
      map_lisp_type(&node->return_or_base_type, type_str, sizeof(type_str));
      fprintf(f, "(defctype %s %s)\n\n", lisp_node_name, type_str);
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      map_lisp_type(&node->return_or_base_type, ret_type_str,
                    sizeof(ret_type_str));
      fprintf(f, "(defcfun (\"%s\" %s) %s\n", node->name, lisp_node_name,
              ret_type_str);
      for (j = 0; j < node->fields_count; j++) {
        lispify_name(node->fields[j].name, arg_name, sizeof(arg_name));
        map_lisp_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(f, "  (%s %s)\n", arg_name, type_str);
      }
      fprintf(f, ")\n\n");
    }
  }

  fclose(f);
  return CDD_C_SUCCESS;
}
