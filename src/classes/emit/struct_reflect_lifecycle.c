/**
 * @file struct_reflect_lifecycle.c
 * @brief Runtime reflection struct lifecycle operations (default, cleanup,
 * deepcopy, eq).
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

/**
 * @brief Initializes a pre-allocated struct buffer `obj` with field default
 * values from `ti`.
 *
 * @param[in,out] obj Zero-initialized struct memory of size `ti->size`.
 * @param[in] ti Runtime type descriptor.
 * @return CDD_C_SUCCESS on success, or error code on allocation failure.
 */
static cdd_c_error_t
init_struct_defaults_in_place(void *obj, const struct cdd_type_info *ti) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  for (i = 0; i < ti->n_fields; ++i) {
    const struct cdd_field_info *f = &ti->fields[i];
    if (!f->default_val || f->default_val[0] == '\0' ||
        (f->is_pointer && f->kind != CDD_FIELD_KIND_STRING)) {
      continue;
    }

    if (f->kind == CDD_FIELD_KIND_INTEGER) {
      unsigned int bw_val = 0U;
      rc = cdd_struct_eval_bitwidth_str(f->default_val, &bw_val);
      rc = cdd_reflect_set_int64(obj, f, (int64_t)bw_val);
    } else if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
      int b = (strcmp(f->default_val, "true") == 0) ? 1 : 0;
      rc = cdd_reflect_set_bool(obj, f, b);
    } else if (f->kind == CDD_FIELD_KIND_NUMBER) {
      double d = strtod(f->default_val, NULL);
      rc = cdd_reflect_set_double(obj, f, d);
    } else if (f->kind == CDD_FIELD_KIND_ENUM) {
      int ev_val = 0;
      int resolved = 0;
      if (f->ref_name && f->ref_name[0] != '\0') {
        const struct cdd_enum_info *ei = NULL;
        if (cdd_reflect_enum_by_name(f->ref_name, &ei) == CDD_C_SUCCESS) {
          const struct cdd_enum_variant_info *ev = NULL;
          if (cdd_reflect_find_enum_variant(ei, f->default_val, &ev) ==
              CDD_C_SUCCESS) {
            ev_val = ev->value;
            resolved = 1;
          }
        }
      }
      if (!resolved && (isdigit((unsigned char)f->default_val[0]) ||
                        f->default_val[0] == '-')) {
        ev_val = (int)strtol(f->default_val, NULL, 0);
      }
      rc = cdd_reflect_set_int64(obj, f, (int64_t)ev_val);
    } else if (f->kind == CDD_FIELD_KIND_STRING) {
      rc = cdd_reflect_set_string(obj, f, f->default_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return rc;
}

/*
 * @brief Allocates and initializes a struct instance with default values using
 * its runtime `cdd_type_info` table.
 *
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_obj Output pointer receiving the newly allocated struct.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on invalid arguments.
 */
cdd_c_error_t cdd_reflect_struct_default(const struct cdd_type_info *ti,
                                         void **out_obj) {
  void *obj;
  cdd_c_error_t rc;
  cdd_c_error_t rc_clean;
  if (!ti || !out_obj || ti->size == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = NULL;
  obj = C_CDD_CALLOC(1, ti->size);
  if (!obj)
    return CDD_C_ERROR_MEMORY;
  rc = init_struct_defaults_in_place(obj, ti);
  if (rc != CDD_C_SUCCESS) {
    rc_clean = cdd_reflect_struct_cleanup(obj, ti);
    C_CDD_FREE(obj);
    return rc;
  }
  *out_obj = obj;
  return rc_clean = CDD_C_SUCCESS;
}

/*
 * @brief Cleans up a single field on `obj` (used by
 * `cdd_reflect_struct_cleanup`).
 *
 * @param[in,out] obj Mutable struct instance.
 * @param[in] f Field descriptor to clean up.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t cdd_reflect_cleanup_single_field(void *obj,
                                               const struct cdd_field_info *f) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (f->bit_width > 0)
    return CDD_C_SUCCESS;

  if (f->kind == CDD_FIELD_KIND_STRING) {
    if (f->is_pointer) {
      char **pptr = (char **)((void *)((unsigned char *)obj + f->offset));
      if (*pptr) {
        C_CDD_FREE(*pptr);
        *pptr = NULL;
      }
    }
  } else if (f->kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *child_ti = NULL;
    if (f->ref_name && f->ref_name[0] != '\0') {
      rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
    }
    if (f->is_pointer) {
      void **pptr = (void **)((void *)((unsigned char *)obj + f->offset));
      if (*pptr) {
        if (child_ti) {
          rc = cdd_reflect_struct_cleanup(*pptr, child_ti);
        }
        C_CDD_FREE(*pptr);
        *pptr = NULL;
      }
    } else if (child_ti) {
      void *child_ptr = (void *)((unsigned char *)obj + f->offset);
      rc = cdd_reflect_struct_cleanup(child_ptr, child_ti);
    }
  } else if (f->kind == CDD_FIELD_KIND_ARRAY) {
    if (f->is_pointer) {
      rc = cdd_reflect_array_resize(obj, f, 0);
    } else if (f->fixed_array_len > 0) {
      void *base = (void *)((unsigned char *)obj + f->offset);
      rc = cdd_reflect_cleanup_shrunk_array_elements(base, f, 0,
                                                     f->fixed_array_len);
    }
  } else if (f->is_pointer) {
    void **pptr = (void **)((void *)((unsigned char *)obj + f->offset));
    if (*pptr) {
      C_CDD_FREE(*pptr);
      *pptr = NULL;
    }
  }
  return rc == CDD_C_ERROR_NOT_FOUND ? CDD_C_SUCCESS : rc;
}

/*
 * @brief Recursively frees all heap-allocated members (strings, dynamic arrays,
 * nested heap objects) owned by `obj` according to `ti` (does not free `obj`
 * itself).
 *
 * @param[in,out] obj Mutable pointer to struct instance to clean up.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * `ti`.
 */
cdd_c_error_t cdd_reflect_struct_cleanup(void *obj,
                                         const struct cdd_type_info *ti) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!ti)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!obj)
    return CDD_C_SUCCESS;
  if (ti->is_union) {
    int active_tag = *(const int *)((const void *)((const unsigned char *)obj +
                                                   ti->tag_offset));
    if (active_tag > 0 && (size_t)active_tag <= ti->n_fields) {
      return cdd_reflect_cleanup_single_field(
          obj, &ti->fields[(size_t)(active_tag - 1)]);
    }
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < ti->n_fields; ++i) {
    rc = cdd_reflect_cleanup_single_field(obj, &ti->fields[i]);
  }
  return rc;
}

static cdd_c_error_t deepcopy_struct_in_place(const void *src, void *dest,
                                              const struct cdd_type_info *ti);

/**
 * @brief Deep-copies a single field `f` from `src` onto `dest` (assuming `dest`
 * already has a shallow byte copy of `src` with heap pointers zeroed or
 * overwritten).
 *
 * @param[in] src Source struct instance.
 * @param[in,out] dest Destination struct instance.
 * @param[in] f Field descriptor.
 * @return CDD_C_SUCCESS on success, or error code on allocation failure.
 */
static cdd_c_error_t deepcopy_single_field(const void *src, void *dest,
                                           const struct cdd_field_info *f) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (f->bit_width > 0) {
    return CDD_C_SUCCESS;
  }

  if (f->kind == CDD_FIELD_KIND_STRING) {
    if (f->is_pointer) {
      const char *s = *(const char *const *)((
          const void *)((const unsigned char *)src + f->offset));
      char **dst_p = (char **)((void *)((unsigned char *)dest + f->offset));
      *dst_p = NULL;
      if (s) {
        return cdd_reflect_set_string(dest, f, s);
      }
    }
    return CDD_C_SUCCESS;
  }

  if (f->kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *child_ti = NULL;
    if (f->ref_name && f->ref_name[0] != '\0') {
      rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (f->is_pointer) {
      const void *src_child = *(const void *const *)((
          const void *)((const unsigned char *)src + f->offset));
      void **dst_p = (void **)((void *)((unsigned char *)dest + f->offset));
      *dst_p = NULL;
      if (src_child && child_ti) {
        return cdd_reflect_struct_deepcopy(src_child, dst_p, child_ti);
      }
    } else if (child_ti) {
      const void *src_child =
          (const void *)((const unsigned char *)src + f->offset);
      void *dst_child = (void *)((unsigned char *)dest + f->offset);
      return deepcopy_struct_in_place(src_child, dst_child, child_ti);
    }
    return CDD_C_SUCCESS;
  }

  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    size_t alen = 0;
    size_t k;
    rc = cdd_reflect_array_len(src, f, &alen);
    if (f->is_pointer) {
      void **dst_p = (void **)((void *)((unsigned char *)dest + f->offset));
      size_t *dst_len =
          (size_t *)((void *)((unsigned char *)dest + f->array_len_offset));
      *dst_p = NULL;
      *dst_len = 0;
      if (alen == 0)
        return rc;
      rc = cdd_reflect_array_resize(dest, f, alen);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (f->fixed_array_len > 0 &&
               (f->elem_kind == CDD_FIELD_KIND_STRING ||
                f->elem_kind == CDD_FIELD_KIND_OBJECT)) {
      memset((unsigned char *)dest + f->offset, 0, f->size);
    }
    for (k = 0; k < alen; ++k) {
      const void *src_elem = NULL;
      void *dst_elem = NULL;
      rc = cdd_reflect_array_at(src, f, k, &src_elem);
      rc = cdd_reflect_array_at_mut(dest, f, k, &dst_elem);
      if (f->elem_kind == CDD_FIELD_KIND_STRING) {
        const char *se = *(const char *const *)src_elem;
        char **de = (char **)dst_elem;
        *de = NULL;
        if (se) {
          size_t sl = strlen(se);
          *de = (char *)C_CDD_MALLOC(sl + 1);
          if (!*de)
            return CDD_C_ERROR_MEMORY;
          memcpy(*de, se, sl + 1);
        }
      } else if (f->elem_kind == CDD_FIELD_KIND_OBJECT) {
        const struct cdd_type_info *elem_ti = NULL;
        rc = cdd_reflect_type_by_name(f->ref_name, &elem_ti);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (f->elem_size == sizeof(void *)) {
          const void *so = *(const void *const *)src_elem;
          void **do_ptr = (void **)dst_elem;
          *do_ptr = NULL;
          if (so) {
            rc = cdd_reflect_struct_deepcopy(so, do_ptr, elem_ti);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        } else {
          rc = deepcopy_struct_in_place(src_elem, dst_elem, elem_ti);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      } else {
        memcpy(dst_elem, src_elem, f->elem_size);
      }
    }
    return CDD_C_SUCCESS;
  }

  if (f->is_pointer) {
    const void *src_p = *(const void *const *)((
        const void *)((const unsigned char *)src + f->offset));
    void **dst_p = (void **)((void *)((unsigned char *)dest + f->offset));
    size_t width = 0;
    int is_u = 0;
    *dst_p = NULL;
    if (src_p) {
      rc = cdd_reflect_resolve_field_scalar_width(f, &width, &is_u);
      *dst_p = C_CDD_MALLOC(width);
      if (!*dst_p)
        return CDD_C_ERROR_MEMORY;
      memcpy(*dst_p, src_p, width);
    }
  }
  return rc;
}

/**
 * @brief Deep-copies `src` into an already allocated buffer `dest` of size
 * `ti->size`.
 *
 * @param[in] src Source struct instance.
 * @param[in,out] dest Pre-allocated destination buffer.
 * @param[in] ti Runtime type descriptor.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t deepcopy_struct_in_place(const void *src, void *dest,
                                              const struct cdd_type_info *ti) {
  size_t i, j;
  cdd_c_error_t rc;
  memcpy(dest, src, ti->size);
  if (ti->is_union) {
    int active_tag = *(const int *)((const void *)((const unsigned char *)src +
                                                   ti->tag_offset));
    if (active_tag > 0 && (size_t)active_tag <= ti->n_fields) {
      const struct cdd_field_info *f = &ti->fields[(size_t)(active_tag - 1)];
      if (f->is_pointer) {
        *(void **)((void *)((unsigned char *)dest + f->offset)) = NULL;
      }
      return deepcopy_single_field(src, dest, f);
    }
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < ti->n_fields; ++i) {
    rc = deepcopy_single_field(src, dest, &ti->fields[i]);
    if (rc != CDD_C_SUCCESS) {
      /* Zero remaining un-copied heap pointers in `dest` before cleanup */
      for (j = i; j < ti->n_fields; ++j) {
        const struct cdd_field_info *fj = &ti->fields[j];
        if (fj->is_pointer) {
          *(void **)((void *)((unsigned char *)dest + fj->offset)) = NULL;
          if (fj->kind == CDD_FIELD_KIND_ARRAY) {
            *(size_t *)((void *)((unsigned char *)dest +
                                 fj->array_len_offset)) = 0;
          }
        } else if (fj->kind == CDD_FIELD_KIND_ARRAY &&
                   fj->fixed_array_len > 0 &&
                   (fj->elem_kind == CDD_FIELD_KIND_STRING ||
                    fj->elem_kind == CDD_FIELD_KIND_OBJECT)) {
          memset((unsigned char *)dest + fj->offset, 0, fj->size);
        } else if (fj->kind == CDD_FIELD_KIND_OBJECT) {
          memset((unsigned char *)dest + fj->offset, 0, fj->size);
        }
      }
      return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Performs a deep recursive copy of `src` into a newly heap-allocated
 * struct `*out_dest` driven by `ti`.
 *
 * @param[in] src Source struct instance pointer.
 * @param[out] out_dest Output pointer receiving the cloned struct instance.
 * @param[in] ti Runtime type descriptor for `src`.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_struct_deepcopy(const void *src, void **out_dest,
                                          const struct cdd_type_info *ti) {
  void *dest;
  cdd_c_error_t rc;
  cdd_c_error_t rc_clean;
  if (!src || !out_dest || !ti || ti->size == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_dest = NULL;
  dest = C_CDD_CALLOC(1, ti->size);
  if (!dest)
    return CDD_C_ERROR_MEMORY;
  rc = deepcopy_struct_in_place(src, dest, ti);
  if (rc != CDD_C_SUCCESS) {
    rc_clean = cdd_reflect_struct_cleanup(dest, ti);
    C_CDD_FREE(dest);
    return rc;
  }
  *out_dest = dest;
  return rc_clean = CDD_C_SUCCESS;
}

/*
 * @brief Compares a single field `f` across `a` and `b` for deep equality.
 *
 * @param[in] a First struct instance.
 * @param[in] b Second struct instance.
 * @param[in] f Field descriptor.
 * @param[out] out_eq Set to `1` if equal, `0` otherwise.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_eq_single_field(const void *a, const void *b,
                                          const struct cdd_field_info *f,
                                          int *out_eq) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  *out_eq = 0;

  if (f->kind == CDD_FIELD_KIND_INTEGER || f->kind == CDD_FIELD_KIND_BOOLEAN ||
      f->kind == CDD_FIELD_KIND_ENUM || f->kind == CDD_FIELD_KIND_NUMBER) {
    if (f->is_pointer) {
      const void *pa = *(const void *const *)((
          const void *)((const unsigned char *)a + f->offset));
      const void *pb = *(const void *const *)((
          const void *)((const unsigned char *)b + f->offset));
      if (pa == pb || !pa || !pb) {
        *out_eq = (pa == pb) ? 1 : 0;
        return CDD_C_SUCCESS;
      }
    }
    if (f->kind == CDD_FIELD_KIND_NUMBER) {
      double da = 0.0, db = 0.0;
      rc = cdd_reflect_get_double(a, f, &da);
      rc = cdd_reflect_get_double(b, f, &db);
      *out_eq = (da == db) ? 1 : 0;
    } else {
      size_t width = 0;
      int is_unsigned = 0;
      rc = cdd_reflect_resolve_field_scalar_width(f, &width, &is_unsigned);
      if (width == 8 && is_unsigned) {
        uint64_t ua = 0, ub = 0;
        rc = cdd_reflect_get_uint64(a, f, &ua);
        rc = cdd_reflect_get_uint64(b, f, &ub);
        *out_eq = (ua == ub) ? 1 : 0;
      } else {
        int64_t va = 0, vb = 0;
        rc = cdd_reflect_get_int64(a, f, &va);
        rc = cdd_reflect_get_int64(b, f, &vb);
        *out_eq = (va == vb) ? 1 : 0;
      }
    }
    return rc;
  }

  if (f->kind == CDD_FIELD_KIND_STRING) {
    const char *sa = NULL;
    const char *sb = NULL;
    rc = cdd_reflect_get_string(a, f, &sa);
    rc = cdd_reflect_get_string(b, f, &sb);
    *out_eq = (sa == sb || (sa && sb && strcmp(sa, sb) == 0)) ? 1 : 0;
    return rc;
  }

  if (f->kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *child_ti = NULL;
    const void *oa =
        f->is_pointer ? *(const void *const *)((
                            const void *)((const unsigned char *)a + f->offset))
                      : (const void *)((const unsigned char *)a + f->offset);
    const void *ob =
        f->is_pointer ? *(const void *const *)((
                            const void *)((const unsigned char *)b + f->offset))
                      : (const void *)((const unsigned char *)b + f->offset);
    if (oa == ob || !oa || !ob) {
      *out_eq = (oa == ob) ? 1 : 0;
      return CDD_C_SUCCESS;
    }
    rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_reflect_struct_eq(oa, ob, child_ti, out_eq);
  }

  *out_eq = 1;
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    size_t la = 0, lb = 0;
    size_t k;
    rc = cdd_reflect_array_len(a, f, &la);
    rc = cdd_reflect_array_len(b, f, &lb);
    if (la != lb) {
      *out_eq = 0;
      return rc;
    }
    for (k = 0; k < la && *out_eq; ++k) {
      const void *ea = NULL;
      const void *eb = NULL;
      rc = cdd_reflect_array_at(a, f, k, &ea);
      rc = cdd_reflect_array_at(b, f, k, &eb);
      if (f->elem_kind == CDD_FIELD_KIND_STRING) {
        const char *sa = *(const char *const *)ea;
        const char *sb = *(const char *const *)eb;
        if (sa != sb && (!sa || !sb || strcmp(sa, sb) != 0))
          *out_eq = 0;
      } else if (f->elem_kind == CDD_FIELD_KIND_OBJECT) {
        const struct cdd_type_info *elem_ti = NULL;
        int is_p = (f->elem_size == sizeof(void *));
        const void *oa = is_p ? *(const void *const *)ea : ea;
        const void *ob = is_p ? *(const void *const *)eb : eb;
        rc = cdd_reflect_type_by_name(f->ref_name, &elem_ti);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = cdd_reflect_struct_eq(oa, ob, elem_ti, out_eq);
      } else {
        *out_eq = (memcmp(ea, eb, f->elem_size) == 0) ? 1 : 0;
      }
    }
  }
  return rc;
}

/*
 * @brief Performs a deep structural equality comparison between two struct
 * instances `a` and `b` driven by `ti`.
 *
 * @param[in] a First struct instance pointer (may be NULL).
 * @param[in] b Second struct instance pointer (may be NULL).
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_eq Output flag set to `1` if equal, `0` otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * `ti` or `out_eq`.
 */
cdd_c_error_t cdd_reflect_struct_eq(const void *a, const void *b,
                                    const struct cdd_type_info *ti,
                                    int *out_eq) {
  size_t i;
  cdd_c_error_t rc;
  if (!ti || !out_eq)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (a == b || !a || !b) {
    *out_eq = (a == b) ? 1 : 0;
    return CDD_C_SUCCESS;
  }
  if (ti->is_union) {
    const unsigned char *ba = (const unsigned char *)a + ti->tag_offset;
    const unsigned char *bb = (const unsigned char *)b + ti->tag_offset;
    int ta = *(const int *)((const void *)ba);
    int tb = *(const int *)((const void *)bb);
    if (ta != tb) {
      *out_eq = 0;
      return CDD_C_SUCCESS;
    }
    if (ta > 0 && (size_t)ta <= ti->n_fields) {
      return cdd_reflect_eq_single_field(a, b, &ti->fields[(size_t)(ta - 1)],
                                         out_eq);
    }
    *out_eq = 1;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < ti->n_fields; ++i) {
    int f_eq = 0;
    rc = cdd_reflect_eq_single_field(a, b, &ti->fields[i], &f_eq);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!f_eq) {
      *out_eq = 0;
      return CDD_C_SUCCESS;
    }
  }
  *out_eq = 1;
  return CDD_C_SUCCESS;
}
