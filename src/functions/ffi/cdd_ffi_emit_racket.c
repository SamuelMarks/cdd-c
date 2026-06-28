/* clang-format off */
#include "cdd_ffi_emit_racket.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

static void racketify_name(const char *c_name, char *out_name, size_t out_sz) {
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

static void map_racket_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      CDD_SNPRINTF(out_type, out_sz, "_bytes");
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, "_pointer");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    CDD_SNPRINTF(out_type, out_sz, "_void");
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz, "_stdbool");
    break;
  case CDD_FFI_KIND_INT8:
    CDD_SNPRINTF(out_type, out_sz, "_int8");
    break;
  case CDD_FFI_KIND_UINT8:
    CDD_SNPRINTF(out_type, out_sz, "_uint8");
    break;
  case CDD_FFI_KIND_INT16:
    CDD_SNPRINTF(out_type, out_sz, "_int16");
    break;
  case CDD_FFI_KIND_UINT16:
    CDD_SNPRINTF(out_type, out_sz, "_uint16");
    break;
  case CDD_FFI_KIND_INT32:
    CDD_SNPRINTF(out_type, out_sz, "_int32");
    break;
  case CDD_FFI_KIND_UINT32:
    CDD_SNPRINTF(out_type, out_sz, "_uint32");
    break;
  case CDD_FFI_KIND_INT64:
    CDD_SNPRINTF(out_type, out_sz, "_int64");
    break;
  case CDD_FFI_KIND_UINT64:
    CDD_SNPRINTF(out_type, out_sz, "_uint64");
    break;
  case CDD_FFI_KIND_FLOAT32:
    CDD_SNPRINTF(out_type, out_sz, "_float");
    break;
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, "_double");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    if (t->ref_name) {
      char racket_name[256];
      racketify_name(t->ref_name, racket_name, sizeof(racket_name));
      CDD_SNPRINTF(out_type, out_sz, "_%s-pointer", racket_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "_pointer");
    }
    break;
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      char racket_name[256];
      racketify_name(t->ref_name, racket_name, sizeof(racket_name));
      CDD_SNPRINTF(out_type, out_sz, "_%s", racket_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "_pointer");
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, "_pointer");
    break;
  }
}

enum cdd_c_error
cdd_ffi_emit_racket(cdd_ffi_ir_t *ir,
                    const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name;
  char racket_lib_name[256];
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_enum_variant_t *var;
  char racket_node_name[256];
  char field_name[256];
  char type_str[256];
  char var_name[256];
  char ret_type_str[256];

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  lib_name = config->library_name ? config->library_name : "mylib";
  racketify_name(lib_name, racket_lib_name, sizeof(racket_lib_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.rkt", config->output_dir,
               racket_lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.rkt", config->output_dir,
               racket_lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, ";; Auto-generated Racket FFI bindings for %s\n", lib_name);
  fprintf(f, "#lang racket/base\n");
  fprintf(f, "(require ffi/unsafe\n");
  fprintf(f, "         ffi/unsafe/define)\n\n");

  fprintf(f, "(provide (all-defined-out))\n\n");

  fprintf(f, "(define %s-lib\n", racket_lib_name);
  fprintf(f, "  (ffi-lib \"%s\"))\n\n", lib_name);

  fprintf(f, "(define-ffi-definer define-%s %s-lib)\n\n", racket_lib_name,
          racket_lib_name);

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    racketify_name(node->name, racket_node_name, sizeof(racket_node_name));

    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      /* In Racket, define-cstruct does not easily support unions. Treating
       * union as opaque or first member. */
      fprintf(f, "(define-cstruct _%s\n", racket_node_name);
      if (node->kind == CDD_FFI_NODE_UNION) {
        fprintf(f, "  ([data _pointer]))\n\n");
      } else {
        fprintf(f, "  (");
        if (node->fields_count == 0) {
          fprintf(f, ")\n\n");
        }
        for (j = 0; j < node->fields_count; j++) {
          racketify_name(node->fields[j].name, field_name, sizeof(field_name));
          map_racket_type(&node->fields[j].type, type_str, sizeof(type_str));
          fprintf(f, "[%s %s]%s", field_name, type_str,
                  j + 1 < node->fields_count ? "\n   " : "))\n\n");
        }
      }
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "(define _%s\n", racket_node_name);
      fprintf(f, "  (_enum\n");
      fprintf(f, "   '(\n");
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        racketify_name(var->name, var_name, sizeof(var_name));
        if (var->value) {
          fprintf(f, "     [%s = %s]\n", var_name, var->value);
        } else {
          fprintf(f, "     %s\n", var_name);
        }
      }
      fprintf(f, "    )))\n\n");
    } else if (node->kind == CDD_FFI_NODE_TYPEDEF) {
      map_racket_type(&node->return_or_base_type, type_str, sizeof(type_str));
      fprintf(f, "(define _%s %s)\n\n", racket_node_name, type_str);
    } else if (node->kind == CDD_FFI_NODE_FUNCTION) {
      map_racket_type(&node->return_or_base_type, ret_type_str,
                      sizeof(ret_type_str));
      fprintf(f, "(define-%s %s\n", racket_lib_name, node->name);
      fprintf(f, "  (_fun ");
      for (j = 0; j < node->fields_count; j++) {
        map_racket_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(f, "%s ", type_str);
      }
      fprintf(f, "-> %s))\n\n", ret_type_str);
    }
  }

  fclose(f);
  return CDD_C_SUCCESS;
}
