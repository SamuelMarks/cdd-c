/**
 * @file openapi_schemas.c
 * @brief OpenAPI emitter schema generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#include "c_cdd/log.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
/* clang-format on */

/* --- Helper Prototypes --- */
static void write_schema_example(JSON_Object *obj,
                                 const struct OpenAPI_Any *example,
                                 int example_set);
static void write_multipart_schema(JSON_Object *parent, const char *key,
                                   const struct OpenAPI_SchemaRef *ref);

/**
 * @brief Executes the schema ref has data operation.
 */
C_CDD_EXPORT cdd_c_error_t
schema_ref_has_data(const struct OpenAPI_SchemaRef *ref) {
  if (!ref)
    return CDD_C_SUCCESS;
  return ref->schema_is_boolean || ref->ref_name || ref->ref ||
         ref->inline_type || ref->n_type_union > 0 || ref->is_array ||
         ref->format || ref->content_media_type || ref->content_encoding ||
         ref->items_format || ref->n_items_type_union > 0 ||
         ref->items_content_media_type || ref->items_content_encoding ||
         ref->n_multipart_fields > 0 || ref->nullable || ref->items_nullable ||
         ref->default_value_set || ref->n_enum_values > 0 ||
         ref->n_items_enum_values > 0 || ref->summary || ref->description ||
         ref->deprecated_set || ref->read_only_set || ref->write_only_set ||
         ref->const_value_set || ref->n_examples > 0 || ref->example_set ||
         ref->has_min || ref->has_max || ref->has_min_len || ref->has_max_len ||
         ref->pattern || ref->has_min_items || ref->has_max_items ||
         ref->unique_items || ref->items_has_min || ref->items_has_max ||
         ref->items_has_min_len || ref->items_has_max_len ||
         ref->items_pattern || ref->items_has_min_items ||
         ref->items_has_max_items || ref->items_unique_items ||
         ref->items_example_set || ref->n_items_examples > 0 ||
         ref->items_schema_is_boolean || ref->schema_extra_json ||
         ref->external_docs_set || ref->discriminator_set || ref->xml_set ||
         ref->items_extra_json || ref->items_const_value_set ||
         ref->items_default_value_set;
}

/**
 * @brief Generates C code for write schema type.
 */
C_CDD_EXPORT void write_schema_type(JSON_Object *obj, const char *type,
                                    int nullable) {
  if (!obj || !type)
    return;
  if (nullable && strcmp(type, "null") != 0) {
    JSON_Value *type_val = json_value_init_array();
    JSON_Array *type_arr = json_value_get_array(type_val);
    if (!type_arr) {
      json_value_free(type_val);
      return;
    }
    json_array_append_string(type_arr, type);
    json_array_append_string(type_arr, "null");
    json_object_set_value(obj, "type", type_val);
    return;
  }
  json_object_set_string(obj, "type", type);
}

/**
 * @brief Executes the type union contains operation.
 */
C_CDD_EXPORT cdd_c_error_t type_union_contains(char **types, size_t n_types,
                                               const char *value) {
  size_t i;
  if (!types || !value)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_types; ++i) {
    if (types[i] && strcmp(types[i], value) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write schema type union.
 */
C_CDD_EXPORT void write_schema_type_union(JSON_Object *obj, const char *type,
                                          int nullable, char **type_union,
                                          size_t n_type_union) {
  size_t i;
  if (!obj)
    return;
  if (type_union && n_type_union > 0) {
    JSON_Value *type_val = json_value_init_array();
    JSON_Array *type_arr = json_value_get_array(type_val);
    if (!type_arr) {
      json_value_free(type_val);
      return;
    }
    for (i = 0; i < n_type_union; ++i) {
      if (type_union[i])
        json_array_append_string(type_arr, type_union[i]);
    }
    if (nullable && !type_union_contains(type_union, n_type_union, "null"))
      json_array_append_string(type_arr, "null");
    json_object_set_value(obj, "type", type_val);
    return;
  }
  if (type)
    write_schema_type(obj, type, nullable);
}
C_CDD_EXPORT void write_enum_any_values(JSON_Object *obj, const char *key,
                                        const struct OpenAPI_Any *values,
                                        size_t n_values) {
  JSON_Value *_ast_any_to_json_value_1;
  JSON_Value *enum_val;
  JSON_Array *enum_arr;
  size_t i;

  if (!obj || !key || !values || n_values == 0)
    return;

  enum_val = json_value_init_array();
  enum_arr = json_value_get_array(enum_val);
  if (!enum_arr) {
    json_value_free(enum_val);
    return;
  }

  for (i = 0; i < n_values; ++i) {
    JSON_Value *val = (any_to_json_value(&values[i], &_ast_any_to_json_value_1),
                       _ast_any_to_json_value_1);
    if (val)
      json_array_append_value(enum_arr, val);
  }

  json_object_set_value(obj, key, enum_val);
}

/**
 * @brief Generates C code for write any array values.
 */
C_CDD_EXPORT void write_any_array_values(JSON_Object *obj, const char *key,
                                         const struct OpenAPI_Any *values,
                                         size_t n_values) {
  write_enum_any_values(obj, key, values, n_values);
}

/**
 * @brief Executes the any to json value operation.
 */
C_CDD_EXPORT cdd_c_error_t any_to_json_value(const struct OpenAPI_Any *val,
                                             JSON_Value **_out_val) {
  if (!val) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  switch (val->type) {
  case OA_ANY_STRING: {
    *_out_val = json_value_init_string(val->string ? val->string : "");
    return CDD_C_SUCCESS;
  }
  case OA_ANY_NUMBER: {
    *_out_val = json_value_init_number(val->number);
    return CDD_C_SUCCESS;
  }
  case OA_ANY_BOOL: {
    *_out_val = json_value_init_boolean(val->boolean ? 1 : 0);
    return CDD_C_SUCCESS;
  }
  case OA_ANY_NULL: {
    *_out_val = json_value_init_null();
    return CDD_C_SUCCESS;
  }
  case OA_ANY_JSON:
    if (val->json) {
      JSON_Value *parsed = json_parse_string(val->json);
      if (parsed) {
        *_out_val = parsed;
        return CDD_C_SUCCESS;
      }
    }
    {
      *_out_val = json_value_init_string(val->json ? val->json : "");
      return CDD_C_SUCCESS;
    }
  default:
    break;
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Construct an inline schema definition for Multipart fields.
 */
static void write_multipart_schema(JSON_Object *parent, const char *key,
                                   const struct OpenAPI_SchemaRef *ref) {
  JSON_Value *sch_val = json_value_init_object();
  JSON_Object *sch_obj = json_value_get_object(sch_val);
  JSON_Value *props_val = json_value_init_object();
  JSON_Object *props_obj = json_value_get_object(props_val);
  size_t i;

  json_object_set_string(sch_obj, "type", "object");

  for (i = 0; i < ref->n_multipart_fields; ++i) {
    const struct OpenAPI_MultipartField *f = &ref->multipart_fields[i];
    JSON_Value *prop_val = json_value_init_object();
    JSON_Object *prop_obj = json_value_get_object(prop_val);

    if (f->is_binary) {
      json_object_set_string(prop_obj, "type", "string");
      json_object_set_string(prop_obj, "format", "binary");
    } else if (f->type) {
      json_object_set_string(prop_obj, "type", f->type);
    } else {
      /* Default to string */
      json_object_set_string(prop_obj, "type", "string");
    }
    json_object_set_value(props_obj, f->name ? f->name : "unknown", prop_val);
  }

  json_object_set_value(sch_obj, "properties", props_val);
  json_object_set_value(parent, key, sch_val);
}

/**
 * @brief Generates C code for write schema example.
 */
static void write_schema_example(JSON_Object *obj,
                                 const struct OpenAPI_Any *example,
                                 int example_set) {
  JSON_Value *_ast_any_to_json_value_6;
  JSON_Value *ex_val;
  if (!obj || !example_set)
    return;
  ex_val = (any_to_json_value(example, &_ast_any_to_json_value_6),
            _ast_any_to_json_value_6);
  if (ex_val)
    json_object_set_value(obj, "example", ex_val);
}

/**
 * @brief Generates C code for write numeric constraints.
 */
C_CDD_EXPORT void write_numeric_constraints(JSON_Object *obj, int has_min,
                                            double min_val, int exclusive_min,
                                            int has_max, double max_val,
                                            int exclusive_max) {
  if (!obj)
    return;
  if (has_min) {
    if (exclusive_min)
      json_object_set_number(obj, "exclusiveMinimum", min_val);
    else
      json_object_set_number(obj, "minimum", min_val);
  } else if (exclusive_min) {
    json_object_set_boolean(obj, "exclusiveMinimum", 1);
  }
  if (has_max) {
    if (exclusive_max)
      json_object_set_number(obj, "exclusiveMaximum", max_val);
    else
      json_object_set_number(obj, "maximum", max_val);
  } else if (exclusive_max) {
    json_object_set_boolean(obj, "exclusiveMaximum", 1);
  }
}

/**
 * @brief Generates C code for write string constraints.
 */
C_CDD_EXPORT void write_string_constraints(JSON_Object *obj, int has_min_len,
                                           size_t min_len, int has_max_len,
                                           size_t max_len,
                                           const char *pattern) {
  if (!obj)
    return;
  if (has_min_len)
    json_object_set_number(obj, "minLength", (double)min_len);
  if (has_max_len)
    json_object_set_number(obj, "maxLength", (double)max_len);
  if (pattern)
    json_object_set_string(obj, "pattern", pattern);
}

/**
 * @brief Generates C code for write array constraints.
 */
C_CDD_EXPORT void write_array_constraints(JSON_Object *obj, int has_min_items,
                                          size_t min_items, int has_max_items,
                                          size_t max_items, int unique_items) {
  if (!obj)
    return;
  if (has_min_items)
    json_object_set_number(obj, "minItems", (double)min_items);
  if (has_max_items)
    json_object_set_number(obj, "maxItems", (double)max_items);
  if (unique_items)
    json_object_set_boolean(obj, "uniqueItems", 1);
}

/**
 * @brief Generates C code for write items schema fields.
 */
C_CDD_EXPORT void
write_items_schema_fields(JSON_Object *item_obj,
                          const struct OpenAPI_SchemaRef *ref) {
  JSON_Value *_ast_any_to_json_value_7;
  JSON_Value *_ast_any_to_json_value_8;
  if (!item_obj || !ref)
    return;

  write_numeric_constraints(item_obj, ref->items_has_min, ref->items_min_val,
                            ref->items_exclusive_min, ref->items_has_max,
                            ref->items_max_val, ref->items_exclusive_max);
  write_string_constraints(item_obj, ref->items_has_min_len, ref->items_min_len,
                           ref->items_has_max_len, ref->items_max_len,
                           ref->items_pattern);
  write_array_constraints(item_obj, ref->items_has_min_items,
                          ref->items_min_items, ref->items_has_max_items,
                          ref->items_max_items, ref->items_unique_items);
  write_schema_example(item_obj, &ref->items_example, ref->items_example_set);
  if (ref->n_items_examples > 0) {
    write_any_array_values(item_obj, "examples", ref->items_examples,
                           ref->n_items_examples);
  }
  if (ref->items_format)
    json_object_set_string(item_obj, "format", ref->items_format);
  if (ref->items_content_media_type)
    json_object_set_string(item_obj, "contentMediaType",
                           ref->items_content_media_type);
  if (ref->items_content_schema) {
    write_schema_ref(item_obj, "contentSchema", ref->items_content_schema);
  }
  if (ref->items_content_encoding)
    json_object_set_string(item_obj, "contentEncoding",
                           ref->items_content_encoding);
  if (ref->n_items_enum_values > 0)
    write_enum_any_values(item_obj, "enum", ref->items_enum_values,
                          ref->n_items_enum_values);
  if (ref->items_const_value_set) {
    JSON_Value *const_val =
        (any_to_json_value(&ref->items_const_value, &_ast_any_to_json_value_7),
         _ast_any_to_json_value_7);
    if (const_val)
      json_object_set_value(item_obj, "const", const_val);
  }
  if (ref->items_default_value_set) {
    JSON_Value *def_val = (any_to_json_value(&ref->items_default_value,
                                             &_ast_any_to_json_value_8),
                           _ast_any_to_json_value_8);
    if (def_val)
      json_object_set_value(item_obj, "default", def_val);
  }
  if (ref->items_extra_json)
    merge_schema_extras_object_openapi(item_obj, ref->items_extra_json);
}

/**
 * @brief Executes the schema ref keyword operation.
 */
C_CDD_EXPORT cdd_c_error_t schema_ref_keyword(int is_dynamic, char **_out_val) {
  {
    *_out_val = is_dynamic ? "$dynamicRef" : "$ref";
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Write a Schema Reference object (or inline Type).
 *
 * Handles `$ref`, `type: array`, and basic types.
 * Populates `parent` at `key` (e.g. key="schema").
 */
C_CDD_EXPORT void write_schema_ref(JSON_Object *parent, const char *key,
                                   const struct OpenAPI_SchemaRef *ref) {
  char *_ast_schema_ref_keyword_9 = NULL;
  char *_ast_schema_ref_keyword_10 = NULL;
  JSON_Value *_ast_any_to_json_value_11;
  JSON_Value *_ast_any_to_json_value_12;
  JSON_Value *_ast_any_to_json_value_13;
  JSON_Value *sch_val;
  JSON_Object *sch_obj;
  char ref_path[256];

  if (!parent || !key || !ref)
    return;

  if (ref->schema_is_boolean) {
    json_object_set_boolean(parent, key, ref->schema_boolean_value ? 1 : 0);
    return;
  }

  sch_val = json_value_init_object();
  sch_obj = json_value_get_object(sch_val);

  /* Case 1: Built-in Multipart Fields (Inline Schema) */
  if (ref->n_multipart_fields > 0) {
    json_value_free(sch_val); /* Discard placeholder */
    write_multipart_schema(parent, key, ref);
    return;
  }

  /* Case 2: Array */
  if (ref->is_array) {
    write_schema_type_union(sch_obj, "array", ref->nullable, ref->type_union,
                            ref->n_type_union);
    if (ref->items_schema_is_boolean) {
      json_object_set_boolean(sch_obj, "items",
                              ref->items_schema_boolean_value ? 1 : 0);
    } else if (ref->inline_type) {
      JSON_Value *item_val = json_value_init_object();
      JSON_Object *item_obj = json_value_get_object(item_val);
      write_schema_type_union(item_obj, ref->inline_type, ref->items_nullable,
                              ref->items_type_union, ref->n_items_type_union);
      write_items_schema_fields(item_obj, ref);
      json_object_set_value(sch_obj, "items", item_val);
    } else if (ref->items_ref) {
      JSON_Value *item_val = json_value_init_object();
      JSON_Object *item_obj = json_value_get_object(item_val);
      json_object_set_string(item_obj,
                             (schema_ref_keyword(ref->items_ref_is_dynamic,
                                                 &_ast_schema_ref_keyword_9),
                              _ast_schema_ref_keyword_9),
                             ref->items_ref);
      write_items_schema_fields(item_obj, ref);
      json_object_set_value(sch_obj, "items", item_val);
    } else if (ref->ref_name) {
      /* Simple detection: if standard type, use type, else ref */
      if (is_schema_primitive_openapi(ref->ref_name)) {
        JSON_Value *item_val = json_value_init_object();
        JSON_Object *item_obj = json_value_get_object(item_val);
        write_schema_type_union(item_obj, ref->ref_name, ref->items_nullable,
                                ref->items_type_union, ref->n_items_type_union);
        write_items_schema_fields(item_obj, ref);
        json_object_set_value(sch_obj, "items", item_val);
      } else {
        JSON_Value *item_val = json_value_init_object();
        JSON_Object *item_obj = json_value_get_object(item_val);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                  ref->ref_name);
#else
        CDD_SNPRINTF(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                     ref->ref_name);
#endif
        json_object_set_string(item_obj, "$ref", ref_path);
        write_items_schema_fields(item_obj, ref);
        json_object_set_value(sch_obj, "items", item_val);
      }
    }
  }
  /* Case 3: Reference or Primitive */
  else if (ref->inline_type) {
    write_schema_type_union(sch_obj, ref->inline_type, ref->nullable,
                            ref->type_union, ref->n_type_union);
  } else if (ref->ref) {
    json_object_set_string(
        sch_obj,
        (schema_ref_keyword(ref->ref_is_dynamic, &_ast_schema_ref_keyword_10),
         _ast_schema_ref_keyword_10),
        ref->ref);
  } else if (ref->ref_name) {
    if (is_schema_primitive_openapi(ref->ref_name)) {
      write_schema_type_union(sch_obj, ref->ref_name, ref->nullable,
                              ref->type_union, ref->n_type_union);
    } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                ref->ref_name);
#else
      CDD_SNPRINTF(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                   ref->ref_name);
#endif
      json_object_set_string(sch_obj, "$ref", ref_path);
    }
  }

  if (ref->format)
    json_object_set_string(sch_obj, "format", ref->format);
  write_numeric_constraints(sch_obj, ref->has_min, ref->min_val,
                            ref->exclusive_min, ref->has_max, ref->max_val,
                            ref->exclusive_max);
  write_string_constraints(sch_obj, ref->has_min_len, ref->min_len,
                           ref->has_max_len, ref->max_len, ref->pattern);
  write_array_constraints(sch_obj, ref->has_min_items, ref->min_items,
                          ref->has_max_items, ref->max_items,
                          ref->unique_items);

  if (ref->has_multiple_of)
    json_object_set_number(sch_obj, "multipleOf", ref->multiple_of);
  if (ref->has_min_properties)
    json_object_set_number(sch_obj, "minProperties",
                           (double)ref->min_properties);
  if (ref->has_max_properties)
    json_object_set_number(sch_obj, "maxProperties",
                           (double)ref->max_properties);

  if (ref->all_of && ref->n_all_of > 0) {
    JSON_Value *arr_val = json_value_init_array();
    JSON_Array *arr = json_value_get_array(arr_val);
    size_t i;
    for (i = 0; i < ref->n_all_of; ++i) {
      JSON_Value *temp_val = json_value_init_object();
      JSON_Value *copied_val;
      write_schema_ref(json_value_get_object(temp_val), "schema",
                       &ref->all_of[i]);
      copied_val = json_value_deep_copy(
          json_object_get_value(json_value_get_object(temp_val), "schema"));
      if (copied_val)
        json_array_append_value(arr, copied_val);
      json_value_free(temp_val);
    }
    json_object_set_value(sch_obj, "allOf", arr_val);
  }
  if (ref->any_of && ref->n_any_of > 0) {
    JSON_Value *arr_val = json_value_init_array();
    JSON_Array *arr = json_value_get_array(arr_val);
    size_t i;
    for (i = 0; i < ref->n_any_of; ++i) {
      JSON_Value *temp_val = json_value_init_object();
      JSON_Value *copied_val;
      write_schema_ref(json_value_get_object(temp_val), "schema",
                       &ref->any_of[i]);
      copied_val = json_value_deep_copy(
          json_object_get_value(json_value_get_object(temp_val), "schema"));
      if (copied_val)
        json_array_append_value(arr, copied_val);
      json_value_free(temp_val);
    }
    json_object_set_value(sch_obj, "anyOf", arr_val);
  }
  if (ref->one_of && ref->n_one_of > 0) {
    JSON_Value *arr_val = json_value_init_array();
    JSON_Array *arr = json_value_get_array(arr_val);
    size_t i;
    for (i = 0; i < ref->n_one_of; ++i) {
      JSON_Value *temp_val = json_value_init_object();
      JSON_Value *copied_val;
      write_schema_ref(json_value_get_object(temp_val), "schema",
                       &ref->one_of[i]);
      copied_val = json_value_deep_copy(
          json_object_get_value(json_value_get_object(temp_val), "schema"));
      if (copied_val)
        json_array_append_value(arr, copied_val);
      json_value_free(temp_val);
    }
    json_object_set_value(sch_obj, "oneOf", arr_val);
  }
  if (ref->not_schema)
    write_schema_ref(sch_obj, "not", ref->not_schema);
  if (ref->if_schema)
    write_schema_ref(sch_obj, "if", ref->if_schema);
  if (ref->then_schema)
    write_schema_ref(sch_obj, "then", ref->then_schema);
  if (ref->else_schema)
    write_schema_ref(sch_obj, "else", ref->else_schema);
  if (ref->content_media_type)
    json_object_set_string(sch_obj, "contentMediaType",
                           ref->content_media_type);
  if (ref->content_schema) {
    write_schema_ref(sch_obj, "contentSchema", ref->content_schema);
  }
  if (ref->content_encoding)
    json_object_set_string(sch_obj, "contentEncoding", ref->content_encoding);
  if (ref->summary && (ref->ref_name || ref->ref))
    json_object_set_string(sch_obj, "summary", ref->summary);
  if (ref->description)
    json_object_set_string(sch_obj, "description", ref->description);
  if (ref->external_docs_set)
    write_external_docs(sch_obj, "externalDocs", &ref->external_docs);
  write_discriminator_object(sch_obj, &ref->discriminator,
                             ref->discriminator_set);
  write_xml_object(sch_obj, &ref->xml, ref->xml_set);
  if (ref->deprecated_set)
    json_object_set_boolean(sch_obj, "deprecated", ref->deprecated ? 1 : 0);
  if (ref->read_only_set)
    json_object_set_boolean(sch_obj, "readOnly", ref->read_only ? 1 : 0);
  if (ref->write_only_set)
    json_object_set_boolean(sch_obj, "writeOnly", ref->write_only ? 1 : 0);
  if (ref->const_value_set) {
    JSON_Value *const_val =
        (any_to_json_value(&ref->const_value, &_ast_any_to_json_value_11),
         _ast_any_to_json_value_11);
    if (const_val)
      json_object_set_value(sch_obj, "const", const_val);
  }
  write_schema_example(sch_obj, &ref->example, ref->example_set);
  if (ref->n_examples > 0 && ref->examples) {
    JSON_Value *examples_val = json_value_init_array();
    JSON_Array *examples_arr = json_value_get_array(examples_val);
    size_t i;
    if (examples_arr) {
      for (i = 0; i < ref->n_examples; ++i) {
        JSON_Value *ex_val =
            (any_to_json_value(&ref->examples[i], &_ast_any_to_json_value_12),
             _ast_any_to_json_value_12);
        if (ex_val)
          json_array_append_value(examples_arr, ex_val);
      }
      json_object_set_value(sch_obj, "examples", examples_val);
    } else {
      json_value_free(examples_val);
    }
  }
  if (ref->n_enum_values > 0)
    write_enum_any_values(sch_obj, "enum", ref->enum_values,
                          ref->n_enum_values);
  if (ref->default_value_set) {
    JSON_Value *def_val =
        (any_to_json_value(&ref->default_value, &_ast_any_to_json_value_13),
         _ast_any_to_json_value_13);
    if (def_val)
      json_object_set_value(sch_obj, "default", def_val);
  }
  if (ref->schema_extra_json)
    merge_schema_extras_object_openapi(sch_obj, ref->schema_extra_json);

  json_object_set_value(parent, key, sch_val);
}

/**
 * @brief Generates C code for write schema from type fields.
 */
C_CDD_EXPORT void write_schema_from_type_fields(JSON_Object *parent,
                                                const char *key,
                                                const char *type, int is_array,
                                                const char *items_type) {
  JSON_Value *sch_val = json_value_init_object();
  JSON_Object *sch_obj = json_value_get_object(sch_val);
  char ref_path[128];

  if (is_array) {
    json_object_set_string(sch_obj, "type", "array");
    if (items_type) {
      JSON_Value *item_val = json_value_init_object();
      JSON_Object *item_obj = json_value_get_object(item_val);
      if (is_schema_primitive_openapi(items_type)) {
        json_object_set_string(item_obj, "type", items_type);
      } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                  items_type);
#else
        CDD_SNPRINTF(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                     items_type);
#endif
        json_object_set_string(item_obj, "$ref", ref_path);
      }
      json_object_set_value(sch_obj, "items", item_val);
    }
    json_object_set_value(parent, key, sch_val);
    return;
  }

  if (type &&
      (is_schema_primitive_openapi(type) || strcmp(type, "array") == 0)) {
    json_object_set_string(sch_obj, "type", type);
  } else if (type) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(ref_path, sizeof(ref_path), "#/components/schemas/%s", type);
#else
    CDD_SNPRINTF(ref_path, sizeof(ref_path), "#/components/schemas/%s", type);
#endif
    json_object_set_string(sch_obj, "$ref", ref_path);
  } else {
    json_object_set_string(sch_obj, "type", "string");
  }

  json_object_set_value(parent, key, sch_val);
}
