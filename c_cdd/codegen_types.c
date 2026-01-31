/**
 * @file codegen_types.c
 * @brief Implementation of Advanced Types generation.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_struct.h" /* for get_type_from_ref */
#include "codegen_types.h"
#include "str_utils.h"

/* Wrapper for fprintf to check errors tersely */
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

/* --- Union Implementation --- */

int write_union_to_json_func(FILE *const fp, const char *const union_name,
                             const struct StructFields *const sf,
                             const struct CodegenTypesConfig *const config) {
  size_t i;
  int needs_nested_rc = 0;

  if (!fp || !union_name || !sf)
    return EINVAL;

  /* Check if recursive call requires return code var */
  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "object") == 0)
      needs_nested_rc = 1;
  }

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->json_guard));

  CHECK_IO(fprintf(
      fp, "int %s_to_json(const struct %s *const obj, char **const json) {\n",
      union_name, union_name));

  if (needs_nested_rc)
    CHECK_IO(fprintf(fp, "  int rc;\n"));

  CHECK_IO(fprintf(fp, "  if (obj == NULL || json == NULL) return EINVAL;\n"
                       "  switch (obj->tag) {\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;

    CHECK_IO(fprintf(fp, "    case %s_%s:\n", union_name, name));
    CHECK_IO(fprintf(fp, "      jasprintf(json, \"{\");\n"));
    CHECK_IO(fprintf(fp, "      if (*json==NULL) return ENOMEM;\n"));

    /* We wrap the union value in an object { "name": value } to match typical
     * oneOf behavior if simple */
    /* NOTE: Actually simple union assumes discriminator implies key presence.
       Current implementation assumes struct-like container: { "optionName":
       value } */

    if (strcmp(type, "integer") == 0) {
      CHECK_IO(fprintf(
          fp, "      jasprintf(json, \"\\\"%s\\\": %%d}\", obj->data.%s);\n",
          name, name));
    } else if (strcmp(type, "string") == 0) {
      CHECK_IO(fprintf(fp,
                       "      jasprintf(json, \"\\\"%s\\\": "
                       "\\\"%%s\\\"}\", obj->data.%s);\n",
                       name, name));
    } else if (strcmp(type, "object") == 0) {
      CHECK_IO(fprintf(fp,
                       "      {\n"
                       "        char *sub = NULL;\n"
                       "        rc = %s_to_json(obj->data.%s, &sub);\n"
                       "        if (rc != 0) return rc;\n"
                       "        jasprintf(json, \"\\\"%s\\\": %%s}\", sub);\n"
                       "        free(sub);\n"
                       "      }\n",
                       get_type_from_ref(ref), name, name));
    }
    CHECK_IO(fprintf(fp, "      break;\n"));
  }

  CHECK_IO(fprintf(fp, "    default:\n"
                       "      jasprintf(json, \"null\");\n"
                       "      break;\n"
                       "  }\n"
                       "  if (*json == NULL) return ENOMEM;\n"
                       "  return 0;\n"
                       "}\n"));

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->json_guard));

  return 0;
}

int write_union_from_jsonObject_func(
    FILE *const fp, const char *const union_name,
    const struct StructFields *const sf,
    const struct CodegenTypesConfig *const config) {
  size_t i;
  int needs_nested_rc = 0;

  if (!fp || !union_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "object") == 0)
      needs_nested_rc = 1;
  }

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->json_guard));

  CHECK_IO(fprintf(fp,
                   "int %s_from_jsonObject(const JSON_Object *const "
                   "jsonObject, struct %s **const out) {\n",
                   union_name, union_name));

  if (needs_nested_rc)
    CHECK_IO(fprintf(fp, "  int rc;\n"));

  CHECK_IO(fprintf(fp,
                   "  struct %s *ret = malloc(sizeof(struct %s));\n"
                   "  if (!ret) return ENOMEM;\n"
                   "  memset(ret, 0, sizeof(*ret));\n\n",
                   union_name, union_name));

  /* Discriminator logic: check which property exists */
  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;

    CHECK_IO(fprintf(fp, "  if (json_object_has_value(jsonObject, \"%s\")) {\n",
                     name));
    CHECK_IO(fprintf(fp, "    ret->tag = %s_%s;\n", union_name, name));

    if (strcmp(type, "integer") == 0) {
      CHECK_IO(
          fprintf(fp,
                  "    ret->data.%s = (int)json_object_get_number(jsonObject, "
                  "\"%s\");\n",
                  name, name));
    } else if (strcmp(type, "string") == 0) {
      CHECK_IO(
          fprintf(fp,
                  "    ret->data.%s = "
                  "c_cdd_strdup(json_object_get_string(jsonObject, \"%s\"));\n"
                  "    if (!ret->data.%s) { free(ret); return ENOMEM; }\n",
                  name, name, name));
    } else if (strcmp(type, "object") == 0) {
      CHECK_IO(fprintf(
          fp,
          "    rc = %s_from_jsonObject(json_object_get_object(jsonObject, "
          "\"%s\"), &ret->data.%s);\n"
          "    if (rc != 0) { free(ret); return rc; }\n",
          get_type_from_ref(ref), name, name));
    }
    CHECK_IO(fprintf(fp, "    *out = ret;\n    return 0;\n  }\n"));
  }

  CHECK_IO(fprintf(fp, "  free(ret);\n  return EINVAL;\n}\n"));

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->json_guard));

  return 0;
}

int write_union_cleanup_func(FILE *const fp, const char *const union_name,
                             const struct StructFields *const sf,
                             const struct CodegenTypesConfig *const config) {
  size_t i;
  if (!fp || !union_name || !sf)
    return EINVAL;

  if (config && config->utils_guard)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->utils_guard));

  CHECK_IO(fprintf(fp, "void %s_cleanup(struct %s *const obj) {\n", union_name,
                   union_name));
  CHECK_IO(fprintf(fp, "  if (!obj) return;\n  switch (obj->tag) {\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    CHECK_IO(fprintf(fp, "    case %s_%s:\n", union_name, name));
    if (strcmp(sf->fields[i].type, "string") == 0) {
      CHECK_IO(fprintf(fp, "      free((void*)obj->data.%s);\n", name));
    } else if (strcmp(sf->fields[i].type, "object") == 0) {
      CHECK_IO(fprintf(fp, "      %s_cleanup(obj->data.%s);\n",
                       get_type_from_ref(sf->fields[i].ref), name));
    }
    CHECK_IO(fprintf(fp, "      break;\n"));
  }

  CHECK_IO(fprintf(fp, "    default: break;\n  }\n  free(obj);\n}\n"));

  if (config && config->utils_guard)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->utils_guard));

  return 0;
}

/* --- Root Array Implementation --- */

int write_root_array_cleanup_func(
    FILE *const fp, const char *const name, const char *const item_type,
    const char *const item_ref, const struct CodegenTypesConfig *const config) {
  if (!fp || !name || !item_type)
    return EINVAL;

  if (config && config->utils_guard)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->utils_guard));

  if (strcmp(item_type, "integer") == 0) {
    /* Simple flat array */
    CHECK_IO(fprintf(fp,
                     "void %s_cleanup(int *in, size_t len) {\n"
                     "  (void)len; free(in);\n}\n",
                     name));
  } else if (strcmp(item_type, "string") == 0) {
    /* Array of pointers to strings */
    CHECK_IO(fprintf(fp,
                     "void %s_cleanup(char **in, size_t len) {\n"
                     "  size_t i;\n"
                     "  if (!in) return;\n"
                     "  for(i=0; i<len; ++i) free(in[i]);\n"
                     "  free(in);\n"
                     "}\n",
                     name));
  } else if (strcmp(item_type, "object") == 0) {
    /* Array of pointers to structs */
    CHECK_IO(fprintf(fp,
                     "void %s_cleanup(struct %s **in, size_t len) {\n"
                     "  size_t i;\n"
                     "  if (!in) return;\n"
                     "  for(i=0; i<len; ++i) %s_cleanup(in[i]);\n"
                     "  free(in);\n"
                     "}\n",
                     name, get_type_from_ref(item_ref),
                     get_type_from_ref(item_ref)));
  } else {
    /* Fallback generic void* */
    CHECK_IO(fprintf(
        fp, "void %s_cleanup(void *in, size_t len) { (void)len; free(in); }\n",
        name));
  }

  if (config && config->utils_guard)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->utils_guard));

  return 0;
}

int write_root_array_to_json_func(
    FILE *const fp, const char *const name, const char *const item_type,
    const char *const item_ref, const struct CodegenTypesConfig *const config) {
  if (!fp || !name || !item_type)
    return EINVAL;

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->json_guard));

  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(fprintf(
        fp, "int %s_to_json(const int *in, size_t len, char **json_out) {\n",
        name));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_IO(fprintf(
        fp, "int %s_to_json(char **const in, size_t len, char **json_out) {\n",
        name));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_IO(fprintf(fp,
                     "int %s_to_json(struct %s **const in, size_t len, char "
                     "**json_out) {\n",
                     name, get_type_from_ref(item_ref)));
  } else {
    CHECK_IO(fprintf(
        fp, "int %s_to_json(const void *in, size_t len, char **json_out) {\n",
        name));
  }

  CHECK_IO(fprintf(fp, "  size_t i;\n"
                       "  if (!in && len > 0) return EINVAL;\n"
                       "  if (!json_out) return EINVAL;\n"
                       "  jasprintf(json_out, \"[\");\n"
                       "  if (!*json_out) return ENOMEM;\n"
                       "  for (i = 0; i < len; ++i) {\n"
                       "    if (i > 0) { jasprintf(json_out, \",\"); "
                       "if(!*json_out) return ENOMEM; }\n"));

  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(fprintf(fp, "    jasprintf(json_out, \"%%d\", in[i]);\n"));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_IO(fprintf(fp, "    jasprintf(json_out, \"\\\"%%s\\\"\", in[i]);\n"));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_IO(fprintf(fp,
                     "    {\n"
                     "      char *tmp = NULL;\n"
                     "      int rc = %s_to_json(in[i], &tmp);\n"
                     "      if (rc != 0) return rc;\n"
                     "      jasprintf(json_out, \"%%s\", tmp);\n"
                     "      free(tmp);\n"
                     "    }\n",
                     get_type_from_ref(item_ref)));
  }
  CHECK_IO(fprintf(fp, "    if (!*json_out) return ENOMEM;\n  }\n"));
  CHECK_IO(fprintf(fp, "  jasprintf(json_out, \"]\");\n  if(!*json_out) "
                       "return ENOMEM;\n  return 0;\n}\n"));

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->json_guard));

  return 0;
}

int write_root_array_from_json_func(
    FILE *const fp, const char *const name, const char *const item_type,
    const char *const item_ref, const struct CodegenTypesConfig *const config) {
  if (!fp || !name || !item_type)
    return EINVAL;

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->json_guard));

  /* Choose arg signature */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(fprintf(
        fp, "int %s_from_json(const char *json, int **out, size_t *len) {\n",
        name));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_IO(fprintf(fp,
                     "int %s_from_json(const char *json, char ***out, size_t "
                     "*len) {\n",
                     name));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_IO(
        fprintf(fp,
                "int %s_from_json(const char *json, struct %s ***out, size_t "
                "*len) {\n",
                name, get_type_from_ref(item_ref)));
  } else {
    /* Fallback */
    CHECK_IO(fprintf(fp,
                     "int %s_from_json(const char *json, void **out, size_t "
                     "*len) {\n",
                     name));
  }

  CHECK_IO(fprintf(
      fp, "  JSON_Value *val;\n"
          "  JSON_Array *arr;\n"
          "  size_t i, count;\n"
          "  if (!json || !out || !len) return EINVAL;\n"
          "  val = json_parse_string(json);\n"
          "  if (!val) return EINVAL;\n"
          "  arr = json_value_get_array(val);\n"
          "  if (!arr) { json_value_free(val); return EINVAL; }\n"
          "  count = json_array_get_count(arr);\n"
          "  *len = count;\n"
          "  if (count == 0) { *out = NULL; json_value_free(val); return 0; "
          "}\n"));

  /* Allocate array container */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(fprintf(fp, "  *out = malloc(count * sizeof(int));\n"));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_IO(fprintf(fp, "  *out = calloc(count, sizeof(char*));\n"));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_IO(fprintf(fp, "  *out = calloc(count, sizeof(struct %s*));\n",
                     get_type_from_ref(item_ref)));
  }

  CHECK_IO(fprintf(fp, "  if (!*out) { json_value_free(val); return ENOMEM; }\n"
                       "  for (i = 0; i < count; ++i) {\n"));

  /* Parse Loop */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(
        fprintf(fp, "    (*out)[i] = (int)json_array_get_number(arr, i);\n"));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_IO(fprintf(
        fp,
        "    const char *s = json_array_get_string(arr, i);\n"
        "    if (s) (*out)[i] = c_cdd_strdup(s);\n"
        "    if (!(*out)[i]) {\n"
        "      /* cleanup */\n"
        "      size_t j;\n"
        "      for(j=0; j<i; j++) free((*out)[j]);\n"
        "      free(*out); *out=NULL; json_value_free(val); return ENOMEM;\n"
        "    }\n"));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_IO(fprintf(
        fp,
        "    int rc = %s_from_jsonObject(json_array_get_object(arr, i), "
        "&(*out)[i]);\n"
        "    if (rc != 0) {\n"
        "      size_t j;\n"
        "      for(j=0; j<i; j++) %s_cleanup((*out)[j]);\n"
        "      free(*out); *out=NULL; json_value_free(val); return rc;\n"
        "    }\n",
        get_type_from_ref(item_ref), get_type_from_ref(item_ref)));
  }

  CHECK_IO(fprintf(fp, "  }\n  json_value_free(val);\n  return 0;\n}\n"));

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->json_guard));

  return 0;
}
