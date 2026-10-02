#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_tcl.h"

#include "../../../include/c_cdd/safe_crt.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../cdd_api.h"
#include "../../win_compat_sym.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

cdd_c_error_t cdd_ffi_emit_tcl(cdd_ffi_ir_t *ir,
                               const cdd_generate_bindings_config_t *config) {
  FILE *c_f = NULL;
  FILE *pkg_f = NULL;
  char c_filepath[1024];
  char pkg_filepath[1024];
  const char *lib_name;
  size_t i, j;
  cdd_ffi_ir_node_t *node;

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  lib_name = (config->library_name) ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(c_filepath, sizeof(c_filepath), "%s\\%s_tcl.c",
               config->output_dir, lib_name);
  if (fopen_s(&c_f, c_filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(pkg_filepath, sizeof(pkg_filepath), "%s\\pkgIndex.tcl",
               config->output_dir);
  if (fopen_s(&pkg_f, pkg_filepath, "w") != 0) {
    fclose(c_f);
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(c_filepath, sizeof(c_filepath), "%s/%s_tcl.c",
               config->output_dir, lib_name);
#if defined(_MSC_VER)
  if (fopen_s(&c_f, c_filepath, "w") != 0)
    c_f = NULL;
#else
  c_f = fopen(c_filepath, "w");
#endif
  if (!c_f) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(pkg_filepath, sizeof(pkg_filepath), "%s/pkgIndex.tcl",
               config->output_dir);
#if defined(_MSC_VER)
  if (fopen_s(&pkg_f, pkg_filepath, "w") != 0)
    pkg_f = NULL;
#else
  pkg_f = fopen(pkg_filepath, "w");
#endif
#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      fclose(pkg_f);
      pkg_f = NULL;
    }
  }
#endif
  if (!pkg_f) {
    fclose(c_f);
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  fprintf(c_f, "/* Auto-generated Tcl Native C Extension for %s */\n",
          lib_name);
  fprintf(c_f, "#include <tcl.h>\n");
  fprintf(c_f, "#include \"%s.h\"\n\n", lib_name);

  /* Generate wrappers */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(c_f,
              "static int Tcl_%s(ClientData clientData, Tcl_Interp *interp, "
              "int objc, Tcl_Obj *const objv[]) {\n",
              node->name);

      if ((unsigned long)node->fields_count > 0) {
        fprintf(c_f, "    if (objc != %lu + 1) {\n",
                (unsigned long)node->fields_count);
        fprintf(c_f, "        Tcl_WrongNumArgs(interp, 1, objv, \"");
        for (j = 0; j < (unsigned long)node->fields_count; j++) {
          fprintf(c_f, "%s ", node->fields[j].name);
        }
        fprintf(c_f, "\");\n");
        fprintf(c_f, "        return TCL_ERROR;\n");
        fprintf(c_f, "    }\n");
      }

      /* Argument extraction */
      for (j = 0; j < (unsigned long)node->fields_count; j++) {
        const char *arg_name = node->fields[j].name;
        if (node->fields[j].type.pointer_depth > 0 &&
            (node->fields[j].type.kind == CDD_FFI_KIND_INT8 ||
             node->fields[j].type.kind == CDD_FFI_KIND_UINT8)) {
          fprintf(c_f, "    int _len_%s = 0;\n", arg_name);
          fprintf(
              c_f,
              "    char *_c_%s = Tcl_GetStringFromObj(objv[%lu], &_len_%s);\n",
              arg_name, (unsigned long)(j + 1), arg_name);
        } else if (node->fields[j].type.pointer_depth > 0 ||
                   node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(c_f,
                  "    void *_c_%s = NULL; /* Stub: Pointer passing "
                  "unsupported in basic Tcl generator */\n",
                  arg_name);
        } else if (node->fields[j].type.kind == CDD_FFI_KIND_FLOAT32 ||
                   node->fields[j].type.kind == CDD_FFI_KIND_FLOAT64) {
          fprintf(c_f, "    double _c_%s = 0.0;\n", arg_name);
          fprintf(c_f,
                  "    if (Tcl_GetDoubleFromObj(interp, objv[%lu], &_c_%s) != "
                  "TCL_OK) return TCL_ERROR;\n",
                  (unsigned long)(j + 1), arg_name);
        } else if (node->fields[j].type.kind == CDD_FFI_KIND_BOOL) {
          fprintf(c_f, "    int _c_%s = 0;\n", arg_name);
          fprintf(c_f,
                  "    if (Tcl_GetBooleanFromObj(interp, objv[%lu], &_c_%s) != "
                  "TCL_OK) return TCL_ERROR;\n",
                  (unsigned long)(j + 1), arg_name);
        } else {
          fprintf(c_f, "    int _c_%s = 0;\n", arg_name);
          fprintf(c_f,
                  "    if (Tcl_GetIntFromObj(interp, objv[%lu], &_c_%s) != "
                  "TCL_OK) return TCL_ERROR;\n",
                  (unsigned long)(j + 1), arg_name);
        }
      }

      /* Call the function */
      fprintf(c_f, "    ");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID ||
          node->return_or_base_type.pointer_depth > 0) {
        if (node->return_or_base_type.pointer_depth > 0 &&
            (node->return_or_base_type.kind == CDD_FFI_KIND_INT8 ||
             node->return_or_base_type.kind == CDD_FFI_KIND_UINT8)) {
          fprintf(c_f, "const char* _c_ret = ");
        } else if (node->return_or_base_type.pointer_depth > 0 ||
                   node->return_or_base_type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(c_f, "void* _c_ret = ");
        } else if (node->return_or_base_type.kind == CDD_FFI_KIND_FLOAT32) {
          fprintf(c_f, "float _c_ret = ");
        } else if (node->return_or_base_type.kind == CDD_FFI_KIND_FLOAT64) {
          fprintf(c_f, "double _c_ret = ");
        } else if (node->return_or_base_type.kind == CDD_FFI_KIND_BOOL) {
          fprintf(c_f, "int _c_ret = ");
        } else {
          fprintf(c_f, "int _c_ret = ");
        }
      }
      fprintf(c_f, "%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        if (j > 0)
          fprintf(c_f, ", ");
        if (node->fields[j].type.kind == CDD_FFI_KIND_FLOAT32 &&
            node->fields[j].type.pointer_depth == 0) {
          fprintf(c_f, "(float)_c_%s", node->fields[j].name);
        } else if ((node->fields[j].type.kind == CDD_FFI_KIND_INT8 ||
                    node->fields[j].type.kind == CDD_FFI_KIND_UINT8 ||
                    node->fields[j].type.kind == CDD_FFI_KIND_INT16 ||
                    node->fields[j].type.kind == CDD_FFI_KIND_UINT16) &&
                   node->fields[j].type.pointer_depth == 0) {
          fprintf(c_f, "(%s)_c_%s",
                  node->fields[j].type.kind == CDD_FFI_KIND_INT8
                      ? "int8_t"
                      : (node->fields[j].type.kind == CDD_FFI_KIND_UINT8
                             ? "uint8_t"
                             : (node->fields[j].type.kind == CDD_FFI_KIND_INT16
                                    ? "int16_t"
                                    : "uint16_t")),
                  node->fields[j].name);
        } else {
          fprintf(c_f, "_c_%s", node->fields[j].name);
        }
      }
      fprintf(c_f, ");\n");

      /* Convert Return value */
      if (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
          node->return_or_base_type.pointer_depth == 0) {
        fprintf(c_f, "    return TCL_OK;\n");
      } else {
        if (node->return_or_base_type.pointer_depth > 0 &&
            (node->return_or_base_type.kind == CDD_FFI_KIND_INT8 ||
             node->return_or_base_type.kind == CDD_FFI_KIND_UINT8)) {
          fprintf(c_f, "    if (_c_ret) Tcl_SetObjResult(interp, "
                       "Tcl_NewStringObj(_c_ret, -1));\n");
        } else if (node->return_or_base_type.pointer_depth > 0 ||
                   node->return_or_base_type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(
              c_f,
              "    /* Tcl returning raw pointer not natively supported */\n");
        } else if (node->return_or_base_type.kind == CDD_FFI_KIND_FLOAT32 ||
                   node->return_or_base_type.kind == CDD_FFI_KIND_FLOAT64) {
          fprintf(c_f, "    Tcl_SetObjResult(interp, "
                       "Tcl_NewDoubleObj((double)_c_ret));\n");
        } else if (node->return_or_base_type.kind == CDD_FFI_KIND_BOOL) {
          fprintf(c_f,
                  "    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(_c_ret));\n");
        } else {
          fprintf(
              c_f,
              "    Tcl_SetObjResult(interp, Tcl_NewIntObj((int)_c_ret));\n");
        }
        fprintf(c_f, "    return TCL_OK;\n");
      }
      fprintf(c_f, "}\n\n");
    }
  }

  /* Init function */
  {
    /* Tcl standard dictates Init function must match library name capitalized:
     * e.g., Mylib_Init */
    char tcl_init_name[64];
    size_t len = strlen(lib_name);

    if (len >= sizeof(tcl_init_name))
      len = sizeof(tcl_init_name) - 1;
    memcpy(tcl_init_name, lib_name, len);
    tcl_init_name[len] = '\0';
    tcl_init_name[0] = (char)toupper((unsigned char)tcl_init_name[0]);

    fprintf(c_f, "int %s_Init(Tcl_Interp *interp) {\n", tcl_init_name);
    fprintf(c_f, "    if (Tcl_InitStubs(interp, \"8.1\", 0) == NULL) {\n");
    fprintf(c_f, "        return TCL_ERROR;\n");
    fprintf(c_f, "    }\n");

    for (i = 0; i < ir->nodes_count; i++) {
      node = &ir->nodes[i];
      if (node->kind == CDD_FFI_NODE_FUNCTION) {
        fprintf(c_f,
                "    Tcl_CreateObjCommand(interp, \"%s::%s\", Tcl_%s, NULL, "
                "NULL);\n",
                lib_name, node->name, node->name);
      }
    }

    fprintf(c_f, "    Tcl_PkgProvide(interp, \"%s\", \"1.0\");\n", lib_name);
    fprintf(c_f, "    return TCL_OK;\n");
    fprintf(c_f, "}\n");
  }

  fclose(c_f);

  /* pkgIndex.tcl */
  fprintf(pkg_f,
          "package ifneeded %s 1.0 [list load [file join $dir lib%s.so]]\n",
          lib_name, lib_name);
  fclose(pkg_f);

  return CDD_C_SUCCESS;
}
