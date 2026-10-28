/**
 * @file cdd_ffi_emit_groovy.c
 * @brief FFI binding emitter for Groovy.
 */
#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_groovy.h"

#include "../../../include/c_cdd/safe_crt.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../cdd_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

/**
 * @brief Maps an FFI IR type to its corresponding Groovy JNA interop type
 * string.
 *
 * @param[in] t FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the resolved Groovy JNA type
 * string.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t map_groovy_jna_type(cdd_ffi_type_t *t,
                                         const char **out_type) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      if (t->is_const) {
        *out_type = "String";
        return CDD_C_SUCCESS;
      }
    }
    *out_type = "Pointer";
    return CDD_C_SUCCESS;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = "void";
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = "boolean";
    break;
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
    *out_type = "byte";
    break;
  case CDD_FFI_KIND_INT16:
  case CDD_FFI_KIND_UINT16:
    *out_type = "short";
    break;
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_UINT32:
    *out_type = "int";
    break;
  case CDD_FFI_KIND_INT64:
  case CDD_FFI_KIND_UINT64:
    *out_type = "long";
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
      *out_type = "Pointer";
    }
    break;
  default:
    *out_type = "Pointer";
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Groovy JNA bindings and Spock specification tests for the given
 * FFI IR.
 *
 * @param[in] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t
cdd_ffi_emit_groovy(cdd_ffi_ir_t *ir,
                    const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name =
      config ? ((config->library_name) ? config->library_name : "mylib")
             : "mylib";
  const char *module_name =
      config ? ((config->module_name) ? config->module_name : "MyLib")
             : "MyLib";
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.groovy", config->output_dir,
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
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.groovy", config->output_dir,
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

  fprintf(f, "// Auto-generated Groovy JNA bindings for %s\n\n", lib_name);
  fprintf(f, "import com.sun.jna.Library\n");
  fprintf(f, "import com.sun.jna.Native\n");
  fprintf(f, "import com.sun.jna.Pointer\n");
  fprintf(f, "import com.sun.jna.Structure\n");
  fprintf(f, "import java.util.List\n");
  fprintf(f, "import java.util.Arrays\n\n");

  /* Structure classes */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "class %s extends Structure {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *gt = NULL;
        map_groovy_jna_type(&node->fields[j].type, &gt);
        fprintf(f, "    public %s %s\n", gt, node->fields[j].name);
      }
      fprintf(f, "\n    @Override\n");
      fprintf(f, "    protected List<String> getFieldOrder() {\n");
      fprintf(f, "        return Arrays.asList(");
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "\"%s\"%s", node->fields[j].name,
                j + 1 < node->fields_count ? ", " : "");
      }
      fprintf(f, ")\n    }\n}\n\n");
    }
  }

  /* Main Library Interface */
  fprintf(f, "interface %s extends Library {\n", module_name);
  fprintf(f, "    %s INSTANCE = (%s) Native.load(\"%s\", %s.class)\n\n",
          module_name, module_name, lib_name, module_name);

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_gt = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      map_groovy_jna_type(&node->return_or_base_type, &ret_gt);
      fprintf(f, "    %s %s(", ret_gt, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *gt = NULL;
        map_groovy_jna_type(&node->fields[j].type, &gt);
        fprintf(f, "%s%s %s", j > 0 ? ", " : "", gt, node->fields[j].name);
      }
      fprintf(f, ")\n");
    }
  }
  fprintf(f, "}\n");

  fclose(f);

  /* Generate Spock Test Template */
#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%sSpec.groovy",
               config->output_dir, module_name);
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
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%sSpec.groovy",
               config->output_dir, module_name);
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
  fprintf(f, "import spock.lang.Specification\n\n");
  fprintf(f, "class %sSpec extends Specification {\n", module_name);
  fprintf(f, "    def \"Library can be loaded\"() {\n");
  fprintf(f, "        expect:\n");
  fprintf(f, "        %s.INSTANCE != null\n", module_name);
  fprintf(f, "    }\n");
  fprintf(f, "}\n");
  fclose(f);

  return CDD_C_SUCCESS;
}
