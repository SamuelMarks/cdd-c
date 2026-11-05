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
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "win_compat_sym.h"
#include <ctype.h>
#include <parson.h>
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
  const char *key =
      (f->wire_name && f->wire_name[0] != '\0') ? f->wire_name : f->name;
  cdd_c_error_t rc;
  if (f->skip || f->skip_serializing)
    return CDD_C_SUCCESS;

  if (f->is_pointer && f->bit_width == 0 && f->kind != CDD_FIELD_KIND_ARRAY) {
    const void *p = *(const void *const *)((
        const void *)((const unsigned char *)obj + f->offset));
    if (!p) {
      if (f->is_nullable) {
        json_object_set_null(parent_jobj, key);
      }
      return CDD_C_SUCCESS;
    }
  }

  if (f->kind == CDD_FIELD_KIND_INTEGER) {
    int64_t v = 0;
    cdd_reflect_get_int64(obj, f, &v);
    json_object_set_number(parent_jobj, key, (double)v);
  } else if (f->kind == CDD_FIELD_KIND_NUMBER) {
    double d = 0.0;
    cdd_reflect_get_double(obj, f, &d);
    json_object_set_number(parent_jobj, key, d);
  } else if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
    int b = 0;
    cdd_reflect_get_bool(obj, f, &b);
    json_object_set_boolean(parent_jobj, key, b);
  } else if (f->kind == CDD_FIELD_KIND_STRING) {
    const char *s = NULL;
    cdd_reflect_get_string(obj, f, &s);
    if (s) {
      json_object_set_string(parent_jobj, key, s);
    }
  } else if (f->kind == CDD_FIELD_KIND_ENUM) {
    int64_t ev = 0;
    const struct cdd_enum_info *ei = NULL;
    cdd_reflect_get_int64(obj, f, &ev);
    if (f->ref_name && f->ref_name[0] != '\0' &&
        cdd_reflect_enum_by_name(f->ref_name, &ei) == CDD_C_SUCCESS && ei) {
      const struct cdd_enum_variant_info *var = NULL;
      if (cdd_reflect_find_enum_by_value(ei, (int)ev, &var) == CDD_C_SUCCESS &&
          var) {
        const char *wn = (var->wire_name && var->wire_name[0] != '\0')
                             ? var->wire_name
                             : var->name;
        json_object_set_string(parent_jobj, key, wn);
        return CDD_C_SUCCESS;
      }
    }
    json_object_set_number(parent_jobj, key, (double)ev);
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
      JSON_Value *sub_val = json_value_init_object();
      rc = serialize_struct_to_json_obj(child_obj, child_ti,
                                        json_value_get_object(sub_val));
      if (rc != CDD_C_SUCCESS) {
        json_value_free(sub_val);
        return rc;
      }
      json_object_set_value(parent_jobj, key, sub_val);
    }
  } else if (f->kind == CDD_FIELD_KIND_ARRAY) {
    size_t alen = 0;
    size_t k;
    JSON_Value *arr_val;
    JSON_Array *jarr;
    if (f->is_pointer && f->fixed_array_len == 0) {
      const void *arr_p = *(const void *const *)((
          const void *)((const unsigned char *)obj + f->offset));
      if (!arr_p)
        return CDD_C_SUCCESS;
    }
    cdd_reflect_array_len(obj, f, &alen);
    arr_val = json_value_init_array();
    jarr = json_value_get_array(arr_val);
    for (k = 0; k < alen; ++k) {
      const void *elem_ptr = NULL;
      cdd_reflect_array_at(obj, f, k, &elem_ptr);
      if (f->elem_kind == CDD_FIELD_KIND_INTEGER ||
          f->elem_kind == CDD_FIELD_KIND_ENUM) {
        struct cdd_field_info ef;
        int64_t ev = 0;
        memset(&ef, 0, sizeof(ef));
        ef.kind = f->elem_kind;
        ef.size = f->elem_size;
        cdd_reflect_get_int64(elem_ptr, &ef, &ev);
        if (f->elem_kind == CDD_FIELD_KIND_ENUM && f->ref_name &&
            f->ref_name[0] != '\0') {
          const struct cdd_enum_info *ei = NULL;
          if (cdd_reflect_enum_by_name(f->ref_name, &ei) == CDD_C_SUCCESS &&
              ei) {
            const struct cdd_enum_variant_info *var = NULL;
            if (cdd_reflect_find_enum_by_value(ei, (int)ev, &var) ==
                    CDD_C_SUCCESS &&
                var) {
              const char *wn = (var->wire_name && var->wire_name[0] != '\0')
                                   ? var->wire_name
                                   : var->name;
              json_array_append_string(jarr, wn);
              continue;
            }
          }
        }
        json_array_append_number(jarr, (double)ev);
      } else if (f->elem_kind == CDD_FIELD_KIND_NUMBER) {
        struct cdd_field_info ef;
        double dv = 0.0;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_NUMBER;
        ef.size = f->elem_size;
        cdd_reflect_get_double(elem_ptr, &ef, &dv);
        json_array_append_number(jarr, dv);
      } else if (f->elem_kind == CDD_FIELD_KIND_BOOLEAN) {
        struct cdd_field_info ef;
        int bv = 0;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_BOOLEAN;
        ef.size = f->elem_size;
        cdd_reflect_get_bool(elem_ptr, &ef, &bv);
        json_array_append_boolean(jarr, bv);
      } else if (f->elem_kind == CDD_FIELD_KIND_STRING &&
                 f->elem_size == sizeof(char *)) {
        const char *se = *(const char *const *)elem_ptr;
        if (se) {
          json_array_append_string(jarr, se);
        } else {
          json_array_append_null(jarr);
        }
      } else if (f->elem_kind == CDD_FIELD_KIND_OBJECT) {
        const struct cdd_type_info *elem_ti = NULL;
        const void *elem_obj;
        JSON_Value *ev_obj;
        rc = cdd_reflect_type_by_name(f->ref_name, &elem_ti);
        if (rc != CDD_C_SUCCESS) {
          json_value_free(arr_val);
          return rc;
        }
        elem_obj =
            (f->elem_size == sizeof(void *) && elem_ti->size != sizeof(void *))
                ? *(const void *const *)elem_ptr
                : elem_ptr;
        if (!elem_obj) {
          json_array_append_null(jarr);
          continue;
        }
        ev_obj = json_value_init_object();
        rc = serialize_struct_to_json_obj(elem_obj, elem_ti,
                                          json_value_get_object(ev_obj));
        if (rc != CDD_C_SUCCESS) {
          json_value_free(ev_obj);
          json_value_free(arr_val);
          return rc;
        }
        json_array_append_value(jarr, ev_obj);
      }
    }
    json_object_set_value(parent_jobj, key, arr_val);
  }
  return CDD_C_SUCCESS;
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
  JSON_Value *root;
  char *parson_str;
  char *dup_str;
  size_t slen;
  cdd_c_error_t rc;

  if (!obj || !ti || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_json = NULL;

  root = json_value_init_object();
  rc = serialize_struct_to_json_obj(obj, ti, json_value_get_object(root));
  if (rc != CDD_C_SUCCESS) {
    json_value_free(root);
    return rc;
  }

  parson_str = json_serialize_to_string(root);
  json_value_free(root);
  slen = strlen(parson_str);
  dup_str = (char *)C_CDD_MALLOC(slen + 1);
  if (!dup_str) {
    json_free_serialized_string(parson_str);
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(dup_str, parson_str, slen + 1);
  json_free_serialized_string(parson_str);
  *out_json = dup_str;
  return CDD_C_SUCCESS;
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
  if (f->de_wire_name && f->de_wire_name[0] != '\0')
    v = json_object_get_value(jobj, f->de_wire_name);
  if (!v && f->wire_name && f->wire_name[0] != '\0')
    v = json_object_get_value(jobj, f->wire_name);
  if (!v && f->name && f->name[0] != '\0')
    v = json_object_get_value(jobj, f->name);
  if (!v && f->aliases) {
    for (k = 0; k < f->n_aliases; ++k) {
      if (f->aliases[k]) {
        v = json_object_get_value(jobj, f->aliases[k]);
        if (v)
          break;
      }
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

  find_field_json_value(jobj, f, &jval);
  if (!jval) {
    if (f->required && (!f->default_val || f->default_val[0] == '\0'))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return CDD_C_SUCCESS;
  }

  jtype = json_value_get_type(jval);
  if (jtype == JSONNull) {
    if (f->is_pointer && (f->is_nullable || !f->required)) {
      return cdd_reflect_cleanup_single_field(obj, f);
    }
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (f->is_pointer && f->bit_width == 0 &&
      (f->kind == CDD_FIELD_KIND_INTEGER || f->kind == CDD_FIELD_KIND_NUMBER ||
       f->kind == CDD_FIELD_KIND_BOOLEAN || f->kind == CDD_FIELD_KIND_ENUM)) {
    void **pptr = (void **)((void *)((unsigned char *)obj + f->offset));
    if (!*pptr) {
      size_t width = 0;
      int is_u = 0;
      cdd_reflect_resolve_field_scalar_width(f, &width, &is_u);
      *pptr = C_CDD_CALLOC(1, width);
      if (!*pptr)
        return CDD_C_ERROR_MEMORY;
    }
  }

  if (f->kind == CDD_FIELD_KIND_INTEGER) {
    if (jtype != JSONNumber)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return cdd_reflect_set_int64(obj, f, (int64_t)json_value_get_number(jval));
  }
  if (f->kind == CDD_FIELD_KIND_NUMBER) {
    if (jtype != JSONNumber)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return cdd_reflect_set_double(obj, f, json_value_get_number(jval));
  }
  if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
    if (jtype != JSONBoolean)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return cdd_reflect_set_bool(obj, f, json_value_get_boolean(jval));
  }
  if (f->kind == CDD_FIELD_KIND_STRING) {
    if (jtype != JSONString)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return cdd_reflect_set_string(obj, f, json_value_get_string(jval));
  }
  if (f->kind == CDD_FIELD_KIND_ENUM) {
    if (jtype == JSONString) {
      const char *es = json_value_get_string(jval);
      const struct cdd_enum_info *ei = NULL;
      const struct cdd_enum_variant_info *var = NULL;
      rc = cdd_reflect_enum_by_name(f->ref_name, &ei);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (cdd_reflect_find_enum_variant(ei, es, &var) == CDD_C_SUCCESS && var) {
        return cdd_reflect_set_int64(obj, f, (int64_t)var->value);
      }
      if (ei->other_idx >= 0 && (size_t)ei->other_idx < ei->n_variants) {
        return cdd_reflect_set_int64(
            obj, f, (int64_t)ei->variants[ei->other_idx].value);
      }
      return CDD_C_ERROR_INVALID_ARGUMENT;
    } else if (jtype == JSONNumber) {
      return cdd_reflect_set_int64(obj, f,
                                   (int64_t)json_value_get_number(jval));
    }
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (f->kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *child_ti = NULL;
    void *child_obj;
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
    return deserialize_struct_from_json_obj(json_value_get_object(jval),
                                            child_ti, child_obj);
  }
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
    JSON_Array *jarr;
    size_t count;
    size_t k;
    if (jtype != JSONArray)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    jarr = json_value_get_array(jval);
    count = json_array_get_count(jarr);
    if (f->fixed_array_len > 0) {
      if (count > f->fixed_array_len)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    } else {
      rc = cdd_reflect_array_resize(obj, f, count);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    for (k = 0; k < count; ++k) {
      JSON_Value *ev = json_array_get_value(jarr, k);
      JSON_Value_Type et = json_value_get_type(ev);
      void *elem_ptr = NULL;
      cdd_reflect_array_at_mut(obj, f, k, &elem_ptr);
      if (f->elem_kind == CDD_FIELD_KIND_INTEGER) {
        struct cdd_field_info ef;
        if (et != JSONNumber)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_INTEGER;
        ef.size = f->elem_size;
        rc = cdd_reflect_set_int64(elem_ptr, &ef,
                                   (int64_t)json_value_get_number(ev));
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (f->elem_kind == CDD_FIELD_KIND_NUMBER) {
        struct cdd_field_info ef;
        if (et != JSONNumber)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_NUMBER;
        ef.size = f->elem_size;
        cdd_reflect_set_double(elem_ptr, &ef, json_value_get_number(ev));
      } else if (f->elem_kind == CDD_FIELD_KIND_BOOLEAN) {
        struct cdd_field_info ef;
        if (et != JSONBoolean)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_BOOLEAN;
        ef.size = f->elem_size;
        cdd_reflect_set_bool(elem_ptr, &ef, json_value_get_boolean(ev));
      } else if (f->elem_kind == CDD_FIELD_KIND_ENUM) {
        struct cdd_field_info ef;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_ENUM;
        ef.size = f->elem_size;
        if (et == JSONString) {
          const struct cdd_enum_info *ei = NULL;
          const struct cdd_enum_variant_info *var = NULL;
          rc = cdd_reflect_enum_by_name(f->ref_name, &ei);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (cdd_reflect_find_enum_variant(ei, json_value_get_string(ev),
                                            &var) != CDD_C_SUCCESS ||
              !var)
            return CDD_C_ERROR_INVALID_ARGUMENT;
          cdd_reflect_set_int64(elem_ptr, &ef, (int64_t)var->value);
        } else if (et == JSONNumber) {
          cdd_reflect_set_int64(elem_ptr, &ef,
                                (int64_t)json_value_get_number(ev));
        } else {
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
      } else if (f->elem_kind == CDD_FIELD_KIND_STRING &&
                 f->elem_size == sizeof(char *)) {
        char **sp = (char **)elem_ptr;
        if (et == JSONNull) {
          if (*sp) {
            C_CDD_FREE(*sp);
            *sp = NULL;
          }
        } else if (et == JSONString) {
          const char *s = json_value_get_string(ev);
          size_t sl = strlen(s);
          char *dup = (char *)C_CDD_MALLOC(sl + 1);
          if (!dup)
            return CDD_C_ERROR_MEMORY;
          memcpy(dup, s, sl + 1);
          if (*sp)
            C_CDD_FREE(*sp);
          *sp = dup;
        } else {
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
      } else if (f->elem_kind == CDD_FIELD_KIND_OBJECT) {
        const struct cdd_type_info *elem_ti = NULL;
        rc = cdd_reflect_type_by_name(f->ref_name, &elem_ti);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (et == JSONNull && f->elem_size == sizeof(void *) &&
            elem_ti->size != sizeof(void *)) {
          void **ep_ptr = (void **)elem_ptr;
          if (*ep_ptr) {
            cdd_reflect_struct_cleanup(*ep_ptr, elem_ti);
            C_CDD_FREE(*ep_ptr);
            *ep_ptr = NULL;
          }
          continue;
        }
        if (et != JSONObject)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        if (f->elem_size == sizeof(void *) && elem_ti->size != sizeof(void *)) {
          void **ep_ptr = (void **)elem_ptr;
          if (!*ep_ptr) {
            rc = cdd_reflect_struct_default(elem_ti, ep_ptr);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          rc = deserialize_struct_from_json_obj(json_value_get_object(ev),
                                                elem_ti, *ep_ptr);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else {
          rc = deserialize_struct_from_json_obj(json_value_get_object(ev),
                                                elem_ti, elem_ptr);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
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
  cdd_c_error_t rc;
  if (ti->is_union) {
    int *tag_ptr = (int *)((void *)((unsigned char *)obj + ti->tag_offset));
    for (i = 0; i < ti->n_fields; ++i) {
      JSON_Value *jv = NULL;
      find_field_json_value(jobj, &ti->fields[i], &jv);
      if (jv) {
        *tag_ptr = (int)(i + 1);
        rc = deserialize_field_from_json(jobj, &ti->fields[i], obj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        break;
      }
    }
    return cdd_reflect_validate(obj, ti);
  }
  for (i = 0; i < ti->n_fields; ++i) {
    rc = deserialize_field_from_json(jobj, &ti->fields[i], obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return cdd_reflect_validate(obj, ti);
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
  JSON_Value *root;
  void *obj = NULL;
  cdd_c_error_t rc;

  if (!json_str || !ti || !out_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = NULL;

  root = json_parse_string(json_str);
  if (!root || json_value_get_type(root) != JSONObject) {
    if (root)
      json_value_free(root);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_reflect_struct_default(ti, &obj);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(root);
    return rc;
  }

  rc = deserialize_struct_from_json_obj(json_value_get_object(root), ti, obj);
  json_value_free(root);
  if (rc != CDD_C_SUCCESS) {
    cdd_reflect_struct_cleanup(obj, ti);
    C_CDD_FREE(obj);
    return rc;
  }

  *out_obj = obj;
  return CDD_C_SUCCESS;
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
  cdd_c_error_t rc;
  for (i = 0; i < ti->n_fields; ++i) {
    const struct cdd_field_info *f = &ti->fields[i];
    const char *key =
        (f->wire_name && f->wire_name[0] != '\0') ? f->wire_name : f->name;
    int is_eq = 0;
    if (f->skip || f->skip_serializing)
      continue;
    rc = cdd_reflect_eq_single_field(source, target, f, &is_eq);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_eq)
      continue;

    if (f->is_pointer && f->bit_width == 0 && f->kind != CDD_FIELD_KIND_ARRAY) {
      const void *tp = *(const void *const *)((
          const void *)((const unsigned char *)target + f->offset));
      if (!tp) {
        json_object_set_null(patch_jobj, key);
        continue;
      }
    }

    if (f->kind == CDD_FIELD_KIND_OBJECT) {
      const struct cdd_type_info *child_ti = NULL;
      const void *so =
          f->is_pointer
              ? *(const void *const *)((
                    const void *)((const unsigned char *)source + f->offset))
              : (const void *)((const unsigned char *)source + f->offset);
      const void *to =
          f->is_pointer
              ? *(const void *const *)((
                    const void *)((const unsigned char *)target + f->offset))
              : (const void *)((const unsigned char *)target + f->offset);
      rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (so && to) {
        JSON_Value *sub_patch = json_value_init_object();
        rc = build_merge_patch_obj(so, to, child_ti,
                                   json_value_get_object(sub_patch));
        if (rc != CDD_C_SUCCESS) {
          json_value_free(sub_patch);
          return rc;
        }
        json_object_set_value(patch_jobj, key, sub_patch);
        continue;
      }
    }

    rc = serialize_field_to_json(target, f, patch_jobj);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
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
  JSON_Value *root;
  char *parson_str;
  char *dup_str;
  size_t slen;
  cdd_c_error_t rc;

  if (!source || !target || !ti || !out_patch_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_patch_json = NULL;

  root = json_value_init_object();
  rc = build_merge_patch_obj(source, target, ti, json_value_get_object(root));
  if (rc != CDD_C_SUCCESS) {
    json_value_free(root);
    return rc;
  }

  parson_str = json_serialize_to_string(root);
  json_value_free(root);
  slen = strlen(parson_str);
  dup_str = (char *)C_CDD_MALLOC(slen + 1);
  if (!dup_str) {
    json_free_serialized_string(parson_str);
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(dup_str, parson_str, slen + 1);
  json_free_serialized_string(parson_str);
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
  size_t count = json_object_get_count(patch_jobj);
  size_t i;
  cdd_c_error_t rc;
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(patch_jobj, i);
    JSON_Value *val = json_object_get_value_at(patch_jobj, i);
    const struct cdd_field_info *f = NULL;
    if (cdd_reflect_find_field(ti, key, &f) != CDD_C_SUCCESS || !f)
      continue;
    if (f->kind == CDD_FIELD_KIND_OBJECT &&
        json_value_get_type(val) == JSONObject) {
      const struct cdd_type_info *child_ti = NULL;
      void *child_obj;
      rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (f->is_pointer) {
        void **pptr = (void **)((void *)((unsigned char *)target + f->offset));
        if (!*pptr) {
          rc = cdd_reflect_struct_default(child_ti, pptr);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        child_obj = *pptr;
      } else {
        child_obj = (void *)((unsigned char *)target + f->offset);
      }
      rc = apply_merge_patch_obj(child_obj, child_ti,
                                 json_value_get_object(val));
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      rc = deserialize_field_from_json(patch_jobj, f, target);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
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
  JSON_Value *root;
  cdd_c_error_t rc;
  if (!target || !ti || !patch_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  root = json_parse_string(patch_json);
  if (!root || json_value_get_type(root) != JSONObject) {
    if (root)
      json_value_free(root);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = apply_merge_patch_obj(target, ti, json_value_get_object(root));
  json_value_free(root);
  return rc;
}
