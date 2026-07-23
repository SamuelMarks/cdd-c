/* clang-format off */
#include "cdd_ffi_emit_clojure.h"

#include "../../cdd_api.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static const char *map_clojure_jna_type(cdd_ffi_type_t *t) {
  if (t->pointer_depth > 0) {
    if (t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) {
      if (t->is_const) {
        return "String";
      }
    }
    return "com.sun.jna.Pointer";
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
    return "com.sun.jna.Pointer";
  default:
    return "com.sun.jna.Pointer";
  }
}

enum cdd_c_error
cdd_ffi_emit_clojure(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  char filepath[1024];
  const char *lib_name = config->library_name ? config->library_name : "mylib";
  const char *module_name =
      config->module_name ? config->module_name : "bindings";
  size_t i, j;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.clj", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.clj", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, ";; Auto-generated Clojure JNA interop bindings for %s\n",
          lib_name);
  fprintf(f, "(ns %s\n", module_name);
  fprintf(f, "  (:import [com.sun.jna Native Pointer]\n");
  fprintf(f, "           [com.sun.jna.ptr IntByReference LongByReference "
             "FloatByReference DoubleByReference PointerByReference]\n");
  fprintf(f, "           [com.sun.jna Structure Library]))\n\n");

  /* Generate gen-interface for Library */
  fprintf(f, "(gen-interface\n");
  fprintf(f, "  :name %s.ILib\n", module_name);
  fprintf(f, "  :extends [com.sun.jna.Library]\n");
  fprintf(f, "  :methods [\n");

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "    ^%s [%s [",
              map_clojure_jna_type(&node->return_or_base_type), node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s%s", j > 0 ? " " : "",
                map_clojure_jna_type(&node->fields[j].type));
      }
      fprintf(f, "]]\n");
    }
  }
  fprintf(f, "  ])\n\n");

  /* Define struct classes via gen-class */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      fprintf(f, "(gen-class\n");
      fprintf(f, "  :name %s.%s\n", module_name, node->name);
      fprintf(f, "  :extends com.sun.jna.Structure\n");
      fprintf(f, "  :state state\n");
      fprintf(f, "  :init init\n");
      fprintf(f, "  :constructors {[] []}\n");
      fprintf(f, "  :methods [[getFieldOrder [] java.util.List]])\n\n");

      fprintf(f, "(defn -init-%s [] [[] (atom {})])\n", node->name);
      fprintf(f,
              "(defn -getFieldOrder-%s [_] (java.util.Arrays/asList "
              "(into-array String [\n",
              node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "  \"%s\"\n", node->fields[j].name);
      }
      fprintf(f, "])))\n\n");
    }
  }

  fprintf(f, "(def lib-instance\n");
  fprintf(f, "  (Native/load \"%s\" %s.ILib))\n\n", lib_name, module_name);

  /* Wrapper functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "(defn %s [", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "%s%s", j > 0 ? " " : "", node->fields[j].name);
      }
      fprintf(f, "]\n");
      fprintf(f, "  (.%s lib-instance", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, " %s", node->fields[j].name);
      }
      fprintf(f, "))\n\n");
    }
  }

  if (f)
    fclose(f);
  f = NULL;

  /* Write deps.edn snippet */
#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\deps.edn", config->output_dir);
  if (fopen_s(&f, filepath, "w") == 0) {
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/deps.edn", config->output_dir);
  f = fopen(filepath, "w");
  if (f) {
#endif
    fprintf(f, "{:deps {net.java.dev.jna/jna {:mvn/version \"5.13.0\"}}}\n");
    fclose(f);
  }

  return CDD_C_SUCCESS;
}
