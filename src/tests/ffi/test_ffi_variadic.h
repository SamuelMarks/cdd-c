/**
 * @file test_ffi_variadic.h
 * @brief Unit tests and mocks in test_ffi_variadic.h.
 */
/* clang-format off */
#ifndef TEST_FFI_VARIADIC_H
#define TEST_FFI_VARIADIC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "greatest.h"
#include "cdd_c_error.h"
#include "../../../include/ffi/cdd_ffi_variadic.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* clang-format on */

/** @brief Test definition dummy_variadic_func_call_count. */
static int dummy_variadic_func_call_count = 0;

/**
 * @brief Helper function dummy_variadic_func.
 * @param[in] fmt Parameter fmt.
 * @return Result status or value.
 */
static cdd_c_error_t dummy_variadic_func(const char *fmt, ...) {
  va_list args;
  const char *p;
  int val = 0;
  char *s = NULL;
  void *ptr = NULL;

  dummy_variadic_func_call_count = 0;
  va_start(args, fmt);
  for (p = fmt; *p != '\0'; p++) {
    if (*p == '%') {
      p++;
      if (*p == 'd') {
        val = va_arg(args, int);
        dummy_variadic_func_call_count++;
      } else if (*p == 's') {
        s = va_arg(args, char *);
        dummy_variadic_func_call_count++;
      } else if (*p == 'p') {
        ptr = va_arg(args, void *);
        dummy_variadic_func_call_count++;
      }
    }
  }
  (void)ptr;
  (void)val;
  (void)s;
  va_end(args);
  return CDD_C_SUCCESS;
}

/**
 * @brief Unit test test_ffi_variadic_format_parser.
 * @return Greatest test result.
 */
TEST test_ffi_variadic_format_parser(void) {
  const char *fmt = cdd_test_mut_str("Hello %d %s %p %% %f");
  cdd_ffi_type_t types[20];
  size_t count;

  cdd_c_error_t rc = 0;

  /* Invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_parse_printf_format(NULL, types, 10, &count));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_parse_printf_format(fmt, NULL, 10, &count));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_parse_printf_format(fmt, types, 10, NULL));

  rc = cdd_ffi_parse_printf_format(fmt, types, 10, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(4, count);

  ASSERT_EQ(CDD_FFI_KIND_INT32, types[0].kind);
  ASSERT_EQ(0, types[0].pointer_depth);

  ASSERT_EQ(CDD_FFI_KIND_INT8, types[1].kind);
  ASSERT_EQ(1, types[1].pointer_depth);

  ASSERT_EQ(CDD_FFI_KIND_OPAQUE_PTR, types[2].kind);
  ASSERT_EQ(0, types[2].pointer_depth);

  ASSERT_EQ(CDD_FFI_KIND_FLOAT64, types[3].kind);
  ASSERT_EQ(0, types[3].pointer_depth);

  /* More format specifiers */
  fmt = "%lld %ld %hd %hhd %zd %jd %td %o %u %x %X %c";
  rc = cdd_ffi_parse_printf_format(fmt, types, 15, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(12, count);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[0].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[1].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[2].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[3].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[4].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[5].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[6].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[7].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[8].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[9].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[10].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[11].kind);

  fmt = "%e %E %g %G %a %A %n %w"; /* %w is unknown */
  rc = cdd_ffi_parse_printf_format(fmt, types, 10, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(8, count);
  ASSERT_EQ(CDD_FFI_KIND_FLOAT64, types[0].kind);
  ASSERT_EQ(CDD_FFI_KIND_FLOAT64, types[1].kind);
  ASSERT_EQ(CDD_FFI_KIND_FLOAT64, types[2].kind);
  ASSERT_EQ(CDD_FFI_KIND_FLOAT64, types[3].kind);
  ASSERT_EQ(CDD_FFI_KIND_FLOAT64, types[4].kind);
  ASSERT_EQ(CDD_FFI_KIND_FLOAT64, types[5].kind);

  ASSERT_EQ(CDD_FFI_KIND_INT32, types[6].kind);
  ASSERT_EQ(1, types[6].pointer_depth);

  ASSERT_EQ(CDD_FFI_KIND_OPAQUE_PTR, types[7].kind);

  /* Trailing percent and width/precision test */
  fmt = "%+10.5d %- *#d %";
  rc = cdd_ffi_parse_printf_format(fmt, types, 20, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(3, count);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[0].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, types[1].kind);
  ASSERT_EQ(CDD_FFI_KIND_OPAQUE_PTR, types[2].kind);

  /* Test max_types limit */
  rc = cdd_ffi_parse_printf_format("%d %f %s", types, 2, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, count);

  PASS();
}

/**
 * @brief Unit test test_ffi_variadic_invoke.
 * @return Greatest test result.
 */
TEST test_ffi_variadic_invoke(void) {
  cdd_ffi_var_arg_t args[10];
  cdd_c_error_t result;
  int d_val = 42;
  char s_buf[] = "test";
  char *s_val = s_buf;
  void *p_val = &d_val;

  memset(args, 0, sizeof(args));
  args[0].i = d_val;
  args[1].p = s_val;
  args[2].p = p_val;

  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "", args, 0);
  ASSERT_EQ(CDD_C_SUCCESS, result);
  ASSERT_EQ(0, dummy_variadic_func_call_count);

  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "%d", args, 1);
  ASSERT_EQ(CDD_C_SUCCESS, result);
  ASSERT_EQ(1, dummy_variadic_func_call_count);

  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "%d %s", args, 2);
  ASSERT_EQ(CDD_C_SUCCESS, result);
  ASSERT_EQ(2, dummy_variadic_func_call_count);

  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "%d %s %p", args, 3);
  ASSERT_EQ(CDD_C_SUCCESS, result);
  ASSERT_EQ(3, dummy_variadic_func_call_count);

  /* Set up more dummy args just to call them */
  args[3].p = NULL;
  args[4].p = NULL;
  args[5].p = NULL;
  args[6].p = NULL;
  args[7].p = NULL;
  args[8].p = NULL;

  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "", args, 4);
  ASSERT_EQ(CDD_C_SUCCESS, result);
  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "", args, 5);
  ASSERT_EQ(CDD_C_SUCCESS, result);
  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "", args, 6);
  ASSERT_EQ(CDD_C_SUCCESS, result);
  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "", args, 7);
  ASSERT_EQ(CDD_C_SUCCESS, result);
  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "", args, 8);
  ASSERT_EQ(CDD_C_SUCCESS, result);

  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "", args, 9);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, result);

  {
    char buf[256];
    char small_buf[6];
    char hello_buf[] = "hello";
    char empty_buf[] = "";
    cdd_ffi_var_arg_t fmt_args[10];
    int dummy_target = 7;

    memset(fmt_args, 0, sizeof(fmt_args));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_format_variadic(NULL, sizeof(buf), "%d", fmt_args, 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_format_variadic(buf, 0, "%d", fmt_args, 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_format_variadic(buf, sizeof(buf), NULL, fmt_args, 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_format_variadic(buf, sizeof(buf), "%d", NULL, 1));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_format_variadic(buf, sizeof(buf), "plain", NULL, 0));
    ASSERT_STR_EQ("plain", buf);

    /* Unterminated or missing argument */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_format_variadic(buf, sizeof(buf), "%", NULL, 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_format_variadic(buf, sizeof(buf), "%d", NULL, 0));

    /* All types and 10 arguments (> 8-arg trampoline limit) */
    fmt_args[0].i = -42;
    fmt_args[1].u = 42u;
    fmt_args[2].l = 100000L;
    fmt_args[3].ll = (int64_t)900000L * (int64_t)10000L;
    fmt_args[4].d = 3.5;
    fmt_args[5].d = 2.25;
    fmt_args[6].p = hello_buf;
    fmt_args[7].p = empty_buf;
    fmt_args[8].p = (void *)&dummy_target;
    fmt_args[9].i = 99;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_format_variadic(
                  buf, sizeof(buf),
                  "%% %d %u %ld %lld %-+ #05.1f %g %s %s %p %d", fmt_args, 10));
    ASSERT(strstr(buf, "% -42 42 100000 9000000000 ") != NULL);
    ASSERT(strstr(buf, "2.25 hello  ") != NULL);
    ASSERT(strstr(buf, " 99") != NULL);

    /* Truncation when output buffer capacity is exceeded */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_format_variadic(small_buf, sizeof(small_buf), "%s-%d",
                                      fmt_args + 6, 2));
    ASSERT_STR_EQ("hello", small_buf);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_format_variadic(small_buf, sizeof(small_buf), "%ld",
                                      fmt_args + 2, 1));
    ASSERT_STR_EQ("10000", small_buf);
  }

  PASS();
}

/**
 * @brief Executes the ffi_variadic_suite test suite.
 */
SUITE(ffi_variadic_suite) {
  RUN_TEST(test_ffi_variadic_format_parser);
  RUN_TEST(test_ffi_variadic_invoke);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_FFI_VARIADIC_H */
