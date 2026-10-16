/**
 * @file cdd_ffi_ir_json.c
 * @brief Core cdd ffi ir json definitions and implementation.
 */
/* clang-format off */
#include "../../include/cdd_c_error.h"
#include "../../include/ffi/cdd_ffi_ir.h"
#include "../../include/c_cdd/memory.h"
#include "../../include/c_cdd/safe_crt.h"
#include "parson.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Safely sets a JSON object property and frees the value on failure */
#define json_object_set_value(obj, name, val)                                  \
  do {                                                                         \
    JSON_Value *_cdd_jv = (val);                                               \
    if ((json_object_set_value)((obj), (name), _cdd_jv) != JSONSuccess)        \
      json_value_free(_cdd_jv);                                                \
  } while (0)
/** @brief Safely appends a JSON array element and frees the value on failure */
#define json_array_append_value(arr, val)                                      \
  do {                                                                         \
    JSON_Value *_cdd_jv = (val);                                               \
    if ((json_array_append_value)((arr), _cdd_jv) != JSONSuccess)              \
      json_value_free(_cdd_jv);                                                \
  } while (0)

/** @brief Sets a string property on a JSON object or returns CDD_C_ERROR_MEMORY
 */
#define SET_STR(o, k, v)                                                       \
  do {                                                                         \
    if (set_string((o), (k), (v)) != CDD_C_SUCCESS) {                          \
      json_value_free(val);                                                    \
      return CDD_C_ERROR_MEMORY;                                               \
    }                                                                          \
  } while (0)
/** @brief Sets an integer property on a JSON object or returns
 * CDD_C_ERROR_MEMORY */
#define SET_INT(o, k, v)                                                       \
  do {                                                                         \
    if (set_int((o), (k), (v)) != CDD_C_SUCCESS) {                             \
      json_value_free(val);                                                    \
      return CDD_C_ERROR_MEMORY;                                               \
    }                                                                          \
  } while (0)

static cdd_c_error_t set_string(JSON_Object *obj, const char *name,
                                const char *val) {
  if (val) {
    if (json_object_set_string(obj, name, val) != JSONSuccess) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t set_int(JSON_Object *obj, const char *name, int val) {
  if (json_object_set_number(obj, name, (double)val) != JSONSuccess) {
    return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t trivia_to_json(const cdd_ffi_trivia_t *trivia,
                                    JSON_Value **out_val) {
  JSON_Value *val = json_value_init_array();
  JSON_Array *arr = json_value_get_array(val);
  const cdd_ffi_trivia_t *curr = trivia;
  while (curr) {
    JSON_Value *t_val = json_value_init_object();
    JSON_Object *t_obj = json_value_get_object(t_val);
    set_int(t_obj, "kind", (int)curr->kind);
    if (set_string(t_obj, "text", curr->text) != CDD_C_SUCCESS) {
      json_value_free(t_val);
      json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    json_array_append_value(arr, t_val);
    curr = curr->next;
  }
  *out_val = val;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t number_to_json(const struct cdd_ffi_number_t *num,
                                    JSON_Value **out_val) {
  JSON_Value *val;
  JSON_Object *obj;
  val = json_value_init_object();
  obj = json_value_get_object(val);
  SET_STR(obj, "raw_spelling", num->raw_spelling);
  set_int(obj, "prefix", (int)num->prefix);
  SET_INT(obj, "has_separators", num->has_separators);
  *out_val = val;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
macro_expansion_to_json(const struct cdd_ffi_macro_expansion_t *m,
                        JSON_Value **out_val) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  SET_STR(obj, "macro_name", m->macro_name);
  SET_STR(obj, "raw_args", m->raw_args);
  SET_STR(obj, "expanded_text", m->expanded_text);
  *out_val = val;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t type_to_json(const cdd_ffi_type_t *type,
                                  JSON_Value **out_val) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);

  set_int(obj, "kind", (int)type->kind);
  SET_INT(obj, "is_const", type->is_const);
  SET_INT(obj, "pointer_depth", type->pointer_depth);
  SET_STR(obj, "ref_name", type->ref_name);
  if (type->array_size != -1) {
    json_object_set_number(obj, "array_size", (double)type->array_size);
  }

  if (type->template_args) {
    JSON_Value *arr_val = json_value_init_array();
    JSON_Array *arr = json_value_get_array(arr_val);
    size_t i;
    for (i = 0; i < type->template_args_count; ++i) {
      JSON_Value *t_val = NULL;
      cdd_c_error_t rc = type_to_json(&type->template_args[i], &t_val);
      if (rc != CDD_C_SUCCESS) {
        json_value_free(arr_val);
        json_value_free(val);
        return rc;
      }
      json_array_append_value(arr, t_val);
    }
    json_object_set_value(obj, "template_args", arr_val);
  }
  if (type->leading_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(type->leading_trivia, &t_val);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(val);
      return rc;
    }
    json_object_set_value(obj, "leading_trivia", t_val);
  }
  if (type->trailing_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(type->trailing_trivia, &t_val);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(val);
      return rc;
    }
    json_object_set_value(obj, "trailing_trivia", t_val);
  }
  SET_STR(obj, "raw_spelling", type->raw_spelling);
  *out_val = val;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t field_to_json(const cdd_ffi_field_t *field,
                                   JSON_Value **out_val) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);

  SET_STR(obj, "name", field->name);
  {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = type_to_json(&field->type, &t_val);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(val);
      return rc;
    }
    json_object_set_value(obj, "type", t_val);
  }
  SET_STR(obj, "doc", field->doc);
  set_int(obj, "intent", (int)field->intent);
  SET_STR(obj, "array_length_ref", field->array_length_ref);
  if (field->leading_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(field->leading_trivia, &t_val);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(val);
      return rc;
    }
    json_object_set_value(obj, "leading_trivia", t_val);
  }
  if (field->trailing_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(field->trailing_trivia, &t_val);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(val);
      return rc;
    }
    json_object_set_value(obj, "trailing_trivia", t_val);
  }
  *out_val = val;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t node_to_json(const cdd_ffi_ir_node_t *node,
                                  JSON_Value **out_val) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  size_t i;

  set_int(obj, "kind", (int)node->kind);
  SET_STR(obj, "name", node->name);
  SET_STR(obj, "doc", node->doc);

  if (node->fields) {
    JSON_Value *arr_val = json_value_init_array();
    JSON_Array *arr = json_value_get_array(arr_val);
    for (i = 0; i < node->fields_count; ++i) {
      {
        JSON_Value *f_val = NULL;
        cdd_c_error_t rc = field_to_json(&node->fields[i], &f_val);
        if (rc != CDD_C_SUCCESS) {
          json_value_free(arr_val);
          json_value_free(val);
          return rc;
        }
        json_array_append_value(arr, f_val);
      }
    }
    json_object_set_value(obj, "fields", arr_val);
  }

  if (node->base_classes) {
    JSON_Value *arr_val = json_value_init_array();
    JSON_Array *arr = json_value_get_array(arr_val);
    for (i = 0; i < node->base_classes_count; ++i) {
      JSON_Value *bc_val = json_value_init_object();
      JSON_Object *bc_obj = json_value_get_object(bc_val);
      if (set_string(bc_obj, "name", node->base_classes[i].name) !=
              CDD_C_SUCCESS ||
          set_int(bc_obj, "is_virtual", node->base_classes[i].is_virtual) !=
              CDD_C_SUCCESS ||
          set_string(bc_obj, "access", node->base_classes[i].access) !=
              CDD_C_SUCCESS) {
        json_value_free(bc_val);
        json_value_free(arr_val);
        json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
      json_array_append_value(arr, bc_val);
    }
    json_object_set_value(obj, "base_classes", arr_val);
  }

  if (node->variants) {
    JSON_Value *arr_val = json_value_init_array();
    JSON_Array *arr = json_value_get_array(arr_val);
    for (i = 0; i < node->variants_count; ++i) {
      JSON_Value *v_val = json_value_init_object();
      JSON_Object *v_obj = json_value_get_object(v_val);
      if (set_string(v_obj, "name", node->variants[i].name) != CDD_C_SUCCESS ||
          set_string(v_obj, "value", node->variants[i].value) !=
              CDD_C_SUCCESS ||
          set_string(v_obj, "doc", node->variants[i].doc) != CDD_C_SUCCESS) {
        json_value_free(v_val);
        json_value_free(arr_val);
        json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
      if (node->variants[i].number_details) {
        JSON_Value *n_val = NULL;
        cdd_c_error_t rc =
            number_to_json(node->variants[i].number_details, &n_val);
        if (rc != CDD_C_SUCCESS) {
          json_value_free(v_val);
          json_value_free(arr_val);
          json_value_free(val);
          return rc;
        }
        json_object_set_value(v_obj, "number_details", n_val);
      }
      if (node->variants[i].leading_trivia) {
        JSON_Value *t_val = NULL;
        cdd_c_error_t rc =
            trivia_to_json(node->variants[i].leading_trivia, &t_val);
        if (rc != CDD_C_SUCCESS) {
          json_value_free(v_val);
          json_value_free(arr_val);
          json_value_free(val);
          return rc;
        }
        json_object_set_value(v_obj, "leading_trivia", t_val);
      }
      if (node->variants[i].trailing_trivia) {
        JSON_Value *t_val = NULL;
        cdd_c_error_t rc =
            trivia_to_json(node->variants[i].trailing_trivia, &t_val);
        if (rc != CDD_C_SUCCESS) {
          json_value_free(v_val);
          json_value_free(arr_val);
          json_value_free(val);
          return rc;
        }
        json_object_set_value(v_obj, "trailing_trivia", t_val);
      }
      if (set_string(v_obj, "raw_spelling", node->variants[i].raw_spelling) !=
          CDD_C_SUCCESS) {
        json_value_free(v_val);
        json_value_free(arr_val);
        json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
      json_array_append_value(arr, v_val);
    }
    json_object_set_value(obj, "variants", arr_val);
  }

  if (node->macro_expansions) {
    JSON_Value *arr_val = json_value_init_array();
    JSON_Array *arr = json_value_get_array(arr_val);
    for (i = 0; i < node->macro_expansions_count; ++i) {
      {
        JSON_Value *m_val = NULL;
        cdd_c_error_t rc =
            macro_expansion_to_json(&node->macro_expansions[i], &m_val);
        if (rc != CDD_C_SUCCESS) {
          json_value_free(arr_val);
          json_value_free(val);
          return rc;
        }
        json_array_append_value(arr, m_val);
      }
    }
    json_object_set_value(obj, "macro_expansions", arr_val);
  }

  if (node->kind == CDD_FFI_NODE_FUNCTION ||
      node->kind == CDD_FFI_NODE_TYPEDEF) {
    {
      JSON_Value *t_val = NULL;
      cdd_c_error_t rc = type_to_json(&node->return_or_base_type, &t_val);
      if (rc != CDD_C_SUCCESS) {
        json_value_free(val);
        return rc;
      }
      json_object_set_value(obj, "return_or_base_type", t_val);
    }
  }

  SET_STR(obj, "evaluated_value", node->evaluated_value);
  set_int(obj, "inferred_type", (int)node->inferred_type);
  SET_INT(obj, "requires_gil_release", node->requires_gil_release);
  SET_INT(obj, "is_variadic", node->is_variadic);

  if (node->leading_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(node->leading_trivia, &t_val);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(val);
      return rc;
    }
    json_object_set_value(obj, "leading_trivia", t_val);
  }
  if (node->trailing_trivia) {
    JSON_Value *t_val = NULL;
    cdd_c_error_t rc = trivia_to_json(node->trailing_trivia, &t_val);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(val);
      return rc;
    }
    json_object_set_value(obj, "trailing_trivia", t_val);
  }
  SET_STR(obj, "raw_body", node->raw_body);
  *out_val = val;
  return CDD_C_SUCCESS;
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_ir_to_json(const cdd_ffi_ir_t *ir,
                                              char **out_json) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Value *nodes_val;
  JSON_Array *nodes_arr;
  size_t i;
  char *serialized;

  if (!ir || !out_json) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);

  nodes_val = json_value_init_array();
  nodes_arr = json_value_get_array(nodes_val);

  for (i = 0; i < ir->nodes_count; ++i) {
    {
      JSON_Value *n_val = NULL;
      cdd_c_error_t rc = node_to_json(&ir->nodes[i], &n_val);
      if (rc != CDD_C_SUCCESS) {
        json_value_free(nodes_val);
        json_value_free(root_val);
        return rc;
      }
      json_array_append_value(nodes_arr, n_val);
    }
  }

  json_object_set_value(root_obj, "nodes", nodes_val);

  serialized = json_serialize_to_string_pretty(root_val);
  json_value_free(root_val);

  if (!serialized) {
    return CDD_C_ERROR_UNKNOWN;
  }

  *out_json = serialized;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t strdup_safe(const char *in, char **out) {
  if (!in) {
    *out = NULL;
    return CDD_C_SUCCESS;
  }
  return (*out = C_CDD_STRDUP(in)) ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

static cdd_c_error_t parse_trivia(JSON_Array *arr, cdd_ffi_trivia_t **out) {
  size_t i, count;
  cdd_ffi_trivia_t *head = NULL, *tail = NULL, *node = NULL;
  if (!arr)
    return CDD_C_SUCCESS;
  count = json_array_get_count(arr);
  for (i = 0; i < count; ++i) {
    JSON_Object *obj = json_array_get_object(arr, i);
    if (!obj)
      continue;
    node = C_CDD_CALLOC(1, sizeof(cdd_ffi_trivia_t));
    if (!node) {
      *out = head;
      return CDD_C_ERROR_MEMORY;
    }
    node->kind = (cdd_ffi_trivia_kind_t)json_object_get_number(obj, "kind");
    if (strdup_safe(json_object_get_string(obj, "text"), &node->text) !=
        CDD_C_SUCCESS) {
      free(node);
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

static cdd_c_error_t parse_number(JSON_Object *obj,
                                  struct cdd_ffi_number_t **out) {
  struct cdd_ffi_number_t *num;
  if (!obj)
    return CDD_C_SUCCESS;
  num = C_CDD_CALLOC(1, sizeof(struct cdd_ffi_number_t));
  if (!num)
    return CDD_C_ERROR_MEMORY;
  strdup_safe(json_object_get_string(obj, "raw_spelling"), &num->raw_spelling);
  num->prefix = (cdd_ffi_number_prefix_t)json_object_get_number(obj, "prefix");
  num->has_separators = (int)json_object_get_number(obj, "has_separators");
  *out = num;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
parse_macro_expansion(JSON_Object *obj, struct cdd_ffi_macro_expansion_t *m) {
  strdup_safe(json_object_get_string(obj, "macro_name"), &m->macro_name);
  strdup_safe(json_object_get_string(obj, "raw_args"), &m->raw_args);
  strdup_safe(json_object_get_string(obj, "expanded_text"), &m->expanded_text);
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_type(JSON_Object *obj, cdd_ffi_type_t *type) {
  JSON_Array *t_arr;
  size_t i;
  cdd_c_error_t rc;

  memset(type, 0, sizeof(cdd_ffi_type_t));
  type->array_size = -1;

  if (!obj)
    return CDD_C_SUCCESS;

  if (json_object_has_value(obj, "kind"))
    type->kind = (cdd_ffi_primitive_kind_t)json_object_get_number(obj, "kind");
  if (json_object_has_value(obj, "is_const"))
    type->is_const = (int)json_object_get_number(obj, "is_const");
  if (json_object_has_value(obj, "pointer_depth"))
    type->pointer_depth = (int)json_object_get_number(obj, "pointer_depth");
  if (json_object_has_value(obj, "array_size"))
    type->array_size = (long)json_object_get_number(obj, "array_size");

  rc = strdup_safe(json_object_get_string(obj, "ref_name"), &type->ref_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  t_arr = json_object_get_array(obj, "template_args");
  if (t_arr) {
    type->template_args_count = json_array_get_count(t_arr);
    type->template_args =
        C_CDD_CALLOC(type->template_args_count, sizeof(cdd_ffi_type_t));
    if (!type->template_args)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < type->template_args_count; ++i) {
      rc = parse_type(json_array_get_object(t_arr, i), &type->template_args[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  parse_trivia(json_object_get_array(obj, "leading_trivia"),
               &type->leading_trivia);
  parse_trivia(json_object_get_array(obj, "trailing_trivia"),
               &type->trailing_trivia);

  rc = strdup_safe(json_object_get_string(obj, "raw_spelling"),
                   &type->raw_spelling);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_field(JSON_Object *obj, cdd_ffi_field_t *field) {
  cdd_c_error_t rc;
  memset(field, 0, sizeof(cdd_ffi_field_t));

  rc = strdup_safe(json_object_get_string(obj, "name"), &field->name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = strdup_safe(json_object_get_string(obj, "doc"), &field->doc);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = strdup_safe(json_object_get_string(obj, "array_length_ref"),
                   &field->array_length_ref);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (json_object_has_value(obj, "intent"))
    field->intent =
        (cdd_ffi_param_intent_t)json_object_get_number(obj, "intent");

  rc = parse_type(json_object_get_object(obj, "type"), &field->type);
  if (rc != CDD_C_SUCCESS)
    return rc;

  parse_trivia(json_object_get_array(obj, "leading_trivia"),
               &field->leading_trivia);
  parse_trivia(json_object_get_array(obj, "trailing_trivia"),
               &field->trailing_trivia);

  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_node(JSON_Object *obj, cdd_ffi_ir_node_t *node) {
  cdd_c_error_t rc;
  JSON_Array *arr;
  size_t i;

  memset(node, 0, sizeof(cdd_ffi_ir_node_t));

  if (json_object_has_value(obj, "kind"))
    node->kind = (cdd_ffi_node_kind_t)json_object_get_number(obj, "kind");

  rc = strdup_safe(json_object_get_string(obj, "name"), &node->name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = strdup_safe(json_object_get_string(obj, "doc"), &node->doc);
  if (rc != CDD_C_SUCCESS)
    return rc;

  arr = json_object_get_array(obj, "fields");
  if (arr) {
    node->fields_count = json_array_get_count(arr);
    node->fields = C_CDD_CALLOC(node->fields_count, sizeof(cdd_ffi_field_t));
    if (!node->fields)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < node->fields_count; ++i) {
      rc = parse_field(json_array_get_object(arr, i), &node->fields[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  arr = json_object_get_array(obj, "base_classes");
  if (arr) {
    node->base_classes_count = json_array_get_count(arr);
    node->base_classes =
        C_CDD_CALLOC(node->base_classes_count, sizeof(cdd_ffi_base_class_t));
    if (!node->base_classes)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < node->base_classes_count; ++i) {
      JSON_Object *bc_obj = json_array_get_object(arr, i);
      rc = strdup_safe(json_object_get_string(bc_obj, "name"),
                       &node->base_classes[i].name);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = strdup_safe(json_object_get_string(bc_obj, "access"),
                       &node->base_classes[i].access);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (json_object_has_value(bc_obj, "is_virtual"))
        node->base_classes[i].is_virtual =
            (int)json_object_get_number(bc_obj, "is_virtual");
    }
  }

  arr = json_object_get_array(obj, "variants");
  if (arr) {
    node->variants_count = json_array_get_count(arr);
    node->variants =
        C_CDD_CALLOC(node->variants_count, sizeof(cdd_ffi_enum_variant_t));
    if (!node->variants)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < node->variants_count; ++i) {
      JSON_Object *v_obj = json_array_get_object(arr, i);
      rc = strdup_safe(json_object_get_string(v_obj, "name"),
                       &node->variants[i].name);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = strdup_safe(json_object_get_string(v_obj, "value"),
                       &node->variants[i].value);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = strdup_safe(json_object_get_string(v_obj, "doc"),
                       &node->variants[i].doc);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = strdup_safe(json_object_get_string(v_obj, "raw_spelling"),
                       &node->variants[i].raw_spelling);
      if (rc != CDD_C_SUCCESS)
        return rc;

      parse_number(json_object_get_object(v_obj, "number_details"),
                   &node->variants[i].number_details);
      parse_trivia(json_object_get_array(v_obj, "leading_trivia"),
                   &node->variants[i].leading_trivia);
      parse_trivia(json_object_get_array(v_obj, "trailing_trivia"),
                   &node->variants[i].trailing_trivia);
    }
  }

  arr = json_object_get_array(obj, "macro_expansions");
  if (arr) {
    node->macro_expansions_count = json_array_get_count(arr);
    node->macro_expansions = C_CDD_CALLOC(
        node->macro_expansions_count, sizeof(struct cdd_ffi_macro_expansion_t));
    if (!node->macro_expansions)
      return CDD_C_ERROR_MEMORY;
    for (i = 0; i < node->macro_expansions_count; ++i) {
      parse_macro_expansion(json_array_get_object(arr, i),
                            &node->macro_expansions[i]);
    }
  }

  if (json_object_has_value(obj, "return_or_base_type")) {
    rc = parse_type(json_object_get_object(obj, "return_or_base_type"),
                    &node->return_or_base_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = strdup_safe(json_object_get_string(obj, "evaluated_value"),
                   &node->evaluated_value);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (json_object_has_value(obj, "inferred_type"))
    node->inferred_type =
        (cdd_ffi_macro_type_t)json_object_get_number(obj, "inferred_type");
  if (json_object_has_value(obj, "requires_gil_release"))
    node->requires_gil_release =
        (int)json_object_get_number(obj, "requires_gil_release");
  if (json_object_has_value(obj, "is_variadic"))
    node->is_variadic = (int)json_object_get_number(obj, "is_variadic");

  parse_trivia(json_object_get_array(obj, "leading_trivia"),
               &node->leading_trivia);
  parse_trivia(json_object_get_array(obj, "trailing_trivia"),
               &node->trailing_trivia);

  rc = strdup_safe(json_object_get_string(obj, "raw_body"), &node->raw_body);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_ir_from_json(const char *json_str,
                                                cdd_ffi_ir_t **out_ir) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Array *nodes_arr;
  cdd_ffi_ir_t *ir;
  size_t i;
  cdd_c_error_t rc;

  if (!json_str || !out_ir) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  root_val = json_parse_string(json_str);
  if (!root_val) {
    return CDD_C_ERROR_PARSE;
  }

  root_obj = json_value_get_object(root_val);
  if (!root_obj) {
    json_value_free(root_val);
    return CDD_C_ERROR_PARSE;
  }

  ir = C_CDD_CALLOC(1, sizeof(cdd_ffi_ir_t));
  if (!ir) {
    json_value_free(root_val);
    return CDD_C_ERROR_MEMORY;
  }

  nodes_arr = json_object_get_array(root_obj, "nodes");
  if (nodes_arr) {
    ir->nodes_count = json_array_get_count(nodes_arr);
    ir->nodes_capacity = ir->nodes_count;
    if (ir->nodes_count > 0) {
      ir->nodes = C_CDD_CALLOC(ir->nodes_count, sizeof(cdd_ffi_ir_node_t));
      if (!ir->nodes) {
        free(ir);
        json_value_free(root_val);
        return CDD_C_ERROR_MEMORY;
      }

      for (i = 0; i < ir->nodes_count; ++i) {
        rc = parse_node(json_array_get_object(nodes_arr, i), &ir->nodes[i]);
        if (rc != CDD_C_SUCCESS) {
          cdd_ffi_ir_free(ir);
          free(ir);
          json_value_free(root_val);
          return rc;
        }
      }
    }
  }

  json_value_free(root_val);
  *out_ir = ir;
  return CDD_C_SUCCESS;
}
