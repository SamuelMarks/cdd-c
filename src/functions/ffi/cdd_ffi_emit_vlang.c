/* clang-format off */
#include "cdd_ffi_emit_vlang.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static const char *map_vlang_type(cdd_ffi_type_t *t) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      return "&char";
    }
    if (t->kind == CDD_FFI_KIND_STRUCT_REF ||
        t->kind == CDD_FFI_KIND_TYPEDEF_REF ||
        t->kind == CDD_FFI_KIND_ENUM_REF) {
      if (t->ref_name) {
        /* In V, pointers to C structs are mapped as `&C.StructName`
           We can't easily format dynamic strings here, so returning `voidptr`
           is a safe fallback for general pointers. */
        return "voidptr";
      }
    }
    return "voidptr";
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    return "void"; /* V uses void for return types in C interop */
  case CDD_FFI_KIND_BOOL:
    return "bool";
  case CDD_FFI_KIND_INT8:
    return "i8";
  case CDD_FFI_KIND_UINT8:
    return "u8";
  case CDD_FFI_KIND_INT16:
    return "i16";
  case CDD_FFI_KIND_UINT16:
    return "u16";
  case CDD_FFI_KIND_INT32:
    return "int";
  case CDD_FFI_KIND_UINT32:
    return "u32";
  case CDD_FFI_KIND_INT64:
    return "i64";
  case CDD_FFI_KIND_UINT64:
    return "u64";
  case CDD_FFI_KIND_FLOAT32:
    return "f32";
  case CDD_FFI_KIND_FLOAT64:
    return "f64";
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      /* Ideally "C.StructName", but we return voidptr as fallback */
      return "voidptr";
    }
    return "voidptr";
  default:
    return "voidptr";
  }
}

int cdd_ffi_emit_vlang(cdd_ffi_ir_t *ir,
                       const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name;
  const char *module_name;
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  lib_name = config->library_name ? config->library_name : "mylib";
  module_name = config->module_name ? config->module_name : "bindings";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.v", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.v", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "// Auto-generated Vlang Native C interop bindings for %s\n\n",
          lib_name);
  fprintf(f, "module %s\n\n", module_name);

  /* Linker and Header pragmas */
  fprintf(f, "#flag -l%s\n", lib_name);
  /* Replace module_name.h with actual header if available */
  fprintf(f, "#include \"%s.h\"\n\n", module_name);

  /* Structures */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      fprintf(f, "[typedef]\n");
      fprintf(f, "struct C.%s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "pub mut:\n");
        /* Note: V uses `name type`, not `type name` */
        fprintf(f, "\t%s %s\n", node->fields[j].name,
                map_vlang_type(&node->fields[j].type));
      }
      fprintf(f, "}\n\n");
    } else if (node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "[typedef]\n");
      fprintf(f, "union C.%s {\n", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "pub mut:\n");
        fprintf(f, "\t%s %s\n", node->fields[j].name,
                map_vlang_type(&node->fields[j].type));
      }
      fprintf(f, "}\n\n");
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      fprintf(f, "enum C.%s {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->value) {
          /* V enums don't natively support arbitrary string values unless
           * strictly integer constants */
          /* For C interop, we usually just list them */
          fprintf(f, "\t%s\n", var->name);
        } else {
          fprintf(f, "\t%s\n", var->name);
        }
      }
      fprintf(f, "}\n\n");
    }
  }

  /* Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_type = NULL;

      fprintf(f, "fn C.%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s%s %s", j > 0 ? ", " : "", node->fields[j].name,
                map_vlang_type(&node->fields[j].type));
      }

      ret_type = map_vlang_type(&node->return_or_base_type);
      if (strcmp(ret_type, "void") != 0) {
        fprintf(f, ") %s\n", ret_type);
      } else {
        fprintf(f, ")\n");
      }
    }
  }

  fprintf(f, "\n/* \n");
  fprintf(f, " * Optional idiomatic V wrapper module template:\n");
  fprintf(f, " *\n");
  fprintf(f, " * pub fn my_func() {\n");
  fprintf(f, " *     unsafe { C.my_func() }\n");
  fprintf(f, " * }\n");
  fprintf(f, " */\n");

  fclose(f);
  return 0;
}
