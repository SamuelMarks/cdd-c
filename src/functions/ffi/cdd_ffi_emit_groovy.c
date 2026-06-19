/* clang-format off */
#include "cdd_ffi_emit_groovy.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static const char *map_groovy_jna_type(cdd_ffi_type_t *t) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      if (t->is_const) {
        return "String";
      }
    }
    return "Pointer";
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "boolean";
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
    return "byte";
  case CDD_FFI_KIND_INT16:
  case CDD_FFI_KIND_UINT16:
    return "short";
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_UINT32:
    return "int";
  case CDD_FFI_KIND_INT64:
  case CDD_FFI_KIND_UINT64:
    return "long";
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
    return "Pointer";
  default:
    return "Pointer";
  }
}

int cdd_ffi_emit_groovy(cdd_ffi_ir_t *ir,
                        const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name = config->library_name ? config->library_name : "mylib";
  const char *module_name = config->module_name ? config->module_name : "MyLib";
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return 1;
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.groovy", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.groovy", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
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
      fprintf(f, "class %s extends Structure {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "    public %s %s\n",
                map_groovy_jna_type(&node->fields[j].type),
                node->fields[j].name);
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
      fprintf(f, "    %s %s(", map_groovy_jna_type(&node->return_or_base_type),
              node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s%s %s", j > 0 ? ", " : "",
                map_groovy_jna_type(&node->fields[j].type),
                node->fields[j].name);
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
  if (fopen_s(&f, filepath, "w") == 0) {
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%sSpec.groovy",
               config->output_dir, module_name);
  f = fopen(filepath, "w");
  if (f) {
#endif
    fprintf(f, "import spock.lang.Specification\n\n");
    fprintf(f, "class %sSpec extends Specification {\n", module_name);
    fprintf(f, "    def \"Library can be loaded\"() {\n");
    fprintf(f, "        expect:\n");
    fprintf(f, "        %s.INSTANCE != null\n", module_name);
    fprintf(f, "    }\n");
    fprintf(f, "}\n");
    fclose(f);
  }

  return 0;
}
