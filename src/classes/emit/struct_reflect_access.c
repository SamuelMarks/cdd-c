/**
 * @file struct_reflect_access.c
 * @brief Runtime reflection field lookup, visitors, scalar/string/array
 * accessors, and dynamic array resizing.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/emit/struct_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "win_compat_sym.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/*
 * @brief Looks up a reflected field descriptor by C member name, serialization
 * wire name, deserialization wire name, or registered alias.
 *
 * @param[in] ti Runtime type descriptor table.
 * @param[in] name C field name, wire name, or alias to look up.
 * @param[out] out_field Output pointer receiving the matching field descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_find_field(const struct cdd_type_info *ti,
                                     const char *name,
                                     const struct cdd_field_info **out_field) {
  size_t i, k;
  if (!ti || !name || !out_field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_field = NULL;
  for (i = 0; i < ti->n_fields; ++i) {
    if (strcmp(ti->fields[i].name, name) == 0 ||
        strcmp(ti->fields[i].wire_name, name) == 0 ||
        (ti->fields[i].de_wire_name &&
         strcmp(ti->fields[i].de_wire_name, name) == 0)) {
      *out_field = &ti->fields[i];
      return CDD_C_SUCCESS;
    }
    if (ti->fields[i].aliases) {
      for (k = 0; k < ti->fields[i].n_aliases; ++k) {
        if (strcmp(ti->fields[i].aliases[k], name) == 0) {
          *out_field = &ti->fields[i];
          return CDD_C_SUCCESS;
        }
      }
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Iterates over all fields of a const struct instance `obj` using its
 * runtime `cdd_type_info` table and invokes `visitor` with the computed member
 * address (or `NULL` for bit-fields). For tagged unions (`ti->is_union != 0`),
 * only the active variant matching the integer tag at `ti->tag_offset` is
 * visited.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Callback invoked for each non-skipped field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error code returned
 * by `visitor`.
 */
cdd_c_error_t cdd_reflect_visit_fields(const void *obj,
                                       const struct cdd_type_info *ti,
                                       cdd_field_visitor_fn visitor,
                                       void *user_data) {
  size_t i;
  if (!obj || !ti || !visitor)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (ti->is_union) {
    int active_tag = *(const int *)((const void *)((const unsigned char *)obj +
                                                   ti->tag_offset));
    if (active_tag <= 0 || (size_t)active_tag > ti->n_fields)
      return CDD_C_SUCCESS;
    i = (size_t)(active_tag - 1);
    if (!ti->fields[i].skip) {
      const void *field_ptr = (ti->fields[i].bit_width > 0)
                                  ? NULL
                                  : (const void *)((const unsigned char *)obj +
                                                   ti->fields[i].offset);
      return visitor(&ti->fields[i], field_ptr, user_data);
    }
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < ti->n_fields; ++i) {
    const void *field_ptr;
    cdd_c_error_t rc;
    if (ti->fields[i].skip)
      continue;
    field_ptr =
        (ti->fields[i].bit_width > 0)
            ? NULL
            : (const void *)((const unsigned char *)obj + ti->fields[i].offset);
    rc = visitor(&ti->fields[i], field_ptr, user_data);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Iterates over all fields of a mutable struct instance `obj` using its
 * runtime `cdd_type_info` table and invokes `visitor` with the computed member
 * address (or `NULL` for bit-fields). For tagged unions (`ti->is_union != 0`),
 * only the active variant matching the integer tag at `ti->tag_offset` is
 * visited.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Mutable callback invoked for each non-skipped field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error code returned
 * by `visitor`.
 */
cdd_c_error_t cdd_reflect_visit_fields_mut(void *obj,
                                           const struct cdd_type_info *ti,
                                           cdd_field_mut_visitor_fn visitor,
                                           void *user_data) {
  size_t i;
  if (!obj || !ti || !visitor)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (ti->is_union) {
    int active_tag = *(const int *)((const void *)((const unsigned char *)obj +
                                                   ti->tag_offset));
    if (active_tag <= 0 || (size_t)active_tag > ti->n_fields)
      return CDD_C_SUCCESS;
    i = (size_t)(active_tag - 1);
    if (!ti->fields[i].skip) {
      void *field_ptr =
          (ti->fields[i].bit_width > 0)
              ? NULL
              : (void *)((unsigned char *)obj + ti->fields[i].offset);
      return visitor(&ti->fields[i], field_ptr, user_data);
    }
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < ti->n_fields; ++i) {
    void *field_ptr;
    cdd_c_error_t rc;
    if (ti->fields[i].skip)
      continue;
    field_ptr = (ti->fields[i].bit_width > 0)
                    ? NULL
                    : (void *)((unsigned char *)obj + ti->fields[i].offset);
    rc = visitor(&ti->fields[i], field_ptr, user_data);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines the integer/float pointee byte width and signedness of a
 * reflected field from `field->c_type` and `field->size`.
 *
 * @param[in] field Reflected field descriptor.
 * @param[out] out_width Effective scalar byte width (1, 2, 4, or 8).
 * @param[out] out_unsigned Non-zero if the type is unsigned.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t
cdd_reflect_resolve_field_scalar_width(const struct cdd_field_info *field,
                                       size_t *out_width, int *out_unsigned) {
  const char *ct = field->c_type;
  size_t w = field->size;
  int is_u = 0;

  if (ct && ct[0] == 'u') {
    is_u = 1;
  }
  if (field->is_pointer) {
    if (field->elem_size > 0) {
      w = field->elem_size;
    } else if (ct) {
      if (strstr(ct, "int8") != NULL || strstr(ct, "char") != NULL) {
        w = 1;
      } else if (strstr(ct, "int16") != NULL || strstr(ct, "short") != NULL) {
        w = 2;
      } else if (strstr(ct, "int64") != NULL || strstr(ct, "double") != NULL) {
        w = 8;
      } else if (strstr(ct, "size_t") != NULL) {
        w = sizeof(size_t);
      } else if (strstr(ct, "float") != NULL) {
        w = sizeof(float);
      } else {
        w = sizeof(int);
      }
    } else {
      w = sizeof(int);
    }
  }
  *out_width = w;
  *out_unsigned = is_u;
  return CDD_C_SUCCESS;
}

/*
 * @brief Reads an integer or boolean field (including bit-fields with a
 * registered `get_bitfield` trampoline) from a struct instance by field
 * descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[out] out_val Output integer value.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments or
 * integer overflow outside `[INT_MIN, INT_MAX]`.
 */
cdd_c_error_t cdd_reflect_get_int(const void *obj,
                                  const struct cdd_field_info *field,
                                  int *out_val) {
  int64_t val64 = 0;
  cdd_c_error_t rc;
  if (!obj || !field || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_get_int64(obj, field, &val64);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (val64 < (int64_t)INT_MIN || val64 > (int64_t)INT_MAX)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = (int)val64;
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes an integer or boolean field (including bit-fields with a
 * registered `set_bitfield` trampoline) on a struct instance by field
 * descriptor.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[in] val Integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_set_int(void *obj, const struct cdd_field_info *field,
                                  int val) {
  return cdd_reflect_set_int64(obj, field, (int64_t)val);
}

/*
 * @brief Reads a 64-bit integer field (or narrower integer/bit-field widened to
 * 64 bits) from a struct instance by field descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[out] out_val Output 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_get_int64(const void *obj,
                                    const struct cdd_field_info *field,
                                    int64_t *out_val) {
  const void *addr;
  size_t width = 0;
  int is_unsigned = 0;
  cdd_c_error_t rc;
  if (!obj || !field || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->kind != CDD_FIELD_KIND_INTEGER &&
      field->kind != CDD_FIELD_KIND_BOOLEAN &&
      field->kind != CDD_FIELD_KIND_ENUM)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->bit_width > 0) {
    int bf_val = 0;
    if (!field->get_bitfield)
      return CDD_C_ERROR_NOT_FOUND;
    rc = field->get_bitfield(obj, &bf_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_reflect_resolve_field_scalar_width(field, &width, &is_unsigned);
    if (is_unsigned && field->bit_width == 32U) {
      *out_val = (int64_t)((uint32_t)bf_val);
    } else {
      *out_val = (int64_t)bf_val;
    }
    return rc;
  }
  rc = cdd_reflect_resolve_field_scalar_width(field, &width, &is_unsigned);
  if (field->is_pointer) {
    const void *const *pptr = (const void *const *)((
        const void *)((const unsigned char *)obj + field->offset));
    if (!*pptr)
      return CDD_C_ERROR_NOT_FOUND;
    addr = *pptr;
  } else {
    addr = (const void *)((const unsigned char *)obj + field->offset);
  }
  if (width == 1) {
    *out_val = is_unsigned ? (int64_t)(*(const uint8_t *)addr)
                           : (int64_t)(*(const int8_t *)addr);
  } else if (width == 2) {
    *out_val = is_unsigned ? (int64_t)(*(const uint16_t *)addr)
                           : (int64_t)(*(const int16_t *)addr);
  } else if (width == 8) {
    if (is_unsigned) {
      uint64_t u64 = *(const uint64_t *)addr;
      uint64_t max_i64 = ((uint64_t)1U << 63) - 1U;
      if (u64 > max_i64)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      *out_val = (int64_t)u64;
    } else {
      *out_val = *(const int64_t *)addr;
    }
  } else {
    *out_val = is_unsigned ? (int64_t)(*(const uint32_t *)addr)
                           : (int64_t)(*(const int *)addr);
  }
  return rc;
}

/*
 * @brief Writes a 64-bit integer value onto an integer field (width-aware for
 * 1, 2, 4, or 8 byte integer fields and bit-fields) on a struct instance.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[in] val 64-bit integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_set_int64(void *obj,
                                    const struct cdd_field_info *field,
                                    int64_t val) {
  void *addr;
  size_t width = 0;
  int is_unsigned = 0;
  cdd_c_error_t rc;
  if (!obj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->kind != CDD_FIELD_KIND_INTEGER &&
      field->kind != CDD_FIELD_KIND_BOOLEAN &&
      field->kind != CDD_FIELD_KIND_ENUM)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_resolve_field_scalar_width(field, &width, &is_unsigned);
  if (field->bit_width > 0) {
    if (!field->set_bitfield)
      return CDD_C_ERROR_NOT_FOUND;
    if (field->bit_width < 32U) {
      if (is_unsigned) {
        int64_t max_u = ((int64_t)1 << field->bit_width) - 1;
        if (val < 0 || val > max_u)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      } else {
        int64_t max_s = ((int64_t)1 << (field->bit_width - 1U)) - 1;
        int64_t min_s = -((int64_t)1 << (field->bit_width - 1U));
        if (field->kind != CDD_FIELD_KIND_BOOLEAN &&
            (val < min_s || val > max_s))
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    } else {
      if (is_unsigned) {
        if (val < 0 || val > (int64_t)0xFFFFFFFFUL)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        return field->set_bitfield(obj, (int)((uint32_t)val));
      }
      if (val < (int64_t)INT_MIN || val > (int64_t)INT_MAX)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    return field->set_bitfield(obj, (int)val);
  }
  if (field->is_pointer) {
    void **pptr = (void **)((void *)((unsigned char *)obj + field->offset));
    if (!*pptr)
      return CDD_C_ERROR_NOT_FOUND;
    addr = *pptr;
  } else {
    addr = (void *)((unsigned char *)obj + field->offset);
  }
  if (width == 1) {
    if (is_unsigned) {
      if (val < 0 || val > 255)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      *(uint8_t *)addr = (uint8_t)val;
    } else {
      if (val < -128 || val > 127)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      *(int8_t *)addr = (int8_t)val;
    }
  } else if (width == 2) {
    if (is_unsigned) {
      if (val < 0 || val > 65535)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      *(uint16_t *)addr = (uint16_t)val;
    } else {
      if (val < -32768 || val > 32767)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      *(int16_t *)addr = (int16_t)val;
    }
  } else if (width == 8) {
    if (is_unsigned && val < 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    *(int64_t *)addr = val;
  } else {
    if (is_unsigned) {
      if (val < 0 || val > (int64_t)0xFFFFFFFFUL)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      *(uint32_t *)addr = (uint32_t)val;
    } else {
      if (val < -((int64_t)1 << 31) || val > (((int64_t)1 << 31) - 1))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      *(int *)addr = (int)val;
    }
  }
  return rc;
}

/*
 * @brief Reads an unsigned 64-bit integer field (or narrower integer/bit-field
 * widened to 64 bits) from a struct instance by field descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[out] out_val Output unsigned 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_get_uint64(const void *obj,
                                     const struct cdd_field_info *field,
                                     uint64_t *out_val) {
  const void *addr;
  size_t width = 0;
  int is_unsigned = 0;
  int64_t val64 = 0;
  cdd_c_error_t rc;
  if (!obj || !field || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_resolve_field_scalar_width(field, &width, &is_unsigned);
  if (width == 8 && is_unsigned) {
    if (field->is_pointer) {
      const void *const *pptr = (const void *const *)((
          const void *)((const unsigned char *)obj + field->offset));
      if (!*pptr)
        return CDD_C_ERROR_NOT_FOUND;
      addr = *pptr;
    } else {
      addr = (const void *)((const unsigned char *)obj + field->offset);
    }
    *out_val = *(const uint64_t *)addr;
    return rc;
  }
  rc = cdd_reflect_get_int64(obj, field, &val64);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (val64 < 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = (uint64_t)val64;
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes an unsigned 64-bit integer value onto an integer field
 * (width-aware for 1, 2, 4, or 8 byte integer fields and bit-fields) on a
 * struct instance.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[in] val Unsigned 64-bit integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_set_uint64(void *obj,
                                     const struct cdd_field_info *field,
                                     uint64_t val) {
  size_t width = 0;
  int is_unsigned = 0;
  uint64_t max_i64 = ((uint64_t)1U << 63) - 1U;
  cdd_c_error_t rc;
  if (!obj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (val <= max_i64)
    return cdd_reflect_set_int64(obj, field, (int64_t)val);
  rc = cdd_reflect_resolve_field_scalar_width(field, &width, &is_unsigned);
  if (width == 8 && is_unsigned) {
    *(uint64_t *)((void *)((unsigned char *)obj + field->offset)) = val;
    return rc;
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/*
 * @brief Reads a floating-point field (`double` or `float`) from a struct
 * instance by field descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor (`CDD_FIELD_KIND_NUMBER`).
 * @param[out] out_val Output `double` value.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_get_double(const void *obj,
                                     const struct cdd_field_info *field,
                                     double *out_val) {
  const void *addr;
  size_t width = 0;
  int is_unsigned = 0;
  cdd_c_error_t rc;
  if (!obj || !field || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->kind != CDD_FIELD_KIND_NUMBER)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_resolve_field_scalar_width(field, &width, &is_unsigned);
  if (field->is_pointer) {
    const void *const *pptr = (const void *const *)((
        const void *)((const unsigned char *)obj + field->offset));
    if (!*pptr)
      return CDD_C_ERROR_NOT_FOUND;
    addr = *pptr;
  } else {
    addr = (const void *)((const unsigned char *)obj + field->offset);
  }
  if (width == sizeof(float)) {
    *out_val = (double)(*(const float *)addr);
  } else {
    *out_val = *(const double *)addr;
  }
  return rc;
}

/*
 * @brief Writes a floating-point value (`double` or `float`) onto a struct
 * instance by field descriptor.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor (`CDD_FIELD_KIND_NUMBER`).
 * @param[in] val Floating-point value to store.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_set_double(void *obj,
                                     const struct cdd_field_info *field,
                                     double val) {
  void *addr;
  size_t width = 0;
  int is_unsigned = 0;
  cdd_c_error_t rc;
  if (!obj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->kind != CDD_FIELD_KIND_NUMBER)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_resolve_field_scalar_width(field, &width, &is_unsigned);
  if (field->is_pointer) {
    void **pptr = (void **)((void *)((unsigned char *)obj + field->offset));
    if (!*pptr)
      return CDD_C_ERROR_NOT_FOUND;
    addr = *pptr;
  } else {
    addr = (void *)((unsigned char *)obj + field->offset);
  }
  if (width == sizeof(float)) {
    *(float *)addr = (float)val;
  } else {
    *(double *)addr = val;
  }
  return rc;
}

/*
 * @brief Reads a boolean field (`0` or `1`) from a struct instance by field
 * descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor (`CDD_FIELD_KIND_BOOLEAN`).
 * @param[out] out_val Output boolean value (`0` or `1`).
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_get_bool(const void *obj,
                                   const struct cdd_field_info *field,
                                   int *out_val) {
  int raw = 0;
  cdd_c_error_t rc;
  if (!obj || !field || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->kind != CDD_FIELD_KIND_BOOLEAN)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_get_int(obj, field, &raw);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_val = (raw != 0) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes a boolean value (`0` or `1`) onto a struct instance by field
 * descriptor.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor (`CDD_FIELD_KIND_BOOLEAN`).
 * @param[in] val Boolean value to store (`0` or non-zero).
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t
cdd_reflect_set_bool(void *obj, const struct cdd_field_info *field, int val) {
  if (!obj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->kind != CDD_FIELD_KIND_BOOLEAN)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return cdd_reflect_set_int(obj, field, val ? 1 : 0);
}

/*
 * @brief Reads a string field (`char *` or inline `char[N]`) from a struct
 * instance by field descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[out] out_str Output string pointer.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_get_string(const void *obj,
                                     const struct cdd_field_info *field,
                                     const char **out_str) {
  const char *const *pptr;
  if (!obj || !field || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->kind != CDD_FIELD_KIND_STRING)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!field->is_pointer) {
    *out_str = (const char *)((const void *)((const unsigned char *)obj +
                                             field->offset));
    return CDD_C_SUCCESS;
  }
  pptr = (const char *const *)((const void *)((const unsigned char *)obj +
                                              field->offset));
  *out_str = *pptr;
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes a string onto a struct instance by field descriptor (either
 * duplicating onto a `char *` heap pointer or copying into an inline `char[N]`
 * buffer).
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[in] val NUL-terminated string to store (or NULL to clear).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * CDD_C_ERROR_BUFFER_TOO_SMALL if `val` exceeds an inline buffer, or
 * CDD_C_ERROR_INVALID_ARGUMENT on invalid type/arguments.
 */
cdd_c_error_t cdd_reflect_set_string(void *obj,
                                     const struct cdd_field_info *field,
                                     const char *val) {
  char **pptr;
  char *dup_val = NULL;
  if (!obj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->kind != CDD_FIELD_KIND_STRING)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!field->is_pointer) {
    size_t cap =
        (field->fixed_array_len > 0) ? field->fixed_array_len : field->size;
    char *buf = (char *)((void *)((unsigned char *)obj + field->offset));
    if (!val) {
      buf[0] = '\0';
      return CDD_C_SUCCESS;
    }
    if (strlen(val) >= cap)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    memcpy(buf, val, strlen(val) + 1);
    return CDD_C_SUCCESS;
  }
  if (val) {
    size_t len = strlen(val);
    dup_val = (char *)C_CDD_MALLOC(len + 1);
    if (!dup_val)
      return CDD_C_ERROR_MEMORY;
    memcpy(dup_val, val, len + 1);
  }
  pptr = (char **)((void *)((unsigned char *)obj + field->offset));
  if (*pptr)
    C_CDD_FREE(*pptr);
  *pptr = dup_val;
  return CDD_C_SUCCESS;
}

/*
 * @brief Reads the runtime element count of an array field (either from the
 * companion `n_<field>` counter at `field->array_len_offset` or from
 * `field->fixed_array_len` for inline fixed arrays).
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected array field descriptor (`CDD_FIELD_KIND_ARRAY`).
 * @param[out] out_len Output element count.
 * @return CDD_C_SUCCESS on success, or error code on invalid arguments.
 */
cdd_c_error_t cdd_reflect_array_len(const void *obj,
                                    const struct cdd_field_info *field,
                                    size_t *out_len) {
  if (!obj || !field || !out_len)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->kind != CDD_FIELD_KIND_ARRAY)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->fixed_array_len > 0) {
    *out_len = field->fixed_array_len;
    return CDD_C_SUCCESS;
  }
  if (field->is_flexible_array)
    return CDD_C_ERROR_NOT_FOUND;
  *out_len = *(const size_t *)((const void *)((const unsigned char *)obj +
                                              field->array_len_offset));
  return CDD_C_SUCCESS;
}

/*
 * @brief Computes the runtime pointer to element `index` of a reflected array
 * field (supporting both heap pointer arrays and inline fixed-size arrays).
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected array field descriptor (`CDD_FIELD_KIND_ARRAY`).
 * @param[in] index Zero-based element index.
 * @param[out] out_elem_ptr Output pointer to element `index`.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds or array pointer is NULL, or CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * arguments.
 */
cdd_c_error_t cdd_reflect_array_at(const void *obj,
                                   const struct cdd_field_info *field,
                                   size_t index, const void **out_elem_ptr) {
  size_t len = 0;
  const unsigned char *base;
  cdd_c_error_t rc;
  if (!obj || !field || !out_elem_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_elem_ptr = NULL;
  rc = cdd_reflect_array_len(obj, field, &len);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (index >= len)
    return CDD_C_ERROR_NOT_FOUND;
  if (field->is_pointer) {
    const void *const *pptr = (const void *const *)((
        const void *)((const unsigned char *)obj + field->offset));
    if (!*pptr)
      return CDD_C_ERROR_NOT_FOUND;
    base = (const unsigned char *)(*pptr);
  } else {
    base = (const unsigned char *)obj + field->offset;
  }
  *out_elem_ptr = (const void *)(base + index * field->elem_size);
  return CDD_C_SUCCESS;
}

/*
 * @brief Computes the mutable runtime pointer to element `index` of a reflected
 * array field (supporting both heap pointer arrays and inline fixed-size
 * arrays).
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected array field descriptor (`CDD_FIELD_KIND_ARRAY`).
 * @param[in] index Zero-based element index.
 * @param[out] out_elem_ptr Output mutable pointer to element `index`.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds or array pointer is NULL, or CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * arguments.
 */
cdd_c_error_t cdd_reflect_array_at_mut(void *obj,
                                       const struct cdd_field_info *field,
                                       size_t index, void **out_elem_ptr) {
  const void *const_ptr = NULL;
  cdd_c_error_t rc;
  if (!obj || !field || !out_elem_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_elem_ptr = NULL;
  rc = cdd_reflect_array_at(obj, field, index, &const_ptr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (field->is_pointer) {
    void **pptr = (void **)((void *)((unsigned char *)obj + field->offset));
    *out_elem_ptr =
        (void *)((unsigned char *)(*pptr) + index * field->elem_size);
  } else {
    *out_elem_ptr = (void *)((unsigned char *)obj + field->offset +
                             index * field->elem_size);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Cleans up truncated elements `[new_len .. old_len - 1]` of a dynamic
 * array buffer before shrinking or freeing.
 *
 * @param[in,out] buf Raw array buffer pointer.
 * @param[in] field Reflected array field descriptor.
 * @param[in] new_len New target element count.
 * @param[in] old_len Previous element count.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t
cdd_reflect_cleanup_shrunk_array_elements(void *buf,
                                          const struct cdd_field_info *field,
                                          size_t new_len, size_t old_len) {
  size_t k;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (field->elem_kind == CDD_FIELD_KIND_STRING) {
    char **sarr = (char **)buf;
    for (k = new_len; k < old_len; ++k) {
      if (sarr[k]) {
        C_CDD_FREE(sarr[k]);
        sarr[k] = NULL;
      }
    }
  } else if (field->elem_kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *elem_ti = NULL;
    rc = cdd_reflect_type_by_name(field->ref_name, &elem_ti);
    if (field->elem_size == sizeof(void *)) {
      void **parr = (void **)buf;
      for (k = new_len; k < old_len; ++k) {
        if (parr[k]) {
          if (elem_ti) {
            rc = cdd_reflect_struct_cleanup(parr[k], elem_ti);
          }
          C_CDD_FREE(parr[k]);
          parr[k] = NULL;
        }
      }
    } else if (elem_ti) {
      for (k = new_len; k < old_len; ++k) {
        void *elem = (void *)((unsigned char *)buf + k * field->elem_size);
        rc = cdd_reflect_struct_cleanup(elem, elem_ti);
      }
    }
  }
  if (rc != CDD_C_SUCCESS)
    rc = CDD_C_SUCCESS;
  return rc;
}

/*
 * @brief Dynamically resizes a heap-allocated array field (`is_pointer != 0`,
 * `fixed_array_len == 0`) on a struct instance, zero-initializing any newly
 * grown elements, deeply cleaning up any shrunk string/object elements, and
 * updating the companion `n_<field>` counter at `field->array_len_offset`.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected array field descriptor (`CDD_FIELD_KIND_ARRAY`).
 * @param[in] new_len Desired new element count (`0` frees the array buffer).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT if `field` is not a dynamic heap array.
 */
cdd_c_error_t cdd_reflect_array_resize(void *obj,
                                       const struct cdd_field_info *field,
                                       size_t new_len) {
  void **pptr;
  size_t *len_ptr;
  size_t old_len;
  void *new_buf;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!obj || !field || field->kind != CDD_FIELD_KIND_ARRAY ||
      !field->is_pointer || field->elem_size == 0) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  pptr = (void **)((void *)((unsigned char *)obj + field->offset));
  len_ptr =
      (size_t *)((void *)((unsigned char *)obj + field->array_len_offset));
  old_len = *len_ptr;

  if (new_len < old_len) {
    rc = cdd_reflect_cleanup_shrunk_array_elements(*pptr, field, new_len,
                                                   old_len);
  }

  if (new_len == 0) {
    if (*pptr) {
      C_CDD_FREE(*pptr);
      *pptr = NULL;
    }
    *len_ptr = 0;
    return rc;
  }

  new_buf = C_CDD_REALLOC(*pptr, new_len * field->elem_size);
  if (!new_buf)
    return CDD_C_ERROR_MEMORY;

  if (new_len > old_len) {
    memset((unsigned char *)new_buf + old_len * field->elem_size, 0,
           (new_len - old_len) * field->elem_size);
  }
  *pptr = new_buf;
  *len_ptr = new_len;
  return rc;
}

/**
 * @brief Context wrapper passed to `visit_deep_field_cb`.
 */
struct DeepVisitCtx {
  const void *obj;              /**< Current struct instance */
  cdd_field_visitor_fn visitor; /**< User visitor callback */
  void *user_data;              /**< User opaque context */
  const void *visited[16];      /**< Ancestor pointers for cycle detection */
  unsigned int depth;           /**< Current recursion depth */
};

static cdd_c_error_t
visit_deep_recursive(const void *obj, const struct cdd_type_info *ti,
                     cdd_field_visitor_fn visitor, void *user_data,
                     const void *const *visited, unsigned int depth);

/**
 * @brief Internal visitor callback for `visit_deep_recursive`.
 *
 * @param[in] f Current field descriptor.
 * @param[in] field_ptr Computed field pointer (or NULL for bit-fields).
 * @param[in] user_data Pointer to `struct DeepVisitCtx`.
 * @return CDD_C_SUCCESS on success, or error code from `visitor`.
 */
static cdd_c_error_t visit_deep_field_cb(const struct cdd_field_info *f,
                                         const void *field_ptr,
                                         void *user_data) {
  const struct DeepVisitCtx *ctx = (const struct DeepVisitCtx *)user_data;
  cdd_c_error_t rc = ctx->visitor(f, field_ptr, ctx->user_data);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (f->kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *child_ti = NULL;
    if (cdd_reflect_type_by_name(f->ref_name, &child_ti) == CDD_C_SUCCESS) {
      const void *child_obj =
          f->is_pointer ? *(const void *const *)field_ptr : field_ptr;
      if (child_obj) {
        rc =
            visit_deep_recursive(child_obj, child_ti, ctx->visitor,
                                 ctx->user_data, ctx->visited, ctx->depth + 1U);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  } else if (f->kind == CDD_FIELD_KIND_ARRAY &&
             f->elem_kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *elem_ti = NULL;
    if (cdd_reflect_type_by_name(f->ref_name, &elem_ti) == CDD_C_SUCCESS) {
      size_t alen = 0;
      size_t idx;
      rc = cdd_reflect_array_len(ctx->obj, f, &alen);
      for (idx = 0; idx < alen; ++idx) {
        const void *elem_ptr = NULL;
        rc = cdd_reflect_array_at(ctx->obj, f, idx, &elem_ptr);
        {
          const void *elem_obj = (f->elem_size == sizeof(void *))
                                     ? *(const void *const *)elem_ptr
                                     : elem_ptr;
          if (elem_obj) {
            rc = visit_deep_recursive(elem_obj, elem_ti, ctx->visitor,
                                      ctx->user_data, ctx->visited,
                                      ctx->depth + 1U);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Internal depth-bounded and cycle-safe recursive helper for
 * `cdd_reflect_visit_deep`.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Callback invoked for each visited field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @param[in] visited Ancestor pointer stack for cycle breaking.
 * @param[in] depth Current recursion depth (capped at 16).
 * @return CDD_C_SUCCESS on success, or error code from `visitor`.
 */
static cdd_c_error_t
visit_deep_recursive(const void *obj, const struct cdd_type_info *ti,
                     cdd_field_visitor_fn visitor, void *user_data,
                     const void *const *visited, unsigned int depth) {
  struct DeepVisitCtx ctx;
  unsigned int k;
  if (depth >= 16U)
    return CDD_C_SUCCESS;
  for (k = 0; k < depth; ++k) {
    if (visited[k] == obj)
      return CDD_C_SUCCESS;
    ctx.visited[k] = visited[k];
  }
  ctx.visited[depth] = obj;
  ctx.obj = obj;
  ctx.visitor = visitor;
  ctx.user_data = user_data;
  ctx.depth = depth;
  return cdd_reflect_visit_fields(obj, ti, visit_deep_field_cb, &ctx);
}

/*
 * @brief Recursively visits all fields of `obj` and any nested
 * `CDD_FIELD_KIND_OBJECT` or `CDD_FIELD_KIND_ARRAY` of objects whose target
 * `ref_name` is registered in the runtime type registry.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Callback invoked for each visited field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error code returned
 * by `visitor`.
 */
cdd_c_error_t cdd_reflect_visit_deep(const void *obj,
                                     const struct cdd_type_info *ti,
                                     cdd_field_visitor_fn visitor,
                                     void *user_data) {
  if (!obj || !ti || !visitor)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return visit_deep_recursive(obj, ti, visitor, user_data, NULL, 0U);
}

/**
 * @brief Context wrapper passed to `visit_deep_mut_field_cb`.
 */
struct DeepVisitMutCtx {
  void *obj;                        /**< Current mutable struct instance */
  cdd_field_mut_visitor_fn visitor; /**< User mutable visitor callback */
  void *user_data;                  /**< User opaque context */
  const void *visited[16];          /**< Ancestor pointers for cycle breaking */
  unsigned int depth;               /**< Current recursion depth */
};

static cdd_c_error_t
visit_deep_mut_recursive(void *obj, const struct cdd_type_info *ti,
                         cdd_field_mut_visitor_fn visitor, void *user_data,
                         const void *const *visited, unsigned int depth);

/**
 * @brief Internal visitor callback for `visit_deep_mut_recursive`.
 *
 * @param[in] f Current field descriptor.
 * @param[in,out] field_ptr Computed mutable field pointer (or NULL for
 * bit-fields).
 * @param[in] user_data Pointer to `struct DeepVisitMutCtx`.
 * @return CDD_C_SUCCESS on success, or error code from `visitor`.
 */
static cdd_c_error_t visit_deep_mut_field_cb(const struct cdd_field_info *f,
                                             void *field_ptr, void *user_data) {
  const struct DeepVisitMutCtx *ctx = (const struct DeepVisitMutCtx *)user_data;
  cdd_c_error_t rc = ctx->visitor(f, field_ptr, ctx->user_data);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (f->kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *child_ti = NULL;
    if (cdd_reflect_type_by_name(f->ref_name, &child_ti) == CDD_C_SUCCESS) {
      void *child_obj = f->is_pointer ? *(void **)field_ptr : field_ptr;
      if (child_obj) {
        rc = visit_deep_mut_recursive(child_obj, child_ti, ctx->visitor,
                                      ctx->user_data, ctx->visited,
                                      ctx->depth + 1U);
      }
    }
  } else if (f->kind == CDD_FIELD_KIND_ARRAY &&
             f->elem_kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *elem_ti = NULL;
    if (cdd_reflect_type_by_name(f->ref_name, &elem_ti) == CDD_C_SUCCESS) {
      size_t alen = 0;
      size_t idx;
      rc = cdd_reflect_array_len(ctx->obj, f, &alen);
      for (idx = 0; idx < alen; ++idx) {
        void *elem_ptr = NULL;
        rc = cdd_reflect_array_at_mut(ctx->obj, f, idx, &elem_ptr);
        {
          void *elem_obj =
              (f->elem_size == sizeof(void *)) ? *(void **)elem_ptr : elem_ptr;
          if (elem_obj) {
            rc = visit_deep_mut_recursive(elem_obj, elem_ti, ctx->visitor,
                                          ctx->user_data, ctx->visited,
                                          ctx->depth + 1U);
          }
        }
      }
    }
  }
  return rc;
}

/**
 * @brief Internal depth-bounded and cycle-safe recursive helper for
 * `cdd_reflect_visit_deep_mut`.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Mutable callback invoked for each visited field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @param[in] visited Ancestor pointer stack for cycle breaking.
 * @param[in] depth Current recursion depth (capped at 16).
 * @return CDD_C_SUCCESS on success, or error code from `visitor`.
 */
static cdd_c_error_t
visit_deep_mut_recursive(void *obj, const struct cdd_type_info *ti,
                         cdd_field_mut_visitor_fn visitor, void *user_data,
                         const void *const *visited, unsigned int depth) {
  struct DeepVisitMutCtx ctx;
  unsigned int k;
  if (depth >= 16U)
    return CDD_C_SUCCESS;
  for (k = 0; k < depth; ++k) {
    if (visited[k] == obj)
      return CDD_C_SUCCESS;
    ctx.visited[k] = visited[k];
  }
  ctx.visited[depth] = obj;
  ctx.obj = obj;
  ctx.visitor = visitor;
  ctx.user_data = user_data;
  ctx.depth = depth;
  return cdd_reflect_visit_fields_mut(obj, ti, visit_deep_mut_field_cb, &ctx);
}

/*
 * @brief Recursively visits and mutates all fields of `obj` and any nested
 * `CDD_FIELD_KIND_OBJECT` or `CDD_FIELD_KIND_ARRAY` of objects whose target
 * `ref_name` is registered in the runtime type registry.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Mutable callback invoked for each visited field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error code returned
 * by `visitor`.
 */
cdd_c_error_t cdd_reflect_visit_deep_mut(void *obj,
                                         const struct cdd_type_info *ti,
                                         cdd_field_mut_visitor_fn visitor,
                                         void *user_data) {
  if (!obj || !ti || !visitor)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return visit_deep_mut_recursive(obj, ti, visitor, user_data, NULL, 0U);
}
