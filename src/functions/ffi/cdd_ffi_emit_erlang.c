/* clang-format off */
#include "cdd_ffi_emit_erlang.h"

#include "../../cdd_api.h"
#include "../../win_compat_sym.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../../include/c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* clang-format on */

static void snake_case_name(const char *c_name, char *out_name, size_t out_sz) {
  size_t i = 0, j = 0;
  if (!c_name || !out_name || out_sz == 0)
    return;
  while (c_name[i] && j < out_sz - 1) {
    if (isupper((unsigned char)c_name[i]) && i > 0) {
      if (j < out_sz - 2 && c_name[i - 1] != '_') {
        out_name[j++] = '_';
      }
    }
    out_name[j++] = (char)tolower((unsigned char)c_name[i]);
    i++;
  }
  out_name[j] = '\0';
}

int cdd_ffi_emit_erlang(cdd_ffi_ir_t *ir,
                        const cdd_generate_bindings_config_t *config) {
  FILE *c_f = NULL;
  FILE *erl_f = NULL;
  char c_filepath[1024];
  char erl_filepath[1024];
  const char *lib_name;
  char erl_module_name[256];
  size_t i, j;
  cdd_ffi_ir_node_t *node;
  char snake_node_name[256];
  int has_functions = 0;

  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  lib_name = config->library_name ? config->library_name : "mylib";

  if (config->module_name) {
    CDD_SNPRINTF(erl_module_name, sizeof(erl_module_name), "%s",
                 config->module_name);
  } else {
    snake_case_name(lib_name, erl_module_name, sizeof(erl_module_name));
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(c_filepath, sizeof(c_filepath), "%s\\%s_nif.c",
               config->output_dir, lib_name);
  if (fopen_s(&c_f, c_filepath, "w") != 0) {
    return 1;
  }
  CDD_SNPRINTF(erl_filepath, sizeof(erl_filepath), "%s\\%s.erl",
               config->output_dir, erl_module_name);
  if (fopen_s(&erl_f, erl_filepath, "w") != 0) {
    fclose(c_f);
    return 1;
  }
#else
  CDD_SNPRINTF(c_filepath, sizeof(c_filepath), "%s/%s_nif.c",
               config->output_dir, lib_name);
  c_f = fopen(c_filepath, "w");
  if (!c_f) {
    return 1;
  }
  CDD_SNPRINTF(erl_filepath, sizeof(erl_filepath), "%s/%s.erl",
               config->output_dir, erl_module_name);
  erl_f = fopen(erl_filepath, "w");
  if (!erl_f) {
    fclose(c_f);
    return 1;
  }
#endif

  fprintf(c_f, "/* Auto-generated Erlang NIF C89 boilerplate for %s */\n",
          lib_name);
  fprintf(c_f, "#include <erl_nif.h>\n");
  fprintf(c_f, "#include \"%s.h\"\n\n", lib_name);

  /* Generate Erlang resource type globals for structs */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION ||
        node->kind == CDD_FFI_NODE_TYPEDEF) {
      if (node->kind == CDD_FFI_NODE_TYPEDEF &&
          node->return_or_base_type.pointer_depth == 0)
        continue;
      fprintf(c_f, "ErlNifResourceType* %s_RES_TYPE;\n", node->name);
    }
  }
  fprintf(c_f, "\n");

  /* Destructor function for resources */
  fprintf(c_f, "static void default_dtor(ErlNifEnv* env, void* obj) {\n");
  fprintf(c_f, "    /* User-defined cleanup can be placed here */\n");
  fprintf(c_f, "}\n\n");

  /* Generate NIF C wrappers for functions */
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      has_functions = 1;
      fprintf(c_f,
              "static ERL_NIF_TERM nif_%s(ErlNifEnv* env, int argc, const "
              "ERL_NIF_TERM argv[]) {\n",
              node->name);

      if (node->fields_count > 0) {
        fprintf(c_f, "    if (argc != %" CDD_SIZE_T_FMT ") {\n",
                node->fields_count);
        fprintf(c_f, "        return enif_make_badarg(env);\n");
        fprintf(c_f, "    }\n");
      }

      /* Basic argument fetching */
      for (j = 0; j < node->fields_count; j++) {
        cdd_ffi_type_t *t = &node->fields[j].type;
        fprintf(c_f,
                "    /* TODO: Fetch argument %" CDD_SIZE_T_FMT
                " based on type %d */\n",
                j, t->kind);
      }

      fprintf(c_f, "    /* TODO: Call actual C function %s(...) */\n",
              node->name);

      if (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
          node->return_or_base_type.pointer_depth == 0) {
        fprintf(c_f, "    return enif_make_atom(env, \"ok\");\n");
      } else {
        fprintf(c_f, "    /* TODO: Convert return value to ERL_NIF_TERM */\n");
        fprintf(c_f, "    return enif_make_tuple2(env, enif_make_atom(env, "
                     "\"ok\"), enif_make_atom(env, \"result\"));\n");
      }
      fprintf(c_f, "}\n\n");
    }
  }

  /* ERL_NIF_INIT */
  fprintf(c_f, "static ErlNifFunc nif_funcs[] = {\n");
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      snake_case_name(node->name, snake_node_name, sizeof(snake_node_name));
      fprintf(c_f, "    {\"%s\", %" CDD_SIZE_T_FMT ", nif_%s, 0},\n",
              snake_node_name, node->fields_count, node->name);
    }
  }
  if (!has_functions) {
    fprintf(c_f, "    {\"dummy\", 0, NULL, 0}\n");
  }
  fprintf(c_f, "};\n\n");

  fprintf(c_f, "static int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM "
               "load_info) {\n");
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION ||
        node->kind == CDD_FFI_NODE_TYPEDEF) {
      if (node->kind == CDD_FFI_NODE_TYPEDEF &&
          node->return_or_base_type.pointer_depth == 0)
        continue;
      fprintf(c_f,
              "    %s_RES_TYPE = enif_open_resource_type(env, NULL, \"%s\", "
              "default_dtor, ERL_NIF_RT_CREATE, NULL);\n",
              node->name, node->name);
    }
  }
  fprintf(c_f, "    return 0;\n");
  fprintf(c_f, "}\n\n");

  fprintf(c_f, "ERL_NIF_INIT(%s, nif_funcs, load, NULL, NULL, NULL)\n",
          erl_module_name);

  fclose(c_f);

  /* Generate Erlang Module */
  fprintf(erl_f, "%% Auto-generated Erlang NIF bindings for %s\n", lib_name);
  fprintf(erl_f, "-module(%s).\n", erl_module_name);
  fprintf(erl_f, "-on_load(init/0).\n\n");

  fprintf(erl_f, "-export([\n");
  has_functions = 0;
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (has_functions)
        fprintf(erl_f, ",\n");
      snake_case_name(node->name, snake_node_name, sizeof(snake_node_name));
      fprintf(erl_f, "    %s/%" CDD_SIZE_T_FMT, snake_node_name,
              node->fields_count);
      has_functions = 1;
    }
  }
  if (!has_functions) {
    fprintf(erl_f, "    dummy/0\n");
  }
  fprintf(erl_f, "\n]).\n\n");

  fprintf(erl_f, "init() ->\n");
  fprintf(erl_f, "    PrivDir = case code:priv_dir(?MODULE) of\n");
  fprintf(erl_f, "        {error, bad_name} ->\n");
  fprintf(erl_f,
          "            EbinDir = filename:dirname(code:which(?MODULE)),\n");
  fprintf(erl_f, "            AppPath = filename:dirname(EbinDir),\n");
  fprintf(erl_f, "            filename:join(AppPath, \"priv\");\n");
  fprintf(erl_f, "        Path -> Path\n");
  fprintf(erl_f, "    end,\n");
  fprintf(erl_f,
          "    erlang:load_nif(filename:join(PrivDir, \"%s_nif\"), 0).\n\n",
          lib_name);

  if (!has_functions) {
    fprintf(erl_f, "dummy() -> erlang:nif_error(nif_not_loaded).\n");
  }

  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      snake_case_name(node->name, snake_node_name, sizeof(snake_node_name));
      fprintf(erl_f, "%s(", snake_node_name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(erl_f, "%s_Arg%" CDD_SIZE_T_FMT, j > 0 ? ", " : "", j);
      }
      fprintf(erl_f, ") ->\n");
      fprintf(erl_f, "    erlang:nif_error(nif_not_loaded).\n\n");
    }
  }

  fclose(erl_f);

  return 0;
}
