/**
 * @file enum.c
 * @brief Implementation of Enum extraction and generation logic.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/emit/enum.h"
#include "c_cdd/code_writer.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "classes/parse/cdd_serde.h"
#include "functions/parse/str.h"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/* Wrapper for fprintf to check errors tersely */
/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

/* Select correct strdup function name for generated code */
#if defined(_MSC_VER)
/** @brief Platform-specific strdup function name for emitted code */
static const char *kStrDupFunc = "_strdup";
#else
/** @brief Platform-specific strdup function name for emitted code */
static const char *kStrDupFunc = "strdup";
#endif

/*
 * @brief Initialize an EnumMembers container.
 *
 * @param[out] em Pointer to the EnumMembers structure to initialize.
 * @return CDD_C_SUCCESS on success, error code on allocation failure.
 */
cdd_c_error_t enum_members_init(struct EnumMembers *em) {
  if (!em)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(em, 0, sizeof(*em));
  em->size = 0;
  em->capacity = 8;
  em->serde_other_idx = -1;
  em->members = (char **)C_CDD_CALLOC(em->capacity, sizeof(char *));
  if (!em->members) {
    em->capacity = 0;
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  em->serde_names = (char **)C_CDD_CALLOC(em->capacity, sizeof(char *));
  if (!em->serde_names) {
    C_CDD_FREE(em->members);
    em->members = NULL;
    em->capacity = 0;
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  em->serde_skip = (int *)C_CDD_CALLOC(em->capacity, sizeof(int));
  if (!em->serde_skip) {
    C_CDD_FREE(em->serde_names);
    em->serde_names = NULL;
    C_CDD_FREE(em->members);
    em->members = NULL;
    em->capacity = 0;
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Free the memory associated with an EnumMembers structure.
 *
 * @param[in,out] em Pointer to the EnumMembers structure to free.
 */
void enum_members_free(struct EnumMembers *em) {
  size_t i;
  if (!em)
    return;
  if (em->members) {
    for (i = 0; i < em->size; ++i) {
      if (em->members[i])
        C_CDD_FREE(em->members[i]);
    }
    C_CDD_FREE(em->members);
    em->members = NULL;
  }
  if (em->serde_names) {
    for (i = 0; i < em->size; ++i) {
      if (em->serde_names[i])
        C_CDD_FREE(em->serde_names[i]);
    }
    C_CDD_FREE(em->serde_names);
    em->serde_names = NULL;
  }
  if (em->serde_skip) {
    C_CDD_FREE(em->serde_skip);
    em->serde_skip = NULL;
  }
  if (em->serde_aliases) {
    for (i = 0; i < em->size; ++i) {
      if (em->serde_aliases[i]) {
        size_t a;
        size_t n_al = em->n_serde_aliases ? em->n_serde_aliases[i] : 0;
        for (a = 0; a < n_al; ++a) {
          if (em->serde_aliases[i][a])
            C_CDD_FREE(em->serde_aliases[i][a]);
        }
        C_CDD_FREE(em->serde_aliases[i]);
      }
    }
    C_CDD_FREE(em->serde_aliases);
    em->serde_aliases = NULL;
  }
  if (em->n_serde_aliases) {
    C_CDD_FREE(em->n_serde_aliases);
    em->n_serde_aliases = NULL;
  }
  if (em->cli_descriptions) {
    for (i = 0; i < em->size; ++i) {
      if (em->cli_descriptions[i])
        C_CDD_FREE(em->cli_descriptions[i]);
    }
    C_CDD_FREE(em->cli_descriptions);
    em->cli_descriptions = NULL;
  }
  if (em->cli_hidden) {
    C_CDD_FREE(em->cli_hidden);
    em->cli_hidden = NULL;
  }
  em->size = 0;
  em->capacity = 0;
  em->serde_other_idx = -1;
}

/*
 * @brief Add a member to the EnumMembers container with optional serde
 * metadata.
 *
 * @param[in,out] em Target EnumMembers container.
 * @param[in] name Enumerator identifier string to append.
 * @param[in] serde_name Optional explicit wire name override (can be NULL).
 * @param[in] serde_skip Non-zero if this variant should be skipped in JSON/str
 * codecs.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t enum_members_add_ex(struct EnumMembers *em, const char *name,
                                  const char *serde_name, int serde_skip) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!em || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (em->size >= em->capacity) {
    const size_t new_cap = em->capacity == 0 ? 8 : em->capacity * 2;
    char **new_members;
    char **new_serde_names;
    int *new_serde_skip;
    size_t k;
    new_members = (char **)C_CDD_REALLOC(em->members, new_cap * sizeof(char *));
    if (!new_members) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    em->members = new_members;
    new_serde_names =
        (char **)C_CDD_REALLOC(em->serde_names, new_cap * sizeof(char *));
    if (!new_serde_names) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    em->serde_names = new_serde_names;
    new_serde_skip =
        (int *)C_CDD_REALLOC(em->serde_skip, new_cap * sizeof(int));
    if (!new_serde_skip) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    em->serde_skip = new_serde_skip;
    if (em->serde_aliases) {
      char ***new_aliases =
          (char ***)C_CDD_REALLOC(em->serde_aliases, new_cap * sizeof(char **));
      if (!new_aliases)
        return CDD_C_ERROR_MEMORY;
      em->serde_aliases = new_aliases;
    }
    if (em->n_serde_aliases) {
      size_t *new_n_aliases = (size_t *)C_CDD_REALLOC(em->n_serde_aliases,
                                                      new_cap * sizeof(size_t));
      if (!new_n_aliases)
        return CDD_C_ERROR_MEMORY;
      em->n_serde_aliases = new_n_aliases;
    }
    if (em->cli_descriptions) {
      char **new_desc = (char **)C_CDD_REALLOC(em->cli_descriptions,
                                               new_cap * sizeof(char *));
      if (!new_desc)
        return CDD_C_ERROR_MEMORY;
      em->cli_descriptions = new_desc;
    }
    if (em->cli_hidden) {
      int *new_hid =
          (int *)C_CDD_REALLOC(em->cli_hidden, new_cap * sizeof(int));
      if (!new_hid)
        return CDD_C_ERROR_MEMORY;
      em->cli_hidden = new_hid;
    }
    for (k = em->capacity; k < new_cap; ++k) {
      em->members[k] = NULL;
      em->serde_names[k] = NULL;
      em->serde_skip[k] = 0;
      if (em->serde_aliases)
        em->serde_aliases[k] = NULL;
      if (em->n_serde_aliases)
        em->n_serde_aliases[k] = 0;
      if (em->cli_descriptions)
        em->cli_descriptions[k] = NULL;
      if (em->cli_hidden)
        em->cli_hidden[k] = 0;
    }
    em->capacity = new_cap;
  } else {
    if (!em->serde_names) {
      em->serde_names = (char **)C_CDD_CALLOC(em->capacity, sizeof(char *));
      if (!em->serde_names)
        return CDD_C_ERROR_MEMORY;
    }
    if (!em->serde_skip) {
      em->serde_skip = (int *)C_CDD_CALLOC(em->capacity, sizeof(int));
      if (!em->serde_skip)
        return CDD_C_ERROR_MEMORY;
    }
  }
  rc = c_cdd_strdup(name, &em->members[em->size]);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (serde_name && serde_name[0]) {
    rc = c_cdd_strdup(serde_name, &em->serde_names[em->size]);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(em->members[em->size]);
      em->members[em->size] = NULL;
      return rc;
    }
  } else {
    em->serde_names[em->size] = NULL;
  }
  em->serde_skip[em->size] = serde_skip ? 1 : 0;
  em->size++;
  return CDD_C_SUCCESS;
}

/*
 * @brief Sets CLI metadata (help description and hidden flag) for an enum
 * variant at `member_idx`.
 *
 * @param[in,out] em Pointer to the EnumMembers container.
 * @param[in] member_idx Index of the enum variant.
 * @param[in] description Optional CLI help text for this variant (can be NULL).
 * @param[in] hidden Non-zero to hide this variant from `--help` output.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t enum_member_set_cli_meta(struct EnumMembers *em,
                                       size_t member_idx,
                                       const char *description, int hidden) {
  size_t cap;
  if (!em || member_idx >= em->size)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  cap = em->capacity > em->size ? em->capacity : em->size + 1;
  if (description && description[0] != '\0') {
    char *dup = NULL;
    cdd_c_error_t rc;
    if (!em->cli_descriptions) {
      em->cli_descriptions = (char **)C_CDD_CALLOC(cap, sizeof(char *));
      if (!em->cli_descriptions)
        return CDD_C_ERROR_MEMORY;
    }
    rc = c_cdd_strdup(description, &dup);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (em->cli_descriptions[member_idx])
      C_CDD_FREE(em->cli_descriptions[member_idx]);
    em->cli_descriptions[member_idx] = dup;
  }
  if (hidden) {
    if (!em->cli_hidden) {
      em->cli_hidden = (int *)C_CDD_CALLOC(cap, sizeof(int));
      if (!em->cli_hidden)
        return CDD_C_ERROR_MEMORY;
    }
    em->cli_hidden[member_idx] = 1;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Appends a deserialization alias to an enum variant at `member_idx`.
 *
 * @param[in,out] em Pointer to the EnumMembers container.
 * @param[in] member_idx Index of the enum variant.
 * @param[in] alias Alternative wire string to accept in `_from_str`.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t enum_member_add_alias(struct EnumMembers *em, size_t member_idx,
                                    const char *alias) {
  char **new_arr;
  char *dup_str = NULL;
  size_t i;
  size_t cur_n;
  cdd_c_error_t rc;
  if (!em || !alias || !alias[0] || member_idx >= em->size)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!em->serde_aliases) {
    size_t cap = em->capacity > em->size ? em->capacity : em->size + 1;
    em->serde_aliases = (char ***)C_CDD_CALLOC(cap, sizeof(char **));
    if (!em->serde_aliases)
      return CDD_C_ERROR_MEMORY;
  }
  if (!em->n_serde_aliases) {
    size_t cap = em->capacity > em->size ? em->capacity : em->size + 1;
    em->n_serde_aliases = (size_t *)C_CDD_CALLOC(cap, sizeof(size_t));
    if (!em->n_serde_aliases)
      return CDD_C_ERROR_MEMORY;
  }
  cur_n = em->n_serde_aliases[member_idx];
  for (i = 0; i < cur_n; ++i) {
    if (em->serde_aliases[member_idx][i] &&
        strcmp(em->serde_aliases[member_idx][i], alias) == 0)
      return CDD_C_SUCCESS;
  }
  rc = c_cdd_strdup(alias, &dup_str);
  if (rc != CDD_C_SUCCESS)
    return rc;
  new_arr = (char **)C_CDD_REALLOC(em->serde_aliases[member_idx],
                                   (cur_n + 1) * sizeof(char *));
  if (!new_arr) {
    C_CDD_FREE(dup_str);
    return CDD_C_ERROR_MEMORY;
  }
  em->serde_aliases[member_idx] = new_arr;
  em->serde_aliases[member_idx][cur_n] = dup_str;
  em->n_serde_aliases[member_idx] = cur_n + 1;
  return CDD_C_SUCCESS;
}

/*
 * @brief Add a member name to the EnumMembers list.
 *
 * @param[in,out] em Target EnumMembers container.
 * @param[in] name Enumerator identifier string to append.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t enum_members_add(struct EnumMembers *em, const char *name) {
  return enum_members_add_ex(em, name, NULL, 0);
}

/*
 * @brief Generate the C code for converting an enum value to a string.
 *
 * @param[in] fp Output file stream.
 * @param[in] enum_name Name of the enum type.
 * @param[in] em Enumerator members container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t write_enum_to_str_func(FILE *fp, const char *enum_name,
                                     const struct EnumMembers *em,
                                     const struct CodegenEnumConfig *config) {
  size_t i;
  if (!fp || !enum_name || !em || !em->members)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "int %s_to_str(enum %s val, char **str_out) {\n"
      "  if (str_out == NULL) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "  switch (val) {\n",
      enum_name, enum_name));

  for (i = 0; i < em->size; i++) {
    if (em->serde_skip && em->serde_skip[i])
      continue;
    if (em->members[i] && strcmp(em->members[i], "UNKNOWN") != 0) {
      char wname[128];
      char esc_wname[256];
      cdd_c_error_t rc_name =
          cdd_serde_resolve_enum_member_name(em, i, wname, sizeof(wname));
      if (rc_name != CDD_C_SUCCESS)
        return rc_name;
      cdd_escape_json_c_str(wname, esc_wname, sizeof(esc_wname));
      CHECK_IO(FPRINTF_HOOK(
          fp, "    case %s_%s:\n      *str_out = %s(\"%s\");\n      break;\n",
          enum_name, em->members[i], kStrDupFunc, esc_wname));
    }
  }
  CHECK_IO(FPRINTF_HOOK(fp,
                        "    case %s_UNKNOWN:\n    default:\n      *str_out = "
                        "%s(\"UNKNOWN\");\n      break;\n  }\n  if "
                        "(*str_out == NULL) return CDD_C_ERROR_MEMORY;\n  "
                        "return CDD_C_SUCCESS;\n}\n",
                        enum_name, kStrDupFunc));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n", config->guard_macro));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "\n"));

  return CDD_C_SUCCESS;
}

/*
 * @brief Generate the C code for parsing a string into an enum value.
 *
 * @param[in] fp Output file stream.
 * @param[in] enum_name Name of the enum type.
 * @param[in] em Enumerator members container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t write_enum_from_str_func(FILE *fp, const char *enum_name,
                                       const struct EnumMembers *em,
                                       const struct CodegenEnumConfig *config) {
  size_t i;
  if (!fp || !enum_name || !em || !em->members)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(
      FPRINTF_HOOK(fp,
                   "int %s_from_str(const char *str, enum %s *val) {\n"
                   "  if (val == NULL) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                   "  else if (str == NULL) *val = %s_UNKNOWN;\n",
                   enum_name, enum_name, enum_name));

  for (i = 0; i < em->size; i++) {
    if (em->serde_skip && em->serde_skip[i])
      continue;
    /* Skip explicit logic for "UNKNOWN" member, let the final else catch it */
    if (em->members[i] && strcmp(em->members[i], "UNKNOWN") != 0) {
      char wname[128];
      char esc_wname[256];
      size_t n_al = (em->n_serde_aliases && em->serde_aliases)
                        ? em->n_serde_aliases[i]
                        : 0;
      cdd_c_error_t rc_name =
          cdd_serde_resolve_enum_member_name(em, i, wname, sizeof(wname));
      if (rc_name != CDD_C_SUCCESS)
        return rc_name;
      cdd_escape_json_c_str(wname, esc_wname, sizeof(esc_wname));
      if (n_al == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "  else if (strcmp(str, \"%s\") == 0) *val = %s_%s;\n",
            esc_wname, enum_name, em->members[i]));
      } else {
        size_t a_idx;
        CHECK_IO(
            FPRINTF_HOOK(fp, "  else if (strcmp(str, \"%s\") == 0", esc_wname));
        for (a_idx = 0; a_idx < n_al; ++a_idx) {
          if (em->serde_aliases[i][a_idx]) {
            char esc_al[256];
            cdd_escape_json_c_str(em->serde_aliases[i][a_idx], esc_al,
                                  sizeof(esc_al));
            CHECK_IO(FPRINTF_HOOK(fp, " || strcmp(str, \"%s\") == 0", esc_al));
          }
        }
        CHECK_IO(
            FPRINTF_HOOK(fp, ") *val = %s_%s;\n", enum_name, em->members[i]));
      }
    }
  }
  if (em->serde_other_idx >= 0 && (size_t)em->serde_other_idx < em->size &&
      em->members[em->serde_other_idx]) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  else *val = %s_%s;\n  return CDD_C_SUCCESS;\n}\n",
                          enum_name, em->members[em->serde_other_idx]));
  } else {
    CHECK_IO(FPRINTF_HOOK(
        fp, "  else *val = %s_UNKNOWN;\n  return CDD_C_SUCCESS;\n}\n",
        enum_name));
  }

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n", config->guard_macro));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "\n"));

  return CDD_C_SUCCESS;
}
