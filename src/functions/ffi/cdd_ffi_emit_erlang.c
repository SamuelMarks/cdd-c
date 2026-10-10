#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
#endif
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_erlang.h"

#include "../../../include/c_cdd/safe_crt.h"
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../cdd_api.h"
#include "../../win_compat_sym.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Emits trivia (comments, whitespace).
 * @param f The FILE pointer.
 * @param trivia The trivia chain.
 */
static void emit_trivia(void *f, cdd_ffi_trivia_t *trivia) {
  FILE *file = (FILE *)f;
  while (trivia) {
    if (trivia->text) {
      fprintf(file, "%s", trivia->text);
    }
    trivia = trivia->next;
  }
}

static void snake_case_name(const char *c_name, char *out_name, size_t out_sz) {
  size_t i = 0, j = 0;
  while (c_name[i] && j < out_sz - 2) {
    if (isupper((unsigned char)c_name[i]) && i > 0) {
      if (c_name[i - 1] != '_') {
        out_name[j++] = '_';
      }
    }
    out_name[j++] = (char)tolower((unsigned char)c_name[i]);
    i++;
  }
  out_name[j] = '\0';
}

cdd_c_error_t
cdd_ffi_emit_erlang(cdd_ffi_ir_t *ir,
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
    return CDD_C_ERROR_UNKNOWN;
  }

  lib_name = (config->library_name) ? config->library_name : "mylib";

  if (config->module_name) {
    CDD_SNPRINTF(erl_module_name, sizeof(erl_module_name), "%s",
                 config->module_name);
  } else {
    snake_case_name(lib_name, erl_module_name, sizeof(erl_module_name));
  }

#if defined(_MSC_VER)
  CDD_SNPRINTF(c_filepath, sizeof(c_filepath), "%s\\%s_nif.c",
               config->output_dir, lib_name);
  {
    int err = 0;
#ifdef CDD_BUILD_TESTS
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      err = 1;
    } else
#endif
    {
      err = fopen_s(&c_f, c_filepath, "w");
    }
    if (err != 0) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }

  CDD_SNPRINTF(erl_filepath, sizeof(erl_filepath), "%s\\%s.erl",
               config->output_dir, erl_module_name);
  {
    int err = 0;
#ifdef CDD_BUILD_TESTS
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      err = 1;
    } else
#endif
    {
      err = fopen_s(&erl_f, erl_filepath, "w");
    }
    if (err != 0) {
      fclose(c_f);
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#else
  CDD_SNPRINTF(c_filepath, sizeof(c_filepath), "%s/%s_nif.c",
               config->output_dir, lib_name);
#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      c_f = NULL;
    } else
#endif
    {
#if defined(_MSC_VER)
      if (fopen_s(&c_f, c_filepath, "w") != 0)
        c_f = NULL;
#else
    c_f = fopen(c_filepath, "w");
#endif
    }
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!c_f) {
    return CDD_C_ERROR_UNKNOWN;
  }

  CDD_SNPRINTF(erl_filepath, sizeof(erl_filepath), "%s/%s.erl",
               config->output_dir, erl_module_name);
#ifdef CDD_BUILD_TESTS
  {
    if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
      erl_f = NULL;
    } else
#endif
    {
#if defined(_MSC_VER)
      if (fopen_s(&erl_f, erl_filepath, "w") != 0)
        erl_f = NULL;
#else
    erl_f = fopen(erl_filepath, "w");
#endif
    }
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!erl_f) {
    fclose(c_f);
    return CDD_C_ERROR_UNKNOWN;
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
      emit_trivia(c_f, node->leading_trivia);
      has_functions = 1;
      fprintf(c_f,
              "static ERL_NIF_TERM nif_%s(ErlNifEnv* env, int argc, const "
              "ERL_NIF_TERM argv[]) {\n",
              node->name);

      if (node->fields_count > 0) {
        fprintf(c_f, "    if (argc != %lu) {\n",
                (unsigned long)node->fields_count);
        fprintf(c_f, "        return enif_make_badarg(env);\n");
        fprintf(c_f, "    }\n");
      }

      /* Argument extraction */
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (node->fields[j].type.pointer_depth > 0 &&
            (node->fields[j].type.kind == CDD_FFI_KIND_INT8 ||
             node->fields[j].type.kind == CDD_FFI_KIND_UINT8)) {
          fprintf(c_f, "    char _c_%s[1024] = {0};\n", arg_name);
          fprintf(
              c_f,
              "    if (enif_get_string(env, argv[%lu], _c_%s, sizeof(_c_%s), "
              "ERL_NIF_LATIN1) <= 0) return enif_make_badarg(env);\n",
              (unsigned long)j, arg_name, arg_name);
        } else if (node->fields[j].type.pointer_depth > 0 ||
                   node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(
              c_f,
              "    void *_c_%s = NULL; /* Basic stub unsupported pointer */\n",
              arg_name);
        } else if (node->fields[j].type.kind == CDD_FFI_KIND_FLOAT32 ||
                   node->fields[j].type.kind == CDD_FFI_KIND_FLOAT64) {
          fprintf(c_f, "    double _c_%s = 0.0;\n", arg_name);
          fprintf(c_f,
                  "    if (!enif_get_double(env, argv[%lu], &_c_%s)) return "
                  "enif_make_badarg(env);\n",
                  (unsigned long)j, arg_name);
        } else if (node->fields[j].type.kind == CDD_FFI_KIND_BOOL) {
          fprintf(c_f, "    int _c_%s = 0;\n", arg_name);
          /* Erlang boolean is atom 'true' or 'false' */
          fprintf(c_f,
                  "    { char _tmp[16]; if (enif_get_atom(env, argv[%lu], "
                  "_tmp, sizeof(_tmp), ERL_NIF_LATIN1) && strcmp(_tmp, "
                  "\"true\") == 0) _c_%s = 1; else _c_%s = 0; }\n",
                  (unsigned long)j, arg_name, arg_name);
        } else {
          fprintf(c_f, "    int _c_%s = 0;\n", arg_name);
          fprintf(c_f,
                  "    if (!enif_get_int(env, argv[%lu], &_c_%s)) return "
                  "enif_make_badarg(env);\n",
                  (unsigned long)j, arg_name);
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
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (j > 0)
          fprintf(c_f, ", ");
        if (node->fields[j].type.kind == CDD_FFI_KIND_FLOAT32 &&
            node->fields[j].type.pointer_depth == 0) {
          fprintf(c_f, "(float)_c_%s", arg_name);
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
                  arg_name);
        } else {
          fprintf(c_f, "_c_%s", arg_name);
        }
      }
      fprintf(c_f, ");\n");

      /* Map return to NIF value */
      if (node->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
          node->return_or_base_type.pointer_depth == 0) {
        fprintf(c_f, "    return enif_make_atom(env, \"ok\");\n");
      } else {
        fprintf(c_f, "    ERL_NIF_TERM _nif_ret;\n");
        if (node->return_or_base_type.pointer_depth > 0 &&
            (node->return_or_base_type.kind == CDD_FFI_KIND_INT8 ||
             node->return_or_base_type.kind == CDD_FFI_KIND_UINT8)) {
          fprintf(c_f, "    _nif_ret = enif_make_string(env, _c_ret ? _c_ret : "
                       "\"\", ERL_NIF_LATIN1);\n");
        } else if (node->return_or_base_type.pointer_depth > 0 ||
                   node->return_or_base_type.kind == CDD_FFI_KIND_STRUCT_REF) {
          fprintf(c_f, "    _nif_ret = enif_make_atom(env, \"null\"); /* Stub "
                       "for pointer */\n");
        } else if (node->return_or_base_type.kind == CDD_FFI_KIND_FLOAT32 ||
                   node->return_or_base_type.kind == CDD_FFI_KIND_FLOAT64) {
          fprintf(c_f,
                  "    _nif_ret = enif_make_double(env, (double)_c_ret);\n");
        } else if (node->return_or_base_type.kind == CDD_FFI_KIND_BOOL) {
          fprintf(c_f, "    _nif_ret = enif_make_atom(env, _c_ret ? \"true\" : "
                       "\"false\");\n");
        } else {
          fprintf(c_f, "    _nif_ret = enif_make_int(env, (int)_c_ret);\n");
        }
        fprintf(c_f, "    return enif_make_tuple2(env, enif_make_atom(env, "
                     "\"ok\"), _nif_ret);\n");
      }
      fprintf(c_f, "}\n\n");
    }
  }

  /* ERL_NIF_INIT */
  fprintf(c_f, "static ErlNifFunc nif_funcs[] = {\n");
  for (i = 0; i < ir->nodes_count; i++) {
    node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      emit_trivia(c_f, node->leading_trivia);
      snake_case_name(node->name, snake_node_name, sizeof(snake_node_name));
      fprintf(c_f, "    {\"%s\", %lu, nif_%s, 0},\n", snake_node_name,
              (unsigned long)node->fields_count, node->name);
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
  fprintf(c_f, "    return CDD_C_SUCCESS;\n");
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
      emit_trivia(erl_f, node->leading_trivia);
      if (has_functions)
        fprintf(erl_f, ",\n");
      snake_case_name(node->name, snake_node_name, sizeof(snake_node_name));
      fprintf(erl_f, "    %s/%lu", snake_node_name,
              (unsigned long)node->fields_count);
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
      emit_trivia(erl_f, node->leading_trivia);
      snake_case_name(node->name, snake_node_name, sizeof(snake_node_name));
      fprintf(erl_f, "%s(", snake_node_name);
      for (j = 0; j < node->fields_count; j++) {
        fprintf(erl_f, "%s_Arg%lu", j > 0 ? ", " : "", (unsigned long)j);
      }
      fprintf(erl_f, ") ->\n");
      fprintf(erl_f, "    erlang:nif_error(nif_not_loaded).\n\n");
    }
  }

  fclose(erl_f);

  return CDD_C_SUCCESS;
}
