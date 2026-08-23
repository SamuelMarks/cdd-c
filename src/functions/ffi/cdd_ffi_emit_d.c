extern volatile int g_fail_io_after;
/* clang-format off */
#include "cdd_ffi_emit_d.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* clang-format on */

static const char *map_d_type(cdd_ffi_type_t *t) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      if (t->is_const) {
        return "const(char)*";
      }
      return "char*";
    }
    if (t->kind == CDD_FFI_KIND_VOID) {
      return "void*";
    }
    return "void*";
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "bool";
  case CDD_FFI_KIND_INT8:
    return "byte";
  case CDD_FFI_KIND_UINT8:
    return "ubyte";
  case CDD_FFI_KIND_INT16:
    return "short";
  case CDD_FFI_KIND_UINT16:
    return "ushort";
  case CDD_FFI_KIND_INT32:
    return "int";
  case CDD_FFI_KIND_UINT32:
    return "uint";
  case CDD_FFI_KIND_INT64:
    return "long";
  case CDD_FFI_KIND_UINT64:
    return "ulong";
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
    return "void*";
  default:
    return "void*";
  }
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
      fprintf(f, "struct %s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "    %s %s;\n", map_d_type(&node->fields[j].type),
                node->fields[j].name);
      }
      fprintf(f, "}\n\n");
    } else if (node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "union %s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "    %s %s;\n", map_d_type(&node->fields[j].type),
                node->fields[j].name);
      }
      fprintf(f, "}\n\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
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
      fprintf(f, "%s %s(", map_d_type(&node->return_or_base_type), node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s%s %s", j > 0 ? ", " : "",
                map_d_type(&node->fields[j].type), node->fields[j].name);
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
