/**
 * @file c2openapi_schema.c
 * @brief Implementation of Schema Registry Integration.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit_schema.h"
#include "classes/emit_struct.h"
#include "functions/parse_str.h"

/* --- Helpers --- */

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

static int schema_exists(const struct OpenAPI_Spec *spec, const char *name) {
  size_t i;
  for (i = 0; i < spec->n_defined_schemas; ++i) {
    if (spec->defined_schema_names[i] &&
        strcmp(spec->defined_schema_names[i], name) == 0) {
      return 1;
    }
  }
  return 0;
}

static int enum_exists(const struct OpenAPI_Spec *spec, const char *name) {
  size_t i;
  for (i = 0; i < spec->n_defined_schemas; ++i) {
    if (spec->defined_schema_names[i] &&
        strcmp(spec->defined_schema_names[i], name) == 0 &&
        spec->defined_schemas[i].is_enum) {
      return 1;
    }
  }
  return 0;
}

static int copy_enum_members(const struct EnumMembers *src,
                             struct EnumMembers *dst) {
  size_t i;
  if (!src || !dst)
    return EINVAL;
  if (enum_members_init(dst) != 0)
    return ENOMEM;
  for (i = 0; i < src->size; ++i) {
    if (src->members[i]) {
      if (enum_members_add(dst, src->members[i]) != 0)
        return ENOMEM;
    }
  }
  return 0;
}

/**
 * @brief Deep copy a StructFields object.
 *
 * Since `c_inspector` owns its memory and we need to transfer/copy into
 * `OpenAPI_Spec` (which manages its own lifecycle), we must copy.
 */
static int copy_struct_fields(const struct StructFields *src,
                              struct StructFields *dst) {
  size_t i;
  if (struct_fields_init(dst) != 0)
    return ENOMEM;
  if (src->schema_extra_json) {
    dst->schema_extra_json = c_cdd_strdup(src->schema_extra_json);
    if (!dst->schema_extra_json)
      return ENOMEM;
  }

  for (i = 0; i < src->size; ++i) {
    const struct StructField *f = &src->fields[i];
    /* struct_fields_add copies strings internally */
    if (struct_fields_add(dst, f->name, f->type,
                          f->ref[0] ? f->ref : NULL, /* handle empty */
                          f->default_val[0] ? f->default_val : NULL,
                          f->bit_width[0] ? f->bit_width : NULL) != 0) {
      return ENOMEM;
    }
    /* Copy constraints manually if add doesn't cover them (add only covers
     * basics) */
    /* StructFields_add zeros the struct then sets basics. We copy extra props.
     */
    {
      struct StructField *dst_field = &dst->fields[dst->size - 1];
      *dst_field = *f; /* Copy fixed-size fields/constraints */
      dst_field->schema_extra_json = NULL;
      dst_field->items_extra_json = NULL;
      dst_field->type_union = NULL;
      dst_field->n_type_union = 0;
      dst_field->items_type_union = NULL;
      dst_field->n_items_type_union = 0;
      if (f->schema_extra_json) {
        dst_field->schema_extra_json = c_cdd_strdup(f->schema_extra_json);
        if (!dst_field->schema_extra_json)
          return ENOMEM;
      }
      if (f->items_extra_json) {
        dst_field->items_extra_json = c_cdd_strdup(f->items_extra_json);
        if (!dst_field->items_extra_json)
          return ENOMEM;
      }
      if (f->type_union && f->n_type_union > 0) {
        if (copy_string_array(&dst_field->type_union, &dst_field->n_type_union,
                              f->type_union, f->n_type_union) != 0)
          return ENOMEM;
      }
      if (f->items_type_union && f->n_items_type_union > 0) {
        if (copy_string_array(&dst_field->items_type_union,
                              &dst_field->n_items_type_union,
                              f->items_type_union, f->n_items_type_union) != 0)
          return ENOMEM;
      }
    }
  }
  return 0;
}

/* --- Core Logic --- */

int c2openapi_register_types(struct OpenAPI_Spec *const spec,
                             const struct TypeDefList *const types) {
  size_t i;
  int rc = 0;

  if (!spec || !types)
    return EINVAL;

  for (i = 0; i < types->size; ++i) {
    const struct TypeDefinition *def = &types->items[i];

    if (def->kind == KIND_STRUCT) {
      if (!schema_exists(spec, def->name)) {
        /* Add new schema */
        size_t new_idx = spec->n_defined_schemas;
        size_t new_cap = new_idx + 1;
        char **new_names;
        struct StructFields *new_schemas;

        /* Realloc names array */
        new_names = (char **)realloc(spec->defined_schema_names,
                                     new_cap * sizeof(char *));
        if (!new_names)
          return ENOMEM;
        spec->defined_schema_names = new_names;

        /* Realloc schemas array */
        new_schemas = (struct StructFields *)realloc(
            spec->defined_schemas, new_cap * sizeof(struct StructFields));
        if (!new_schemas)
          return ENOMEM;
        spec->defined_schemas = new_schemas;

        /* Store */
        spec->defined_schema_names[new_idx] = c_cdd_strdup(def->name);
        if (!spec->defined_schema_names[new_idx])
          return ENOMEM;

        rc = copy_struct_fields(def->details.struct_fields,
                                &spec->defined_schemas[new_idx]);
        if (rc != 0)
          return rc;

        spec->n_defined_schemas++;
      }
    } else if (def->kind == KIND_ENUM) {
      if (!enum_exists(spec, def->name)) {
        size_t new_idx = spec->n_defined_schemas;
        size_t new_cap = new_idx + 1;
        char **new_names;
        struct StructFields *new_schemas;

        new_names = (char **)realloc(spec->defined_schema_names,
                                     new_cap * sizeof(char *));
        if (!new_names)
          return ENOMEM;
        spec->defined_schema_names = new_names;

        new_schemas = (struct StructFields *)realloc(
            spec->defined_schemas, new_cap * sizeof(struct StructFields));
        if (!new_schemas)
          return ENOMEM;
        spec->defined_schemas = new_schemas;

        spec->defined_schema_names[new_idx] = c_cdd_strdup(def->name);
        if (!spec->defined_schema_names[new_idx])
          return ENOMEM;

        if (struct_fields_init(&spec->defined_schemas[new_idx]) != 0)
          return ENOMEM;
        spec->defined_schemas[new_idx].is_enum = 1;
        if (def->details.enum_members) {
          if (copy_enum_members(def->details.enum_members,
                                &spec->defined_schemas[new_idx].enum_members) !=
              0)
            return ENOMEM;
        }
        spec->n_defined_schemas++;
      }
    }
  }

  return 0;
}
