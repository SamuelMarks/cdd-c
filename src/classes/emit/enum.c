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

/**
 * @brief Initialize an EnumMembers container.
 *
 * @return CDD_C_SUCCESS on success, error code on allocation failure.
 */
cdd_c_error_t enum_members_init(struct EnumMembers *em) {
  if (!em)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  em->size = 0;
  em->capacity = 8;
  em->members = (char **)C_CDD_CALLOC(em->capacity, sizeof(char *));
  if (!em->members) {
    em->capacity = 0;
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Free the memory associated with an EnumMembers structure.
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
  em->size = 0;
  em->capacity = 0;
}

/**
 * @brief Add a member name to the EnumMembers list.
 *
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t enum_members_add(struct EnumMembers *em, const char *name) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!em || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (em->size >= em->capacity) {
    const size_t new_cap = em->capacity == 0 ? 8 : em->capacity * 2;
    char **new_members;
    new_members = (char **)C_CDD_REALLOC(em->members, new_cap * sizeof(char *));
    if (!new_members) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    em->members = new_members;
    em->capacity = new_cap;
  }
  rc = c_cdd_strdup(name, &em->members[em->size]);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  em->size++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generate the C code for converting an enum value to a string.
 *
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
    if (em->members[i] && strcmp(em->members[i], "UNKNOWN") != 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "    case %s_%s:\n      *str_out = %s(\"%s\");\n      break;\n",
          enum_name, em->members[i], kStrDupFunc, em->members[i]));
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

/**
 * @brief Generate the C code for parsing a string into an enum value.
 *
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
    /* Skip explicit logic for "UNKNOWN" member, let the final else catch it */
    if (em->members[i] && strcmp(em->members[i], "UNKNOWN") != 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  else if (strcmp(str, \"%s\") == 0) *val = %s_%s;\n",
          em->members[i], enum_name, em->members[i]));
    }
  }
  CHECK_IO(FPRINTF_HOOK(
      fp, "  else *val = %s_UNKNOWN;\n  return CDD_C_SUCCESS;\n}\n",
      enum_name));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n", config->guard_macro));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "\n"));

  return CDD_C_SUCCESS;
}
