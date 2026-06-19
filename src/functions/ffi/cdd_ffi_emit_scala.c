/* clang-format off */
#include "cdd_ffi_emit_scala.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static const char *map_scala_type(cdd_ffi_type_t *t) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      return "CString";
    }
    if (t->kind == CDD_FFI_KIND_STRUCT_REF ||
        t->kind == CDD_FFI_KIND_TYPEDEF_REF ||
        t->kind == CDD_FFI_KIND_ENUM_REF) {
      if (t->ref_name) {
        /* This isn't perfect since we can't easily return Ptr[StructName]
           dynamically from this static function, so we fallback to Ptr[Byte]
           for arbitrary pointers unless we build a dynamic string, which we
           avoid for simplicity here. */
        return "Ptr[Byte]";
      }
    }
    return "Ptr[Byte]";
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    return "Unit";
  case CDD_FFI_KIND_BOOL:
    return "CBool";
  case CDD_FFI_KIND_INT8:
    return "CSignedChar";
  case CDD_FFI_KIND_UINT8:
    return "CUnsignedChar";
  case CDD_FFI_KIND_INT16:
    return "CShort";
  case CDD_FFI_KIND_UINT16:
    return "CUnsignedShort";
  case CDD_FFI_KIND_INT32:
    return "CInt";
  case CDD_FFI_KIND_UINT32:
    return "CUnsignedInt";
  case CDD_FFI_KIND_INT64:
    return "CLongLong";
  case CDD_FFI_KIND_UINT64:
    return "CUnsignedLongLong";
  case CDD_FFI_KIND_FLOAT32:
    return "CFloat";
  case CDD_FFI_KIND_FLOAT64:
    return "CDouble";
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      return t->ref_name;
    }
    return "Ptr[Byte]";
  default:
    return "Ptr[Byte]";
  }
}

int cdd_ffi_emit_scala(cdd_ffi_ir_t *ir,
                       const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name = config->library_name ? config->library_name : "mylib";
  const char *module_name =
      config->module_name ? config->module_name : "Bindings";
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return 1;
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.scala", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.scala", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "// Auto-generated Scala Native bindings for %s\n\n", lib_name);
  fprintf(f, "import scalanative.unsafe._\n\n");

  /* Create an object to hold the types */
  fprintf(f, "object %sTypes {\n", module_name);

  /* Enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "  type %s = CInt\n", node->name);
      fprintf(f, "  object %s {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          fprintf(f, "    final val %s: %s = %s\n", var->name, node->name,
                  var->value);
        } else {
          fprintf(f, "    final val %s: %s = %lu\n", var->name, node->name,
                  (unsigned long)j);
        }
      }
      fprintf(f, "  }\n\n");
    }
  }

  /* Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      /* Scala Native supports up to CStruct22 */
      if (node->fields_count > 0 && node->fields_count <= 22) {
        fprintf(f, "  type %s = CStruct%lu[", node->name,
                (unsigned long)node->fields_count);
        for (j = 0; j < node->fields_count; j++) {
          fprintf(f, "%s%s", j > 0 ? ", " : "",
                  map_scala_type(&node->fields[j].type));
        }
        fprintf(f, "]\n");
      } else if (node->fields_count > 22) {
        fprintf(f,
                "  // Struct %s has more than 22 fields, Scala Native CStructN "
                "only supports up to 22.\n",
                node->name);
        fprintf(f, "  type %s = CArray[Byte, Nat._0] // Fallback mapping\n",
                node->name);
      } else {
        fprintf(f, "  type %s = CStruct0\n", node->name);
      }
    }
  }
  fprintf(f, "}\n\n");

  /* The actual extern object */
  fprintf(f, "import %sTypes._\n\n", module_name);
  fprintf(f, "@link(\"%s\")\n", lib_name);
  fprintf(f, "@extern\n");
  fprintf(f, "object %s {\n", module_name);

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "  def %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s%s: %s", j > 0 ? ", " : "", node->fields[j].name,
                map_scala_type(&node->fields[j].type));
      }
      fprintf(f, "): %s = extern\n",
              map_scala_type(&node->return_or_base_type));
    }
  }

  fprintf(f, "}\n");

  fclose(f);
  return 0;
}
