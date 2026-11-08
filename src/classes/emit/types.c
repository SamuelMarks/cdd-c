/**
 * @file types.c
 * @brief Implementation of Advanced Types generation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/emit/types.h"
#include "c_cdd/code_writer.h"
#include "c_cdd/log.h"
#include "classes/emit/struct.h" /* for get_type_from_ref */
#include "classes/parse/cdd_serde.h"
#include "functions/parse/str.h"
#include "win_compat_sym.h"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

/* --- Union Implementation --- */

/*
 * @brief Emits C source code for union to json func.
 *
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union type.
 * @param[in] sf Struct fields descriptor container for the union variants.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_union_to_json_func(FILE *fp, const char *union_name,
                         const struct StructFields *sf,
                         const struct CodegenTypesConfig *config) {
  size_t i;
  cdd_c_error_t rc_esc = CDD_C_SUCCESS;

  if (!fp || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->json_guard));

  CHECK_IO(FPRINTF_HOOK(fp,
                        "cdd_c_error_t %s_to_json(const struct %s *obj, "
                        "char **const json) {\n"
                        "  int rc;\n",
                        union_name, union_name));

  CHECK_IO(FPRINTF_HOOK(fp, "  if (obj == NULL || json == NULL) return "
                            "CDD_C_ERROR_INVALID_ARGUMENT;\n"
                            "  switch (obj->tag) {\n"));

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *vf = &sf->fields[i];
    const char *name = vf->name;
    const char *type = vf->type;
    const char *ref = vf->ref;
    const char *ref_type = NULL;
    const char *disc_tag =
        (sf->serde_tag[0] != '\0') ? sf->serde_tag : sf->union_discriminator;
    const char *disc_val = NULL;
    if (vf->serde_rename[0] != '\0') {
      disc_val = vf->serde_rename;
    } else if (sf->union_variants && i < sf->n_union_variants &&
               sf->union_variants[i].disc_value) {
      disc_val = sf->union_variants[i].disc_value;
    } else {
      disc_val = name;
    }

    CHECK_IO(FPRINTF_HOOK(fp, "    case %s_%s:\n", union_name, name));
    if (disc_tag && disc_tag[0] != '\0' && sf->serde_content[0] != '\0' &&
        !sf->serde_untagged) {
      char esc_tag[128], esc_val[128], esc_cnt[128];
      rc_esc = cdd_escape_json_c_str(disc_tag, esc_tag, sizeof(esc_tag));
      rc_esc = cdd_escape_json_c_str(disc_val, esc_val, sizeof(esc_val));
      rc_esc =
          cdd_escape_json_c_str(sf->serde_content, esc_cnt, sizeof(esc_cnt));
      CHECK_IO(FPRINTF_HOOK(fp,
                            "      rc = c89stringutils_jasprintf(json, "
                            "\"{\\\"%s\\\":\\\"%s\\\",\\\"%s\\\":\");\n"
                            "      if (rc < 0 || !*json) return "
                            "CDD_C_ERROR_MEMORY;\n",
                            esc_tag, esc_val, esc_cnt));
    }
    if (strcmp(type, "integer") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "      rc = c89stringutils_jasprintf(json, "
                            "\"%%d\", obj->data.%s);\n",
                            name));
    } else if (strcmp(type, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "      rc = c89stringutils_jasprintf(json, "
                            "\"%%g\", obj->data.%s);\n",
                            name));
    } else if (strcmp(type, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      rc = c89stringutils_jasprintf(json, \"%s\", obj->data.%s ? "
          "\"true\" : \"false\");\n",
          "%s", name));
    } else if (strcmp(type, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp, "      if (obj->data.%s) {\n", name));
      CHECK_IO(FPRINTF_HOOK(fp,
                            "        rc = c89stringutils_jasprintf(json, "
                            "\"\\\"%%s\\\"\", obj->data.%s);\n",
                            name));
      CHECK_IO(FPRINTF_HOOK(fp, "      } else { rc = c89stringutils_jasprintf("
                                "json, \"null\"); }\n"));
    } else if (strcmp(type, "enum") == 0) {
      cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      { char *s = NULL; rc = %s_to_str(obj->data.%s, "
          "&s); if (rc != CDD_C_SUCCESS) { free(s); return rc; }\n",
          ref_type, name));
      CHECK_IO(FPRINTF_HOOK(fp, "        rc = c89stringutils_jasprintf(json, "
                                "\"\\\"%%s\\\"\", s); free(s); "
                                "}\n"));
    } else if (strcmp(type, "object") == 0) {
      cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      if (sf->serde_tag[0] != '\0' && sf->serde_content[0] == '\0' &&
          !sf->serde_untagged) {
        char esc_tag[128], esc_val[128];
        rc_esc = cdd_escape_json_c_str(sf->serde_tag, esc_tag, sizeof(esc_tag));
        rc_esc = cdd_escape_json_c_str(disc_val, esc_val, sizeof(esc_val));
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "      {\n"
            "        char *sub = NULL;\n"
            "        rc = %s_to_json(obj->data.%s, &sub);\n"
            "        if (rc != CDD_C_SUCCESS) { free(sub); return rc; }\n",
            ref_type, name));
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        if (sub && sub[0] == '{' && strncmp(sub + 1, "
            "\"\\\"%s\\\":\", %lu) != 0) {\n"
            "          if (sub[1] == '}') {\n"
            "            rc = c89stringutils_jasprintf(json, "
            "\"{\\\"%s\\\":\\\"%s\\\"}\");\n"
            "          } else {\n"
            "            rc = c89stringutils_jasprintf(json, "
            "\"{\\\"%s\\\":\\\"%s\\\",%%s\", sub + 1);\n"
            "          }\n"
            "        } else {\n"
            "          rc = c89stringutils_jasprintf(json, \"%%s\", sub);\n"
            "        }\n"
            "        free(sub);\n"
            "      }\n",
            esc_tag, (unsigned long)(strlen(esc_tag) + 3), esc_tag, esc_val,
            esc_tag, esc_val));
      } else {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "      {\n"
            "        char *sub = NULL;\n"
            "        rc = %s_to_json(obj->data.%s, &sub);\n"
            "        if (rc != CDD_C_SUCCESS) { free(sub); return rc; }\n"
            "        rc = c89stringutils_jasprintf(json, \"%%s\", sub);\n"
            "        free(sub);\n"
            "      }\n",
            ref_type, name));
      }
    } else if (strcmp(type, "array") == 0) {
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "      {\n"
                       "        size_t i;\n"
                       "        rc = c89stringutils_jasprintf(json, \"[\");\n"
                       "        if (rc < 0 || !*json) return "
                       "CDD_C_ERROR_MEMORY;\n"
                       "        for (i = 0; i < obj->data.%s.n_%s; ++i) {\n"
                       "          if (i > 0) { rc = c89stringutils_jasprintf("
                       "json, \",\"); if "
                       "(rc < 0 || !*json) return CDD_C_ERROR_MEMORY; }\n",
                       name, name));
      if (strcmp(ref, "integer") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "          rc = c89stringutils_jasprintf(json, \"%%d\", "
            "obj->data.%s.%s[i]);\n",
            name, name));
      } else if (strcmp(ref, "number") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "          rc = c89stringutils_jasprintf(json, \"%%g\", "
            "obj->data.%s.%s[i]);\n",
            name, name));
      } else if (strcmp(ref, "boolean") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "          rc = c89stringutils_jasprintf(json, \"%%s\", "
            "obj->data.%s.%s[i] ? \"true\" : \"false\");\n",
            name, name));
      } else if (strcmp(ref, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "          rc = c89stringutils_jasprintf(json, \"\\\"%%s\\\"\", "
            "obj->data.%s.%s[i]);\n",
            name, name));
      } else {
        cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
        if (rc_ref != CDD_C_SUCCESS)
          return rc_ref;
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "          {\n"
            "            char *sub = NULL;\n"
            "            rc = %s_to_json(obj->data.%s.%s[i], &sub);\n"
            "            if (rc != CDD_C_SUCCESS) { free(sub); return rc; }\n"
            "            rc = c89stringutils_jasprintf(json, \"%%s\", sub);\n"
            "            free(sub);\n"
            "          }\n",
            ref_type, name, name));
      }
      CHECK_IO(FPRINTF_HOOK(
          fp, "          if (rc < 0 || !*json) return CDD_C_ERROR_MEMORY;\n"
              "        }\n"
              "        rc = c89stringutils_jasprintf(json, \"]\");\n"
              "      }\n"));
    } else if (strcmp(type, "null") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "      rc = c89stringutils_jasprintf(json, \"null\");\n"));
    }
    if (disc_tag && disc_tag[0] != '\0' && sf->serde_content[0] != '\0' &&
        !sf->serde_untagged) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "      if (rc < 0 || !*json) return CDD_C_ERROR_MEMORY;\n"
              "      rc = c89stringutils_jasprintf(json, \"}\");\n"));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "      break;\n"));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "    default:\n"
          "      rc = c89stringutils_jasprintf(json, \"null\");\n"
          "      break;\n"
          "  }\n"
          "  if (rc < 0 || *json == NULL) return CDD_C_ERROR_MEMORY;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"));

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->json_guard));

  return rc_esc;
}

/*
 * @brief Emits C source code for union from jsonObject func.
 *
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union type.
 * @param[in] sf Struct fields descriptor container for the union variants.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_union_from_jsonObject_func(FILE *fp, const char *union_name,
                                 const struct StructFields *sf,
                                 const struct CodegenTypesConfig *config) {
  size_t i;
  int needs_nested_rc = 0;
  const char *disc_tag;
  cdd_c_error_t rc_esc = CDD_C_SUCCESS;

  if (!fp || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  disc_tag =
      (sf->serde_tag[0] != '\0') ? sf->serde_tag : sf->union_discriminator;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "object") == 0)
      needs_nested_rc = 1;
  }

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->json_guard));

  CHECK_IO(FPRINTF_HOOK(fp,
                        "cdd_c_error_t %s_from_jsonObject(const JSON_Object *"
                        "jsonObject, struct %s **const out) {\n",
                        union_name, union_name));

  if (needs_nested_rc)
    CHECK_IO(FPRINTF_HOOK(fp, "  int rc;\n"));

  CHECK_IO(FPRINTF_HOOK(fp,
                        "  struct %s *ret = malloc(sizeof(struct %s));\n"
                        "  if (!ret) return CDD_C_ERROR_MEMORY;\n"
                        "  memset(ret, 0, sizeof(*ret));\n"
                        "  if (!jsonObject || !out) { free(ret); return "
                        "CDD_C_ERROR_INVALID_ARGUMENT; }\n\n",
                        union_name, union_name));

  if (!sf->serde_untagged && disc_tag && disc_tag[0]) {
    char esc_tag[128];
    rc_esc = cdd_escape_json_c_str(disc_tag, esc_tag, sizeof(esc_tag));
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "  {\n"
        "    const char *disc = json_object_get_string(jsonObject, \"%s\");\n"
        "    if (disc) {\n",
        esc_tag));
    for (i = 0; i < sf->size; ++i) {
      const struct StructField *vf = &sf->fields[i];
      const char *name = vf->name;
      const char *type = vf->type;
      const char *ref = vf->ref;
      const char *disc_val = NULL;
      const char *ref_type = NULL;
      char esc_dval[128];
      size_t a_idx;
      if (vf->serde_rename[0] != '\0')
        disc_val = vf->serde_rename;
      else if (sf->union_variants && i < sf->n_union_variants &&
               sf->union_variants[i].disc_value)
        disc_val = sf->union_variants[i].disc_value;
      else if (sf->serde_tag[0] != '\0')
        disc_val = name;
      if (!disc_val)
        continue;
      if (strcmp(type, "object") != 0 && sf->serde_content[0] == '\0')
        continue;
      rc_esc = cdd_escape_json_c_str(disc_val, esc_dval, sizeof(esc_dval));
      CHECK_IO(
          FPRINTF_HOOK(fp, "      if (strcmp(disc, \"%s\") == 0", esc_dval));
      for (a_idx = 0; a_idx < vf->n_serde_aliases; ++a_idx) {
        if (vf->serde_aliases[a_idx]) {
          char esc_al[128];
          rc_esc = cdd_escape_json_c_str(vf->serde_aliases[a_idx], esc_al,
                                         sizeof(esc_al));
          CHECK_IO(FPRINTF_HOOK(fp, " || strcmp(disc, \"%s\") == 0", esc_al));
        }
      }
      CHECK_IO(FPRINTF_HOOK(fp, ") {\n"));
      CHECK_IO(
          FPRINTF_HOOK(fp, "        ret->tag = %s_%s;\n", union_name, name));
      if (sf->serde_content[0] != '\0') {
        char esc_cnt[128];
        rc_esc =
            cdd_escape_json_c_str(sf->serde_content, esc_cnt, sizeof(esc_cnt));
        if (strcmp(type, "object") == 0) {
          cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
          if (rc_ref != CDD_C_SUCCESS)
            return rc_ref;
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "        rc = %s_from_jsonObject(json_object_get_object("
              "jsonObject, \"%s\"), &ret->data.%s);\n"
              "        if (rc != CDD_C_SUCCESS) { free(ret); return rc; }\n",
              ref_type, esc_cnt, name));
        } else if (strcmp(type, "integer") == 0) {
          CHECK_IO(
              FPRINTF_HOOK(fp,
                           "        ret->data.%s = (int)json_object_get_number("
                           "jsonObject, \"%s\");\n",
                           name, esc_cnt));
        } else if (strcmp(type, "number") == 0) {
          CHECK_IO(FPRINTF_HOOK(fp,
                                "        ret->data.%s = json_object_get_number("
                                "jsonObject, \"%s\");\n",
                                name, esc_cnt));
        } else if (strcmp(type, "boolean") == 0) {
          CHECK_IO(
              FPRINTF_HOOK(fp,
                           "        ret->data.%s = json_object_get_boolean("
                           "jsonObject, \"%s\");\n",
                           name, esc_cnt));
        } else if (strcmp(type, "string") == 0) {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "        { const char *_cs = json_object_get_string("
              "jsonObject, \"%s\");\n"
              "          if (_cs) { ret->data.%s = strdup(_cs); if "
              "(!ret->data.%s) { free(ret); return CDD_C_ERROR_MEMORY; } } }\n",
              esc_cnt, name, name));
        }
        CHECK_IO(FPRINTF_HOOK(fp, "        *out = ret;\n"
                                  "        return CDD_C_SUCCESS;\n"
                                  "      }\n"));
      } else {
        cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
        if (rc_ref != CDD_C_SUCCESS)
          return rc_ref;
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        rc = %s_from_jsonObject(jsonObject, "
            "&ret->data.%s);\n"
            "        if (rc != CDD_C_SUCCESS) { free(ret); return rc; }\n"
            "        *out = ret;\n"
            "        return CDD_C_SUCCESS;\n"
            "      }\n",
            ref_type, name));
      }
    }
    CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
  }

  if (sf->serde_untagged) {
    for (i = 0; i < sf->size; ++i) {
      const char *name = sf->fields[i].name;
      const char *type = sf->fields[i].type;
      const char *ref = sf->fields[i].ref;
      const char *ref_type = NULL;
      cdd_c_error_t rc_ref;
      if (strcmp(type, "object") != 0)
        continue;
      rc_ref = get_type_from_ref(ref, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "  if (%s_from_jsonObject(jsonObject, &ret->data.%s) == "
                       "CDD_C_SUCCESS) {\n"
                       "    ret->tag = %s_%s;\n"
                       "    *out = ret;\n"
                       "    return CDD_C_SUCCESS;\n"
                       "  }\n",
                       ref_type, name, union_name, name));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "  free(ret);\n"
                              "  return CDD_C_ERROR_INVALID_ARGUMENT;\n}\n"));
    if (config && config->json_guard)
      CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->json_guard));
    return CDD_C_SUCCESS;
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  {\n    int match_count = 0;\n"
                            "    int match_idx = -1;\n"));
  for (i = 0; i < sf->size; ++i) {
    const char *type = sf->fields[i].type;
    const struct UnionVariantMeta *meta =
        (sf->union_variants && i < sf->n_union_variants)
            ? &sf->union_variants[i]
            : NULL;
    size_t k;

    if (strcmp(type, "object") != 0)
      continue;

    if (meta && meta->n_required_props > 0) {
      CHECK_IO(FPRINTF_HOOK(fp, "    if ("));
      for (k = 0; k < meta->n_required_props; ++k) {
        const char *req = meta->required_props[k];
        if (!req)
          continue;
        if (k > 0)
          CHECK_IO(FPRINTF_HOOK(fp, " && "));
        CHECK_IO(
            FPRINTF_HOOK(fp, "json_object_has_value(jsonObject, \"%s\")", req));
      }
      CHECK_IO(FPRINTF_HOOK(fp,
                            ") { match_count++; if (match_idx < 0) match_idx = "
                            "%lu; }\n",
                            (unsigned long)i));
    } else if (meta && meta->n_property_names > 0) {
      CHECK_IO(FPRINTF_HOOK(fp, "    if ("));
      for (k = 0; k < meta->n_property_names; ++k) {
        const char *prop = meta->property_names[k];
        if (!prop)
          continue;
        if (k > 0)
          CHECK_IO(FPRINTF_HOOK(fp, " || "));
        CHECK_IO(FPRINTF_HOOK(fp, "json_object_has_value(jsonObject, \"%s\")",
                              prop));
      }
      CHECK_IO(FPRINTF_HOOK(fp,
                            ") { match_count++; if (match_idx < 0) match_idx = "
                            "%lu; }\n",
                            (unsigned long)i));
    } else {
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "    if (json_object_get_count(jsonObject) > 0) { "
                       "match_count++; if (match_idx < 0) match_idx = %lu; }\n",
                       (unsigned long)i));
    }
  }

  if (!sf->union_is_anyof) {
    CHECK_IO(FPRINTF_HOOK(fp, "    if (match_count > 1) { free(ret); return "
                              "CDD_C_ERROR_INVALID_ARGUMENT; }\n"));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "    if (match_idx < 0) { free(ret); return "
                            "CDD_C_ERROR_INVALID_ARGUMENT; }\n"
                            "    switch (match_idx) {\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;
    const char *ref_type = NULL;
    cdd_c_error_t rc_ref;
    if (strcmp(type, "object") != 0)
      continue;
    rc_ref = get_type_from_ref(ref, &ref_type);
    if (rc_ref != CDD_C_SUCCESS)
      return rc_ref;
    CHECK_IO(FPRINTF_HOOK(fp, "    case %lu:\n", (unsigned long)i));
    CHECK_IO(FPRINTF_HOOK(fp, "      ret->tag = %s_%s;\n", union_name, name));
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "      rc = %s_from_jsonObject(jsonObject, "
        "&ret->data.%s);\n"
        "      if (rc != CDD_C_SUCCESS) { free(ret); return rc; }\n"
        "      break;\n",
        ref_type, name));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "    default:\n"
                            "      free(ret);\n"
                            "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                            "    }\n"
                            "  }\n"
                            "  *out = ret;\n"
                            "  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->json_guard));

  return rc_esc;
}

/*
 * @brief Emits C source code for union from json func.
 *
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union type.
 * @param[in] sf Struct fields descriptor container for the union variants.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_union_from_json_func(FILE *fp, const char *union_name,
                           const struct StructFields *sf,
                           const struct CodegenTypesConfig *config) {
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
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < sf->size; ++i) {
    enum UnionVariantJsonType jtype = UNION_JSON_UNKNOWN;
    const char *t = sf->fields[i].type;
    if (sf->union_variants && i < sf->n_union_variants)
      jtype = sf->union_variants[i].json_type;
    if (jtype == UNION_JSON_UNKNOWN) {
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
      if ((strcmp(item, "integer") != 0 && strcmp(item, "number") != 0 &&
           strcmp(item, "string") != 0 && strcmp(item, "boolean") != 0))
        array_needs_rc = 1;
      break;
    }
    default:
      break;
    }
  }

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->json_guard));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_from_json(const char *json, struct %s **const "
      "out) {\n",
      union_name, union_name));

  if (has_object || array_needs_rc)
    CHECK_IO(FPRINTF_HOOK(fp, "  int rc;\n"));

  CHECK_IO(FPRINTF_HOOK(
      fp, "  JSON_Value *val;\n"
          "  JSON_Value_Type typ;\n"
          "  if (!json || !out) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  val = json_parse_string(json);\n"
          "  if (!val) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  typ = json_value_get_type(val);\n"
          "  switch (typ) {\n"));

  CHECK_IO(FPRINTF_HOOK(fp, "    case JSONObject:\n"));
  if (has_object) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "      rc = %s_from_jsonObject(json_value_get_object(val), "
        "out);\n"
        "      json_value_free(val);\n"
        "      return rc;\n",
        union_name));
  } else {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "    case JSONArray:\n"));
  if (array_count == 0) {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  } else if (!sf->union_is_anyof && array_count > 1) {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  } else {
    const char *name = sf->fields[array_idx].name;
    const char *ref = sf->fields[array_idx].ref;
    CHECK_IO(
        FPRINTF_HOOK(fp,
                     "      {\n"
                     "        JSON_Array *arr = json_value_get_array(val);\n"
                     "        size_t i, count;\n"
                     "        struct %s *ret = malloc(sizeof(struct %s));\n"
                     "        if (!ret) { json_value_free(val); return "
                     "CDD_C_ERROR_MEMORY; }\n"
                     "        memset(ret, 0, sizeof(*ret));\n"
                     "        ret->tag = %s_%s;\n"
                     "        count = json_array_get_count(arr);\n"
                     "        ret->data.%s.n_%s = count;\n"
                     "        if (count > 0) {\n",
                     union_name, union_name, union_name, name, name, name));
    if (strcmp(ref, "integer") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "          ret->data.%s.%s = malloc(count * sizeof(int));\n"
          "          if (!ret->data.%s.%s) { free(ret); "
          "json_value_free(val); return CDD_C_ERROR_MEMORY; }\n"
          "          for (i = 0; i < count; ++i) ret->data.%s.%s[i] = "
          "(int)json_array_get_number(arr, i);\n",
          name, name, name, name, name, name));
    } else if (strcmp(ref, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "          ret->data.%s.%s = malloc(count * sizeof(double));\n"
          "          if (!ret->data.%s.%s) { free(ret); json_value_free(val); "
          "return CDD_C_ERROR_MEMORY; }\n"
          "          for (i = 0; i < count; ++i) ret->data.%s.%s[i] = "
          "json_array_get_number(arr, i);\n",
          name, name, name, name, name, name));
    } else if (strcmp(ref, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "          ret->data.%s.%s = malloc(count * sizeof(int));\n"
          "          if (!ret->data.%s.%s) { free(ret); "
          "json_value_free(val); return CDD_C_ERROR_MEMORY; }\n"
          "          for (i = 0; i < count; ++i) ret->data.%s.%s[i] = "
          "json_array_get_boolean(arr, i) ? 1 : 0;\n",
          name, name, name, name, name, name));
    } else if (strcmp(ref, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "          ret->data.%s.%s = calloc(count, sizeof(char *));\n"
          "          if (!ret->data.%s.%s) { free(ret); "
          "json_value_free(val); return CDD_C_ERROR_MEMORY; }\n"
          "          for (i = 0; i < count; ++i) {\n"
          "            const char *s = json_array_get_string(arr, i);\n"
          "            if (s) ret->data.%s.%s[i] = strdup(s);\n"
          "            if (!ret->data.%s.%s[i]) {\n",
          name, name, name, name, name, name, name, name));
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "              size_t j;\n"
          "              for (j = 0; j < i; ++j) free(ret->data.%s.%s[j]);\n"
          "              free(ret->data.%s.%s);\n"
          "              free(ret);\n"
          "              json_value_free(val);\n"
          "              return CDD_C_ERROR_MEMORY;\n"
          "            }\n"
          "          }\n",
          name, name, name, name));
    } else {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "          ret->data.%s.%s = calloc(count, sizeof(struct %s*));\n"
          "          if (!ret->data.%s.%s) { free(ret); json_value_free(val); "
          "return CDD_C_ERROR_MEMORY; }\n"
          "          for (i = 0; i < count; ++i) {\n"
          "            rc = %s_from_jsonObject(json_array_get_object(arr, i), "
          "&ret->data.%s.%s[i]);\n"
          "            if (rc != CDD_C_SUCCESS) {\n",
          name, name, ref, name, name, ref, name, name));
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "              size_t j;\n"
                       "              for (j = 0; j < i; ++j) { "
                       "if (ret->data.%s.%s[j]) %s_cleanup(ret->data.%s.%s[j]);"
                       " }\n"
                       "              free(ret->data.%s.%s);\n"
                       "              free(ret);\n"
                       "              json_value_free(val);\n"
                       "              return rc;\n"
                       "            }\n"
                       "          }\n",
                       name, name, ref, name, name, name, name));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "        }\n"
                              "        *out = ret;\n"
                              "        json_value_free(val);\n"
                              "        return CDD_C_SUCCESS;\n"
                              "      }\n"));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "    case JSONString:\n"));
  if (string_count == 0) {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  } else {
    const char *name = sf->fields[string_idx].name;
    if (!sf->union_is_anyof && string_count > 1) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "      json_value_free(val);\n"
                            "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
    } else {
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "      {\n"
                       "        const char *s = json_value_get_string(val);\n"
                       "        struct %s *ret;\n"
                       "        if (!s) { json_value_free(val); return "
                       "CDD_C_ERROR_INVALID_ARGUMENT; }\n"
                       "        ret = malloc(sizeof(struct %s));\n"
                       "        if (!ret) { json_value_free(val); return "
                       "CDD_C_ERROR_MEMORY; }\n",
                       union_name, union_name));
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "        memset(ret, 0, sizeof(*ret));\n"
                       "        ret->tag = %s_%s;\n"
                       "        ret->data.%s = strdup(s);\n"
                       "        if (!ret->data.%s) { free(ret); "
                       "json_value_free(val); return CDD_C_ERROR_MEMORY; }\n"
                       "        *out = ret;\n"
                       "        json_value_free(val);\n"
                       "        return CDD_C_SUCCESS;\n"
                       "      }\n",
                       union_name, name, name, name));
    }
  }

  CHECK_IO(FPRINTF_HOOK(fp, "    case JSONNumber:\n"));
  if (int_count == 0 && num_count == 0) {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  } else if (!sf->union_is_anyof && (int_count > 1 || num_count > 1)) {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  } else {
    CHECK_IO(
        FPRINTF_HOOK(fp, "      {\n"
                         "        double num = json_value_get_number(val);\n"));
    if (int_count > 0 && num_count == 0) {
      const char *name = sf->fields[int_idx].name;
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "        if (num != (int)num) { json_value_free(val); "
                       "return CDD_C_ERROR_INVALID_ARGUMENT; "
                       "}\n"
                       "        { struct %s *ret = malloc(sizeof(struct %s));\n"
                       "          if (!ret) { json_value_free(val); return "
                       "CDD_C_ERROR_MEMORY; }\n"
                       "          memset(ret, 0, sizeof(*ret));\n"
                       "          ret->tag = %s_%s;\n"
                       "          ret->data.%s = (int)num;\n"
                       "          *out = ret;\n"
                       "          json_value_free(val);\n"
                       "          return CDD_C_SUCCESS; }\n"
                       "      }\n",
                       union_name, union_name, union_name, name, name));
    } else if (int_count == 0) {
      const char *name = sf->fields[num_idx].name;
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "        { struct %s *ret = malloc(sizeof(struct %s));\n"
                       "          if (!ret) { json_value_free(val); return "
                       "CDD_C_ERROR_MEMORY; }\n"
                       "          memset(ret, 0, sizeof(*ret));\n"
                       "          ret->tag = %s_%s;\n"
                       "          ret->data.%s = num;\n"
                       "          *out = ret;\n"
                       "          json_value_free(val);\n"
                       "          return CDD_C_SUCCESS; }\n"
                       "      }\n",
                       union_name, union_name, union_name, name, name));
    } else {
      const char *int_name = sf->fields[int_idx].name;
      const char *num_name = sf->fields[num_idx].name;
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "        if (num == (int)num) {\n"
                       "          struct %s *ret = malloc(sizeof(struct %s));\n"
                       "          if (!ret) { json_value_free(val); return "
                       "CDD_C_ERROR_MEMORY; }\n"
                       "          memset(ret, 0, sizeof(*ret));\n"
                       "          ret->tag = %s_%s;\n"
                       "          ret->data.%s = (int)num;\n"
                       "          *out = ret;\n"
                       "          json_value_free(val);\n"
                       "          return CDD_C_SUCCESS;\n"
                       "        } else {\n",
                       union_name, union_name, union_name, int_name, int_name));
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "          struct %s *ret = malloc(sizeof(struct %s));\n"
                       "          if (!ret) { json_value_free(val); return "
                       "CDD_C_ERROR_MEMORY; }\n"
                       "          memset(ret, 0, sizeof(*ret));\n"
                       "          ret->tag = %s_%s;\n"
                       "          ret->data.%s = num;\n"
                       "          *out = ret;\n"
                       "          json_value_free(val);\n"
                       "          return CDD_C_SUCCESS;\n"
                       "        }\n"
                       "      }\n",
                       union_name, union_name, union_name, num_name, num_name));
    }
  }

  CHECK_IO(FPRINTF_HOOK(fp, "    case JSONBoolean:\n"));
  if (bool_count == 0) {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  } else if (!sf->union_is_anyof && bool_count > 1) {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  } else {
    const char *name = sf->fields[bool_idx].name;
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "      {\n"
        "        struct %s *ret = malloc(sizeof(struct %s));\n"
        "        if (!ret) { json_value_free(val); return CDD_C_ERROR_MEMORY; "
        "}\n"
        "        memset(ret, 0, sizeof(*ret));\n"
        "        ret->tag = %s_%s;\n"
        "        ret->data.%s = json_value_get_boolean(val) ? 1 : 0;\n"
        "        *out = ret;\n"
        "        json_value_free(val);\n"
        "        return CDD_C_SUCCESS;\n"
        "      }\n",
        union_name, union_name, union_name, name, name));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "    case JSONNull:\n"));
  if (null_count == 0) {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  } else if (!sf->union_is_anyof && null_count > 1) {
    CHECK_IO(FPRINTF_HOOK(fp, "      json_value_free(val);\n"
                              "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  } else {
    const char *name = sf->fields[null_idx].name;
    CHECK_IO(
        FPRINTF_HOOK(fp,
                     "      {\n"
                     "        struct %s *ret = malloc(sizeof(struct %s));\n"
                     "        if (!ret) { json_value_free(val); return "
                     "CDD_C_ERROR_MEMORY; }\n"
                     "        memset(ret, 0, sizeof(*ret));\n"
                     "        ret->tag = %s_%s;\n"
                     "        ret->data.%s = 0;\n"
                     "        *out = ret;\n"
                     "        json_value_free(val);\n"
                     "        return CDD_C_SUCCESS;\n"
                     "      }\n",
                     union_name, union_name, union_name, name, name));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "    default:\n"
                            "      json_value_free(val);\n"
                            "      return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                            "  }\n"
                            "}\n"));

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->json_guard));

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits C source code for union cleanup func.
 *
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union type.
 * @param[in] sf Struct fields descriptor container for the union variants.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_union_cleanup_func(FILE *fp, const char *union_name,
                         const struct StructFields *sf,
                         const struct CodegenTypesConfig *config) {
  size_t i;
  int iter_needed = 0;
  if (!fp || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0) {
      iter_needed = 1;
      break;
    }
  }

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->utils_guard));

  CHECK_IO(FPRINTF_HOOK(fp, "cdd_c_error_t %s_cleanup(struct %s *obj) {\n",
                        union_name, union_name));
  CHECK_IO(FPRINTF_HOOK(fp, "  if (!obj) return CDD_C_SUCCESS;\n"));
  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  size_t i;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  switch (obj->tag) {\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *ref_type = NULL;
    CHECK_IO(FPRINTF_HOOK(fp, "    case %s_%s:\n", union_name, name));
    if (strcmp(sf->fields[i].type, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp, "      free((void*)obj->data.%s);\n", name));
    } else if (strcmp(sf->fields[i].type, "object") == 0) {
      cdd_c_error_t rc_ref = get_type_from_ref(sf->fields[i].ref, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (obj->data.%s) { cdd_c_error_t _rc = "
          "%s_cleanup(obj->data.%s); if (_rc != CDD_C_SUCCESS) return _rc; }\n",
          name, ref_type, name));
    } else if (strcmp(sf->fields[i].type, "array") == 0) {
      const char *r = sf->fields[i].ref;
      if (strcmp(r, "string") == 0 ||
          (strcmp(r, "integer") != 0 && strcmp(r, "boolean") != 0 &&
           strcmp(r, "number") != 0 && strcmp(r, "enum") != 0)) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "      for (i = 0; i < obj->data.%s.n_%s; ++i) {\n", name,
            name));
        if (strcmp(r, "string") == 0) {
          CHECK_IO(FPRINTF_HOOK(fp, "        free(obj->data.%s.%s[i]);\n", name,
                                name));
        } else {
          cdd_c_error_t rc_ref = get_type_from_ref(r, &ref_type);
          if (rc_ref != CDD_C_SUCCESS)
            return rc_ref;
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "        if (obj->data.%s.%s[i]) { cdd_c_error_t _rc = "
              "%s_cleanup(obj->data.%s.%s[i]); if (_rc != CDD_C_SUCCESS) "
              "return _rc; }\n",
              name, name, ref_type, name, name));
        }
        CHECK_IO(FPRINTF_HOOK(fp, "      }\n"));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "      free(obj->data.%s.%s);\n", name, name));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "      break;\n"));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "    default: break;\n  }\n  free(obj);\n  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->utils_guard));

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits C source code for union default initialization func.
 *
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union type.
 * @param[in] sf Struct fields descriptor container for the union variants.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_union_default_func(FILE *fp, const char *union_name,
                         const struct StructFields *sf,
                         const struct CodegenTypesConfig *config) {
  if (!fp || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->utils_guard));

  CHECK_IO(FPRINTF_HOOK(fp,
                        "cdd_c_error_t %s_default(struct %s **out) {\n"
                        "  if (!out) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                        "  *out = calloc(1, sizeof(struct %s));\n"
                        "  if (!*out) return CDD_C_ERROR_MEMORY;\n"
                        "  (*out)->tag = %s_UNKNOWN;\n"
                        "  return CDD_C_SUCCESS;\n"
                        "}\n",
                        union_name, union_name, union_name, union_name));

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->utils_guard));

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits C source code for union deepcopy func.
 *
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union type.
 * @param[in] sf Struct fields descriptor container for the union variants.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_union_deepcopy_func(FILE *fp, const char *union_name,
                          const struct StructFields *sf,
                          const struct CodegenTypesConfig *config) {
  size_t i;
  if (!fp || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->utils_guard));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_deepcopy(const struct %s *src, struct %s **dest) {\n"
      "  if (!dest) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "  if (!src) { *dest = NULL; return CDD_C_SUCCESS; }\n"
      "  *dest = calloc(1, sizeof(struct %s));\n"
      "  if (!*dest) return CDD_C_ERROR_MEMORY;\n"
      "  (*dest)->tag = src->tag;\n"
      "  switch (src->tag) {\n",
      union_name, union_name, union_name, union_name));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;
    const char *ref_type = NULL;

    CHECK_IO(FPRINTF_HOOK(fp, "    case %s_%s:\n", union_name, name));
    if (strcmp(type, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (src->data.%s) {\n"
          "        (*dest)->data.%s = strdup(src->data.%s);\n"
          "        if (!(*dest)->data.%s) { free(*dest); *dest = NULL; return "
          "CDD_C_ERROR_MEMORY; }\n"
          "      }\n",
          name, name, name, name));
    } else if (strcmp(type, "object") == 0) {
      cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (src->data.%s) {\n"
          "        cdd_c_error_t _rc = %s_deepcopy(src->data.%s, "
          "&(*dest)->data.%s);\n"
          "        if (_rc != CDD_C_SUCCESS) { free(*dest); *dest = NULL; "
          "return _rc; }\n"
          "      }\n",
          name, ref_type, name, name));
    } else if (strcmp(type, "array") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "      if (src->data.%s.%s && src->data.%s.n_%s > 0) {\n", name,
          name, name, name));
      if (strcmp(ref, "integer") == 0 || strcmp(ref, "boolean") == 0 ||
          strcmp(ref, "enum") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        (*dest)->data.%s.%s = malloc(src->data.%s.n_%s * "
            "sizeof(int));\n"
            "        if (!(*dest)->data.%s.%s) { free(*dest); *dest = NULL; "
            "return CDD_C_ERROR_MEMORY; }\n"
            "        memcpy((*dest)->data.%s.%s, src->data.%s.%s, "
            "src->data.%s.n_%s * sizeof(int));\n"
            "        (*dest)->data.%s.n_%s = src->data.%s.n_%s;\n",
            name, name, name, name, name, name, name, name, name, name, name,
            name, name, name, name, name));
      } else if (strcmp(ref, "number") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        (*dest)->data.%s.%s = malloc(src->data.%s.n_%s * "
            "sizeof(double));\n"
            "        if (!(*dest)->data.%s.%s) { free(*dest); *dest = NULL; "
            "return CDD_C_ERROR_MEMORY; }\n"
            "        memcpy((*dest)->data.%s.%s, src->data.%s.%s, "
            "src->data.%s.n_%s * sizeof(double));\n"
            "        (*dest)->data.%s.n_%s = src->data.%s.n_%s;\n",
            name, name, name, name, name, name, name, name, name, name, name,
            name, name, name, name, name));
      } else if (strcmp(ref, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        size_t _i;\n"
            "        (*dest)->data.%s.%s = calloc(src->data.%s.n_%s, "
            "sizeof(char *));\n"
            "        if (!(*dest)->data.%s.%s) { free(*dest); *dest = NULL; "
            "return CDD_C_ERROR_MEMORY; }\n"
            "        (*dest)->data.%s.n_%s = src->data.%s.n_%s;\n",
            name, name, name, name, name, name, name, name, name, name));
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        for (_i = 0; _i < src->data.%s.n_%s; ++_i) {\n"
            "          if (src->data.%s.%s[_i]) {\n"
            "            (*dest)->data.%s.%s[_i] = "
            "strdup(src->data.%s.%s[_i]);\n"
            "            if (!(*dest)->data.%s.%s[_i]) { %s_cleanup(*dest); "
            "*dest = NULL; return CDD_C_ERROR_MEMORY; }\n"
            "          }\n"
            "        }\n",
            name, name, name, name, name, name, name, name, name, name,
            union_name));
      } else {
        cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
        if (rc_ref != CDD_C_SUCCESS)
          return rc_ref;
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        size_t _i;\n"
            "        (*dest)->data.%s.%s = calloc(src->data.%s.n_%s, "
            "sizeof(struct %s *));\n"
            "        if (!(*dest)->data.%s.%s) { free(*dest); *dest = NULL; "
            "return CDD_C_ERROR_MEMORY; }\n"
            "        (*dest)->data.%s.n_%s = src->data.%s.n_%s;\n",
            name, name, name, name, ref_type, name, name, name, name, name,
            name));
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        for (_i = 0; _i < src->data.%s.n_%s; ++_i) {\n"
            "          if (src->data.%s.%s[_i]) {\n"
            "            cdd_c_error_t _rc = "
            "%s_deepcopy(src->data.%s.%s[_i], &(*dest)->data.%s.%s[_i]);\n"
            "            if (_rc != CDD_C_SUCCESS) { %s_cleanup(*dest); "
            "*dest = NULL; return _rc; }\n"
            "          }\n"
            "        }\n",
            name, name, name, name, ref_type, name, name, name, name,
            union_name));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "      }\n"));
    } else if (strcmp(type, "null") != 0) {
      CHECK_IO(FPRINTF_HOOK(fp, "      (*dest)->data.%s = src->data.%s;\n",
                            name, name));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "      break;\n"));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "    default: break;\n  }\n  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->utils_guard));

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits C source code for union equality comparison func.
 *
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union type.
 * @param[in] sf Struct fields descriptor container for the union variants.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t write_union_eq_func(FILE *fp, const char *union_name,
                                  const struct StructFields *sf,
                                  const struct CodegenTypesConfig *config) {
  size_t i;
  if (!fp || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->utils_guard));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_eq(const struct %s *a, const struct %s *b, int "
      "*out_eq) {\n"
      "  if (!out_eq) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "  if (a == b) { *out_eq = 1; return CDD_C_SUCCESS; }\n"
      "  if (!a || !b || a->tag != b->tag) { *out_eq = 0; return "
      "CDD_C_SUCCESS; }\n"
      "  switch (a->tag) {\n",
      union_name, union_name, union_name));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;
    const char *ref_type = NULL;

    CHECK_IO(FPRINTF_HOOK(fp, "    case %s_%s:\n", union_name, name));
    if (strcmp(type, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (a->data.%s != b->data.%s && (!a->data.%s || !b->data.%s "
          "|| strcmp(a->data.%s, b->data.%s) != 0)) { *out_eq = 0; return "
          "CDD_C_SUCCESS; }\n",
          name, name, name, name, name, name));
    } else if (strcmp(type, "object") == 0) {
      cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      { int _t = 0; cdd_c_error_t _rc = %s_eq(a->data.%s, "
          "b->data.%s, &_t); if (_rc != CDD_C_SUCCESS) return _rc; if (!_t) { "
          "*out_eq = 0; return CDD_C_SUCCESS; } }\n",
          ref_type, name, name));
    } else if (strcmp(type, "array") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      {\n"
          "        size_t _i;\n"
          "        if (a->data.%s.n_%s != b->data.%s.n_%s) { *out_eq = 0; "
          "return CDD_C_SUCCESS; }\n"
          "        for (_i = 0; _i < a->data.%s.n_%s; ++_i) {\n",
          name, name, name, name, name, name));
      if (strcmp(ref, "integer") == 0 || strcmp(ref, "boolean") == 0 ||
          strcmp(ref, "number") == 0 || strcmp(ref, "enum") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "          if (a->data.%s.%s[_i] != b->data.%s.%s[_i]) { *out_eq = "
            "0; return CDD_C_SUCCESS; }\n",
            name, name, name, name));
      } else if (strcmp(ref, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "          if (a->data.%s.%s[_i] != b->data.%s.%s[_i] && "
            "(!a->data.%s.%s[_i] || !b->data.%s.%s[_i] || "
            "strcmp(a->data.%s.%s[_i], b->data.%s.%s[_i]) != 0)) { *out_eq = "
            "0; return CDD_C_SUCCESS; }\n",
            name, name, name, name, name, name, name, name, name, name, name,
            name));
      } else {
        cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
        if (rc_ref != CDD_C_SUCCESS)
          return rc_ref;
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "          { int _t = 0; cdd_c_error_t _rc = "
            "%s_eq(a->data.%s.%s[_i], b->data.%s.%s[_i], &_t); if (_rc != "
            "CDD_C_SUCCESS) return _rc; if (!_t) { *out_eq = 0; return "
            "CDD_C_SUCCESS; } }\n",
            ref_type, name, name, name, name));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "        }\n      }\n"));
    } else if (strcmp(type, "null") != 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (a->data.%s != b->data.%s) { *out_eq = 0; return "
          "CDD_C_SUCCESS; }\n",
          name, name));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "      break;\n"));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "    default: break;\n  }\n  *out_eq = 1;\n  "
                            "return CDD_C_SUCCESS;\n}\n"));

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->utils_guard));

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits C source code for union debug print func.
 *
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union type.
 * @param[in] sf Struct fields descriptor container for the union variants.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t write_union_debug_func(FILE *fp, const char *union_name,
                                     const struct StructFields *sf,
                                     const struct CodegenTypesConfig *config) {
  size_t i;
  if (!fp || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->utils_guard));

  CHECK_IO(
      FPRINTF_HOOK(fp,
                   "cdd_c_error_t %s_debug(const struct %s *obj, FILE *fp) {\n"
                   "  if (!fp) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                   "  if (!obj) { if (fprintf(fp, \"(null)\\n\") < 0) return "
                   "CDD_C_ERROR_IO; return CDD_C_SUCCESS; }\n"
                   "  switch (obj->tag) {\n",
                   union_name, union_name));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;
    const char *ref_type = NULL;

    CHECK_IO(FPRINTF_HOOK(fp, "    case %s_%s:\n", union_name, name));
    if (strcmp(type, "integer") == 0 || strcmp(type, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (fprintf(fp, \"%s::%s(%%d)\\n\", obj->data.%s) < 0) return "
          "CDD_C_ERROR_IO;\n",
          union_name, name, name));
    } else if (strcmp(type, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (fprintf(fp, \"%s::%s(%%g)\\n\", obj->data.%s) < 0) return "
          "CDD_C_ERROR_IO;\n",
          union_name, name, name));
    } else if (strcmp(type, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (fprintf(fp, \"%s::%s(\\\"%%s\\\")\\n\", obj->data.%s ? "
          "obj->data.%s : \"(null)\") < 0) return CDD_C_ERROR_IO;\n",
          union_name, name, name, name));
    } else if (strcmp(type, "object") == 0) {
      cdd_c_error_t rc_ref = get_type_from_ref(ref, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (fprintf(fp, \"%s::%s:\\n\") < 0) return CDD_C_ERROR_IO;\n"
          "      { cdd_c_error_t _rc = %s_debug(obj->data.%s, fp); if (_rc != "
          "CDD_C_SUCCESS) return _rc; }\n",
          union_name, name, ref_type, name));
    } else {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      if (fprintf(fp, \"%s::%s\\n\") < 0) return CDD_C_ERROR_IO;\n",
          union_name, name));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "      break;\n"));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "    default:\n"
      "      if (fprintf(fp, \"%s::UNKNOWN\\n\") < 0) return CDD_C_ERROR_IO;\n"
      "      break;\n"
      "  }\n"
      "  return CDD_C_SUCCESS;\n"
      "}\n",
      union_name));

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->utils_guard));

  return CDD_C_SUCCESS;
}

/* --- Root Array Implementation --- */
