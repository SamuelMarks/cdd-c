/**
 * @file struct_reflect_registry.c
 * @brief Runtime reflection type, enum, and function registries and path
 * resolution.
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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Dynamic runtime registry of struct/union descriptors */
static const struct cdd_type_info **g_cdd_type_registry = NULL;
/** @brief Count of registered struct/union descriptors */
static size_t g_cdd_type_registry_count = 0;
/** @brief Capacity of registered struct/union descriptors */
static size_t g_cdd_type_registry_cap = 0;

/** @brief Dynamic runtime registry of enum descriptors */
static const struct cdd_enum_info **g_cdd_enum_registry = NULL;
/** @brief Count of registered enum descriptors */
static size_t g_cdd_enum_registry_count = 0;
/** @brief Capacity of registered enum descriptors */
static size_t g_cdd_enum_registry_cap = 0;

/** @brief Dynamic runtime registry of function descriptors */
static const struct cdd_func_info **g_cdd_func_registry = NULL;
/** @brief Count of registered function descriptors */
static size_t g_cdd_func_registry_count = 0;
/** @brief Capacity of registered function descriptors */
static size_t g_cdd_func_registry_cap = 0;

/*
 * @brief Looks up a reflected enum variant descriptor by C member name, wire
 * name, or alias.
 *
 * @param[in] ei Runtime enum descriptor table.
 * @param[in] name C variant suffix name, wire name, or alias to look up.
 * @param[out] out_variant Output pointer receiving the matching variant.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_find_enum_variant(
    const struct cdd_enum_info *ei, const char *name,
    const struct cdd_enum_variant_info **out_variant) {
  size_t i, k;
  if (!ei || !name || !out_variant)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_variant = NULL;
  for (i = 0; i < ei->n_variants; ++i) {
    if (strcmp(ei->variants[i].name, name) == 0 ||
        strcmp(ei->variants[i].wire_name, name) == 0) {
      *out_variant = &ei->variants[i];
      return CDD_C_SUCCESS;
    }
    if (ei->variants[i].aliases) {
      for (k = 0; k < ei->variants[i].n_aliases; ++k) {
        if (ei->variants[i].aliases[k] &&
            strcmp(ei->variants[i].aliases[k], name) == 0) {
          *out_variant = &ei->variants[i];
          return CDD_C_SUCCESS;
        }
      }
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Looks up a reflected enum variant descriptor by integer enumerator
 * value.
 *
 * @param[in] ei Runtime enum descriptor table.
 * @param[in] value Integer enumerator value to search for.
 * @param[out] out_variant Output pointer receiving the matching variant.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_find_enum_by_value(
    const struct cdd_enum_info *ei, int value,
    const struct cdd_enum_variant_info **out_variant) {
  size_t i;
  if (!ei || !out_variant)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_variant = NULL;
  for (i = 0; i < ei->n_variants; ++i) {
    if (ei->variants[i].value == value) {
      *out_variant = &ei->variants[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Registers a `cdd_type_info` descriptor in the runtime type registry.
 *
 * @param[in] ti Runtime struct/union type descriptor to register.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL/unnamed descriptor.
 */
cdd_c_error_t cdd_reflect_register_type(const struct cdd_type_info *ti) {
  size_t i;
  if (!ti || !ti->name || ti->name[0] == '\0')
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < g_cdd_type_registry_count; ++i) {
    if (strcmp(g_cdd_type_registry[i]->name, ti->name) == 0) {
      g_cdd_type_registry[i] = ti;
      return CDD_C_SUCCESS;
    }
  }
  if (g_cdd_type_registry_count >= g_cdd_type_registry_cap) {
    size_t new_cap =
        (g_cdd_type_registry_cap == 0) ? 8 : g_cdd_type_registry_cap * 2;
    const struct cdd_type_info **new_arr =
        (const struct cdd_type_info **)C_CDD_REALLOC(
            (void *)g_cdd_type_registry,
            new_cap * sizeof(const struct cdd_type_info *));
    if (!new_arr)
      return CDD_C_ERROR_MEMORY;
    g_cdd_type_registry = new_arr;
    g_cdd_type_registry_cap = new_cap;
  }
  g_cdd_type_registry[g_cdd_type_registry_count++] = ti;
  return CDD_C_SUCCESS;
}

/*
 * @brief Looks up a registered `cdd_type_info` descriptor by struct name.
 *
 * @param[in] name Struct/union tag name.
 * @param[out] out_ti Output pointer receiving the registered descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_type_by_name(const char *name,
                                       const struct cdd_type_info **out_ti) {
  size_t i;
  if (!name || !out_ti)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ti = NULL;
  for (i = 0; i < g_cdd_type_registry_count; ++i) {
    if (strcmp(g_cdd_type_registry[i]->name, name) == 0) {
      *out_ti = g_cdd_type_registry[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Registers a `cdd_enum_info` descriptor in the runtime enum registry.
 *
 * @param[in] ei Runtime enum descriptor to register.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL/unnamed descriptor.
 */
cdd_c_error_t cdd_reflect_register_enum(const struct cdd_enum_info *ei) {
  size_t i;
  if (!ei || !ei->name || ei->name[0] == '\0')
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < g_cdd_enum_registry_count; ++i) {
    if (strcmp(g_cdd_enum_registry[i]->name, ei->name) == 0) {
      g_cdd_enum_registry[i] = ei;
      return CDD_C_SUCCESS;
    }
  }
  if (g_cdd_enum_registry_count >= g_cdd_enum_registry_cap) {
    size_t new_cap =
        (g_cdd_enum_registry_cap == 0) ? 8 : g_cdd_enum_registry_cap * 2;
    const struct cdd_enum_info **new_arr =
        (const struct cdd_enum_info **)C_CDD_REALLOC(
            (void *)g_cdd_enum_registry,
            new_cap * sizeof(const struct cdd_enum_info *));
    if (!new_arr)
      return CDD_C_ERROR_MEMORY;
    g_cdd_enum_registry = new_arr;
    g_cdd_enum_registry_cap = new_cap;
  }
  g_cdd_enum_registry[g_cdd_enum_registry_count++] = ei;
  return CDD_C_SUCCESS;
}

/*
 * @brief Looks up a registered `cdd_enum_info` descriptor by enum name.
 *
 * @param[in] name Enum tag name.
 * @param[out] out_ei Output pointer receiving the registered enum descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_enum_by_name(const char *name,
                                       const struct cdd_enum_info **out_ei) {
  size_t i;
  if (!name || !out_ei)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ei = NULL;
  for (i = 0; i < g_cdd_enum_registry_count; ++i) {
    if (strcmp(g_cdd_enum_registry[i]->name, name) == 0) {
      *out_ei = g_cdd_enum_registry[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Registers a `cdd_func_info` descriptor in the runtime function
 * registry.
 *
 * @param[in] fi Runtime function descriptor to register.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL/unnamed descriptor.
 */
cdd_c_error_t cdd_reflect_register_func(const struct cdd_func_info *fi) {
  size_t i;
  if (!fi || !fi->name || fi->name[0] == '\0')
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < g_cdd_func_registry_count; ++i) {
    if (strcmp(g_cdd_func_registry[i]->name, fi->name) == 0) {
      g_cdd_func_registry[i] = fi;
      return CDD_C_SUCCESS;
    }
  }
  if (g_cdd_func_registry_count >= g_cdd_func_registry_cap) {
    size_t new_cap =
        (g_cdd_func_registry_cap == 0) ? 8 : g_cdd_func_registry_cap * 2;
    const struct cdd_func_info **new_arr =
        (const struct cdd_func_info **)C_CDD_REALLOC(
            (void *)g_cdd_func_registry,
            new_cap * sizeof(const struct cdd_func_info *));
    if (!new_arr)
      return CDD_C_ERROR_MEMORY;
    g_cdd_func_registry = new_arr;
    g_cdd_func_registry_cap = new_cap;
  }
  g_cdd_func_registry[g_cdd_func_registry_count++] = fi;
  return CDD_C_SUCCESS;
}

/*
 * @brief Looks up a registered `cdd_func_info` descriptor by function name.
 *
 * @param[in] name C function identifier.
 * @param[out] out_fi Output pointer receiving the registered function
 * descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_func_by_name(const char *name,
                                       const struct cdd_func_info **out_fi) {
  size_t i;
  if (!name || !out_fi)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_fi = NULL;
  for (i = 0; i < g_cdd_func_registry_count; ++i) {
    if (strcmp(g_cdd_func_registry[i]->name, name) == 0) {
      *out_fi = g_cdd_func_registry[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Looks up a parameter descriptor inside a `cdd_func_info` by parameter
 * name.
 *
 * @param[in] fi Runtime function descriptor.
 * @param[in] name Parameter identifier to search for.
 * @param[out] out_param Output pointer receiving the matching parameter
 * descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_find_param(const struct cdd_func_info *fi,
                                     const char *name,
                                     const struct cdd_param_info **out_param) {
  size_t i;
  if (!fi || !name || !out_param)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_param = NULL;
  for (i = 0; i < fi->n_params; ++i) {
    if (fi->params[i].name && strcmp(fi->params[i].name, name) == 0) {
      *out_param = &fi->params[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Retrieves the number of currently registered struct/union types in the
 * runtime type registry.
 *
 * @param[out] out_count Output count of registered types.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_count` is NULL.
 */
cdd_c_error_t cdd_reflect_registered_types_count(size_t *out_count) {
  if (!out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_count = g_cdd_type_registry_count;
  return CDD_C_SUCCESS;
}

/*
 * @brief Retrieves the registered `cdd_type_info` descriptor at `index`.
 *
 * @param[in] index Zero-based index into the type registry.
 * @param[out] out_ti Output pointer receiving the descriptor.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds, or CDD_C_ERROR_INVALID_ARGUMENT if `out_ti` is NULL.
 */
cdd_c_error_t
cdd_reflect_registered_type_at(size_t index,
                               const struct cdd_type_info **out_ti) {
  if (!out_ti)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ti = NULL;
  if (index >= g_cdd_type_registry_count)
    return CDD_C_ERROR_NOT_FOUND;
  *out_ti = g_cdd_type_registry[index];
  return CDD_C_SUCCESS;
}

/*
 * @brief Retrieves the number of currently registered enum descriptors in the
 * runtime enum registry.
 *
 * @param[out] out_count Output count of registered enums.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_count` is NULL.
 */
cdd_c_error_t cdd_reflect_registered_enums_count(size_t *out_count) {
  if (!out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_count = g_cdd_enum_registry_count;
  return CDD_C_SUCCESS;
}

/*
 * @brief Retrieves the registered `cdd_enum_info` descriptor at `index`.
 *
 * @param[in] index Zero-based index into the enum registry.
 * @param[out] out_ei Output pointer receiving the descriptor.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds, or CDD_C_ERROR_INVALID_ARGUMENT if `out_ei` is NULL.
 */
cdd_c_error_t
cdd_reflect_registered_enum_at(size_t index,
                               const struct cdd_enum_info **out_ei) {
  if (!out_ei)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ei = NULL;
  if (index >= g_cdd_enum_registry_count)
    return CDD_C_ERROR_NOT_FOUND;
  *out_ei = g_cdd_enum_registry[index];
  return CDD_C_SUCCESS;
}

/*
 * @brief Retrieves the number of currently registered function descriptors in
 * the runtime function registry.
 *
 * @param[out] out_count Output count of registered functions.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_count` is NULL.
 */
cdd_c_error_t cdd_reflect_registered_funcs_count(size_t *out_count) {
  if (!out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_count = g_cdd_func_registry_count;
  return CDD_C_SUCCESS;
}

/*
 * @brief Retrieves the registered `cdd_func_info` descriptor at `index`.
 *
 * @param[in] index Zero-based index into the function registry.
 * @param[out] out_fi Output pointer receiving the descriptor.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds, or CDD_C_ERROR_INVALID_ARGUMENT if `out_fi` is NULL.
 */
cdd_c_error_t
cdd_reflect_registered_func_at(size_t index,
                               const struct cdd_func_info **out_fi) {
  if (!out_fi)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_fi = NULL;
  if (index >= g_cdd_func_registry_count)
    return CDD_C_ERROR_NOT_FOUND;
  *out_fi = g_cdd_func_registry[index];
  return CDD_C_SUCCESS;
}

/*
 * @brief Clears all dynamically registered struct, enum, and function
 * descriptors from the runtime type registry and frees internal storage.
 *
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t cdd_reflect_clear_registry(void) {
  if (g_cdd_type_registry) {
    C_CDD_FREE((void *)g_cdd_type_registry);
    g_cdd_type_registry = NULL;
  }
  g_cdd_type_registry_count = 0;
  g_cdd_type_registry_cap = 0;

  if (g_cdd_enum_registry) {
    C_CDD_FREE((void *)g_cdd_enum_registry);
    g_cdd_enum_registry = NULL;
  }
  g_cdd_enum_registry_count = 0;
  g_cdd_enum_registry_cap = 0;

  if (g_cdd_func_registry) {
    C_CDD_FREE((void *)g_cdd_func_registry);
    g_cdd_func_registry = NULL;
  }
  g_cdd_func_registry_count = 0;
  g_cdd_func_registry_cap = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts the next path token (RFC 6901 `/seg` or dot/bracket
 * `seg[0].next`) from `*p` into `seg_buf`.
 *
 * @param[in,out] p Cursor into the path string.
 * @param[out] seg_buf Output token buffer.
 * @param[in] seg_cap Capacity of `seg_buf`.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on empty or
 * oversized segment.
 */
static cdd_c_error_t next_path_segment(const char **p, char *seg_buf,
                                       size_t seg_cap) {
  size_t len = 0;
  const char *cur = *p;
  int in_bracket = 0;
  if (*cur == '[') {
    in_bracket = 1;
    cur++;
  }
  for (; *cur != '\0'; cur++) {
    if (in_bracket) {
      if (*cur == ']')
        break;
    } else if (*cur == '/' || *cur == '.' || *cur == '[') {
      break;
    }
    if (len + 1 >= seg_cap)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (*cur == '~' && (cur[1] == '0' || cur[1] == '1')) {
      seg_buf[len++] = (cur[1] == '0') ? '~' : '/';
      cur++;
    } else {
      seg_buf[len++] = *cur;
    }
  }
  *p = cur + (in_bracket && *cur == ']' ? 1 : 0);
  seg_buf[len] = '\0';
  return (in_bracket && (*cur != ']' || len == 0))
             ? CDD_C_ERROR_INVALID_ARGUMENT
             : CDD_C_SUCCESS;
}

/**
 * @brief Resolves a dot-path or RFC 6901 JSON Pointer path to a mutable field
 * address and descriptor within a reflected struct instance.
 *
 * @param[in,out] obj Root struct instance.
 * @param[in] ti Root runtime type descriptor.
 * @param[in] path Dot-separated or slash-separated field path.
 * @param[out] out_field Optional output pointer receiving the leaf field info.
 * @param[out] out_parent_obj Optional output pointer receiving the parent
 * struct instance.
 * @param[out] out_field_ptr Optional output pointer receiving the leaf field
 * memory address.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t resolve_path_mut(void *obj, const struct cdd_type_info *ti,
                                      const char *path,
                                      const struct cdd_field_info **out_field,
                                      void **out_parent_obj,
                                      void **out_field_ptr) {
  const char *p = path;
  void *cur_obj = obj;
  const struct cdd_type_info *cur_ti = ti;
  const struct cdd_field_info *cur_field = NULL;
  void *parent_obj = NULL;
  void *cur_ptr = NULL;
  cdd_c_error_t rc;

  if (!obj || !ti || !path)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  while (*p == '/' || *p == '.')
    p++;
  if (*p == '\0')
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (;;) {
    char seg[128];
    rc = next_path_segment(&p, seg, sizeof(seg));
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = cdd_reflect_find_field(cur_ti, seg, &cur_field);
    if (rc != CDD_C_SUCCESS)
      return rc;

    parent_obj = cur_obj;
    cur_ptr = (cur_field->bit_width > 0)
                  ? NULL
                  : (void *)((unsigned char *)cur_obj + cur_field->offset);

    while (*p == '/' || *p == '.')
      p++;
    if (*p == '\0')
      break;

    if (cur_field->kind == CDD_FIELD_KIND_OBJECT) {
      if (!cur_field->ref_name || cur_field->ref_name[0] == '\0')
        return CDD_C_ERROR_NOT_FOUND;
      rc = cdd_reflect_type_by_name(cur_field->ref_name, &cur_ti);
      if (rc != CDD_C_SUCCESS)
        return rc;
      cur_obj = cur_field->is_pointer ? *(void **)cur_ptr : cur_ptr;
      if (!cur_obj)
        return CDD_C_ERROR_NOT_FOUND;
    } else if (cur_field->kind == CDD_FIELD_KIND_ARRAY) {
      char idx_seg[64];
      size_t idx = 0;
      size_t k;
      void *elem_ptr = NULL;
      rc = next_path_segment(&p, idx_seg, sizeof(idx_seg));
      if (rc != CDD_C_SUCCESS)
        return rc;
      for (k = 0; idx_seg[k] != '\0'; ++k) {
        if (!isdigit((unsigned char)idx_seg[k]))
          return CDD_C_ERROR_INVALID_ARGUMENT;
        idx = idx * 10 + (size_t)(idx_seg[k] - '0');
      }
      rc = cdd_reflect_array_at_mut(cur_obj, cur_field, idx, &elem_ptr);
      if (rc != CDD_C_SUCCESS)
        return rc;
      cur_ptr = elem_ptr;
      while (*p == '/' || *p == '.')
        p++;
      if (*p == '\0')
        break;
      if (cur_field->elem_kind != CDD_FIELD_KIND_OBJECT ||
          !cur_field->ref_name || cur_field->ref_name[0] == '\0')
        return CDD_C_ERROR_NOT_FOUND;
      rc = cdd_reflect_type_by_name(cur_field->ref_name, &cur_ti);
      if (rc != CDD_C_SUCCESS)
        return rc;
      cur_obj = (cur_field->elem_size == sizeof(void *)) ? *(void **)elem_ptr
                                                         : elem_ptr;
      if (!cur_obj)
        return CDD_C_ERROR_NOT_FOUND;
    } else {
      return CDD_C_ERROR_NOT_FOUND;
    }
  }

  *out_field = cur_field;
  *out_parent_obj = parent_obj;
  *out_field_ptr = cur_ptr;
  return CDD_C_SUCCESS;
}

/*
 * @brief Resolves a JSON Pointer (`"/user/items/0/id"`) or dot/bracket path
 * (`"user.items[0].id"`) against a reflected struct hierarchy.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path Path string to resolve.
 * @param[out] out_field Output pointer receiving the leaf field descriptor.
 * @param[out] out_parent_obj Output pointer receiving the immediate parent
 * struct containing the resolved leaf field.
 * @param[out] out_field_ptr Output pointer receiving the address of the leaf
 * field or array element (or `NULL` for bit-fields).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if any segment is
 * missing or NULL, or CDD_C_ERROR_INVALID_ARGUMENT on invalid arguments.
 */
cdd_c_error_t cdd_reflect_resolve_path(const void *obj,
                                       const struct cdd_type_info *ti,
                                       const char *path,
                                       const struct cdd_field_info **out_field,
                                       const void **out_parent_obj,
                                       const void **out_field_ptr) {
  void *mut_parent = NULL;
  void *mut_field = NULL;
  union {
    const void *c_ptr;
    void *m_ptr;
  } u_root;
  cdd_c_error_t rc;
  if (!out_field || !out_parent_obj || !out_field_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_field = NULL;
  *out_parent_obj = NULL;
  *out_field_ptr = NULL;
  u_root.c_ptr = obj;
  rc = resolve_path_mut(u_root.m_ptr, ti, path, out_field, &mut_parent,
                        &mut_field);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_parent_obj = mut_parent;
  *out_field_ptr = mut_field;
  return CDD_C_SUCCESS;
}

/*
 * @brief Reads a 64-bit integer value at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_val Output 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_get_int64_by_path(const void *obj,
                                            const struct cdd_type_info *ti,
                                            const char *path,
                                            int64_t *out_val) {
  const struct cdd_field_info *f = NULL;
  const void *parent_obj = NULL;
  const void *field_ptr = NULL;
  cdd_c_error_t rc;
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_resolve_path(obj, ti, path, &f, &parent_obj, &field_ptr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = f->elem_kind;
    elem_f.size = f->elem_size;
    return cdd_reflect_get_int64(field_ptr, &elem_f, out_val);
  }
  return cdd_reflect_get_int64(parent_obj, f, out_val);
}

/*
 * @brief Writes a 64-bit integer value at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val 64-bit integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_set_int64_by_path(void *obj,
                                            const struct cdd_type_info *ti,
                                            const char *path, int64_t val) {
  const struct cdd_field_info *f = NULL;
  void *mut_parent = NULL;
  void *mut_field = NULL;
  cdd_c_error_t rc =
      resolve_path_mut(obj, ti, path, &f, &mut_parent, &mut_field);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = f->elem_kind;
    elem_f.size = f->elem_size;
    return cdd_reflect_set_int64(mut_field, &elem_f, val);
  }
  return cdd_reflect_set_int64(mut_parent, f, val);
}

/*
 * @brief Reads an unsigned 64-bit integer value at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_val Output unsigned 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_get_uint64_by_path(const void *obj,
                                             const struct cdd_type_info *ti,
                                             const char *path,
                                             uint64_t *out_val) {
  const struct cdd_field_info *f = NULL;
  const void *parent_obj = NULL;
  const void *field_ptr = NULL;
  cdd_c_error_t rc;
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_resolve_path(obj, ti, path, &f, &parent_obj, &field_ptr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = f->elem_kind;
    elem_f.size = f->elem_size;
    elem_f.c_type = f->c_type;
    return cdd_reflect_get_uint64(field_ptr, &elem_f, out_val);
  }
  return cdd_reflect_get_uint64(parent_obj, f, out_val);
}

/*
 * @brief Writes an unsigned 64-bit integer value at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val Unsigned 64-bit integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_set_uint64_by_path(void *obj,
                                             const struct cdd_type_info *ti,
                                             const char *path, uint64_t val) {
  const struct cdd_field_info *f = NULL;
  void *mut_parent = NULL;
  void *mut_field = NULL;
  cdd_c_error_t rc =
      resolve_path_mut(obj, ti, path, &f, &mut_parent, &mut_field);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = f->elem_kind;
    elem_f.size = f->elem_size;
    elem_f.c_type = f->c_type;
    return cdd_reflect_set_uint64(mut_field, &elem_f, val);
  }
  return cdd_reflect_set_uint64(mut_parent, f, val);
}

/*
 * @brief Reads a boolean value (`0` or `1`) at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_val Output boolean value (`0` or `1`).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_get_bool_by_path(const void *obj,
                                           const struct cdd_type_info *ti,
                                           const char *path, int *out_val) {
  const struct cdd_field_info *f = NULL;
  const void *parent_obj = NULL;
  const void *field_ptr = NULL;
  cdd_c_error_t rc;
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_resolve_path(obj, ti, path, &f, &parent_obj, &field_ptr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    if (f->elem_kind != CDD_FIELD_KIND_BOOLEAN)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = CDD_FIELD_KIND_BOOLEAN;
    elem_f.size = f->elem_size;
    return cdd_reflect_get_bool(field_ptr, &elem_f, out_val);
  }
  return cdd_reflect_get_bool(parent_obj, f, out_val);
}

/*
 * @brief Writes a boolean value (`0` or `1`) at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val Boolean value to store (`0` or non-zero).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_set_bool_by_path(void *obj,
                                           const struct cdd_type_info *ti,
                                           const char *path, int val) {
  const struct cdd_field_info *f = NULL;
  void *mut_parent = NULL;
  void *mut_field = NULL;
  cdd_c_error_t rc =
      resolve_path_mut(obj, ti, path, &f, &mut_parent, &mut_field);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    if (f->elem_kind != CDD_FIELD_KIND_BOOLEAN)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = CDD_FIELD_KIND_BOOLEAN;
    elem_f.size = f->elem_size;
    return cdd_reflect_set_bool(mut_field, &elem_f, val);
  }
  return cdd_reflect_set_bool(mut_parent, f, val);
}

/*
 * @brief Reads a floating-point value (`double`) at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_val Output `double` value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_get_double_by_path(const void *obj,
                                             const struct cdd_type_info *ti,
                                             const char *path,
                                             double *out_val) {
  const struct cdd_field_info *f = NULL;
  const void *parent_obj = NULL;
  const void *field_ptr = NULL;
  cdd_c_error_t rc;
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_resolve_path(obj, ti, path, &f, &parent_obj, &field_ptr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    if (f->elem_kind != CDD_FIELD_KIND_NUMBER)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = CDD_FIELD_KIND_NUMBER;
    elem_f.size = f->elem_size;
    return cdd_reflect_get_double(field_ptr, &elem_f, out_val);
  }
  return cdd_reflect_get_double(parent_obj, f, out_val);
}

/*
 * @brief Writes a floating-point value (`double`) at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val Floating-point value to store.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_set_double_by_path(void *obj,
                                             const struct cdd_type_info *ti,
                                             const char *path, double val) {
  const struct cdd_field_info *f = NULL;
  void *mut_parent = NULL;
  void *mut_field = NULL;
  cdd_c_error_t rc =
      resolve_path_mut(obj, ti, path, &f, &mut_parent, &mut_field);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    if (f->elem_kind != CDD_FIELD_KIND_NUMBER)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = CDD_FIELD_KIND_NUMBER;
    elem_f.size = f->elem_size;
    return cdd_reflect_set_double(mut_field, &elem_f, val);
  }
  return cdd_reflect_set_double(mut_parent, f, val);
}

/*
 * @brief Reads a string pointer at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_str Output string pointer.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_get_string_by_path(const void *obj,
                                             const struct cdd_type_info *ti,
                                             const char *path,
                                             const char **out_str) {
  const struct cdd_field_info *f = NULL;
  const void *parent_obj = NULL;
  const void *field_ptr = NULL;
  cdd_c_error_t rc;
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_resolve_path(obj, ti, path, &f, &parent_obj, &field_ptr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    if (f->elem_kind != CDD_FIELD_KIND_STRING)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = CDD_FIELD_KIND_STRING;
    elem_f.size = f->elem_size;
    elem_f.is_pointer = (f->elem_size == sizeof(char *)) ? 1 : 0;
    return cdd_reflect_get_string(field_ptr, &elem_f, out_str);
  }
  return cdd_reflect_get_string(parent_obj, f, out_str);
}

/*
 * @brief Writes a string value at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val NUL-terminated string to store (or NULL to clear).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_set_string_by_path(void *obj,
                                             const struct cdd_type_info *ti,
                                             const char *path,
                                             const char *val) {
  const struct cdd_field_info *f = NULL;
  void *mut_parent = NULL;
  void *mut_field = NULL;
  cdd_c_error_t rc =
      resolve_path_mut(obj, ti, path, &f, &mut_parent, &mut_field);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    struct cdd_field_info elem_f;
    if (f->elem_kind != CDD_FIELD_KIND_STRING)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    memset(&elem_f, 0, sizeof(elem_f));
    elem_f.kind = CDD_FIELD_KIND_STRING;
    elem_f.size = f->elem_size;
    elem_f.is_pointer = (f->elem_size == sizeof(char *)) ? 1 : 0;
    return cdd_reflect_set_string(mut_field, &elem_f, val);
  }
  return cdd_reflect_set_string(mut_parent, f, val);
}

/*
 * @brief Invokes a reflected function descriptor using its type-erased
 * `invoke` trampoline.
 *
 * @param[in] fi Runtime function descriptor.
 * @param[in] args Array of pointers to argument values (may be NULL if
 * `fi->n_params == 0`).
 * @param[out] ret_out Optional pointer to receive the return value.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `fi->invoke` is
 * NULL, or error code returned by the trampoline.
 */
cdd_c_error_t cdd_reflect_invoke_func(const struct cdd_func_info *fi,
                                      const void *const *args, void *ret_out) {
  if (!fi)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (fi->n_params > 0 && !args)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!fi->invoke)
    return CDD_C_ERROR_NOT_FOUND;
  return fi->invoke(args, ret_out);
}

/*
 * @brief Looks up a registered function by `func_name` and invokes its
 * type-erased trampoline.
 *
 * @param[in] func_name Registered function identifier.
 * @param[in] args Array of pointers to argument values.
 * @param[out] ret_out Optional pointer to receive the return value.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if not registered or
 * no trampoline, or error code returned by the trampoline.
 */
cdd_c_error_t cdd_reflect_invoke_by_name(const char *func_name,
                                         const void *const *args,
                                         void *ret_out) {
  const struct cdd_func_info *fi = NULL;
  cdd_c_error_t rc;
  if (!func_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_func_by_name(func_name, &fi);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_reflect_invoke_func(fi, args, ret_out);
}
