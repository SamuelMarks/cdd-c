/**
 * @file codegen_enum.c
 * @brief Implementation of Enum extraction and generation logic.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit_enum.h"
#include "functions/parse_str.h"

/* Wrapper for fprintf to check errors tersely */
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

int enum_members_init(struct EnumMembers *const em) {
  if (!em)
    return EINVAL;
  em->size = 0;
  em->capacity = 8;
  em->members = (char **)calloc(em->capacity, sizeof(char *));
  if (!em->members)
    return ENOMEM;
  return 0;
}

void enum_members_free(struct EnumMembers *const em) {
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

int enum_members_add(struct EnumMembers *const em, const char *const name) {
  if (!em || !name)
    return EINVAL;
  if (em->size >= em->capacity) {
    const size_t new_cap = em->capacity == 0 ? 8 : em->capacity * 2;
    char **new_members =
        (char **)realloc(em->members, new_cap * sizeof(char *));
    if (!new_members)
      return ENOMEM;
    em->members = new_members;
    em->capacity = new_cap;
  }
  em->members[em->size] = c_cdd_strdup(name);
  if (!em->members[em->size])
    return ENOMEM;
  em->size++;
  return 0;
}

int write_enum_to_str_func(FILE *const fp, const char *const enum_name,
                           const struct EnumMembers *const em,
                           const struct CodegenEnumConfig *const config) {
  size_t i;
  if (!fp || !enum_name || !em || !em->members)
    return EINVAL;

  if (config && config->guard_macro) {
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(fprintf(fp,
                   "int %s_to_str(enum %s val, char **str_out) {\n"
                   "  if (str_out == NULL) return EINVAL;\n"
                   "  switch (val) {\n",
                   enum_name, enum_name));

  for (i = 0; i < em->size; i++) {
    if (em->members[i] && strcmp(em->members[i], "UNKNOWN") != 0) {
      CHECK_IO(fprintf(
          fp, "    case %s_%s:\n      *str_out = %s(\"%s\");\n      break;\n",
          enum_name, em->members[i], kStrDupFunc, em->members[i]));
    }
  }
  CHECK_IO(fprintf(fp,
                   "    case %s_UNKNOWN:\n    default:\n      *str_out = "
                   "%s(\"UNKNOWN\");\n      break;\n  }\n  if "
                   "(*str_out == NULL) return ENOMEM;\n  return 0;\n}\n",
                   enum_name, kStrDupFunc));

  if (config && config->guard_macro) {
    CHECK_IO(fprintf(fp, "#endif /* %s */\n", config->guard_macro));
  }
  CHECK_IO(fprintf(fp, "\n"));

  return 0;
}

int write_enum_from_str_func(FILE *const fp, const char *const enum_name,
                             const struct EnumMembers *const em,
                             const struct CodegenEnumConfig *const config) {
  size_t i;
  if (!fp || !enum_name || !em || !em->members)
    return EINVAL;

  if (config && config->guard_macro) {
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(fprintf(fp,
                   "int %s_from_str(const char *const str, enum %s *val) {\n"
                   "  if (val == NULL) return EINVAL;\n"
                   "  else if (str == NULL) *val = %s_UNKNOWN;\n",
                   enum_name, enum_name, enum_name));

  for (i = 0; i < em->size; i++) {
    /* Skip explicit logic for "UNKNOWN" member, let the final else catch it */
    if (em->members[i] && strcmp(em->members[i], "UNKNOWN") != 0) {
      CHECK_IO(fprintf(fp,
                       "  else if (strcmp(str, \"%s\") == 0) *val = %s_%s;\n",
                       em->members[i], enum_name, em->members[i]));
    }
  }
  CHECK_IO(
      fprintf(fp, "  else *val = %s_UNKNOWN;\n  return 0;\n}\n", enum_name));

  if (config && config->guard_macro) {
    CHECK_IO(fprintf(fp, "#endif /* %s */\n", config->guard_macro));
  }
  CHECK_IO(fprintf(fp, "\n"));

  return 0;
}
