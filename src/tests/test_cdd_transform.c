/**
 * @file test_cdd_transform.c
 * @brief Standalone C89 unit test suite for `c_cdd_transform` (including MS-DOS
 * 16-bit I86 and 32-bit I386 targets).
 */

/* clang-format off */
#include <greatest.h>
#include <c_str_span.h>
#include "c_cdd/code_writer.h"
#include "c_cdd/context.h"
#include "cdd_c_error.h"
#include "cdd_cst_transform.h"
#include "cdd_int128.h"
#include "classes/emit/cdd_cst_emit.h"
#include "classes/parse/cdd_cst_parser.h"
#include "routes/parse/cli_cst.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Verify error string formatting and error preservation helpers.
 *
 * @return Greatest test result status.
 */
TEST test_error_helpers(void) {
  const char *msg = NULL;
  cdd_c_error_t rc;

  rc = cdd_c_strerror(CDD_C_SUCCESS, &msg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(msg != NULL);

  rc = cdd_c_strerror(CDD_C_ERROR_INVALID_ARGUMENT, &msg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(msg != NULL);

  rc = cdd_c_strerror(CDD_C_SUCCESS, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_c_keep_err(CDD_C_ERROR_MEMORY, CDD_C_ERROR_IO));
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_c_keep_err(CDD_C_SUCCESS, CDD_C_ERROR_IO));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_c_keep_err(CDD_C_SUCCESS, CDD_C_SUCCESS));
  PASS();
}

/**
 * @brief Verify default `cdd_context_t` initialization and global accessors.
 *
 * @return Greatest test result status.
 */
TEST test_context_defaults(void) {
  cdd_context_t ctx;
  const cdd_context_t *global_ctx = NULL;
  void *ptr = NULL;
  cdd_c_error_t rc;

  rc = cdd_context_init_default(&ctx);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(ctx.allocator.malloc_fn != NULL);
  ASSERT(ctx.allocator.free_fn != NULL);

  rc = ctx.allocator.malloc_fn((size_t)32, &ptr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(ptr != NULL);

  rc = ctx.allocator.free_fn(ptr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_context_get_global(&global_ctx);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(global_ctx != NULL);
  PASS();
}

/**
 * @brief Verify in-memory `cdd_code_writer_t` indentation and formatting.
 *
 * @return Greatest test result status.
 */
TEST test_code_writer_buffer(void) {
  cdd_code_writer_t writer;
  cdd_c_error_t sticky = CDD_C_SUCCESS;
  char *out = NULL;
  cdd_c_error_t rc;

  rc = cdd_code_writer_init_buffer(&writer);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_code_writer_writeln(&writer, "int f(void) {");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_code_writer_indent(&writer);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_code_writer_printf(&writer, "return %d;\n", 42);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_code_writer_dedent(&writer);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_code_writer_write(&writer, "}\n");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_code_writer_check_error(&writer, &sticky);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(CDD_C_SUCCESS, sticky);

  rc = cdd_code_writer_take_string(&writer, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out != NULL);
  ASSERT(strstr(out, "return 42;") != NULL);

  free(out);
  rc = cdd_code_writer_free(&writer);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  PASS();
}

/**
 * @brief Verify 128-bit integer polyfill addition, subtraction, and multiply.
 *
 * @return Greatest test result status.
 */
TEST test_int128_arithmetic(void) {
  cdd_uint128_t a;
  cdd_uint128_t b;
  cdd_uint128_t sum;
  cdd_uint128_t diff;
  cdd_uint128_t prod;
  cdd_c_error_t rc;

  rc = cdd_make_uint128((uint64_t)0, (uint64_t)0xFFFFFFFFUL, &a);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_make_uint128((uint64_t)0, (uint64_t)1, &b);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_uint128_add(a, b, &sum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ((uint64_t)0, sum.high);
  ASSERT_EQ(((uint64_t)1 << 32), sum.low);

  rc = cdd_uint128_sub(sum, b, &diff);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(a.low, diff.low);

  rc = cdd_make_uint128((uint64_t)0, (uint64_t)1000000UL, &a);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_make_uint128((uint64_t)0, (uint64_t)1000000UL, &b);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_uint128_mul(a, b, &prod);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ((uint64_t)0, prod.high);
  ASSERT_EQ(((uint64_t)1000000UL * (uint64_t)1000000UL), prod.low);
  PASS();
}

/**
 * @brief Verify lossless CST parsing and re-emission round-trip.
 *
 * @return Greatest test result status.
 */
TEST test_cst_parse_emit_roundtrip(void) {
  static char src[] = "int add(int a, int b) {\n  return a + b;\n}\n";
  cdd_cst_tree_t *tree = NULL;
  char *emitted = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t free_rc;

  rc = cdd_cst_parse(az_span_create_from_str(src), &tree);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tree != NULL);

  rc = cdd_cst_emit(tree, &emitted);
  free_rc = cdd_cst_tree_free(tree);
  ASSERT_EQ(CDD_C_SUCCESS, free_rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(emitted != NULL);
  ASSERT_STR_EQ(src, emitted);

  free(emitted);
  PASS();
}

/**
 * @brief Verify `extern "C"` CST guard insertion pass.
 *
 * @return Greatest test result status.
 */
TEST test_transform_extern_c_pass(void) {
  static char src[] = "int foo(int x);\n";
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  char *emitted = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t free_rc;

  memset(&cfg, 0, sizeof(cfg));
  rc = cdd_cst_parse(az_span_create_from_str(src), &tree);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_transform_extern_c(tree, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_cst_emit(tree, &emitted);
  free_rc = cdd_cst_tree_free(tree);
  ASSERT_EQ(CDD_C_SUCCESS, free_rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(emitted != NULL);
  ASSERT(strstr(emitted, "__cplusplus") != NULL);
  ASSERT(strstr(emitted, "extern \"C\"") != NULL);

  free(emitted);
  PASS();
}

#if !defined(CDD_DOS_I86_LEAN)
/**
 * @brief Verify C99-to-C89/Watcom CST lowering pass (`//` comments and
 * `for`-init declaration hoisting).
 *
 * @return Greatest test result status.
 */
TEST test_transform_c_lowering_watcom(void) {
  static char src[] = "// line comment\n"
                      "int sum_n(int n) {\n"
                      "  int s = 0;\n"
                      "  for (int i = 0; i < n; i++) {\n"
                      "    s += i;\n"
                      "  }\n"
                      "  return s;\n"
                      "}\n";
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  char *emitted = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t free_rc;

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_c89 = 1;
  cfg.target_profile = CDD_TARGET_PROFILE_WATCOM;

  rc = cdd_cst_parse(az_span_create_from_str(src), &tree);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_transform_c_lowering(tree, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_cst_emit(tree, &emitted);
  free_rc = cdd_cst_tree_free(tree);
  ASSERT_EQ(CDD_C_SUCCESS, free_rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(emitted != NULL);
  ASSERT(strstr(emitted, "/*") != NULL);
  ASSERT(strstr(emitted, "for (int i") == NULL);

  free(emitted);
  PASS();
}

/**
 * @brief Verify multi-pass CST transformation pipeline (`CDD_PASS_GNU` +
 * `CDD_PASS_C_LOWERING`).
 *
 * @return Greatest test result status.
 */
TEST test_transform_pipeline_watcom(void) {
  static char src[] = "__attribute__((unused)) int id(int x) {\n"
                      "  return x;\n"
                      "}\n";
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  char *emitted = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t free_rc;

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_c89 = 1;
  cfg.target_profile = CDD_TARGET_PROFILE_WATCOM;

  rc = cdd_cst_parse(az_span_create_from_str(src), &tree);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_transform_pipeline(
      tree, (unsigned int)(CDD_PASS_GNU | CDD_PASS_C_LOWERING), &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_cst_emit(tree, &emitted);
  free_rc = cdd_cst_tree_free(tree);
  ASSERT_EQ(CDD_C_SUCCESS, free_rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(emitted != NULL);
  ASSERT(strstr(emitted, "__attribute__") == NULL);
  ASSERT(strstr(emitted, "return x;") != NULL);

  free(emitted);
  PASS();
}

/**
 * @brief Verify standalone CLI dispatcher `--help` and invalid subcommand
 * error propagation.
 *
 * @return Greatest test result status.
 */
TEST test_cli_transform_dispatch(void) {
  char prog[] = "cdd-c-transform";
  char help_flag[] = "--help";
  char bad_cmd[] = "non_existent_command";
  char *argv_help[2];
  char *argv_bad[2];
  cdd_c_error_t rc;

  argv_help[0] = prog;
  argv_help[1] = help_flag;
  rc = cli_transform_dispatch_main(2, argv_help);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  argv_bad[0] = prog;
  argv_bad[1] = bad_cmd;
  rc = cli_transform_dispatch_main(2, argv_bad);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  PASS();
}
#endif /* !defined(CDD_DOS_I86_LEAN) */

/**
 * @brief Standalone `c_cdd_transform` test suite definition.
 */
SUITE(cdd_transform_suite) {
  RUN_TEST(test_error_helpers);
  RUN_TEST(test_context_defaults);
  RUN_TEST(test_code_writer_buffer);
  RUN_TEST(test_int128_arithmetic);
  RUN_TEST(test_cst_parse_emit_roundtrip);
  RUN_TEST(test_transform_extern_c_pass);
#if !defined(CDD_DOS_I86_LEAN)
  RUN_TEST(test_transform_c_lowering_watcom);
  RUN_TEST(test_transform_pipeline_watcom);
  RUN_TEST(test_cli_transform_dispatch);
#endif
}

/** @brief Greatest test runner global state definitions. */
GREATEST_MAIN_DEFS();

/**
 * @brief Entry point for the standalone `c_cdd_transform` test runner.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument values.
 * @return 0 if all tests pass, non-zero otherwise.
 */
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cdd_transform_suite);
  GREATEST_MAIN_END();
}
