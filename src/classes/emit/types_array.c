/**
 * @file types_array.c
 * @brief Code generation for root array types.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/emit/types.h"
#include "c_cdd/log.h"
#include "classes/emit/struct.h"
#include "functions/parse/str.h"
#include "win_compat_sym.h"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/* Wrapper for fprintf to check errors tersely */
#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)));
#else
    ;
#endif
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  int ret;
  va_list args;
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  va_start(args, format);
  ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
/** @brief FPRINTF_HOOK macro */
#define FPRINTF_HOOK test_cdd_fprintf_hook
#else
/** @brief FPRINTF_HOOK macro */
#define FPRINTF_HOOK fprintf
#endif

/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

/**
 * @brief Generates C code for write root array cleanup func.
 */
cdd_c_error_t
write_root_array_cleanup_func(FILE *fp, const char *name, const char *item_type,
                              const char *item_ref,
                              const struct CodegenTypesConfig *config) {
  char *_ast_get_type_from_ref_7 = NULL;
  char *_ast_get_type_from_ref_8 = NULL;
  if (!fp || !name || !item_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->utils_guard));

  if (strcmp(item_type, "integer") == 0) {
    /* Simple flat array */
    CHECK_IO(FPRINTF_HOOK(fp,
                          "cdd_c_error_t %s_cleanup(int *in, size_t len) {\n"
                          "  (void)len; free(in);\n}\n",
                          name));
  } else if (strcmp(item_type, "string") == 0) {
    /* Array of pointers to strings */
    CHECK_IO(FPRINTF_HOOK(fp,
                          "cdd_c_error_t %s_cleanup(char **in, size_t len) {\n"
                          "  size_t i;\n"
                          "  if (!in) return;\n"
                          "  for(i=0; i<len; ++i) free(in[i]);\n"
                          "  free(in);\n"
                          "}\n",
                          name));
  } else if (strcmp(item_type, "object") == 0) {
    /* Array of pointers to structs */
    /* LCOV_EXCL_START */
    CHECK_IO(
        FPRINTF_HOOK(fp,
                     "cdd_c_error_t %s_cleanup(struct %s **in, size_t len) {\n"
                     "  size_t i;\n"
                     "  if (!in) return;\n"
                     "  for(i=0; i<len; ++i) %s_cleanup(in[i]);\n"
                     "  free(in);\n"
                     "}\n",
                     name,
                     (get_type_from_ref(item_ref, &_ast_get_type_from_ref_7),
                      _ast_get_type_from_ref_7),
                     (get_type_from_ref(item_ref, &_ast_get_type_from_ref_8),
                      _ast_get_type_from_ref_8)));
    /* LCOV_EXCL_STOP */
  } else {
    /* Fallback generic void* */
    CHECK_IO(FPRINTF_HOOK(fp,
                          "cdd_c_error_t %s_cleanup(void *in, size_t len) { "
                          "(void)len; free(in); }\n",
                          name));
  }

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->utils_guard));

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write root array to json func.
 */
cdd_c_error_t
write_root_array_to_json_func(FILE *fp, const char *name, const char *item_type,
                              const char *item_ref,
                              const struct CodegenTypesConfig *config) {
  char *_ast_get_type_from_ref_10 = NULL;
  if (!fp || !name || !item_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->json_guard));

  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "cdd_c_error_t %s_to_json(const int *in, size_t "
                          "len, char **json_out) {\n",
                          name));
  } else if (strcmp(item_type, "string") == 0) {
    /* LCOV_EXCL_START */
    CHECK_IO(FPRINTF_HOOK(fp,
                          "cdd_c_error_t %s_to_json(char **const in, size_t "
                          "len, char **json_out) {\n",
                          name));
    /* LCOV_EXCL_STOP */
  } else if (strcmp(item_type, "object") == 0) {
    {
      char *tn = NULL;
      get_type_from_ref(item_ref, &tn);
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "cdd_c_error_t %s_to_json(struct %s **const in, size_t len, char "
          "**json_out) {\n",
          name, tn));
    }
  } else {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "cdd_c_error_t %s_to_json(const void *in, size_t "
                          "len, char **json_out) {\n",
                          name));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "  size_t i;\n"
          "  if (!in && len > 0) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  if (!json_out) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  c89stringutils_jasprintf(json_out, \"[\");\n"
          "  if (!*json_out) return CDD_C_ERROR_MEMORY;\n"
          "  for (i = 0; i < len; ++i) {\n"
          "    if (i > 0) { c89stringutils_jasprintf(json_out, \",\"); "
          "if(!*json_out) return CDD_C_ERROR_MEMORY; }\n"));

  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp, "    c89stringutils_jasprintf(json_out, \"%%d\", in[i]);\n"));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_IO(FPRINTF_HOOK(fp, "    c89stringutils_jasprintf(json_out, "
                              "\"\\\"%%s\\\"\", in[i]);\n"));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "    {\n"
        "      char *tmp = NULL;\n"
        "      int rc = %s_to_json(in[i], &tmp);\n"
        "      if (rc != CDD_C_SUCCESS) { free(tmp); return rc; }\n"
        "      c89stringutils_jasprintf(json_out, \"%%s\", tmp);\n"
        "      free(tmp);\n"
        "    }\n",
        (get_type_from_ref(item_ref, &_ast_get_type_from_ref_10),
         _ast_get_type_from_ref_10)));
  }
  CHECK_IO(FPRINTF_HOOK(
      fp, "    if (!*json_out) return CDD_C_ERROR_MEMORY;\n  }\n"));
  CHECK_IO(FPRINTF_HOOK(
      fp, "  c89stringutils_jasprintf(json_out, \"]\");\n  if(!*json_out) "
          "return CDD_C_ERROR_MEMORY;\n  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->json_guard));

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write root array from json func.
 */
cdd_c_error_t
write_root_array_from_json_func(FILE *fp, const char *name,
                                const char *item_type, const char *item_ref,
                                const struct CodegenTypesConfig *config) {
  char *_ast_get_type_from_ref_11 = NULL;
  char *_ast_get_type_from_ref_12 = NULL;
  char *_ast_get_type_from_ref_13 = NULL;
  char *_ast_get_type_from_ref_14 = NULL;
  if (!fp || !name || !item_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->json_guard));

  /* Choose arg signature */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "cdd_c_error_t %s_from_json(const char *json, int "
                          "**out, size_t *len) {\n",
                          name));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "cdd_c_error_t %s_from_json(const char *json, char ***out, size_t "
        "*len) {\n",
        name));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_IO(
        FPRINTF_HOOK(fp,
                     "cdd_c_error_t %s_from_json(const char *json, struct "
                     "%s ***out, size_t "
                     "*len) {\n",
                     name,
                     (get_type_from_ref(item_ref, &_ast_get_type_from_ref_11),
                      _ast_get_type_from_ref_11)));
  } else {
    /* Fallback */
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "cdd_c_error_t %s_from_json(const char *json, void **out, size_t "
        "*len) {\n",
        name));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "  JSON_Value *val;\n"
          "  JSON_Array *arr;\n"
          "  size_t i, count;\n"
          "  if (!json || !out || !len) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  val = json_parse_string(json);\n"
          "  if (!val) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  arr = json_value_get_array(val);\n"
          "  if (!arr) { json_value_free(val); return "
          "CDD_C_ERROR_INVALID_ARGUMENT; }\n"
          "  count = json_array_get_count(arr);\n"
          "  *len = count;\n"
          "  if (count == 0) { *out = NULL; json_value_free(val); return "
          "CDD_C_SUCCESS; "
          "}\n"));

  /* Allocate array container */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(FPRINTF_HOOK(fp, "  *out = malloc(count * sizeof(int));\n"));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_IO(FPRINTF_HOOK(fp, "  *out = calloc(count, sizeof(char *));\n"));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_IO(
        FPRINTF_HOOK(fp, "  *out = calloc(count, sizeof(struct %s*));\n",
                     (get_type_from_ref(item_ref, &_ast_get_type_from_ref_12),
                      _ast_get_type_from_ref_12)));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "  if (!*out) { json_value_free(val); return CDD_C_ERROR_MEMORY; }\n"
          "  for (i = 0; i < count; ++i) {\n"));

  /* Parse Loop */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp, "    (*out)[i] = (int)json_array_get_number(arr, i);\n"));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "    const char *s = json_array_get_string(arr, i);\n"
                          "    if (s) (*out)[i] = strdup(s);\n"
                          "    if (!(*out)[i]) {\n"
                          "      /* cleanup */\n"
                          "      size_t j;\n"
                          "      for(j=0; j<i; j++) free((*out)[j]);\n"
                          "      free(*out); *out=NULL; json_value_free(val); "
                          "return CDD_C_ERROR_MEMORY;\n"
                          "    }\n"));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "    int rc = %s_from_jsonObject(json_array_get_object(arr, i), "
        "&(*out)[i]);\n"
        "    if (rc != CDD_C_SUCCESS) {\n"
        "      size_t j;\n"
        "      for(j=0; j<i; j++) %s_cleanup((*out)[j]);\n"
        "      free(*out); *out=NULL; json_value_free(val); return rc;\n"
        "    }\n",
        (get_type_from_ref(item_ref, &_ast_get_type_from_ref_13),
         _ast_get_type_from_ref_13),
        (get_type_from_ref(item_ref, &_ast_get_type_from_ref_14),
         _ast_get_type_from_ref_14)));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "  }\n  json_value_free(val);\n  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->json_guard));

  return CDD_C_SUCCESS;
}
