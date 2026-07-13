/* clang-format off */
#include "cdd_ffi_emit_perl.h"

#include "../../cdd_api.h"
#include "../../win_compat_sym.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

static void map_perl_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      CDD_SNPRINTF(out_type, out_sz, "string");
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, "opaque");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    CDD_SNPRINTF(out_type, out_sz, "void");
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz,
                 "bool"); /* Requires FFI::Platypus::Type::StringArray/Bool
                             usually, but Platypus maps bool */
    break;
  case CDD_FFI_KIND_INT8:
    CDD_SNPRINTF(out_type, out_sz, "sint8");
    break;
  case CDD_FFI_KIND_UINT8:
    CDD_SNPRINTF(out_type, out_sz, "uint8");
    break;
  case CDD_FFI_KIND_INT16:
    CDD_SNPRINTF(out_type, out_sz, "sint16");
    break;
  case CDD_FFI_KIND_UINT16:
    CDD_SNPRINTF(out_type, out_sz, "uint16");
    break;
  case CDD_FFI_KIND_INT32:
    CDD_SNPRINTF(out_type, out_sz, "sint32");
    break;
  case CDD_FFI_KIND_UINT32:
    CDD_SNPRINTF(out_type, out_sz, "uint32");
    break;
  case CDD_FFI_KIND_INT64:
    CDD_SNPRINTF(out_type, out_sz, "sint64");
    break;
  case CDD_FFI_KIND_UINT64:
    CDD_SNPRINTF(out_type, out_sz, "uint64");
    break;
  case CDD_FFI_KIND_FLOAT32:
    CDD_SNPRINTF(out_type, out_sz, "float");
    break;
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, "double");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "record(%s)", t->ref_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "opaque");
    }
    break;
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "%s", t->ref_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "opaque");
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, "opaque");
    break;
  }
}

static void map_xs_type(cdd_ffi_type_t *t, char *out_type, size_t out_sz) {
  if (t->pointer_depth > 0) {
    if ((t->kind == CDD_FFI_KIND_INT8 || t->kind == CDD_FFI_KIND_UINT8) &&
        t->is_const) {
      /* LCOV_EXCL_START */
      CDD_SNPRINTF(out_type, out_sz, "const char *");
      return;
      /* LCOV_EXCL_STOP */
    }
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "%s *", t->ref_name);
      return;
    }
    CDD_SNPRINTF(out_type, out_sz, "void *");
    return;
  }
  switch (t->kind) {
  case CDD_FFI_KIND_VOID:
    CDD_SNPRINTF(out_type, out_sz, "void");
    break;
  case CDD_FFI_KIND_BOOL:
    CDD_SNPRINTF(out_type, out_sz, "bool");
    break;
  case CDD_FFI_KIND_INT8:
    CDD_SNPRINTF(out_type, out_sz, "int8_t");
    break;
  case CDD_FFI_KIND_UINT8:
    CDD_SNPRINTF(out_type, out_sz, "uint8_t");
    break;
  case CDD_FFI_KIND_INT16:
    CDD_SNPRINTF(out_type, out_sz, "int16_t");
    break;
  case CDD_FFI_KIND_UINT16:
    CDD_SNPRINTF(out_type, out_sz, "uint16_t");
    break;
  case CDD_FFI_KIND_INT32:
    CDD_SNPRINTF(out_type, out_sz, "int32_t");
    break;
  case CDD_FFI_KIND_UINT32:
    CDD_SNPRINTF(out_type, out_sz, "uint32_t");
    break;
  case CDD_FFI_KIND_INT64:
    CDD_SNPRINTF(out_type, out_sz, "int64_t");
    break;
  case CDD_FFI_KIND_UINT64:
    CDD_SNPRINTF(out_type, out_sz, "uint64_t");
    break;
  case CDD_FFI_KIND_FLOAT32:
    CDD_SNPRINTF(out_type, out_sz, "float");
    break;
  case CDD_FFI_KIND_FLOAT64:
    CDD_SNPRINTF(out_type, out_sz, "double");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (t->ref_name) {
      CDD_SNPRINTF(out_type, out_sz, "%s", t->ref_name);
    } else {
      CDD_SNPRINTF(out_type, out_sz, "int");
    }
    break;
  default:
    CDD_SNPRINTF(out_type, out_sz, "int");
    break;
  }
}

enum cdd_c_error
cdd_ffi_emit_perl(cdd_ffi_ir_t *ir,
                  const cdd_generate_bindings_config_t *config) {
  FILE *f = NULL;
  FILE *xs_f = NULL;
  FILE *make_f = NULL;
  FILE *typemap_f = NULL;
  char filepath[1024];
  char xs_filepath[1024];
  char make_filepath[1024];
  char typemap_filepath[1024];
  const char *lib_name;
  const char *module_name;
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_enum_variant_t *var;
  char type_str[256];
  char ret_type_str[256];

  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  lib_name = config->library_name ? config->library_name : "mylib";
  module_name = config->module_name ? config->module_name : "MyLib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.pm", config->output_dir,
               module_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(xs_filepath, sizeof(xs_filepath), "%s\\%s.xs",
               config->output_dir, module_name);
  if (fopen_s(&xs_f, xs_filepath, "w") != 0) {
    fclose(f);
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(make_filepath, sizeof(make_filepath), "%s\\Makefile.PL",
               config->output_dir);
  if (fopen_s(&make_f, make_filepath, "w") != 0) {
    fclose(f);
    fclose(xs_f);
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(typemap_filepath, sizeof(typemap_filepath), "%s\\typemap",
               config->output_dir);
  if (fopen_s(&typemap_f, typemap_filepath, "w") != 0) {
    fclose(f);
    fclose(xs_f);
    fclose(make_f);
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.pm", config->output_dir,
               module_name);
  f = fopen(filepath, "w");
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
  CDD_SNPRINTF(xs_filepath, sizeof(xs_filepath), "%s/%s.xs", config->output_dir,
               module_name);
  xs_f = fopen(xs_filepath, "w");
  if (!xs_f) {
    /* LCOV_EXCL_START */
    fclose(f);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }
  CDD_SNPRINTF(make_filepath, sizeof(make_filepath), "%s/Makefile.PL",
               config->output_dir);
  make_f = fopen(make_filepath, "w");
  if (!make_f) {
    /* LCOV_EXCL_START */
    fclose(f);
    fclose(xs_f);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }
  CDD_SNPRINTF(typemap_filepath, sizeof(typemap_filepath), "%s/typemap",
               config->output_dir);
  typemap_f = fopen(typemap_filepath, "w");
  if (!typemap_f) {
    /* LCOV_EXCL_START */
    fclose(f);
    fclose(xs_f);
    fclose(make_f);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }
#endif

  fprintf(f, "# Auto-generated Perl FFI::Platypus bindings for %s\n", lib_name);
  fprintf(f, "package %s;\n\n", module_name);
  fprintf(f, "use strict;\n");
  fprintf(f, "use warnings;\n");
  fprintf(f, "use FFI::Platypus 2.00;\n");
  fprintf(f, "use FFI::Platypus::Record;\n\n");

  fprintf(f, "my $ffi = FFI::Platypus->new( api => 2 );\n");
  fprintf(f, "my $lib_path = $^O eq 'MSWin32' ? '%s.dll' :\n", lib_name);
  fprintf(f, "               $^O eq 'darwin'  ? 'lib%s.dylib' :\n", lib_name);
  fprintf(f, "                                  'lib%s.so';\n", lib_name);
  fprintf(f, "$ffi->lib($lib_path);\n\n");

  /* Definitions: Typedefs and Structs/Unions */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION) {
      fprintf(f, "package %s::%s;\n", module_name, node->name);
      fprintf(f, "use FFI::Platypus::Record;\n");
      fprintf(f, "record_layout(\n");
      if (node->kind == CDD_FFI_NODE_UNION) {
        fprintf(f, "  opaque => 'data', # Union mapping simplified to opaque "
                   "pointer\n");
      } else {
        for (j = 0; j < node->fields_count; j++) {
          map_perl_type(&node->fields[j].type, type_str, sizeof(type_str));
          fprintf(f, "  '%s' => '%s'%s\n", type_str, node->fields[j].name,
                  j + 1 < node->fields_count ? "," : "");
        }
      }
      fprintf(f, ");\n\n");
      fprintf(f, "package %s;\n", module_name); /* Revert to main package */
      fprintf(f, "$ffi->type('record(%s::%s)' => '%s');\n\n", module_name,
              node->name, node->name);
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      /* Map enum variants to constants */
      for (j = 0; j < node->variants_count; j++) {
        var = &node->variants[j];
        if (var->value) {
          fprintf(f, "use constant %s => %s;\n", var->name, var->value);
        } else {
          fprintf(f, "use constant %s => %" CDD_SIZE_T_FMT ";\n", var->name, j);
        }
      }
      fprintf(f, "$ffi->type('sint32' => '%s');\n\n", node->name);
    } else if (node->kind == CDD_FFI_NODE_TYPEDEF) {
      map_perl_type(&node->return_or_base_type, type_str, sizeof(type_str));
      fprintf(f, "$ffi->type('%s' => '%s');\n\n", type_str, node->name);
    }
  }

  /* Attach Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      map_perl_type(&node->return_or_base_type, ret_type_str,
                    sizeof(ret_type_str));
      fprintf(f, "$ffi->attach('%s' => [", node->name);
      for (j = 0; j < node->fields_count; j++) {
        map_perl_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(f, "'%s'%s", type_str, j + 1 < node->fields_count ? ", " : "");
      }
      fprintf(f, "] => '%s');\n", ret_type_str);
    }
  }

  fprintf(f, "\n1;\n");
  fclose(f);

  /* ---------------- Phase 2: XS Generation ---------------- */

  /* Makefile.PL */
  fprintf(make_f, "use ExtUtils::MakeMaker;\n\n");
  fprintf(make_f, "WriteMakefile(\n");
  fprintf(make_f, "    NAME              => '%s',\n", module_name);
  fprintf(make_f, "    VERSION_FROM      => '%s.pm', # finds $VERSION\n",
          module_name);
  fprintf(make_f, "    PREREQ_PM         => {},\n");
  fprintf(make_f, "    LIBS              => ['-L. -l%s'],\n", lib_name);
  fprintf(make_f, "    DEFINE            => '',\n");
  fprintf(make_f, "    INC               => '-I.',\n");
  fprintf(make_f, "    OBJECT            => '$(O_FILES)',\n");
  fprintf(make_f, ");\n");
  fclose(make_f);

  /* typemap */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION ||
        node->kind == CDD_FFI_NODE_TYPEDEF) {
      if (node->kind == CDD_FFI_NODE_TYPEDEF &&
          node->return_or_base_type.pointer_depth == 0)
        continue;
      fprintf(typemap_f, "%s *\tT_PTROBJ\n", node->name);
    }
  }
  fclose(typemap_f);

  /* mylib.xs */
  fprintf(xs_f, "#define PERL_NO_GET_CONTEXT\n");
  fprintf(xs_f, "#include \"EXTERN.h\"\n");
  fprintf(xs_f, "#include \"perl.h\"\n");
  fprintf(xs_f, "#include \"XSUB.h\"\n\n");
  fprintf(xs_f, "#include \"%s.h\"\n\n", lib_name);

  fprintf(xs_f, "MODULE = %s\tPACKAGE = %s\n\n", module_name, module_name);

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      map_xs_type(&node->return_or_base_type, ret_type_str,
                  sizeof(ret_type_str));
      fprintf(xs_f, "%s\n", ret_type_str);
      fprintf(xs_f, "%s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(xs_f, "%s%s", node->fields[j].name,
                j + 1 < node->fields_count ? ", " : "");
      }
      fprintf(xs_f, ")\n");
      for (j = 0; j < node->fields_count; j++) {
        map_xs_type(&node->fields[j].type, type_str, sizeof(type_str));
        fprintf(xs_f, "    %s %s\n", type_str, node->fields[j].name);
      }
      fprintf(xs_f, "\n");
    }
  }

  fclose(xs_f);

  return CDD_C_SUCCESS;
}
