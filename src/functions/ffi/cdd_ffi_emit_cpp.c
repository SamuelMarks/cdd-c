/**
 * @file cdd_ffi_emit_cpp.c
 * @brief FFI binding emitter for Cpp.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_cpp.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_ffi_emit_common.h"
/* clang-format on */

static cdd_c_error_t get_cpp_type(cdd_ffi_type_t type, const char **out_type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      *out_type = type.is_const ? "const char*" : "char*";
      return CDD_C_SUCCESS;
    }
    *out_type = "void*";
    return CDD_C_SUCCESS;
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = "void";
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = "bool";
    break;
  case CDD_FFI_KIND_INT8:
    *out_type = "int8_t";
    break;
  case CDD_FFI_KIND_UINT8:
    *out_type = "uint8_t";
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = "int16_t";
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = "uint16_t";
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = "int32_t";
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = "uint32_t";
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = "int64_t";
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = "uint64_t";
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = "float";
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = "double";
    break;
  case CDD_FFI_KIND_FUNCTION_PTR:
    *out_type = "void* /* callback */";
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    *out_type = type.ref_name ? type.ref_name : "void*";
    break;
  case CDD_FFI_KIND_ENUM_REF:
    *out_type = type.ref_name ? type.ref_name : "int";
    break;
  case CDD_FFI_KIND_TYPEDEF_REF:
    *out_type = type.ref_name ? type.ref_name : "void*";
    break;
  case CDD_FFI_KIND_OPAQUE_PTR:
    *out_type = "void*";
    break;
  default:
    *out_type = "void*";
    break;
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
emit_cpp_hpp(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name;

  lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.hpp", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.hpp", config->output_dir,
               lib_name);
#if defined(_MSC_VER)
  if (fopen_s(&f, filepath, "w") != 0)
    f = NULL;
#else
  f = fopen(filepath, "w");
#endif
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "// Auto-generated C++ bindings for %s\n", lib_name);
  fprintf(f, "#pragma once\n\n");
  fprintf(f, "#include <cstdint>\n");
  fprintf(f, "#include <string>\n");
  fprintf(f, "#include <memory>\n");
  fprintf(f, "#include <vector>\n");
  fprintf(f, "#include <stdexcept>\n\n");

  fprintf(f, "extern \"C\" {\n");
  if (config->input) {
    fprintf(f, "  #include \"%s\"\n", config->input);
  }
  fprintf(f, "}\n\n");

  fprintf(f, "namespace %s {\n\n", lib_name);

  /* Forward decls */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      fprintf(f, "class %s_Wrapper;\n", node->name);
    }
  }
  fprintf(f, "\n");

  /* Struct and Trampoline Wrappers */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (strstr(node->name, "_Trampoline")) {
        char base_name[256];
        /* Emit the C ABI struct for Trampoline */
        fprintf(f, "extern \"C\" {\n");
        fprintf(f, "    struct %s {\n", node->name);
        for (j = 0; j < node->fields_count; j++) {
          const char *ctype = NULL;
          get_cpp_type(node->fields[j].type, &ctype);
          fprintf(f, "        %s %s;\n", ctype, node->fields[j].name);
        }
        fprintf(f, "    };\n");
        fprintf(f, "}\n\n");

        /* Determine base class name */
        {
          size_t copy_len =
              strlen(node->name) > 11 ? strlen(node->name) - 11 : 0;
          if (copy_len > sizeof(base_name) - 1)
            copy_len = sizeof(base_name) - 1;
          memcpy(base_name, node->name, copy_len);
          base_name[copy_len] = '\0';
        }

        fprintf(f, "class %s_Impl : public ::%s {\n", node->name, base_name);
        fprintf(f, "private:\n");
        fprintf(f, "    %s c_api;\n", node->name);
        fprintf(f, "public:\n");
        fprintf(f, "    %s_Impl(const %s& api) : c_api(api) {}\n", node->name,
                node->name);
        fprintf(f, "    virtual ~%s_Impl() {\n", node->name);
        fprintf(f, "        if (c_api.cb_Release) {\n");
        fprintf(f, "            void (*release_fn)(void*) = (void "
                   "(*)(void*))c_api.cb_Release;\n");
        fprintf(f, "            release_fn(c_api.target_lang_ctx);\n");
        fprintf(f, "        }\n");
        fprintf(f, "    }\n");

        /* Emit virtual methods */
        for (j = 3; j < node->fields_count;
             j++) { /* skip ctx, AddRef, Release */
          const char *fn_name = node->fields[j].name + 3; /* skip cb_ */
          fprintf(f, "    virtual void %s() {\n", fn_name);
          fprintf(f, "        if (c_api.%s) {\n", node->fields[j].name);
          fprintf(
              f, "            void (*cb)(void*) = (void (*)(void*))c_api.%s;\n",
              node->fields[j].name);
          fprintf(f, "#if defined(CDD_TARGET_PYTHON)\n");
          fprintf(
              f,
              "            /* Thread State Management (GIL) for Python */\n");
          fprintf(
              f,
              "            PyGILState_STATE gstate = PyGILState_Ensure();\n");
          fprintf(f, "#elif defined(CDD_TARGET_JAVA)\n");
          fprintf(f,
                  "            /* Thread State Management (JNI) for Java */\n");
          fprintf(
              f, "            /* AttachCurrentThread logic would be here */\n");
          fprintf(f, "#endif\n");
          fprintf(f, "            cb(c_api.target_lang_ctx);\n");
          fprintf(f, "#if defined(CDD_TARGET_PYTHON)\n");
          fprintf(f, "            PyGILState_Release(gstate);\n");
          fprintf(f, "#endif\n");
          fprintf(f, "        }\n");
          fprintf(f, "    }\n");
        }
        fprintf(f, "};\n\n");
        continue; /* Skip generating Wrapper for Trampoline */
      }
      if (node->doc)
        fprintf(f, "/**\n * %s\n */\n", node->doc);
      fprintf(f, "class %s_Wrapper {\n", node->name);
      fprintf(f, "private:\n");
      fprintf(f, "    ::%s inner;\n", node->name);
      fprintf(f, "public:\n");
      fprintf(f, "    %s_Wrapper() : inner{} {}\n", node->name);
      fprintf(f, "    %s_Wrapper(const ::%s& c_struct) : inner(c_struct) {}\n",
              node->name, node->name);
      fprintf(f, "    ::%s* get() { return &inner; }\n", node->name);
      fprintf(f, "    const ::%s* get() const { return &inner; }\n",
              node->name);

      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ctype = NULL;
        get_cpp_type(node->fields[j].type, &ctype);
        fprintf(f, "    %s get_%s() const { return inner.%s; }\n", ctype, fname,
                fname);
        fprintf(f, "    void set_%s(%s val) { inner.%s = val; }\n", fname,
                ctype, fname);
      }
      fprintf(f, "};\n\n");
    }
  }

  /* Function Wrappers */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      const char *ret_ctype = NULL;
      cdd_ffi_emit_trivia(f, node->leading_trivia);
      if (node->doc)
        fprintf(f, "/**\n * %s\n */\n", node->doc);
      get_cpp_type(node->return_or_base_type, &ret_ctype);
      fprintf(f, "inline %s %s(", ret_ctype, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        const char *arg_ctype = NULL;
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "this") == 0)
          arg_name = "this_";
        get_cpp_type(node->fields[j].type, &arg_ctype);
        fprintf(f, "%s %s", arg_ctype, arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      if (node->fields_count > 0)
        fprintf(f, ", ");
      fprintf(f, "cdd_ffi_error_t* err) {\n");
      fprintf(f, "    if (err) { err->code = 0; err->message = nullptr; }\n");
      fprintf(f, "    try {\n");

      fprintf(f, "        ");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "return ");
      }
      fprintf(f, "::%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "this") == 0)
          arg_name = "this_";
        fprintf(f, "%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ");\n");
      fprintf(f, "    } catch (const std::exception& e) {\n");
      fprintf(f, "        if (err) {\n");
      fprintf(f, "            err->code = 1;\n");
      fprintf(f, "            err->message = const_cast<char*>(e.what());\n");
      fprintf(f, "        }\n");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "        return %s{};\n", ret_ctype);
      }
      fprintf(f, "    } catch (...) {\n");
      fprintf(f, "        if (err) {\n");
      fprintf(f, "            err->code = 2;\n");
      fprintf(f, "            err->message = const_cast<char*>(\"Unknown C++ "
                 "exception\");\n");
      fprintf(f, "        }\n");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "        return %s{};\n", ret_ctype);
      }
      fprintf(f, "    }\n");
      fprintf(f, "}\n\n");
    }
  }

  fprintf(f, "} // namespace %s\n", lib_name);

  fclose(f);
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_ffi_emit_cpp(cdd_ffi_ir_t *ir,
                               const cdd_generate_bindings_config_t *config) {
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  return emit_cpp_hpp(ir, config);
}
