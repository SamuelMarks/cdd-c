/**
 * @file enum.c
 * @brief Implementation of Enum extraction and generation logic.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "classes/emit/enum.h"
#include "functions/parse/str.h"
#include "c_cdd/log.h"
#include "c_cdd_export.h"
#include <stdarg.h>

/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
static int cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)));
#else
    ;
#endif
static int cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  va_list args;
  int rc;
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  va_start(args, format);
  rc = vfprintf(stream, format, args);
  va_end(args);
  return rc;
}
/** @brief FPRINTF_HOOK macro */
#define FPRINTF_HOOK cdd_fprintf_hook
#else
/** @brief FPRINTF_HOOK macro */
#define FPRINTF_HOOK fprintf
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127) /* conditional expression is constant */
#endif

/* Wrapper for fprintf to check errors tersely */
/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

/* Select correct strdup function name for generated code */
#if defined(_MSC_VER)
static const char *kStrDupFunc = "_strdup";
#else
static const char *kStrDupFunc = "strdup";
#endif

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_enum_members_init_fail = 0;
C_CDD_EXPORT int g_enum_members_add_fail = 0;
C_CDD_EXPORT int g_enum_members_add_strdup_fail = 0;
#endif

int enum_members_init(struct EnumMembers *em) {
  if (!em)
    return EINVAL;
  em->size = 0;
  em->capacity = 8;
#ifdef CDD_BUILD_TESTS
  if (g_enum_members_init_fail) {
    em->members = NULL;
  } else {
#endif
    em->members = (char **)calloc(em->capacity, sizeof(char *));
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!em->members) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }
  return 0;
}

void enum_members_free(struct EnumMembers *em) {
  size_t i;
  if (!em)
    return;
  if (em->members) {
    for (i = 0; i < em->size; ++i) {
      if (em->members[i])
        free(em->members[i]);
    }
    free(em->members);
    em->members = NULL;
  }
  em->size = 0;
  em->capacity = 0;
}

int enum_members_add(struct EnumMembers *em, const char *name) {
  if (!em || !name)
    return EINVAL;
  if (em->size >= em->capacity) {
    const size_t new_cap = em->capacity == 0 ? 8 : em->capacity * 2;
    char **new_members;
#ifdef CDD_BUILD_TESTS
    if (g_enum_members_add_fail) {
      new_members = NULL;
    } else {
#endif
      new_members = (char **)realloc(em->members, new_cap * sizeof(char *));
#ifdef CDD_BUILD_TESTS
    }
#endif
    if (!new_members) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return ENOMEM;
    }
    em->members = new_members;
    em->capacity = new_cap;
  }
#ifdef CDD_BUILD_TESTS
  if (g_enum_members_add_strdup_fail) {
    em->members[em->size] = NULL;
  } else {
#endif
    c_cdd_strdup(name, &em->members[em->size]);
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!em->members[em->size])
    return ENOMEM;
  em->size++;
  return 0;
}

int write_enum_to_str_func(FILE *fp, const char *enum_name,
                           const struct EnumMembers *em,
                           const struct CodegenEnumConfig *config) {
  size_t i;
  if (!fp || !enum_name || !em || !em->members)
    return EINVAL;

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(FPRINTF_HOOK(fp,
                        "int %s_to_str(enum %s val, char **str_out) {\n"
                        "  if (str_out == NULL) return EINVAL;\n"
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
                        "(*str_out == NULL) return ENOMEM;\n  return 0;\n}\n",
                        enum_name, kStrDupFunc));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n", config->guard_macro));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "\n"));

  return 0;
}

int write_enum_from_str_func(FILE *fp, const char *enum_name,
                             const struct EnumMembers *em,
                             const struct CodegenEnumConfig *config) {
  size_t i;
  if (!fp || !enum_name || !em || !em->members)
    return EINVAL;

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(FPRINTF_HOOK(fp,
                        "int %s_from_str(const char *str, enum %s *val) {\n"
                        "  if (val == NULL) return EINVAL;\n"
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
  CHECK_IO(FPRINTF_HOOK(fp, "  else *val = %s_UNKNOWN;\n  return 0;\n}\n",
                        enum_name));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n", config->guard_macro));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "\n"));

  return 0;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
