/**
 * @file struct_reflect_json.c
 * @brief Runtime reflection JSON serialization, deserialization, and RFC 7396
 * JSON Merge Patch.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/emit/struct_internal.h"
#include "c_cdd/json_dom.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "win_compat_sym.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static cdd_c_error_t
serialize_struct_to_json_obj(const void *obj, const struct cdd_type_info *ti,
                             JSON_Object *jobj);

/**
 * @brief Converts a single reflected field `f` on `obj` into a Parson
 * `JSON_Value *` (or flattens its properties onto `parent_jobj` when
 * `f->flatten != 0`).
 *
 * @param[in] obj Struct instance.
 * @param[in] f Field descriptor.
 * @param[in,out] parent_jobj Target JSON object.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t serialize_field_to_json(const void *obj,
                                             const struct cdd_field_info *f,
                                             JSON_Object *parent_jobj) {
  const char *key = f->wire_name ? f->wire_name : f->name;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (f->skip || f->skip_serializing)
    return CDD_C_SUCCESS;

  if (f->is_pointer && f->kind != CDD_FIELD_KIND_ARRAY) {
    const void *p = *(const void *const *)((
        const void *)((const unsigned char *)obj + f->offset));
    if (!p) {
      if (f->is_nullable) {
        rc = cdd_json_object_set_null(parent_jobj, key);
      }
      return rc;
    }
  }

  if (f->kind == CDD_FIELD_KIND_INTEGER) {
    size_t width = 0;
    int is_unsigned = 0;
    rc = cdd_reflect_resolve_field_scalar_width(f, &width, &is_unsigned);
    if (width == 8 && is_unsigned) {
      uint64_t uv = 0;
      rc = cdd_reflect_get_uint64(obj, f, &uv);
      rc = cdd_json_object_set_number(parent_jobj, key, (double)uv);
    } else {
      int64_t v = 0;
      rc = cdd_reflect_get_int64(obj, f, &v);
      rc = cdd_json_object_set_number(parent_jobj, key, (double)v);
    }
    return rc;
  } else if (f->kind == CDD_FIELD_KIND_NUMBER) {
    double d = 0.0;
    rc = cdd_reflect_get_double(obj, f, &d);
    rc = cdd_json_object_set_number(parent_jobj, key, d);
    return rc;
  } else if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
    int b = 0;
    rc = cdd_reflect_get_bool(obj, f, &b);
    rc = cdd_json_object_set_boolean(parent_jobj, key, b);
    return rc;
  } else if (f->kind == CDD_FIELD_KIND_STRING) {
    const char *s = NULL;
    rc = cdd_reflect_get_string(obj, f, &s);
    rc = cdd_json_object_set_string(parent_jobj, key, s);
    return rc;
  } else if (f->kind == CDD_FIELD_KIND_ENUM) {
    int64_t ev = 0;
    const struct cdd_enum_info *ei = NULL;
    rc = cdd_reflect_get_int64(obj, f, &ev);
    if (cdd_reflect_enum_by_name(f->ref_name, &ei) == CDD_C_SUCCESS) {
      const struct cdd_enum_variant_info *var = NULL;
      if (cdd_reflect_find_enum_by_value(ei, (int)ev, &var) == CDD_C_SUCCESS) {
        rc = cdd_json_object_set_string(
            parent_jobj, key, var->wire_name ? var->wire_name : var->name);
        return rc;
      }
    }
    rc = cdd_json_object_set_number(parent_jobj, key, (double)ev);
    return rc;
  } else if (f->kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *child_ti = NULL;
    const void *child_obj =
        f->is_pointer
            ? *(const void *const *)((const void *)((const unsigned char *)obj +
                                                    f->offset))
            : (const void *)((const unsigned char *)obj + f->offset);
    rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (f->flatten) {
      return serialize_struct_to_json_obj(child_obj, child_ti, parent_jobj);
    } else {
      JSON_Value *sub_val = NULL;
      JSON_Object *sub_obj = NULL;
      rc = cdd_json_value_init_object(&sub_val);
      rc = cdd_json_value_get_object(sub_val, &sub_obj);
      rc = serialize_struct_to_json_obj(child_obj, child_ti, sub_obj);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(sub_val);
        return rc;
      }
      return cdd_json_object_set_value(parent_jobj, key, sub_val);
    }
  } else if (f->kind == CDD_FIELD_KIND_ARRAY) {
    size_t alen = 0;
    size_t k;
    JSON_Value *arr_val = NULL;
    JSON_Array *jarr = NULL;
    if (f->is_pointer) {
      const void *arr_p = *(const void *const *)((
          const void *)((const unsigned char *)obj + f->offset));
      if (!arr_p)
        return CDD_C_SUCCESS;
    }
    rc = cdd_reflect_array_len(obj, f, &alen);
    rc = cdd_json_value_init_array(&arr_val);
    rc = cdd_json_value_get_array(arr_val, &jarr);
    for (k = 0; k < alen; ++k) {
      const void *elem_ptr = NULL;
      rc = cdd_reflect_array_at(obj, f, k, &elem_ptr);
      if (f->elem_kind == CDD_FIELD_KIND_INTEGER ||
          f->elem_kind == CDD_FIELD_KIND_ENUM) {
        struct cdd_field_info ef;
        int64_t ev = 0;
        memset(&ef, 0, sizeof(ef));
        ef.kind = f->elem_kind;
        ef.size = f->elem_size;
        rc = cdd_reflect_get_int64(elem_ptr, &ef, &ev);
        if (f->elem_kind == CDD_FIELD_KIND_ENUM) {
          const struct cdd_enum_info *ei = NULL;
          if (cdd_reflect_enum_by_name(f->ref_name, &ei) == CDD_C_SUCCESS) {
            const struct cdd_enum_variant_info *var = NULL;
            if (cdd_reflect_find_enum_by_value(ei, (int)ev, &var) ==
                CDD_C_SUCCESS) {
              const char *wn = var->wire_name ? var->wire_name : var->name;
              rc = cdd_json_array_append_string(jarr, wn);
              continue;
            }
          }
        }
        rc = cdd_json_array_append_number(jarr, (double)ev);
      } else if (f->elem_kind == CDD_FIELD_KIND_NUMBER) {
        struct cdd_field_info ef;
        double dv = 0.0;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_NUMBER;
        ef.size = f->elem_size;
        rc = cdd_reflect_get_double(elem_ptr, &ef, &dv);
        rc = cdd_json_array_append_number(jarr, dv);
      } else if (f->elem_kind == CDD_FIELD_KIND_BOOLEAN) {
        struct cdd_field_info ef;
        int bv = 0;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_BOOLEAN;
        ef.size = f->elem_size;
        rc = cdd_reflect_get_bool(elem_ptr, &ef, &bv);
        rc = cdd_json_array_append_boolean(jarr, bv);
      } else if (f->elem_kind == CDD_FIELD_KIND_STRING) {
        const char *se = *(const char *const *)elem_ptr;
        if (se) {
          rc = cdd_json_array_append_string(jarr, se);
        } else {
          rc = cdd_json_array_append_null(jarr);
        }
      } else {
        const struct cdd_type_info *elem_ti = NULL;
        const void *elem_obj;
        JSON_Value *ev_obj = NULL;
        JSON_Object *ev_jobj = NULL;
        rc = cdd_reflect_type_by_name(f->ref_name, &elem_ti);
        if (rc != CDD_C_SUCCESS) {
          g_cdd_last_alloc_err = cdd_json_value_free(arr_val);
          return rc;
        }
        elem_obj = (f->elem_size == sizeof(void *))
                       ? *(const void *const *)elem_ptr
                       : elem_ptr;
        if (!elem_obj) {
          rc = cdd_json_array_append_null(jarr);
          continue;
        }
        rc = cdd_json_value_init_object(&ev_obj);
        rc = cdd_json_value_get_object(ev_obj, &ev_jobj);
        rc = serialize_struct_to_json_obj(elem_obj, elem_ti, ev_jobj);
        if (rc != CDD_C_SUCCESS) {
          g_cdd_last_alloc_err = cdd_json_value_free(ev_obj);
          g_cdd_last_alloc_err = cdd_json_value_free(arr_val);
          return rc;
        }
        rc = cdd_json_array_append_value(jarr, ev_obj);
      }
    }
    rc = cdd_json_object_set_value(parent_jobj, key, arr_val);
  }
  return rc;
}

/**
 * @brief Populates a Parson `JSON_Object` from a struct instance `obj` and its
 * runtime `cdd_type_info` table.
 *
 * @param[in] obj Struct instance.
 * @param[in] ti Runtime type descriptor.
 * @param[in,out] jobj Parson JSON object to populate.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
serialize_struct_to_json_obj(const void *obj, const struct cdd_type_info *ti,
                             JSON_Object *jobj) {
  size_t i;
  cdd_c_error_t rc;
  if (ti->is_union) {
    int active_tag = *(const int *)((const void *)((const unsigned char *)obj +
                                                   ti->tag_offset));
    if (active_tag > 0 && (size_t)active_tag <= ti->n_fields) {
      return serialize_field_to_json(obj, &ti->fields[(size_t)(active_tag - 1)],
                                     jobj);
    }
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < ti->n_fields; ++i) {
    rc = serialize_field_to_json(obj, &ti->fields[i], jobj);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Serializes a reflected struct instance `obj` into a newly allocated
 * JSON string driven purely by its runtime `cdd_type_info` table.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[out] out_json Output pointer receiving the heap-allocated JSON string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on invalid arguments.
 */
cdd_c_error_t cdd_reflect_to_json(const void *obj,
                                  const struct cdd_type_info *ti,
                                  char **out_json) {
  JSON_Value *root = NULL;
  JSON_Object *root_obj = NULL;
  char *parson_str = NULL;
  char *dup_str;
  size_t slen;
  cdd_c_error_t rc;

  if (!obj || !ti || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_json = NULL;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&root);
  g_cdd_last_alloc_err = cdd_json_value_get_object(root, &root_obj);
  rc = serialize_struct_to_json_obj(obj, ti, root_obj);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = cdd_json_value_free(root);
    return rc;
  }

  g_cdd_last_alloc_err = cdd_json_serialize_to_string(root, &parson_str);
  g_cdd_last_alloc_err = cdd_json_value_free(root);
  slen = strlen(parson_str);
  dup_str = (char *)C_CDD_MALLOC(slen + 1);
  if (!dup_str) {
    g_cdd_last_alloc_err = cdd_json_free_serialized_string(parson_str);
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(dup_str, parson_str, slen + 1);
  g_cdd_last_alloc_err = cdd_json_free_serialized_string(parson_str);
  *out_json = dup_str;
  return CDD_C_SUCCESS;
}

/*
 * @brief Populates an existing `JSON_Object` in-place from a struct instance
 * `obj` driven purely by its runtime `cdd_type_info` table.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in,out] jobj Target JSON object (as `void *` for header decoupling).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_to_json_object(const void *obj,
                                         const struct cdd_type_info *ti,
                                         void *jobj) {
  if (!obj || !ti || !jobj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return serialize_struct_to_json_obj(obj, ti, (JSON_Object *)jobj);
}

/**
 * @brief Finds the JSON value corresponding to field `f` in `jobj` (checking
 * `de_wire_name`, `wire_name`, `name`, and `aliases`).
 *
 * @param[in] jobj Parson JSON object.
 * @param[in] f Field descriptor.
 * @param[out] out_jval Matching `JSON_Value *`, or NULL if absent.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t find_field_json_value(const JSON_Object *jobj,
                                           const struct cdd_field_info *f,
                                           JSON_Value **out_jval) {
  size_t k;
  JSON_Value *v = NULL;
  if (f->de_wire_name)
    g_cdd_last_alloc_err = cdd_json_object_get_value(jobj, f->de_wire_name, &v);
  if (!v && f->wire_name)
    g_cdd_last_alloc_err = cdd_json_object_get_value(jobj, f->wire_name, &v);
  if (!v)
    g_cdd_last_alloc_err = cdd_json_object_get_value(jobj, f->name, &v);
  if (!v && f->aliases) {
    for (k = 0; k < f->n_aliases && !v; ++k) {
      g_cdd_last_alloc_err = cdd_json_object_get_value(jobj, f->aliases[k], &v);
    }
  }
  *out_jval = v;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
deserialize_struct_from_json_obj(const JSON_Object *jobj,
                                 const struct cdd_type_info *ti, void *obj);

/**
 * @brief Populates a single reflected field `f` on `obj` from `jobj`.
 *
 * @param[in] jobj Source JSON object.
 * @param[in] f Field descriptor.
 * @param[in,out] obj Target struct instance.
 * @return CDD_C_SUCCESS on success, or error code on type mismatch/OOM.
 */
static cdd_c_error_t deserialize_field_from_json(const JSON_Object *jobj,
                                                 const struct cdd_field_info *f,
                                                 void *obj) {
  JSON_Value *jval = NULL;
  JSON_Value_Type jtype;
  cdd_c_error_t rc;

  if (f->skip || f->skip_deserializing)
    return CDD_C_SUCCESS;

  if (f->kind == CDD_FIELD_KIND_OBJECT && f->flatten) {
    const struct cdd_type_info *child_ti = NULL;
    void *child_obj;
    rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (f->is_pointer) {
      void **pptr = (void **)((void *)((unsigned char *)obj + f->offset));
      if (!*pptr) {
        rc = cdd_reflect_struct_default(child_ti, pptr);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      child_obj = *pptr;
    } else {
      child_obj = (void *)((unsigned char *)obj + f->offset);
    }
    return deserialize_struct_from_json_obj(jobj, child_ti, child_obj);
  }

  rc = find_field_json_value(jobj, f, &jval);
  if (!jval) {
    if (f->required && !f->default_val)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return rc;
  }

  jtype = json_value_get_type(jval);
  if (jtype == JSONNull) {
    if (f->is_pointer && (f->is_nullable || !f->required)) {
      return cdd_reflect_cleanup_single_field(obj, f);
    }
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (f->is_pointer && f->kind == CDD_FIELD_KIND_INTEGER) {
    void **pptr = (void **)((void *)((unsigned char *)obj + f->offset));
    if (!*pptr) {
      size_t width = 0;
      int is_u = 0;
      rc = cdd_reflect_resolve_field_scalar_width(f, &width, &is_u);
      *pptr = C_CDD_CALLOC(1, width);
      if (!*pptr)
        return CDD_C_ERROR_MEMORY;
    }
  }

  if (f->kind == CDD_FIELD_KIND_INTEGER) {
    size_t width = 0;
    int is_unsigned = 0;
    double dnum = 0.0;
    if (jtype != JSONNumber)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    g_cdd_last_alloc_err = cdd_json_value_get_number(jval, &dnum);
    rc = cdd_reflect_resolve_field_scalar_width(f, &width, &is_unsigned);
    if (width == 8 && is_unsigned) {
      if (dnum < 0.0)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      return cdd_reflect_set_uint64(obj, f, (uint64_t)dnum);
    }
    return cdd_reflect_set_int64(obj, f, (int64_t)dnum);
  }
  if (f->kind == CDD_FIELD_KIND_NUMBER) {
    double dnum = 0.0;
    if (jtype != JSONNumber)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    g_cdd_last_alloc_err = cdd_json_value_get_number(jval, &dnum);
    return cdd_reflect_set_double(obj, f, dnum);
  }
  if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
    int bval = 0;
    if (jtype != JSONBoolean)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    g_cdd_last_alloc_err = cdd_json_value_get_boolean(jval, &bval);
    return cdd_reflect_set_bool(obj, f, bval);
  }
  if (f->kind == CDD_FIELD_KIND_STRING) {
    const char *sval = NULL;
    if (jtype != JSONString)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    g_cdd_last_alloc_err = cdd_json_value_get_string(jval, &sval);
    return cdd_reflect_set_string(obj, f, sval);
  }
  if (f->kind == CDD_FIELD_KIND_ENUM) {
    if (jtype == JSONString) {
      const char *es = NULL;
      const struct cdd_enum_info *ei = NULL;
      const struct cdd_enum_variant_info *var = NULL;
      g_cdd_last_alloc_err = cdd_json_value_get_string(jval, &es);
      rc = cdd_reflect_enum_by_name(f->ref_name, &ei);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (cdd_reflect_find_enum_variant(ei, es, &var) == CDD_C_SUCCESS) {
        return cdd_reflect_set_int64(obj, f, (int64_t)var->value);
      }
      if (ei->other_idx >= 0) {
        int ov = ei->variants[ei->other_idx].value;
        return cdd_reflect_set_int64(obj, f, (int64_t)ov);
      }
      return CDD_C_ERROR_INVALID_ARGUMENT;
    } else if (jtype == JSONNumber) {
      double dnum = 0.0;
      g_cdd_last_alloc_err = cdd_json_value_get_number(jval, &dnum);
      return cdd_reflect_set_int64(obj, f, (int64_t)dnum);
    }
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (f->kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *child_ti = NULL;
    void *child_obj;
    JSON_Object *sub_jobj = NULL;
    if (jtype != JSONObject)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (f->is_pointer) {
      void **pptr = (void **)((void *)((unsigned char *)obj + f->offset));
      if (!*pptr) {
        rc = cdd_reflect_struct_default(child_ti, pptr);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      child_obj = *pptr;
    } else {
      child_obj = (void *)((unsigned char *)obj + f->offset);
    }
    g_cdd_last_alloc_err = cdd_json_value_get_object(jval, &sub_jobj);
    return deserialize_struct_from_json_obj(sub_jobj, child_ti, child_obj);
  }
  {
    JSON_Array *jarr = NULL;
    size_t count = 0;
    size_t k;
    if (jtype != JSONArray)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    g_cdd_last_alloc_err = cdd_json_value_get_array(jval, &jarr);
    g_cdd_last_alloc_err = cdd_json_array_get_count(jarr, &count);
    if (f->fixed_array_len > 0) {
      if (count > f->fixed_array_len)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    } else {
      rc = cdd_reflect_array_resize(obj, f, count);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    for (k = 0; k < count; ++k) {
      JSON_Value *ev = NULL;
      JSON_Value_Type et;
      void *elem_ptr = NULL;
      g_cdd_last_alloc_err = cdd_json_array_get_value(jarr, k, &ev);
      et = json_value_get_type(ev);
      rc = cdd_reflect_array_at_mut(obj, f, k, &elem_ptr);
      if (f->elem_kind == CDD_FIELD_KIND_INTEGER) {
        struct cdd_field_info ef;
        double dnum = 0.0;
        if (et != JSONNumber)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        g_cdd_last_alloc_err = cdd_json_value_get_number(ev, &dnum);
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_INTEGER;
        ef.size = f->elem_size;
        rc = cdd_reflect_set_int64(elem_ptr, &ef, (int64_t)dnum);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (f->elem_kind == CDD_FIELD_KIND_NUMBER) {
        struct cdd_field_info ef;
        double dnum = 0.0;
        if (et != JSONNumber)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        g_cdd_last_alloc_err = cdd_json_value_get_number(ev, &dnum);
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_NUMBER;
        ef.size = f->elem_size;
        rc = cdd_reflect_set_double(elem_ptr, &ef, dnum);
      } else if (f->elem_kind == CDD_FIELD_KIND_BOOLEAN) {
        struct cdd_field_info ef;
        int bval = 0;
        if (et != JSONBoolean)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        g_cdd_last_alloc_err = cdd_json_value_get_boolean(ev, &bval);
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_BOOLEAN;
        ef.size = f->elem_size;
        rc = cdd_reflect_set_bool(elem_ptr, &ef, bval);
      } else if (f->elem_kind == CDD_FIELD_KIND_ENUM) {
        struct cdd_field_info ef;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_ENUM;
        ef.size = f->elem_size;
        if (et == JSONString) {
          const struct cdd_enum_info *ei = NULL;
          const struct cdd_enum_variant_info *var = NULL;
          const char *es = NULL;
          g_cdd_last_alloc_err = cdd_json_value_get_string(ev, &es);
          rc = cdd_reflect_enum_by_name(f->ref_name, &ei);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (cdd_reflect_find_enum_variant(ei, es, &var) != CDD_C_SUCCESS)
            return CDD_C_ERROR_INVALID_ARGUMENT;
          rc = cdd_reflect_set_int64(elem_ptr, &ef, (int64_t)var->value);
        } else if (et == JSONNumber) {
          double dnum = 0.0;
          g_cdd_last_alloc_err = cdd_json_value_get_number(ev, &dnum);
          rc = cdd_reflect_set_int64(elem_ptr, &ef, (int64_t)dnum);
        } else {
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
      } else if (f->elem_kind == CDD_FIELD_KIND_STRING) {
        char **sp = (char **)elem_ptr;
        if (et == JSONNull) {
          if (*sp) {
            C_CDD_FREE(*sp);
            *sp = NULL;
          }
        } else if (et == JSONString) {
          const char *s = NULL;
          char *dup;
          g_cdd_last_alloc_err = cdd_json_value_get_string(ev, &s);
          dup = C_CDD_STRDUP(s);
          if (!dup)
            return CDD_C_ERROR_MEMORY;
          if (*sp)
            C_CDD_FREE(*sp);
          *sp = dup;
        } else {
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
      } else {
        const struct cdd_type_info *elem_ti = NULL;
        JSON_Object *ev_jobj = NULL;
        rc = cdd_reflect_type_by_name(f->ref_name, &elem_ti);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (et == JSONNull && f->elem_size == sizeof(void *)) {
          void **ep_ptr = (void **)elem_ptr;
          if (*ep_ptr) {
            rc = cdd_reflect_struct_cleanup(*ep_ptr, elem_ti);
            C_CDD_FREE(*ep_ptr);
            *ep_ptr = NULL;
          }
          continue;
        }
        if (et != JSONObject)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        g_cdd_last_alloc_err = cdd_json_value_get_object(ev, &ev_jobj);
        if (f->elem_size == sizeof(void *)) {
          void **ep_ptr = (void **)elem_ptr;
          if (!*ep_ptr) {
            rc = cdd_reflect_struct_default(elem_ti, ep_ptr);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          rc = deserialize_struct_from_json_obj(ev_jobj, elem_ti, *ep_ptr);
        } else {
          rc = deserialize_struct_from_json_obj(ev_jobj, elem_ti, elem_ptr);
        }
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }
  return rc;
}

/**
 * @brief Populates an existing struct instance `obj` from a Parson
 * `JSON_Object` using `ti` and validates constraints.
 *
 * @param[in] jobj Source JSON object.
 * @param[in] ti Runtime type descriptor.
 * @param[in,out] obj Target struct instance.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
deserialize_struct_from_json_obj(const JSON_Object *jobj,
                                 const struct cdd_type_info *ti, void *obj) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (ti->is_union) {
    int *tag_ptr = (int *)((void *)((unsigned char *)obj + ti->tag_offset));
    for (i = 0; i < ti->n_fields; ++i) {
      JSON_Value *jv = NULL;
      rc = find_field_json_value(jobj, &ti->fields[i], &jv);
      if (jv) {
        *tag_ptr = (int)(i + 1);
        rc = deserialize_field_from_json(jobj, &ti->fields[i], obj);
        break;
      }
    }
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_reflect_validate(obj, ti);
  }
  for (i = 0; i < ti->n_fields; ++i) {
    rc = deserialize_field_from_json(jobj, &ti->fields[i], obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return cdd_reflect_validate(obj, ti);
}

cdd_c_error_t cdd_reflect_from_json_object(const void *jobj,
                                           const struct cdd_type_info *ti,
                                           void *obj) {
  if (!jobj || !ti || !obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return deserialize_struct_from_json_obj((const JSON_Object *)jobj, ti, obj);
}

/*
 * @brief Deserializes a JSON string into a newly allocated struct instance
 * `*out_obj` driven purely by its runtime `cdd_type_info` table.
 *
 * @param[in] json_str NUL-terminated JSON string.
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_obj Output pointer receiving the allocated struct instance.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on invalid JSON or constraint failure.
 */
cdd_c_error_t cdd_reflect_from_json(const char *json_str,
                                    const struct cdd_type_info *ti,
                                    void **out_obj) {
  JSON_Value *root = NULL;
  void *obj = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t rc_c = CDD_C_SUCCESS;

  if (!json_str || !ti || !out_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = NULL;

  if (cdd_json_parse_string(json_str, &root) != CDD_C_SUCCESS ||
      json_value_get_type(root) != JSONObject) {
    if (root)
      g_cdd_last_alloc_err = cdd_json_value_free(root);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_reflect_struct_default(ti, &obj);
  if (rc == CDD_C_SUCCESS) {
    JSON_Object *jobj = NULL;
    g_cdd_last_alloc_err = cdd_json_value_get_object(root, &jobj);
    rc = deserialize_struct_from_json_obj(jobj, ti, obj);
    if (rc == CDD_C_SUCCESS) {
      *out_obj = obj;
    } else {
      rc_c = cdd_reflect_struct_cleanup(obj, ti);
      C_CDD_FREE(obj);
    }
  }
  g_cdd_last_alloc_err = cdd_json_value_free(root);
  return (cdd_c_error_t)((int)rc | (int)rc_c);
}

/**
 * @brief Recursively computes an RFC 7396 JSON Merge Patch object representing
 * differences from `source` to `target` for a reflected struct type `ti`.
 *
 * @param[in] source Baseline struct instance.
 * @param[in] target Modified struct instance.
 * @param[in] ti Runtime type descriptor.
 * @param[in,out] patch_jobj Output Parson JSON object receiving diff keys.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t build_merge_patch_obj(const void *source,
                                           const void *target,
                                           const struct cdd_type_info *ti,
                                           JSON_Object *patch_jobj) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  for (i = 0; i < ti->n_fields && rc == CDD_C_SUCCESS; ++i) {
    const struct cdd_field_info *f = &ti->fields[i];
    const char *key = f->wire_name ? f->wire_name : f->name;
    const void *sa = (const unsigned char *)source + f->offset;
    const void *ta = (const unsigned char *)target + f->offset;
    int is_eq = 1;
    if (!f->skip && !f->skip_serializing)
      rc = cdd_reflect_eq_single_field(source, target, f, &is_eq);
    if (rc != CDD_C_SUCCESS || is_eq)
      continue;

    if (f->is_pointer && f->kind != CDD_FIELD_KIND_ARRAY &&
        !*(const void *const *)ta) {
      rc = cdd_json_object_set_null(patch_jobj, key);
    } else if (f->kind == CDD_FIELD_KIND_OBJECT && !f->is_pointer) {
      const struct cdd_type_info *child_ti = NULL;
      JSON_Value *sub_patch = NULL;
      JSON_Object *sub_jobj = NULL;
      rc = cdd_json_value_init_object(&sub_patch);
      rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
      g_cdd_last_alloc_err = cdd_json_value_get_object(sub_patch, &sub_jobj);
      rc = build_merge_patch_obj(sa, ta, child_ti, sub_jobj);
      rc = cdd_json_object_set_value(patch_jobj, key, sub_patch);
    } else {
      rc = serialize_field_to_json(target, f, patch_jobj);
    }
  }
  return rc;
}

/*
 * @brief Computes an RFC 7396 JSON Merge Patch string representing the changes
 * from `source` to `target` for a reflected struct type `ti`.
 *
 * @param[in] source Baseline struct instance.
 * @param[in] target Modified struct instance.
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_patch_json Output pointer receiving the JSON Merge Patch
 * string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_merge_patch(const void *source, const void *target,
                                      const struct cdd_type_info *ti,
                                      char **out_patch_json) {
  JSON_Value *root = NULL;
  JSON_Object *root_obj = NULL;
  char *parson_str = NULL;
  char *dup_str;
  size_t slen;
  cdd_c_error_t rc;

  if (!source || !target || !ti || !out_patch_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_patch_json = NULL;

  rc = cdd_json_value_init_object(&root);
  g_cdd_last_alloc_err = cdd_json_value_get_object(root, &root_obj);
  rc = build_merge_patch_obj(source, target, ti, root_obj);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = cdd_json_value_free(root);
    return rc;
  }

  rc = cdd_json_serialize_to_string(root, &parson_str);
  g_cdd_last_alloc_err = cdd_json_value_free(root);
  slen = strlen(parson_str);
  dup_str = (char *)C_CDD_MALLOC(slen + 1);
  if (!dup_str) {
    g_cdd_last_alloc_err = cdd_json_free_serialized_string(parson_str);
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(dup_str, parson_str, slen + 1);
  g_cdd_last_alloc_err = cdd_json_free_serialized_string(parson_str);
  *out_patch_json = dup_str;
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursively applies an RFC 7396 JSON Merge Patch object onto `target`.
 *
 * @param[in,out] target Mutable struct instance.
 * @param[in] ti Runtime type descriptor.
 * @param[in] patch_jobj Parson JSON object containing patch keys.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t apply_merge_patch_obj(void *target,
                                           const struct cdd_type_info *ti,
                                           const JSON_Object *patch_jobj) {
  size_t count = 0;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_get_count(patch_jobj, &count);
  for (i = 0; i < count && rc == CDD_C_SUCCESS; ++i) {
    const char *key = NULL;
    JSON_Value *val = NULL;
    const struct cdd_field_info *f = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_name(patch_jobj, i, &key);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(patch_jobj, i, &val);
    if (cdd_reflect_find_field(ti, key, &f) != CDD_C_SUCCESS)
      continue;
    if (f->kind == CDD_FIELD_KIND_OBJECT &&
        json_value_get_type(val) == JSONObject) {
      const struct cdd_type_info *child_ti = NULL;
      void *child_obj = (void *)((unsigned char *)target + f->offset);
      JSON_Object *sub_jobj = NULL;
      g_cdd_last_alloc_err = cdd_json_value_get_object(val, &sub_jobj);
      rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
      if (rc == CDD_C_SUCCESS && f->is_pointer) {
        void **pptr = (void **)child_obj;
        if (!*pptr)
          rc = cdd_reflect_struct_default(child_ti, pptr);
        child_obj = *pptr;
      }
      if (rc == CDD_C_SUCCESS)
        rc = apply_merge_patch_obj(child_obj, child_ti, sub_jobj);
    } else {
      rc = deserialize_field_from_json(patch_jobj, f, target);
    }
  }
  return rc;
}

/*
 * @brief Applies an RFC 7396 JSON Merge Patch string onto an existing mutable
 * struct instance `target` using its runtime `cdd_type_info` table.
 *
 * @param[in,out] target Mutable struct instance to patch in-place.
 * @param[in] ti Runtime type descriptor.
 * @param[in] patch_json NUL-terminated JSON Merge Patch string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_apply_merge_patch(void *target,
                                            const struct cdd_type_info *ti,
                                            const char *patch_json) {
  JSON_Value *root = NULL;
  JSON_Object *root_obj = NULL;
  cdd_c_error_t rc;
  if (!target || !ti || !patch_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (cdd_json_parse_string(patch_json, &root) != CDD_C_SUCCESS ||
      json_value_get_type(root) != JSONObject) {
    if (root)
      g_cdd_last_alloc_err = cdd_json_value_free(root);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  g_cdd_last_alloc_err = cdd_json_value_get_object(root, &root_obj);
  rc = apply_merge_patch_obj(target, ti, root_obj);
  g_cdd_last_alloc_err = cdd_json_value_free(root);
  return rc;
}

/**
 * @brief Populates a Parson `JSON_Object` with JSON Schema / OpenAPI 3.2.0
 * property constraints for a single reflected field `f`.
 *
 * @param[in] f Reflected field descriptor.
 * @param[in,out] prop_obj Target JSON Schema property object.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t build_field_schema_obj(const struct cdd_field_info *f,
                                            JSON_Object *prop_obj) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (f->kind == CDD_FIELD_KIND_INTEGER) {
    rc = cdd_json_object_set_string(prop_obj, "type", "integer");
  } else if (f->kind == CDD_FIELD_KIND_NUMBER) {
    rc = cdd_json_object_set_string(prop_obj, "type", "number");
  } else if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
    rc = cdd_json_object_set_string(prop_obj, "type", "boolean");
  } else if (f->kind == CDD_FIELD_KIND_STRING) {
    rc = cdd_json_object_set_string(prop_obj, "type", "string");
    if (f->has_min_len)
      rc =
          cdd_json_object_set_number(prop_obj, "minLength", (double)f->min_len);
    if (f->has_max_len)
      rc =
          cdd_json_object_set_number(prop_obj, "maxLength", (double)f->max_len);
    if (f->pattern)
      rc = cdd_json_object_set_string(prop_obj, "pattern", f->pattern);
  } else if (f->kind == CDD_FIELD_KIND_ENUM) {
    const struct cdd_enum_info *ei = NULL;
    rc = cdd_json_object_set_string(prop_obj, "type", "string");
    if (cdd_reflect_enum_by_name(f->ref_name, &ei) == CDD_C_SUCCESS) {
      JSON_Value *ev_arr_val = NULL;
      JSON_Array *ev_arr = NULL;
      size_t k;
      g_cdd_last_alloc_err = cdd_json_value_init_array(&ev_arr_val);
      g_cdd_last_alloc_err = cdd_json_value_get_array(ev_arr_val, &ev_arr);
      for (k = 0; k < ei->n_variants; ++k) {
        if (!ei->variants[k].skip) {
          rc = cdd_json_array_append_string(ev_arr, ei->variants[k].name);
        }
      }
      rc = cdd_json_object_set_value(prop_obj, "enum", ev_arr_val);
    }
  } else if (f->kind == CDD_FIELD_KIND_OBJECT) {
    char ref_buf[192];
    CDD_SNPRINTF(ref_buf, sizeof(ref_buf), "#/components/schemas/%s",
                 f->ref_name);
    rc = cdd_json_object_set_string(prop_obj, "$ref", ref_buf);
  } else {
    JSON_Value *items_val = NULL;
    JSON_Object *items_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&items_val);
    g_cdd_last_alloc_err = cdd_json_value_get_object(items_val, &items_obj);
    rc = cdd_json_object_set_string(prop_obj, "type", "array");
    if (f->has_min_len)
      rc = cdd_json_object_set_number(prop_obj, "minItems", (double)f->min_len);
    if (f->has_max_len)
      rc = cdd_json_object_set_number(prop_obj, "maxItems", (double)f->max_len);
    if (f->unique_items)
      rc = cdd_json_object_set_boolean(prop_obj, "uniqueItems", 1);
    if (f->elem_kind == CDD_FIELD_KIND_INTEGER) {
      rc = cdd_json_object_set_string(items_obj, "type", "integer");
    } else if (f->elem_kind == CDD_FIELD_KIND_NUMBER) {
      rc = cdd_json_object_set_string(items_obj, "type", "number");
    } else if (f->elem_kind == CDD_FIELD_KIND_BOOLEAN) {
      rc = cdd_json_object_set_string(items_obj, "type", "boolean");
    } else if (f->elem_kind == CDD_FIELD_KIND_STRING) {
      rc = cdd_json_object_set_string(items_obj, "type", "string");
    } else if (f->elem_kind == CDD_FIELD_KIND_OBJECT) {
      char ref_buf[192];
      CDD_SNPRINTF(ref_buf, sizeof(ref_buf), "#/components/schemas/%s",
                   f->ref_name);
      rc = cdd_json_object_set_string(items_obj, "$ref", ref_buf);
    }
    rc = cdd_json_object_set_value(prop_obj, "items", items_val);
  }

  if (f->has_min) {
    rc = cdd_json_object_set_number(
        prop_obj, f->exclusive_min ? "exclusiveMinimum" : "minimum",
        f->min_val);
  }
  if (f->has_max) {
    rc = cdd_json_object_set_number(
        prop_obj, f->exclusive_max ? "exclusiveMaximum" : "maximum",
        f->max_val);
  }
  if (f->format)
    rc = cdd_json_object_set_string(prop_obj, "format", f->format);
  if (f->doc)
    rc = cdd_json_object_set_string(prop_obj, "description", f->doc);
  if (f->deprecated)
    rc = cdd_json_object_set_boolean(prop_obj, "deprecated", 1);
  if (f->read_only)
    rc = cdd_json_object_set_boolean(prop_obj, "readOnly", 1);
  if (f->write_only)
    rc = cdd_json_object_set_boolean(prop_obj, "writeOnly", 1);
  return rc;
}

/*
 * @brief Generates an OpenAPI 3.2.0 / JSON Schema Draft 2020-12 object string
 * directly from a runtime `cdd_type_info` descriptor table.
 *
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_schema_json Output pointer receiving the heap-allocated JSON
 * Schema string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_to_json_schema(const struct cdd_type_info *ti,
                                         char **out_schema_json) {
  JSON_Value *root = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Value *props_val = NULL;
  JSON_Object *props_obj = NULL;
  JSON_Value *req_val = NULL;
  JSON_Array *req_arr = NULL;
  char *parson_str = NULL;
  char *dup_str;
  size_t slen;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!ti || !out_schema_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_schema_json = NULL;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&root);
  g_cdd_last_alloc_err = cdd_json_value_get_object(root, &root_obj);
  rc = cdd_json_object_set_string(root_obj, "title", ti->name);
  rc = cdd_json_object_set_string(root_obj, "type", "object");

  g_cdd_last_alloc_err = cdd_json_value_init_object(&props_val);
  g_cdd_last_alloc_err = cdd_json_value_get_object(props_val, &props_obj);

  for (i = 0; i < ti->n_fields; ++i) {
    const struct cdd_field_info *f = &ti->fields[i];
    const char *key = f->wire_name ? f->wire_name : f->name;
    JSON_Value *f_val = NULL;
    JSON_Object *f_obj = NULL;
    if (f->skip)
      continue;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&f_val);
    g_cdd_last_alloc_err = cdd_json_value_get_object(f_val, &f_obj);
    rc = build_field_schema_obj(f, f_obj);
    rc = cdd_json_object_set_value(props_obj, key, f_val);
    if (f->required) {
      if (!req_val) {
        g_cdd_last_alloc_err = cdd_json_value_init_array(&req_val);
        g_cdd_last_alloc_err = cdd_json_value_get_array(req_val, &req_arr);
      }
      rc = cdd_json_array_append_string(req_arr, key);
    }
  }

  rc = cdd_json_object_set_value(root_obj, "properties", props_val);
  if (req_val) {
    rc = cdd_json_object_set_value(root_obj, "required", req_val);
  }

  g_cdd_last_alloc_err = cdd_json_serialize_to_string(root, &parson_str);
  g_cdd_last_alloc_err = cdd_json_value_free(root);
  slen = strlen(parson_str);
  dup_str = (char *)C_CDD_MALLOC(slen + 1);
  if (!dup_str) {
    g_cdd_last_alloc_err = cdd_json_free_serialized_string(parson_str);
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(dup_str, parson_str, slen + 1);
  g_cdd_last_alloc_err = cdd_json_free_serialized_string(parson_str);
  *out_schema_json = dup_str;
  return rc;
}

/**
 * @brief Parses a single scalar string token `val_str` onto field `f` of `obj`.
 *
 * @param[in,out] obj Mutable struct instance.
 * @param[in] f Field descriptor.
 * @param[in] val_str NUL-terminated string representation of the field value.
 * @return CDD_C_SUCCESS on success, or error code on invalid value.
 */
static cdd_c_error_t parse_scalar_str_into_field(void *obj,
                                                 const struct cdd_field_info *f,
                                                 const char *val_str) {
  if (f->kind == CDD_FIELD_KIND_STRING) {
    return cdd_reflect_set_string(obj, f, val_str);
  }
  if (f->kind == CDD_FIELD_KIND_INTEGER) {
    char *endp = NULL;
    long v = strtol(val_str, &endp, 0);
    if (endp == val_str || *endp != '\0')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return cdd_reflect_set_int64(obj, f, (int64_t)v);
  }
  if (f->kind == CDD_FIELD_KIND_NUMBER) {
    char *endp = NULL;
    double d = strtod(val_str, &endp);
    if (endp == val_str || *endp != '\0')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return cdd_reflect_set_double(obj, f, d);
  }
  if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
    int b = (strcmp(val_str, "true") == 0 || strcmp(val_str, "1") == 0) ? 1 : 0;
    return cdd_reflect_set_bool(obj, f, b);
  }
  if (f->kind == CDD_FIELD_KIND_ENUM) {
    const struct cdd_enum_info *ei = NULL;
    const struct cdd_enum_variant_info *var = NULL;
    cdd_c_error_t rc = cdd_reflect_enum_by_name(f->ref_name, &ei);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_reflect_find_enum_variant(ei, val_str, &var);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_reflect_set_int64(obj, f, (int64_t)var->value);
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/*
 * @brief Serializes a reflected struct instance `obj` into an
 * `application/x-www-form-urlencoded` query string (`key=value&...`) driven
 * by its runtime `cdd_type_info` table.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[out] out_form Output pointer receiving the heap-allocated form string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
cdd_c_error_t cdd_reflect_to_form_urlencoded(const void *obj,
                                             const struct cdd_type_info *ti,
                                             char **out_form) {
  size_t cap = 64;
  size_t len = 0;
  char *buf;
  size_t i;

  if (!obj || !ti || !out_form)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_form = NULL;

  buf = (char *)C_CDD_MALLOC(cap);
  if (!buf)
    return CDD_C_ERROR_MEMORY;
  buf[0] = '\0';

  for (i = 0; i < ti->n_fields; ++i) {
    const struct cdd_field_info *f = &ti->fields[i];
    const char *key = f->wire_name ? f->wire_name : f->name;
    char val_buf[128];
    const char *val_str = NULL;
    const void *fp = (const void *)((const unsigned char *)obj + f->offset);
    size_t pair_len;

    if (!f->skip && !f->skip_serializing) {
      if (f->kind == CDD_FIELD_KIND_INTEGER || f->kind == CDD_FIELD_KIND_ENUM ||
          f->kind == CDD_FIELD_KIND_BOOLEAN) {
        int64_t v = 0;
        if (cdd_reflect_get_int64(obj, f, &v) == CDD_C_SUCCESS) {
          CDD_SNPRINTF(val_buf, sizeof(val_buf), "%ld", (long)v);
          val_str = (f->kind == CDD_FIELD_KIND_BOOLEAN) ? (v ? "true" : "false")
                                                        : val_buf;
        }
      } else if (f->kind == CDD_FIELD_KIND_NUMBER) {
        CDD_SNPRINTF(val_buf, sizeof(val_buf), "%g", *(const double *)fp);
        val_str = val_buf;
      } else if (f->kind == CDD_FIELD_KIND_STRING) {
        val_str = f->is_pointer ? *(const char *const *)fp : (const char *)fp;
      }
    }
    if (!val_str)
      continue;

    pair_len = strlen(key) + 1 + strlen(val_str) + (len > 0 ? 1 : 0);
    if (len + pair_len + 1 > cap) {
      size_t new_cap = cap * 2 + pair_len + 1;
      char *new_buf = (char *)C_CDD_REALLOC(buf, new_cap);
      if (!new_buf) {
        C_CDD_FREE(buf);
        return CDD_C_ERROR_MEMORY;
      }
      buf = new_buf;
      cap = new_cap;
    }
    if (len > 0)
      buf[len++] = '&';
    memcpy(buf + len, key, strlen(key));
    len += strlen(key);
    buf[len++] = '=';
    memcpy(buf + len, val_str, strlen(val_str));
    len += strlen(val_str);
    buf[len] = '\0';
  }

  *out_form = buf;
  return CDD_C_SUCCESS;
}

/*
 * @brief Deserializes an `application/x-www-form-urlencoded` string
 * (`key=value&...`) into a newly allocated struct instance `*out_obj` driven by
 * `ti`.
 *
 * @param[in] form_str NUL-terminated form-urlencoded string.
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_obj Output pointer receiving the allocated struct instance.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_from_form_urlencoded(const char *form_str,
                                               const struct cdd_type_info *ti,
                                               void **out_obj) {
  void *obj = NULL;
  const char *p;
  cdd_c_error_t rc;
  cdd_c_error_t rc_clean;

  if (!form_str || !ti || !out_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = NULL;

  rc = cdd_reflect_struct_default(ti, &obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  p = form_str;
  while (*p != '\0') {
    char key_buf[128];
    char val_buf[256];
    size_t klen = 0, vlen = 0;
    const struct cdd_field_info *f = NULL;

    while (*p == '&')
      p++;
    if (*p == '\0')
      break;

    while (*p != '\0' && *p != '=' && *p != '&') {
      if (klen + 1 < sizeof(key_buf))
        key_buf[klen++] = *p;
      p++;
    }
    key_buf[klen] = '\0';
    if (*p == '=') {
      p++;
      while (*p != '\0' && *p != '&') {
        if (vlen + 1 < sizeof(val_buf))
          val_buf[vlen++] = (*p == '+') ? ' ' : *p;
        p++;
      }
    }
    val_buf[vlen] = '\0';

    if (cdd_reflect_find_field(ti, key_buf, &f) == CDD_C_SUCCESS && !f->skip &&
        !f->skip_deserializing) {
      rc = parse_scalar_str_into_field(obj, f, val_buf);
      if (rc != CDD_C_SUCCESS) {
        rc_clean = cdd_reflect_struct_cleanup(obj, ti);
        C_CDD_FREE(obj);
        return (cdd_c_error_t)((int)rc | (int)rc_clean);
      }
    }
  }

  *out_obj = obj;
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses CLI `argc`/`argv` flags (`--<field> <val>` or
 * `--<field>=<val>`) into a newly allocated struct instance `*out_obj` driven
 * by `ti`.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_obj Output pointer receiving the allocated struct instance.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_reflect_from_cli(int argc, const char *const *argv,
                                   const struct cdd_type_info *ti,
                                   void **out_obj) {
  void *obj = NULL;
  int i;
  cdd_c_error_t rc;
  cdd_c_error_t rc_clean;

  if (argc < 0 || (argc > 0 && !argv) || !ti || !out_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = NULL;

  rc = cdd_reflect_struct_default(ti, &obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < argc; ++i) {
    const char *arg = argv[i];
    const char *eq;
    char key_buf[128];
    const char *val_str = NULL;
    const struct cdd_field_info *f = NULL;

    if (!arg || strncmp(arg, "--", 2) != 0)
      continue;
    arg += 2;
    eq = strchr(arg, '=');
    if (eq) {
      size_t klen = (size_t)(eq - arg);
      if (klen < sizeof(key_buf)) {
        memcpy(key_buf, arg, klen);
        key_buf[klen] = '\0';
      }
      val_str = eq + 1;
    } else {
      CDD_STRCPY(key_buf, sizeof(key_buf), arg);
    }

    if (cdd_reflect_find_field(ti, key_buf, &f) != CDD_C_SUCCESS)
      continue;

    if (!val_str) {
      if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
        val_str = "true";
      } else if (i + 1 < argc) {
        val_str = argv[++i];
      } else {
        rc_clean = cdd_reflect_struct_cleanup(obj, ti);
        C_CDD_FREE(obj);
        return (cdd_c_error_t)((int)CDD_C_ERROR_INVALID_ARGUMENT |
                               (int)rc_clean);
      }
    }

    rc = parse_scalar_str_into_field(obj, f, val_str);
    if (rc != CDD_C_SUCCESS) {
      rc_clean = cdd_reflect_struct_cleanup(obj, ti);
      C_CDD_FREE(obj);
      return (cdd_c_error_t)((int)rc | (int)rc_clean);
    }
  }

  *out_obj = obj;
  return CDD_C_SUCCESS;
}

/*
 * @briefApplies an RFC 6902 JSON Patch array (`[{"op": "add"|"remove"|
 * "replace"|"move"|"copy"|"test", "path": "/...", "value": ...}]`) onto a
 * mutable reflected struct instance `target`.
 *
 * @param[in,out] target Mutable struct instance to patch.
 * @param[in] ti Runtime type descriptor for `target`.
 * @param[in] patch_json NUL-terminated RFC 6902 JSON Patch array string.
 * @return CDD_C_SUCCESS on success, or error code on invalid patch or failed
 * `test` assertion.
 */
cdd_c_error_t cdd_reflect_apply_json_patch(void *target,
                                           const struct cdd_type_info *ti,
                                           const char *patch_json) {
  JSON_Value *root = NULL;
  JSON_Array *ops = NULL;
  size_t count = 0;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!target || !ti || !patch_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (cdd_json_parse_string(patch_json, &root) != CDD_C_SUCCESS ||
      json_value_get_type(root) != JSONArray) {
    if (root)
      g_cdd_last_alloc_err = cdd_json_value_free(root);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  g_cdd_last_alloc_err = cdd_json_value_get_array(root, &ops);
  g_cdd_last_alloc_err = cdd_json_array_get_count(ops, &count);
  for (i = 0; i < count; ++i) {
    JSON_Object *op_obj = NULL;
    const char *op_name = NULL;
    const char *path = NULL;
    JSON_Value *val = NULL;
    const struct cdd_field_info *f = NULL;
    const void *parent_obj = NULL;
    const void *field_ptr = NULL;

    g_cdd_last_alloc_err = cdd_json_array_get_object(ops, i, &op_obj);
    g_cdd_last_alloc_err = cdd_json_object_get_string(op_obj, "op", &op_name);
    g_cdd_last_alloc_err = cdd_json_object_get_string(op_obj, "path", &path);
    g_cdd_last_alloc_err = cdd_json_object_get_value(op_obj, "value", &val);

    if (!op_name || !path) {
      g_cdd_last_alloc_err = cdd_json_value_free(root);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    rc =
        cdd_reflect_resolve_path(target, ti, path, &f, &parent_obj, &field_ptr);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(root);
      return rc;
    }

    if (strcmp(op_name, "add") == 0 || strcmp(op_name, "replace") == 0) {
      if (!val) {
        g_cdd_last_alloc_err = cdd_json_value_free(root);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      if (json_value_get_type(val) == JSONNumber) {
        double dnum = 0.0;
        g_cdd_last_alloc_err = cdd_json_value_get_number(val, &dnum);
        if (f->kind == CDD_FIELD_KIND_NUMBER ||
            f->elem_kind == CDD_FIELD_KIND_NUMBER) {
          rc = cdd_reflect_set_double_by_path(target, ti, path, dnum);
        } else {
          rc = cdd_reflect_set_int64_by_path(target, ti, path, (int64_t)dnum);
        }
      } else if (json_value_get_type(val) == JSONBoolean) {
        int bval = 0;
        g_cdd_last_alloc_err = cdd_json_value_get_boolean(val, &bval);
        rc = cdd_reflect_set_bool_by_path(target, ti, path, bval);
      } else if (json_value_get_type(val) == JSONString) {
        const char *sval = NULL;
        g_cdd_last_alloc_err = cdd_json_value_get_string(val, &sval);
        rc = cdd_reflect_set_string_by_path(target, ti, path, sval);
      } else {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
      }
    } else if (strcmp(op_name, "remove") == 0) {
      union {
        const void *c_ptr;
        void *m_ptr;
      } u_p;
      u_p.c_ptr = parent_obj;
      rc = cdd_reflect_cleanup_single_field(u_p.m_ptr, f);
    } else if (strcmp(op_name, "test") == 0) {
      if (!val) {
        g_cdd_last_alloc_err = cdd_json_value_free(root);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      if (json_value_get_type(val) == JSONNumber) {
        int64_t iv = 0;
        double dnum = 0.0;
        g_cdd_last_alloc_err = cdd_json_value_get_number(val, &dnum);
        rc = cdd_reflect_get_int64_by_path(target, ti, path, &iv);
        if (iv != (int64_t)dnum)
          rc = CDD_C_ERROR_INVALID_ARGUMENT;
      } else if (json_value_get_type(val) == JSONString) {
        const char *sv = NULL;
        rc = cdd_reflect_get_string_by_path(target, ti, path, &sv);
        if (!sv || strcmp(sv, val->string_val) != 0)
          rc = CDD_C_ERROR_INVALID_ARGUMENT;
      } else {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
      }
    } else if (strcmp(op_name, "copy") == 0 || strcmp(op_name, "move") == 0) {
      const char *from_path = NULL;
      const struct cdd_field_info *from_f = NULL;
      const void *from_parent = NULL;
      const void *from_ptr = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(op_obj, "from", &from_path);
      if (!from_path) {
        g_cdd_last_alloc_err = cdd_json_value_free(root);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      rc = cdd_reflect_resolve_path(target, ti, from_path, &from_f,
                                    &from_parent, &from_ptr);
      if (rc == CDD_C_SUCCESS) {
        if (from_f->kind == CDD_FIELD_KIND_STRING) {
          const char *sv = NULL;
          rc = cdd_reflect_get_string_by_path(target, ti, from_path, &sv);
          rc = cdd_reflect_set_string_by_path(target, ti, path, sv);
        } else {
          int64_t iv = 0;
          rc = cdd_reflect_get_int64_by_path(target, ti, from_path, &iv);
          rc = cdd_reflect_set_int64_by_path(target, ti, path, iv);
        }
        if (strcmp(op_name, "move") == 0) {
          union {
            const void *c_ptr;
            void *m_ptr;
          } u_fp;
          u_fp.c_ptr = from_parent;
          rc = cdd_reflect_cleanup_single_field(u_fp.m_ptr, from_f);
        }
      }
    } else {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
    }

    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(root);
      return rc;
    }
  }

  g_cdd_last_alloc_err = cdd_json_value_free(root);
  return rc;
}
