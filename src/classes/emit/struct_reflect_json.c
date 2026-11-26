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
 * @brief Frees a Parson serialized string via a `*_free`-named helper.
 *
 * @param[in,out] str Serialized JSON string (may be NULL).
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t reflect_serialized_str_free(char *str) {
  return cdd_json_free_serialized_string(str);
}

/**
 * @brief Frees a heap buffer `obj` and returns `err`.
 *
 * @param[in,out] obj Heap pointer to free.
 * @param[in] err Error code to return.
 * @return `err`.
 */
static cdd_c_error_t free_json_obj_and_return_err(void *obj,
                                                  cdd_c_error_t err) {
  C_CDD_FREE(obj);
  return err;
}

/**
 * @brief Cleans up and frees a heap-allocated reflected struct instance.
 *
 * @param[in,out] obj Heap struct pointer.
 * @param[in] ti Runtime type descriptor.
 * @return Cleanup status.
 */
static cdd_c_error_t
reflect_struct_cleanup_free(void *obj, const struct cdd_type_info *ti) {
  return free_json_obj_and_return_err(obj, cdd_reflect_struct_cleanup(obj, ti));
}

/**
 * @brief Looks up an optional JSON value property on `jobj`, leaving `*io_val`
 * NULL if absent while percolating fault injection.
 *
 * @param[in] jobj JSON object.
 * @param[in] key Property name.
 * @param[in,out] io_val Output JSON value pointer.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t reflect_json_opt_value(const JSON_Object *jobj,
                                            const char *key,
                                            JSON_Value **io_val) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *io_val = NULL;
  rc = cdd_json_object_get_value(jobj, key, io_val);
  if (rc == CDD_C_SUCCESS)
    return rc;
  *io_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up an optional JSON string property on `jobj`, setting
 * `*out_str` to NULL if absent while percolating fault injection.
 *
 * @param[in] jobj JSON object.
 * @param[in] key Property name.
 * @param[out] out_str Output string pointer or NULL.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t reflect_json_opt_string(const JSON_Object *jobj,
                                             const char *key,
                                             const char **out_str) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_str = NULL;
  rc = cdd_json_object_get_string(jobj, key, out_str);
  if (rc == CDD_C_SUCCESS)
    return rc;
  *out_str = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a JSON string into `*out_root` if valid and matching
 * `expected_type`, setting `*out_ok` to 1 on match or 0 otherwise (freeing any
 * mismatched `JSON_Value`).
 *
 * @param[in] json_str Input JSON string.
 * @param[in] expected_type Expected root `JSON_Value_Type`.
 * @param[out] out_root Parsed `JSON_Value *` on match, or NULL.
 * @param[out] out_ok Receives 1 on match, 0 on parse/type mismatch.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t reflect_parse_typed_json(const char *json_str,
                                              JSON_Value_Type expected_type,
                                              JSON_Value **out_root,
                                              int *out_ok) {
  JSON_Value *root = NULL;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_root = NULL;
  *out_ok = 0;
  rc = cdd_json_parse_string(json_str, &root);
  if (rc == CDD_C_SUCCESS) {
    if (root->type != expected_type) {
      return cdd_json_value_free(root);
    }
    *out_root = root;
    *out_ok = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up an optional reflected field by name/wire_name/alias, setting
 * `*out_field` to NULL when not found while percolating fault injection.
 *
 * @param[in] ti Runtime type descriptor.
 * @param[in] name Field key name.
 * @param[out] out_field Output field descriptor or NULL.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t
reflect_find_field_opt(const struct cdd_type_info *ti, const char *name,
                       const struct cdd_field_info **out_field) {
  cdd_c_error_t rc;
  *out_field = NULL;
  rc = cdd_reflect_find_field(ti, name, out_field);
  if (rc == CDD_C_SUCCESS)
    return rc;
  if (rc == CDD_C_ERROR_NOT_FOUND)
    return CDD_C_SUCCESS;
  return rc;
}

/**
 * @brief Serializes and frees `root` into a `C_CDD_MALLOC`-duplicated string at
 * `*out_json`.
 *
 * @param[in,out] root Parson root value (always freed).
 * @param[out] out_json Output heap-allocated JSON string.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
static cdd_c_error_t reflect_serialize_root_dup(JSON_Value *root,
                                                char **out_json) {
  char *parson_str = NULL;
  char *dup_str;
  size_t slen;
  cdd_c_error_t rc = cdd_json_serialize_to_string(root, &parson_str);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }
  slen = strlen(parson_str);
  dup_str = (char *)C_CDD_MALLOC(slen + 1);
  if (!dup_str) {
    return cdd_c_keep_err(
        CDD_C_ERROR_MEMORY,
        cdd_c_keep_err(reflect_serialized_str_free(parson_str),
                       cdd_json_value_free(root)));
  }
  memcpy(dup_str, parson_str, slen + 1);
  *out_json = dup_str;
  return cdd_c_keep_err(reflect_serialized_str_free(parson_str),
                        cdd_json_value_free(root));
}

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
  cdd_c_error_t rc;
  if (f->skip || f->skip_serializing)
    return CDD_C_SUCCESS;

  if (f->is_pointer && f->kind != CDD_FIELD_KIND_ARRAY) {
    const void *p = *(const void *const *)((
        const void *)((const unsigned char *)obj + f->offset));
    if (!p) {
      if (f->is_nullable) {
        return cdd_json_object_set_null(parent_jobj, key);
      }
      return CDD_C_SUCCESS;
    }
  }

  if (f->kind == CDD_FIELD_KIND_INTEGER) {
    size_t width = 0;
    int is_unsigned = 0;
    rc = cdd_reflect_resolve_field_scalar_width(f, &width, &is_unsigned);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (width == 16) {
      const void *addr = (const void *)((const unsigned char *)obj + f->offset);
      if (is_unsigned) {
        return cdd_json_object_set_uint128(parent_jobj, key,
                                           *(const cdd_uint128_t *)addr);
      }
      return cdd_json_object_set_int128(parent_jobj, key,
                                        *(const cdd_int128_t *)addr);
    }
    if (width == 8 && is_unsigned) {
      uint64_t uv = 0;
      rc = cdd_reflect_get_uint64(obj, f, &uv);
      if (rc != CDD_C_SUCCESS)
        return rc;
      return cdd_json_object_set_uint64(parent_jobj, key, uv);
    }
    {
      int64_t v = 0;
      rc = cdd_reflect_get_int64(obj, f, &v);
      if (rc != CDD_C_SUCCESS)
        return rc;
      return cdd_json_object_set_int64(parent_jobj, key, v);
    }
  }
  if (f->kind == CDD_FIELD_KIND_NUMBER) {
    double d = 0.0;
    rc = cdd_reflect_get_double(obj, f, &d);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_json_object_set_number(parent_jobj, key, d);
  }
  if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
    int b = 0;
    rc = cdd_reflect_get_bool(obj, f, &b);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_json_object_set_boolean(parent_jobj, key, b);
  }
  if (f->kind == CDD_FIELD_KIND_STRING) {
    const char *s = NULL;
    rc = cdd_reflect_get_string(obj, f, &s);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_json_object_set_string(parent_jobj, key, s);
  }
  if (f->kind == CDD_FIELD_KIND_ENUM) {
    int64_t ev = 0;
    const struct cdd_enum_info *ei = NULL;
    rc = cdd_reflect_get_int64(obj, f, &ev);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_reflect_lookup_enum_opt(f->ref_name, &ei);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (ei) {
      const struct cdd_enum_variant_info *var = NULL;
      rc = cdd_reflect_lookup_enum_value_opt(ei, (int)ev, &var);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (var) {
        return cdd_json_object_set_string(
            parent_jobj, key, var->wire_name ? var->wire_name : var->name);
      }
    }
    return cdd_json_object_set_number(parent_jobj, key, (double)ev);
  }
  if (f->kind == CDD_FIELD_KIND_OBJECT) {
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
    }
    {
      JSON_Value *sub_val = NULL;
      JSON_Object *sub_obj = NULL;
      rc = cdd_json_value_init_object(&sub_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_json_value_get_object(sub_val, &sub_obj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(sub_val));
      }
      rc = serialize_struct_to_json_obj(child_obj, child_ti, sub_obj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(sub_val));
      }
      rc = cdd_json_object_set_value(parent_jobj, key, sub_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(sub_val));
      }
      return CDD_C_SUCCESS;
    }
  }
  if (f->kind == CDD_FIELD_KIND_ARRAY) {
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
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_value_init_array(&arr_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_value_get_array(arr_val, &jarr);
    if (rc != CDD_C_SUCCESS)
      goto err_arr;
    for (k = 0; k < alen; ++k) {
      const void *elem_ptr = NULL;
      rc = cdd_reflect_array_at(obj, f, k, &elem_ptr);
      if (rc != CDD_C_SUCCESS)
        goto err_arr;
      if (f->elem_kind == CDD_FIELD_KIND_INTEGER ||
          f->elem_kind == CDD_FIELD_KIND_ENUM) {
        struct cdd_field_info ef;
        int64_t ev = 0;
        memset(&ef, 0, sizeof(ef));
        ef.kind = f->elem_kind;
        ef.size = f->elem_size;
        rc = cdd_reflect_get_int64(elem_ptr, &ef, &ev);
        if (rc != CDD_C_SUCCESS)
          goto err_arr;
        if (f->elem_kind == CDD_FIELD_KIND_ENUM) {
          const struct cdd_enum_info *ei = NULL;
          rc = cdd_reflect_lookup_enum_opt(f->ref_name, &ei);
          if (rc != CDD_C_SUCCESS)
            goto err_arr;
          if (ei) {
            const struct cdd_enum_variant_info *var = NULL;
            rc = cdd_reflect_lookup_enum_value_opt(ei, (int)ev, &var);
            if (rc != CDD_C_SUCCESS)
              goto err_arr;
            if (var) {
              const char *wn = var->wire_name ? var->wire_name : var->name;
              rc = cdd_json_array_append_string(jarr, wn);
              if (rc != CDD_C_SUCCESS)
                goto err_arr;
              continue;
            }
          }
        }
        rc = cdd_json_array_append_number(jarr, (double)ev);
        if (rc != CDD_C_SUCCESS)
          goto err_arr;
      } else if (f->elem_kind == CDD_FIELD_KIND_NUMBER) {
        struct cdd_field_info ef;
        double dv = 0.0;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_NUMBER;
        ef.size = f->elem_size;
        rc = cdd_reflect_get_double(elem_ptr, &ef, &dv);
        if (rc != CDD_C_SUCCESS)
          goto err_arr;
        rc = cdd_json_array_append_number(jarr, dv);
        if (rc != CDD_C_SUCCESS)
          goto err_arr;
      } else if (f->elem_kind == CDD_FIELD_KIND_BOOLEAN) {
        struct cdd_field_info ef;
        int bv = 0;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_BOOLEAN;
        ef.size = f->elem_size;
        rc = cdd_reflect_get_bool(elem_ptr, &ef, &bv);
        if (rc != CDD_C_SUCCESS)
          goto err_arr;
        rc = cdd_json_array_append_boolean(jarr, bv);
        if (rc != CDD_C_SUCCESS)
          goto err_arr;
      } else if (f->elem_kind == CDD_FIELD_KIND_STRING) {
        const char *se = *(const char *const *)elem_ptr;
        if (se) {
          rc = cdd_json_array_append_string(jarr, se);
          if (rc != CDD_C_SUCCESS)
            goto err_arr;
        } else {
          rc = cdd_json_array_append_null(jarr);
          if (rc != CDD_C_SUCCESS)
            goto err_arr;
        }
      } else {
        const struct cdd_type_info *elem_ti = NULL;
        const void *elem_obj;
        int is_ptr = 0;
        JSON_Value *ev_obj = NULL;
        JSON_Object *ev_jobj = NULL;
        rc = cdd_reflect_type_by_name(f->ref_name, &elem_ti);
        if (rc != CDD_C_SUCCESS)
          goto err_arr;
        rc = cdd_reflect_is_obj_elem_ptr(f, elem_ti, &is_ptr);
        if (rc != CDD_C_SUCCESS)
          goto err_arr;
        elem_obj = is_ptr ? *(const void *const *)elem_ptr : elem_ptr;
        if (!elem_obj) {
          rc = cdd_json_array_append_null(jarr);
          if (rc != CDD_C_SUCCESS)
            goto err_arr;
          continue;
        }
        rc = cdd_json_value_init_object(&ev_obj);
        if (rc != CDD_C_SUCCESS)
          goto err_arr;
        rc = cdd_json_value_get_object(ev_obj, &ev_jobj);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc,
                                cdd_c_keep_err(cdd_json_value_free(ev_obj),
                                               cdd_json_value_free(arr_val)));
        }
        rc = serialize_struct_to_json_obj(elem_obj, elem_ti, ev_jobj);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc,
                                cdd_c_keep_err(cdd_json_value_free(ev_obj),
                                               cdd_json_value_free(arr_val)));
        }
        rc = cdd_json_array_append_value(jarr, ev_obj);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc,
                                cdd_c_keep_err(cdd_json_value_free(ev_obj),
                                               cdd_json_value_free(arr_val)));
        }
      }
    }
    rc = cdd_json_object_set_value(parent_jobj, key, arr_val);
    if (rc != CDD_C_SUCCESS)
      goto err_arr;
    return CDD_C_SUCCESS;
  err_arr:;
    return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
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
  JSON_Value *root = NULL;
  JSON_Object *root_obj = NULL;
  cdd_c_error_t rc;

  if (!obj || !ti || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_json = NULL;

  rc = cdd_json_value_init_object(&root);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_value_get_object(root, &root_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }
  rc = serialize_struct_to_json_obj(obj, ti, root_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }
  return reflect_serialize_root_dup(root, out_json);
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
  cdd_c_error_t rc;
  if (f->de_wire_name) {
    rc = reflect_json_opt_value(jobj, f->de_wire_name, &v);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (!v && f->wire_name) {
    rc = reflect_json_opt_value(jobj, f->wire_name, &v);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (!v) {
    rc = reflect_json_opt_value(jobj, f->name, &v);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (!v && f->aliases) {
    for (k = 0; k < f->n_aliases && !v; ++k) {
      rc = reflect_json_opt_value(jobj, f->aliases[k], &v);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!jval) {
    if (f->required && !f->default_val)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return CDD_C_SUCCESS;
  }

  jtype = jval->type;
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
      if (rc != CDD_C_SUCCESS)
        return rc;
      *pptr = C_CDD_CALLOC(1, width);
      if (!*pptr)
        return CDD_C_ERROR_MEMORY;
    }
  }

  if (f->kind == CDD_FIELD_KIND_INTEGER) {
    size_t width = 0;
    int is_unsigned = 0;
    if (jtype != JSONNumber)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = cdd_reflect_resolve_field_scalar_width(f, &width, &is_unsigned);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (width == 16) {
      void *addr = (void *)((unsigned char *)obj + f->offset);
      if (is_unsigned) {
        return cdd_json_value_get_uint128(jval, (cdd_uint128_t *)addr);
      }
      return cdd_json_value_get_int128(jval, (cdd_int128_t *)addr);
    }
    if (width == 8 && is_unsigned) {
      uint64_t uval = 0;
      rc = cdd_json_value_get_uint64(jval, &uval);
      if (rc != CDD_C_SUCCESS)
        return rc;
      return cdd_reflect_set_uint64(obj, f, uval);
    }
    {
      int64_t ival = 0;
      rc = cdd_json_value_get_int64(jval, &ival);
      if (rc != CDD_C_SUCCESS)
        return rc;
      return cdd_reflect_set_int64(obj, f, ival);
    }
  }
  if (f->kind == CDD_FIELD_KIND_NUMBER) {
    double dnum = 0.0;
    if (jtype != JSONNumber)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = cdd_json_value_get_number(jval, &dnum);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_reflect_set_double(obj, f, dnum);
  }
  if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
    int bval = 0;
    if (jtype != JSONBoolean)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = cdd_json_value_get_boolean(jval, &bval);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_reflect_set_bool(obj, f, bval);
  }
  if (f->kind == CDD_FIELD_KIND_STRING) {
    const char *sval = NULL;
    if (jtype != JSONString)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = cdd_json_value_get_string(jval, &sval);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_reflect_set_string(obj, f, sval);
  }
  if (f->kind == CDD_FIELD_KIND_ENUM) {
    if (jtype == JSONString) {
      const char *es = NULL;
      const struct cdd_enum_info *ei = NULL;
      const struct cdd_enum_variant_info *var = NULL;
      rc = cdd_json_value_get_string(jval, &es);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_reflect_enum_by_name(f->ref_name, &ei);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_reflect_lookup_enum_variant_opt(ei, es, &var);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (var) {
        return cdd_reflect_set_int64(obj, f, (int64_t)var->value);
      }
      if (ei->other_idx >= 0) {
        int ov = ei->variants[ei->other_idx].value;
        return cdd_reflect_set_int64(obj, f, (int64_t)ov);
      }
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (jtype == JSONNumber) {
      double dnum = 0.0;
      rc = cdd_json_value_get_number(jval, &dnum);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
    rc = cdd_json_value_get_object(jval, &sub_jobj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return deserialize_struct_from_json_obj(sub_jobj, child_ti, child_obj);
  }
  {
    JSON_Array *jarr = NULL;
    size_t count = 0;
    size_t k;
    if (jtype != JSONArray)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = cdd_json_value_get_array(jval, &jarr);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_array_get_count(jarr, &count);
    if (rc != CDD_C_SUCCESS)
      return rc;
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
      rc = cdd_json_array_get_value(jarr, k, &ev);
      if (rc != CDD_C_SUCCESS)
        return rc;
      et = ev->type;
      rc = cdd_reflect_array_at_mut(obj, f, k, &elem_ptr);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (f->elem_kind == CDD_FIELD_KIND_INTEGER) {
        struct cdd_field_info ef;
        double dnum = 0.0;
        if (et != JSONNumber)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        rc = cdd_json_value_get_number(ev, &dnum);
        if (rc != CDD_C_SUCCESS)
          return rc;
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
        rc = cdd_json_value_get_number(ev, &dnum);
        if (rc != CDD_C_SUCCESS)
          return rc;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_NUMBER;
        ef.size = f->elem_size;
        rc = cdd_reflect_set_double(elem_ptr, &ef, dnum);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (f->elem_kind == CDD_FIELD_KIND_BOOLEAN) {
        struct cdd_field_info ef;
        int bval = 0;
        if (et != JSONBoolean)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        rc = cdd_json_value_get_boolean(ev, &bval);
        if (rc != CDD_C_SUCCESS)
          return rc;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_BOOLEAN;
        ef.size = f->elem_size;
        rc = cdd_reflect_set_bool(elem_ptr, &ef, bval);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (f->elem_kind == CDD_FIELD_KIND_ENUM) {
        struct cdd_field_info ef;
        memset(&ef, 0, sizeof(ef));
        ef.kind = CDD_FIELD_KIND_ENUM;
        ef.size = f->elem_size;
        if (et == JSONString) {
          const struct cdd_enum_info *ei = NULL;
          const struct cdd_enum_variant_info *var = NULL;
          const char *es = NULL;
          rc = cdd_json_value_get_string(ev, &es);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = cdd_reflect_enum_by_name(f->ref_name, &ei);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = cdd_reflect_lookup_enum_variant_opt(ei, es, &var);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (!var)
            return CDD_C_ERROR_INVALID_ARGUMENT;
          rc = cdd_reflect_set_int64(elem_ptr, &ef, (int64_t)var->value);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (et == JSONNumber) {
          double dnum = 0.0;
          rc = cdd_json_value_get_number(ev, &dnum);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = cdd_reflect_set_int64(elem_ptr, &ef, (int64_t)dnum);
          if (rc != CDD_C_SUCCESS)
            return rc;
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
          rc = cdd_json_value_get_string(ev, &s);
          if (rc != CDD_C_SUCCESS)
            return rc;
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
        int is_ptr = 0;
        rc = cdd_reflect_type_by_name(f->ref_name, &elem_ti);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = cdd_reflect_is_obj_elem_ptr(f, elem_ti, &is_ptr);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (et == JSONNull && is_ptr) {
          void **ep_ptr = (void **)elem_ptr;
          if (*ep_ptr) {
            rc = cdd_reflect_struct_cleanup(*ep_ptr, elem_ti);
            if (rc != CDD_C_SUCCESS)
              return rc;
            C_CDD_FREE(*ep_ptr);
            *ep_ptr = NULL;
          }
          continue;
        }
        if (et != JSONObject)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        rc = cdd_json_value_get_object(ev, &ev_jobj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_ptr) {
          void **ep_ptr = (void **)elem_ptr;
          if (!*ep_ptr) {
            rc = cdd_reflect_struct_default(elem_ti, ep_ptr);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          rc = deserialize_struct_from_json_obj(ev_jobj, elem_ti, *ep_ptr);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else {
          rc = deserialize_struct_from_json_obj(ev_jobj, elem_ti, elem_ptr);
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
      rc = find_field_json_value(jobj, &ti->fields[i], &jv);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
  JSON_Object *jobj = NULL;
  void *obj = NULL;
  int parse_ok = 0;
  cdd_c_error_t rc;

  if (!json_str || !ti || !out_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = NULL;

  rc = reflect_parse_typed_json(json_str, JSONObject, &root, &parse_ok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!parse_ok)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_reflect_struct_default(ti, &obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }
  rc = cdd_json_value_get_object(root, &jobj);
  if (rc != CDD_C_SUCCESS)
    goto err_from_json;
  rc = deserialize_struct_from_json_obj(jobj, ti, obj);
  if (rc != CDD_C_SUCCESS)
    goto err_from_json;
  *out_obj = obj;
  return cdd_json_value_free(root);

err_from_json:;
  return cdd_c_keep_err(rc, cdd_c_keep_err(reflect_struct_cleanup_free(obj, ti),
                                           cdd_json_value_free(root)));
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
    const char *key = f->wire_name ? f->wire_name : f->name;
    const void *sa = (const unsigned char *)source + f->offset;
    const void *ta = (const unsigned char *)target + f->offset;
    int is_eq = 1;
    if (!f->skip && !f->skip_serializing) {
      rc = cdd_reflect_eq_single_field(source, target, f, &is_eq);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (is_eq)
      continue;

    if (f->is_pointer && f->kind != CDD_FIELD_KIND_ARRAY &&
        !*(const void *const *)ta) {
      rc = cdd_json_object_set_null(patch_jobj, key);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (f->kind == CDD_FIELD_KIND_OBJECT && !f->is_pointer) {
      const struct cdd_type_info *child_ti = NULL;
      JSON_Value *sub_patch = NULL;
      JSON_Object *sub_jobj = NULL;
      rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_json_value_init_object(&sub_patch);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_json_value_get_object(sub_patch, &sub_jobj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(sub_patch));
      }
      rc = build_merge_patch_obj(sa, ta, child_ti, sub_jobj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(sub_patch));
      }
      rc = cdd_json_object_set_value(patch_jobj, key, sub_patch);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(sub_patch));
      }
    } else {
      rc = serialize_field_to_json(target, f, patch_jobj);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
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
  JSON_Value *root = NULL;
  JSON_Object *root_obj = NULL;
  cdd_c_error_t rc;

  if (!source || !target || !ti || !out_patch_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_patch_json = NULL;

  rc = cdd_json_value_init_object(&root);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_value_get_object(root, &root_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }
  rc = build_merge_patch_obj(source, target, ti, root_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }
  return reflect_serialize_root_dup(root, out_patch_json);
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
  cdd_c_error_t rc = cdd_json_object_get_count(patch_jobj, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < count; ++i) {
    const char *key = NULL;
    JSON_Value *val = NULL;
    const struct cdd_field_info *f = NULL;
    rc = cdd_json_object_get_name(patch_jobj, i, &key);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_get_value_at(patch_jobj, i, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = reflect_find_field_opt(ti, key, &f);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!f)
      continue;
    if (f->kind == CDD_FIELD_KIND_OBJECT && val->type == JSONObject) {
      const struct cdd_type_info *child_ti = NULL;
      void *child_obj = (void *)((unsigned char *)target + f->offset);
      JSON_Object *sub_jobj = NULL;
      rc = cdd_json_value_get_object(val, &sub_jobj);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_reflect_type_by_name(f->ref_name, &child_ti);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (f->is_pointer) {
        void **pptr = (void **)child_obj;
        if (!*pptr) {
          rc = cdd_reflect_struct_default(child_ti, pptr);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        child_obj = *pptr;
      }
      rc = apply_merge_patch_obj(child_obj, child_ti, sub_jobj);
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
  JSON_Value *root = NULL;
  JSON_Object *root_obj = NULL;
  int parse_ok = 0;
  cdd_c_error_t rc;
  if (!target || !ti || !patch_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = reflect_parse_typed_json(patch_json, JSONObject, &root, &parse_ok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!parse_ok)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_json_value_get_object(root, &root_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }
  rc = apply_merge_patch_obj(target, ti, root_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }
  return cdd_json_value_free(root);
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
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (f->kind == CDD_FIELD_KIND_NUMBER) {
    rc = cdd_json_object_set_string(prop_obj, "type", "number");
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
    rc = cdd_json_object_set_string(prop_obj, "type", "boolean");
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (f->kind == CDD_FIELD_KIND_STRING) {
    rc = cdd_json_object_set_string(prop_obj, "type", "string");
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (f->has_min_len) {
      rc =
          cdd_json_object_set_number(prop_obj, "minLength", (double)f->min_len);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (f->has_max_len) {
      rc =
          cdd_json_object_set_number(prop_obj, "maxLength", (double)f->max_len);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (f->pattern) {
      rc = cdd_json_object_set_string(prop_obj, "pattern", f->pattern);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  } else if (f->kind == CDD_FIELD_KIND_ENUM) {
    const struct cdd_enum_info *ei = NULL;
    rc = cdd_json_object_set_string(prop_obj, "type", "string");
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_reflect_lookup_enum_opt(f->ref_name, &ei);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (ei) {
      JSON_Value *ev_arr_val = NULL;
      JSON_Array *ev_arr = NULL;
      size_t k;
      rc = cdd_json_value_init_array(&ev_arr_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_json_value_get_array(ev_arr_val, &ev_arr);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(ev_arr_val));
      }
      for (k = 0; k < ei->n_variants; ++k) {
        if (!ei->variants[k].skip) {
          rc = cdd_json_array_append_string(ev_arr, ei->variants[k].name);
          if (rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(rc, cdd_json_value_free(ev_arr_val));
          }
        }
      }
      rc = cdd_json_object_set_value(prop_obj, "enum", ev_arr_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(ev_arr_val));
      }
    }
  } else if (f->kind == CDD_FIELD_KIND_OBJECT) {
    char ref_buf[192];
    CDD_SNPRINTF(ref_buf, sizeof(ref_buf), "#/components/schemas/%s",
                 f->ref_name);
    rc = cdd_json_object_set_string(prop_obj, "$ref", ref_buf);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    JSON_Value *items_val = NULL;
    JSON_Object *items_obj = NULL;
    rc = cdd_json_object_set_string(prop_obj, "type", "array");
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (f->has_min_len) {
      rc = cdd_json_object_set_number(prop_obj, "minItems", (double)f->min_len);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (f->has_max_len) {
      rc = cdd_json_object_set_number(prop_obj, "maxItems", (double)f->max_len);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (f->unique_items) {
      rc = cdd_json_object_set_boolean(prop_obj, "uniqueItems", 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = cdd_json_value_init_object(&items_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_value_get_object(items_val, &items_obj);
    if (rc != CDD_C_SUCCESS)
      goto err_items;
    if (f->elem_kind == CDD_FIELD_KIND_INTEGER) {
      rc = cdd_json_object_set_string(items_obj, "type", "integer");
      if (rc != CDD_C_SUCCESS)
        goto err_items;
    } else if (f->elem_kind == CDD_FIELD_KIND_NUMBER) {
      rc = cdd_json_object_set_string(items_obj, "type", "number");
      if (rc != CDD_C_SUCCESS)
        goto err_items;
    } else if (f->elem_kind == CDD_FIELD_KIND_BOOLEAN) {
      rc = cdd_json_object_set_string(items_obj, "type", "boolean");
      if (rc != CDD_C_SUCCESS)
        goto err_items;
    } else if (f->elem_kind == CDD_FIELD_KIND_STRING) {
      rc = cdd_json_object_set_string(items_obj, "type", "string");
      if (rc != CDD_C_SUCCESS)
        goto err_items;
    } else if (f->elem_kind == CDD_FIELD_KIND_OBJECT) {
      char ref_buf[192];
      CDD_SNPRINTF(ref_buf, sizeof(ref_buf), "#/components/schemas/%s",
                   f->ref_name);
      rc = cdd_json_object_set_string(items_obj, "$ref", ref_buf);
      if (rc != CDD_C_SUCCESS)
        goto err_items;
    }
    rc = cdd_json_object_set_value(prop_obj, "items", items_val);
    if (rc != CDD_C_SUCCESS)
      goto err_items;
    goto after_items;
  err_items:;
    return cdd_c_keep_err(rc, cdd_json_value_free(items_val));
  after_items:;
  }

  if (f->has_min) {
    rc = cdd_json_object_set_number(
        prop_obj, f->exclusive_min ? "exclusiveMinimum" : "minimum",
        f->min_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (f->has_max) {
    rc = cdd_json_object_set_number(
        prop_obj, f->exclusive_max ? "exclusiveMaximum" : "maximum",
        f->max_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (f->format) {
    rc = cdd_json_object_set_string(prop_obj, "format", f->format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (f->doc) {
    rc = cdd_json_object_set_string(prop_obj, "description", f->doc);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (f->deprecated) {
    rc = cdd_json_object_set_boolean(prop_obj, "deprecated", 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (f->read_only) {
    rc = cdd_json_object_set_boolean(prop_obj, "readOnly", 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (f->write_only) {
    rc = cdd_json_object_set_boolean(prop_obj, "writeOnly", 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
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
  size_t i;
  cdd_c_error_t rc;

  if (!ti || !out_schema_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_schema_json = NULL;

  rc = cdd_json_value_init_object(&root);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_value_get_object(root, &root_obj);
  if (rc != CDD_C_SUCCESS)
    goto err_schema;
  rc = cdd_json_object_set_string(root_obj, "title", ti->name);
  if (rc != CDD_C_SUCCESS)
    goto err_schema;
  rc = cdd_json_object_set_string(root_obj, "type", "object");
  if (rc != CDD_C_SUCCESS)
    goto err_schema;
  rc = cdd_json_value_init_object(&props_val);
  if (rc != CDD_C_SUCCESS)
    goto err_schema;
  rc = cdd_json_value_get_object(props_val, &props_obj);
  if (rc != CDD_C_SUCCESS)
    goto err_schema;

  for (i = 0; i < ti->n_fields; ++i) {
    const struct cdd_field_info *f = &ti->fields[i];
    const char *key = f->wire_name ? f->wire_name : f->name;
    JSON_Value *f_val = NULL;
    JSON_Object *f_obj = NULL;
    if (f->skip)
      continue;
    rc = cdd_json_value_init_object(&f_val);
    if (rc != CDD_C_SUCCESS)
      goto err_schema;
    rc = cdd_json_value_get_object(f_val, &f_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(
                  cdd_json_value_free(f_val),
                  cdd_c_keep_err(cdd_json_value_free(req_val),
                                 cdd_c_keep_err(cdd_json_value_free(props_val),
                                                cdd_json_value_free(root)))));
    }
    rc = build_field_schema_obj(f, f_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(
                  cdd_json_value_free(f_val),
                  cdd_c_keep_err(cdd_json_value_free(req_val),
                                 cdd_c_keep_err(cdd_json_value_free(props_val),
                                                cdd_json_value_free(root)))));
    }
    rc = cdd_json_object_set_value(props_obj, key, f_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(
                  cdd_json_value_free(f_val),
                  cdd_c_keep_err(cdd_json_value_free(req_val),
                                 cdd_c_keep_err(cdd_json_value_free(props_val),
                                                cdd_json_value_free(root)))));
    }
    if (f->required) {
      if (!req_val) {
        rc = cdd_json_value_init_array(&req_val);
        if (rc != CDD_C_SUCCESS)
          goto err_schema;
        rc = cdd_json_value_get_array(req_val, &req_arr);
        if (rc != CDD_C_SUCCESS)
          goto err_schema;
      }
      rc = cdd_json_array_append_string(req_arr, key);
      if (rc != CDD_C_SUCCESS)
        goto err_schema;
    }
  }

  rc = cdd_json_object_set_value(root_obj, "properties", props_val);
  if (rc != CDD_C_SUCCESS)
    goto err_schema;
  props_val = NULL;
  if (req_val) {
    rc = cdd_json_object_set_value(root_obj, "required", req_val);
    if (rc != CDD_C_SUCCESS)
      goto err_schema;
    req_val = NULL;
  }
  return reflect_serialize_root_dup(root, out_schema_json);

err_schema:;
  return cdd_c_keep_err(
      rc, cdd_c_keep_err(cdd_json_value_free(req_val),
                         cdd_c_keep_err(cdd_json_value_free(props_val),
                                        cdd_json_value_free(root))));
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

/**
 * @brief Reads an optional 64-bit integer value for form-urlencoded emission,
 * setting `*out_ok` to 1 when representable or 0 when skipped while percolating
 * `CDD_C_ERROR_UNKNOWN` fault injection.
 *
 * @param[in] obj Struct instance.
 * @param[in] f Field descriptor.
 * @param[out] out_val Output 64-bit integer.
 * @param[out] out_ok Receives 1 on success, 0 when skipped.
 * @return CDD_C_SUCCESS on completion, or `CDD_C_ERROR_UNKNOWN` on fault
 * injection.
 */
static cdd_c_error_t reflect_opt_get_int64(const void *obj,
                                           const struct cdd_field_info *f,
                                           int64_t *out_val, int *out_ok) {
  cdd_c_error_t rc;
  *out_val = 0;
  *out_ok = 0;
  rc = cdd_reflect_get_int64(obj, f, out_val);
  if (rc == CDD_C_SUCCESS) {
    *out_ok = 1;
    return rc;
  }
  if (rc == CDD_C_ERROR_UNKNOWN)
    return rc;
  return CDD_C_SUCCESS;
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
        int ok = 0;
        cdd_c_error_t rc = reflect_opt_get_int64(obj, f, &v, &ok);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(buf);
          return rc;
        }
        if (ok) {
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

    rc = reflect_find_field_opt(ti, key_buf, &f);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, reflect_struct_cleanup_free(obj, ti));
    }
    if (f && !f->skip && !f->skip_deserializing) {
      rc = parse_scalar_str_into_field(obj, f, val_buf);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, reflect_struct_cleanup_free(obj, ti));
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
    key_buf[0] = '\0';
    eq = strchr(arg, '=');
    if (eq) {
      size_t klen = (size_t)(eq - arg);
      if (klen >= sizeof(key_buf))
        continue;
      memcpy(key_buf, arg, klen);
      key_buf[klen] = '\0';
      val_str = eq + 1;
    } else {
      if (strlen(arg) >= sizeof(key_buf))
        continue;
      CDD_STRCPY(key_buf, sizeof(key_buf), arg);
    }

    rc = reflect_find_field_opt(ti, key_buf, &f);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, reflect_struct_cleanup_free(obj, ti));
    }
    if (!f)
      continue;

    if (!val_str) {
      if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
        val_str = "true";
      } else if (i + 1 < argc) {
        val_str = argv[++i];
      } else {
        return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                              reflect_struct_cleanup_free(obj, ti));
      }
    }

    rc = parse_scalar_str_into_field(obj, f, val_str);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, reflect_struct_cleanup_free(obj, ti));
    }
  }

  *out_obj = obj;
  return CDD_C_SUCCESS;
}

/*
 * @brief Applies an RFC 6902 JSON Patch array (`[{"op": "add"|"remove"|
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
  int parse_ok = 0;
  cdd_c_error_t rc;

  if (!target || !ti || !patch_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = reflect_parse_typed_json(patch_json, JSONArray, &root, &parse_ok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!parse_ok)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_json_value_get_array(root, &ops);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_jpatch;
  rc = cdd_json_array_get_count(ops, &count);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_jpatch;
  for (i = 0; i < count; ++i) {
    JSON_Object *op_obj = NULL;
    const char *op_name = NULL;
    const char *path = NULL;
    JSON_Value *val = NULL;
    const struct cdd_field_info *f = NULL;
    const void *parent_obj = NULL;
    const void *field_ptr = NULL;

    rc = cdd_json_array_get_object(ops, i, &op_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_jpatch;
    rc = reflect_json_opt_string(op_obj, "op", &op_name);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_jpatch;
    rc = reflect_json_opt_string(op_obj, "path", &path);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_jpatch;
    rc = reflect_json_opt_value(op_obj, "value", &val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_jpatch;

    if (!op_name || !path) {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
      goto cleanup_jpatch;
    }

    rc =
        cdd_reflect_resolve_path(target, ti, path, &f, &parent_obj, &field_ptr);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_jpatch;

    if (strcmp(op_name, "add") == 0 || strcmp(op_name, "replace") == 0) {
      if (!val) {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
        goto cleanup_jpatch;
      }
      if (val->type == JSONNumber) {
        double dnum = 0.0;
        rc = cdd_json_value_get_number(val, &dnum);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
        if (f->kind == CDD_FIELD_KIND_NUMBER ||
            f->elem_kind == CDD_FIELD_KIND_NUMBER) {
          rc = cdd_reflect_set_double_by_path(target, ti, path, dnum);
          if (rc != CDD_C_SUCCESS)
            goto cleanup_jpatch;
        } else {
          rc = cdd_reflect_set_int64_by_path(target, ti, path, (int64_t)dnum);
          if (rc != CDD_C_SUCCESS)
            goto cleanup_jpatch;
        }
      } else if (val->type == JSONBoolean) {
        int bval = 0;
        rc = cdd_json_value_get_boolean(val, &bval);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
        rc = cdd_reflect_set_bool_by_path(target, ti, path, bval);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
      } else if (val->type == JSONString) {
        const char *sval = NULL;
        rc = cdd_json_value_get_string(val, &sval);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
        rc = cdd_reflect_set_string_by_path(target, ti, path, sval);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
      } else {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
        goto cleanup_jpatch;
      }
    } else if (strcmp(op_name, "remove") == 0) {
      union {
        const void *c_ptr;
        void *m_ptr;
      } u_p;
      u_p.c_ptr = parent_obj;
      rc = cdd_reflect_cleanup_single_field(u_p.m_ptr, f);
      if (rc != CDD_C_SUCCESS)
        goto cleanup_jpatch;
    } else if (strcmp(op_name, "test") == 0) {
      if (!val) {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
        goto cleanup_jpatch;
      }
      if (val->type == JSONNumber) {
        int64_t iv = 0;
        double dnum = 0.0;
        rc = cdd_json_value_get_number(val, &dnum);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
        rc = cdd_reflect_get_int64_by_path(target, ti, path, &iv);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
        if (iv != (int64_t)dnum) {
          rc = CDD_C_ERROR_INVALID_ARGUMENT;
          goto cleanup_jpatch;
        }
      } else if (val->type == JSONString) {
        const char *sv = NULL;
        rc = cdd_reflect_get_string_by_path(target, ti, path, &sv);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
        if (!sv || strcmp(sv, val->string_val) != 0) {
          rc = CDD_C_ERROR_INVALID_ARGUMENT;
          goto cleanup_jpatch;
        }
      } else {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
        goto cleanup_jpatch;
      }
    } else if (strcmp(op_name, "copy") == 0 || strcmp(op_name, "move") == 0) {
      const char *from_path = NULL;
      const struct cdd_field_info *from_f = NULL;
      const void *from_parent = NULL;
      const void *from_ptr = NULL;
      rc = reflect_json_opt_string(op_obj, "from", &from_path);
      if (rc != CDD_C_SUCCESS)
        goto cleanup_jpatch;
      if (!from_path) {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
        goto cleanup_jpatch;
      }
      rc = cdd_reflect_resolve_path(target, ti, from_path, &from_f,
                                    &from_parent, &from_ptr);
      if (rc != CDD_C_SUCCESS)
        goto cleanup_jpatch;
      if (from_f->kind == CDD_FIELD_KIND_STRING) {
        const char *sv = NULL;
        rc = cdd_reflect_get_string_by_path(target, ti, from_path, &sv);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
        rc = cdd_reflect_set_string_by_path(target, ti, path, sv);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
      } else {
        int64_t iv = 0;
        rc = cdd_reflect_get_int64_by_path(target, ti, from_path, &iv);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
        rc = cdd_reflect_set_int64_by_path(target, ti, path, iv);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
      }
      if (strcmp(op_name, "move") == 0) {
        union {
          const void *c_ptr;
          void *m_ptr;
        } u_fp;
        u_fp.c_ptr = from_parent;
        rc = cdd_reflect_cleanup_single_field(u_fp.m_ptr, from_f);
        if (rc != CDD_C_SUCCESS)
          goto cleanup_jpatch;
      }
    } else {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
      goto cleanup_jpatch;
    }
  }

cleanup_jpatch:;
  return cdd_c_keep_err(rc, cdd_json_value_free(root));
}
