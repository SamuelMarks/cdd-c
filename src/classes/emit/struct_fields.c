/**
 * @file struct_fields.c
 * @brief StructFields container lifecycle and field manipulation functions.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/emit/struct.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "classes/parse/code2schema.h"
#include "classes/parse/inspector.h"
#include "functions/parse/str.h"
#include "win_compat_sym.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Frees the memory associated with a dynamically allocated string array.
 *
 * @param[in] arr Array of heap-allocated strings.
 * @param[in] n Number of elements in the array.
 */
static void free_string_array(char **arr, size_t n) {
  size_t i;
  for (i = 0; i < n; ++i) {
    if (arr[i]) {
      C_CDD_FREE(arr[i]);
      arr[i] = NULL;
    }
  }
  C_CDD_FREE(arr);
}

/*
 * @brief Extracts the type name from a JSON Schema `$ref` path.
 *
 * @param[in] ref JSON Schema `$ref` string (e.g.,
 * `"#/components/schemas/Foo"`).
 * @param[out] _out_val Output pointer receiving the extracted type name suffix.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t get_type_from_ref(const char *ref, const char **_out_val) {
  static char s_ref_buf[16][256];
  static size_t s_ref_idx = 0;
  if (ref == NULL) {
    *_out_val = "";
    return CDD_C_SUCCESS;
  }
  {
    const char *after = NULL;
    int needs_san = 0;
    cdd_c_error_t rc_after = c_cdd_str_after_last(ref, '/', &after);
    if (rc_after != CDD_C_SUCCESS) {
      return rc_after;
    }
    if (strchr(ref, '/') != NULL && after[0] != '\0') {
      c2s_needs_identifier_sanitization(after, &needs_san);
      if (needs_san) {
        char *san = NULL;
        if (sanitize_identifier(after, &san) == CDD_C_SUCCESS) {
          size_t slot = s_ref_idx % 16;
          s_ref_idx++;
          CDD_STRCPY(s_ref_buf[slot], sizeof(s_ref_buf[slot]), san);
          C_CDD_FREE(san);
          *_out_val = s_ref_buf[slot];
          return CDD_C_SUCCESS;
        }
      }
    }
    *_out_val = after;
    return CDD_C_SUCCESS;
  }
}

/*
 * @brief Initializes a dynamic array of struct field descriptors.
 *
 * @param[out] sf Pointer to the StructFields structure to initialize.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t struct_fields_init(struct StructFields *sf) {
  if (!sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(sf, 0, sizeof(*sf));
  sf->capacity = 8;
  sf->fields = (struct StructField *)C_CDD_CALLOC(sf->capacity,
                                                  sizeof(struct StructField));
  if (!sf->fields) {
    sf->capacity = 0;
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Releases all memory owned by a StructFields container.
 *
 * @param[in,out] sf Pointer to the StructFields structure to free.
 */
void struct_fields_free(struct StructFields *sf) {
  if (sf && sf->fields) {
    size_t i;
    for (i = 0; i < sf->size; ++i) {
      if (sf->fields[i].schema_extra_json) {
        C_CDD_FREE(sf->fields[i].schema_extra_json);
        sf->fields[i].schema_extra_json = NULL;
      }
      if (sf->fields[i].items_extra_json) {
        C_CDD_FREE(sf->fields[i].items_extra_json);
        sf->fields[i].items_extra_json = NULL;
      }
      if (sf->fields[i].type_union) {
        free_string_array(sf->fields[i].type_union, sf->fields[i].n_type_union);
        sf->fields[i].type_union = NULL;
        sf->fields[i].n_type_union = 0;
      }
      if (sf->fields[i].items_type_union) {
        free_string_array(sf->fields[i].items_type_union,
                          sf->fields[i].n_items_type_union);
        sf->fields[i].items_type_union = NULL;
        sf->fields[i].n_items_type_union = 0;
      }
      if (sf->fields[i].serde_aliases) {
        free_string_array(sf->fields[i].serde_aliases,
                          sf->fields[i].n_serde_aliases);
        sf->fields[i].serde_aliases = NULL;
        sf->fields[i].n_serde_aliases = 0;
      }
      if (sf->fields[i].cli_long_help) {
        C_CDD_FREE(sf->fields[i].cli_long_help);
        sf->fields[i].cli_long_help = NULL;
      }
      if (sf->fields[i].cli_conflicts_with) {
        free_string_array(sf->fields[i].cli_conflicts_with,
                          sf->fields[i].n_cli_conflicts_with);
        sf->fields[i].cli_conflicts_with = NULL;
        sf->fields[i].n_cli_conflicts_with = 0;
      }
      if (sf->fields[i].cli_requires) {
        free_string_array(sf->fields[i].cli_requires,
                          sf->fields[i].n_cli_requires);
        sf->fields[i].cli_requires = NULL;
        sf->fields[i].n_cli_requires = 0;
      }
      if (sf->fields[i].cli_required_unless) {
        free_string_array(sf->fields[i].cli_required_unless,
                          sf->fields[i].n_cli_required_unless);
        sf->fields[i].cli_required_unless = NULL;
        sf->fields[i].n_cli_required_unless = 0;
      }
    }
    C_CDD_FREE(sf->fields);
    sf->fields = NULL;
  }
  if (sf) {
    if (sf->schema_extra_json) {
      C_CDD_FREE(sf->schema_extra_json);
      sf->schema_extra_json = NULL;
    }
    if (sf->cli_long_about) {
      C_CDD_FREE(sf->cli_long_about);
      sf->cli_long_about = NULL;
    }
    if (sf->cli_before_help) {
      C_CDD_FREE(sf->cli_before_help);
      sf->cli_before_help = NULL;
    }
    if (sf->cli_after_help) {
      C_CDD_FREE(sf->cli_after_help);
      sf->cli_after_help = NULL;
    }
    if (sf->cli_aliases) {
      free_string_array(sf->cli_aliases, sf->n_cli_aliases);
      sf->cli_aliases = NULL;
      sf->n_cli_aliases = 0;
    }
    if (sf->union_discriminator) {
      C_CDD_FREE(sf->union_discriminator);
      sf->union_discriminator = NULL;
    }
    if (sf->union_variants) {
      size_t i;
      for (i = 0; i < sf->n_union_variants; ++i) {
        struct UnionVariantMeta *meta = &sf->union_variants[i];
        if (meta->required_props) {
          free_string_array(meta->required_props, meta->n_required_props);
          meta->required_props = NULL;
          meta->n_required_props = 0;
        }
        if (meta->property_names) {
          free_string_array(meta->property_names, meta->n_property_names);
          meta->property_names = NULL;
          meta->n_property_names = 0;
        }
        if (meta->disc_value) {
          C_CDD_FREE(meta->disc_value);
          meta->disc_value = NULL;
        }
      }
      C_CDD_FREE(sf->union_variants);
      sf->union_variants = NULL;
      sf->n_union_variants = 0;
    }
    enum_members_free(&sf->enum_members);
    sf->is_enum = 0;
    sf->is_union = 0;
    sf->union_is_anyof = 0;
  }
}

/*
 * @brief Appends a new field descriptor to a StructFields container.
 *
 * @param[in,out] sf Target StructFields container.
 * @param[in] name Field identifier name.
 * @param[in] type Field JSON Schema type string.
 * @param[in] ref Optional `$ref` target or array item type.
 * @param[in] default_val Optional default value literal string.
 * @param[in] bit_width Optional C bit-field width string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t struct_fields_add(struct StructFields *sf, const char *name,
                                const char *type, const char *ref,
                                const char *default_val,
                                const char *bit_width) {
  struct StructField *f;
  if (!sf || !name || !type)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (sf->size >= sf->capacity) {
    const size_t new_cap = sf->capacity == 0 ? 8 : sf->capacity * 2;
    struct StructField *new_arr;
    new_arr = (struct StructField *)C_CDD_REALLOC(
        sf->fields, new_cap * sizeof(struct StructField));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    sf->fields = new_arr;
    sf->capacity = new_cap;
  }

  f = &sf->fields[sf->size];
  /* Zero out new slot to clear constraints/flags */
  memset(f, 0, sizeof(struct StructField));
  f->cli_index = -1;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(f->name, sizeof(f->name), name, _TRUNCATE);
  strncpy_s(f->type, sizeof(f->type), type, _TRUNCATE);
  if (ref) {
    strncpy_s(f->ref, sizeof(f->ref), ref, _TRUNCATE);
  }
  if (default_val) {
    strncpy_s(f->default_val, sizeof(f->default_val), default_val, _TRUNCATE);
  }
  if (bit_width) {
    strncpy_s(f->bit_width, sizeof(f->bit_width), bit_width, _TRUNCATE);
  }
#else
  strncpy(f->name, name, sizeof(f->name) - 1);
  f->name[sizeof(f->name) - 1] = '\0';
  strncpy(f->type, type, sizeof(f->type) - 1);
  f->type[sizeof(f->type) - 1] = '\0';
  if (ref) {
    strncpy(f->ref, ref, sizeof(f->ref) - 1);
    f->ref[sizeof(f->ref) - 1] = '\0';
  }
  if (default_val) {
    strncpy(f->default_val, default_val, sizeof(f->default_val) - 1);
    f->default_val[sizeof(f->default_val) - 1] = '\0';
  }
  if (bit_width) {
    strncpy(f->bit_width, bit_width, sizeof(f->bit_width) - 1);
    f->bit_width[sizeof(f->bit_width) - 1] = '\0';
  }
#endif
  sf->size++;
  return CDD_C_SUCCESS;
}

/*
 * @brief Finds a struct field descriptor by name.
 *
 * @param[in] sf StructFields container to search.
 * @param[in] name Field identifier name to look up.
 * @param[out] _out_val Output pointer receiving the matching StructField or
 * NULL.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t struct_fields_get(const struct StructFields *sf, const char *name,
                                struct StructField **_out_val) {
  size_t i;
  if (!sf || !name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].name, name) == 0) {
      *_out_val = &sf->fields[i];
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Appends a unique heap-duplicated string to a dynamic `char **` array.
 *
 * @param[in,out] arr_ptr Pointer to the dynamic string array.
 * @param[in,out] count_ptr Pointer to the element count.
 * @param[in] val Non-empty string to append.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t append_unique_str_to_array(char ***arr_ptr,
                                                size_t *count_ptr,
                                                const char *val) {
  char **new_arr;
  char *dup_str = NULL;
  size_t i;
  cdd_c_error_t rc;
  if (!val || !val[0])
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < *count_ptr; ++i) {
    if ((*arr_ptr)[i] && strcmp((*arr_ptr)[i], val) == 0)
      return CDD_C_SUCCESS;
  }
  rc = c_cdd_strdup(val, &dup_str);
  if (rc != CDD_C_SUCCESS)
    return rc;
  new_arr = (char **)C_CDD_REALLOC(*arr_ptr, (*count_ptr + 1) * sizeof(char *));
  if (!new_arr) {
    C_CDD_FREE(dup_str);
    return CDD_C_ERROR_MEMORY;
  }
  *arr_ptr = new_arr;
  (*arr_ptr)[*count_ptr] = dup_str;
  (*count_ptr)++;
  return CDD_C_SUCCESS;
}

/*
 * @brief Appends a deserialization alias to a StructField.
 *
 * @param[in,out] f Pointer to the target StructField.
 * @param[in] alias Alternative JSON key string to accept during
 * deserialization.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t struct_field_add_alias(struct StructField *f, const char *alias) {
  if (!f)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return append_unique_str_to_array(&f->serde_aliases, &f->n_serde_aliases,
                                    alias);
}

/*
 * @brief Appends a CLI subcommand alias to a StructFields container.
 *
 * @param[in,out] sf Pointer to the target StructFields container.
 * @param[in] alias Subcommand alias string to accept.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t struct_fields_add_cli_alias(struct StructFields *sf,
                                          const char *alias) {
  if (!sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return append_unique_str_to_array(&sf->cli_aliases, &sf->n_cli_aliases,
                                    alias);
}

/*
 * @brief Appends a conflicting CLI field name to a StructField.
 *
 * @param[in,out] f Pointer to the target StructField.
 * @param[in] conflict Name of the conflicting field.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t struct_field_add_cli_conflict(struct StructField *f,
                                            const char *conflict) {
  if (!f)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return append_unique_str_to_array(&f->cli_conflicts_with,
                                    &f->n_cli_conflicts_with, conflict);
}

/*
 * @brief Appends a required companion CLI field name to a StructField.
 *
 * @param[in,out] f Pointer to the target StructField.
 * @param[in] req Name of the required field.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t struct_field_add_cli_requires(struct StructField *f,
                                            const char *req) {
  if (!f)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return append_unique_str_to_array(&f->cli_requires, &f->n_cli_requires, req);
}

/*
 * @brief Appends an alternative satisfying CLI field name (`required_unless`)
 * to a StructField.
 *
 * @param[in,out] f Pointer to the target StructField.
 * @param[in] req Name of the alternative field.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t struct_field_add_cli_required_unless(struct StructField *f,
                                                   const char *req) {
  if (!f)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return append_unique_str_to_array(&f->cli_required_unless,
                                    &f->n_cli_required_unless, req);
}
