/**
 * @file cdd_ffi_ir_types.c
 * @brief Type mapping and template type parsing for FFI IR.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/format_specifiers.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "functions/ffi/cdd_ffi_ir_internal.h"
#include "ffi/cdd_ffi_ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Parses a C++ template type string into an FFI type representation.
 *
 * @param c_type C++ template type string.
 * @param out_type Destination FFI type pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
cdd_c_error_t parse_template_type(const char *c_type,
                                  cdd_ffi_type_t *out_type) {
  const char *lt;
  const char *gt;
  size_t base_len;
  char *base_name;
  char *inner_type_str;
  size_t inner_len;
  const char *inner_lt;
  const char *inner_gt;
  cdd_c_error_t rc;

  if (!c_type || !out_type) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  lt = strchr(c_type, '<');
  gt = strrchr(c_type, '>');

  if (!lt || !gt || gt < lt) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  base_len = (size_t)(lt - c_type);
  base_name = (char *)(size_t)CDD_MALLOC(base_len + 1);
  if (!base_name)
    return CDD_C_ERROR_MEMORY;
#if defined(_MSC_VER)
  strncpy_s(base_name, base_len + 1, c_type, base_len);
#else
  strncpy(base_name, c_type, base_len);
#endif
  base_name[base_len] = '\0';

  out_type->ref_name = base_name;

  inner_len = (size_t)(gt - lt - 1);
  inner_type_str = (char *)(size_t)CDD_MALLOC(inner_len + 1);
  if (!inner_type_str) {
    free(base_name);
    out_type->ref_name = NULL;
    return CDD_C_ERROR_MEMORY;
  }
#if defined(_MSC_VER)
  strncpy_s(inner_type_str, inner_len + 1, lt + 1, inner_len);
#else
  strncpy(inner_type_str, lt + 1, inner_len);
#endif
  inner_type_str[inner_len] = '\0';

  out_type->template_args_count = 1;
  out_type->template_args =
      (cdd_ffi_type_t *)CDD_CALLOC(1, sizeof(cdd_ffi_type_t));
  if (!out_type->template_args) {
    free(inner_type_str);
    free(base_name);
    out_type->ref_name = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  rc = map_c_type_to_ffi_kind(inner_type_str, &out_type->template_args[0].kind);
  if (rc != CDD_C_SUCCESS) {
    free(inner_type_str);
    free(base_name);
    return rc;
  }
  if (out_type->template_args[0].kind == CDD_FFI_KIND_STRUCT_REF ||
      out_type->template_args[0].kind == CDD_FFI_KIND_TEMPLATE_STRUCT_REF) {
    inner_lt = strchr(inner_type_str, '<');
    inner_gt = strrchr(inner_type_str, '>');
    if (inner_lt && inner_gt && inner_gt > inner_lt) {
      cdd_c_error_t rc_ex =
          parse_template_type(inner_type_str, &out_type->template_args[0]);
      if (rc_ex != CDD_C_SUCCESS) {
        free(inner_type_str);
        return rc_ex;
      }
    } else {
      out_type->template_args[0].ref_name = CDD_STRDUP(inner_type_str);
      if (!out_type->template_args[0].ref_name) {
        free(inner_type_str);
        return CDD_C_ERROR_MEMORY;
      }
    }
  }

  free(inner_type_str);
  return CDD_C_SUCCESS;
}

cdd_c_error_t map_c_type_to_ffi_kind(const char *c_type,
                                     cdd_ffi_primitive_kind_t *out_kind) {
  if (!out_kind)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!c_type) {
    *out_kind = CDD_FFI_KIND_VOID;
    return CDD_C_SUCCESS;
  }

  if (c_type[0] == '\0') {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (strstr(c_type, "std::string") || strstr(c_type, "std_string")) {
    *out_kind = CDD_FFI_KIND_STD_STRING;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "std::vector") || strstr(c_type, "std_vector")) {
    *out_kind = CDD_FFI_KIND_STD_VECTOR;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "std::shared_ptr") || strstr(c_type, "std_shared_ptr")) {
    *out_kind = CDD_FFI_KIND_STD_SHARED_PTR;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "std::unique_ptr") || strstr(c_type, "std_unique_ptr")) {
    *out_kind = CDD_FFI_KIND_STD_UNIQUE_PTR;
    return CDD_C_SUCCESS;
  }

  if (strchr(c_type, '<') && strchr(c_type, '>')) {
    *out_kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    return CDD_C_SUCCESS;
  }

  if (strstr(c_type, "uint8_t") || strcmp(c_type, "unsigned char") == 0) {
    *out_kind = CDD_FFI_KIND_UINT8;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int8_t") || strcmp(c_type, "char") == 0) {
    *out_kind = CDD_FFI_KIND_INT8;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint16_t") || strcmp(c_type, "unsigned short") == 0) {
    *out_kind = CDD_FFI_KIND_UINT16;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int16_t") || strcmp(c_type, "short") == 0) {
    *out_kind = CDD_FFI_KIND_INT16;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint32_t") || strcmp(c_type, "unsigned int") == 0) {
    *out_kind = CDD_FFI_KIND_UINT32;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int32_t") || strcmp(c_type, "int") == 0 ||
      strcmp(c_type, "integer") == 0) {
    *out_kind = CDD_FFI_KIND_INT32;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint64_t") || strcmp(c_type, "unsigned long long") == 0) {
    *out_kind = CDD_FFI_KIND_UINT64;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int64_t") || strcmp(c_type, "long long") == 0) {
    *out_kind = CDD_FFI_KIND_INT64;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "float")) {
    *out_kind = CDD_FFI_KIND_FLOAT32;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "double") || strcmp(c_type, "number") == 0) {
    *out_kind = CDD_FFI_KIND_FLOAT64;
    return CDD_C_SUCCESS;
  }
  if (strcmp(c_type, "boolean") == 0 || strstr(c_type, "bool")) {
    *out_kind = CDD_FFI_KIND_BOOL;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "void")) {
    *out_kind = CDD_FFI_KIND_VOID;
    return CDD_C_SUCCESS;
  }

  *out_kind = CDD_FFI_KIND_STRUCT_REF;
  return CDD_C_SUCCESS;
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_mangle_cpp_name(const char *ns_name,
                                                   const char *class_name,
                                                   const char *method_name,
                                                   char **out_mangled) {
  size_t len = 0;
  char *mangled = NULL;

  if (!method_name || !out_mangled) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (ns_name)
    len += strlen(ns_name) + 1;
  if (class_name)
    len += strlen(class_name) + 1;
  len += strlen(method_name) + 1;

  mangled = (char *)(size_t)CDD_MALLOC(len);
  if (!mangled)
    return CDD_C_ERROR_MEMORY;

#if defined(_MSC_VER)
  if (ns_name && class_name) {
    sprintf_s(mangled, len, "%s_%s_%s", ns_name, class_name, method_name);
  } else if (ns_name) {
    sprintf_s(mangled, len, "%s_%s", ns_name, method_name);
  } else if (class_name) {
    sprintf_s(mangled, len, "%s_%s", class_name, method_name);
  } else {
    strcpy_s(mangled, len, method_name);
  }
#else
  if (ns_name && class_name) {
    CDD_SNPRINTF(mangled, 1024, "%s_%s_%s", ns_name, class_name, method_name);
  } else if (ns_name) {
    CDD_SNPRINTF(mangled, 1024, "%s_%s", ns_name, method_name);
  } else if (class_name) {
    CDD_SNPRINTF(mangled, 1024, "%s_%s", class_name, method_name);
  } else {
    strcpy(mangled, method_name);
  }
#endif

  *out_mangled = mangled;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT cdd_c_error_t cdd_ffi_map_c_type_to_ffi_kind_test(
    const char *c_type, cdd_ffi_primitive_kind_t *out_kind) {
  return map_c_type_to_ffi_kind(c_type, out_kind);
}

C_CDD_EXPORT cdd_c_error_t
cdd_ffi_parse_template_type_test(const char *c_type, cdd_ffi_type_t *out_type) {
  return parse_template_type(c_type, out_type);
}
#endif /* CDD_BUILD_TESTS */
