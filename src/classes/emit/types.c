/**
 * @file codegen_types.c
 * @brief Implementation of Advanced Types generation.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/struct.h" /* for get_type_from_ref */
#include "classes/emit/types.h"
#include "functions/parse/str.h"

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
    const char *t = sf->fields[i].type;
    if (strcmp(t, "object") == 0) {
      needs_nested_rc = 1;
    } else if (strcmp(t, "array") == 0) {
      const char *item = sf->fields[i].ref;
      if (!item ||
          (strcmp(item, "integer") != 0 && strcmp(item, "number") != 0 &&
           strcmp(item, "string") != 0 && strcmp(item, "boolean") != 0))
        needs_nested_rc = 1;
    }
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
    if (strcmp(type, "integer") == 0) {
      CHECK_IO(
          fprintf(fp, "      jasprintf(json, \"%%d\", obj->data.%s);\n", name));
    } else if (strcmp(type, "number") == 0) {
      CHECK_IO(
          fprintf(fp, "      jasprintf(json, \"%%g\", obj->data.%s);\n", name));
    } else if (strcmp(type, "boolean") == 0) {
      CHECK_IO(fprintf(fp,
                       "      jasprintf(json, \"%s\", obj->data.%s ? "
                       "\"true\" : \"false\");\n",
                       "%s", name));
    } else if (strcmp(type, "string") == 0) {
      CHECK_IO(fprintf(fp, "      if (obj->data.%s) {\n", name));
      CHECK_IO(fprintf(
          fp, "        jasprintf(json, \"\\\"%%s\\\"\", obj->data.%s);\n",
          name));
      CHECK_IO(fprintf(fp, "      } else { jasprintf(json, \"null\"); }\n"));
    } else if (strcmp(type, "enum") == 0) {
      CHECK_IO(fprintf(fp,
                       "      { char *s = NULL; rc = %s_to_str(obj->data.%s, "
                       "&s); if (rc != 0) return rc;\n",
                       get_type_from_ref(ref), name));
      CHECK_IO(fprintf(fp,
                       "        jasprintf(json, \"\\\"%%s\\\"\", s); free(s); "
                       "}\n"));
    } else if (strcmp(type, "object") == 0) {
      CHECK_IO(fprintf(fp,
                       "      {\n"
                       "        char *sub = NULL;\n"
                       "        rc = %s_to_json(obj->data.%s, &sub);\n"
                       "        if (rc != 0) return rc;\n"
                       "        jasprintf(json, \"%%s\", sub);\n"
                       "        free(sub);\n"
                       "      }\n",
                       get_type_from_ref(ref), name));
    } else if (strcmp(type, "array") == 0) {
      CHECK_IO(fprintf(fp,
                       "      {\n"
                       "        size_t i;\n"
                       "        jasprintf(json, \"[\");\n"
                       "        if (!*json) return ENOMEM;\n"
                       "        for (i = 0; i < obj->data.%s.n_%s; ++i) {\n"
                       "          if (i > 0) { jasprintf(json, \",\"); if "
                       "(!*json) return ENOMEM; }\n",
                       name, name));
      if (strcmp(ref, "integer") == 0) {
        CHECK_IO(fprintf(fp,
                         "          jasprintf(json, \"%%d\", "
                         "obj->data.%s.%s[i]);\n",
                         name, name));
      } else if (strcmp(ref, "number") == 0) {
        CHECK_IO(fprintf(fp,
                         "          jasprintf(json, \"%%g\", "
                         "obj->data.%s.%s[i]);\n",
                         name, name));
      } else if (strcmp(ref, "boolean") == 0) {
        CHECK_IO(fprintf(fp,
                         "          jasprintf(json, \"%%s\", "
                         "obj->data.%s.%s[i] ? \"true\" : \"false\");\n",
                         name, name));
      } else if (strcmp(ref, "string") == 0) {
        CHECK_IO(fprintf(fp,
                         "          jasprintf(json, \"\\\"%%s\\\"\", "
                         "obj->data.%s.%s[i]);\n",
                         name, name));
      } else {
        CHECK_IO(
            fprintf(fp,
                    "          {\n"
                    "            char *sub = NULL;\n"
                    "            rc = %s_to_json(obj->data.%s.%s[i], &sub);\n"
                    "            if (rc != 0) return rc;\n"
                    "            jasprintf(json, \"%%s\", sub);\n"
                    "            free(sub);\n"
                    "          }\n",
                    get_type_from_ref(ref), name, name));
      }
      CHECK_IO(fprintf(fp, "          if (!*json) return ENOMEM;\n"
                           "        }\n"
                           "        jasprintf(json, \"]\");\n"
                           "        if (!*json) return ENOMEM;\n"
                           "      }\n"));
    } else if (strcmp(type, "null") == 0) {
      CHECK_IO(fprintf(fp, "      jasprintf(json, \"null\");\n"));
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

  CHECK_IO(
      fprintf(fp,
              "  struct %s *ret = malloc(sizeof(struct %s));\n"
              "  if (!ret) return ENOMEM;\n"
              "  memset(ret, 0, sizeof(*ret));\n"
              "  if (!jsonObject || !out) { free(ret); return EINVAL; }\n\n",
              union_name, union_name));

  if (sf->union_discriminator && sf->union_discriminator[0]) {
    CHECK_IO(fprintf(
        fp,
        "  {\n"
        "    const char *disc = json_object_get_string(jsonObject, \"%s\");\n"
        "    if (disc) {\n",
        sf->union_discriminator));
    for (i = 0; i < sf->size; ++i) {
      const char *name = sf->fields[i].name;
      const char *type = sf->fields[i].type;
      const char *ref = sf->fields[i].ref;
      const char *disc_val = NULL;
      if (sf->union_variants && i < sf->n_union_variants)
        disc_val = sf->union_variants[i].disc_value;
      if (!disc_val)
        continue;
      if (strcmp(type, "object") != 0)
        continue;
      CHECK_IO(
          fprintf(fp, "      if (strcmp(disc, \"%s\") == 0) {\n", disc_val));
      CHECK_IO(fprintf(fp, "        ret->tag = %s_%s;\n", union_name, name));
      CHECK_IO(fprintf(fp,
                       "        rc = %s_from_jsonObject(jsonObject, "
                       "&ret->data.%s);\n"
                       "        if (rc != 0) { free(ret); return rc; }\n"
                       "        *out = ret;\n"
                       "        return 0;\n"
                       "      }\n",
                       get_type_from_ref(ref), name));
    }
    CHECK_IO(fprintf(fp, "    }\n  }\n"));
  }

  CHECK_IO(fprintf(fp, "  {\n    int match_count = 0;\n"
                       "    int match_idx = -1;\n"));
  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const struct UnionVariantMeta *meta =
        (sf->union_variants && i < sf->n_union_variants)
            ? &sf->union_variants[i]
            : NULL;
    size_t k;

    if (strcmp(type, "object") != 0)
      continue;

    if (meta && meta->n_required_props > 0) {
      CHECK_IO(fprintf(fp, "    if ("));
      for (k = 0; k < meta->n_required_props; ++k) {
        const char *req = meta->required_props[k];
        if (!req)
          continue;
        if (k > 0)
          CHECK_IO(fprintf(fp, " && "));
        CHECK_IO(fprintf(fp, "json_object_has_value(jsonObject, \"%s\")", req));
      }
      CHECK_IO(fprintf(fp,
                       ") { match_count++; if (match_idx < 0) match_idx = "
                       "%zu; }\n",
                       i));
    } else if (meta && meta->n_property_names > 0) {
      CHECK_IO(fprintf(fp, "    if ("));
      for (k = 0; k < meta->n_property_names; ++k) {
        const char *prop = meta->property_names[k];
        if (!prop)
          continue;
        if (k > 0)
          CHECK_IO(fprintf(fp, " || "));
        CHECK_IO(
            fprintf(fp, "json_object_has_value(jsonObject, \"%s\")", prop));
      }
      CHECK_IO(fprintf(fp,
                       ") { match_count++; if (match_idx < 0) match_idx = "
                       "%zu; }\n",
                       i));
    } else {
      CHECK_IO(fprintf(fp,
                       "    if (json_object_get_count(jsonObject) > 0) { "
                       "match_count++; if (match_idx < 0) match_idx = %zu; }\n",
                       i));
    }
  }

  if (!sf->union_is_anyof) {
    CHECK_IO(fprintf(
        fp, "    if (match_count > 1) { free(ret); return EINVAL; }\n"));
  }

  CHECK_IO(fprintf(fp, "    if (match_idx < 0) { free(ret); return EINVAL; }\n"
                       "    switch (match_idx) {\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;
    if (strcmp(type, "object") != 0)
      continue;
    CHECK_IO(fprintf(fp, "    case %zu:\n", i));
    CHECK_IO(fprintf(fp, "      ret->tag = %s_%s;\n", union_name, name));
    CHECK_IO(fprintf(fp,
                     "      rc = %s_from_jsonObject(jsonObject, "
                     "&ret->data.%s);\n"
                     "      if (rc != 0) { free(ret); return rc; }\n"
                     "      break;\n",
                     get_type_from_ref(ref), name));
  }

  CHECK_IO(fprintf(fp, "    default:\n"
                       "      free(ret);\n"
                       "      return EINVAL;\n"
                       "    }\n"
                       "  }\n"
                       "  *out = ret;\n"
                       "  return 0;\n}\n"));

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->json_guard));

  return 0;
}

int write_union_from_json_func(FILE *const fp, const char *const union_name,
                               const struct StructFields *const sf,
                               const struct CodegenTypesConfig *const config) {
  size_t i;
  int has_object = 0;
  int string_count = 0;
  int int_count = 0;
  int num_count = 0;
  int bool_count = 0;
  int null_count = 0;
  int array_count = 0;
  int array_needs_rc = 0;
  size_t string_idx = 0;
  size_t int_idx = 0;
  size_t num_idx = 0;
  size_t bool_idx = 0;
  size_t null_idx = 0;
  size_t array_idx = 0;

  if (!fp || !union_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    enum UnionVariantJsonType jtype = UNION_JSON_UNKNOWN;
    const char *t = sf->fields[i].type;
    if (sf->union_variants && i < sf->n_union_variants)
      jtype = sf->union_variants[i].json_type;
    if (jtype == UNION_JSON_UNKNOWN && t) {
      if (strcmp(t, "object") == 0)
        jtype = UNION_JSON_OBJECT;
      else if (strcmp(t, "string") == 0 || strcmp(t, "enum") == 0)
        jtype = UNION_JSON_STRING;
      else if (strcmp(t, "integer") == 0)
        jtype = UNION_JSON_INTEGER;
      else if (strcmp(t, "number") == 0)
        jtype = UNION_JSON_NUMBER;
      else if (strcmp(t, "boolean") == 0)
        jtype = UNION_JSON_BOOLEAN;
      else if (strcmp(t, "array") == 0)
        jtype = UNION_JSON_ARRAY;
      else if (strcmp(t, "null") == 0)
        jtype = UNION_JSON_NULL;
    }

    switch (jtype) {
    case UNION_JSON_OBJECT:
      has_object = 1;
      break;
    case UNION_JSON_STRING:
      if (string_count == 0)
        string_idx = i;
      string_count++;
      break;
    case UNION_JSON_INTEGER:
      if (int_count == 0)
        int_idx = i;
      int_count++;
      break;
    case UNION_JSON_NUMBER:
      if (num_count == 0)
        num_idx = i;
      num_count++;
      break;
    case UNION_JSON_BOOLEAN:
      if (bool_count == 0)
        bool_idx = i;
      bool_count++;
      break;
    case UNION_JSON_NULL:
      if (null_count == 0)
        null_idx = i;
      null_count++;
      break;
    case UNION_JSON_ARRAY: {
      const char *item = sf->fields[i].ref;
      if (array_count == 0)
        array_idx = i;
      array_count++;
      if (!item ||
          (strcmp(item, "integer") != 0 && strcmp(item, "number") != 0 &&
           strcmp(item, "string") != 0 && strcmp(item, "boolean") != 0))
        array_needs_rc = 1;
      break;
    }
    default:
      break;
    }
  }

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->json_guard));

  CHECK_IO(fprintf(fp,
                   "int %s_from_json(const char *const json, struct %s **const "
                   "out) {\n",
                   union_name, union_name));

  if (has_object || array_needs_rc)
    CHECK_IO(fprintf(fp, "  int rc;\n"));

  CHECK_IO(fprintf(fp, "  JSON_Value *val;\n"
                       "  JSON_Value_Type typ;\n"
                       "  if (!json || !out) return EINVAL;\n"
                       "  val = json_parse_string(json);\n"
                       "  if (!val) return EINVAL;\n"
                       "  typ = json_value_get_type(val);\n"
                       "  switch (typ) {\n"));

  CHECK_IO(fprintf(fp, "    case JSONObject:\n"));
  if (has_object) {
    CHECK_IO(
        fprintf(fp,
                "      rc = %s_from_jsonObject(json_value_get_object(val), "
                "out);\n"
                "      json_value_free(val);\n"
                "      return rc;\n",
                union_name));
  } else {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  }

  CHECK_IO(fprintf(fp, "    case JSONArray:\n"));
  if (array_count == 0) {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  } else if (!sf->union_is_anyof && array_count > 1) {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  } else {
    const char *name = sf->fields[array_idx].name;
    const char *ref = sf->fields[array_idx].ref;
    CHECK_IO(
        fprintf(fp,
                "      {\n"
                "        JSON_Array *arr = json_value_get_array(val);\n"
                "        size_t i, count;\n"
                "        struct %s *ret = malloc(sizeof(struct %s));\n"
                "        if (!ret) { json_value_free(val); return ENOMEM; }\n"
                "        memset(ret, 0, sizeof(*ret));\n"
                "        ret->tag = %s_%s;\n"
                "        count = json_array_get_count(arr);\n"
                "        ret->data.%s.n_%s = count;\n"
                "        if (count > 0) {\n",
                union_name, union_name, union_name, name, name, name));
    if (strcmp(ref, "integer") == 0) {
      CHECK_IO(
          fprintf(fp,
                  "          ret->data.%s.%s = malloc(count * sizeof(int));\n"
                  "          if (!ret->data.%s.%s) { free(ret); "
                  "json_value_free(val); return ENOMEM; }\n"
                  "          for (i = 0; i < count; ++i) ret->data.%s.%s[i] = "
                  "(int)json_array_get_number(arr, i);\n",
                  name, name, name, name, name, name));
    } else if (strcmp(ref, "number") == 0) {
      CHECK_IO(fprintf(
          fp,
          "          ret->data.%s.%s = malloc(count * sizeof(double));\n"
          "          if (!ret->data.%s.%s) { free(ret); json_value_free(val); "
          "return ENOMEM; }\n"
          "          for (i = 0; i < count; ++i) ret->data.%s.%s[i] = "
          "json_array_get_number(arr, i);\n",
          name, name, name, name, name, name));
    } else if (strcmp(ref, "boolean") == 0) {
      CHECK_IO(
          fprintf(fp,
                  "          ret->data.%s.%s = malloc(count * sizeof(int));\n"
                  "          if (!ret->data.%s.%s) { free(ret); "
                  "json_value_free(val); return ENOMEM; }\n"
                  "          for (i = 0; i < count; ++i) ret->data.%s.%s[i] = "
                  "json_array_get_boolean(arr, i) ? 1 : 0;\n",
                  name, name, name, name, name, name));
    } else if (strcmp(ref, "string") == 0) {
      CHECK_IO(fprintf(
          fp,
          "          ret->data.%s.%s = calloc(count, sizeof(char*));\n"
          "          if (!ret->data.%s.%s) { free(ret); json_value_free(val); "
          "return ENOMEM; }\n"
          "          for (i = 0; i < count; ++i) {\n"
          "            const char *s = json_array_get_string(arr, i);\n"
          "            if (s) ret->data.%s.%s[i] = strdup(s);\n"
          "            if (!ret->data.%s.%s[i]) {\n"
          "              size_t j;\n"
          "              for (j = 0; j < i; ++j) free(ret->data.%s.%s[j]);\n"
          "              free(ret->data.%s.%s);\n"
          "              free(ret);\n"
          "              json_value_free(val);\n"
          "              return ENOMEM;\n"
          "            }\n"
          "          }\n",
          name, name, name, name, name, name, name, name, name, name, name,
          name));
    } else {
      CHECK_IO(fprintf(
          fp,
          "          ret->data.%s.%s = calloc(count, sizeof(struct %s*));\n"
          "          if (!ret->data.%s.%s) { free(ret); json_value_free(val); "
          "return ENOMEM; }\n"
          "          for (i = 0; i < count; ++i) {\n"
          "            rc = %s_from_jsonObject(json_array_get_object(arr, i), "
          "&ret->data.%s.%s[i]);\n"
          "            if (rc != 0) {\n"
          "              size_t j;\n"
          "              for (j = 0; j < i; ++j) { "
          "%s_cleanup(ret->data.%s.%s[j]); free(ret->data.%s.%s[j]); }\n"
          "              free(ret->data.%s.%s);\n"
          "              free(ret);\n"
          "              json_value_free(val);\n"
          "              return rc;\n"
          "            }\n"
          "          }\n",
          name, name, get_type_from_ref(ref), name, name,
          get_type_from_ref(ref), name, name, get_type_from_ref(ref), name,
          name, name, name, name, name));
    }
    CHECK_IO(fprintf(fp, "        }\n"
                         "        *out = ret;\n"
                         "        json_value_free(val);\n"
                         "        return 0;\n"
                         "      }\n"));
  }

  CHECK_IO(fprintf(fp, "    case JSONString:\n"));
  if (string_count == 0) {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  } else {
    const char *name = sf->fields[string_idx].name;
    if (!sf->union_is_anyof && string_count > 1) {
      CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                           "      return EINVAL;\n"));
    } else {
      CHECK_IO(
          fprintf(fp,
                  "      {\n"
                  "        const char *s = json_value_get_string(val);\n"
                  "        struct %s *ret;\n"
                  "        if (!s) { json_value_free(val); return EINVAL; }\n"
                  "        ret = malloc(sizeof(struct %s));\n"
                  "        if (!ret) { json_value_free(val); return ENOMEM; }\n"
                  "        memset(ret, 0, sizeof(*ret));\n"
                  "        ret->tag = %s_%s;\n"
                  "        ret->data.%s = strdup(s);\n"
                  "        if (!ret->data.%s) { free(ret); "
                  "json_value_free(val); return ENOMEM; }\n"
                  "        *out = ret;\n"
                  "        json_value_free(val);\n"
                  "        return 0;\n"
                  "      }\n",
                  union_name, union_name, union_name, name, name, name));
    }
  }

  CHECK_IO(fprintf(fp, "    case JSONNumber:\n"));
  if (int_count == 0 && num_count == 0) {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  } else if (!sf->union_is_anyof && (int_count > 1 || num_count > 1)) {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  } else {
    CHECK_IO(fprintf(fp, "      {\n"
                         "        double num = json_value_get_number(val);\n"));
    if (int_count > 0 && num_count == 0) {
      const char *name = sf->fields[int_idx].name;
      CHECK_IO(fprintf(
          fp,
          "        if (num != (int)num) { json_value_free(val); return EINVAL; "
          "}\n"
          "        { struct %s *ret = malloc(sizeof(struct %s));\n"
          "          if (!ret) { json_value_free(val); return ENOMEM; }\n"
          "          memset(ret, 0, sizeof(*ret));\n"
          "          ret->tag = %s_%s;\n"
          "          ret->data.%s = (int)num;\n"
          "          *out = ret;\n"
          "          json_value_free(val);\n"
          "          return 0; }\n"
          "      }\n",
          union_name, union_name, union_name, name, name));
    } else if (int_count == 0 && num_count > 0) {
      const char *name = sf->fields[num_idx].name;
      CHECK_IO(fprintf(
          fp,
          "        { struct %s *ret = malloc(sizeof(struct %s));\n"
          "          if (!ret) { json_value_free(val); return ENOMEM; }\n"
          "          memset(ret, 0, sizeof(*ret));\n"
          "          ret->tag = %s_%s;\n"
          "          ret->data.%s = num;\n"
          "          *out = ret;\n"
          "          json_value_free(val);\n"
          "          return 0; }\n"
          "      }\n",
          union_name, union_name, union_name, name, name));
    } else {
      const char *int_name = sf->fields[int_idx].name;
      const char *num_name = sf->fields[num_idx].name;
      CHECK_IO(fprintf(
          fp,
          "        if (num == (int)num) {\n"
          "          struct %s *ret = malloc(sizeof(struct %s));\n"
          "          if (!ret) { json_value_free(val); return ENOMEM; }\n"
          "          memset(ret, 0, sizeof(*ret));\n"
          "          ret->tag = %s_%s;\n"
          "          ret->data.%s = (int)num;\n"
          "          *out = ret;\n"
          "          json_value_free(val);\n"
          "          return 0;\n"
          "        } else {\n"
          "          struct %s *ret = malloc(sizeof(struct %s));\n"
          "          if (!ret) { json_value_free(val); return ENOMEM; }\n"
          "          memset(ret, 0, sizeof(*ret));\n"
          "          ret->tag = %s_%s;\n"
          "          ret->data.%s = num;\n"
          "          *out = ret;\n"
          "          json_value_free(val);\n"
          "          return 0;\n"
          "        }\n"
          "      }\n",
          union_name, union_name, union_name, int_name, int_name, union_name,
          union_name, union_name, num_name, num_name));
    }
  }

  CHECK_IO(fprintf(fp, "    case JSONBoolean:\n"));
  if (bool_count == 0) {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  } else if (!sf->union_is_anyof && bool_count > 1) {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  } else {
    const char *name = sf->fields[bool_idx].name;
    CHECK_IO(
        fprintf(fp,
                "      {\n"
                "        struct %s *ret = malloc(sizeof(struct %s));\n"
                "        if (!ret) { json_value_free(val); return ENOMEM; }\n"
                "        memset(ret, 0, sizeof(*ret));\n"
                "        ret->tag = %s_%s;\n"
                "        ret->data.%s = json_value_get_boolean(val) ? 1 : 0;\n"
                "        *out = ret;\n"
                "        json_value_free(val);\n"
                "        return 0;\n"
                "      }\n",
                union_name, union_name, union_name, name, name));
  }

  CHECK_IO(fprintf(fp, "    case JSONNull:\n"));
  if (null_count == 0) {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  } else if (!sf->union_is_anyof && null_count > 1) {
    CHECK_IO(fprintf(fp, "      json_value_free(val);\n"
                         "      return EINVAL;\n"));
  } else {
    const char *name = sf->fields[null_idx].name;
    CHECK_IO(
        fprintf(fp,
                "      {\n"
                "        struct %s *ret = malloc(sizeof(struct %s));\n"
                "        if (!ret) { json_value_free(val); return ENOMEM; }\n"
                "        memset(ret, 0, sizeof(*ret));\n"
                "        ret->tag = %s_%s;\n"
                "        ret->data.%s = 0;\n"
                "        *out = ret;\n"
                "        json_value_free(val);\n"
                "        return 0;\n"
                "      }\n",
                union_name, union_name, union_name, name, name));
  }

  CHECK_IO(fprintf(fp, "    default:\n"
                       "      json_value_free(val);\n"
                       "      return EINVAL;\n"
                       "  }\n"
                       "}\n"));

  if (config && config->json_guard)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->json_guard));

  return 0;
}

int write_union_cleanup_func(FILE *const fp, const char *const union_name,
                             const struct StructFields *const sf,
                             const struct CodegenTypesConfig *const config) {
  size_t i;
  int iter_needed = 0;
  if (!fp || !union_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0) {
      iter_needed = 1;
      break;
    }
  }

  if (config && config->utils_guard)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->utils_guard));

  CHECK_IO(fprintf(fp, "void %s_cleanup(struct %s *const obj) {\n", union_name,
                   union_name));
  CHECK_IO(fprintf(fp, "  if (!obj) return;\n"));
  if (iter_needed)
    CHECK_IO(fprintf(fp, "  size_t i;\n"));
  CHECK_IO(fprintf(fp, "  switch (obj->tag) {\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    CHECK_IO(fprintf(fp, "    case %s_%s:\n", union_name, name));
    if (strcmp(sf->fields[i].type, "string") == 0) {
      CHECK_IO(fprintf(fp, "      free((void*)obj->data.%s);\n", name));
    } else if (strcmp(sf->fields[i].type, "object") == 0) {
      CHECK_IO(fprintf(fp, "      %s_cleanup(obj->data.%s);\n",
                       get_type_from_ref(sf->fields[i].ref), name));
    } else if (strcmp(sf->fields[i].type, "array") == 0) {
      const char *r = sf->fields[i].ref;
      if (strcmp(r, "string") == 0 ||
          (strcmp(r, "integer") != 0 && strcmp(r, "boolean") != 0 &&
           strcmp(r, "number") != 0)) {
        CHECK_IO(fprintf(fp,
                         "      for (i = 0; i < obj->data.%s.n_%s; ++i) {\n",
                         name, name));
        if (strcmp(r, "string") == 0) {
          CHECK_IO(
              fprintf(fp, "        free(obj->data.%s.%s[i]);\n", name, name));
        } else {
          CHECK_IO(fprintf(fp,
                           "        %s_cleanup(obj->data.%s.%s[i]);\n"
                           "        free(obj->data.%s.%s[i]);\n",
                           get_type_from_ref(r), name, name, name, name));
        }
        CHECK_IO(fprintf(fp, "      }\n"));
      }
      CHECK_IO(fprintf(fp, "      free(obj->data.%s.%s);\n", name, name));
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
