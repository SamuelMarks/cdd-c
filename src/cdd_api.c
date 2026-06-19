/* clang-format off */
#include "cdd_api.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_semantic.h"
#include "classes/parse/cdd_cst_scope.h"
#include "routes/emit/serve_json_rpc.h"
#include "routes/parse/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_fail_alloc = 0;
C_CDD_EXPORT int g_cdd_fprintf_fail = 0;
C_CDD_EXPORT int g_cdd_mock_dlopen_success = 0;
#endif

#define MAX_ARGS 32

int cdd_generate_from_openapi(const cdd_from_openapi_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;

  argv[argc++] = "from_openapi";

  if (config->subcommand) {
    argv[argc++] = (char *)config->subcommand;
  } else {
    argv[argc++] = "to_sdk";
  }

  if (config->input) {
    argv[argc++] = "-i";
    argv[argc++] = (char *)config->input;
  } else if (config->input_dir) {
    argv[argc++] = "--input-dir";
    argv[argc++] = (char *)config->input_dir;
  }

  if (config->output) {
    argv[argc++] = "-o";
    argv[argc++] = (char *)config->output;
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

int cdd_generate_to_openapi(const cdd_to_openapi_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;

  argv[argc++] = "to_openapi";

  if (config->input) {
    argv[argc++] = "-i";
    argv[argc++] = (char *)config->input;
  }

  if (config->output) {
    argv[argc++] = "-o";
    argv[argc++] = (char *)config->output;
  }

  return to_openapi_cli_main(argc, argv);
}

int cdd_generate_docs_json(const cdd_docs_json_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;

  argv[argc++] = "to_docs_json";

  if (config->input) {
    argv[argc++] = "-i";
    argv[argc++] = (char *)config->input;
  }

  if (config->output) {
    argv[argc++] = "-o";
    argv[argc++] = (char *)config->output;
  }

  if (config->no_imports) {
    argv[argc++] = "--no-imports";
  }

  if (config->no_wrapping) {
    argv[argc++] = "--no-wrapping";
  }

  return to_docs_json_cli_main(argc, argv);
}

int cdd_serve_json_rpc(const cdd_serve_json_rpc_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;
  char port_str[32];

  argv[argc++] = "serve_json_rpc";

  if (config->port > 0) {
    sprintf(port_str, "%d", config->port);
    argv[argc++] = "-p";
    argv[argc++] = port_str;
  }

  if (config->listen_host) {
    argv[argc++] = "-l";
    argv[argc++] = (char *)config->listen_host;
  }

  return serve_json_rpc_main(argc, argv);
}

/* LCOV_EXCL_START */

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

/**
 * @brief Generate SWIG-like FFI bindings for multiple target languages.
 */
int cdd_generate_bindings(const cdd_generate_bindings_config_t *config) {
  cdd_ffi_ir_t *ir = NULL;
  int rc;

  char *file_content = NULL;
  size_t file_size = 0;

  if (!config || !config->input || !config->output_dir ||
      !config->target_langs) {
    return 1; /* EINVAL */
  }

  /* Read file to string */
  rc = read_to_file(config->input, "rb", &file_content, &file_size);
  if (rc != 0) {
    return rc;
  }

  /* Extract exports into FFI IR */
  rc = cdd_ffi_ir_extract_exports(config->input, file_content, config, &ir);
  if (rc != 0) {
    free(file_content);
    return rc;
  }

  /* Sort IR dependencies */
  rc = cdd_ffi_ir_topological_sort(ir);
  if (rc != 0) {
    cdd_ffi_ir_free(ir);
    free(file_content);
    return rc;
  }

  /* Dispatch to Emitters */
  if (config->target_langs) {
    if (strstr(config->target_langs, "python") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_python(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "rust") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_rust(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "csharp") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_csharp(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "typescript") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_typescript(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "napi") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_napi(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "java") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_java(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "cpp") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_cpp(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "go") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_go(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "swift") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_swift(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "dart") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_dart(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "ruby") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_ruby(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "kotlin") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_kotlin(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "php") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_php(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "lua") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_lua(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "zig") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_zig(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "odin") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_odin(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "julia") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_julia(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "r") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_r(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "matlab") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_matlab(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "haskell") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_haskell(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "ocaml") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_ocaml(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "elixir") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_elixir(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "erlang") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_erlang(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "common_lisp") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_common_lisp(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "racket") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_racket(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "scheme") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_scheme(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "scala") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_scala(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "fsharp") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_fsharp(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "clojure") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_clojure(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "groovy") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_groovy(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "webassembly") ||
        strcmp(config->target_langs, "wasm") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_webassembly(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "nim") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_nim(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "vlang") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_vlang(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "dlang") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_d(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "perl") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_perl(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "tcl") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_tcl(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "fortran") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_fortran(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "delphi") ||
        strcmp(config->target_langs, "pascal") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_delphi(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "ada") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_ada(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "objc") ||
        strcmp(config->target_langs, "objective-c") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_objc(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
    if (strstr(config->target_langs, "crystal") ||
        strcmp(config->target_langs, "all") == 0) {
      rc = cdd_ffi_emit_crystal(ir, config);
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        free(file_content);
        return rc;
      }
    }
  }

  /* Cleanup */
  cdd_ffi_ir_free(ir);
  free(file_content);

  return 0;
}
