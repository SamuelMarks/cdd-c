/**
 * @file struct.c
 * @brief Implementation of struct lifecycle generation.
 *
 * Updates include:
 * - Integration of `numeric_parser` to handle binary literals.
 * - String comparison to handle `nullptr`.
 * - Bit-field width persistence in StructField.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/emit/struct.h"
#include "c_cdd/code_writer.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "classes/parse/code2schema.h"
#include "classes/parse/numeric.h"
#include "functions/parse/str.h"
#include "functions/str_includes.h" /* For NUM_LONG_FMT macros if needed, here mostly standard */
#include "win_compat_sym.h"
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/* Select correct strdup function name for generated code */
#if defined(_MSC_VER)
/** @brief Platform-specific strdup function name for emitted code */
static const char *kStrDupFunc = "_strdup";
#else
/** @brief Platform-specific strdup function name for emitted code */
static const char *kStrDupFunc = "strdup";
#endif

/* Terser error checking */
/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

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
  static char s_ref_buf[4][64];
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
          size_t slot = s_ref_idx % 4;
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

/* --- Generation Implementing --- */

/*
 * @brief Emits C source code for struct cleanup func.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields descriptor container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_struct_cleanup_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config) {
  size_t i;
  int iter_needed = 0;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = 1;
  }

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(fp,
                        "cdd_c_error_t %s_cleanup(struct %s *obj) {\n"
                        "  if (!obj) return CDD_C_SUCCESS;\n",
                        struct_name, struct_name));

  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  { size_t i;\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    char *r = sf->fields[i].ref;

    if (strcmp(t, "string") == 0) {
      CHECK_IO(
          FPRINTF_HOOK(fp, "  if (obj->%s) free((void*)obj->%s);\n", n, n));
    } else if (strcmp(t, "object") == 0) {
      {
        const char *tn = NULL;
        get_type_from_ref(r, &tn);
        CHECK_IO(FPRINTF_HOOK(
            fp, "  if (obj->%s) {%s_cleanup(obj->%s); free(obj->%s); }\n", n,
            tn, n, n));
      }
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp, "  for (i = 0; i < obj->n_%s; ++i) {\n", n));
      if (strcmp(r, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp, "    free(obj->%s[i]);\n", n));
      } else if (strcmp(r, "object") == 0 ||
                 (strcmp(r, "integer") != 0 && strcmp(r, "boolean") != 0 &&
                  strcmp(r, "number") != 0)) {
        {
          const char *tn = NULL;
          cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
          int io_rc;
          if (rc_ref != CDD_C_SUCCESS) {
            return rc_ref;
          }
          io_rc = FPRINTF_HOOK(
              fp, "    %s_cleanup(obj->%s[i]); free(obj->%s[i]);\n", tn, n, n);
          CHECK_IO(io_rc);
        }
      }
      CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "  free(obj->%s);\n", n));
    }
  }

  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));

  CHECK_IO(FPRINTF_HOOK(fp, "  free(obj);\n  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits C source code for struct deepcopy func.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields descriptor container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_struct_deepcopy_func(FILE *fp, const char *struct_name,
                           const struct StructFields *sf,
                           const struct CodegenStructConfig *config) {
  size_t i;
  int has_ptr_fields = 0;
  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < sf->size; ++i) {
    const char *t = sf->fields[i].type;
    if (strcmp(t, "string") == 0 || strcmp(t, "object") == 0 ||
        strcmp(t, "array") == 0) {
      has_ptr_fields = 1;
      break;
    }
  }

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(fp,
                        "cdd_c_error_t %s_deepcopy(const struct %s *src, "
                        "struct %s **dest) {\n",
                        struct_name, struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(fp,
                        "  if (!dest) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                        "  if (!src) { *dest = NULL; return CDD_C_SUCCESS; }\n"
                        "  *dest = malloc(sizeof(struct %s));\n"
                        "  if (!*dest) return CDD_C_ERROR_MEMORY;\n"
                        "  memcpy(*dest, src, sizeof(struct %s));\n\n",
                        struct_name, struct_name));

  if (has_ptr_fields) {
    /* Zero pointer fields first so early cleanup on OOM never frees src's
     * pointers */
    for (i = 0; i < sf->size; ++i) {
      const char *n = sf->fields[i].name;
      const char *t = sf->fields[i].type;
      if (strcmp(t, "string") == 0 || strcmp(t, "object") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp, "  (*dest)->%s = NULL;\n", n));
      } else if (strcmp(t, "array") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp, "  (*dest)->%s = NULL; (*dest)->n_%s = 0;\n",
                              n, n));
      }
    }

    for (i = 0; i < sf->size; ++i) {
      const char *n = sf->fields[i].name;
      const char *t = sf->fields[i].type;
      const char *r = sf->fields[i].ref;

      if (strcmp(t, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  if (src->%s) {\n"
                              "    (*dest)->%s = %s(src->%s);\n"
                              "    if (!(*dest)->%s) { %s_cleanup(*dest); "
                              "*dest=NULL; return CDD_C_ERROR_MEMORY; }\n"
                              "  }\n",
                              n, n, kStrDupFunc, n, n, struct_name));
      } else if (strcmp(t, "object") == 0) {
        const char *tn = NULL;
        get_type_from_ref(r, &tn);
        if (tn[0] != '\0') {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "  if (src->%s) {\n"
              "    if (%s_deepcopy(src->%s, &(*dest)->%s) != 0) { "
              "%s_cleanup(*dest); *dest=NULL; return CDD_C_ERROR_MEMORY; }\n"
              "  }\n",
              n, tn, n, n, struct_name));
        }
      } else if (strcmp(t, "array") == 0) {
        if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "  if (src->%s && src->n_%s > 0) {\n"
              "    (*dest)->%s = malloc(src->n_%s * sizeof(int));\n"
              "    if (!(*dest)->%s) { %s_cleanup(*dest); *dest=NULL; return "
              "CDD_C_ERROR_MEMORY; }\n"
              "    memcpy((*dest)->%s, src->%s, src->n_%s * sizeof(int));\n"
              "    (*dest)->n_%s = src->n_%s;\n"
              "  }\n",
              n, n, n, n, n, struct_name, n, n, n, n, n));
        } else if (strcmp(r, "number") == 0) {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "  if (src->%s && src->n_%s > 0) {\n"
              "    (*dest)->%s = malloc(src->n_%s * sizeof(double));\n"
              "    if (!(*dest)->%s) { %s_cleanup(*dest); *dest=NULL; return "
              "CDD_C_ERROR_MEMORY; }\n"
              "    memcpy((*dest)->%s, src->%s, src->n_%s * sizeof(double));\n"
              "    (*dest)->n_%s = src->n_%s;\n"
              "  }\n",
              n, n, n, n, n, struct_name, n, n, n, n, n));
        } else if (strcmp(r, "string") == 0) {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "  if (src->%s && src->n_%s > 0) {\n"
              "    size_t _i;\n"
              "    (*dest)->%s = calloc(src->n_%s, sizeof(char *));\n"
              "    if (!(*dest)->%s) { %s_cleanup(*dest); *dest=NULL; return "
              "CDD_C_ERROR_MEMORY; }\n"
              "    (*dest)->n_%s = src->n_%s;\n"
              "    for (_i = 0; _i < src->n_%s; ++_i) {\n"
              "      if (src->%s[_i]) {\n"
              "        (*dest)->%s[_i] = %s(src->%s[_i]);\n"
              "        if (!(*dest)->%s[_i]) { %s_cleanup(*dest); *dest=NULL; "
              "return CDD_C_ERROR_MEMORY; }\n"
              "      }\n"
              "    }\n"
              "  }\n",
              n, n, n, n, n, struct_name, n, n, n, n, n, kStrDupFunc, n, n,
              struct_name));
        } else if (strcmp(r, "object") != 0 && strcmp(r, "unknown") != 0 &&
                   r[0] != '\0') {
          const char *tn = NULL;
          get_type_from_ref(r, &tn);
          if (tn[0] != '\0') {
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "  if (src->%s && src->n_%s > 0) {\n"
                "    size_t _i;\n"
                "    (*dest)->%s = calloc(src->n_%s, sizeof(struct %s *));\n"
                "    if (!(*dest)->%s) { %s_cleanup(*dest); *dest=NULL; return "
                "CDD_C_ERROR_MEMORY; }\n"
                "    (*dest)->n_%s = src->n_%s;\n"
                "    for (_i = 0; _i < src->n_%s; ++_i) {\n"
                "      if (src->%s[_i]) {\n"
                "        if (%s_deepcopy(src->%s[_i], &(*dest)->%s[_i]) != 0) "
                "{ %s_cleanup(*dest); *dest=NULL; return CDD_C_ERROR_MEMORY; "
                "}\n"
                "      }\n"
                "    }\n"
                "  }\n",
                n, n, n, n, tn, n, struct_name, n, n, n, n, tn, n, n,
                struct_name));
          }
        }
      }
    }
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits C source code for struct eq func.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields descriptor container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t write_struct_eq_func(FILE *fp, const char *struct_name,
                                   const struct StructFields *sf,
                                   const struct CodegenStructConfig *config) {
  size_t i;
  int iter_needed = 0;
  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = 1;
  }

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(fp,
                        "cdd_c_error_t %s_eq(const struct %s *a, const "
                        "struct %s *b, int *out_eq) {\n",
                        struct_name, struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      fp, "  if (a == b) { *out_eq = 1; return CDD_C_SUCCESS; }\n"
          "  if (!a || !b) { *out_eq = 0; return CDD_C_SUCCESS; }\n"));

  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  { size_t i;\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = NULL;
    cdd_c_error_t rc_ref = get_type_from_ref(sf->fields[i].ref, &r);
    if (rc_ref != CDD_C_SUCCESS) {
      return rc_ref;
    }

    if (strcmp(t, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  if (a->%s != b->%s && (!a->%s || !b->%s || "
                            "strcmp(a->%s, b->%s) != 0)) { *out_eq = 0; return "
                            "CDD_C_SUCCESS; }\n",
                            n, n, n, n, n, n));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "  { int _t = 0; cdd_c_error_t _rc = %s_eq(a->%s, "
                       "b->%s, &_t); if (_rc != CDD_C_SUCCESS) return _rc; if "
                       "(!_t) { *out_eq = 0; return CDD_C_SUCCESS; } }\n",
                       r, n, n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  if (a->n_%s != b->n_%s) { *out_eq = 0; return CDD_C_SUCCESS; }\n",
          n, n));
      CHECK_IO(FPRINTF_HOOK(fp, "  for (i = 0; i < a->n_%s; ++i) {\n", n));
      if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0 ||
          strcmp(r, "number") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "    if (a->%s[i] != b->%s[i]) { *out_eq = 0; "
                              "return CDD_C_SUCCESS; }\n",
                              n, n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "    if (strcmp(a->%s[i], b->%s[i]) != 0) { "
                              "*out_eq = 0; return CDD_C_SUCCESS; }\n",
                              n, n));
      } else {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "    { int _t = 0; cdd_c_error_t _rc = %s_eq(a->%s[i], "
            "b->%s[i], &_t); if (_rc != CDD_C_SUCCESS) return _rc; if (!_t) { "
            "*out_eq = 0; return CDD_C_SUCCESS; } }\n",
            r, n, n));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));
    } else {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  if (a->%s != b->%s) { *out_eq = 0; return CDD_C_SUCCESS; }\n",
          n, n));
    }
  }

  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));

  CHECK_IO(FPRINTF_HOOK(fp, "  *out_eq = 1; return CDD_C_SUCCESS;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits C source code for struct default func.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields descriptor container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_struct_default_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config) {
  size_t i;
  int rc_needed = 0;
  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Check if we need 'rc' variable */
  for (i = 0; i < sf->size; ++i) {
    if (sf->fields[i].default_val[0] != '\0' &&
        strcmp(sf->fields[i].type, "enum") == 0) {
      rc_needed = 1;
      break;
    }
  }

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(fp, "cdd_c_error_t %s_default(struct %s **out) {\n",
                        struct_name, struct_name));
  if (rc_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  int rc;\n"));

  CHECK_IO(
      FPRINTF_HOOK(fp, "  if (!out) return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  *out = calloc(1, sizeof(**out));\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  if (!*out) return CDD_C_ERROR_MEMORY;\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *def = sf->fields[i].default_val;
    if (def[0] != '\0') {
      const char *n = sf->fields[i].name;
      const char *t = sf->fields[i].type;
      const char *r = NULL;
      cdd_c_error_t rc_ref = get_type_from_ref(sf->fields[i].ref, &r);
      if (rc_ref != CDD_C_SUCCESS) {
        return rc_ref;
      }

      if (strcmp(t, "string") == 0) {
        if (strcmp(def, "nullptr") == 0) {
          CHECK_IO(FPRINTF_HOOK(fp, "  (*out)->%s = NULL;\n", n));
        } else {
          CHECK_IO(FPRINTF_HOOK(fp, "  (*out)->%s = %s(%s);\n", n, kStrDupFunc,
                                def));
          CHECK_IO(
              FPRINTF_HOOK(fp,
                           "  if (!(*out)->%s) { %s_cleanup(*out); *out=NULL; "
                           "return CDD_C_ERROR_MEMORY; }\n",
                           n, struct_name));
        }
      } else if (strcmp(t, "enum") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp, "  rc = %s_from_str(%s, &(*out)->%s);\n", r,
                              def, n));
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  if (rc != CDD_C_SUCCESS) { %s_cleanup(*out); "
                              "*out=NULL; return rc; }\n",
                              struct_name));
      } else {
        /* Primitives (integer/boolean/number) */
        /* Check for C23 nullptr -> NULL pointer mapping */
        if (strcmp(def, "nullptr") == 0) {
          CHECK_IO(FPRINTF_HOOK(fp, "  (*out)->%s = NULL;\n", n));
        }
        /* Check for C23 Binary Literal -> Decimal Conversion */
        else if (strlen(def) > 2 && def[0] == '0' &&
                 (def[1] == 'b' || def[1] == 'B')) {
          struct NumericValue nv;
          /* If using C89 target, we must emit decimal. */
          /* Attempt to parse binary literal */
          if (parse_numeric_literal(def, &nv) == 0) {
            /* Emit as hex to avoid C89 %llu warning on GCC */
            CHECK_IO(FPRINTF_HOOK(
                fp, "  (*out)->%s = 0x%08lx%08lxULL;\n", n,
                (unsigned long)((uint64_t)nv.data.integer.value >> 32),
                (unsigned long)((uint64_t)nv.data.integer.value & 0xFFFFFFFF)));
          } else {
            /* Fallback: print as is (if parse failed or invalid) */
            CHECK_IO(FPRINTF_HOOK(fp, "  (*out)->%s = %s;\n", n, def));
          }
        } else {
          CHECK_IO(FPRINTF_HOOK(fp, "  (*out)->%s = %s;\n", n, def));
        }
      }
    }
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits C source code for struct debug func.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields descriptor container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_struct_debug_func(FILE *fp, const char *struct_name,
                        const struct StructFields *sf,
                        const struct CodegenStructConfig *config) {
  size_t i;
  int iter_needed = 0;
  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = 1;
  }

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(
      fp, "cdd_c_error_t %s_debug(const struct %s *obj, FILE *fp) {\n",
      struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      fp, "  int fprintf_rc = 0;\n  cdd_c_error_t rc = CDD_C_SUCCESS;\n"));
  CHECK_IO(
      FPRINTF_HOOK(fp, "  if (!fp) return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  if (!obj) { return fprintf(fp, "
                            "\"(null)\\n\") < 0 ? -1 : 0; }\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  fprintf_rc = fprintf(fp, \"struct %s {\\n\");\n",
                        struct_name));
  CHECK_IO(FPRINTF_HOOK(fp, "  if (fprintf_rc < 0) return CDD_C_ERROR_IO;\n"));

  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  { size_t i;\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = NULL;
    cdd_c_error_t rc_ref = get_type_from_ref(sf->fields[i].ref, &r);
    if (rc_ref != CDD_C_SUCCESS) {
      return rc_ref;
    }

    if (strcmp(t, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  fprintf_rc = fprintf(fp, \"  %s: \\\"%%s\\\"\\n\", obj->%s ? "
          "obj->%s : \"(null)\");\n",
          n, n, n));
      CHECK_IO(
          FPRINTF_HOOK(fp, "  if (fprintf_rc < 0) return CDD_C_ERROR_IO;\n"));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(
          FPRINTF_HOOK(fp, "  fprintf_rc = fprintf(fp, \"  %s: \");\n", n));
      CHECK_IO(
          FPRINTF_HOOK(fp, "  if (fprintf_rc < 0) return CDD_C_ERROR_IO;\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "  rc = %s_debug(obj->%s, fp);\n", r, n));
      CHECK_IO(FPRINTF_HOOK(fp, "  if (rc != CDD_C_SUCCESS) return rc;\n"));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(
          FPRINTF_HOOK(fp, "  fprintf_rc = fprintf(fp, \"  %s: [\\n\");\n", n));
      CHECK_IO(
          FPRINTF_HOOK(fp, "  if (fprintf_rc < 0) return CDD_C_ERROR_IO;\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "  for (i = 0; i < obj->n_%s; ++i) {\n", n));
      if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0 ||
          strcmp(r, "enum") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "    fprintf_rc = fprintf(fp, \"    %%d\\n\", (int)obj->%s[i]);\n",
            n));
      } else if (strcmp(r, "number") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "    fprintf_rc = fprintf(fp, \"    %%f\\n\", "
                              "(double)obj->%s[i]);\n",
                              n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_IO(
            FPRINTF_HOOK(fp,
                         "    fprintf_rc = fprintf(fp, \"    \\\"%%s\\\"\\n\", "
                         "obj->%s[i] ? obj->%s[i] : \"(null)\");\n",
                         n, n));
      } else {
        CHECK_IO(
            FPRINTF_HOOK(fp, "    rc = %s_debug(obj->%s[i], fp);\n", r, n));
        CHECK_IO(FPRINTF_HOOK(fp, "    if (rc != CDD_C_SUCCESS) return rc;\n"));
      }
      CHECK_IO(
          FPRINTF_HOOK(fp, "    if (fprintf_rc < 0) return CDD_C_ERROR_IO;\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "  fprintf_rc = fprintf(fp, \"  ]\\n\");\n"));
      CHECK_IO(
          FPRINTF_HOOK(fp, "  if (fprintf_rc < 0) return CDD_C_ERROR_IO;\n"));
    } else if (strcmp(t, "integer") == 0 || strcmp(t, "boolean") == 0 ||
               strcmp(t, "enum") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  fprintf_rc = fprintf(fp, \"  %s: %%d\\n\", (int)obj->%s);\n",
          n, n));
      CHECK_IO(
          FPRINTF_HOOK(fp, "  if (fprintf_rc < 0) return CDD_C_ERROR_IO;\n"));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  fprintf_rc = fprintf(fp, \"  %s: %%f\\n\", (double)obj->%s);\n", n,
          n));
      CHECK_IO(
          FPRINTF_HOOK(fp, "  if (fprintf_rc < 0) return CDD_C_ERROR_IO;\n"));
    } else {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  fprintf_rc = fprintf(fp, \"  %s: (unknown)\\n\");\n", n));
      CHECK_IO(
          FPRINTF_HOOK(fp, "  if (fprintf_rc < 0) return CDD_C_ERROR_IO;\n"));
    }
  }

  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));

  CHECK_IO(FPRINTF_HOOK(fp, "  fprintf_rc = fprintf(fp, \"}\\n\");\n"));
  CHECK_IO(FPRINTF_HOOK(
      fp, "  return fprintf_rc < 0 ? CDD_C_ERROR_IO : CDD_C_SUCCESS;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits C source code for struct display func.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields descriptor container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_struct_display_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config) {
  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(
      fp, "cdd_c_error_t %s_display(const struct %s *obj, FILE *fp) {\n",
      struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(fp, "  return %s_debug(obj, fp);\n}\n", struct_name));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return CDD_C_SUCCESS;
}
