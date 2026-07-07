/**
 * @file test_preprocessor_macros.h
 * @brief Unit tests for preprocessor macros parsing.
 */

#ifndef TEST_PREPROCESSOR_MACROS_H
#define TEST_PREPROCESSOR_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "functions/parse/preprocessor.h"
#include "functions/parse/macro_evaluator.h"
/* clang-format on */

/**
 * @brief Tests basic evaluation of arithmetic and logical macro expressions.
 *
 * @return The result of the test.
 */
TEST test_macro_evaluator_basic(void) {
  struct PreprocessorContext ctx;
  cdd_macro_eval_result_t res;
  int rc;

  pp_context_init(&ctx);

  /* Test 1: Simple addition */
  rc = cdd_macro_evaluate(&ctx, "1 + 2 * 3", &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(MACRO_EVAL_TYPE_INT, res.type);
  ASSERT_EQ(7, res.int_val);
  cdd_macro_eval_result_free(&res);

  /* Test 2: Bitwise and shifts */
  rc = cdd_macro_evaluate(&ctx, "(1 << 3) | 2", &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(MACRO_EVAL_TYPE_INT, res.type);
  ASSERT_EQ(10, res.int_val);
  cdd_macro_eval_result_free(&res);

  /* Test 3: Logical operations */
  rc = cdd_macro_evaluate(&ctx, "1 && (0 || !0)", &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(MACRO_EVAL_TYPE_INT, res.type);
  ASSERT_EQ(1, res.int_val);
  cdd_macro_eval_result_free(&res);

  /* Test 4: Macro referencing another macro */
  write_to_file("test_eval_macros.h", "#define A 10\n#define B (A + 5)\n");
  rc = pp_scan_defines(&ctx, "test_eval_macros.h");
  ASSERT_EQ(0, rc);
  rc = cdd_macro_evaluate(&ctx, "B * 2", &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(MACRO_EVAL_TYPE_INT, res.type);
  ASSERT_EQ(30, res.int_val);
  cdd_macro_eval_result_free(&res);

  pp_context_free(&ctx);
  remove("test_eval_macros.h");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests parsing of object-like macros.
 *
 * @return The result of the test.
 */
TEST test_pp_define_object_like(void) {
  const char *fname = "test_defs.h";
  int rc;
  struct PreprocessorContext ctx;

  write_to_file(fname, "#define MAX_SIZE 100\n#define PI 3.14\n");

  pp_context_init(&ctx);
  rc = pp_scan_defines(&ctx, fname);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(2, ctx.macro_count);
  ASSERT_STR_EQ("MAX_SIZE", ctx.macros[0].name);
  ASSERT_EQ(0, ctx.macros[0].is_function_like);

  ASSERT_STR_EQ("PI", ctx.macros[1].name);
  ASSERT_EQ(0, ctx.macros[1].is_function_like);

  pp_context_free(&ctx);
  remove(fname);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests parsing of function-like macros.
 *
 * @return The result of the test.
 */
TEST test_pp_define_function_like(void) {
  const char *fname = "test_func_macros.h";
  int rc;
  struct PreprocessorContext ctx;

  write_to_file(fname, "#define MIN(a, b) ((a)<(b)?(a):(b))\n");

  pp_context_init(&ctx);
  rc = pp_scan_defines(&ctx, fname);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, ctx.macro_count);
  ASSERT_STR_EQ("MIN", ctx.macros[0].name);
  ASSERT(ctx.macros[0].is_function_like);
  ASSERT_EQ(2, ctx.macros[0].arg_count);
  ASSERT_STR_EQ("a", ctx.macros[0].args[0]);
  ASSERT_STR_EQ("b", ctx.macros[0].args[1]);

  pp_context_free(&ctx);
  remove(fname);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests parsing of standard variadic macros.
 *
 * @return The result of the test.
 */
TEST test_pp_define_variadic_standard(void) {
  const char *fname = "test_variadic.h";
  int rc;
  struct PreprocessorContext ctx;

  /* Standard C99: trailing ellipsis */
  write_to_file(fname, "#define LOG(level, ...) printf(level, __VA_ARGS__)\n");

  pp_context_init(&ctx);
  rc = pp_scan_defines(&ctx, fname);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, ctx.macro_count);
  ASSERT_STR_EQ("LOG", ctx.macros[0].name);
  ASSERT(ctx.macros[0].is_function_like);
  ASSERT(ctx.macros[0].is_variadic);

  /* Should recognise 'level' as explicit arg, '...' implies variadic */
  ASSERT_EQ(1, ctx.macros[0].arg_count);
  ASSERT_STR_EQ("level", ctx.macros[0].args[0]);

  pp_context_free(&ctx);
  remove(fname);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests parsing of empty variadic macros.
 *
 * @return The result of the test.
 */
TEST test_pp_define_variadic_empty(void) {
  const char *fname = "test_var_empty.h";
  int rc;
  struct PreprocessorContext ctx;

  /* #define TRACE(...) */
  write_to_file(fname, "#define TRACE(...) trace_impl(__VA_ARGS__)\n");

  pp_context_init(&ctx);
  rc = pp_scan_defines(&ctx, fname);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, ctx.macro_count);
  ASSERT_STR_EQ("TRACE", ctx.macros[0].name);
  ASSERT(ctx.macros[0].is_variadic);
  ASSERT_EQ(0, ctx.macros[0].arg_count);

  pp_context_free(&ctx);
  remove(fname);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests parsing of GCC-style named variadic macros.
 *
 * @return The result of the test.
 */
TEST test_pp_define_variadic_gcc(void) {
  const char *fname = "test_var_gcc.h";
  int rc;
  struct PreprocessorContext ctx;

  /* GCC named variadic: #define LOG(args...) */
  /* Our parser detects this if it finds ID then ... */
  write_to_file(fname, "#define LOG(args...) printf(args)\n");

  pp_context_init(&ctx);
  rc = pp_scan_defines(&ctx, fname);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, ctx.macro_count);
  ASSERT_STR_EQ("LOG", ctx.macros[0].name);
  ASSERT(ctx.macros[0].is_variadic);
  ASSERT_EQ(1, ctx.macros[0].arg_count);
  ASSERT_STR_EQ("args", ctx.macros[0].args[0]);

  pp_context_free(&ctx);
  remove(fname);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Preprocessor macros test suite.
 */
SUITE(preprocessor_macros_suite) {
  RUN_TEST(test_macro_evaluator_basic);
  RUN_TEST(test_pp_define_object_like);
  RUN_TEST(test_pp_define_function_like);
  RUN_TEST(test_pp_define_variadic_standard);
  RUN_TEST(test_pp_define_variadic_empty);
  RUN_TEST(test_pp_define_variadic_gcc);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_PREPROCESSOR_MACROS_H */
