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

/** @brief Sets a string property on a JSON object or returns CDD_C_ERROR_MEMORY
 */
#define SET_STR(o, k, v)                                                       \
  do {                                                                         \
    if (set_string((o), (k), (v)) != CDD_C_SUCCESS) {                          \
      cdd_json_value_free(val);                                                \
      return CDD_C_ERROR_MEMORY;                                               \
    }                                                                          \
  } while (0)
/** @brief Sets an integer property on a JSON object or returns
 * CDD_C_ERROR_MEMORY */
#define SET_INT(o, k, v)                                                       \
  do {                                                                         \
    if (set_int((o), (k), (v)) != CDD_C_SUCCESS) {                             \
      cdd_json_value_free(val);                                                \
      return CDD_C_ERROR_MEMORY;                                               \
    }                                                                          \
  } while (0)

/**
 * @brief Helper to read a number property from a JSON_Object with 0.0 default.
 *
 * @param[in] obj JSON_Object to read from.
 * @param[in] name Key name.
 * @param[out] out_num Output number.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t get_num_opt(const JSON_Object *obj, const char *name,
                                 double *out_num) {
  *out_num = 0.0;
  cdd_json_object_get_number(obj, name, out_num);
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to read a string property from a JSON_Object with NULL default.
 *
 * @param[in] obj JSON_Object to read from.
 * @param[in] name Key name.
 * @param[out] out_str Output string pointer.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t get_str_opt(const JSON_Object *obj, const char *name,
                                 const char **out_str) {
  *out_str = NULL;
  cdd_json_object_get_string(obj, name, out_str);
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to read an object property from a JSON_Object with NULL
 * default.
 *
 * @param[in] obj JSON_Object to read from.
 * @param[in] name Key name.
 * @param[out] out_obj Output JSON_Object pointer.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t get_obj_opt(const JSON_Object *obj, const char *name,
                                 JSON_Object **out_obj) {
  *out_obj = NULL;
  cdd_json_object_get_object(obj, name, out_obj);
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to read an array property from a JSON_Object with NULL default.
 *
 * @param[in] obj JSON_Object to read from.
 * @param[in] name Key name.
 * @param[out] out_arr Output JSON_Array pointer.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t get_arr_opt(const JSON_Object *obj, const char *name,
                                 JSON_Array **out_arr) {
  *out_arr = NULL;
  cdd_json_object_get_array(obj, name, out_arr);
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to check if a JSON_Object has a property.
 *
 * @param[in] obj JSON_Object to inspect.
 * @param[in] name Key name.
 * @param[out] out_has Output 1 if present, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t has_val_opt(const JSON_Object *obj, const char *name,
                                 int *out_has) {
  *out_has = 0;
  cdd_json_object_has_value(obj, name, out_has);
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets a string property on a Parson `JSON_Object` if `val` is non-NULL.
 *
 * @param[in,out] obj Target JSON object.
 * @param[in] name Property key name.
 * @param[in] val String value to set (ignored if NULL).
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_UNKNOWN` on failure.
 */
static cdd_c_error_t set_string(JSON_Object *obj, const char *name,
                                const char *val) {
  if (val) {
    if (cdd_json_object_set_string(obj, name, val) != CDD_C_SUCCESS) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets an integer property on a Parson `JSON_Object`.
 *
 * @param[in,out] obj Target JSON object.
 * @param[in] name Property key name.
 * @param[in] val Integer value to set.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_UNKNOWN` on failure.
 */
static cdd_c_error_t set_int(JSON_Object *obj, const char *name, int val) {
  if (cdd_json_object_set_number(obj, name, (double)val) != CDD_C_SUCCESS) {
    return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Serializes a linked list of `cdd_ffi_trivia_t` into a JSON array.
 *
 * @param[in] trivia Head of trivia linked list.
 * @param[out] out_val Output pointer receiving the allocated `JSON_Value *`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
static cdd_c_error_t trivia_to_json(const cdd_ffi_trivia_t *trivia,
                                    JSON_Value **out_val) {
  JSON_Value *val = NULL;
  JSON_Array *arr = NULL;
  const cdd_ffi_trivia_t *curr = trivia;
  if (cdd_json_value_init_array(&val) != CDD_C_SUCCESS)
    return CDD_C_ERROR_MEMORY;
  cdd_json_value_get_array(val, &arr);
  while (curr) {
    JSON_Value *t_val = NULL;
    JSON_Object *t_obj = NULL;
    if (cdd_json_value_init_object(&t_val) != CDD_C_SUCCESS) {
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    cdd_json_value_get_object(t_val, &t_obj);
    if (set_int(t_obj, "kind", (int)curr->kind) != CDD_C_SUCCESS ||
        set_string(t_obj, "text", curr->text) != CDD_C_SUCCESS ||
        cdd_json_array_append_value(arr, t_val) != CDD_C_SUCCESS) {
      cdd_json_value_free(t_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    curr = curr->next;
  }
  *out_val = val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Serializes a `cdd_ffi_number_t` structure into a JSON object.
 *
 * @param[in] num Numeric literal metadata structure.
 * @param[out] out_val Output pointer receiving the allocated `JSON_Value *`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
static cdd_c_error_t number_to_json(const struct cdd_ffi_number_t *num,
                                    JSON_Value **out_val) {
  char *jstr = NULL;
  cdd_c_error_t rc =
      cdd_reflect_to_json(num, &g_cdd_ffi_number_type_info, &jstr);
  if (rc != CDD_C_SUCCESS)
    return CDD_C_ERROR_MEMORY;
  rc = cdd_json_parse_string(jstr, out_val);
  C_CDD_FREE(jstr);
  return rc == CDD_C_SUCCESS ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Serializes a `cdd_ffi_macro_expansion_t` structure into a JSON object.
 *
 * @param[in] m Macro expansion metadata structure.
 * @param[out] out_val Output pointer receiving the allocated `JSON_Value *`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
static cdd_c_error_t
macro_expansion_to_json(const struct cdd_ffi_macro_expansion_t *m,
                        JSON_Value **out_val) {
  char *jstr = NULL;
  cdd_c_error_t rc =
      cdd_reflect_to_json(m, &g_cdd_ffi_macro_expansion_type_info, &jstr);
  if (rc != CDD_C_SUCCESS)
    return CDD_C_ERROR_MEMORY;
  rc = cdd_json_parse_string(jstr, out_val);
  C_CDD_FREE(jstr);
  return rc == CDD_C_SUCCESS ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

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
  if (cdd_json_value_init_object(&val) != CDD_C_SUCCESS)
    return CDD_C_ERROR_MEMORY;
  cdd_json_value_get_object(val, &obj);

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
    if (cdd_json_value_init_array(&arr_val) != CDD_C_SUCCESS) {
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    cdd_json_value_get_array(arr_val, &arr);
    for (i = 0; i < type->template_args_count; ++i) {
      JSON_Value *t_val = NULL;
      cdd_c_error_t rc = type_to_json(&type->template_args[i], &t_val);
      if (rc != CDD_C_SUCCESS ||
          cdd_json_array_append_value(arr, t_val) != CDD_C_SUCCESS) {
        if (t_val)
          cdd_json_value_free(t_val);
        cdd_json_value_free(arr_val);
        cdd_json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (cdd_json_object_set_value(obj, "template_args", arr_val) !=
        CDD_C_SUCCESS) {
      cdd_json_value_free(arr_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }
  if (type->leading_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(type->leading_trivia, &t_val);
    if (rc != CDD_C_SUCCESS ||
        cdd_json_object_set_value(obj, "leading_trivia", t_val) !=
            CDD_C_SUCCESS) {
      if (t_val)
        cdd_json_value_free(t_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }
  if (type->trailing_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(type->trailing_trivia, &t_val);
    if (rc != CDD_C_SUCCESS ||
        cdd_json_object_set_value(obj, "trailing_trivia", t_val) !=
            CDD_C_SUCCESS) {
      if (t_val)
        cdd_json_value_free(t_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }
  SET_STR(obj, "raw_spelling", type->raw_spelling);
  *out_val = val;
  return CDD_C_SUCCESS;
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
  if (cdd_json_value_init_object(&val) != CDD_C_SUCCESS)
    return CDD_C_ERROR_MEMORY;
  cdd_json_value_get_object(val, &obj);

  SET_STR(obj, "name", field->name);
  {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = type_to_json(&field->type, &t_val);
    if (rc != CDD_C_SUCCESS ||
        cdd_json_object_set_value(obj, "type", t_val) != CDD_C_SUCCESS) {
      if (t_val)
        cdd_json_value_free(t_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }
  SET_STR(obj, "doc", field->doc);
  SET_INT(obj, "intent", (int)field->intent);
  SET_STR(obj, "array_length_ref", field->array_length_ref);
  if (field->leading_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(field->leading_trivia, &t_val);
    if (rc != CDD_C_SUCCESS ||
        cdd_json_object_set_value(obj, "leading_trivia", t_val) !=
            CDD_C_SUCCESS) {
      if (t_val)
        cdd_json_value_free(t_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }
  if (field->trailing_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(field->trailing_trivia, &t_val);
    if (rc != CDD_C_SUCCESS ||
        cdd_json_object_set_value(obj, "trailing_trivia", t_val) !=
            CDD_C_SUCCESS) {
      if (t_val)
        cdd_json_value_free(t_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }
  *out_val = val;
  return CDD_C_SUCCESS;
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

  if (cdd_json_value_init_object(&val) != CDD_C_SUCCESS)
    return CDD_C_ERROR_MEMORY;
  cdd_json_value_get_object(val, &obj);

  SET_INT(obj, "kind", (int)node->kind);
  SET_STR(obj, "name", node->name);
  SET_STR(obj, "doc", node->doc);

  if (node->fields) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    if (cdd_json_value_init_array(&arr_val) != CDD_C_SUCCESS) {
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    cdd_json_value_get_array(arr_val, &arr);
    for (i = 0; i < node->fields_count; ++i) {
      JSON_Value *f_val = NULL;
      cdd_c_error_t rc = field_to_json(&node->fields[i], &f_val);
      if (rc != CDD_C_SUCCESS ||
          cdd_json_array_append_value(arr, f_val) != CDD_C_SUCCESS) {
        if (f_val)
          cdd_json_value_free(f_val);
        cdd_json_value_free(arr_val);
        cdd_json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (cdd_json_object_set_value(obj, "fields", arr_val) != CDD_C_SUCCESS) {
      cdd_json_value_free(arr_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (node->base_classes) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    if (cdd_json_value_init_array(&arr_val) != CDD_C_SUCCESS) {
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    cdd_json_value_get_array(arr_val, &arr);
    for (i = 0; i < node->base_classes_count; ++i) {
      JSON_Value *bc_val = NULL;
      char *bc_str = NULL;
      if (cdd_reflect_to_json(&node->base_classes[i],
                              &g_cdd_ffi_base_class_type_info,
                              &bc_str) != CDD_C_SUCCESS ||
          cdd_json_parse_string(bc_str, &bc_val) != CDD_C_SUCCESS) {
        if (bc_str)
          C_CDD_FREE(bc_str);
        cdd_json_value_free(arr_val);
        cdd_json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
      C_CDD_FREE(bc_str);
      if (cdd_json_array_append_value(arr, bc_val) != CDD_C_SUCCESS) {
        cdd_json_value_free(bc_val);
        cdd_json_value_free(arr_val);
        cdd_json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (cdd_json_object_set_value(obj, "base_classes", arr_val) !=
        CDD_C_SUCCESS) {
      cdd_json_value_free(arr_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (node->variants) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    if (cdd_json_value_init_array(&arr_val) != CDD_C_SUCCESS) {
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    cdd_json_value_get_array(arr_val, &arr);
    for (i = 0; i < node->variants_count; ++i) {
      JSON_Value *v_val = NULL;
      JSON_Object *v_obj = NULL;
      if (cdd_json_value_init_object(&v_val) != CDD_C_SUCCESS) {
        cdd_json_value_free(arr_val);
        cdd_json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
      cdd_json_value_get_object(v_val, &v_obj);
      if (set_string(v_obj, "name", node->variants[i].name) != CDD_C_SUCCESS ||
          set_string(v_obj, "value", node->variants[i].value) !=
              CDD_C_SUCCESS ||
          set_string(v_obj, "doc", node->variants[i].doc) != CDD_C_SUCCESS) {
        cdd_json_value_free(v_val);
        cdd_json_value_free(arr_val);
        cdd_json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
      if (node->variants[i].number_details) {
        JSON_Value *n_val = NULL;
        cdd_c_error_t rc =
            number_to_json(node->variants[i].number_details, &n_val);
        if (rc != CDD_C_SUCCESS ||
            cdd_json_object_set_value(v_obj, "number_details", n_val) !=
                CDD_C_SUCCESS) {
          if (n_val)
            cdd_json_value_free(n_val);
          cdd_json_value_free(v_val);
          cdd_json_value_free(arr_val);
          cdd_json_value_free(val);
          return CDD_C_ERROR_MEMORY;
        }
      }
      if (node->variants[i].leading_trivia) {
        JSON_Value *t_val = NULL;
        cdd_c_error_t rc =
            trivia_to_json(node->variants[i].leading_trivia, &t_val);
        if (rc != CDD_C_SUCCESS ||
            cdd_json_object_set_value(v_obj, "leading_trivia", t_val) !=
                CDD_C_SUCCESS) {
          if (t_val)
            cdd_json_value_free(t_val);
          cdd_json_value_free(v_val);
          cdd_json_value_free(arr_val);
          cdd_json_value_free(val);
          return CDD_C_ERROR_MEMORY;
        }
      }
      if (node->variants[i].trailing_trivia) {
        JSON_Value *t_val = NULL;
        cdd_c_error_t rc =
            trivia_to_json(node->variants[i].trailing_trivia, &t_val);
        if (rc != CDD_C_SUCCESS ||
            cdd_json_object_set_value(v_obj, "trailing_trivia", t_val) !=
                CDD_C_SUCCESS) {
          if (t_val)
            cdd_json_value_free(t_val);
          cdd_json_value_free(v_val);
          cdd_json_value_free(arr_val);
          cdd_json_value_free(val);
          return CDD_C_ERROR_MEMORY;
        }
      }
      if (set_string(v_obj, "raw_spelling", node->variants[i].raw_spelling) !=
              CDD_C_SUCCESS ||
          cdd_json_array_append_value(arr, v_val) != CDD_C_SUCCESS) {
        cdd_json_value_free(v_val);
        cdd_json_value_free(arr_val);
        cdd_json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (cdd_json_object_set_value(obj, "variants", arr_val) != CDD_C_SUCCESS) {
      cdd_json_value_free(arr_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (node->macro_expansions) {
    JSON_Value *arr_val = NULL;
    JSON_Array *arr = NULL;
    if (cdd_json_value_init_array(&arr_val) != CDD_C_SUCCESS) {
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    cdd_json_value_get_array(arr_val, &arr);
    for (i = 0; i < node->macro_expansions_count; ++i) {
      JSON_Value *m_val = NULL;
      cdd_c_error_t rc =
          macro_expansion_to_json(&node->macro_expansions[i], &m_val);
      if (rc != CDD_C_SUCCESS ||
          cdd_json_array_append_value(arr, m_val) != CDD_C_SUCCESS) {
        if (m_val)
          cdd_json_value_free(m_val);
        cdd_json_value_free(arr_val);
        cdd_json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (cdd_json_object_set_value(obj, "macro_expansions", arr_val) !=
        CDD_C_SUCCESS) {
      cdd_json_value_free(arr_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (node->kind == CDD_FFI_NODE_FUNCTION ||
      node->kind == CDD_FFI_NODE_TYPEDEF) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = type_to_json(&node->return_or_base_type, &t_val);
    if (rc != CDD_C_SUCCESS ||
        cdd_json_object_set_value(obj, "return_or_base_type", t_val) !=
            CDD_C_SUCCESS) {
      if (t_val)
        cdd_json_value_free(t_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  SET_STR(obj, "evaluated_value", node->evaluated_value);
  SET_INT(obj, "inferred_type", (int)node->inferred_type);
  SET_INT(obj, "requires_gil_release", node->requires_gil_release);
  SET_INT(obj, "is_variadic", node->is_variadic);

  if (node->leading_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(node->leading_trivia, &t_val);
    if (rc != CDD_C_SUCCESS ||
        cdd_json_object_set_value(obj, "leading_trivia", t_val) !=
            CDD_C_SUCCESS) {
      if (t_val)
        cdd_json_value_free(t_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }
  if (node->trailing_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(node->trailing_trivia, &t_val);
    if (rc != CDD_C_SUCCESS ||
        cdd_json_object_set_value(obj, "trailing_trivia", t_val) !=
            CDD_C_SUCCESS) {
      if (t_val)
        cdd_json_value_free(t_val);
      cdd_json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
  }
  SET_STR(obj, "raw_body", node->raw_body);
  *out_val = val;
  return CDD_C_SUCCESS;
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

  if (!ir || !out_json) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (cdd_json_value_init_object(&root_val) != CDD_C_SUCCESS)
    return CDD_C_ERROR_UNKNOWN;
  cdd_json_value_get_object(root_val, &root_obj);
  if (cdd_json_value_init_array(&nodes_val) != CDD_C_SUCCESS) {
    cdd_json_value_free(root_val);
    return CDD_C_ERROR_UNKNOWN;
  }
  cdd_json_value_get_array(nodes_val, &nodes_arr);

  for (i = 0; i < ir->nodes_count; ++i) {
    JSON_Value *n_val = NULL;
    cdd_c_error_t rc = node_to_json(&ir->nodes[i], &n_val);
    if (rc != CDD_C_SUCCESS ||
        cdd_json_array_append_value(nodes_arr, n_val) != CDD_C_SUCCESS) {
      if (n_val)
        cdd_json_value_free(n_val);
      cdd_json_value_free(nodes_val);
      cdd_json_value_free(root_val);
      return rc != CDD_C_SUCCESS ? rc : CDD_C_ERROR_MEMORY;
    }
  }

  if (cdd_json_object_set_value(root_obj, "nodes", nodes_val) !=
      CDD_C_SUCCESS) {
    cdd_json_value_free(nodes_val);
    cdd_json_value_free(root_val);
    return CDD_C_ERROR_UNKNOWN;
  }

  if (cdd_json_serialize_to_string_pretty(root_val, &serialized) !=
      CDD_C_SUCCESS) {
    cdd_json_value_free(root_val);
    return CDD_C_ERROR_UNKNOWN;
  }
  cdd_json_value_free(root_val);

  *out_json = serialized;
  return CDD_C_SUCCESS;
}

/**
 * @brief Safely duplicates a nullable C string.
 *
 * @param[in] in Input string (may be NULL).
 * @param[out] out Output pointer receiving the duplicated string (or NULL if
 * `in` is NULL).
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t strdup_safe(const char *in, char **out) {
  if (!in) {
    *out = NULL;
    return CDD_C_SUCCESS;
  }
  return (*out = C_CDD_STRDUP(in)) ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Deserializes a JSON array of trivia objects into a linked list of
 * `cdd_ffi_trivia_t`.
 *
 * @param[in] arr Source JSON array (may be NULL).
 * @param[out] out Output pointer receiving the head of the trivia list.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_trivia(JSON_Array *arr, cdd_ffi_trivia_t **out) {
  size_t i, count = 0;
  cdd_ffi_trivia_t *head = NULL, *tail = NULL, *node = NULL;
  if (!arr)
    return CDD_C_SUCCESS;
  cdd_json_array_get_count(arr, &count);
  for (i = 0; i < count; ++i) {
    JSON_Object *obj = NULL;
    double d_val = 0.0;
    const char *s_val = NULL;
    cdd_json_array_get_object(arr, i, &obj);
    if (!obj)
      continue;
    node = C_CDD_CALLOC(1, sizeof(cdd_ffi_trivia_t));
    if (!node) {
      *out = head;
      return CDD_C_ERROR_MEMORY;
    }
    get_num_opt(obj, "kind", &d_val);
    node->kind = (cdd_ffi_trivia_kind_t)d_val;
    get_str_opt(obj, "text", &s_val);
    if (strdup_safe(s_val, &node->text) != CDD_C_SUCCESS) {
      C_CDD_FREE(node);
      *out = head;
      return CDD_C_ERROR_MEMORY;
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
  const char *s_val = NULL;
  double d_val = 0.0;
  int has_v = 0;
  size_t i;
  cdd_c_error_t rc;

  memset(type, 0, sizeof(cdd_ffi_type_t));
  type->array_size = -1;

  if (!obj)
    return CDD_C_SUCCESS;

  has_val_opt(obj, "kind", &has_v);
  if (has_v) {
    get_num_opt(obj, "kind", &d_val);
    type->kind = (cdd_ffi_primitive_kind_t)d_val;
  }
  has_val_opt(obj, "is_const", &has_v);
  if (has_v) {
    get_num_opt(obj, "is_const", &d_val);
    type->is_const = (int)d_val;
  }
  has_val_opt(obj, "pointer_depth", &has_v);
  if (has_v) {
    get_num_opt(obj, "pointer_depth", &d_val);
    type->pointer_depth = (int)d_val;
  }
  has_val_opt(obj, "array_size", &has_v);
  if (has_v) {
    get_num_opt(obj, "array_size", &d_val);
    type->array_size = (long)d_val;
  }

  get_str_opt(obj, "ref_name", &s_val);
  rc = strdup_safe(s_val, &type->ref_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  get_arr_opt(obj, "template_args", &t_arr);
  if (t_arr) {
    cdd_json_array_get_count(t_arr, &type->template_args_count);
    type->template_args =
        C_CDD_CALLOC(type->template_args_count, sizeof(cdd_ffi_type_t));
    if (!type->template_args)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < type->template_args_count; ++i) {
      JSON_Object *sub_obj = NULL;
      cdd_json_array_get_object(t_arr, i, &sub_obj);
      rc = parse_type(sub_obj, &type->template_args[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  get_arr_opt(obj, "leading_trivia", &tr_arr);
  rc = parse_trivia(tr_arr, &type->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;
  get_arr_opt(obj, "trailing_trivia", &tr_arr);
  rc = parse_trivia(tr_arr, &type->trailing_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  get_str_opt(obj, "raw_spelling", &s_val);
  rc = strdup_safe(s_val, &type->raw_spelling);
  if (rc != CDD_C_SUCCESS)
    return rc;
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
  const char *s_val = NULL;
  double d_val = 0.0;
  int has_v = 0;
  JSON_Object *sub_obj = NULL;
  JSON_Array *tr_arr = NULL;
  cdd_c_error_t rc;
  memset(field, 0, sizeof(cdd_ffi_field_t));

  get_str_opt(obj, "name", &s_val);
  rc = strdup_safe(s_val, &field->name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  get_str_opt(obj, "doc", &s_val);
  rc = strdup_safe(s_val, &field->doc);
  if (rc != CDD_C_SUCCESS)
    return rc;

  get_str_opt(obj, "array_length_ref", &s_val);
  rc = strdup_safe(s_val, &field->array_length_ref);
  if (rc != CDD_C_SUCCESS)
    return rc;

  has_val_opt(obj, "intent", &has_v);
  if (has_v) {
    get_num_opt(obj, "intent", &d_val);
    field->intent = (cdd_ffi_param_intent_t)d_val;
  }

  get_obj_opt(obj, "type", &sub_obj);
  rc = parse_type(sub_obj, &field->type);
  if (rc != CDD_C_SUCCESS)
    return rc;

  get_arr_opt(obj, "leading_trivia", &tr_arr);
  rc = parse_trivia(tr_arr, &field->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;
  get_arr_opt(obj, "trailing_trivia", &tr_arr);
  rc = parse_trivia(tr_arr, &field->trailing_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
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
  const char *s_val = NULL;
  double d_val = 0.0;
  int has_v = 0;
  size_t i;

  memset(node, 0, sizeof(cdd_ffi_ir_node_t));

  has_val_opt(obj, "kind", &has_v);
  if (has_v) {
    get_num_opt(obj, "kind", &d_val);
    node->kind = (cdd_ffi_node_kind_t)d_val;
  }

  get_str_opt(obj, "name", &s_val);
  rc = strdup_safe(s_val, &node->name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  get_str_opt(obj, "doc", &s_val);
  rc = strdup_safe(s_val, &node->doc);
  if (rc != CDD_C_SUCCESS)
    return rc;

  get_arr_opt(obj, "fields", &arr);
  if (arr) {
    cdd_json_array_get_count(arr, &node->fields_count);
    node->fields = C_CDD_CALLOC(node->fields_count, sizeof(cdd_ffi_field_t));
    if (!node->fields)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < node->fields_count; ++i) {
      JSON_Object *f_obj = NULL;
      cdd_json_array_get_object(arr, i, &f_obj);
      rc = parse_field(f_obj, &node->fields[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  get_arr_opt(obj, "base_classes", &arr);
  if (arr) {
    cdd_json_array_get_count(arr, &node->base_classes_count);
    node->base_classes =
        C_CDD_CALLOC(node->base_classes_count, sizeof(cdd_ffi_base_class_t));
    if (!node->base_classes)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < node->base_classes_count; ++i) {
      JSON_Object *bc_obj = NULL;
      cdd_json_array_get_object(arr, i, &bc_obj);
      if (bc_obj) {
        rc = cdd_reflect_from_json_object(
            bc_obj, &g_cdd_ffi_base_class_type_info, &node->base_classes[i]);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  get_arr_opt(obj, "variants", &arr);
  if (arr) {
    cdd_json_array_get_count(arr, &node->variants_count);
    node->variants =
        C_CDD_CALLOC(node->variants_count, sizeof(cdd_ffi_enum_variant_t));
    if (!node->variants)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < node->variants_count; ++i) {
      JSON_Object *v_obj = NULL;
      JSON_Object *nd_obj = NULL;
      JSON_Array *tr_arr = NULL;
      cdd_json_array_get_object(arr, i, &v_obj);
      get_str_opt(v_obj, "name", &s_val);
      rc = strdup_safe(s_val, &node->variants[i].name);
      if (rc != CDD_C_SUCCESS)
        return rc;
      get_str_opt(v_obj, "value", &s_val);
      rc = strdup_safe(s_val, &node->variants[i].value);
      if (rc != CDD_C_SUCCESS)
        return rc;
      get_str_opt(v_obj, "doc", &s_val);
      rc = strdup_safe(s_val, &node->variants[i].doc);
      if (rc != CDD_C_SUCCESS)
        return rc;
      get_str_opt(v_obj, "raw_spelling", &s_val);
      rc = strdup_safe(s_val, &node->variants[i].raw_spelling);
      if (rc != CDD_C_SUCCESS)
        return rc;

      get_obj_opt(v_obj, "number_details", &nd_obj);
      rc = parse_number(nd_obj, &node->variants[i].number_details);
      if (rc != CDD_C_SUCCESS)
        return rc;
      get_arr_opt(v_obj, "leading_trivia", &tr_arr);
      rc = parse_trivia(tr_arr, &node->variants[i].leading_trivia);
      if (rc != CDD_C_SUCCESS)
        return rc;
      get_arr_opt(v_obj, "trailing_trivia", &tr_arr);
      rc = parse_trivia(tr_arr, &node->variants[i].trailing_trivia);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  get_arr_opt(obj, "macro_expansions", &arr);
  if (arr) {
    cdd_json_array_get_count(arr, &node->macro_expansions_count);
    node->macro_expansions = C_CDD_CALLOC(
        node->macro_expansions_count, sizeof(struct cdd_ffi_macro_expansion_t));
    if (!node->macro_expansions)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < node->macro_expansions_count; ++i) {
      JSON_Object *m_obj = NULL;
      cdd_json_array_get_object(arr, i, &m_obj);
      rc = parse_macro_expansion(m_obj, &node->macro_expansions[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  has_val_opt(obj, "return_or_base_type", &has_v);
  if (has_v) {
    JSON_Object *rt_obj = NULL;
    get_obj_opt(obj, "return_or_base_type", &rt_obj);
    rc = parse_type(rt_obj, &node->return_or_base_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  get_str_opt(obj, "evaluated_value", &s_val);
  rc = strdup_safe(s_val, &node->evaluated_value);
  if (rc != CDD_C_SUCCESS)
    return rc;

  has_val_opt(obj, "inferred_type", &has_v);
  if (has_v) {
    get_num_opt(obj, "inferred_type", &d_val);
    node->inferred_type = (cdd_ffi_macro_type_t)d_val;
  }
  has_val_opt(obj, "requires_gil_release", &has_v);
  if (has_v) {
    get_num_opt(obj, "requires_gil_release", &d_val);
    node->requires_gil_release = (int)d_val;
  }
  has_val_opt(obj, "is_variadic", &has_v);
  if (has_v) {
    get_num_opt(obj, "is_variadic", &d_val);
    node->is_variadic = (int)d_val;
  }

  get_arr_opt(obj, "leading_trivia", &arr);
  rc = parse_trivia(arr, &node->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;
  get_arr_opt(obj, "trailing_trivia", &arr);
  rc = parse_trivia(arr, &node->trailing_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  get_str_opt(obj, "raw_body", &s_val);
  rc = strdup_safe(s_val, &node->raw_body);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
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
  cdd_ffi_ir_t *ir;
  size_t i;
  cdd_c_error_t rc;

  if (!json_str || !out_ir) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (cdd_json_parse_string(json_str, &root_val) != CDD_C_SUCCESS) {
    return CDD_C_ERROR_PARSE;
  }

  if (cdd_json_value_get_object(root_val, &root_obj) != CDD_C_SUCCESS) {
    cdd_json_value_free(root_val);
    return CDD_C_ERROR_PARSE;
  }

  ir = C_CDD_CALLOC(1, sizeof(cdd_ffi_ir_t));
  if (!ir) {
    cdd_json_value_free(root_val);
    return CDD_C_ERROR_MEMORY;
  }

  get_arr_opt(root_obj, "nodes", &nodes_arr);
  if (nodes_arr) {
    cdd_json_array_get_count(nodes_arr, &ir->nodes_count);
    ir->nodes_capacity = ir->nodes_count;
    if (ir->nodes_count > 0) {
      ir->nodes = C_CDD_CALLOC(ir->nodes_count, sizeof(cdd_ffi_ir_node_t));
      if (!ir->nodes) {
        C_CDD_FREE(ir);
        cdd_json_value_free(root_val);
        return CDD_C_ERROR_MEMORY;
      }

      for (i = 0; i < ir->nodes_count; ++i) {
        JSON_Object *n_obj = NULL;
        cdd_json_array_get_object(nodes_arr, i, &n_obj);
        rc = parse_node(n_obj, &ir->nodes[i]);
        if (rc != CDD_C_SUCCESS) {
          cdd_ffi_ir_free(ir);
          C_CDD_FREE(ir);
          cdd_json_value_free(root_val);
          return rc;
        }
      }
    }
  }

  cdd_json_value_free(root_val);
  *out_ir = ir;
  return CDD_C_SUCCESS;
}
