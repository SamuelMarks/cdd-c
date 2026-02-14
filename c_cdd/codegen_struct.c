/**
 * @file codegen_struct.c
 * @brief Implementation of struct lifecycle generation.
 *
 * Updates include:
 * - Integration of `numeric_parser` to handle binary literals.
 * - String comparison to handle `nullptr`.
 * - Bit-field width persistence in StructField.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_struct.h"
#include "numeric_parser.h"
#include "str_includes.h" /* For NUM_LONG_FMT macros if needed, here mostly standard */
#include "str_utils.h"

/* Select correct strdup function name for generated code */
#if defined(_MSC_VER)
static const char *kStrDupFunc = "_strdup";
#else
static const char *kStrDupFunc = "strdup";
#endif

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

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

const char *get_type_from_ref(const char *const ref) {
  if (ref == NULL)
    return "";
  return c_cdd_str_after_last(ref, '/');
}

int struct_fields_init(struct StructFields *const sf) {
  if (!sf)
    return EINVAL;
  sf->size = 0;
  sf->capacity = 8;
  sf->fields =
      (struct StructField *)calloc(sf->capacity, sizeof(struct StructField));
  if (!sf->fields)
    return ENOMEM;
  sf->is_enum = 0;
  sf->enum_members.members = NULL;
  sf->enum_members.size = 0;
  sf->enum_members.capacity = 0;
  sf->schema_extra_json = NULL;
  sf->is_union = 0;
  sf->union_is_anyof = 0;
  sf->union_discriminator = NULL;
  sf->union_variants = NULL;
  sf->n_union_variants = 0;
  return 0;
}

void struct_fields_free(struct StructFields *const sf) {
  if (sf && sf->fields) {
    size_t i;
    for (i = 0; i < sf->size; ++i) {
      if (sf->fields[i].schema_extra_json) {
        free(sf->fields[i].schema_extra_json);
        sf->fields[i].schema_extra_json = NULL;
      }
      if (sf->fields[i].items_extra_json) {
        free(sf->fields[i].items_extra_json);
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
    }
    free(sf->fields);
    sf->fields = NULL;
  }
  if (sf) {
    if (sf->schema_extra_json) {
      free(sf->schema_extra_json);
      sf->schema_extra_json = NULL;
    }
    if (sf->union_discriminator) {
      free(sf->union_discriminator);
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
          free(meta->disc_value);
          meta->disc_value = NULL;
        }
      }
      free(sf->union_variants);
      sf->union_variants = NULL;
      sf->n_union_variants = 0;
    }
    enum_members_free(&sf->enum_members);
    sf->is_enum = 0;
    sf->is_union = 0;
    sf->union_is_anyof = 0;
  }
}

int struct_fields_add(struct StructFields *const sf, const char *const name,
                      const char *const type, const char *const ref,
                      const char *const default_val,
                      const char *const bit_width) {
  struct StructField *f;
  if (!sf || !name || !type)
    return EINVAL;

  if (sf->size >= sf->capacity) {
    const size_t new_cap = sf->capacity * 2;
    struct StructField *new_arr = (struct StructField *)realloc(
        sf->fields, new_cap * sizeof(struct StructField));
    if (!new_arr)
      return ENOMEM;
    sf->fields = new_arr;
    sf->capacity = new_cap;
  }

  f = &sf->fields[sf->size];
  /* Zero out new slot to clear constraints/flags */
  memset(f, 0, sizeof(struct StructField));

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

  sf->size++;
  return 0;
}

struct StructField *struct_fields_get(const struct StructFields *const sf,
                                      const char *const name) {
  size_t i;
  if (!sf || !name)
    return NULL;
  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].name, name) == 0)
      return &sf->fields[i];
  }
  return NULL;
}

/* --- Generation Implementing --- */

int write_struct_cleanup_func(FILE *const fp, const char *const struct_name,
                              const struct StructFields *const sf,
                              const struct CodegenStructConfig *const config) {
  size_t i;
  bool iter_needed = false;

  if (!fp || !struct_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = true;
  }

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(fprintf(fp,
                   "void %s_cleanup(struct %s *const obj) {\n"
                   "  if (!obj) return;\n",
                   struct_name, struct_name));

  if (iter_needed)
    CHECK_IO(fprintf(fp, "  { size_t i;\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;

    if (strcmp(t, "string") == 0) {
      CHECK_IO(fprintf(fp, "  if (obj->%s) free((void*)obj->%s);\n", n, n));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(
          fprintf(fp, "  if (obj->%s) {%s_cleanup(obj->%s); free(obj->%s); }\n",
                  n, get_type_from_ref(r), n, n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(fp, "  for (i = 0; i < obj->n_%s; ++i) {\n", n));
      if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(fp, "    free(obj->%s[i]);\n", n));
      } else if (strcmp(r, "object") == 0 ||
                 (strcmp(r, "integer") != 0 && strcmp(r, "boolean") != 0 &&
                  strcmp(r, "number") != 0)) {
        CHECK_IO(fprintf(fp, "    %s_cleanup(obj->%s[i]); free(obj->%s[i]);\n",
                         get_type_from_ref(r), n, n));
      }
      CHECK_IO(fprintf(fp, "  }\n"));
      CHECK_IO(fprintf(fp, "  free(obj->%s);\n", n));
    }
  }

  if (iter_needed)
    CHECK_IO(fprintf(fp, "  }\n"));

  CHECK_IO(fprintf(fp, "  free(obj);\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}

int write_struct_deepcopy_func(FILE *const fp, const char *const struct_name,
                               const struct StructFields *const sf,
                               const struct CodegenStructConfig *const config) {
  if (!fp || !struct_name || !sf)
    return EINVAL;

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(
      fprintf(fp, "int %s_deepcopy(const struct %s *src, struct %s **dest) {\n",
              struct_name, struct_name, struct_name));
  CHECK_IO(fprintf(fp,
                   "  if (!dest) return EINVAL;\n"
                   "  if (!src) { *dest = NULL; return 0; }\n"
                   "  *dest = malloc(sizeof(struct %s));\n"
                   "  if (!*dest) return ENOMEM;\n"
                   "  memcpy(*dest, src, sizeof(struct %s));\n\n",
                   struct_name, struct_name));

  {
    size_t i;
    for (i = 0; i < sf->size; ++i) {
      const char *n = sf->fields[i].name;
      const char *t = sf->fields[i].type;

      if (strcmp(t, "string") == 0) {
        CHECK_IO(fprintf(fp,
                         "  if (src->%s) {\n"
                         "    (*dest)->%s = %s(src->%s);\n"
                         "    if (!(*dest)->%s) { %s_cleanup(*dest); "
                         "*dest=NULL; return ENOMEM; }\n"
                         "  }\n",
                         n, n, kStrDupFunc, n, n, struct_name));
      }
    }
  }

  CHECK_IO(fprintf(fp, "  return 0;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}

int write_struct_eq_func(FILE *const fp, const char *const struct_name,
                         const struct StructFields *const sf,
                         const struct CodegenStructConfig *const config) {
  size_t i;
  int iter_needed = 0;
  if (!fp || !struct_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = 1;
  }

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(fprintf(fp, "int %s_eq(const struct %s *a, const struct %s *b) {\n",
                   struct_name, struct_name, struct_name));
  CHECK_IO(fprintf(fp, "  if (a == b) return 1;\n"
                       "  if (!a || !b) return 0;\n"));

  if (iter_needed)
    CHECK_IO(fprintf(fp, "  { size_t i;\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = get_type_from_ref(sf->fields[i].ref);

    if (strcmp(t, "string") == 0) {
      CHECK_IO(fprintf(fp,
                       "  if (a->%s != b->%s && (!a->%s || !b->%s || "
                       "strcmp(a->%s, b->%s) != 0)) return 0;\n",
                       n, n, n, n, n, n));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(fprintf(fp, "  if (!%s_eq(a->%s, b->%s)) return 0;\n", r, n, n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(fp, "  if (a->n_%s != b->n_%s) return 0;\n", n, n));
      CHECK_IO(fprintf(fp, "  for (i = 0; i < a->n_%s; ++i) {\n", n));
      if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0 ||
          strcmp(r, "number") == 0) {
        CHECK_IO(
            fprintf(fp, "    if (a->%s[i] != b->%s[i]) return 0;\n", n, n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(
            fp, "    if (strcmp(a->%s[i], b->%s[i]) != 0) return 0;\n", n, n));
      } else {
        CHECK_IO(fprintf(fp, "    if (!%s_eq(a->%s[i], b->%s[i])) return 0;\n",
                         r, n, n));
      }
      CHECK_IO(fprintf(fp, "  }\n"));
    } else {
      CHECK_IO(fprintf(fp, "  if (a->%s != b->%s) return 0;\n", n, n));
    }
  }

  if (iter_needed)
    CHECK_IO(fprintf(fp, "  }\n"));

  CHECK_IO(fprintf(fp, "  return 1;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}

int write_struct_default_func(FILE *const fp, const char *const struct_name,
                              const struct StructFields *const sf,
                              const struct CodegenStructConfig *const config) {
  size_t i;
  int rc_needed = 0;
  if (!fp || !struct_name || !sf)
    return EINVAL;

  /* Check if we need 'rc' variable */
  for (i = 0; i < sf->size; ++i) {
    if (sf->fields[i].default_val[0] != '\0' &&
        strcmp(sf->fields[i].type, "enum") == 0) {
      rc_needed = 1;
      break;
    }
  }

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(fprintf(fp, "int %s_default(struct %s **out) {\n", struct_name,
                   struct_name));
  if (rc_needed)
    CHECK_IO(fprintf(fp, "  int rc;\n"));

  CHECK_IO(fprintf(fp, "  if (!out) return EINVAL;\n"));
  CHECK_IO(fprintf(fp, "  *out = calloc(1, sizeof(**out));\n"));
  CHECK_IO(fprintf(fp, "  if (!*out) return ENOMEM;\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *def = sf->fields[i].default_val;
    if (def[0] != '\0') {
      const char *n = sf->fields[i].name;
      const char *t = sf->fields[i].type;
      const char *r = get_type_from_ref(sf->fields[i].ref);

      if (strcmp(t, "string") == 0) {
        if (strcmp(def, "nullptr") == 0) {
          CHECK_IO(fprintf(fp, "  (*out)->%s = NULL;\n", n));
        } else {
          CHECK_IO(
              fprintf(fp, "  (*out)->%s = %s(%s);\n", n, kStrDupFunc, def));
          CHECK_IO(fprintf(fp,
                           "  if (!(*out)->%s) { %s_cleanup(*out); *out=NULL; "
                           "return ENOMEM; }\n",
                           n, struct_name));
        }
      } else if (strcmp(t, "enum") == 0) {
        CHECK_IO(
            fprintf(fp, "  rc = %s_from_str(%s, &(*out)->%s);\n", r, def, n));
        CHECK_IO(fprintf(
            fp, "  if (rc != 0) { %s_cleanup(*out); *out=NULL; return rc; }\n",
            struct_name));
      } else {
        /* Primitives (integer/boolean/number) */
        /* Check for C23 nullptr -> NULL pointer mapping */
        if (strcmp(def, "nullptr") == 0) {
          CHECK_IO(fprintf(fp, "  (*out)->%s = NULL;\n", n));
        }
        /* Check for C23 Binary Literal -> Decimal Conversion */
        else if (strlen(def) > 2 && def[0] == '0' &&
                 (def[1] == 'b' || def[1] == 'B')) {
          struct NumericValue nv;
          /* If using C89 target, we must emit decimal. */
          /* Attempt to parse binary literal */
          if (parse_numeric_literal(def, &nv) == 0 &&
              nv.kind == NUMERIC_INTEGER) {
            /* Emit as largest decimal constant suffix-aware?
               Usually just cast logic is sufficient in C source. */
            /* Using unsigned long long format */
#if defined(_MSC_VER)
            CHECK_IO(fprintf(fp, "  (*out)->%s = %I64u;\n", n,
                             nv.data.integer.value));
#else
            CHECK_IO(fprintf(fp, "  (*out)->%s = %llu;\n", n,
                             (unsigned long long)nv.data.integer.value));
#endif
          } else {
            /* Fallback: print as is (if parse failed or invalid) */
            CHECK_IO(fprintf(fp, "  (*out)->%s = %s;\n", n, def));
          }
        } else {
          CHECK_IO(fprintf(fp, "  (*out)->%s = %s;\n", n, def));
        }
      }
    }
  }

  CHECK_IO(fprintf(fp, "  return 0;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}

int write_struct_debug_func(FILE *const fp, const char *const struct_name,
                            const struct StructFields *const sf,
                            const struct CodegenStructConfig *const config) {
  if (!fp || !struct_name || !sf)
    return EINVAL;

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(fprintf(fp, "int %s_debug(const struct %s *obj, FILE *fp) {\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(fp, "  if (!fp) return EINVAL;\n  if (!obj) return "
                       "fprintf(fp, \"(null)\");\n"));
  CHECK_IO(fprintf(fp, "  return fprintf(fp, \"Struct %s debug\\n\");\n}\n",
                   struct_name));

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}

int write_struct_display_func(FILE *const fp, const char *const struct_name,
                              const struct StructFields *const sf,
                              const struct CodegenStructConfig *const config) {
  if (!fp || !struct_name || !sf)
    return EINVAL;

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(fprintf(fp, "int %s_display(const struct %s *obj, FILE *fp) {\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(fp, "  return %s_debug(obj, fp);\n}\n", struct_name));

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}
