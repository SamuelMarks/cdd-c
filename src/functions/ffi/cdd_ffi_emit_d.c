/**
 * @file cdd_ffi_emit_d.c
 * @brief FFI binding emitter for D.
 */
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_d.h"

#include "../../../include/c_cdd/safe_crt.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../cdd_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

static cdd_c_error_t map_d_type(cdd_ffi_type_t *t, const char **out_type) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      if (t->is_const) {
        *out_type = "const(char)*";
        return CDD_C_SUCCESS;
      }
      *out_type = "char*";
      return CDD_C_SUCCESS;
    }
    if (t->kind == CDD_FFI_KIND_VOID) {
      *out_type = "void*";
      return CDD_C_SUCCESS;
    }
    *out_type = "void*";
    return CDD_C_SUCCESS;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = "void";
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = "bool";
    break;
  case CDD_FFI_KIND_INT8:
    *out_type = "byte";
    break;
  case CDD_FFI_KIND_UINT8:
    *out_type = "ubyte";
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = "short";
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = "ushort";
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = "int";
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = "uint";
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = "long";
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = "ulong";
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
      *out_type = "void*";
    }
    break;
  default:
    *out_type = "void*";
    break;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_ffi_emit_d(cdd_ffi_ir_t *ir,
                             const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name =
      config ? ((config->library_name) ? config->library_name : "mylib")
             : "mylib";
  const char *module_name =
      config ? ((config->module_name) ? config->module_name : "bindings")
             : "bindings";
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.d", config->output_dir,
               module_name);
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
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.d", config->output_dir,
               module_name);
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

  fprintf(f, "// Auto-generated D bindings for %s\n", lib_name);
  fprintf(f, "module %s;\n\n", module_name);
  fprintf(f, "import core.stdc.config;\n");
  fprintf(f, "import core.stdc.stddef;\n\n");
  fprintf(f, "extern(C) @nogc nothrow:\n\n");

  /* structs and unions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "struct %s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *dt = NULL;
        map_d_type(&node->fields[j].type, &dt);
        fprintf(f, "    %s %s;\n", dt, node->fields[j].name);
      }
      fprintf(f, "}\n\n");
    } else if (node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "union %s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *dt = NULL;
        map_d_type(&node->fields[j].type, &dt);
        fprintf(f, "    %s %s;\n", dt, node->fields[j].name);
      }
      fprintf(f, "}\n\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "enum %s : int {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s,\n", var->name, var->value);
        } else {
          fprintf(f, "    %s,\n", var->name);
        }
      }
      fprintf(f, "}\n\n");
    }
  }

  /* functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_dt = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      map_d_type(&node->return_or_base_type, &ret_dt);
      fprintf(f, "%s %s(", ret_dt, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *dt = NULL;
        map_d_type(&node->fields[j].type, &dt);
        fprintf(f, "%s%s %s", j > 0 ? ", " : "", dt, node->fields[j].name);
      }
      fprintf(f, ");\n");
    }
  }

  fclose(f);

  /* Generate dub.json */
#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\dub.json", config->output_dir);
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
      return CDD_C_ERROR_IO;
    }
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/dub.json", config->output_dir);
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
  if (!f)
    return CDD_C_ERROR_IO;
#endif
  fprintf(f, "{\n");
  fprintf(f, "  \"name\": \"%s\",\n", module_name);
  fprintf(f, "  \"description\": \"Auto-generated D bindings for %s\",\n",
          lib_name);
  fprintf(f, "  \"targetType\": \"library\",\n");
  fprintf(f, "  \"sourcePaths\": [ \".\" ],\n");
  fprintf(f, "  \"libs\": [ \"%s\" ]\n", lib_name);
  fprintf(f, "}\n");
  fclose(f);

  return CDD_C_SUCCESS;
}
