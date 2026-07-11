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

static int dummy_variadic_func_call_count = 0;

static enum cdd_c_error dummy_variadic_func(const char *fmt, ...) {
  va_list args;
  const char *p;

  dummy_variadic_func_call_count = 0;
  va_start(args, fmt);
  for (p = fmt; *p != '\0'; p++) {
    if (*p == '%') {
      p++;
      if (*p == 'd') {
        int val = va_arg(args, int);
        (void)val;
        dummy_variadic_func_call_count++;
      } else if (*p == 's') {
        char *s = va_arg(args, char *);
        (void)s;
        dummy_variadic_func_call_count++;
      } else if (*p == 'p') {
        void *ptr = va_arg(args, void *);
        (void)ptr;
        dummy_variadic_func_call_count++;
      }
    }
  }
  va_end(args);
  return CDD_C_SUCCESS;
}

TEST test_ffi_variadic_format_parser(void) {
  const char *fmt = "Hello %d %s %p %% %f";
  cdd_ffi_type_t types[20];
  size_t count;

  enum cdd_c_error rc;

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

TEST test_ffi_variadic_invoke(void) {
  cdd_ffi_var_arg_t args[10];
  enum cdd_c_error result;
  int d_val = 42;
  char *s_val = "test";
  void *p_val = &d_val;

  args[0].p = (void *)(size_t)d_val;
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
  ASSERT_EQ((enum cdd_c_error) - 1, result);

  PASS();
}

SUITE(ffi_variadic_suite) {
  RUN_TEST(test_ffi_variadic_format_parser);
  RUN_TEST(test_ffi_variadic_invoke);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_FFI_VARIADIC_H */
