/**
 * @file test_codegen_root_arrays.h
 * @brief Unit tests for root array codegen.
 */

#ifndef TEST_CODEGEN_ROOT_ARRAYS_H
#define TEST_CODEGEN_ROOT_ARRAYS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/types.h"
/* clang-format on */

/* Helper to capture output. Updated signature to match codegen_types functions.
 */
static enum cdd_c_error generate_ra_code(
    enum cdd_c_error (*fn)(FILE *, const char *, const char *, const char *,
                           const struct CodegenTypesConfig *),
    const char *name, const char *type, const char *ref, char **_out_val) {
  FILE *tmp;
  long sz;
  char *content = NULL;
  enum cdd_c_error rc;

  tmp = tmpfile();
  if (!tmp)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = fn(tmp, name, type, ref, NULL);
  if (rc != CDD_C_SUCCESS) {
    fclose(tmp);
    return rc;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  if (sz <= 0) {
    fclose(tmp);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, sz + 1);
  if (!content) {
    fclose(tmp);
    return CDD_C_ERROR_MEMORY;
  }
  if (fread(content, 1, sz, tmp) != (size_t)sz) {
    C_CDD_FREE(content);
    fclose(tmp);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  fclose(tmp);
  *_out_val = content;
  return CDD_C_SUCCESS;
}

TEST test_root_int_array_from_json(void) {
  char *_ast_generate_ra_code_0 = NULL;
  char *code = (generate_ra_code(write_root_array_from_json_func, "IntList",
                                 "integer", NULL, &_ast_generate_ra_code_0),
                _ast_generate_ra_code_0);
  ASSERT(code);
  /* Check signature */
  ASSERT(strstr(code, "enum cdd_c_error IntList_from_json(const char *json, "
                      "int **out, size_t *len)"));
  /* Check malloc */
  ASSERT(strstr(code, "*out = C_CDD_MALLOC(count * sizeof(int));"));
  /* Check assignment cast */
  ASSERT(strstr(code, "(*out)[i] = (int)json_array_get_number(arr, i);"));
  C_CDD_FREE(code);

  PASS();
}

TEST test_root_string_array_from_json(void) {
  char *_ast_generate_ra_code_1 = NULL;
  char *code = (generate_ra_code(write_root_array_from_json_func, "StrList",
                                 "string", NULL, &_ast_generate_ra_code_1),
                _ast_generate_ra_code_1);
  ASSERT(code);
  /* Signature: char ***out */
  ASSERT(strstr(code, "enum cdd_c_error StrList_from_json(const char *json, "
                      "char ***out, size_t *len)"));
  /* Check deep copy loop */
  ASSERT(strstr(code, "json_array_get_string(arr, i)"));
  ASSERT(strstr(code, "strdup(s)"));
  /* cleanup on failure */
  ASSERT(strstr(code, "C_CDD_FREE((*out)[j])"));
  C_CDD_FREE(code);

  PASS();
}

TEST test_root_obj_array_from_json(void) {
  char *_ast_generate_ra_code_2 = NULL;
  /* Array of objects references "MyObj" */
  char *code = (generate_ra_code(write_root_array_from_json_func, "ObjList",
                                 "object", "MyObj", &_ast_generate_ra_code_2),
                _ast_generate_ra_code_2);
  ASSERT(code);
  /* Signature: struct MyObj ***out */
  ASSERT(strstr(
      code, "enum cdd_c_error ObjList_from_json(const char *json, struct MyObj "
            "***out, size_t *len)"));
  /* Recursive parse */
  ASSERT(strstr(
      code,
      "MyObj_from_jsonObject(json_array_get_object(arr, i), &(*out)[i])"));
  C_CDD_FREE(code);

  PASS();
}

TEST test_root_int_array_to_json(void) {
  char *_ast_generate_ra_code_3 = NULL;
  char *code = (generate_ra_code(write_root_array_to_json_func, "IntList",
                                 "integer", NULL, &_ast_generate_ra_code_3),
                _ast_generate_ra_code_3);
  ASSERT(code);
  ASSERT(strstr(code, "enum cdd_c_error IntList_to_json(const int *in, size_t "
                      "len, char **json_out)"));
  ASSERT(strstr(code, "c89stringutils_jasprintf(json_out, \"[\")"));
  ASSERT(strstr(code, "c89stringutils_jasprintf(json_out, \"%d\", in[i])"));
  C_CDD_FREE(code);

  PASS();
}

TEST test_root_obj_array_to_json(void) {
  char *_ast_generate_ra_code_4 = NULL;
  char *code = (generate_ra_code(write_root_array_to_json_func, "ObjList",
                                 "object", "MyObj", &_ast_generate_ra_code_4),
                _ast_generate_ra_code_4);
  ASSERT(code);
  ASSERT(strstr(
      code, "enum cdd_c_error ObjList_to_json(struct MyObj **const in, size_t "
            "len, char **json_out)"));
  ASSERT(strstr(code, "MyObj_to_json(in[i], &tmp)"));
  C_CDD_FREE(code);

  PASS();
}

TEST test_root_array_cleanup(void) {
  char *_ast_generate_ra_code_5 = NULL;
  char *_ast_generate_ra_code_6 = NULL;
  /* String cleanup */
  char *code = (generate_ra_code(write_root_array_cleanup_func, "StrList",
                                 "string", NULL, &_ast_generate_ra_code_5),
                _ast_generate_ra_code_5);
  ASSERT(code);
  ASSERT(
      strstr(code, "enum cdd_c_error StrList_cleanup(char **in, size_t len)"));
  ASSERT(strstr(code, "C_CDD_FREE(in[i])"));
  ASSERT(strstr(code, "C_CDD_FREE(in)"));
  C_CDD_FREE(code);

  /* Int cleanup (simple free) */
  code = (generate_ra_code(write_root_array_cleanup_func, "IntList", "integer",
                           NULL, &_ast_generate_ra_code_6),
          _ast_generate_ra_code_6);
  ASSERT(code);
  ASSERT(strstr(code, "C_CDD_FREE(in)"));
  C_CDD_FREE(code);

  PASS();
}

TEST test_root_string_array_to_json(void) {
  char *_ast_generate_ra_code_7 = NULL;
  char *code = (generate_ra_code(write_root_array_to_json_func, "StrList",
                                 "string", NULL, &_ast_generate_ra_code_7),
                _ast_generate_ra_code_7);
  ASSERT(code);
  ASSERT(strstr(code, "enum cdd_c_error StrList_to_json(char **const in, "
                      "size_t len, char **json_out)"));
  ASSERT(strstr(code,
                "c89stringutils_jasprintf(json_out, \"\\\"%s\\\"\", in[i])"));
  C_CDD_FREE(code);

  PASS();
}

TEST test_root_fallback_to_json(void) {
  char *_ast_generate_ra_code_8 = NULL;
  char *code = (generate_ra_code(write_root_array_to_json_func, "UnknownList",
                                 "unknown", NULL, &_ast_generate_ra_code_8),
                _ast_generate_ra_code_8);
  ASSERT(code);
  ASSERT(strstr(code, "enum cdd_c_error UnknownList_to_json(const void *in, "
                      "size_t len, char **json_out)"));
  C_CDD_FREE(code);

  char *_ast_generate_ra_code_9 = NULL;
  code = (generate_ra_code(write_root_array_cleanup_func, "UnknownList",
                           "unknown", NULL, &_ast_generate_ra_code_9),
          _ast_generate_ra_code_9);
  ASSERT(code);
  ASSERT(strstr(code,
                "enum cdd_c_error UnknownList_cleanup(void *in, size_t len)"));
  C_CDD_FREE(code);

  PASS();
}

SUITE(root_array_suite) {
  RUN_TEST(test_root_string_array_to_json);
  RUN_TEST(test_root_fallback_to_json);
  RUN_TEST(test_root_int_array_from_json);
  RUN_TEST(test_root_string_array_from_json);
  RUN_TEST(test_root_obj_array_from_json);
  RUN_TEST(test_root_int_array_to_json);
  RUN_TEST(test_root_obj_array_to_json);
  RUN_TEST(test_root_array_cleanup);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_ROOT_ARRAYS_H */
