/**
 * @file code2schema.c
 * @brief Implementation of C header parsing and code-to-schema conversion.
 * Refactored to usage `c_mapping` for type resolution.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#include "c_mapping.h"
#include "code2schema.h"
#include "codegen.h"
#include "codegen_struct.h"
#include "numeric_parser.h"
#include "str_utils.h"

#define MAX_LINE_LENGTH 1024

/* Helper to read a line from file and strip newline characters */
static int read_line(FILE *fp, char *buf, size_t bufsz) {
  if (!fgets(buf, (int)bufsz, fp))
    return 0;
  {
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
      buf[--len] = '\0';
  }
  return 1;
}

void trim_trailing(char *str) {
  size_t len;
  if (!str)
    return;
  c_cdd_str_trim_trailing_whitespace(str);
  len = strlen(str);
  while (len > 0 &&
         (str[len - 1] == ';' || isspace((unsigned char)str[len - 1]))) {
    str[--len] = '\0';
  }
}

bool str_starts_with(const char *str, const char *prefix) {
  return c_cdd_str_starts_with(str, prefix);
}

static int key_in_list(const char *key, const char *const *list, size_t count) {
  size_t i;
  if (!key || !list)
    return 0;
  for (i = 0; i < count; ++i) {
    if (list[i] && strcmp(list[i], key) == 0)
      return 1;
  }
  return 0;
}

static JSON_Value *clone_json_value(const JSON_Value *val) {
  char *serialized;
  JSON_Value *copy;

  if (!val)
    return NULL;
  serialized = json_serialize_to_string((JSON_Value *)val);
  if (!serialized)
    return NULL;
  copy = json_parse_string(serialized);
  json_free_serialized_string(serialized);
  return copy;
}

static void free_string_array(char **arr, size_t n) {
  size_t i;
  if (!arr)
    return;
  for (i = 0; i < n; ++i) {
    if (arr[i]) {
      free(arr[i]);
      arr[i] = NULL;
    }
  }
  free(arr);
}

static int copy_string_array(char ***dst, size_t *dst_count, char *const *src,
                             size_t src_count) {
  size_t i;
  char **out;
  if (!dst || !dst_count)
    return EINVAL;
  *dst = NULL;
  *dst_count = 0;
  if (!src || src_count == 0)
    return 0;
  out = (char **)calloc(src_count, sizeof(char *));
  if (!out)
    return ENOMEM;
  for (i = 0; i < src_count; ++i) {
    if (src[i]) {
      out[i] = c_cdd_strdup(src[i]);
      if (!out[i]) {
        free_string_array(out, src_count);
        return ENOMEM;
      }
    }
  }
  *dst = out;
  *dst_count = src_count;
  return 0;
}

static int parse_type_union_array(const JSON_Array *arr, char ***out_union,
                                  size_t *out_count, const char **out_primary,
                                  int *out_nullable) {
  size_t i, count, n = 0;
  char **types;
  const char *primary = NULL;
  int saw_null = 0;

  if (out_union)
    *out_union = NULL;
  if (out_count)
    *out_count = 0;
  if (out_primary)
    *out_primary = NULL;
  if (out_nullable)
    *out_nullable = 0;

  if (!arr)
    return 0;

  count = json_array_get_count(arr);
  if (count == 0)
    return 0;

  types = (char **)calloc(count, sizeof(char *));
  if (!types)
    return ENOMEM;

  for (i = 0; i < count; ++i) {
    const char *t = json_array_get_string(arr, i);
    if (!t)
      continue;
    types[n] = c_cdd_strdup(t);
    if (!types[n]) {
      free_string_array(types, count);
      return ENOMEM;
    }
    if (strcmp(t, "null") == 0) {
      saw_null = 1;
      if (out_nullable)
        *out_nullable = 1;
    } else if (!primary) {
      primary = types[n];
    }
    n++;
  }

  if (n == 0) {
    free(types);
    return 0;
  }

  if (!primary && saw_null)
    primary = "null";

  if (out_union)
    *out_union = types;
  if (out_count)
    *out_count = n;
  if (out_primary)
    *out_primary = primary;
  return 0;
}

static int collect_schema_extras(const JSON_Object *obj,
                                 const char *const *skip_keys,
                                 size_t skip_count, char **out_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
  char *serialized;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return EINVAL;

  extras_val = json_value_init_object();
  if (!extras_val)
    return ENOMEM;
  extras_obj = json_value_get_object(extras_val);

  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (!key || key_in_list(key, skip_keys, skip_count))
      continue;
    val = json_object_get_value(obj, key);
    copy = clone_json_value(val);
    if (!copy) {
      json_value_free(extras_val);
      return ENOMEM;
    }
    if (json_object_set_value(extras_obj, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return ENOMEM;
    }
  }

  if (json_object_get_count(extras_obj) == 0) {
    json_value_free(extras_val);
    return 0;
  }

  serialized = json_serialize_to_string(extras_val);
  if (!serialized) {
    json_value_free(extras_val);
    return ENOMEM;
  }
  *out_json = c_cdd_strdup(serialized);
  json_free_serialized_string(serialized);
  json_value_free(extras_val);
  return *out_json ? 0 : ENOMEM;
}

static int merge_schema_extras_object(JSON_Object *target,
                                      const char *extras_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;

  if (!target || !extras_json || extras_json[0] == '\0')
    return 0;

  extras_val = json_parse_string(extras_json);
  if (!extras_val)
    return 0;
  extras_obj = json_value_get_object(extras_val);
  if (!extras_obj) {
    json_value_free(extras_val);
    return 0;
  }

  count = json_object_get_count(extras_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(extras_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (!key || json_object_has_value(target, key))
      continue;
    val = json_object_get_value(extras_obj, key);
    copy = clone_json_value(val);
    if (!copy) {
      json_value_free(extras_val);
      return ENOMEM;
    }
    if (json_object_set_value(target, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return ENOMEM;
    }
  }

  json_value_free(extras_val);
  return 0;
}

static int merge_schema_extras_strings(char **dest_json, const char *src_json) {
  JSON_Value *dest_val;
  JSON_Value *src_val;
  JSON_Object *dest_obj;
  JSON_Object *src_obj;
  size_t i, count;
  char *serialized;

  if (!dest_json || !src_json || src_json[0] == '\0')
    return 0;
  if (!*dest_json) {
    *dest_json = c_cdd_strdup(src_json);
    return *dest_json ? 0 : ENOMEM;
  }

  dest_val = json_parse_string(*dest_json);
  src_val = json_parse_string(src_json);
  if (!dest_val || !src_val) {
    if (dest_val)
      json_value_free(dest_val);
    if (src_val)
      json_value_free(src_val);
    return 0;
  }
  dest_obj = json_value_get_object(dest_val);
  src_obj = json_value_get_object(src_val);
  if (!dest_obj || !src_obj) {
    json_value_free(dest_val);
    json_value_free(src_val);
    return 0;
  }

  count = json_object_get_count(src_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(src_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;
    if (!key || json_object_has_value(dest_obj, key))
      continue;
    val = json_object_get_value(src_obj, key);
    copy = clone_json_value(val);
    if (!copy) {
      json_value_free(dest_val);
      json_value_free(src_val);
      return ENOMEM;
    }
    if (json_object_set_value(dest_obj, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(dest_val);
      json_value_free(src_val);
      return ENOMEM;
    }
  }

  serialized = json_serialize_to_string(dest_val);
  if (!serialized) {
    json_value_free(dest_val);
    json_value_free(src_val);
    return ENOMEM;
  }
  {
    char *dup = c_cdd_strdup(serialized);
    json_free_serialized_string(serialized);
    if (!dup) {
      json_value_free(dest_val);
      json_value_free(src_val);
      return ENOMEM;
    }
    free(*dest_json);
    *dest_json = dup;
  }

  json_value_free(dest_val);
  json_value_free(src_val);
  return 0;
}

static const char *const k_schema_skip_keys[] = {
    "type", "$ref", "properties", "required", "allOf", "anyOf", "oneOf"};

static const char *const k_property_skip_keys[] = {"type",
                                                   "$ref",
                                                   "items",
                                                   "default",
                                                   "minimum",
                                                   "maximum",
                                                   "exclusiveMinimum",
                                                   "exclusiveMaximum",
                                                   "minLength",
                                                   "maxLength",
                                                   "pattern",
                                                   "minItems",
                                                   "maxItems",
                                                   "uniqueItems",
                                                   "description",
                                                   "format",
                                                   "deprecated",
                                                   "readOnly",
                                                   "writeOnly",
                                                   "x-c-bitwidth"};

static const char *const k_items_skip_keys[] = {"type", "$ref"};

static int openapi_type_is_primitive(const char *type) {
  if (!type)
    return 0;
  return strcmp(type, "integer") == 0 || strcmp(type, "number") == 0 ||
         strcmp(type, "string") == 0 || strcmp(type, "boolean") == 0;
}

static int schema_object_is_string_enum(const JSON_Object *schema_obj,
                                        const JSON_Array **enum_arr_out);
static int ref_points_to_string_enum(const JSON_Object *root, const char *ref);
static int required_name_in_list(const JSON_Array *required, const char *name);
static const JSON_Object *resolve_schema_ref_object(const JSON_Object *root,
                                                    const char *ref);
static int merge_struct_fields(struct StructFields *dest,
                               const struct StructFields *src);
static void merge_struct_field(struct StructField *dest,
                               const struct StructField *src);
static int apply_allof_to_struct_fields(const JSON_Array *all_of,
                                        struct StructFields *dest,
                                        const JSON_Object *root);
static int apply_union_to_struct_fields_fallback(const JSON_Array *union_arr,
                                                 struct StructFields *dest,
                                                 const JSON_Object *root);
static int apply_union_to_struct_fields_ex(
    const JSON_Array *union_arr, struct StructFields *dest, JSON_Object *root,
    const char *schema_name, int is_anyof, const JSON_Object *schema_obj,
    int allow_inline);

int parse_struct_member_line(const char *line, struct StructFields *sf) {
  char buf[MAX_LINE_LENGTH];
  char *last_space;
  char name[64] = {0};
  char type_raw[64] = {0};
  char bit_width[16] = {0};
  int is_fam = 0;
  int is_ptr = 0;
  char *colon_ptr = NULL;

  /* Helper for mapping result */
  struct OpenApiTypeMapping mapping;
  int rc;

  if (!line || !sf)
    return EINVAL;

  /* Basic heuristic parsing: "Type name;" or "Type name : width;" */
  strncpy(buf, line, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';
  trim_trailing(buf);

  /* Handle Bit-fields */
  colon_ptr = strrchr(buf, ':');
  if (colon_ptr) {
    char *w = colon_ptr + 1;
    while (*w && isspace((unsigned char)*w))
      w++;
    if (*w && (isdigit((unsigned char)*w) || *w == '(' ||
               isalpha((unsigned char)*w))) {
      strncpy(bit_width, w, sizeof(bit_width) - 1);
      *colon_ptr = '\0';
      trim_trailing(buf);
    }
  }

  last_space = strrchr(buf, ' ');
  if (!last_space) {
    /* Maybe it's "int*p;" without space? Parser assumes space separator. */
    /* Check for * split if no space */
    last_space = strrchr(buf, '*');
    if (!last_space)
      return 0; // Skip invalid
  }

  /* Extact Name */
  {
    char *n = last_space + 1;
    if (last_space[0] == '*')
      n = last_space; /* Treat * as part of name logic temporarility? No. */
    /* "int *p" -> last_space=' '. n="*p" */

    /* Re-evaluate split logic carefully */
    /* If strict space split: "int *p" -> type="int", name="*p" */
    *last_space = '\0'; /* Split string */

    /* n points to start of declared name (maybe with *) */
    while (*n == '*') {
      is_ptr = 1;
      n++;
    }
    strncpy(name, n, 63);

    /* Check FAM */
    {
      size_t nlen = strlen(name);
      if (nlen > 2 && name[nlen - 1] == ']' && name[nlen - 2] == '[') {
        is_fam = 1;
        name[nlen - 2] = '\0';
      }
    }
  }

  /* Extract Type Raw */
  /* If buf was "int *p", and we split at space, buf="int", name="*p". */
  /* If buf was "struct S* p", split at last space (' '), buf="struct S*",
   * name="p" */
  strncpy(type_raw, buf, 63);
  /* Ensure we capture the pointer asterisk if it was on the type side */
  /* "struct S* p" -> type="struct S*" */
  /* "struct S *p" -> type="struct S", name="*p" (handled above) */
  /* Reconstruct logical type for mapper: If name had *, append * to type_raw?
     Yes, C Mapping needs "Type *" to detect pointers correctly esp for
     arrays/strings */
  if (is_ptr) {
    /* Append * if not present */
    if (type_raw[strlen(type_raw) - 1] != '*') {
      strncat(type_raw, "*", 63 - strlen(type_raw));
    }
  }

  /* --- Use Mapper --- */
  rc = c_mapping_map_type(type_raw, name, &mapping);
  if (rc != 0)
    return rc;

  /* Translate Mapper Result to StructFields format */
  /* StructFields uses "type" string and "ref" string. */
  /* if kind==PRIMITIVE -> type=oa_type */
  /* if kind==OBJECT -> type="object", ref=ref_name */
  /* if kind==ARRAY -> type="array", ref=oa_type (if prim) or ref_name (if obj)
   */
  /* if CHAR* -> mapped to PRIMITIVE "string", handled correctly */

  {
    const char *final_type = "string";
    const char *final_ref = NULL;

    if (mapping.kind == OA_TYPE_PRIMITIVE) {
      final_type = mapping.oa_type;
    } else if (mapping.kind == OA_TYPE_OBJECT) {
      final_type = "object";
      final_ref = mapping.ref_name;
    } else if (mapping.kind == OA_TYPE_ARRAY) {
      final_type = "array";
      /* Item type goes in ref for StructFields logic currently */
      final_ref = mapping.ref_name ? mapping.ref_name : mapping.oa_type;
    }

    if (struct_fields_add(sf, name, final_type, final_ref, NULL,
                          bit_width[0] ? bit_width : NULL) == 0) {
      struct StructField *field = &sf->fields[sf->size - 1];
      if (is_fam) {
        field->is_flexible_array = 1;
      }
      if (mapping.oa_format && mapping.oa_type) {
        if (mapping.kind == OA_TYPE_PRIMITIVE) {
          strncpy(field->format, mapping.oa_format, sizeof(field->format) - 1);
          field->format[sizeof(field->format) - 1] = '\0';
        } else if (mapping.kind == OA_TYPE_ARRAY &&
                   openapi_type_is_primitive(mapping.oa_type)) {
          char fmt_json[64];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(fmt_json, sizeof(fmt_json), "{\"format\":\"%s\"}",
                    mapping.oa_format);
#else
          sprintf(fmt_json, "{\"format\":\"%s\"}", mapping.oa_format);
#endif
          if (merge_schema_extras_strings(&field->items_extra_json, fmt_json) !=
              0) {
            rc = ENOMEM;
          }
        }
      }
    } else {
      rc = ENOMEM;
    }
  }

  c_mapping_free(&mapping);
  return rc;
}

int json_array_to_enum_members(const JSON_Array *a, struct EnumMembers *e) {
  size_t i, count;
  if (!a || !e)
    return EINVAL;
  count = json_array_get_count(a);
  for (i = 0; i < count; i++) {
    const char *s = json_array_get_string(a, i);
    if (!s)
      continue;
    if (enum_members_add(e, s) != 0)
      return ENOMEM;
  }
  return 0;
}

static int json_object_to_struct_fields_internal(const JSON_Object *o,
                                                 struct StructFields *f,
                                                 JSON_Object *root,
                                                 const char *schema_name,
                                                 int allow_inline_union) {
  JSON_Object *props;
  size_t i, count;
  const JSON_Array *required;
  const JSON_Array *enum_arr;
  const JSON_Array *all_of;
  const JSON_Array *any_of;
  const JSON_Array *one_of;
  int rc;

  if (!o || !f)
    return EINVAL;

  if (collect_schema_extras(o, k_schema_skip_keys,
                            sizeof(k_schema_skip_keys) /
                                sizeof(k_schema_skip_keys[0]),
                            &f->schema_extra_json) != 0)
    return ENOMEM;

  if (schema_object_is_string_enum(o, &enum_arr)) {
    f->is_enum = 1;
    if (enum_members_init(&f->enum_members) != 0)
      return ENOMEM;
    if (json_array_to_enum_members(enum_arr, &f->enum_members) != 0)
      return ENOMEM;
    return 0;
  }

  all_of = json_object_get_array(o, "allOf");
  if (all_of) {
    rc = apply_allof_to_struct_fields(all_of, f, root);
    if (rc != 0)
      return rc;
  }

  any_of = json_object_get_array(o, "anyOf");
  if (any_of) {
    rc = apply_union_to_struct_fields_ex(any_of, f, root, schema_name, 1, o,
                                         allow_inline_union);
    if (rc != 0)
      return rc;
    if (!f->is_union) {
      rc = apply_union_to_struct_fields_fallback(any_of, f, root);
      if (rc != 0)
        return rc;
    }
  }

  one_of = json_object_get_array(o, "oneOf");
  if (one_of) {
    rc = apply_union_to_struct_fields_ex(one_of, f, root, schema_name, 0, o,
                                         allow_inline_union);
    if (rc != 0)
      return rc;
    if (!f->is_union) {
      rc = apply_union_to_struct_fields_fallback(one_of, f, root);
      if (rc != 0)
        return rc;
    }
  }

  if (f->is_union)
    return 0;

  required = json_object_get_array(o, "required");

  props = json_object_get_object(o, "properties");
  if (!props)
    return 0;

  count = json_object_get_count(props);
  for (i = 0; i < count; i++) {
    const char *name = json_object_get_name(props, i);
    JSON_Object *prop = json_object_get_object(props, name);
    const char *type = json_object_get_string(prop, "type");
    const JSON_Array *type_arr = NULL;
    const char *ref = json_object_get_string(prop, "$ref");
    const char *bw = json_object_get_string(prop, "x-c-bitwidth");
    char default_buf[128] = {0};
    const char *d_val = NULL;
    struct StructField *field = NULL;
    int field_added = 0;
    char **type_union = NULL;
    size_t n_type_union = 0;

    if (json_object_has_value_of_type(prop, "default", JSONString)) {
      const char *s = json_object_get_string(prop, "default");
      sprintf(default_buf, "\"%s\"", s);
      d_val = default_buf;
    } else if (json_object_has_value_of_type(prop, "default", JSONNumber)) {
      double d = json_object_get_number(prop, "default");
      if (type && strcmp(type, "integer") == 0)
        sprintf(default_buf, "%d", (int)d);
      else
        sprintf(default_buf, "%f", d);
      d_val = default_buf;
    } else if (json_object_has_value_of_type(prop, "default", JSONBoolean)) {
      int b = json_object_get_boolean(prop, "default");
      sprintf(default_buf, "%d", b);
      d_val = default_buf;
    }

    if (!type) {
      type_arr = json_object_get_array(prop, "type");
      if (type_arr) {
        const char *primary = NULL;
        rc = parse_type_union_array(type_arr, &type_union, &n_type_union,
                                    &primary, NULL);
        if (rc != 0) {
          free_string_array(type_union, n_type_union);
          return rc;
        }
        type = primary;
      }
    }

    if (type) {
      if (strcmp(type, "array") == 0) {
        JSON_Object *items = json_object_get_object(prop, "items");
        const char *item_type = json_object_get_string(items, "type");
        const JSON_Array *item_type_arr = NULL;
        char **items_type_union = NULL;
        size_t n_items_type_union = 0;
        const char *item_ref = json_object_get_string(items, "$ref");
        if (item_ref && ref_points_to_string_enum(root, item_ref)) {
          /* Enum arrays are not strongly typed yet; treat as string arrays */
          item_ref = NULL;
          item_type = "string";
        }
        if (!item_ref && !item_type && items) {
          item_type_arr = json_object_get_array(items, "type");
          if (item_type_arr) {
            const char *primary = NULL;
            rc = parse_type_union_array(item_type_arr, &items_type_union,
                                        &n_items_type_union, &primary, NULL);
            if (rc != 0) {
              free_string_array(type_union, n_type_union);
              free_string_array(items_type_union, n_items_type_union);
              return rc;
            }
            item_type = primary;
          }
        }
        if (struct_fields_add(f, name, "array", item_ref ? item_ref : item_type,
                              NULL, bw) != 0) {
          free_string_array(type_union, n_type_union);
          free_string_array(items_type_union, n_items_type_union);
          return ENOMEM;
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        if (type_union && n_type_union > 0) {
          field->type_union = type_union;
          field->n_type_union = n_type_union;
          type_union = NULL;
          n_type_union = 0;
        }
        if (items_type_union && n_items_type_union > 0) {
          field->items_type_union = items_type_union;
          field->n_items_type_union = n_items_type_union;
          items_type_union = NULL;
          n_items_type_union = 0;
        }
        if (items) {
          if (collect_schema_extras(items, k_items_skip_keys,
                                    sizeof(k_items_skip_keys) /
                                        sizeof(k_items_skip_keys[0]),
                                    &field->items_extra_json) != 0)
            return ENOMEM;
        }
        free_string_array(items_type_union, n_items_type_union);
      } else {
        if (struct_fields_add(f, name, type, ref, d_val, bw) != 0) {
          free_string_array(type_union, n_type_union);
          return ENOMEM;
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        if (type_union && n_type_union > 0) {
          field->type_union = type_union;
          field->n_type_union = n_type_union;
          type_union = NULL;
          n_type_union = 0;
        }
      }
    } else if (ref) {
      const char *field_type =
          ref_points_to_string_enum(root, ref) ? "enum" : "object";
      if (struct_fields_add(f, name, field_type, ref, NULL, bw) != 0) {
        free_string_array(type_union, n_type_union);
        return ENOMEM;
      }
      field = &f->fields[f->size - 1];
      field_added = 1;
    }

    if (!field_added) {
      free_string_array(type_union, n_type_union);
      continue;
    }

    if (required_name_in_list(required, name))
      field->required = 1;
    if (type && (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0)) {
      if (json_object_has_value_of_type(prop, "minimum", JSONNumber)) {
        field->has_min = 1;
        field->min_val = json_object_get_number(prop, "minimum");
      }
      if (json_object_has_value_of_type(prop, "exclusiveMinimum", JSONNumber)) {
        field->has_min = 1;
        field->min_val = json_object_get_number(prop, "exclusiveMinimum");
        field->exclusive_min = 1;
      } else if (json_object_has_value_of_type(prop, "exclusiveMinimum",
                                               JSONBoolean) &&
                 json_object_get_boolean(prop, "exclusiveMinimum")) {
        field->exclusive_min = 1;
      }

      if (json_object_has_value_of_type(prop, "maximum", JSONNumber)) {
        field->has_max = 1;
        field->max_val = json_object_get_number(prop, "maximum");
      }

      if (json_object_has_value_of_type(prop, "exclusiveMaximum", JSONNumber)) {
        field->has_max = 1;
        field->max_val = json_object_get_number(prop, "exclusiveMaximum");
        field->exclusive_max = 1;
      } else if (json_object_has_value_of_type(prop, "exclusiveMaximum",
                                               JSONBoolean) &&
                 json_object_get_boolean(prop, "exclusiveMaximum")) {
        field->exclusive_max = 1;
      }

    } else if (type && strcmp(type, "string") == 0) {
      if (json_object_has_value_of_type(prop, "minLength", JSONNumber)) {
        field->has_min_len = 1;
        field->min_len = (size_t)json_object_get_number(prop, "minLength");
      }

      if (json_object_has_value_of_type(prop, "maxLength", JSONNumber)) {
        field->has_max_len = 1;
        field->max_len = (size_t)json_object_get_number(prop, "maxLength");
      }

      if (json_object_has_value_of_type(prop, "pattern", JSONString)) {
        strncpy(field->pattern, json_object_get_string(prop, "pattern"),
                sizeof(field->pattern) - 1);
      }
    } else if (type && strcmp(type, "array") == 0) {
      if (json_object_has_value_of_type(prop, "minItems", JSONNumber)) {
        field->has_min_items = 1;
        field->min_items = (size_t)json_object_get_number(prop, "minItems");
      }

      if (json_object_has_value_of_type(prop, "maxItems", JSONNumber)) {
        field->has_max_items = 1;
        field->max_items = (size_t)json_object_get_number(prop, "maxItems");
      }

      if (json_object_has_value_of_type(prop, "uniqueItems", JSONBoolean)) {
        field->unique_items = json_object_get_boolean(prop, "uniqueItems");
      }
    }

    {
      const char *desc = json_object_get_string(prop, "description");
      const char *fmt = json_object_get_string(prop, "format");
      if (desc) {
        strncpy(field->description, desc, sizeof(field->description) - 1);
        field->description[sizeof(field->description) - 1] = '\0';
      }
      if (fmt) {
        strncpy(field->format, fmt, sizeof(field->format) - 1);
        field->format[sizeof(field->format) - 1] = '\0';
      }
      if (json_object_has_value(prop, "deprecated")) {
        field->deprecated_set = 1;
        field->deprecated = json_object_get_boolean(prop, "deprecated");
      }
      if (json_object_has_value(prop, "readOnly")) {
        field->read_only_set = 1;
        field->read_only = json_object_get_boolean(prop, "readOnly");
      }
      if (json_object_has_value(prop, "writeOnly")) {
        field->write_only_set = 1;
        field->write_only = json_object_get_boolean(prop, "writeOnly");
      }
    }

    if (collect_schema_extras(prop, k_property_skip_keys,
                              sizeof(k_property_skip_keys) /
                                  sizeof(k_property_skip_keys[0]),
                              &field->schema_extra_json) != 0)
      return ENOMEM;
  }
  return 0;
}

int json_object_to_struct_fields_ex(const JSON_Object *schema_obj,
                                    struct StructFields *fields,
                                    const JSON_Object *schemas_obj_root,
                                    const char *schema_name) {
  return json_object_to_struct_fields_internal(
      schema_obj, fields, (JSON_Object *)schemas_obj_root, schema_name, 0);
}

int json_object_to_struct_fields_ex_codegen(const JSON_Object *schema_obj,
                                            struct StructFields *fields,
                                            JSON_Object *schemas_obj_root,
                                            const char *schema_name) {
  return json_object_to_struct_fields_internal(
      schema_obj, fields, schemas_obj_root, schema_name, 1);
}

int json_object_to_struct_fields(const JSON_Object *schema_obj,
                                 struct StructFields *fields,
                                 const JSON_Object *schemas_obj_root) {
  return json_object_to_struct_fields_ex(schema_obj, fields, schemas_obj_root,
                                         NULL);
}

static const char *strip_quotes(const char *in, char *buf, size_t bufsz) {
  size_t len;
  if (!in || !buf || bufsz == 0)
    return in;
  len = strlen(in);
  if (len >= 2 && in[0] == '"' && in[len - 1] == '"') {
    size_t inner = len - 2;
    if (inner >= bufsz)
      inner = bufsz - 1;
    memcpy(buf, in + 1, inner);
    buf[inner] = '\0';
    return buf;
  }
  return in;
}

static int parse_bool_default(const char *in, int *out) {
  if (!in || !out)
    return 0;
  if (strcmp(in, "true") == 0 || strcmp(in, "1") == 0) {
    *out = 1;
    return 1;
  }
  if (strcmp(in, "false") == 0 || strcmp(in, "0") == 0) {
    *out = 0;
    return 1;
  }
  return 0;
}

static int parse_number_default(const char *in, double *out) {
  struct NumericValue nv;
  if (!in || !out)
    return 0;
  if (parse_numeric_literal(in, &nv) == 0) {
    if (nv.kind == NUMERIC_INTEGER) {
      *out = (double)nv.data.integer.value;
      return 1;
    }
    if (nv.kind == NUMERIC_FLOAT) {
      *out = nv.data.floating.value;
      return 1;
    }
  }
  return 0;
}

static int schema_object_is_string_enum(const JSON_Object *schema_obj,
                                        const JSON_Array **enum_arr_out) {
  const JSON_Array *enum_arr;
  size_t i, count;
  const char *type;

  if (enum_arr_out)
    *enum_arr_out = NULL;
  if (!schema_obj)
    return 0;

  enum_arr = json_object_get_array(schema_obj, "enum");
  if (!enum_arr)
    return 0;

  count = json_array_get_count(enum_arr);
  if (count == 0)
    return 0;

  type = json_object_get_string(schema_obj, "type");
  if (type && strcmp(type, "string") != 0)
    return 0;

  for (i = 0; i < count; ++i) {
    if (!json_array_get_string(enum_arr, i))
      return 0;
  }

  if (enum_arr_out)
    *enum_arr_out = enum_arr;
  return 1;
}

static int ref_points_to_string_enum(const JSON_Object *root, const char *ref) {
  const char *name;
  const JSON_Object *schema_obj;
  if (!root || !ref)
    return 0;
  name = c_cdd_str_after_last(ref, '/');
  if (!name || !*name)
    return 0;
  schema_obj = json_object_get_object(root, name);
  if (!schema_obj)
    return 0;
  return schema_object_is_string_enum(schema_obj, NULL);
}

static int required_name_in_list(const JSON_Array *required, const char *name) {
  size_t i, count;
  if (!required || !name)
    return 0;
  count = json_array_get_count(required);
  for (i = 0; i < count; ++i) {
    const char *req_name = json_array_get_string(required, i);
    if (req_name && strcmp(req_name, name) == 0)
      return 1;
  }
  return 0;
}

static const JSON_Object *resolve_schema_ref_object(const JSON_Object *root,
                                                    const char *ref) {
  const char *name;
  if (!root || !ref)
    return NULL;
  name = c_cdd_str_after_last(ref, '/');
  if (!name || !*name)
    return NULL;
  return json_object_get_object(root, name);
}

static enum UnionVariantJsonType
detect_union_json_type(const JSON_Object *schema_obj) {
  const char *type;
  const JSON_Object *props;
  if (!schema_obj)
    return UNION_JSON_UNKNOWN;
  if (schema_object_is_string_enum(schema_obj, NULL))
    return UNION_JSON_STRING;
  type = json_object_get_string(schema_obj, "type");
  if (type) {
    if (strcmp(type, "object") == 0)
      return UNION_JSON_OBJECT;
    if (strcmp(type, "string") == 0)
      return UNION_JSON_STRING;
    if (strcmp(type, "integer") == 0)
      return UNION_JSON_INTEGER;
    if (strcmp(type, "number") == 0)
      return UNION_JSON_NUMBER;
    if (strcmp(type, "boolean") == 0)
      return UNION_JSON_BOOLEAN;
    if (strcmp(type, "array") == 0)
      return UNION_JSON_ARRAY;
    if (strcmp(type, "null") == 0)
      return UNION_JSON_NULL;
  }
  props = json_object_get_object(schema_obj, "properties");
  if (props)
    return UNION_JSON_OBJECT;
  return UNION_JSON_UNKNOWN;
}

static int collect_string_array(const JSON_Array *arr, char ***out,
                                size_t *out_count) {
  size_t i, count;
  char **vals;
  if (!out || !out_count)
    return EINVAL;
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return 0;
  count = json_array_get_count(arr);
  if (count == 0)
    return 0;
  vals = (char **)calloc(count, sizeof(char *));
  if (!vals)
    return ENOMEM;
  for (i = 0; i < count; ++i) {
    const char *s = json_array_get_string(arr, i);
    if (s) {
      vals[i] = c_cdd_strdup(s);
      if (!vals[i]) {
        free_string_array(vals, count);
        return ENOMEM;
      }
    }
  }
  *out = vals;
  *out_count = count;
  return 0;
}

static int collect_property_names(const JSON_Object *schema_obj, char ***out,
                                  size_t *out_count) {
  size_t i, count;
  char **vals;
  const JSON_Object *props;
  if (!out || !out_count)
    return EINVAL;
  *out = NULL;
  *out_count = 0;
  if (!schema_obj)
    return 0;
  props = json_object_get_object(schema_obj, "properties");
  if (!props)
    return 0;
  count = json_object_get_count(props);
  if (count == 0)
    return 0;
  vals = (char **)calloc(count, sizeof(char *));
  if (!vals)
    return ENOMEM;
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(props, i);
    if (name) {
      vals[i] = c_cdd_strdup(name);
      if (!vals[i]) {
        free_string_array(vals, count);
        return ENOMEM;
      }
    }
  }
  *out = vals;
  *out_count = count;
  return 0;
}

static char *sanitize_identifier(const char *in) {
  size_t i, len;
  char *out;
  if (!in || !*in)
    return c_cdd_strdup("Variant");
  len = strlen(in);
  out = (char *)calloc(len + 1, sizeof(char));
  if (!out)
    return NULL;
  for (i = 0; i < len; ++i) {
    const char c = in[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_') {
      out[i] = c;
    } else {
      out[i] = '_';
    }
  }
  if (!out[0]) {
    free(out);
    return c_cdd_strdup("Variant");
  }
  return out;
}

static char *make_unique_variant_name(const struct StructFields *dest,
                                      const char *base, size_t index) {
  char buf[128];
  char *sanitized;
  char *out;
  if (!dest)
    return NULL;
  sanitized = sanitize_identifier(base);
  if (!sanitized)
    return NULL;
  if (!struct_fields_get(dest, sanitized))
    return sanitized;
  snprintf(buf, sizeof(buf), "%s_%zu", sanitized, index + 1);
  free(sanitized);
  out = c_cdd_strdup(buf);
  if (!out)
    return NULL;
  if (!struct_fields_get(dest, out))
    return out;
  free(out);
  snprintf(buf, sizeof(buf), "Variant_%zu", index + 1);
  return c_cdd_strdup(buf);
}

static char *make_inline_schema_name(const char *schema_name,
                                     const char *variant_name,
                                     const char *suffix) {
  char buf[256];
  const char *base_schema =
      (schema_name && *schema_name) ? schema_name : "Union";
  const char *base_variant =
      (variant_name && *variant_name) ? variant_name : "Variant";
  if (suffix && *suffix)
    snprintf(buf, sizeof(buf), "%s_%s_%s", base_schema, base_variant, suffix);
  else
    snprintf(buf, sizeof(buf), "%s_%s", base_schema, base_variant);
  return sanitize_identifier(buf);
}

static int register_inline_schema(JSON_Object *root, const char *schema_name,
                                  const char *variant_name, const char *suffix,
                                  const JSON_Value *schema_val,
                                  char **out_name) {
  char *name;

  if (!root || !schema_val || !out_name)
    return EINVAL;

  name = make_inline_schema_name(schema_name, variant_name, suffix);
  if (!name)
    return ENOMEM;

  if (!json_object_has_value(root, name)) {
    JSON_Value *copy = clone_json_value(schema_val);
    if (!copy) {
      free(name);
      return ENOMEM;
    }
    if (json_object_set_value(root, name, copy) != JSONSuccess) {
      json_value_free(copy);
      free(name);
      return ENOMEM;
    }
  }

  *out_name = name;
  return 0;
}

static char *discriminator_value_for_variant(const JSON_Object *disc_obj,
                                             const char *schema_name,
                                             const char *ref) {
  const JSON_Object *mapping;
  size_t i, count;
  const char *ref_name;

  if (!schema_name && !ref)
    return NULL;

  ref_name = ref ? c_cdd_str_after_last(ref, '/') : NULL;

  if (!disc_obj)
    goto fallback;

  mapping = json_object_get_object(disc_obj, "mapping");
  if (!mapping)
    goto fallback;

  count = json_object_get_count(mapping);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(mapping, i);
    const char *val = json_object_get_string(mapping, key);
    if (!key || !val)
      continue;
    if (ref && strcmp(val, ref) == 0)
      return c_cdd_strdup(key);
    if (ref_name && strcmp(val, ref_name) == 0)
      return c_cdd_strdup(key);
    if (schema_name && strcmp(val, schema_name) == 0)
      return c_cdd_strdup(key);
  }

fallback:
  if (schema_name)
    return c_cdd_strdup(schema_name);
  if (ref_name)
    return c_cdd_strdup(ref_name);
  return NULL;
}

static void merge_struct_field(struct StructField *dest,
                               const struct StructField *src) {
  if (!dest || !src)
    return;

  if (!dest->default_val[0] && src->default_val[0]) {
    strncpy(dest->default_val, src->default_val, sizeof(dest->default_val) - 1);
    dest->default_val[sizeof(dest->default_val) - 1] = '\0';
  }

  if (src->required)
    dest->required = 1;

  if (src->has_min) {
    if (!dest->has_min || src->min_val > dest->min_val ||
        (src->min_val == dest->min_val && src->exclusive_min &&
         !dest->exclusive_min)) {
      dest->has_min = 1;
      dest->min_val = src->min_val;
      dest->exclusive_min = src->exclusive_min;
    }
  }

  if (src->has_max) {
    if (!dest->has_max || src->max_val < dest->max_val ||
        (src->max_val == dest->max_val && src->exclusive_max &&
         !dest->exclusive_max)) {
      dest->has_max = 1;
      dest->max_val = src->max_val;
      dest->exclusive_max = src->exclusive_max;
    }
  }

  if (src->has_min_len) {
    if (!dest->has_min_len || src->min_len > dest->min_len) {
      dest->has_min_len = 1;
      dest->min_len = src->min_len;
    }
  }

  if (src->has_max_len) {
    if (!dest->has_max_len || src->max_len < dest->max_len) {
      dest->has_max_len = 1;
      dest->max_len = src->max_len;
    }
  }

  if (src->has_min_items) {
    if (!dest->has_min_items || src->min_items > dest->min_items) {
      dest->has_min_items = 1;
      dest->min_items = src->min_items;
    }
  }

  if (src->has_max_items) {
    if (!dest->has_max_items || src->max_items < dest->max_items) {
      dest->has_max_items = 1;
      dest->max_items = src->max_items;
    }
  }

  if (src->unique_items)
    dest->unique_items = 1;

  if (!dest->pattern[0] && src->pattern[0]) {
    strncpy(dest->pattern, src->pattern, sizeof(dest->pattern) - 1);
    dest->pattern[sizeof(dest->pattern) - 1] = '\0';
  }

  if (src->is_flexible_array)
    dest->is_flexible_array = 1;

  if (!dest->bit_width[0] && src->bit_width[0]) {
    strncpy(dest->bit_width, src->bit_width, sizeof(dest->bit_width) - 1);
    dest->bit_width[sizeof(dest->bit_width) - 1] = '\0';
  }

  if (merge_schema_extras_strings(&dest->schema_extra_json,
                                  src->schema_extra_json) != 0) {
    /* Best-effort: ignore merge failures */
  }
  if (merge_schema_extras_strings(&dest->items_extra_json,
                                  src->items_extra_json) != 0) {
    /* Best-effort: ignore merge failures */
  }

  if (!dest->type_union && src->type_union && src->n_type_union > 0) {
    if (copy_string_array(&dest->type_union, &dest->n_type_union,
                          src->type_union, src->n_type_union) != 0) {
      /* Best-effort: ignore copy failures */
    }
  }
  if (!dest->items_type_union && src->items_type_union &&
      src->n_items_type_union > 0) {
    if (copy_string_array(&dest->items_type_union, &dest->n_items_type_union,
                          src->items_type_union,
                          src->n_items_type_union) != 0) {
      /* Best-effort: ignore copy failures */
    }
  }
}

static int merge_struct_fields(struct StructFields *dest,
                               const struct StructFields *src) {
  size_t i;

  if (!dest || !src)
    return 0;

  if (src->is_enum)
    return 0;

  if (merge_schema_extras_strings(&dest->schema_extra_json,
                                  src->schema_extra_json) != 0)
    return ENOMEM;

  for (i = 0; i < src->size; ++i) {
    const struct StructField *src_field = &src->fields[i];
    struct StructField *dest_field = struct_fields_get(dest, src_field->name);

    if (!dest_field) {
      const char *ref = src_field->ref[0] ? src_field->ref : NULL;
      const char *def =
          src_field->default_val[0] ? src_field->default_val : NULL;
      const char *bw = src_field->bit_width[0] ? src_field->bit_width : NULL;
      if (struct_fields_add(dest, src_field->name, src_field->type, ref, def,
                            bw) != 0)
        return ENOMEM;
      dest_field = struct_fields_get(dest, src_field->name);
      if (!dest_field)
        return ENOMEM;
      {
        struct StructField tmp = *src_field;
        *dest_field = tmp;
        dest_field->schema_extra_json = NULL;
        dest_field->items_extra_json = NULL;
        dest_field->type_union = NULL;
        dest_field->n_type_union = 0;
        dest_field->items_type_union = NULL;
        dest_field->n_items_type_union = 0;
        if (src_field->schema_extra_json) {
          dest_field->schema_extra_json =
              c_cdd_strdup(src_field->schema_extra_json);
          if (!dest_field->schema_extra_json)
            return ENOMEM;
        }
        if (src_field->items_extra_json) {
          dest_field->items_extra_json =
              c_cdd_strdup(src_field->items_extra_json);
          if (!dest_field->items_extra_json)
            return ENOMEM;
        }
        if (src_field->type_union && src_field->n_type_union > 0) {
          if (copy_string_array(
                  &dest_field->type_union, &dest_field->n_type_union,
                  src_field->type_union, src_field->n_type_union) != 0)
            return ENOMEM;
        }
        if (src_field->items_type_union && src_field->n_items_type_union > 0) {
          if (copy_string_array(&dest_field->items_type_union,
                                &dest_field->n_items_type_union,
                                src_field->items_type_union,
                                src_field->n_items_type_union) != 0)
            return ENOMEM;
        }
      }
      continue;
    }

    merge_struct_field(dest_field, src_field);
  }

  return 0;
}

static int apply_allof_to_struct_fields(const JSON_Array *all_of,
                                        struct StructFields *dest,
                                        const JSON_Object *root) {
  size_t i, count;

  if (!all_of || !dest)
    return 0;

  count = json_array_get_count(all_of);
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(all_of, i);
    const JSON_Object *resolved = sub;
    const char *ref;
    struct StructFields tmp;
    int rc;

    if (!sub)
      continue;

    ref = json_object_get_string(sub, "$ref");
    if (ref && root)
      resolved = resolve_schema_ref_object(root, ref);
    if (!resolved)
      continue;

    rc = struct_fields_init(&tmp);
    if (rc != 0)
      return rc;

    rc = json_object_to_struct_fields(resolved, &tmp, root);
    if (rc != 0) {
      struct_fields_free(&tmp);
      return rc;
    }

    rc = merge_struct_fields(dest, &tmp);
    struct_fields_free(&tmp);
    if (rc != 0)
      return rc;
  }

  return 0;
}

static int apply_union_to_struct_fields_fallback(const JSON_Array *union_arr,
                                                 struct StructFields *dest,
                                                 const JSON_Object *root) {
  size_t i, count;

  if (!union_arr || !dest)
    return 0;

  if (dest->size > 0)
    return 0;

  count = json_array_get_count(union_arr);
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(union_arr, i);
    const JSON_Object *resolved = sub;
    const char *ref;
    struct StructFields tmp;
    int rc;

    if (!sub)
      continue;

    ref = json_object_get_string(sub, "$ref");
    if (ref && root)
      resolved = resolve_schema_ref_object(root, ref);
    if (!resolved)
      continue;

    rc = struct_fields_init(&tmp);
    if (rc != 0)
      return rc;

    rc = json_object_to_struct_fields(resolved, &tmp, root);
    if (rc != 0) {
      struct_fields_free(&tmp);
      return rc;
    }

    if (tmp.size > 0) {
      rc = merge_struct_fields(dest, &tmp);
      struct_fields_free(&tmp);
      return rc;
    }

    struct_fields_free(&tmp);
  }

  return 0;
}

static int union_array_items_supported(const JSON_Object *schema_obj,
                                       const JSON_Object *root,
                                       int allow_inline) {
  const JSON_Object *items;
  const JSON_Array *item_type_arr = NULL;
  const char *item_ref;
  const char *item_type;
  char **items_type_union = NULL;
  size_t n_items_type_union = 0;
  const char *primary = NULL;

  if (!schema_obj)
    return 0;

  items = json_object_get_object(schema_obj, "items");
  if (!items)
    return 0;

  item_ref = json_object_get_string(items, "$ref");
  if (item_ref && root && ref_points_to_string_enum(root, item_ref))
    return 1;

  item_type = json_object_get_string(items, "type");
  if (!item_ref && !item_type) {
    item_type_arr = json_object_get_array(items, "type");
    if (item_type_arr) {
      if (parse_type_union_array(item_type_arr, &items_type_union,
                                 &n_items_type_union, &primary, NULL) == 0)
        item_type = primary;
    } else if (json_object_get_object(items, "properties")) {
      item_type = "object";
    }
  }

  if (item_ref) {
    free_string_array(items_type_union, n_items_type_union);
    return 1;
  }
  if (!item_type) {
    free_string_array(items_type_union, n_items_type_union);
    return 0;
  }
  if (strcmp(item_type, "array") == 0) {
    free_string_array(items_type_union, n_items_type_union);
    return 0;
  }
  if (strcmp(item_type, "object") == 0 && (!allow_inline || !root)) {
    free_string_array(items_type_union, n_items_type_union);
    return 0;
  }

  free_string_array(items_type_union, n_items_type_union);
  return 1;
}

static int apply_union_to_struct_fields_ex(
    const JSON_Array *union_arr, struct StructFields *dest, JSON_Object *root,
    const char *schema_name, int is_anyof, const JSON_Object *schema_obj,
    int allow_inline) {
  size_t i, count;
  const JSON_Object *disc_obj;

  if (!union_arr || !dest)
    return 0;

  if (dest->size > 0)
    return 0;

  count = json_array_get_count(union_arr);
  if (count == 0)
    return 0;

  /* Validate variant support */
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(union_arr, i);
    const JSON_Object *resolved = sub;
    const char *ref;
    enum UnionVariantJsonType jtype;
    if (!sub)
      continue;
    ref = json_object_get_string(sub, "$ref");
    if (ref && root)
      resolved = resolve_schema_ref_object(root, ref);
    jtype = detect_union_json_type(resolved);
    if (jtype == UNION_JSON_ARRAY) {
      if (!allow_inline)
        return 0;
      if (!union_array_items_supported(resolved, root, allow_inline))
        return 0;
    }
    if (jtype == UNION_JSON_OBJECT && !ref) {
      if (!allow_inline || !root)
        return 0;
      if (!json_array_get_value(union_arr, i))
        return 0;
    }
    if (jtype == UNION_JSON_UNKNOWN)
      return 0;
  }

  dest->is_union = 1;
  dest->union_is_anyof = is_anyof ? 1 : 0;

  disc_obj =
      schema_obj ? json_object_get_object(schema_obj, "discriminator") : NULL;
  if (disc_obj) {
    const char *prop = json_object_get_string(disc_obj, "propertyName");
    if (prop && *prop) {
      dest->union_discriminator = c_cdd_strdup(prop);
      if (!dest->union_discriminator)
        return ENOMEM;
    }
  }

  dest->union_variants =
      (struct UnionVariantMeta *)calloc(count, sizeof(struct UnionVariantMeta));
  if (!dest->union_variants)
    return ENOMEM;
  dest->n_union_variants = count;

  for (i = 0; i < count; ++i) {
    const JSON_Value *sub_val = json_array_get_value(union_arr, i);
    const JSON_Object *sub = json_value_get_object(sub_val);
    const JSON_Object *resolved = sub;
    const char *ref;
    enum UnionVariantJsonType jtype;
    const char *type_name = NULL;
    const char *name_hint = NULL;
    char *variant_name = NULL;
    char *inline_ref_name = NULL;
    char *inline_item_ref = NULL;
    const char *item_ref = NULL;
    const char *item_type = NULL;
    char **items_type_union = NULL;
    size_t n_items_type_union = 0;
    struct StructField *field = NULL;
    struct UnionVariantMeta *meta = &dest->union_variants[i];
    int rc;

    if (!sub)
      continue;

    ref = json_object_get_string(sub, "$ref");
    if (ref && root)
      resolved = resolve_schema_ref_object(root, ref);

    jtype = detect_union_json_type(resolved);
    meta->json_type = jtype;

    if (ref) {
      name_hint = c_cdd_str_after_last(ref, '/');
    } else if (resolved) {
      name_hint = json_object_get_string(resolved, "title");
      if (!name_hint)
        name_hint = json_object_get_string(resolved, "type");
    }
    if (!name_hint)
      name_hint = schema_name ? schema_name : "Variant";

    variant_name = make_unique_variant_name(dest, name_hint, i);
    if (!variant_name)
      return ENOMEM;

    if (jtype == UNION_JSON_OBJECT && !ref) {
      if (allow_inline && root && sub_val) {
        rc = register_inline_schema(root, schema_name, variant_name, NULL,
                                    sub_val, &inline_ref_name);
        if (rc != 0) {
          free(variant_name);
          return rc;
        }
        ref = inline_ref_name;
      }
    }

    if (jtype == UNION_JSON_ARRAY && resolved) {
      const JSON_Object *items = json_object_get_object(resolved, "items");
      const JSON_Value *items_val = json_object_get_value(resolved, "items");
      const JSON_Array *item_type_arr = NULL;

      if (items) {
        item_ref = json_object_get_string(items, "$ref");
        if (item_ref && root && ref_points_to_string_enum(root, item_ref)) {
          item_ref = NULL;
          item_type = "string";
        }
        item_type = json_object_get_string(items, "type");
        if (!item_ref && !item_type) {
          item_type_arr = json_object_get_array(items, "type");
          if (item_type_arr) {
            rc = parse_type_union_array(item_type_arr, &items_type_union,
                                        &n_items_type_union, &item_type, NULL);
            if (rc != 0) {
              free(variant_name);
              free(inline_ref_name);
              return rc;
            }
          } else if (json_object_get_object(items, "properties")) {
            item_type = "object";
          }
        }
        if (!item_ref && item_type && strcmp(item_type, "object") == 0) {
          if (allow_inline && root && items_val) {
            rc = register_inline_schema(root, schema_name, variant_name, "Item",
                                        items_val, &inline_item_ref);
            if (rc != 0) {
              free(variant_name);
              free(inline_ref_name);
              free_string_array(items_type_union, n_items_type_union);
              return rc;
            }
            item_ref = inline_item_ref;
          }
        }
      }
    }

    switch (jtype) {
    case UNION_JSON_OBJECT:
      type_name = "object";
      break;
    case UNION_JSON_STRING:
      type_name = "string";
      break;
    case UNION_JSON_INTEGER:
      type_name = "integer";
      break;
    case UNION_JSON_NUMBER:
      type_name = "number";
      break;
    case UNION_JSON_BOOLEAN:
      type_name = "boolean";
      break;
    case UNION_JSON_ARRAY:
      type_name = "array";
      break;
    case UNION_JSON_NULL:
      type_name = "null";
      break;
    default:
      type_name = "object";
      break;
    }

    if (struct_fields_add(dest, variant_name, type_name,
                          (jtype == UNION_JSON_OBJECT) ? ref
                          : (jtype == UNION_JSON_ARRAY)
                              ? (item_ref ? item_ref : item_type)
                              : NULL,
                          NULL, NULL) != 0) {
      free(variant_name);
      free(inline_ref_name);
      free(inline_item_ref);
      free_string_array(items_type_union, n_items_type_union);
      return ENOMEM;
    }
    field = &dest->fields[dest->size - 1];

    if (jtype == UNION_JSON_ARRAY && items_type_union &&
        n_items_type_union > 0) {
      field->items_type_union = items_type_union;
      field->n_items_type_union = n_items_type_union;
      items_type_union = NULL;
      n_items_type_union = 0;
    }

    free(variant_name);

    if (jtype == UNION_JSON_OBJECT && resolved) {
      const JSON_Array *required = json_object_get_array(resolved, "required");
      if (collect_string_array(required, &meta->required_props,
                               &meta->n_required_props) != 0)
        return ENOMEM;
      if (collect_property_names(resolved, &meta->property_names,
                                 &meta->n_property_names) != 0)
        return ENOMEM;
    }

    meta->disc_value = discriminator_value_for_variant(
        disc_obj, (ref ? c_cdd_str_after_last(ref, '/') : name_hint), ref);

    free(inline_ref_name);
    free(inline_item_ref);
    free_string_array(items_type_union, n_items_type_union);
  }

  return 0;
}

static void write_default_value(JSON_Object *pobj,
                                const struct StructField *field) {
  const char *def;
  const char *typ;
  char buf[256];
  int bval;
  double nval;
  JSON_Value *null_val;

  if (!pobj || !field)
    return;
  def = field->default_val;
  if (!def || def[0] == '\0')
    return;
  typ = field->type;

  if (strcmp(def, "nullptr") == 0) {
    null_val = json_value_init_null();
    if (null_val)
      json_object_set_value(pobj, "default", null_val);
    return;
  }

  if (typ && strcmp(typ, "string") == 0) {
    const char *s = strip_quotes(def, buf, sizeof(buf));
    json_object_set_string(pobj, "default", s ? s : "");
    return;
  }

  if (typ && strcmp(typ, "boolean") == 0) {
    if (parse_bool_default(def, &bval))
      json_object_set_boolean(pobj, "default", bval);
    return;
  }

  if (typ && (strcmp(typ, "integer") == 0 || strcmp(typ, "number") == 0)) {
    if (parse_number_default(def, &nval))
      json_object_set_number(pobj, "default", nval);
    return;
  }
}

static void write_numeric_constraints(JSON_Object *pobj,
                                      const struct StructField *field) {
  if (!pobj || !field)
    return;
  if (!field->type || !(strcmp(field->type, "integer") == 0 ||
                        strcmp(field->type, "number") == 0))
    return;
  if (field->has_min) {
    if (field->exclusive_min)
      json_object_set_number(pobj, "exclusiveMinimum", field->min_val);
    else
      json_object_set_number(pobj, "minimum", field->min_val);
  }
  if (field->has_max) {
    if (field->exclusive_max)
      json_object_set_number(pobj, "exclusiveMaximum", field->max_val);
    else
      json_object_set_number(pobj, "maximum", field->max_val);
  }
}

static void write_string_constraints(JSON_Object *pobj,
                                     const struct StructField *field) {
  if (!pobj || !field)
    return;
  if (!field->type || strcmp(field->type, "string") != 0)
    return;
  if (field->has_min_len)
    json_object_set_number(pobj, "minLength", (double)field->min_len);
  if (field->has_max_len)
    json_object_set_number(pobj, "maxLength", (double)field->max_len);
  if (field->pattern[0] != '\0')
    json_object_set_string(pobj, "pattern", field->pattern);
}

static void write_array_constraints(JSON_Object *pobj,
                                    const struct StructField *field) {
  if (!pobj || !field)
    return;
  if (!field->type || strcmp(field->type, "array") != 0)
    return;
  if (field->has_min_items)
    json_object_set_number(pobj, "minItems", (double)field->min_items);
  if (field->has_max_items)
    json_object_set_number(pobj, "maxItems", (double)field->max_items);
  if (field->unique_items)
    json_object_set_boolean(pobj, "uniqueItems", 1);
}

static void write_type_union(JSON_Object *obj, const char *type,
                             char **type_union, size_t n_type_union) {
  size_t i;
  JSON_Value *arr_val;
  JSON_Array *arr;

  if (!obj)
    return;

  if (type_union && n_type_union > 0) {
    arr_val = json_value_init_array();
    if (!arr_val)
      return;
    arr = json_value_get_array(arr_val);
    for (i = 0; i < n_type_union; ++i) {
      if (type_union[i])
        json_array_append_string(arr, type_union[i]);
    }
    json_object_set_value(obj, "type", arr_val);
    return;
  }

  if (type)
    json_object_set_string(obj, "type", type);
}

int write_struct_to_json_schema(JSON_Object *schemas_obj,
                                const char *struct_name,
                                const struct StructFields *sf) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  JSON_Value *props_val = json_value_init_object();
  JSON_Object *props_obj = json_value_get_object(props_val);
  JSON_Value *req_val = NULL;
  JSON_Array *req_arr = NULL;
  size_t i;

  if (!schemas_obj || !struct_name || !sf) {
    json_value_free(val);
    json_value_free(props_val);
    return EINVAL;
  }

  if (sf->is_enum) {
    JSON_Value *enum_val = json_value_init_array();
    JSON_Array *enum_arr = json_value_get_array(enum_val);
    json_object_set_string(obj, "type", "string");
    for (i = 0; i < sf->enum_members.size; ++i) {
      const char *member = sf->enum_members.members[i];
      if (member)
        json_array_append_string(enum_arr, member);
    }
    json_object_set_value(obj, "enum", enum_val);
    if (merge_schema_extras_object(obj, sf->schema_extra_json) != 0) {
      json_value_free(val);
      json_value_free(props_val);
      return ENOMEM;
    }
    json_object_set_value(schemas_obj, struct_name, val);
    json_value_free(props_val);
    return 0;
  }

  json_object_set_string(obj, "type", "object");
  json_object_set_value(obj, "properties", props_val);

  for (i = 0; i < sf->size; i++) {
    JSON_Value *pval = json_value_init_object();
    JSON_Object *pobj = json_value_get_object(pval);
    const struct StructField *field = &sf->fields[i];
    const char *typ = field->type;
    const char *ref = field->ref;
    const char *bw = field->bit_width;

    if (bw && *bw) {
      json_object_set_string(pobj, "x-c-bitwidth", bw);
    }

    if (strcmp(typ, "array") == 0) {
      JSON_Value *items_val = json_value_init_object();
      JSON_Object *items_obj = json_value_get_object(items_val);
      write_type_union(pobj, "array", field->type_union, field->n_type_union);
      if (field->items_type_union && field->n_items_type_union > 0) {
        write_type_union(items_obj, ref, field->items_type_union,
                         field->n_items_type_union);
      } else if (ref && *ref) {
        if (strcmp(ref, "integer") == 0 || strcmp(ref, "string") == 0 ||
            strcmp(ref, "boolean") == 0 || strcmp(ref, "number") == 0) {
          json_object_set_string(items_obj, "type", ref);
        } else {
          char ref_str[128];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(ref_str, sizeof(ref_str), "#/components/schemas/%s", ref);
#else
          sprintf(ref_str, "#/components/schemas/%s", ref);
#endif
          json_object_set_string(items_obj, "$ref", ref_str);
        }
      }
      if (merge_schema_extras_object(items_obj, field->items_extra_json) != 0) {
        json_value_free(items_val);
        json_value_free(pval);
        json_value_free(val);
        return ENOMEM;
      }
      json_object_set_value(pobj, "items", items_val);
      write_array_constraints(pobj, field);
    } else {
      if (strcmp(typ, "object") == 0 || strcmp(typ, "enum") == 0) {
        char ref_str[128];
        if (ref && *ref) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(ref_str, sizeof(ref_str), "#/components/schemas/%s", ref);
#else
          sprintf(ref_str, "#/components/schemas/%s", ref);
#endif
          json_object_set_string(pobj, "$ref", ref_str);
        } else {
          write_type_union(pobj, "object", field->type_union,
                           field->n_type_union);
        }
      } else {
        write_type_union(pobj, typ, field->type_union, field->n_type_union);
      }
    }
    write_numeric_constraints(pobj, field);
    write_string_constraints(pobj, field);
    write_default_value(pobj, field);
    if (field->description[0])
      json_object_set_string(pobj, "description", field->description);
    if (field->format[0])
      json_object_set_string(pobj, "format", field->format);
    if (field->deprecated_set)
      json_object_set_boolean(pobj, "deprecated", field->deprecated ? 1 : 0);
    if (field->read_only_set)
      json_object_set_boolean(pobj, "readOnly", field->read_only ? 1 : 0);
    if (field->write_only_set)
      json_object_set_boolean(pobj, "writeOnly", field->write_only ? 1 : 0);
    if (merge_schema_extras_object(pobj, field->schema_extra_json) != 0) {
      json_value_free(pval);
      json_value_free(val);
      return ENOMEM;
    }
    json_object_set_value(props_obj, sf->fields[i].name, pval);
    if (field->required) {
      if (!req_val) {
        req_val = json_value_init_array();
        req_arr = json_value_get_array(req_val);
      }
      if (req_arr)
        json_array_append_string(req_arr, field->name);
    }
  }

  if (req_val)
    json_object_set_value(obj, "required", req_val);

  if (merge_schema_extras_object(obj, sf->schema_extra_json) != 0) {
    json_value_free(val);
    return ENOMEM;
  }

  json_object_set_value(schemas_obj, struct_name, val);
  return 0;
}

static int parse_union_and_write(FILE *fp, JSON_Object *schemas_obj,
                                 const char *union_name) {
  /* (Implementation preserved from previous code2schema.c) */
  char line[512];
  JSON_Value *union_val = json_value_init_object();
  JSON_Object *union_obj = json_value_get_object(union_val);
  JSON_Value *oneof_val = json_value_init_array();
  JSON_Array *oneof_arr = json_value_get_array(oneof_val);
  char *p;

  while (read_line(fp, line, sizeof(line))) {
    p = line;
    while (isspace((unsigned char)*p))
      p++;
    if (*p == '}')
      break;
    if (!*p)
      continue;
    {
      char typebuf[64] = {0};
      char namebuf[64] = {0};
      if (sscanf(p, "%63s %63[^;]", typebuf, namebuf) == 2) {
        char *n = namebuf;
        char *t = typebuf;
        if (n[0] == '*')
          n++;
        {
          JSON_Value *option_val = json_value_init_object();
          JSON_Object *option_obj = json_value_get_object(option_val);
          JSON_Value *props_val = json_value_init_object();
          JSON_Object *props_obj = json_value_get_object(props_val);
          JSON_Value *field_val = json_value_init_object();
          JSON_Object *field_obj = json_value_get_object(field_val);

          if (strcmp(t, "int") == 0)
            json_object_set_string(field_obj, "type", "integer");
          else if (strcmp(t, "char") == 0)
            json_object_set_string(field_obj, "type", "string");
          else if (strcmp(t, "float") == 0)
            json_object_set_string(field_obj, "type", "number");
          else
            json_object_set_string(field_obj, "type", "object");

          json_object_set_value(props_obj, n, field_val);
          json_object_set_string(option_obj, "type", "object");
          json_object_set_value(option_obj, "properties", props_val);
          json_object_set_string(option_obj, "title", n);
          json_array_append_value(oneof_arr, option_val);
        }
      }
    }
  }

  json_object_set_value(union_obj, "oneOf", oneof_val);
  json_object_set_string(union_obj, "type", "object");
  json_object_set_value(schemas_obj, union_name, union_val);
  return 0;
}

int code2schema_main(int argc, char **argv) {
  FILE *fp;
  char line[MAX_LINE_LENGTH];
  JSON_Value *root = json_value_init_object();
  JSON_Object *root_obj = json_value_get_object(root);
  JSON_Value *schemas_val = json_value_init_object();
  JSON_Object *schemas_obj = json_value_get_object(schemas_val);
  JSON_Value *comp_val = json_value_init_object();
  JSON_Object *comp_obj = json_value_get_object(comp_val);

  if (argc != 2) {
    json_value_free(root);
    return EXIT_FAILURE;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (fopen_s(&fp, argv[0], "r") != 0)
    fp = NULL;
#else
  fp = fopen(argv[0], "r");
#endif

  if (!fp) {
    json_value_free(root);
    return EXIT_FAILURE;
  }

  json_object_set_value(comp_obj, "schemas", schemas_val);
  json_object_set_value(root_obj, "components", comp_val);

  while (read_line(fp, line, sizeof(line))) {
    char *p = line;
    while (isspace((unsigned char)*p))
      p++;

    if (strncmp(p, "union ", 6) == 0) {
      char union_name[64] = {0};
      char *brace = strchr(p, '{');
      if (brace) {
        *brace = 0;
        if (sscanf(p + 6, "%63s", union_name) == 1) {
          parse_union_and_write(fp, schemas_obj, union_name);
        }
      }
    } else if (strncmp(p, "struct ", 7) == 0) {
      char struct_name[64] = {0};
      char *brace = strchr(p, '{');
      if (brace) {
        struct StructFields sf;
        *brace = 0;
        if (sscanf(p + 7, "%63s", struct_name) == 1) {
          if (struct_fields_init(&sf) == 0) {
            char subline[MAX_LINE_LENGTH];
            while (read_line(fp, subline, sizeof(subline))) {
              char *sp = subline;
              while (isspace((unsigned char)*sp))
                sp++;
              if (*sp == '}')
                break;
              if (*sp)
                parse_struct_member_line(sp, &sf);
            }
            write_struct_to_json_schema(schemas_obj, struct_name, &sf);
            struct_fields_free(&sf);
          }
        }
      }
    } else if (strncmp(p, "enum ", 5) == 0) {
      char enum_name[64] = {0};
      char *brace = strchr(p, '{');
      if (brace) {
        JSON_Value *eval = json_value_init_object();
        JSON_Object *eobj = json_value_get_object(eval);
        JSON_Value *arrval = json_value_init_array();
        JSON_Array *earr = json_value_get_array(arrval);

        *brace = 0;
        if (sscanf(p + 5, "%63s", enum_name) == 1) {
          const char *delim = ",}";
          char *token;
          char *ctx = NULL;
          char *rest = brace + 1;
          char full_body[4096] = {0};

          /* Accumulate body */
          strcat(full_body, rest);
          while (!strchr(full_body, '}')) {
            if (!read_line(fp, line, sizeof(line)))
              break;
            strcat(full_body, line);
          }

          token = strtok_r(full_body, delim, &ctx);
          while (token) {
            char *tm = token;
            while (isspace((unsigned char)*tm))
              tm++;
            c_cdd_str_trim_trailing_whitespace(tm);
            {
              char *eq = strchr(tm, '=');
              if (eq)
                *eq = 0;
              c_cdd_str_trim_trailing_whitespace(tm);
            }
            if (*tm)
              json_array_append_string(earr, tm);
            token = strtok_r(NULL, delim, &ctx);
          }
          json_object_set_string(eobj, "type", "string");
          json_object_set_value(eobj, "enum", arrval);
          json_object_set_value(schemas_obj, enum_name, eval);
        } else {
          json_value_free(eval);
          json_value_free(arrval);
        }
      }
    }
  }

  json_serialize_to_file_pretty(root, argv[1]);
  fclose(fp);
  json_value_free(root);
  return EXIT_SUCCESS;
}
