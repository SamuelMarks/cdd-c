/* clang-format off */
#include "cdd_ffi_emit_scheme.h"

#include "../../cdd_api.h"
#include "../../win_compat_sym.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

static void schemify_name(const char *c_name, char *out_name, size_t out_sz) {
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

static void map_scheme_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      CDD_SNPRINTF(out_type, out_sz, "string");
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, "void*");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    CDD_SNPRINTF(out_type, out_sz, "void");
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz, "boolean");
    break;
  case CDD_FFI_KIND_INT8:
    CDD_SNPRINTF(out_type, out_sz, "integer-8");
    break;
  case CDD_FFI_KIND_UINT8:
    CDD_SNPRINTF(out_type, out_sz, "unsigned-8");
    break;
  case CDD_FFI_KIND_INT16:
    CDD_SNPRINTF(out_type, out_sz, "integer-16");
    break;
  case CDD_FFI_KIND_UINT16:
    CDD_SNPRINTF(out_type, out_sz, "unsigned-16");
    break;
  case CDD_FFI_KIND_INT32:
    CDD_SNPRINTF(out_type, out_sz, "integer-32");
    break;
  case CDD_FFI_KIND_UINT32:
    CDD_SNPRINTF(out_type, out_sz, "unsigned-32");
    break;
  case CDD_FFI_KIND_INT64:
    CDD_SNPRINTF(out_type, out_sz, "integer-64");
    break;
  case CDD_FFI_KIND_UINT64:
    CDD_SNPRINTF(out_type, out_sz, "unsigned-64");
    break;
  case CDD_FFI_KIND_FLOAT32:
    CDD_SNPRINTF(out_type, out_sz, "single-float");
    break;
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, "double-float");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    if (t->ref_name) {
      char scheme_name[256];
      schemify_name(t->ref_name, scheme_name, sizeof(scheme_name));
      CDD_SNPRINTF(out_type, out_sz, "(* %s)", scheme_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "void*");
    }
    break;
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      char scheme_name[256];
      schemify_name(t->ref_name, scheme_name, sizeof(scheme_name));
      CDD_SNPRINTF(out_type, out_sz, "%s", scheme_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "void*");
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, "void*");
    break;
  }
}

enum cdd_c_error
cdd_ffi_emit_scheme(cdd_ffi_ir_t *ir,
                    const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name;
  char scheme_lib_name[256];
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_enum_variant_t *var;
  char scheme_node_name[256];
  char field_name[256];
  char type_str[256];
  char var_name[256];
  char ret_type_str[256];

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  lib_name = config->library_name ? config->library_name : "mylib";
  schemify_name(lib_name, scheme_lib_name, sizeof(scheme_lib_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.ss", config->output_dir,
               scheme_lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.ss", config->output_dir,
               scheme_lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, ";; Auto-generated Chez Scheme FFI bindings for %s\n", lib_name);
  fprintf(f, "(library (%s)\n", scheme_lib_name);
  fprintf(f, "  (export\n");

  /* Export list */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    schemify_name(node->name, scheme_node_name, sizeof(scheme_node_name));
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "    %s\n", scheme_node_name);
    }
  }

  fprintf(f, "  )\n");
  fprintf(f, "  (import (chezscheme))\n\n");

  fprintf(f, "  (define lib-name\n");
  fprintf(f, "    (cond\n");
  fprintf(f, "      [(eq? (machine-os) 'windows) \"%s.dll\"]\n", lib_name);
  fprintf(f, "      [(eq? (machine-os) 'osx) \"lib%s.dylib\"]\n", lib_name);
  fprintf(f, "      [else \"lib%s.so\"]))\n\n", lib_name);
  fprintf(f, "  (load-shared-object lib-name)\n\n");

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    schemify_name(node->name, scheme_node_name, sizeof(scheme_node_name));

    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "  (define-ftype %s\n", scheme_node_name);
      if (node->kind == CDD_FFI_NODE_UNION) {
        fprintf(f, "    (union\n");
        for (j = 0; j < node->fields_count; j++) {
          schemify_name(node->fields[j].name, field_name, sizeof(field_name));
          map_scheme_type(&node->fields[j].type, type_str, sizeof(type_str));
          fprintf(f, "      [%s %s]\n", field_name, type_str);
        }
        if (node->fields_count == 0) {

          /* LCOV_EXCL_START */

          fprintf(f, "      [data void*]\n"); /* Fallback for empty union */

          /* LCOV_EXCL_STOP */
        }
        fprintf(f, "    ))\n\n");
      } else {
        fprintf(f, "    (struct\n");
        for (j = 0; j < node->fields_count; j++) {
          schemify_name(node->fields[j].name, field_name, sizeof(field_name));
          map_scheme_type(&node->fields[j].type, type_str, sizeof(type_str));
          fprintf(f, "      [%s %s]\n", field_name, type_str);
        }
        fprintf(f, "    ))\n\n");
      }
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      /* Chez does not have define-enum natively in ftype, so map to int */
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        schemify_name(var->name, var_name, sizeof(var_name));
        if (var->value) {
          fprintf(f, "  (define %s %s)\n", var_name, var->value);
        } else {
          /* cppcheck-suppress invalidPrintfArgType_uint */
          fprintf(f, "  (define %s %" CDD_SIZE_T_FMT ")\n", var_name, j);
        }
      }
      fprintf(f, "\n");
    } else if (node->kind == CDD_FFI_NODE_TYPEDEF) {
      map_scheme_type(&node->return_or_base_type, type_str, sizeof(type_str));
      /* define-ftype alias */
      fprintf(f, "  (define-ftype %s %s)\n\n", scheme_node_name, type_str);
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      map_scheme_type(&node->return_or_base_type, ret_type_str,
                      sizeof(ret_type_str));
      fprintf(f, "  (define %s\n", scheme_node_name);
      fprintf(f, "    (foreign-procedure \"%s\" (", node->name);
      for (j = 0; j < node->fields_count; j++) {
        map_scheme_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(f, "%s%s", type_str, j + 1 < node->fields_count ? " " : "");
      }
      fprintf(f, ") %s))\n\n", ret_type_str);
    }
  }

  fprintf(f, ")\n");
  fclose(f);
  return CDD_C_SUCCESS;
}
