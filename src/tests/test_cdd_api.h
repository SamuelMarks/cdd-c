#ifndef TEST_CDD_API_H
#define TEST_CDD_API_H

/* clang-format off */
#include "cdd_api.h"
#include "c_cdd/code_writer.h"
#include "functions/ffi/cdd_ffi_emit_common.h"
#include "functions/parse/fs.h"
#include "openapi/parse/openapi.h"
#include "routes/parse/cli.h"
#include <greatest.h>
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_ffi_ir_calloc_fail;
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_fprintf_fail;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

TEST test_cdd_generate_from_openapi(void) {
  cdd_from_openapi_config_t config = {0};
  FILE *f;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_from_openapi(NULL));

  config.input = cdd_test_mut_str("nonexistent.json");
  config.output = cdd_test_mut_str("build/out");
  config.subcommand = cdd_test_mut_str("to_sdk");
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_generate_from_openapi(&config));

  /* Test OOM on fs_path_join in cdd_generate_from_openapi */
  f = fopen("build/test_min_oas.json", "w");
  if (f) {
    fprintf(f, "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
               "\"1\"},\"paths\":{}}");
    fclose(f);
  }
  config.input = cdd_test_mut_str("build/test_min_oas.json");
  config.output = cdd_test_mut_str("build/out");
#ifdef CDD_BUILD_TESTS
  {
    int step;
    for (step = 1; step <= 25; ++step) {
      g_cdd_alloc_fail = step;
      cdd_generate_from_openapi(&config);
      g_cdd_alloc_fail = 0;
    }
  }
#endif
  remove("build/test_min_oas.json");

  config.subcommand = NULL;
  config.input = NULL;
  config.input_dir = cdd_test_mut_str("nonexistent_dir");
  config.no_github_actions = 1;
  config.no_installable_package = 1;
  config.tests = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_from_openapi(&config));

  /* Cover branches where flags are false but input_dir is not set, input not
   * set */
  config.input_dir = NULL;
  config.no_github_actions = 0;
  config.no_installable_package = 0;
  config.tests = 0;
  config.output = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_from_openapi(&config));

  PASS();
}

TEST test_cdd_generate_to_openapi(void) {
  cdd_to_openapi_config_t config = {0};

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_to_openapi(NULL));

  config.input = cdd_test_mut_str("nonexistent.c");
  config.output = cdd_test_mut_str("build/out.json");
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_generate_to_openapi(&config));

  config.output = NULL;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_generate_to_openapi(&config));
  remove("openapi.json");

  config.output = cdd_test_mut_str("build/out.json");
  config.base_file = cdd_test_mut_str("nonexistent_base.json");
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_generate_to_openapi(&config));
  config.base_file = NULL;

#ifdef CDD_BUILD_TESTS
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_generate_to_openapi(&config));
  g_cdd_alloc_fail = 0;
#endif

  config.input = NULL;
  config.output = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_to_openapi(&config));

  PASS();
}

TEST test_cdd_generate_docs_json(void) {
  cdd_docs_json_config_t config = {0};

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_docs_json(NULL));

  config.input = cdd_test_mut_str("nonexistent.json");
  config.output = cdd_test_mut_str("build/out.json");
  config.no_imports = 1;
  config.no_wrapping = 1;
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_generate_docs_json(&config));

  config.input = NULL;
  config.no_imports = 0;
  config.no_wrapping = 0;
  config.output = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_docs_json(&config));

  PASS();
}

TEST test_cdd_serve_json_rpc(void) {
  cdd_serve_json_rpc_config_t config = {0};

  config.port = 1; /* port > 0 */
  config.listen_host = cdd_test_mut_str("255");
  ASSERT_NEQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_serve_json_rpc(&config));

  config.port = 0;
  config.listen_host = NULL;
  ASSERT_NEQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_serve_json_rpc(&config));

  PASS();
}

#ifndef __EMSCRIPTEN__
TEST test_bin_cdd(void) {
  int rc = 1;
  const char *env_bin = getenv("CDD_C_BIN");
  const char *candidates[16];
  size_t n_cands = 0;
  size_t i;
  candidates[n_cands++] = "./bin/nonexistent_dummy";
  if (env_bin != NULL && env_bin[0] != '\0') {
    candidates[n_cands++] = env_bin;
  }
#if defined(_WIN32)
  candidates[n_cands++] = ".\\bin\\nonexistent_dummy.exe";
  candidates[n_cands++] = ".\\cdd-c.exe";
  candidates[n_cands++] = "..\\cdd-c.exe";
  candidates[n_cands++] = "..\\..\\cdd-c.exe";
  candidates[n_cands++] = ".\\bin\\cdd-c.exe";
  candidates[n_cands++] = "..\\..\\bin\\cdd-c.exe";
  candidates[n_cands++] = ".\\build_msvc_wine_shared\\cdd-c.exe";
  candidates[n_cands++] = ".\\build_msvc_wine_static\\cdd-c.exe";
  candidates[n_cands++] = ".\\build\\bin\\cdd-c.exe";
  candidates[n_cands++] = ".\\build\\cdd-c.exe";
#else
  candidates[n_cands++] = "./bin/nonexistent_dummy";
  candidates[n_cands++] = "./bin/cdd-c";
  candidates[n_cands++] = "./build_gcc/bin/cdd-c";
  candidates[n_cands++] = "./build_clang/bin/cdd-c";
  candidates[n_cands++] = "./build/bin/cdd-c";
  candidates[n_cands++] = "./build_cov/bin/cdd-c";
  candidates[n_cands++] = "../bin/cdd-c";
#endif
  /* Exercise non-zero return code branch */
#if defined(_WIN32)
  rc = system("\"nonexistent_cdd_bin.exe\" --help > NUL 2>&1");
#else
  rc = system("\"./nonexistent_cdd_bin\" --help > /dev/null 2>&1");
#endif
  ASSERT(rc != 0);

  for (i = 0; i < n_cands; ++i) {
    char cmd[1024];
#if defined(_MSC_VER)
    sprintf_s(cmd, sizeof(cmd), "\"%s\" --help > NUL 2>&1", candidates[i]);
#elif defined(_WIN32)
    CDD_SNPRINTF(cmd, sizeof(cmd), "\"%s\" --help > NUL 2>&1", candidates[i]);
#else
    CDD_SNPRINTF(cmd, sizeof(cmd), "\"%s\" --help > /dev/null 2>&1",
                 candidates[i]);
#endif
    rc = system(cmd);
    if (rc == 0) {
      char err_cmd[1024];
#if defined(_MSC_VER)
      sprintf_s(err_cmd, sizeof(err_cmd),
                "\"%s\" invalid_subcommand_xyz > NUL 2>&1", candidates[i]);
#elif defined(_WIN32)
      CDD_SNPRINTF(err_cmd, sizeof(err_cmd),
                   "\"%s\" invalid_subcommand_xyz > NUL 2>&1", candidates[i]);
#else
      CDD_SNPRINTF(err_cmd, sizeof(err_cmd),
                   "\"%s\" invalid_subcommand_xyz > /dev/null 2>&1",
                   candidates[i]);
#endif
      ASSERT_NEQ(0, system(err_cmd));
      break;
    }
  }
  ASSERT_EQ(0, rc);
  PASS();
}
#endif

TEST test_code_writer_and_ffi_common(void) {
  cdd_code_writer_t w;
  cdd_c_error_t err = CDD_C_SUCCESS;
  char *str = NULL;
  FILE *fp = NULL;
  char long_buf[2200];

  memset(long_buf, 'A', sizeof(long_buf) - 1);
  long_buf[sizeof(long_buf) - 1] = '\0';

  /* Null argument validation */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_code_writer_init_stream(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_code_writer_init_stream(&w, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_code_writer_init_buffer(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_code_writer_free(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_code_writer_indent(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_code_writer_dedent(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_code_writer_write(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_code_writer_writeln(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_code_writer_printf(NULL, "%s", "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_code_writer_check_error(NULL, &err));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_code_writer_check_error(&w, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_code_writer_take_string(NULL, &str));

  /* Buffer mode: empty take_string */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_code_writer_write(&w, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_code_writer_writeln(&w, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_code_writer_printf(&w, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_code_writer_take_string(&w, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_dedent(&w)); /* dedent at 0 */
#ifdef CDD_BUILD_TESTS
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_code_writer_take_string(&w, &str));
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_code_writer_take_string(&w, &str));
#endif
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_take_string(&w, &str));
  ASSERT_STR_EQ("", str);
  free(str);
  str = NULL;

  /* Buffer mode: indent, dedent, writeln, printf (short & >2048), free */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_write(&w, ""));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_write(&w, "\n"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_indent(&w));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_writeln(&w, "line1\nline2"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_dedent(&w));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_printf(&w, "val=%d\n", 42));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_printf(&w, "%s", long_buf));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_check_error(&w, &err));
  ASSERT_EQ(CDD_C_SUCCESS, err);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_free(&w));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_code_writer_free(&w)); /* free when buf is NULL */

  /* Buffer mode: take_string non-empty */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_write(&w, "hello"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_take_string(&w, &str));
  ASSERT_STR_EQ("hello", str);
  free(str);
  str = NULL;

#ifdef CDD_BUILD_TESTS
  /* OOM on realloc in buffer mode */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_code_writer_write(&w, "abc"));
  g_cdd_alloc_fail = 0;
  /* Sticky error on write, writeln, printf */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_code_writer_write(&w, "def"));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_code_writer_writeln(&w, "def"));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_code_writer_printf(&w, "%s", "def"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_free(&w));

  /* OOM on large printf malloc */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_code_writer_printf(&w, "%s", long_buf));
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_free(&w));

  /* Fault injection via g_fail_io_after and g_cdd_fprintf_fail on indent, seg,
   * newline, rem */
  {
    int step;
    for (step = 0; step < 6; ++step) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
      cdd_code_writer_indent(&w);
      g_fail_io_after = step;
      cdd_code_writer_writeln(&w, "seg1\nseg2");
      g_fail_io_after = -1;
      cdd_code_writer_free(&w);
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
    g_cdd_fprintf_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_IO, cdd_code_writer_write(&w, "abc"));
    g_cdd_fprintf_fail = 0;
    cdd_code_writer_free(&w);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
    g_cdd_fprintf_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_IO, cdd_code_writer_writeln(&w, "abc"));
    g_cdd_fprintf_fail = 0;
    cdd_code_writer_free(&w);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_buffer(&w));
    cdd_code_writer_indent(&w);
    g_cdd_fprintf_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_IO, cdd_code_writer_write(&w, "a\nb"));
    g_cdd_fprintf_fail = 0;
    cdd_code_writer_free(&w);
  }
#endif

  /* Stream failure when writing to a read-only FILE* */
  fp = fopen("build/test_ro_stream.txt", "w");
  if (fp) {
    fclose(fp);
  }
  fp = fopen("build/test_ro_stream.txt", "r");
  if (fp) {
    ASSERT_EQ(CDD_C_SUCCESS, cdd_code_writer_init_stream(&w, fp));
    ASSERT_EQ(CDD_C_ERROR_IO, cdd_code_writer_write(&w, "fail_write"));
    fclose(fp);
  }
  remove("build/test_ro_stream.txt");

  /* Test cdd_ffi_emit_common functions */
  {
    cdd_ffi_trivia_t t1, t2;
    cdd_ffi_type_t ty;
    const char *mapped = NULL;
    cdd_ffi_type_map_table_t tbl;

    memset(&tbl, 0, sizeof(tbl));
    memset(&ty, 0, sizeof(ty));
    tbl.void_type = "void";
    tbl.int32_type = "i32";
    tbl.void_ptr_type = "void*";

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_trivia(NULL, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_map_type_from_table(NULL, ty, &mapped));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_map_type_from_table(&tbl, ty, NULL));

    /* char_ptr_type == NULL fallback to void_ptr_type */
    ty.kind = CDD_FFI_KIND_INT8;
    ty.pointer_depth = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_type_from_table(&tbl, ty, &mapped));
    ASSERT_STR_EQ("void*", mapped);

    /* enum_type == NULL fallback to int32_type */
    ty.pointer_depth = 0;
    ty.kind = CDD_FFI_KIND_ENUM_REF;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_type_from_table(&tbl, ty, &mapped));
    ASSERT_STR_EQ("i32", mapped);

    /* trivia with NULL text and fputs error */
    t2.text = cdd_test_mut_str("/* c2 */");
    t2.next = NULL;
    t1.text = NULL;
    t1.next = &t2;
    fp = fopen("build/test_trivia_out.txt", "w");
    if (fp) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_trivia(fp, &t1));
      fclose(fp);
    }
    fp = fopen("build/test_trivia_out.txt", "r");
    if (fp) {
      ASSERT_EQ(CDD_C_ERROR_IO, cdd_ffi_emit_trivia(fp, &t2));
      fclose(fp);
    }
    remove("build/test_trivia_out.txt");

    /* cdd_ffi_open_output_file */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_open_output_file(NULL, "a", &fp));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_open_output_file("build", NULL, &fp));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_open_output_file("build", "a", NULL));
#ifdef CDD_BUILD_TESTS
    g_fail_io_after = 1;
    ASSERT_EQ(CDD_C_ERROR_IO,
              cdd_ffi_open_output_file("build", "test_open_out.txt", &fp));
    g_fail_io_after = 2;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_open_output_file("build", "test_open_out.txt", &fp));
    if (fp) {
      fclose(fp);
      fp = NULL;
    }
    g_fail_io_after = -1;
#endif
    ASSERT_EQ(CDD_C_ERROR_IO,
              cdd_ffi_open_output_file("nonexistent_dir_xyz_99", "a.txt", &fp));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_open_output_file("build", "test_open_out.txt", &fp));
    if (fp)
      fclose(fp);
    remove("build/test_open_out.txt");

    /* span_equals_str & c2openapi_generate NULL args */
    {
      int eq_out = 1;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                span_equals_str(az_span_empty(), "a", NULL));
      ASSERT_EQ(CDD_C_SUCCESS, span_equals_str(az_span_empty(), NULL, &eq_out));
      ASSERT_EQ(0, eq_out);
      ASSERT_EQ(CDD_C_SUCCESS,
                span_equals_str(az_span_create_from_str("a"), "a", &eq_out));
      ASSERT_EQ(1, eq_out);
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_generate(NULL));
    }

    /* copy_operation_fields servers and responses OOM */
    {
      int k;
      const char *pi_json =
          "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
          "\"components\":{\"pathItems\":{\"P\":{\"get\":{\"operationId\":"
          "\"op\","
          "\"servers\":[{\"url\":\"https://"
          "example.com\",\"description\":\"d\"}],"
          "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}},"
          "\"paths\":{\"/p\":{\"$ref\":\"#/components/pathItems/P\"}}}";
      JSON_Value *jv = json_parse_string(pi_json);
      for (k = 1; k < 40; ++k) {
        struct OpenAPI_Spec sp = {0};
        g_cdd_strdup_fail = k;
        if (openapi_load_from_json(jv, &sp) == CDD_C_SUCCESS) {
          openapi_spec_free(&sp);
        }
        g_cdd_strdup_fail = 0;
      }
      json_value_free(jv);
    }
  }

  PASS();
}

extern C_CDD_EXPORT volatile int g_ffi_extractor_alloc_fail;
/* extern C_CDD_EXPORT int g_cdd_ffi_ir_calloc_fail; (moved to global) */

#include "cdd_test_helpers_export.h"
extern CDD_TEST_HELPERS_EXPORT int g_cdd_alloc_fail;

TEST test_cdd_generate_bindings(void) {
  cdd_generate_bindings_config_t config = {0};
  FILE *f;
  size_t i;
  const char *langs[46];

  /* NULL config / args */

  langs[0] = cdd_test_mut_str("python");
  langs[1] = cdd_test_mut_str("rust");
  langs[2] = cdd_test_mut_str("csharp");
  langs[3] = cdd_test_mut_str("typescript");
  langs[4] = cdd_test_mut_str("napi");
  langs[5] = cdd_test_mut_str("java");
  langs[6] = cdd_test_mut_str("cpp");
  langs[7] = cdd_test_mut_str("go");
  langs[8] = cdd_test_mut_str("swift");
  langs[9] = cdd_test_mut_str("dart");
  langs[10] = cdd_test_mut_str("ruby");
  langs[11] = cdd_test_mut_str("kotlin");
  langs[12] = cdd_test_mut_str("php");
  langs[13] = cdd_test_mut_str("lua");
  langs[14] = cdd_test_mut_str("zig");
  langs[15] = cdd_test_mut_str("odin");
  langs[16] = cdd_test_mut_str("julia");
  langs[17] = cdd_test_mut_str("r");
  langs[18] = cdd_test_mut_str("matlab");
  langs[19] = cdd_test_mut_str("haskell");
  langs[20] = cdd_test_mut_str("ocaml");
  langs[21] = cdd_test_mut_str("elixir");
  langs[22] = cdd_test_mut_str("erlang");
  langs[23] = cdd_test_mut_str("common_lisp");
  langs[24] = cdd_test_mut_str("racket");
  langs[25] = cdd_test_mut_str("scheme");
  langs[26] = cdd_test_mut_str("scala");
  langs[27] = cdd_test_mut_str("fsharp");
  langs[28] = cdd_test_mut_str("clojure");
  langs[29] = cdd_test_mut_str("groovy");
  langs[30] = cdd_test_mut_str("webassembly");
  langs[31] = cdd_test_mut_str("wasm");
  langs[32] = cdd_test_mut_str("nim");
  langs[33] = cdd_test_mut_str("vlang");
  langs[34] = cdd_test_mut_str("dlang");
  langs[35] = cdd_test_mut_str("perl");
  langs[36] = cdd_test_mut_str("tcl");
  langs[37] = cdd_test_mut_str("fortran");
  langs[38] = cdd_test_mut_str("delphi");
  langs[39] = cdd_test_mut_str("pascal");
  langs[40] = cdd_test_mut_str("ada");
  langs[41] = cdd_test_mut_str("objc");
  langs[42] = cdd_test_mut_str("objective-c");
  langs[43] = cdd_test_mut_str("crystal");
  langs[44] = cdd_test_mut_str("all");
  langs[45] = cdd_test_mut_str("*");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_bindings(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_bindings(&config));
  config.input = cdd_test_mut_str("input");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_bindings(&config));
  config.output_dir = cdd_test_mut_str("out");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_bindings(&config));

  config.input = cdd_test_mut_str("nonexistent.c");
  config.output_dir = cdd_test_mut_str("out_dir");
  config.target_langs = cdd_test_mut_str("all");
  /* Read file failure */
  ASSERT_NEQ(0, cdd_generate_bindings(&config));

  /* Create a valid dummy file */
  f = fopen("build/test_dummy_bindings.h", "w");
  if (f) {
    fprintf(f, "/* dummy */\nstruct Point { int x; };\n");
    if (f)
      fclose(f);
  }

  config.input = cdd_test_mut_str("build/test_dummy_bindings.h");

  /* Extractor failure */
  g_ffi_extractor_alloc_fail = 1;
  ASSERT_NEQ(0, cdd_generate_bindings(&config));
  g_ffi_extractor_alloc_fail = 0;

  /* Toposort failure */
  g_cdd_ffi_ir_calloc_fail = 1;
  ASSERT_NEQ(0, cdd_generate_bindings(&config));
  g_cdd_ffi_ir_calloc_fail = 0;

  /* Test failure branch (rc != 0) for each language */
  config.output_dir = cdd_test_mut_str("nonexistent_dir_12345/nonexistent");
  for (i = 0; i < sizeof(langs) / sizeof(langs[0]); i++) {
    config.target_langs = langs[i];
    ASSERT_NEQ(0, cdd_generate_bindings(&config));
  }

  /* Test success branch for each language */
  config.output_dir = cdd_test_mut_str("build/test_bindings_out");
  ASSERT_EQ(CDD_C_SUCCESS, makedirs(config.output_dir));
  for (i = 0; i < sizeof(langs) / sizeof(langs[0]); i++) {
    config.target_langs = langs[i];
    ASSERT_EQ(0, cdd_generate_bindings(&config));
  }

  config.target_langs = cdd_test_mut_str("python,wasm");
  ASSERT_EQ(0, cdd_generate_bindings(&config));

  config.target_langs = cdd_test_mut_str("wasm,python");
  ASSERT_EQ(0, cdd_generate_bindings(&config));

  config.target_langs = cdd_test_mut_str("xyzwasmAbc");
  ASSERT_EQ(0, cdd_generate_bindings(&config));

  remove("build/test_dummy_bindings.h");

  PASS();
}

TEST test_cdd_generate_bindings_from_dsl(void) {
  cdd_generate_bindings_config_t config = {0};
  const char *valid_dsl = "interface Test {}";
  const char *invalid_dsl = "invalid dsl ++";

  config.output_dir = "build";

  /* NULL args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_generate_bindings_from_dsl(NULL, &config));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_generate_bindings_from_dsl(valid_dsl, NULL));

  /* Exhaustive OOM testing to cover all error branches in tokenizer and parser
   */
#ifdef CDD_BUILD_TESTS
  {
    int i;
    for (i = 1; i < 50; ++i) {
      cdd_c_error_t rc_dsl;
      g_ffi_extractor_alloc_fail = i;
      rc_dsl = cdd_generate_bindings_from_dsl(valid_dsl, &config);
      ASSERT(rc_dsl == CDD_C_SUCCESS || rc_dsl == CDD_C_ERROR_MEMORY);
    }
    g_ffi_extractor_alloc_fail = 0;
  }
#endif

  {
    cdd_c_error_t rc_inv = cdd_generate_bindings_from_dsl(invalid_dsl, &config);
    ASSERT(rc_inv == CDD_C_SUCCESS || rc_inv == CDD_C_ERROR_PARSE);
  }

  /* valid, no target_langs */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_bindings_from_dsl(valid_dsl, &config));

  /* valid, with target_langs = dsl */
  config.target_langs = "dsl";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_bindings_from_dsl(valid_dsl, &config));

  /* valid, with target_langs not containing dsl */
  config.target_langs = "c";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_bindings_from_dsl(valid_dsl, &config));

  PASS();
}

TEST test_cdd_generate_bindings_json(void) {
  cdd_generate_bindings_config_t config = {0};
  FILE *f;

  /* create dummy .json */
  f = fopen("build/test_dummy.json", "w");
  if (f) {
    fprintf(f, "{\"nodes\":[]}");
    fclose(f);
  } else {
    printf("Failed to create build/test_dummy.json\n");
  }

  config.input = "build/test_dummy.json";
  config.output_dir = "build";
  config.target_langs = "ir"; /* just emit it back as json */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_bindings(&config));

  /* invalid json */
  f = fopen("build/test_dummy_invalid.json", "w");
  if (f) {
    fprintf(f, "invalid");
    fclose(f);
  }
  config.input = "build/test_dummy_invalid.json";
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_generate_bindings(&config));

  /* len <= 5 */
  f = fopen("a.c", "w");
  if (f) {
    fprintf(f, "/* dummy */");
    fclose(f);
  }
  config.input = "a.c";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_bindings(&config));

  remove("build/test_dummy.json");
  remove("build/test_dummy_invalid.json");
  remove("a.c");

  /* cdd_ffi_ir_json.c additional branch coverage */
  {
    cdd_ffi_ir_t ir_tmp;
    cdd_ffi_ir_node_t node_tmp;
    cdd_ffi_ir_t *parsed_ir = NULL;
    char *json_str = NULL;
    memset(&ir_tmp, 0, sizeof(ir_tmp));
    memset(&node_tmp, 0, sizeof(node_tmp));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_ir_to_json(&ir_tmp, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_ir_from_json("{}", NULL));
    node_tmp.kind = CDD_FFI_NODE_FUNCTION;
    node_tmp.return_or_base_type.array_size = -1;
    ir_tmp.nodes = &node_tmp;
    ir_tmp.nodes_count = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_to_json(&ir_tmp, &json_str));
    free(json_str);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_from_json("{}", &parsed_ir));
    if (parsed_ir) {
      cdd_ffi_ir_free(parsed_ir);
      free(parsed_ir);
      parsed_ir = NULL;
    }
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_ffi_ir_from_json(
            "{\"nodes\":[{\"fields\":[{\"type\":{}}],\"base_classes\":[{}]}]}",
            &parsed_ir));
    if (parsed_ir) {
      cdd_ffi_ir_free(parsed_ir);
      free(parsed_ir);
      parsed_ir = NULL;
    }
  }
  PASS();
}

SUITE(cdd_api_suite) {
  RUN_TEST(test_cdd_generate_from_openapi);
  RUN_TEST(test_cdd_generate_to_openapi);
  RUN_TEST(test_cdd_generate_docs_json);
  RUN_TEST(test_cdd_serve_json_rpc);
#ifndef __EMSCRIPTEN__
  RUN_TEST(test_bin_cdd);
#endif
  RUN_TEST(test_code_writer_and_ffi_common);
  RUN_TEST(test_cdd_generate_bindings);
  RUN_TEST(test_cdd_generate_bindings_from_dsl);
  RUN_TEST(test_cdd_generate_bindings_json);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
