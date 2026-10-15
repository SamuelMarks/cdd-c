/**
 * @file cdd_ffi_emit_nim.c
 * @brief FFI binding emitter for Nim.
 */
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_nim.h"

#include "../../../include/c_cdd/safe_crt.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../cdd_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

static cdd_c_error_t map_nim_type(cdd_ffi_type_t *t, const char **out_type) {
  static const cdd_ffi_type_map_table_t k_nim_types = {
      "void",    "bool",    "int8",      "uint8",      "int16",  "uint16",
      "cint",    "cuint",   "clonglong", "culonglong", "cfloat", "cdouble",
      "cstring", "pointer", NULL,        "pointer",    1,        1,
      1};
  return cdd_ffi_map_type_from_table(&k_nim_types, *t, out_type);
}

cdd_c_error_t cdd_ffi_emit_nim(cdd_ffi_ir_t *ir,
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
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.nim", config->output_dir,
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
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.nim", config->output_dir,
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

  fprintf(f, "# Auto-generated Nim bindings for %s\n\n", lib_name);
  fprintf(f, "{.passL: \"-l%s\".}\n", lib_name);
  /* Assuming the original header name matches the module_name or is accessible
   */
  fprintf(f, "## {.header: \"%s.h\".}\n\n", module_name);

  /* Forward declarations for structs if any cyclic dependency, but Nim allows
   * types block */
  fprintf(f, "type\n");

  /* Structures and Unions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "  %s* {.importc: \"%s\", bycopy.} = object\n", node->name,
              node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *nt = NULL;
        map_nim_type(&node->fields[j].type, &nt);
        fprintf(f, "    %s*: %s\n", node->fields[j].name, nt);
      }
      fprintf(f, "\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "  %s* {.size: sizeof(cint).} = enum\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    %s = %s,\n", var->name, var->value);
        } else {
          fprintf(f, "    %s,\n", var->name);
        }
      }
      fprintf(f, "\n");
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_nt = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "proc %s*(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *nt = NULL;
        map_nim_type(&node->fields[j].type, &nt);
        fprintf(f, "%s%s: %s", j > 0 ? ", " : "", node->fields[j].name, nt);
      }
      map_nim_type(&node->return_or_base_type, &ret_nt);
      fprintf(f, "): %s {.importc: \"%s\", cdecl.}\n\n", ret_nt, node->name);
    }
  }

  fclose(f);

  /* Generate .nimble file */
#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.nimble", config->output_dir,
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
      return CDD_C_ERROR_IO;
    }
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.nimble", config->output_dir,
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
  if (!f)
    return CDD_C_ERROR_IO;
#endif
  fprintf(f, "version       = \"0.1.0\"\n");
  fprintf(f, "author        = \"Auto-generated\"\n");
  fprintf(f, "description   = \"Nim bindings for %s\"\n", lib_name);
  fprintf(f, "license       = \"MIT\"\n");
  fprintf(f, "srcDir        = \".\"\n");
  fprintf(f, "requires \"nim >= 1.6.0\"\n");
  fclose(f);

  return CDD_C_SUCCESS;
}
