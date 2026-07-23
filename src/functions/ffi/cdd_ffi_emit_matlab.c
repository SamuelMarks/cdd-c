/* clang-format off */
#include "c_cdd/safe_crt.h"
#include "cdd_ffi_emit_matlab.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

static enum cdd_c_error
emit_matlab_mex(cdd_ffi_ir_t *ir,
                const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s_mex.c", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s_mex.c", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "/* Auto-generated MATLAB MEX FFI bindings for %s */\n\n",
          lib_name);
  fprintf(f, "#include \"mex.h\"\n");
  if (config->input) {
    fprintf(f, "#include \"%s\"\n", config->input);
  }
  fprintf(f, "#include <string.h>\n\n ");

  /* We generate a single mexFunction dispatcher for all exported functions */
  fprintf(f, "void mexFunction(int nlhs, mxArray *plhs[],\n");
  fprintf(f, "                 int nrhs, const mxArray *prhs[]) {\n");
  fprintf(f, "    char func_name[256];\n\n");

  fprintf(f, "    if (nrhs < 1) {\n");
  fprintf(f, "        mexErrMsgIdAndTxt(\"cdd:mex:nrhs\", \"First input must "
             "be the function name string.\");\n");
  fprintf(f, "    }\n\n");

  fprintf(
      f,
      "    if (mxGetString(prhs[0], func_name, sizeof(func_name)) != 0) {\n");
  fprintf(f, "        mexErrMsgIdAndTxt(\"cdd:mex:func_name\", \"Could not "
             "convert function name to string.\");\n");
  fprintf(f, "    }\n\n");

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (i > 0) {
        fprintf(f, "    else if (strcmp(func_name, \"%s\") == 0) {\n",
                node->name);
      } else {
        fprintf(f, "    if (strcmp(func_name, \"%s\") == 0) {\n", node->name);
      }

      if (node->fields_count > 0) {
        fprintf(f, "        if (nrhs < %" CDD_PRIz ") {\n",
                node->fields_count + 1);
        fprintf(f, "            mexErrMsgIdAndTxt(\"cdd:mex:nrhs\", \"Not "
                   "enough input arguments.\");\n");
        fprintf(f, "        }\n");
      }

      fprintf(f, "        /* Map MATLAB inputs to C types here... */\n");
      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "        /* prhs[%" CDD_PRIz "] -> %s */\n", j + 1,
                node->fields[j].name ? node->fields[j].name : "arg");
      }

      fprintf(f, "\n        /* Call original function */\n");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "        /* result = */ %s(...);\n", node->name);
      } else {
        fprintf(f, "        %s(...);\n", node->name);
      }

      fprintf(f, "\n        /* Map C return to MATLAB plhs[0] here... */\n");
      fprintf(f, "    }\n");
    }
  }

  if (ir->nodes_count > 0) {
    fprintf(f, "    else {\n");
    fprintf(f, "        mexErrMsgIdAndTxt(\"cdd:mex:unknown_func\", \"Unknown "
               "function name.\");\n");
    fprintf(f, "    }\n");
  }

  fprintf(f, "}\n");

  fclose(f);
  return CDD_C_SUCCESS;
}

static enum cdd_c_error
emit_matlab_m(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.m", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.m", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(f, "classdef %s\n", lib_name);
  fprintf(f, "    %% Auto-generated MATLAB bindings for %s\n\n", lib_name);
  fprintf(f, "    methods (Static)\n");

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->doc)
        fprintf(f, "        %% %s\n", node->doc);
      fprintf(f, "        function ");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "ret = ");
      }
      fprintf(f, "%s(", node->name);

      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        /* avoid matlab keywords */
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "function") == 0)
          arg_name = "func";
        fprintf(f, "%s", arg_name);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ")\n");

      fprintf(f, "            ");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "ret = ");
      }

      fprintf(f, "%s_mex('%s'", lib_name, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        if (strcmp(arg_name, "function") == 0)
          arg_name = "func";
        fprintf(f, ", %s", arg_name);
      }
      fprintf(f, ");\n");
      fprintf(f, "        end\n\n");
    }
  }

  fprintf(f, "    end\n");
  fprintf(f, "end\n");

  fclose(f);
  return CDD_C_SUCCESS;
}

enum cdd_c_error
cdd_ffi_emit_matlab(cdd_ffi_ir_t *ir,
                    const cdd_generate_bindings_config_t *config) {
  int rc;
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  rc = emit_matlab_mex(ir, config);
  if (rc != 0)
    return rc;

  return emit_matlab_m(ir, config);
}
