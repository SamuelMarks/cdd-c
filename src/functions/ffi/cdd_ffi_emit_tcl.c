/* clang-format off */
#include "cdd_ffi_emit_tcl.h"

#include "../../cdd_api.h"
#include "../../win_compat_sym.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

int cdd_ffi_emit_tcl(cdd_ffi_ir_t *ir,
                     const cdd_generate_bindings_config_t *config) {
  FILE *c_f = NULL;
  FILE *pkg_f = NULL;
  char c_filepath[1024];
  char pkg_filepath[1024];
  const char *lib_name;
  size_t i, j;
  cdd_ffi_ir_node_t *node;

  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(c_filepath, sizeof(c_filepath), "%s\\%s_tcl.c",
               config->output_dir, lib_name);
  if (fopen_s(&c_f, c_filepath, "w") != 0) {
    return 1;
  }
  CDD_SNPRINTF(pkg_filepath, sizeof(pkg_filepath), "%s\\pkgIndex.tcl",
               config->output_dir);
  if (fopen_s(&pkg_f, pkg_filepath, "w") != 0) {
    fclose(c_f);
    return 1;
  }
#else
  CDD_SNPRINTF(c_filepath, sizeof(c_filepath), "%s/%s_tcl.c",
               config->output_dir, lib_name);
  c_f = fopen(c_filepath, "w");
  if (!c_f) {
    return 1;
  }
  CDD_SNPRINTF(pkg_filepath, sizeof(pkg_filepath), "%s/pkgIndex.tcl",
               config->output_dir);
  pkg_f = fopen(pkg_filepath, "w");
  if (!pkg_f) {
    fclose(c_f);
    return 1;
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

      if (node->fields_count > 0) {
        fprintf(c_f, "    if (objc != %" CDD_SIZE_T_FMT " + 1) {\n",
                node->fields_count);
        fprintf(c_f, "        Tcl_WrongNumArgs(interp, 1, objv, \"");
        for (j = 0; j < node->fields_count; j++) {
          fprintf(c_f, "%s ", node->fields[j].name);
        }
        fprintf(c_f, "\");\n");
        fprintf(c_f, "        return TCL_ERROR;\n");
        fprintf(c_f, "    }\n");
      }

      /* Basic argument fetching (simplified stub) */
      for (j = 0; j < node->fields_count; j++) {
        fprintf(c_f,
                "    /* TODO: Fetch objv[%" CDD_SIZE_T_FMT
                "] into local %s */\n",
                j + 1, node->fields[j].name);
      }

      fprintf(c_f, "    /* TODO: Call %s(...) */\n", node->name);

      if (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
          node->return_or_base_type.pointer_depth == 0) {
        fprintf(c_f, "    return TCL_OK;\n");
      } else {
        fprintf(c_f, "    /* TODO: Convert result to Tcl_Obj* and "
                     "Tcl_SetObjResult */\n");
        fprintf(c_f, "    return TCL_OK;\n");
      }
      fprintf(c_f, "}\n\n");
    }
  }

  /* Init function */
  {
    /* Tcl standard dictates Init function must match library name capitalized:
     * e.g., Mylib_Init */
    char tcl_init_name[256];
    size_t len = strlen(lib_name);
    if (len > sizeof(tcl_init_name) - 1)
      len = sizeof(tcl_init_name) - 1;
    strncpy(tcl_init_name, lib_name, len);
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

  return 0;
}
