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
  const char *filename = (char *)(size_t)(size_t) "dummy_complex_lib.h";
  const char code[] = {
      47,  42,  32,  68,  117, 109, 109, 121, 32,  67,  111, 109, 112, 108, 101,
      120, 32,  67,  111, 100, 101, 98,  97,  115, 101, 32,  102, 111, 114, 32,
      69,  50,  69,  32,  84,  101, 115, 116, 105, 110, 103, 32,  42,  47,  10,
      10,  116, 121, 112, 101, 100, 101, 102, 32,  101, 110, 117, 109, 32,  123,
      10,  32,  32,  67,  79,  77,  80,  76,  69,  88,  95,  79,  75,  32,  61,
      32,  48,  44,  10,  32,  32,  67,  79,  77,  80,  76,  69,  88,  95,  69,
      82,  82,  79,  82,  32,  61,  32,  49,  44,  10,  32,  32,  67,  79,  77,
      80,  76,  69,  88,  95,  84,  73,  77,  69,  79,  85,  84,  32,  61,  32,
      50,  10,  125, 32,  99,  111, 109, 112, 108, 101, 120, 95,  115, 116, 97,
      116, 117, 115, 95,  116, 59,  10,  10,  115, 116, 114, 117, 99,  116, 32,
      99,  111, 109, 112, 108, 101, 120, 95,  99,  111, 110, 102, 105, 103, 95,
      116, 32,  123, 10,  32,  32,  105, 110, 116, 32,  109, 97,  120, 95,  114,
      101, 116, 114, 105, 101, 115, 59,  10,  32,  32,  100, 111, 117, 98,  108,
      101, 32,  116, 105, 109, 101, 111, 117, 116, 95,  115, 101, 99,  111, 110,
      100, 115, 59,  10,  32,  32,  99,  104, 97,  114, 32,  42,  117, 115, 101,
      114, 95,  97,  103, 101, 110, 116, 59,  10,  32,  32,  98,  111, 111, 108,
      32,  117, 115, 101, 95,  115, 115, 108, 59,  10,  125, 59,  10,  10,  115,
      116, 114, 117, 99,  116, 32,  99,  111, 109, 112, 108, 101, 120, 95,  99,
      111, 110, 116, 101, 120, 116, 95,  116, 32,  123, 10,  32,  32,  115, 116,
      114, 117, 99,  116, 32,  99,  111, 109, 112, 108, 101, 120, 95,  99,  111,
      110, 102, 105, 103, 95,  116, 32,  99,  111, 110, 102, 105, 103, 59,  10,
      32,  32,  118, 111, 105, 100, 32,  42,  105, 110, 116, 101, 114, 110, 97,
      108, 95,  115, 116, 97,  116, 101, 59,  10,  125, 59,  10,  10,  99,  111,
      109, 112, 108, 101, 120, 95,  115, 116, 97,  116, 117, 115, 95,  116, 32,
      99,  111, 109, 112, 108, 101, 120, 95,  105, 110, 105, 116, 40,  115, 116,
      114, 117, 99,  116, 32,  99,  111, 109, 112, 108, 101, 120, 95,  99,  111,
      110, 116, 101, 120, 116, 95,  116, 32,  42,  99,  116, 120, 44,  32,  115,
      116, 114, 117, 99,  116, 32,  99,  111, 109, 112, 108, 101, 120, 95,  99,
      111, 110, 102, 105, 103, 95,  116, 32,  42,  99,  102, 103, 41,  59,  10,
      99,  111, 109, 112, 108, 101, 120, 95,  115, 116, 97,  116, 117, 115, 95,
      116, 32,  99,  111, 109, 112, 108, 101, 120, 95,  100, 111, 95,  119, 111,
      114, 107, 40,  115, 116, 114, 117, 99,  116, 32,  99,  111, 109, 112, 108,
      101, 120, 95,  99,  111, 110, 116, 101, 120, 116, 95,  116, 32,  42,  99,
      116, 120, 44,  32,  99,  111, 110, 115, 116, 32,  99,  104, 97,  114, 32,
      42,  112, 97,  121, 108, 111, 97,  100, 41,  59,  10,  99,  100, 100, 95,
      99,  95,  101, 114, 114, 111, 114, 95,  116, 32,  99,  111, 109, 112, 108,
      101, 120, 95,  99,  108, 101, 97,  110, 117, 112, 40,  115, 116, 114, 117,
      99,  116, 32,  99,  111, 109, 112, 108, 101, 120, 95,  99,  111, 110, 116,
      101, 120, 116, 95,  116, 32,  42,  99,  116, 120, 41,  59,  10,  0};

  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_ffi_e2e_out";
  int rc;
  FILE *f;

  (void)rc;
  write_to_file(filename, code);
  makedir(output_dir);

  config.input = filename;
  config.output_dir = output_dir;
  config.library_name = (char *)(size_t)(size_t) "complex_lib";
  config.generate_tests = 1;

  /* Run extraction and emission headlessly for multiple core languages */
  config.target_langs = (char *)(size_t)(size_t) "python,rust,csharp";

  /* cdd_generate_bindings is the high-level API entry point */
  rc = cdd_generate_bindings(&config);
  ASSERT_EQ(0, rc);

  /* Assert Python bindings generated */
#if defined(_MSC_VER)
  fopen_s(&f, "test_ffi_e2e_out\\cdd_bindings.py", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_ffi_e2e_out/cdd_bindings.py", "r") != 0)
    f = NULL;
#else
  f = fopen("test_ffi_e2e_out/cdd_bindings.py", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f) {
    if (f)
      fclose(f);
  }

  /* Assert Rust bindings generated */
#if defined(_MSC_VER)
  fopen_s(&f, "test_ffi_e2e_out\\Cargo.toml", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_ffi_e2e_out/Cargo.toml", "r") != 0)
    f = NULL;
#else
  f = fopen("test_ffi_e2e_out/Cargo.toml", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f) {
    if (f)
      fclose(f);
  }

  /* Assert C# bindings generated */
#if defined(_MSC_VER)
  fopen_s(&f, "test_ffi_e2e_out\\Bindings.cs", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_ffi_e2e_out/Bindings.cs", "r") != 0)
    f = NULL;
#else
  f = fopen("test_ffi_e2e_out/Bindings.cs", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  remove(filename);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TEST_FFI_E2E_H */
