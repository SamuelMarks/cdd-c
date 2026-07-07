#ifndef TEST_FFI_E2E_H
#define TEST_FFI_E2E_H

/* clang-format off */
#include "../cdd_test_helpers/cdd_helpers.h"
#include "../../cdd_api.h"
#include "../../functions/ffi/cdd_ffi_ir_extractor.h"
#include "../../include/ffi/cdd_ffi_ir.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Headless E2E Automation test.
 *
 * Simulates a complex C header (like a miniature SQLite or cURL API),
 * runs the FFI generator to extract IR, and then invokes multiple language
 * emitters (e.g. python, rust, csharp) all at once to ensure they can run
 * headlessly and successfully process a complete realistic codebase.
 */
TEST test_ffi_e2e_complex_codebase(void) {
  const char *filename = "dummy_complex_lib.h";
  const char *code =
      "/* Dummy Complex Codebase for E2E Testing */\n"
      "\n"
      "typedef enum {\n"
      "  COMPLEX_OK = 0,\n"
      "  COMPLEX_ERROR = 1,\n"
      "  COMPLEX_TIMEOUT = 2\n"
      "} complex_status_t;\n"
      "\n"
      "struct complex_config_t {\n"
      "  int max_retries;\n"
      "  double timeout_seconds;\n"
      "  char *user_agent;\n"
      "  bool use_ssl;\n"
      "};\n"
      "\n"
      "struct complex_context_t {\n"
      "  struct complex_config_t config;\n"
      "  void *internal_state;\n"
      "};\n"
      "\n"
      "complex_status_t complex_init(struct complex_context_t "
      "*ctx, struct complex_config_t *cfg);\n"
      "complex_status_t complex_do_work(struct "
      "complex_context_t *ctx, const char *payload);\n"
      "enum cdd_c_error complex_cleanup(struct complex_context_t *ctx);\n";

  cdd_generate_bindings_config_t config = {0};
  char *output_dir = "test_ffi_e2e_out";
  int rc;
  FILE *f;

  write_to_file(filename, code);
  makedir(output_dir);

  config.input = filename;
  config.output_dir = output_dir;
  config.library_name = "complex_lib";
  config.generate_tests = 1;

  /* Run extraction and emission headlessly for multiple core languages */
  config.target_langs = "python,rust,csharp";

  /* cdd_generate_bindings is the high-level API entry point */
  rc = cdd_generate_bindings(&config);
  ASSERT_EQ(0, rc);

  /* Assert Python bindings generated */
#if defined(_MSC_VER)
  fopen_s(&f, "test_ffi_e2e_out\\cdd_bindings.py", "r");
#else
  f = fopen("test_ffi_e2e_out/cdd_bindings.py", "r");
#endif
  ASSERT_EQ(1, f != NULL);
  if (f) {
    fclose(f);
  }

  /* Assert Rust bindings generated */
#if defined(_MSC_VER)
  fopen_s(&f, "test_ffi_e2e_out\\Cargo.toml", "r");
#else
  f = fopen("test_ffi_e2e_out/Cargo.toml", "r");
#endif
  ASSERT_EQ(1, f != NULL);
  if (f) {
    fclose(f);
  }

  /* Assert C# bindings generated */
#if defined(_MSC_VER)
  fopen_s(&f, "test_ffi_e2e_out\\Bindings.cs", "r");
#else
  f = fopen("test_ffi_e2e_out/Bindings.cs", "r");
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    fclose(f);

  remove(filename);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TEST_FFI_E2E_H */
