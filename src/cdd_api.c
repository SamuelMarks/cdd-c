/* clang-format off */
#include "c_cdd/memory.h"
#include "cdd_api.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_crypto_fail_sha256 = 0;
C_CDD_EXPORT int g_crypto_fail_mdctx_new = 0;
C_CDD_EXPORT int g_crypto_fail_digestinit = 0;
C_CDD_EXPORT int g_crypto_fail_digestupdate = 0;
C_CDD_EXPORT int g_crypto_fail_digestfinal = 0;
C_CDD_EXPORT int g_crypto_fail_digestfinal_len = 0;
C_CDD_EXPORT int g_crypto_fail_hmac = 0;
C_CDD_EXPORT int g_crypto_fail_hmac_len = 0;
#endif

#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_semantic.h"
#include "classes/parse/cdd_cst_scope.h"
#include "routes/emit/serve_json_rpc.h"
#include "routes/parse/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
/* clang-format on */
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_fail_alloc = 0;
C_CDD_EXPORT int g_cdd_fprintf_fail = 0;
C_CDD_EXPORT int g_cdd_mock_dlopen_success = 0;
#endif

#define MAX_ARGS 32

enum cdd_c_error
cdd_generate_from_openapi(const cdd_from_openapi_config_t *config) {
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

enum cdd_c_error
cdd_generate_to_openapi(const cdd_to_openapi_config_t *config) {
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

enum cdd_c_error cdd_generate_docs_json(const cdd_docs_json_config_t *config) {
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

enum cdd_c_error cdd_serve_json_rpc(const cdd_serve_json_rpc_config_t *config) {
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

/**
 * @brief Generate SWIG-like FFI bindings for multiple target languages.
 */

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_has_lang_fail = -1;
#endif

static enum cdd_c_error has_lang(const char *langs, const char *lang,
                                 int *out) {
  const char *p = langs;
  size_t len = strlen(lang);
  *out = 0;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_has_lang_fail > 0) {
    g_cdd_has_lang_fail--;
  } else if (g_cdd_has_lang_fail == 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  while ((p = strstr(p, lang)) != NULL) {
    if ((p == langs || p[-1] == ',') && (p[len] == '\0' || p[len] == ',')) {
      *out = 1;
      return CDD_C_SUCCESS;
    }
    p += len;
  }
  return CDD_C_SUCCESS;
}

enum cdd_c_error
cdd_generate_bindings(const cdd_generate_bindings_config_t *config) {

  cdd_ffi_ir_t *ir = NULL;
  int rc;

  char *file_content = NULL;
  size_t file_size = 0;

  if (!config || !config->input || !config->output_dir ||
      !config->target_langs) {
    return CDD_C_ERROR_UNKNOWN; /* EINVAL */
  }

  /* Read file to string */
  rc = read_to_file(config->input, "rb", &file_content, &file_size);
  if (rc != 0) {
    return rc;
  }

  /* Extract exports into FFI IR */
  rc = cdd_ffi_ir_extract_exports(config->input, file_content, config, &ir);
  if (rc != 0) {
    C_CDD_FREE(file_content);
    return rc;
  }

  /* Sort IR dependencies */
  rc = cdd_ffi_ir_topological_sort(ir);
  if (rc != 0) {
    cdd_ffi_ir_free(ir);
    C_CDD_FREE(ir);
    C_CDD_FREE(file_content);
    return rc;
  }

  /* Dispatch to Emitters */
  if (config->target_langs) {
    int has_lang_python = 0;
    int has_lang_rust = 0;
    int has_lang_csharp = 0;
    int has_lang_typescript = 0;
    int has_lang_napi = 0;
    int has_lang_java = 0;
    int has_lang_cpp = 0;
    int has_lang_go = 0;
    int has_lang_swift = 0;
    int has_lang_dart = 0;
    int has_lang_ruby = 0;
    int has_lang_kotlin = 0;
    int has_lang_php = 0;
    int has_lang_lua = 0;
    int has_lang_zig = 0;
    int has_lang_odin = 0;
    int has_lang_julia = 0;
    int has_lang_r = 0;
    int has_lang_matlab = 0;
    int has_lang_haskell = 0;
    int has_lang_ocaml = 0;
    int has_lang_elixir = 0;
    int has_lang_erlang = 0;
    int has_lang_common_lisp = 0;
    int has_lang_racket = 0;
    int has_lang_scheme = 0;
    int has_lang_scala = 0;
    int has_lang_fsharp = 0;
    int has_lang_clojure = 0;
    int has_lang_groovy = 0;
    int has_lang_webassembly = 0;
    int has_lang_nim = 0;
    int has_lang_vlang = 0;
    int has_lang_dlang = 0;
    int has_lang_perl = 0;
    int has_lang_tcl = 0;
    int has_lang_fortran = 0;
    int has_lang_delphi = 0;
    int has_lang_ada = 0;
    int has_lang_objc = 0;
    int has_lang_crystal = 0;
    rc = has_lang(config->target_langs, "python", &has_lang_python);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_python || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_python(ir, config);
      if (rc != 0) {
        printf("Failed emitter python with %d\n", rc);
      }
      if (rc != 0) {
        printf("Failed at python, rc = %d\n", rc);
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "rust", &has_lang_rust);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_rust || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_rust(ir, config);
      if (rc != 0) {
        printf("Failed emitter rust with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "csharp", &has_lang_csharp);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_csharp || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_csharp(ir, config);
      if (rc != 0) {
        printf("Failed emitter csharp with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "typescript", &has_lang_typescript);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_typescript || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_typescript(ir, config);
      if (rc != 0) {
        printf("Failed emitter typescript with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "napi", &has_lang_napi);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_napi || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_napi(ir, config);
      if (rc != 0) {
        printf("Failed emitter napi with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "java", &has_lang_java);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_java || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_java(ir, config);
      if (rc != 0) {
        printf("Failed emitter java with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "cpp", &has_lang_cpp);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_cpp || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_cpp(ir, config);
      if (rc != 0) {
        printf("Failed emitter cpp with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "go", &has_lang_go);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_go || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_go(ir, config);
      if (rc != 0) {
        printf("Failed emitter go with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "swift", &has_lang_swift);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_swift || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_swift(ir, config);
      if (rc != 0) {
        printf("Failed emitter swift with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "dart", &has_lang_dart);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_dart || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_dart(ir, config);
      if (rc != 0) {
        printf("Failed emitter dart with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "ruby", &has_lang_ruby);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_ruby || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_ruby(ir, config);
      if (rc != 0) {
        printf("Failed emitter ruby with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "kotlin", &has_lang_kotlin);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_kotlin || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_kotlin(ir, config);
      if (rc != 0) {
        printf("Failed emitter kotlin with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "php", &has_lang_php);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_php || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_php(ir, config);
      if (rc != 0) {
        printf("Failed emitter php with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "lua", &has_lang_lua);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_lua || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_lua(ir, config);
      if (rc != 0) {
        printf("Failed emitter lua with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "zig", &has_lang_zig);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_zig || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_zig(ir, config);
      if (rc != 0) {
        printf("Failed emitter zig with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "odin", &has_lang_odin);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_odin || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_odin(ir, config);
      if (rc != 0) {
        printf("Failed emitter odin with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "julia", &has_lang_julia);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_julia || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_julia(ir, config);
      if (rc != 0) {
        printf("Failed emitter julia with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "r", &has_lang_r);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_r || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_r(ir, config);
      if (rc != 0) {
        printf("Failed emitter r with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "matlab", &has_lang_matlab);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_matlab || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_matlab(ir, config);
      if (rc != 0) {
        printf("Failed emitter matlab with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "haskell", &has_lang_haskell);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_haskell || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_haskell(ir, config);
      if (rc != 0) {
        printf("Failed emitter haskell with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "ocaml", &has_lang_ocaml);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_ocaml || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_ocaml(ir, config);
      if (rc != 0) {
        printf("Failed emitter ocaml with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "elixir", &has_lang_elixir);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_elixir || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_elixir(ir, config);
      if (rc != 0) {
        printf("Failed emitter elixir with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "erlang", &has_lang_erlang);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_erlang || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_erlang(ir, config);
      if (rc != 0) {
        printf("Failed emitter erlang with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "common_lisp", &has_lang_common_lisp);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_common_lisp || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_common_lisp(ir, config);
      if (rc != 0) {
        printf("Failed emitter common_lisp with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "racket", &has_lang_racket);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_racket || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_racket(ir, config);
      if (rc != 0) {
        printf("Failed emitter racket with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "scheme", &has_lang_scheme);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_scheme || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_scheme(ir, config);
      if (rc != 0) {
        printf("Failed emitter scheme with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "scala", &has_lang_scala);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_scala || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_scala(ir, config);
      if (rc != 0) {
        printf("Failed emitter scala with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "fsharp", &has_lang_fsharp);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_fsharp || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_fsharp(ir, config);
      if (rc != 0) {
        printf("Failed emitter fsharp with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "clojure", &has_lang_clojure);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_clojure || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_clojure(ir, config);
      if (rc != 0) {
        printf("Failed emitter clojure with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "groovy", &has_lang_groovy);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_groovy || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_groovy(ir, config);
      if (rc != 0) {
        printf("Failed emitter groovy with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "webassembly", &has_lang_webassembly);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_webassembly || strcmp(config->target_langs, "wasm") == 0 ||
        strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_webassembly(ir, config);
      if (rc != 0) {
        printf("Failed emitter webassembly with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "nim", &has_lang_nim);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_nim || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_nim(ir, config);
      if (rc != 0) {
        printf("Failed emitter nim with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "vlang", &has_lang_vlang);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_vlang || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_vlang(ir, config);
      if (rc != 0) {
        printf("Failed emitter vlang with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "dlang", &has_lang_dlang);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_dlang || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_d(ir, config);
      if (rc != 0) {
        printf("Failed emitter d with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "perl", &has_lang_perl);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_perl || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_perl(ir, config);
      if (rc != 0) {
        printf("Failed emitter perl with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "tcl", &has_lang_tcl);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_tcl || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_tcl(ir, config);
      if (rc != 0) {
        printf("Failed emitter tcl with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "fortran", &has_lang_fortran);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_fortran || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_fortran(ir, config);
      if (rc != 0) {
        printf("Failed emitter fortran with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "delphi", &has_lang_delphi);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_delphi || strcmp(config->target_langs, "pascal") == 0 ||
        strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_delphi(ir, config);
      if (rc != 0) {
        printf("Failed emitter delphi with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "ada", &has_lang_ada);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_ada || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_ada(ir, config);
      if (rc != 0) {
        printf("Failed emitter ada with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "objc", &has_lang_objc);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_objc || strcmp(config->target_langs, "objective-c") == 0 ||
        strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_objc(ir, config);
      if (rc != 0) {
        printf("Failed emitter objc with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
    rc = has_lang(config->target_langs, "crystal", &has_lang_crystal);
    if (rc != CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      C_CDD_FREE(ir);
      C_CDD_FREE(file_content);
      return rc;
    }
    if (has_lang_crystal || strcmp(config->target_langs, "all") == 0 ||
        strcmp(config->target_langs, "*") == 0) {
      rc = cdd_ffi_emit_crystal(ir, config);
      if (rc != 0) {
        printf("Failed emitter crystal with %d\n", rc);
      }
      if (rc != 0) {
        cdd_ffi_ir_free(ir);
        C_CDD_FREE(ir);
        C_CDD_FREE(file_content);
        return rc;
      }
    }
  }

  /* Cleanup */
  cdd_ffi_ir_free(ir);
  C_CDD_FREE(ir);
  C_CDD_FREE(file_content);

  return CDD_C_SUCCESS;
}
extern int g_fail_io_after;
