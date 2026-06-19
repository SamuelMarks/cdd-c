/* clang-format off */
#include "cdd_ffi_emit_typescript.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_deno_ffi_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "\"buffer\""; /* Commonly used for char / uint8_t ptr */
    }
    return "\"pointer\"";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "\"void\"";
  case CDD_FFI_KIND_BOOL:
    return "\"u8\""; /* Deno uses u8 or bool (bool since 1.32, but u8 is safe
                        fallback) */
  case CDD_FFI_KIND_INT8:
    return "\"i8\"";
  case CDD_FFI_KIND_UINT8:
    return "\"u8\"";
  case CDD_FFI_KIND_INT16:
    return "\"i16\"";
  case CDD_FFI_KIND_UINT16:
    return "\"u16\"";
  case CDD_FFI_KIND_INT32:
    return "\"i32\"";
  case CDD_FFI_KIND_UINT32:
    return "\"u32\"";
  case CDD_FFI_KIND_INT64:
    return "\"i64\"";
  case CDD_FFI_KIND_UINT64:
    return "\"u64\"";
  case CDD_FFI_KIND_FLOAT32:
    return "\"f32\"";
  case CDD_FFI_KIND_FLOAT64:
    return "\"f64\"";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "\"function\"";
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    return "\"i32\""; /* Enums/Typedefs might be i32. For struct by value,
                         complex, default to pointer */
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "\"pointer\"";
  default:
    return "\"pointer\"";
  }
}

static const char *get_ts_type(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "Uint8Array";
    }
    return "Deno.PointerValue";
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "boolean";
  case CDD_FFI_KIND_INT8:
  case CDD_FFI_KIND_UINT8:
  case CDD_FFI_KIND_INT16:
  case CDD_FFI_KIND_UINT16:
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_UINT32:
  case CDD_FFI_KIND_FLOAT32:
  case CDD_FFI_KIND_FLOAT64:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    return "number";
  case CDD_FFI_KIND_INT64:
  case CDD_FFI_KIND_UINT64:
    return "bigint | number";
  case CDD_FFI_KIND_FUNCTION_PTR:
  case CDD_FFI_KIND_OPAQUE_PTR:
  case CDD_FFI_KIND_STRUCT_REF:
    return "Deno.PointerValue";
  default:
    return "unknown";
  }
}

static int emit_deno_module(cdd_ffi_ir_t *ir,
                            const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.ts", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.ts", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "// Auto-generated Deno FFI bindings for %s\n\n", lib_name);

  /* Emit enums */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    if (node->kind == CDD_FFI_NODE_MACRO) {
      if (node->evaluated_value) {
        if (node->inferred_type == CDD_FFI_MACRO_TYPE_STRING) {
          fprintf(f, "export const %s: string = %s;\n\n", node->name,
                  node->evaluated_value);
        } else {
          fprintf(f, "export const %s: number = %s;\n\n", node->name,
                  node->evaluated_value);
        }
      }
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      if (node->doc) {
        fprintf(f, "/**\n * %s\n */\n", node->doc);
      }
      fprintf(f, "export enum %s {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->doc) {
          fprintf(f, "  /** %s */\n", var->doc);
        }
        if (var->value) {
          fprintf(f, "  %s = %s,\n", var->name, var->value);
        } else {
          fprintf(f, "  %s,\n", var->name);
        }
      }
      fprintf(f, "}\n\n");
    }
  }

  /* Setup Deno.dlopen symbols */
  fprintf(f, "const _libPath = Deno.env.get(\"%s_LIB_PATH\") ?? \"%s\";\n",
          lib_name, lib_name);
  fprintf(f, "export const lib = Deno.dlopen(_libPath, {\n");

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "  %s: {\n", node->name);
      fprintf(f, "    parameters: [\n");
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "      %s", get_deno_ffi_type(node->fields[j].type));
        if (j < node->fields_count - 1)
          fprintf(f, ",");
        fprintf(f, " /* %s */\n",
                node->fields[j].name ? node->fields[j].name : "arg");
      }
      fprintf(f, "    ],\n");
      fprintf(f, "    result: %s\n",
              get_deno_ffi_type(node->return_or_base_type));
      fprintf(f, "  },\n");
    }
  }
  fprintf(f, "});\n\n");

  /* Setup TypeScript Wrappers */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      int first_arg = 1;
      if (node->doc) {
        fprintf(f, "/**\n * %s\n */\n", node->doc);
      }
      fprintf(f, "export function %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        if (node->fields[j].intent != CDD_FFI_INTENT_OUT) {
          const char *arg_name =
              node->fields[j].name ? node->fields[j].name : "arg";
          /* handle TS keywords */
          if (strcmp(arg_name, "function") == 0)
            arg_name = "func";
          if (!first_arg)
            fprintf(f, ", ");
          fprintf(f, "%s: %s", arg_name, get_ts_type(node->fields[j].type));
          first_arg = 0;
        }
      }
      if (node->is_variadic) {
        if (!first_arg)
          fprintf(f, ", ");
        fprintf(f, "...args: any[]");
      }

      {
        int out_count = 0;
        for (j = 0; j < node->fields_count; j++) {
          if (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
              node->fields[j].intent == CDD_FFI_INTENT_INOUT)
            out_count++;
        }
        if (out_count > 0) {
          fprintf(f, "): { res: %s", get_ts_type(node->return_or_base_type));
          for (j = 0; j < node->fields_count; j++) {
            if (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
                node->fields[j].intent == CDD_FFI_INTENT_INOUT) {
              const char *arg_name =
                  node->fields[j].name ? node->fields[j].name : "arg";
              fprintf(f, ", %s: number", arg_name); /* Naively map all ptrs out
                                                       to number via dataview */
            }
          }
          fprintf(f, " } {\n");
        } else {
          fprintf(f, "): %s {\n", get_ts_type(node->return_or_base_type));
        }
      }

      if (node->is_variadic) {
        fprintf(f, "  // Warning: Deno FFI currently requires explicit type "
                   "packing for variadics.\n");
        fprintf(f, "  // You may need to modify the dlopen definition and pass "
                   "pre-packed Buffers.\n");
      }

      for (j = 0; j < node->fields_count; j++) {
        if (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
            node->fields[j].intent == CDD_FFI_INTENT_INOUT) {
          const char *arg_name =
              node->fields[j].name ? node->fields[j].name : "arg";
          fprintf(f, "  const out_%s = new Uint8Array(8);\n", arg_name);
          if (node->fields[j].intent == CDD_FFI_INTENT_INOUT) {
            fprintf(f,
                    "  new DataView(out_%s.buffer).setBigInt64(0, BigInt(%s), "
                    "true);\n",
                    arg_name, arg_name);
          }
        }
      }

      fprintf(f, "  const res = lib.symbols.%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "function") == 0)
          arg_name = "func";

        if (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
            node->fields[j].intent == CDD_FFI_INTENT_INOUT) {
          fprintf(f, "out_%s", arg_name);
        } else {
          fprintf(f, "%s", arg_name);
        }
        if (j < node->fields_count - 1 || node->is_variadic)
          fprintf(f, ", ");
      }
      if (node->is_variadic) {
        fprintf(f, "...args");
      }
      fprintf(f, ");\n");

      {
        int out_count = 0;
        for (j = 0; j < node->fields_count; j++) {
          if (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
              node->fields[j].intent == CDD_FFI_INTENT_INOUT)
            out_count++;
        }
        if (out_count > 0) {
          fprintf(f, "  return {\n    res");
          for (j = 0; j < node->fields_count; j++) {
            if (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
                node->fields[j].intent == CDD_FFI_INTENT_INOUT) {
              const char *arg_name =
                  node->fields[j].name ? node->fields[j].name : "arg";
              fprintf(f,
                      ",\n    %s: Number(new "
                      "DataView(out_%s.buffer).getBigInt64(0, true))",
                      arg_name, arg_name);
            }
          }
          fprintf(f, "\n  };\n");
        } else {
          fprintf(f, "  return res;\n");
        }
      }

      fprintf(f, "}\n\n");
    }
  }

  fclose(f);
  return 0;
}

int cdd_ffi_emit_typescript(cdd_ffi_ir_t *ir,
                            const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  return emit_deno_module(ir, config);
}
