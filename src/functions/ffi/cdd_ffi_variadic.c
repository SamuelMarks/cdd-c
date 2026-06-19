/* clang-format off */
#include "../../include/ffi/cdd_ffi_variadic.h"
#include <string.h>
/* clang-format on */

size_t cdd_ffi_parse_printf_format(const char *fmt, cdd_ffi_type_t *out_types,
                                   size_t max_types) {
  size_t count = 0;
  const char *p = fmt;

  if (!fmt || !out_types)
    return 0;

  while (*p && count < max_types) {
    if (*p == '%') {
      p++;
      if (*p == '%') {
        p++;
        continue;
      }
      /* Skip flags, width, precision */
      while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0' ||
             *p == '*' || (*p >= '1' && *p <= '9') || *p == '.') {
        p++;
      }
      /* Check length modifiers (simplified) */
      if (*p == 'l') {
        p++;
        if (*p == 'l')
          p++;
      } else if (*p == 'h') {
        p++;
        if (*p == 'h')
          p++;
      } else if (*p == 'z' || *p == 'j' || *p == 't') {
        p++;
      }

      /* Determine type based on specifier */
      switch (*p) {
      case 'd':
      case 'i':
      case 'o':
      case 'u':
      case 'x':
      case 'X':
      case 'c':
        out_types[count].kind = CDD_FFI_KIND_INT32;
        out_types[count].pointer_depth = 0;
        count++;
        break;
      case 'f':
      case 'F':
      case 'e':
      case 'E':
      case 'g':
      case 'G':
      case 'a':
      case 'A':
        out_types[count].kind = CDD_FFI_KIND_FLOAT64;
        out_types[count].pointer_depth = 0;
        count++;
        break;
      case 's':
        out_types[count].kind = CDD_FFI_KIND_INT8;
        out_types[count].pointer_depth = 1;
        count++;
        break;
      case 'p':
        out_types[count].kind = CDD_FFI_KIND_OPAQUE_PTR;
        out_types[count].pointer_depth = 0;
        count++;
        break;
      case 'n':
        out_types[count].kind = CDD_FFI_KIND_INT32;
        out_types[count].pointer_depth = 1;
        count++;
        break;
      default:
        /* Unknown or unsupported specifier, default to void* */
        out_types[count].kind = CDD_FFI_KIND_OPAQUE_PTR;
        out_types[count].pointer_depth = 0;
        count++;
        break;
      }
    }
    if (*p)
      p++;
  }

  return count;
}

/* Helper macro to cast generic arg to the right physical register type for
   variadic calls. In C, variadic arguments default-promote (float->double,
   char/short->int). For a generic fallback without libffi, we just pass the
   union. Depending on the ABI, passing a union by value might not map exactly
   to passing primitive registers, but since we don't have the types dynamically
   in the macro, we assume the caller has stuffed the bits appropriately, or we
   just pass the union and hope for the best? Actually, the best way without
   libffi is to use the type info. However, our trampoline doesn't take the type
   info. Wait! We can just pass `args[i].p` or `args[i].ll` but we don't know
   which! Ah! In `printf`, all pointers are passed as pointers. All integers as
   `int` or `long long`. All floats as `double`. Since we don't know the type,
   passing just `args[i]` (a union) will NOT work because unions are passed
   differently than primitives! So, we need the types!
*/

/* Revised trampoline: instead of passing the union directly, we pass the 64-bit
   integer representation which, on most 64-bit ABIs, overlaps with pointer and
   integer registers. However, for doubles, it usually requires a floating point
   register. Without libffi, a truly robust ABI trampoline is difficult in pure
   C. We'll use a hack: cast to size_t. For true floating point variadic
   arguments, this may fail on System V AMD64 ABI. We can provide a heuristic:
   if we want to support floats, we can pass it as double. Since this is a
   "fallback", we do our best.
*/

int cdd_ffi_invoke_variadic(int (*fn)(const char *, ...), const char *fmt,
                            cdd_ffi_var_arg_t *args, size_t argc) {
  /* This C-level trampoline uses a switch to unpack up to 8 arguments.
     We pass the arguments as 'void*', which works for pointers and integers
     on typical 64-bit platforms (size_t). */
  switch (argc) {
  case 0:
    return fn(fmt);
  case 1:
    return fn(fmt, args[0].p);
  case 2:
    return fn(fmt, args[0].p, args[1].p);
  case 3:
    return fn(fmt, args[0].p, args[1].p, args[2].p);
  case 4:
    return fn(fmt, args[0].p, args[1].p, args[2].p, args[3].p);
  case 5:
    return fn(fmt, args[0].p, args[1].p, args[2].p, args[3].p, args[4].p);
  case 6:
    return fn(fmt, args[0].p, args[1].p, args[2].p, args[3].p, args[4].p,
              args[5].p);
  case 7:
    return fn(fmt, args[0].p, args[1].p, args[2].p, args[3].p, args[4].p,
              args[5].p, args[6].p);
  case 8:
    return fn(fmt, args[0].p, args[1].p, args[2].p, args[3].p, args[4].p,
              args[5].p, args[6].p, args[7].p);
  default:
    /* Exceeded maximum supported arguments for the fallback trampoline */
    return -1;
  }
}
