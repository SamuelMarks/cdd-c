/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_api.h"
#include "c_cdd/memory.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"

#ifdef CDD_BUILD_TESTS
#endif

#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_scope.h"
#include "classes/parse/cdd_cst_semantic.h"
#include "functions/ffi/cdd_ffi_emit_ada.h"
#include "functions/ffi/cdd_ffi_emit_clojure.h"
#include "functions/ffi/cdd_ffi_emit_common_lisp.h"
#include "functions/ffi/cdd_ffi_emit_cpp.h"
#include "functions/ffi/cdd_ffi_emit_crystal.h"
#include "functions/ffi/cdd_ffi_emit_csharp.h"
#include "functions/ffi/cdd_ffi_emit_d.h"
#include "functions/ffi/cdd_ffi_emit_dart.h"
#include "functions/ffi/cdd_ffi_emit_delphi.h"
#include "functions/ffi/cdd_ffi_emit_elixir.h"
#include "functions/ffi/cdd_ffi_emit_erlang.h"
#include "functions/ffi/cdd_ffi_emit_fortran.h"
#include "functions/ffi/cdd_ffi_emit_fsharp.h"
#include "functions/ffi/cdd_ffi_emit_go.h"
#include "functions/ffi/cdd_ffi_emit_groovy.h"
#include "functions/ffi/cdd_ffi_emit_haskell.h"
#include "functions/ffi/cdd_ffi_emit_java.h"
#include "functions/ffi/cdd_ffi_emit_julia.h"
#include "functions/ffi/cdd_ffi_emit_kotlin.h"
#include "functions/ffi/cdd_ffi_emit_lua.h"
#include "functions/ffi/cdd_ffi_emit_matlab.h"
#include "functions/ffi/cdd_ffi_emit_napi.h"
#include "functions/ffi/cdd_ffi_emit_nim.h"
#include "functions/ffi/cdd_ffi_emit_objc.h"
#include "functions/ffi/cdd_ffi_emit_ocaml.h"
#include "functions/ffi/cdd_ffi_emit_odin.h"
#include "functions/ffi/cdd_ffi_emit_perl.h"
#include "functions/ffi/cdd_ffi_emit_php.h"
#include "functions/ffi/cdd_ffi_emit_python.h"
#include "functions/ffi/cdd_ffi_emit_r.h"
#include "functions/ffi/cdd_ffi_emit_racket.h"
#include "functions/ffi/cdd_ffi_emit_ruby.h"
#include "functions/ffi/cdd_ffi_emit_rust.h"
#include "functions/ffi/cdd_ffi_emit_scala.h"
#include "functions/ffi/cdd_ffi_emit_scheme.h"
#include "functions/ffi/cdd_ffi_emit_swift.h"
#include "functions/ffi/cdd_ffi_emit_tcl.h"
#include "functions/ffi/cdd_ffi_emit_typescript.h"
#include "functions/ffi/cdd_ffi_emit_vlang.h"
#include "functions/ffi/cdd_ffi_emit_webassembly.h"
#include "functions/ffi/cdd_ffi_emit_zig.h"
#include "functions/ffi/cdd_ffi_ir_extractor.h"
#include "routes/emit/serve_json_rpc.h"
#include "routes/parse/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define MAX_ARGS 32

cdd_c_error_t
cdd_generate_from_openapi(const cdd_from_openapi_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;

  argv[argc++] = "from_openapi";

  if (config->subcommand) {
    argv[argc++] = (char *)(size_t)config->subcommand;
  } else {
    argv[argc++] = "to_sdk";
  }

  if (config->input) {
    argv[argc++] = "-i";
    argv[argc++] = (char *)(size_t)config->input;
  } else if (config->input_dir) {
    argv[argc++] = "--input-dir";
    argv[argc++] = (char *)(size_t)config->input_dir;
  }

  if (config->output) {
    argv[argc++] = "-o";
    argv[argc++] = (char *)(size_t)config->output;
  }

  if (config->no_github_actions) {
    argv[argc++] = "--no-github-actions";
  }

  if (config->no_installable_package) {
    argv[argc++] = "--no-installable-package";
  }

  if (config->tests) {
    argv[argc++] = "--tests";
  }

  return from_openapi_cli_main(argc, argv);
}

cdd_c_error_t cdd_generate_to_openapi(const cdd_to_openapi_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;

  argv[argc++] = "to_openapi";

  if (config->input) {
    argv[argc++] = "-i";
    argv[argc++] = (char *)(size_t)config->input;
  }

  if (config->output) {
    argv[argc++] = "-o";
    argv[argc++] = (char *)(size_t)config->output;
  }

  return to_openapi_cli_main(argc, argv);
}

cdd_c_error_t cdd_generate_docs_json(const cdd_docs_json_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;

  argv[argc++] = "to_docs_json";

  if (config->input) {
    argv[argc++] = "-i";
    argv[argc++] = (char *)(size_t)config->input;
  }

  if (config->output) {
    argv[argc++] = "-o";
    argv[argc++] = (char *)(size_t)config->output;
  }

  if (config->no_imports) {
    argv[argc++] = "--no-imports";
  }

  if (config->no_wrapping) {
    argv[argc++] = "--no-wrapping";
  }

  return to_docs_json_cli_main(argc, argv);
}

cdd_c_error_t cdd_serve_json_rpc(const cdd_serve_json_rpc_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;
  char port_str[32];

  argv[argc++] = "serve_json_rpc";

  if (config->port > 0) {
    CDD_SNPRINTF(port_str, sizeof(port_str), "%d", config->port);
    argv[argc++] = "-p";
    argv[argc++] = port_str;
  }

  if (config->listen_host) {
    argv[argc++] = "-l";
    argv[argc++] = (char *)(size_t)config->listen_host;
  }

  return serve_json_rpc_main(argc, argv);
}

/**
 * @brief Generate SWIG-like FFI bindings for multiple target languages.
 */

#define HAS_LANG(langs, lang, out_result)                                      \
  do {                                                                         \
    const char *_p = (langs);                                                  \
    size_t _len = strlen(lang);                                                \
    *(out_result) = 0;                                                         \
    while ((_p = strstr(_p, (lang))) != NULL) {                                \
      if ((_p == (langs) || _p[-1] == ',') &&                                  \
          (_p[_len] == '\0' || _p[_len] == ',')) {                             \
        *(out_result) = 1;                                                     \
        break;                                                                 \
      }                                                                        \
      _p += _len;                                                              \
    }                                                                          \
  } while (0)

typedef enum cdd_c_error (*ffi_emitter_fn_t)(
    cdd_ffi_ir_t *, const cdd_generate_bindings_config_t *);

struct FfiEmitterEntry {
  const char *name;
  const char *alias;
  ffi_emitter_fn_t emit;
};

static const struct FfiEmitterEntry EMITTERS[] = {
    {"python", NULL, cdd_ffi_emit_python},
    {"rust", NULL, cdd_ffi_emit_rust},
    {"csharp", NULL, cdd_ffi_emit_csharp},
    {"typescript", NULL, cdd_ffi_emit_typescript},
    {"napi", NULL, cdd_ffi_emit_napi},
    {"java", NULL, cdd_ffi_emit_java},
    {"cpp", NULL, cdd_ffi_emit_cpp},
    {"go", NULL, cdd_ffi_emit_go},
    {"swift", NULL, cdd_ffi_emit_swift},
    {"dart", NULL, cdd_ffi_emit_dart},
    {"ruby", NULL, cdd_ffi_emit_ruby},
    {"kotlin", NULL, cdd_ffi_emit_kotlin},
    {"php", NULL, cdd_ffi_emit_php},
    {"lua", NULL, cdd_ffi_emit_lua},
    {"zig", NULL, cdd_ffi_emit_zig},
    {"odin", NULL, cdd_ffi_emit_odin},
    {"julia", NULL, cdd_ffi_emit_julia},
    {"r", NULL, cdd_ffi_emit_r},
    {"matlab", NULL, cdd_ffi_emit_matlab},
    {"haskell", NULL, cdd_ffi_emit_haskell},
    {"ocaml", NULL, cdd_ffi_emit_ocaml},
    {"elixir", NULL, cdd_ffi_emit_elixir},
    {"erlang", NULL, cdd_ffi_emit_erlang},
    {"common_lisp", NULL, cdd_ffi_emit_common_lisp},
    {"racket", NULL, cdd_ffi_emit_racket},
    {"scheme", NULL, cdd_ffi_emit_scheme},
    {"scala", NULL, cdd_ffi_emit_scala},
    {"fsharp", NULL, cdd_ffi_emit_fsharp},
    {"clojure", NULL, cdd_ffi_emit_clojure},
    {"groovy", NULL, cdd_ffi_emit_groovy},
    {"webassembly", "wasm", cdd_ffi_emit_webassembly},
    {"nim", NULL, cdd_ffi_emit_nim},
    {"vlang", NULL, cdd_ffi_emit_vlang},
    {"dlang", "d", cdd_ffi_emit_d},
    {"perl", NULL, cdd_ffi_emit_perl},
    {"tcl", NULL, cdd_ffi_emit_tcl},
    {"fortran", NULL, cdd_ffi_emit_fortran},
    {"delphi", "pascal", cdd_ffi_emit_delphi},
    {"ada", NULL, cdd_ffi_emit_ada},
    {"objc", "objective-c", cdd_ffi_emit_objc},
    {"crystal", NULL, cdd_ffi_emit_crystal}};

cdd_c_error_t
cdd_generate_bindings(const cdd_generate_bindings_config_t *config) {

  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;

  char *file_content = NULL;
  size_t file_size = 0;

  if (!config || !config->input || !config->output_dir ||
      !config->target_langs) {
    return CDD_C_ERROR_UNKNOWN; /* EINVAL */
  }

  /* Read file to string */
  rc = read_to_file(config->input, "rb", &file_content, &file_size);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  /* Extract exports into FFI IR */
  rc = cdd_ffi_ir_extract_exports(config->input, file_content, config, &ir);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(file_content);
    return rc;
  }

  /* Sort IR dependencies */
  rc = cdd_ffi_ir_topological_sort(ir);
  if (rc != CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    C_CDD_FREE(ir);
    C_CDD_FREE(file_content);
    return rc;
  }

  /* Dispatch to Emitters */
  {
    size_t i;
    for (i = 0; i < sizeof(EMITTERS) / sizeof(EMITTERS[0]); ++i) {
      int should_emit = 0;
      if (strcmp(config->target_langs, "all") == 0 ||
          strcmp(config->target_langs, "*") == 0) {
        should_emit = 1;
      } else {
        int out_result = 0;
        HAS_LANG(config->target_langs, EMITTERS[i].name, &out_result);
        if (out_result) {
          should_emit = 1;
        } else if (EMITTERS[i].alias) {
          HAS_LANG(config->target_langs, EMITTERS[i].alias, &out_result);
          if (out_result)
            should_emit = 1;
        }
      }
      if (should_emit) {
        rc = EMITTERS[i].emit(ir, config);
        if (rc != CDD_C_SUCCESS) {
          printf("Failed at %s, rc = %d\n", EMITTERS[i].name, rc);
          cdd_ffi_ir_free(ir);
          C_CDD_FREE(ir);
          C_CDD_FREE(file_content);
          return rc;
        }
      }
    }
  }

  /* Cleanup */
  cdd_ffi_ir_free(ir);
  C_CDD_FREE(ir);
  C_CDD_FREE(file_content);

  return CDD_C_SUCCESS;
}
