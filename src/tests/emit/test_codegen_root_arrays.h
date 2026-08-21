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
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/types.h"
/* clang-format on */
#include "c_cdd/memory.h"

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
static FILE *mock_tmpfile_ra(void) {
  if (g_fail_io_after >= 0 && ++g_io_calls == g_fail_io_after)
    return NULL;
  return tmpfile();
}
static long mock_ftell_ra(FILE *stream) {
  if (g_fail_io_after == 999)
    return 0;
  return ftell(stream);
}
static size_t mock_fread_ra(void *ptr, size_t size, size_t nitems,
                            FILE *stream) {
  if (g_fail_io_after == 998)
    return 0;
  return fread(ptr, size, nitems, stream);
}
#ifdef TMPFILE
#undef TMPFILE
#endif
#define TMPFILE mock_tmpfile_ra
#ifdef FTELL
#undef FTELL
#endif
#define FTELL mock_ftell_ra
#ifdef FREAD
#undef FREAD
#endif
#define FREAD mock_fread_ra
#else
#define TMPFILE tmpfile
#define FTELL ftell
#define FREAD fread
#endif

/* Helper to capture output. Updated signature to match codegen_types functions.
 */
static cdd_c_error_t generate_ra_code(
    cdd_c_error_t (*fn)(FILE *, const char *, const char *, const char *,
                        const struct CodegenTypesConfig *),
    const char *name, const char *type, const char *ref, char **_out_val) {
  FILE *tmp;
  long sz;
  char *content = NULL;
  cdd_c_error_t rc;

  tmp = TMPFILE();
  if (!tmp)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = fn(tmp, name, type, ref, NULL);
  if (rc != CDD_C_SUCCESS) {
    fclose(tmp);
    return rc;
  }

  fseek(tmp, 0, SEEK_END);
  sz = FTELL(tmp);
  if (sz <= 0) {
    fclose(tmp);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rewind(tmp);
  content = (char *)C_CDD_CALLOC(1, (size_t)sz + 1);
  if (!content) {
    fclose(tmp);
    return CDD_C_ERROR_MEMORY;
  }
  if (FREAD(content, 1, (size_t)sz, tmp) != (size_t)sz) {
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
  ASSERT(strstr(code, "cdd_c_error_t IntList_from_json(const char *json, "
                      "int **out, size_t *len)"));
  /* Check malloc */
  ASSERT(strstr(code, "*out = malloc(count * sizeof(int));"));
  /* Check assignment cast */
  ASSERT(strstr(code, "(*out)[i] = (int)json_array_get_number(arr, i);"));
  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_root_string_array_from_json(void) {
  char *_ast_generate_ra_code_1 = NULL;
  char *code = (generate_ra_code(write_root_array_from_json_func, "StrList",
                                 "string", NULL, &_ast_generate_ra_code_1),
                _ast_generate_ra_code_1);
  ASSERT(code);
  /* Signature: char ***out */
  ASSERT(strstr(code, "cdd_c_error_t StrList_from_json(const char *json, "
                      "char ***out, size_t *len)"));
  /* Check deep copy loop */
  ASSERT(strstr(code, "json_array_get_string(arr, i)"));
  ASSERT(strstr(code, "strdup(s)"));
  /* cleanup on failure */
  ASSERT(strstr(code, "free((*out)[j])"));
  free(code);
  g_fail_io_after = -1;
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
      code, "cdd_c_error_t ObjList_from_json(const char *json, struct MyObj "
            "***out, size_t *len)"));
  /* Recursive parse */
  ASSERT(strstr(
      code,
      "MyObj_from_jsonObject(json_array_get_object(arr, i), &(*out)[i])"));
  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_root_int_array_to_json(void) {
  char *_ast_generate_ra_code_3 = NULL;
  char *code = (generate_ra_code(write_root_array_to_json_func, "IntList",
                                 "integer", NULL, &_ast_generate_ra_code_3),
                _ast_generate_ra_code_3);
  ASSERT(code);
  ASSERT(strstr(code, "cdd_c_error_t IntList_to_json(const int *in, size_t "
                      "len, char **json_out)"));
  ASSERT(strstr(code, "c89stringutils_jasprintf(json_out, \"[\")"));
  ASSERT(strstr(code, "c89stringutils_jasprintf(json_out, \"%d\", in[i])"));
  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_root_obj_array_to_json(void) {
  char *_ast_generate_ra_code_4 = NULL;
  char *code = (generate_ra_code(write_root_array_to_json_func, "ObjList",
                                 "object", "MyObj", &_ast_generate_ra_code_4),
                _ast_generate_ra_code_4);
  ASSERT(code);
  ASSERT(strstr(code,
                "cdd_c_error_t ObjList_to_json(struct MyObj **const in, size_t "
                "len, char **json_out)"));
  ASSERT(strstr(code, "MyObj_to_json(in[i], &tmp)"));
  free(code);
  g_fail_io_after = -1;
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
  ASSERT(strstr(code, "cdd_c_error_t StrList_cleanup(char **in, size_t len)"));
  ASSERT(strstr(code, "free(in[i])"));
  ASSERT(strstr(code, "free(in)"));
  free(code);

  /* Int cleanup (simple free) */
  code = (generate_ra_code(write_root_array_cleanup_func, "IntList", "integer",
                           NULL, &_ast_generate_ra_code_6),
          _ast_generate_ra_code_6);
  ASSERT(code);
  ASSERT(strstr(code, "free(in)"));
  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_root_arrays_errors(void) {
  char *_out = NULL;

  g_io_calls = 0;
  g_fail_io_after = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            generate_ra_code(write_root_array_from_json_func, "MyArr", "string",
                             NULL, &_out));

  g_io_calls = 0;
  g_fail_io_after = 2; /* fn fails (write_root_array_from_json_func) */
  ASSERT_EQ(CDD_C_ERROR_IO, generate_ra_code(write_root_array_from_json_func,
                                             "MyArr", "string", NULL, &_out));

  g_fail_io_after = 999;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            generate_ra_code(write_root_array_from_json_func, "MyArr", "string",
                             NULL, &_out));

  g_fail_io_after = 998;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            generate_ra_code(write_root_array_from_json_func, "MyArr", "string",
                             NULL, &_out));

  g_fail_io_after = -1;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            generate_ra_code(write_root_array_from_json_func, "MyArr", "string",
                             NULL, &_out));
  g_cdd_alloc_fail = 0;

  PASS();
}

SUITE(root_array_suite) {
  RUN_TEST(test_root_arrays_errors);
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
