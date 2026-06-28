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
  cdd_ffi_type_t types[10];
  size_t count;

  enum cdd_c_error rc;

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

  PASS();
}

TEST test_ffi_variadic_invoke(void) {
  cdd_ffi_var_arg_t args[3];
  enum cdd_c_error result;
  int d_val = 42;
  char *s_val = "test";
  void *p_val = &d_val;

  args[0].p = (void *)(size_t)d_val;
  args[1].p = s_val;
  args[2].p = p_val;

  result = cdd_ffi_invoke_variadic(dummy_variadic_func, "%d %s %p", args, 3);
  ASSERT_EQ(CDD_C_SUCCESS, result);
  ASSERT_EQ(3, dummy_variadic_func_call_count);

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
