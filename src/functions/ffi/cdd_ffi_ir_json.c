/**
 * @file cdd_ffi_ir_json.c
 * @brief Core cdd ffi ir json definitions and implementation.
 */
/* clang-format off */
#include "../../include/cdd_c_error.h"
#include "../../include/ffi/cdd_ffi_ir.h"
#include "../../include/c_cdd/json_dom.h"
#include "../../include/c_cdd/memory.h"
#include "../../include/c_cdd/reflect.h"
#include "../../include/c_cdd/safe_crt.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Reflected fields for `struct cdd_ffi_number_t`. */
static const struct cdd_field_info g_cdd_ffi_number_fields[] = {
    {"raw_spelling",
     "raw_spelling",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct cdd_ffi_number_t, raw_spelling),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"prefix",
     "prefix",
     "int",
     NULL,
     CDD_FIELD_KIND_INTEGER,
     offsetof(struct cdd_ffi_number_t, prefix),
     sizeof(cdd_ffi_number_prefix_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"has_separators",
     "has_separators",
     "int",
     NULL,
     CDD_FIELD_KIND_INTEGER,
     offsetof(struct cdd_ffi_number_t, has_separators),
     sizeof(int),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

/** @brief Runtime reflection descriptor for `struct cdd_ffi_number_t`. */
static const struct cdd_type_info g_cdd_ffi_number_type_info = {
    "cdd_ffi_number_t",
    sizeof(struct cdd_ffi_number_t),
    0,
    g_cdd_ffi_number_fields,
    sizeof(g_cdd_ffi_number_fields) / sizeof(g_cdd_ffi_number_fields[0]),
    0};

/** @brief Reflected fields for `struct cdd_ffi_macro_expansion_t`. */
static const struct cdd_field_info g_cdd_ffi_macro_expansion_fields[] = {
    {"macro_name",
     "macro_name",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct cdd_ffi_macro_expansion_t, macro_name),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"raw_args",
     "raw_args",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct cdd_ffi_macro_expansion_t, raw_args),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"expanded_text",
     "expanded_text",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct cdd_ffi_macro_expansion_t, expanded_text),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

/** @brief Runtime reflection descriptor for `struct cdd_ffi_macro_expansion_t`.
 */
static const struct cdd_type_info g_cdd_ffi_macro_expansion_type_info = {
    "cdd_ffi_macro_expansion_t",
    sizeof(struct cdd_ffi_macro_expansion_t),
    0,
    g_cdd_ffi_macro_expansion_fields,
    sizeof(g_cdd_ffi_macro_expansion_fields) /
        sizeof(g_cdd_ffi_macro_expansion_fields[0]),
    0};

/** @brief Reflected fields for `cdd_ffi_base_class_t`. */
static const struct cdd_field_info g_cdd_ffi_base_class_fields[] = {
    {"name",
     "name",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_ffi_base_class_t, name),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"is_virtual",
     "is_virtual",
     "int",
     NULL,
     CDD_FIELD_KIND_INTEGER,
     offsetof(cdd_ffi_base_class_t, is_virtual),
     sizeof(int),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"access",
     "access",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_ffi_base_class_t, access),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

/** @brief Runtime reflection descriptor for `cdd_ffi_base_class_t`. */
static const struct cdd_type_info g_cdd_ffi_base_class_type_info = {
    "cdd_ffi_base_class_t",
    sizeof(cdd_ffi_base_class_t),
    0,
    g_cdd_ffi_base_class_fields,
    sizeof(g_cdd_ffi_base_class_fields) /
        sizeof(g_cdd_ffi_base_class_fields[0]),
    0};

/** @brief Sets a string property on a JSON object or returns error */
#define SET_STR(o, k, v)                                                       \
  do {                                                                         \
    cdd_c_error_t _rc_s = set_string((o), (k), (v));                           \
    if (_rc_s != CDD_C_SUCCESS) {                                              \
      return cdd_c_keep_err(_rc_s, cdd_json_value_free(val));                  \
    }                                                                          \
  } while (0)
/** @brief Sets an integer property on a JSON object or returns error */
#define SET_INT(o, k, v)                                                       \
  do {                                                                         \
    cdd_c_error_t _rc_i = set_int((o), (k), (v));                              \
    if (_rc_i != CDD_C_SUCCESS) {                                              \
      return cdd_c_keep_err(_rc_i, cdd_json_value_free(val));                  \
    }                                                                          \
  } while (0)

/**
 * @brief Allocates a new `JSON_Value *` object and retrieves its `JSON_Object`.
 *
 * @param[out] out_val Output `JSON_Value *`.
 * @param[out] out_obj Output `JSON_Object *`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t ffi_new_json_object(JSON_Value **out_val,
                                         JSON_Object **out_obj) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc = cdd_json_value_init_object(&v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_value_get_object(v, out_obj);
  if (rc != CDD_C_SUCCESS) {
    *out_obj = NULL;
    return cdd_c_keep_err(rc, cdd_json_value_free(v));
  }
  *out_val = v;
  return CDD_C_SUCCESS;
}

/**
 * @brief Allocates a new `JSON_Value *` array and retrieves its `JSON_Array`.
 *
 * @param[out] out_val Output `JSON_Value *`.
 * @param[out] out_arr Output `JSON_Array *`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t ffi_new_json_array(JSON_Value **out_val,
                                        JSON_Array **out_arr) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc = cdd_json_value_init_array(&v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_value_get_array(v, out_arr);
  if (rc != CDD_C_SUCCESS) {
    *out_arr = NULL;
    return cdd_c_keep_err(rc, cdd_json_value_free(v));
  }
  *out_val = v;
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets `child` under `key` on `obj`, freeing `child` on failure.
 *
 * @param[in,out] obj Target JSON object.
 * @param[in] key Property name.
 * @param[in] child Child value to attach.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t ffi_set_val_safe(JSON_Object *obj, const char *key,
                                      JSON_Value *child) {
  cdd_c_error_t rc = cdd_json_object_set_value(obj, key, child);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(child));
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends `child` to `arr`, freeing `child` on failure.
 *
 * @param[in,out] arr Target JSON array.
 * @param[in] child Child value to append.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t ffi_append_val_safe(JSON_Array *arr, JSON_Value *child) {
  cdd_c_error_t rc = cdd_json_array_append_value(arr, child);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(child));
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to read a number property from a JSON_Object with 0.0 default.
 *
 * @param[in] obj JSON_Object to read from.
 * @param[in] name Key name.
 * @param[out] out_num Output number.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t get_num_opt(const JSON_Object *obj, const char *name,
                                 double *out_num) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_num = 0.0;
  rc = cdd_json_object_get_number(obj, name, out_num);
  if (rc == CDD_C_SUCCESS)
    return rc;
  *out_num = 0.0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Reads a numeric property when `name` is present on `obj` (defaulting
 * to 0.0 if non-numeric), setting `*out_has` to 1 when present.
 *
 * @param[in] obj JSON object to inspect.
 * @param[in] name Key name.
 * @param[out] out_num Output number (0.0 if absent or non-numeric).
 * @param[out] out_has Receives 1 if `name` is present on `obj`, 0 otherwise.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t get_num_if_present(const JSON_Object *obj,
                                        const char *name, double *out_num,
                                        int *out_has) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_num = 0.0;
  *out_has = 0;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc == CDD_C_SUCCESS) {
    *out_has = 1;
    *out_num = (v->type == JSONNumber) ? v->number_val : 0.0;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to read a string property from a JSON_Object with NULL default.
 *
 * @param[in] obj JSON_Object to read from.
 * @param[in] name Key name.
 * @param[out] out_str Output string pointer.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t get_str_opt(const JSON_Object *obj, const char *name,
                                 const char **out_str) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_str = NULL;
  rc = cdd_json_object_get_string(obj, name, out_str);
  if (rc == CDD_C_SUCCESS)
    return rc;
  *out_str = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to read an object property from a JSON_Object with NULL
 * default.
 *
 * @param[in] obj JSON_Object to read from.
 * @param[in] name Key name.
 * @param[out] out_obj Output JSON_Object pointer.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t get_obj_opt(const JSON_Object *obj, const char *name,
                                 JSON_Object **out_obj) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_obj = NULL;
  rc = cdd_json_object_get_object(obj, name, out_obj);
  if (rc == CDD_C_SUCCESS)
    return rc;
  *out_obj = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to read an array property from a JSON_Object with NULL default.
 *
 * @param[in] obj JSON_Object to read from.
 * @param[in] name Key name.
 * @param[out] out_arr Output JSON_Array pointer.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
static cdd_c_error_t get_arr_opt(const JSON_Object *obj, const char *name,
                                 JSON_Array **out_arr) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_arr = NULL;
  rc = cdd_json_object_get_array(obj, name, out_arr);
  if (rc == CDD_C_SUCCESS)
    return rc;
  *out_arr = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Reads an optional `JSON_Object *` element at `arr[idx]`, setting
 * `*out_obj` to NULL if the element is null/non-object.
 *
 * @param[in] arr Source JSON array.
 * @param[in] idx Element index.
 * @param[out] out_obj Output `JSON_Object *` or NULL.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t ffi_arr_obj_opt(const JSON_Array *arr, size_t idx,
                                     JSON_Object **out_obj) {
  JSON_Value *ev = NULL;
  cdd_c_error_t rc = cdd_json_array_get_value(arr, idx, &ev);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_obj = (ev->type == JSONObject) ? ev->object_val : NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets a string property on a Parson `JSON_Object` if `val` is non-NULL.
 *
 * @param[in,out] obj Target JSON object.
 * @param[in] name Property key name.
 * @param[in] val String value to set (ignored if NULL).
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t set_string(JSON_Object *obj, const char *name,
                                const char *val) {
  if (val) {
    return cdd_json_object_set_string(obj, name, val);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets an integer property on a Parson `JSON_Object`.
 *
 * @param[in,out] obj Target JSON object.
 * @param[in] name Property key name.
 * @param[in] val Integer value to set.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t set_int(JSON_Object *obj, const char *name, int val) {
  return cdd_json_object_set_number(obj, name, (double)val);
}

/**
 * @brief Serializes a linked list of `cdd_ffi_trivia_t` into a JSON array.
 *
 * @param[in] trivia Head of trivia linked list.
 * @param[out] out_val Output pointer receiving the allocated `JSON_Value *`.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t trivia_to_json(const cdd_ffi_trivia_t *trivia,
                                    JSON_Value **out_val) {
  JSON_Value *val = NULL;
  JSON_Array *arr = NULL;
  const cdd_ffi_trivia_t *curr = trivia;
  cdd_c_error_t rc = ffi_new_json_array(&val, &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  while (curr) {
    JSON_Value *t_val = NULL;
    JSON_Object *t_obj = NULL;
    rc = ffi_new_json_object(&t_val, &t_obj);
    if (rc != CDD_C_SUCCESS)
      goto err_triv;
    rc = set_int(t_obj, "kind", (int)curr->kind);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(t_val),
                                               cdd_json_value_free(val)));
    }
    rc = set_string(t_obj, "text", curr->text);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(t_val),
                                               cdd_json_value_free(val)));
    }
    rc = ffi_append_val_safe(arr, t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_triv;
    curr = curr->next;
  }
  *out_val = val;
  return CDD_C_SUCCESS;

err_triv:;
  return cdd_c_keep_err(rc, cdd_json_value_free(val));
}

/**
 * @brief Serializes a `cdd_ffi_number_t` structure into a JSON object.
 *
 * @param[in] num Numeric literal metadata structure.
 * @param[out] out_val Output pointer receiving the allocated `JSON_Value *`.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t number_to_json(const struct cdd_ffi_number_t *num,
                                    JSON_Value **out_val) {
  JSON_Value *val = NULL;
  JSON_Object *obj = NULL;
  cdd_c_error_t rc = ffi_new_json_object(&val, &obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_reflect_to_json_object(num, &g_cdd_ffi_number_type_info, obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(val));
  }
  *out_val = val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Serializes a `cdd_ffi_macro_expansion_t` structure into a JSON object.
 *
 * @param[in] m Macro expansion metadata structure.
 * @param[out] out_val Output pointer receiving the allocated `JSON_Value *`.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t
macro_expansion_to_json(const struct cdd_ffi_macro_expansion_t *m,
                        JSON_Value **out_val) {
  JSON_Value *val = NULL;
  JSON_Object *obj = NULL;
  cdd_c_error_t rc = ffi_new_json_object(&val, &obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_reflect_to_json_object(m, &g_cdd_ffi_macro_expansion_type_info, obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(val));
  }
  *out_val = val;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t field_to_json(const cdd_ffi_field_t *field,
                                   JSON_Value **out_val);

/**
 * @brief Serializes a `cdd_ffi_type_t` structure into a JSON object.
 *
 * @param[in] type FFI type descriptor to serialize.
 * @param[out] out_val Output pointer receiving the allocated `JSON_Value *`.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t type_to_json(const cdd_ffi_type_t *type,
                                  JSON_Value **out_val) {
  JSON_Value *val = NULL;
  JSON_Object *obj = NULL;
  cdd_c_error_t rc = ffi_new_json_object(&val, &obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  SET_INT(obj, "kind", (int)type->kind);
  SET_INT(obj, "is_const", type->is_const);
  SET_INT(obj, "pointer_depth", type->pointer_depth);
  SET_STR(obj, "ref_name", type->ref_name);
  if (type->array_size != -1) {
    SET_INT(obj, "array_size", (int)type->array_size);
  }

  if (type->template_args) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    size_t i;
    rc = ffi_new_json_array(&arr_val, &arr);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
    for (i = 0; i < type->template_args_count; ++i) {
      JSON_Value *t_val = NULL;
      rc = type_to_json(&type->template_args[i], &t_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
      rc = ffi_append_val_safe(arr, t_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
    }
    rc = ffi_set_val_safe(obj, "template_args", arr_val);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
  }
  if (type->leading_trivia) {
    JSON_Value *t_val = NULL;
    rc = trivia_to_json(type->leading_trivia, &t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
    rc = ffi_set_val_safe(obj, "leading_trivia", t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
  }
  if (type->trailing_trivia) {
    JSON_Value *t_val = NULL;
    rc = trivia_to_json(type->trailing_trivia, &t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
    rc = ffi_set_val_safe(obj, "trailing_trivia", t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
  }
  SET_STR(obj, "raw_spelling", type->raw_spelling);
  if (type->fn_return_type) {
    JSON_Value *rt_val = NULL;
    rc = type_to_json(type->fn_return_type, &rt_val);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
    rc = ffi_set_val_safe(obj, "fn_return_type", rt_val);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
  }
  if (type->fn_params) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    size_t i;
    rc = ffi_new_json_array(&arr_val, &arr);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
    for (i = 0; i < type->fn_params_count; ++i) {
      JSON_Value *f_val = NULL;
      rc = field_to_json(&type->fn_params[i], &f_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
      rc = ffi_append_val_safe(arr, f_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
    }
    rc = ffi_set_val_safe(obj, "fn_params", arr_val);
    if (rc != CDD_C_SUCCESS)
      goto err_ty;
  }
  if (type->fn_is_variadic) {
    SET_INT(obj, "fn_is_variadic", type->fn_is_variadic);
  }
  *out_val = val;
  return CDD_C_SUCCESS;

err_ty:;
  return cdd_c_keep_err(rc, cdd_json_value_free(val));
}

/**
 * @brief Serializes a `cdd_ffi_field_t` structure into a JSON object.
 *
 * @param[in] field FFI field or parameter descriptor.
 * @param[out] out_val Output pointer receiving the allocated `JSON_Value *`.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t field_to_json(const cdd_ffi_field_t *field,
                                   JSON_Value **out_val) {
  JSON_Value *val = NULL;
  JSON_Object *obj = NULL;
  JSON_Value *t_val = NULL;
  cdd_c_error_t rc = ffi_new_json_object(&val, &obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  SET_STR(obj, "name", field->name);
  rc = type_to_json(&field->type, &t_val);
  if (rc != CDD_C_SUCCESS)
    goto err_fld;
  rc = ffi_set_val_safe(obj, "type", t_val);
  if (rc != CDD_C_SUCCESS)
    goto err_fld;

  SET_STR(obj, "doc", field->doc);
  SET_INT(obj, "intent", (int)field->intent);
  SET_STR(obj, "array_length_ref", field->array_length_ref);
  if (field->leading_trivia) {
    rc = trivia_to_json(field->leading_trivia, &t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_fld;
    rc = ffi_set_val_safe(obj, "leading_trivia", t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_fld;
  }
  if (field->trailing_trivia) {
    rc = trivia_to_json(field->trailing_trivia, &t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_fld;
    rc = ffi_set_val_safe(obj, "trailing_trivia", t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_fld;
  }
  *out_val = val;
  return CDD_C_SUCCESS;

err_fld:;
  return cdd_c_keep_err(rc, cdd_json_value_free(val));
}

/**
 * @brief Serializes a `cdd_ffi_ir_node_t` structure into a JSON object.
 *
 * @param[in] node FFI IR node to serialize.
 * @param[out] out_val Output pointer receiving the allocated `JSON_Value *`.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t node_to_json(const cdd_ffi_ir_node_t *node,
                                  JSON_Value **out_val) {
  JSON_Value *val = NULL;
  JSON_Object *obj = NULL;
  size_t i;
  cdd_c_error_t rc = ffi_new_json_object(&val, &obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  SET_INT(obj, "kind", (int)node->kind);
  SET_STR(obj, "name", node->name);
  SET_STR(obj, "doc", node->doc);

  if (node->fields) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    rc = ffi_new_json_array(&arr_val, &arr);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
    for (i = 0; i < node->fields_count; ++i) {
      JSON_Value *f_val = NULL;
      rc = field_to_json(&node->fields[i], &f_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
      rc = ffi_append_val_safe(arr, f_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
    }
    rc = ffi_set_val_safe(obj, "fields", arr_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
  }

  if (node->base_classes) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    rc = ffi_new_json_array(&arr_val, &arr);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
    for (i = 0; i < node->base_classes_count; ++i) {
      JSON_Value *bc_val = NULL;
      JSON_Object *bc_obj = NULL;
      rc = ffi_new_json_object(&bc_val, &bc_obj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
      rc = cdd_reflect_to_json_object(&node->base_classes[i],
                                      &g_cdd_ffi_base_class_type_info, bc_obj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(
            rc, cdd_c_keep_err(cdd_json_value_free(bc_val),
                               cdd_c_keep_err(cdd_json_value_free(arr_val),
                                              cdd_json_value_free(val))));
      }
      rc = ffi_append_val_safe(arr, bc_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
    }
    rc = ffi_set_val_safe(obj, "base_classes", arr_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
  }

  if (node->variants) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    rc = ffi_new_json_array(&arr_val, &arr);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
    for (i = 0; i < node->variants_count; ++i) {
      JSON_Value *v_val = NULL;
      JSON_Object *v_obj = NULL;
      rc = ffi_new_json_object(&v_val, &v_obj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
      rc = set_string(v_obj, "name", node->variants[i].name);
      if (rc != CDD_C_SUCCESS)
        goto err_var;
      rc = set_string(v_obj, "value", node->variants[i].value);
      if (rc != CDD_C_SUCCESS)
        goto err_var;
      rc = set_string(v_obj, "doc", node->variants[i].doc);
      if (rc != CDD_C_SUCCESS)
        goto err_var;
      if (node->variants[i].number_details) {
        JSON_Value *n_val = NULL;
        rc = number_to_json(node->variants[i].number_details, &n_val);
        if (rc != CDD_C_SUCCESS)
          goto err_var;
        rc = ffi_set_val_safe(v_obj, "number_details", n_val);
        if (rc != CDD_C_SUCCESS)
          goto err_var;
      }
      if (node->variants[i].leading_trivia) {
        JSON_Value *t_val = NULL;
        rc = trivia_to_json(node->variants[i].leading_trivia, &t_val);
        if (rc != CDD_C_SUCCESS)
          goto err_var;
        rc = ffi_set_val_safe(v_obj, "leading_trivia", t_val);
        if (rc != CDD_C_SUCCESS)
          goto err_var;
      }
      if (node->variants[i].trailing_trivia) {
        JSON_Value *t_val = NULL;
        rc = trivia_to_json(node->variants[i].trailing_trivia, &t_val);
        if (rc != CDD_C_SUCCESS)
          goto err_var;
        rc = ffi_set_val_safe(v_obj, "trailing_trivia", t_val);
        if (rc != CDD_C_SUCCESS)
          goto err_var;
      }
      rc = set_string(v_obj, "raw_spelling", node->variants[i].raw_spelling);
      if (rc != CDD_C_SUCCESS)
        goto err_var;
      rc = ffi_append_val_safe(arr, v_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
      continue;
    err_var:;
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(cdd_json_value_free(v_val),
                             cdd_c_keep_err(cdd_json_value_free(arr_val),
                                            cdd_json_value_free(val))));
    }
    rc = ffi_set_val_safe(obj, "variants", arr_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
  }

  if (node->macro_expansions) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    rc = ffi_new_json_array(&arr_val, &arr);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
    for (i = 0; i < node->macro_expansions_count; ++i) {
      JSON_Value *m_val = NULL;
      rc = macro_expansion_to_json(&node->macro_expansions[i], &m_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
      rc = ffi_append_val_safe(arr, m_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(arr_val),
                                                 cdd_json_value_free(val)));
      }
    }
    rc = ffi_set_val_safe(obj, "macro_expansions", arr_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
  }

  if (node->kind == CDD_FFI_NODE_FUNCTION ||
      node->kind == CDD_FFI_NODE_TYPEDEF) {
    JSON_Value *t_val = NULL;
    rc = type_to_json(&node->return_or_base_type, &t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
    rc = ffi_set_val_safe(obj, "return_or_base_type", t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
  }

  SET_STR(obj, "evaluated_value", node->evaluated_value);
  SET_INT(obj, "inferred_type", (int)node->inferred_type);
  SET_INT(obj, "requires_gil_release", node->requires_gil_release);
  SET_INT(obj, "is_variadic", node->is_variadic);

  if (node->leading_trivia) {
    JSON_Value *t_val = NULL;
    rc = trivia_to_json(node->leading_trivia, &t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
    rc = ffi_set_val_safe(obj, "leading_trivia", t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
  }
  if (node->trailing_trivia) {
    JSON_Value *t_val = NULL;
    rc = trivia_to_json(node->trailing_trivia, &t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
    rc = ffi_set_val_safe(obj, "trailing_trivia", t_val);
    if (rc != CDD_C_SUCCESS)
      goto err_node;
  }
  SET_STR(obj, "raw_body", node->raw_body);
  *out_val = val;
  return CDD_C_SUCCESS;

err_node:;
  return cdd_c_keep_err(rc, cdd_json_value_free(val));
}

/*
 * @brief Serializes an entire `cdd_ffi_ir_t` into a pretty-printed JSON string.
 *
 * @param[in] ir FFI IR structure to serialize.
 * @param[out] out_json Output pointer receiving the newly allocated JSON
 * string.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_ir_to_json(const cdd_ffi_ir_t *ir,
                                              char **out_json) {
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Value *nodes_val = NULL;
  JSON_Array *nodes_arr = NULL;
  size_t i;
  char *serialized = NULL;
  cdd_c_error_t rc;

  if (!ir || !out_json) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = ffi_new_json_object(&root_val, &root_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = ffi_new_json_array(&nodes_val, &nodes_arr);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  }

  for (i = 0; i < ir->nodes_count; ++i) {
    JSON_Value *n_val = NULL;
    rc = node_to_json(&ir->nodes[i], &n_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(nodes_val),
                                               cdd_json_value_free(root_val)));
    }
    rc = ffi_append_val_safe(nodes_arr, n_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(nodes_val),
                                               cdd_json_value_free(root_val)));
    }
  }

  rc = ffi_set_val_safe(root_obj, "nodes", nodes_val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  }

  rc = cdd_json_serialize_to_string_pretty(root_val, &serialized);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  }
  *out_json = serialized;
  return cdd_json_value_free(root_val);
}

/**
 * @brief Duplicates an optional JSON string property `name` from `obj` into
 * `*out`.
 *
 * @param[in] obj JSON object to query.
 * @param[in] name Property name.
 * @param[out] out Output heap string pointer (or NULL if absent).
 * @return `CDD_C_SUCCESS` on success, or error code on OOM/fault injection.
 */
static cdd_c_error_t dup_str_opt(const JSON_Object *obj, const char *name,
                                 char **out) {
  const char *in = NULL;
  cdd_c_error_t rc = get_str_opt(obj, name, &in);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!in) {
    *out = NULL;
    return CDD_C_SUCCESS;
  }
  *out = C_CDD_STRDUP(in);
  return *out ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Frees a heap-allocated `cdd_ffi_ir_t` instance and its child nodes.
 *
 * @param[in,out] ir Heap IR pointer.
 * @return Cleanup status.
 */
static cdd_c_error_t ffi_ir_heap_free(cdd_ffi_ir_t *ir) {
  cdd_c_error_t rc = cdd_ffi_ir_free(ir);
  C_CDD_FREE(ir);
  return rc;
}

/**
 * @brief Deserializes a JSON array of trivia objects into a linked list of
 * `cdd_ffi_trivia_t`.
 *
 * @param[in] arr Source JSON array (may be NULL).
 * @param[out] out Output pointer receiving the head of the trivia list.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM/fault injection.
 */
static cdd_c_error_t parse_trivia(JSON_Array *arr, cdd_ffi_trivia_t **out) {
  size_t i, count = 0;
  cdd_ffi_trivia_t *head = NULL, *tail = NULL, *node = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_SUCCESS;
  rc = cdd_json_array_get_count(arr, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < count; ++i) {
    JSON_Object *obj = NULL;
    double d_val = 0.0;
    rc = ffi_arr_obj_opt(arr, i, &obj);
    if (rc != CDD_C_SUCCESS) {
      *out = head;
      return rc;
    }
    if (!obj)
      continue;
    node = C_CDD_CALLOC(1, sizeof(cdd_ffi_trivia_t));
    if (!node) {
      *out = head;
      return CDD_C_ERROR_MEMORY;
    }
    rc = get_num_opt(obj, "kind", &d_val);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(node);
      *out = head;
      return rc;
    }
    node->kind = (cdd_ffi_trivia_kind_t)d_val;
    rc = dup_str_opt(obj, "text", &node->text);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(node);
      *out = head;
      return rc;
    }
    if (!head) {
      head = node;
      tail = node;
    } else {
      tail->next = node;
      tail = node;
    }
  }
  *out = head;
  return CDD_C_SUCCESS;
}

/**
 * @brief Deserializes a JSON object into a newly allocated `cdd_ffi_number_t`.
 *
 * @param[in] obj Source JSON object (may be NULL).
 * @param[out] out Output pointer receiving the allocated `cdd_ffi_number_t`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_number(JSON_Object *obj,
                                  struct cdd_ffi_number_t **out) {
  struct cdd_ffi_number_t *num;
  cdd_c_error_t rc;
  if (!obj)
    return CDD_C_SUCCESS;
  num = C_CDD_CALLOC(1, sizeof(struct cdd_ffi_number_t));
  if (!num)
    return CDD_C_ERROR_MEMORY;
  rc = cdd_reflect_from_json_object(obj, &g_cdd_ffi_number_type_info, num);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(num->raw_spelling);
    C_CDD_FREE(num);
    return rc;
  }
  *out = num;
  return CDD_C_SUCCESS;
}

/**
 * @brief Deserializes a JSON object into a `cdd_ffi_macro_expansion_t`.
 *
 * @param[in] obj Source JSON object.
 * @param[out] m Destination macro expansion structure.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t
parse_macro_expansion(JSON_Object *obj, struct cdd_ffi_macro_expansion_t *m) {
  if (!obj)
    return CDD_C_SUCCESS;
  return cdd_reflect_from_json_object(obj, &g_cdd_ffi_macro_expansion_type_info,
                                      m);
}

static cdd_c_error_t parse_field(JSON_Object *obj, cdd_ffi_field_t *field);

/**
 * @brief Deserializes a JSON object into a `cdd_ffi_type_t` structure.
 *
 * @param[in] obj Source JSON object (may be NULL).
 * @param[out] type Destination FFI type structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_type(JSON_Object *obj, cdd_ffi_type_t *type) {
  JSON_Array *t_arr = NULL;
  JSON_Array *tr_arr = NULL;
  JSON_Object *fn_ret_obj = NULL;
  double d_val = 0.0;
  int has_v = 0;
  size_t i;
  cdd_c_error_t rc;

  memset(type, 0, sizeof(cdd_ffi_type_t));
  type->array_size = -1;

  if (!obj)
    return CDD_C_SUCCESS;

  rc = get_num_if_present(obj, "kind", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    type->kind = (cdd_ffi_primitive_kind_t)d_val;

  rc = get_num_if_present(obj, "is_const", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    type->is_const = (int)d_val;

  rc = get_num_if_present(obj, "pointer_depth", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    type->pointer_depth = (int)d_val;

  rc = get_num_if_present(obj, "array_size", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    type->array_size = (long)d_val;

  rc = dup_str_opt(obj, "ref_name", &type->ref_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = get_arr_opt(obj, "template_args", &t_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (t_arr) {
    size_t count = 0;
    rc = cdd_json_array_get_count(t_arr, &count);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (count > 0) {
      type->template_args = C_CDD_CALLOC(count, sizeof(cdd_ffi_type_t));
      if (!type->template_args)
        return CDD_C_ERROR_MEMORY;
      type->template_args_count = count;
      for (i = 0; i < count; ++i) {
        JSON_Object *sub_obj = NULL;
        rc = ffi_arr_obj_opt(t_arr, i, &sub_obj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_type(sub_obj, &type->template_args[i]);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  rc = get_arr_opt(obj, "leading_trivia", &tr_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_trivia(tr_arr, &type->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = get_arr_opt(obj, "trailing_trivia", &tr_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_trivia(tr_arr, &type->trailing_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = dup_str_opt(obj, "raw_spelling", &type->raw_spelling);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = get_obj_opt(obj, "fn_return_type", &fn_ret_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (fn_ret_obj) {
    type->fn_return_type = C_CDD_CALLOC(1, sizeof(cdd_ffi_type_t));
    if (!type->fn_return_type)
      return CDD_C_ERROR_MEMORY;
    rc = parse_type(fn_ret_obj, type->fn_return_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = get_arr_opt(obj, "fn_params", &t_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (t_arr) {
    size_t count = 0;
    rc = cdd_json_array_get_count(t_arr, &count);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (count > 0) {
      type->fn_params = C_CDD_CALLOC(count, sizeof(cdd_ffi_field_t));
      if (!type->fn_params)
        return CDD_C_ERROR_MEMORY;
      type->fn_params_count = count;
      for (i = 0; i < count; ++i) {
        JSON_Object *sub_obj = NULL;
        rc = ffi_arr_obj_opt(t_arr, i, &sub_obj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_field(sub_obj, &type->fn_params[i]);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  rc = get_num_if_present(obj, "fn_is_variadic", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    type->fn_is_variadic = (int)d_val;

  return CDD_C_SUCCESS;
}

/**
 * @brief Deserializes a JSON object into a `cdd_ffi_field_t` structure.
 *
 * @param[in] obj Source JSON object.
 * @param[out] field Destination FFI field structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_field(JSON_Object *obj, cdd_ffi_field_t *field) {
  double d_val = 0.0;
  int has_v = 0;
  JSON_Object *sub_obj = NULL;
  JSON_Array *tr_arr = NULL;
  cdd_c_error_t rc;
  memset(field, 0, sizeof(cdd_ffi_field_t));
  if (!obj)
    return CDD_C_SUCCESS;

  rc = dup_str_opt(obj, "name", &field->name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_str_opt(obj, "doc", &field->doc);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_str_opt(obj, "array_length_ref", &field->array_length_ref);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = get_num_if_present(obj, "intent", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    field->intent = (cdd_ffi_param_intent_t)d_val;

  rc = get_obj_opt(obj, "type", &sub_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_type(sub_obj, &field->type);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = get_arr_opt(obj, "leading_trivia", &tr_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_trivia(tr_arr, &field->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = get_arr_opt(obj, "trailing_trivia", &tr_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_trivia(tr_arr, &field->trailing_trivia);
}

/**
 * @brief Deserializes a JSON object into a `cdd_ffi_ir_node_t` structure.
 *
 * @param[in] obj Source JSON object.
 * @param[out] node Destination FFI IR node structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_node(JSON_Object *obj, cdd_ffi_ir_node_t *node) {
  cdd_c_error_t rc;
  JSON_Array *arr = NULL;
  JSON_Object *rt_obj = NULL;
  double d_val = 0.0;
  int has_v = 0;
  size_t i;

  memset(node, 0, sizeof(cdd_ffi_ir_node_t));
  if (!obj)
    return CDD_C_SUCCESS;

  rc = get_num_if_present(obj, "kind", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    node->kind = (cdd_ffi_node_kind_t)d_val;

  rc = dup_str_opt(obj, "name", &node->name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = dup_str_opt(obj, "doc", &node->doc);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = get_arr_opt(obj, "fields", &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (arr) {
    size_t count = 0;
    rc = cdd_json_array_get_count(arr, &count);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (count > 0) {
      node->fields = C_CDD_CALLOC(count, sizeof(cdd_ffi_field_t));
      if (!node->fields)
        return CDD_C_ERROR_MEMORY;
      node->fields_count = count;
      for (i = 0; i < count; ++i) {
        JSON_Object *f_obj = NULL;
        rc = ffi_arr_obj_opt(arr, i, &f_obj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_field(f_obj, &node->fields[i]);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  rc = get_arr_opt(obj, "base_classes", &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (arr) {
    size_t count = 0;
    rc = cdd_json_array_get_count(arr, &count);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (count > 0) {
      node->base_classes = C_CDD_CALLOC(count, sizeof(cdd_ffi_base_class_t));
      if (!node->base_classes)
        return CDD_C_ERROR_MEMORY;
      node->base_classes_count = count;
      for (i = 0; i < count; ++i) {
        JSON_Object *bc_obj = NULL;
        rc = ffi_arr_obj_opt(arr, i, &bc_obj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (bc_obj) {
          rc = cdd_reflect_from_json_object(
              bc_obj, &g_cdd_ffi_base_class_type_info, &node->base_classes[i]);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  rc = get_arr_opt(obj, "variants", &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (arr) {
    size_t count = 0;
    rc = cdd_json_array_get_count(arr, &count);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (count > 0) {
      node->variants = C_CDD_CALLOC(count, sizeof(cdd_ffi_enum_variant_t));
      if (!node->variants)
        return CDD_C_ERROR_MEMORY;
      node->variants_count = count;
      for (i = 0; i < count; ++i) {
        JSON_Object *v_obj = NULL;
        JSON_Object *nd_obj = NULL;
        JSON_Array *tr_arr = NULL;
        rc = ffi_arr_obj_opt(arr, i, &v_obj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (!v_obj)
          continue;
        rc = dup_str_opt(v_obj, "name", &node->variants[i].name);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = dup_str_opt(v_obj, "value", &node->variants[i].value);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = dup_str_opt(v_obj, "doc", &node->variants[i].doc);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc =
            dup_str_opt(v_obj, "raw_spelling", &node->variants[i].raw_spelling);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = get_obj_opt(v_obj, "number_details", &nd_obj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_number(nd_obj, &node->variants[i].number_details);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = get_arr_opt(v_obj, "leading_trivia", &tr_arr);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_trivia(tr_arr, &node->variants[i].leading_trivia);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = get_arr_opt(v_obj, "trailing_trivia", &tr_arr);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_trivia(tr_arr, &node->variants[i].trailing_trivia);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  rc = get_arr_opt(obj, "macro_expansions", &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (arr) {
    size_t count = 0;
    rc = cdd_json_array_get_count(arr, &count);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (count > 0) {
      node->macro_expansions =
          C_CDD_CALLOC(count, sizeof(struct cdd_ffi_macro_expansion_t));
      if (!node->macro_expansions)
        return CDD_C_ERROR_MEMORY;
      node->macro_expansions_count = count;
      for (i = 0; i < count; ++i) {
        JSON_Object *m_obj = NULL;
        rc = ffi_arr_obj_opt(arr, i, &m_obj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_macro_expansion(m_obj, &node->macro_expansions[i]);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  rc = get_obj_opt(obj, "return_or_base_type", &rt_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (rt_obj) {
    rc = parse_type(rt_obj, &node->return_or_base_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = dup_str_opt(obj, "evaluated_value", &node->evaluated_value);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = get_num_if_present(obj, "inferred_type", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    node->inferred_type = (cdd_ffi_macro_type_t)d_val;
  rc = get_num_if_present(obj, "requires_gil_release", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    node->requires_gil_release = (int)d_val;
  rc = get_num_if_present(obj, "is_variadic", &d_val, &has_v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_v)
    node->is_variadic = (int)d_val;

  rc = get_arr_opt(obj, "leading_trivia", &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_trivia(arr, &node->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = get_arr_opt(obj, "trailing_trivia", &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_trivia(arr, &node->trailing_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return dup_str_opt(obj, "raw_body", &node->raw_body);
}

/*
 * @brief Deserializes a JSON string into a newly allocated `cdd_ffi_ir_t`
 * structure.
 *
 * @param[in] json_str Raw JSON string representing a serialized FFI IR.
 * @param[out] out_ir Output pointer receiving the newly allocated FFI IR
 * structure.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_PARSE` on malformed JSON, or
 * `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_ir_from_json(const char *json_str,
                                                cdd_ffi_ir_t **out_ir) {
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Array *nodes_arr = NULL;
  cdd_ffi_ir_t *ir = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!json_str || !out_ir) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_json_parse_string(json_str, &root_val);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (root_val->type != JSONObject) {
    return cdd_c_keep_err(CDD_C_ERROR_PARSE, cdd_json_value_free(root_val));
  }
  root_obj = root_val->object_val;

  ir = C_CDD_CALLOC(1, sizeof(cdd_ffi_ir_t));
  if (!ir) {
    return cdd_c_keep_err(CDD_C_ERROR_MEMORY, cdd_json_value_free(root_val));
  }

  rc = get_arr_opt(root_obj, "nodes", &nodes_arr);
  if (rc != CDD_C_SUCCESS)
    goto err_ir;
  if (nodes_arr) {
    size_t count = 0;
    rc = cdd_json_array_get_count(nodes_arr, &count);
    if (rc != CDD_C_SUCCESS)
      goto err_ir;
    if (count > 0) {
      ir->nodes = C_CDD_CALLOC(count, sizeof(cdd_ffi_ir_node_t));
      if (!ir->nodes) {
        rc = CDD_C_ERROR_MEMORY;
        goto err_ir;
      }
      ir->nodes_count = count;
      ir->nodes_capacity = count;

      for (i = 0; i < count; ++i) {
        JSON_Object *n_obj = NULL;
        rc = ffi_arr_obj_opt(nodes_arr, i, &n_obj);
        if (rc != CDD_C_SUCCESS)
          goto err_ir;
        rc = parse_node(n_obj, &ir->nodes[i]);
        if (rc != CDD_C_SUCCESS)
          goto err_ir;
      }
    }
  }

  *out_ir = ir;
  return cdd_json_value_free(root_val);

err_ir:;
  return cdd_c_keep_err(
      rc, cdd_c_keep_err(ffi_ir_heap_free(ir), cdd_json_value_free(root_val)));
}
