/**
 * @file cdd_ffi_emit_common.c
 * @brief Implementation of shared FFI type-mapping, trivia emission, and
 * file-scaffolding utilities.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_common.h"
#include "c_cdd/safe_crt.h"
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_fail_io_after;
#endif

cdd_c_error_t cdd_ffi_emit_trivia(FILE *f, const cdd_ffi_trivia_t *trivia) {
  if (!f)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  while (trivia) {
    if (trivia->text) {
      if (fputs(trivia->text, f) < 0)
        return CDD_C_ERROR_IO;
    }
    trivia = trivia->next;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_ffi_map_type_from_table(const cdd_ffi_type_map_table_t *table,
                                          cdd_ffi_type_t type,
                                          const char **out_type) {
  if (!table || !out_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      *out_type =
          table->char_ptr_type ? table->char_ptr_type : table->void_ptr_type;
      return CDD_C_SUCCESS;
    }
    *out_type = table->void_ptr_type;
    return CDD_C_SUCCESS;
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = table->void_type;
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = table->bool_type;
    break;
  case CDD_FFI_KIND_INT8:
    *out_type = table->int8_type;
    break;
  case CDD_FFI_KIND_UINT8:
    *out_type = table->uint8_type;
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = table->int16_type;
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = table->uint16_type;
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = table->int32_type;
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = table->uint32_type;
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = table->int64_type;
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = table->uint64_type;
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = table->float32_type;
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = table->float64_type;
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    if (table->use_ref_name_for_struct && type.ref_name) {
      *out_type = type.ref_name;
    } else {
      *out_type = table->void_ptr_type;
    }
    break;
  case CDD_FFI_KIND_ENUM_REF:
    if (table->use_ref_name_for_enum && type.ref_name) {
      *out_type = type.ref_name;
    } else {
      *out_type = table->enum_type ? table->enum_type : table->int32_type;
    }
    break;
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (table->use_ref_name_for_typedef && type.ref_name) {
      *out_type = type.ref_name;
    } else {
      *out_type = table->void_ptr_type;
    }
    break;
  case CDD_FFI_KIND_FUNCTION_PTR:
    *out_type = table->function_ptr_type ? table->function_ptr_type
                                         : table->void_ptr_type;
    break;
  case CDD_FFI_KIND_OPAQUE_PTR:
  default:
    *out_type = table->void_ptr_type;
    break;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_ffi_open_output_file(const char *output_dir,
                                       const char *filename, FILE **out_fp) {
  char filepath[1024];
  FILE *fp = NULL;

  if (!output_dir || !filename || !out_fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_fp = NULL;

#if defined(_WIN32)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s", output_dir, filename);
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s", output_dir, filename);
#endif

#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
    return CDD_C_ERROR_IO;
  }
#endif

#if defined(_MSC_VER)
  if (fopen_s(&fp, filepath, "w") != 0)
    fp = NULL;
#else
  fp = fopen(filepath, "w");
#endif

  if (!fp)
    return CDD_C_ERROR_IO;

  *out_fp = fp;
  return CDD_C_SUCCESS;
}
