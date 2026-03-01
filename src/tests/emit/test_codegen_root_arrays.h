#ifndef TEST_CODEGEN_ROOT_ARRAYS_H
#define TEST_CODEGEN_ROOT_ARRAYS_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/types.h"

/* Helper to capture output. Updated signature to match codegen_types functions.
 */
static char *
generate_ra_code(int (*fn)(FILE *, const char *, const char *, const char *,
                           const struct CodegenTypesConfig *),
                 const char *name, const char *type, const char *ref) {
  FILE *tmp = tmpfile();
  long sz;
  char *content;
  if (!tmp)
    return NULL;

  if (fn(tmp, name, type, ref, NULL) != 0) {
    fclose(tmp);
    return NULL;
  }
  fseek(tmp, 0, SEEK_END);
  if ((sz = ftell(tmp)) <= 0) {
    fclose(tmp);
    return NULL;
  }
  rewind(tmp);
  content = calloc(1, sz + 1);
  fread(content, 1, sz, tmp);
  fclose(tmp);
  return content;
}

TEST test_root_int_array_from_json(void) {
  char *code = generate_ra_code(write_root_array_from_json_func, "IntList",
                                "integer", NULL);
  ASSERT(code);
  /* Check signature */
  ASSERT(strstr(
      code, "int IntList_from_json(const char *json, int **out, size_t *len)"));
  /* Check malloc */
  ASSERT(strstr(code, "*out = malloc(count * sizeof(int));"));
  /* Check assignment cast */
  ASSERT(strstr(code, "(*out)[i] = (int)json_array_get_number(arr, i);"));
  free(code);
  PASS();
}

TEST test_root_string_array_from_json(void) {
  char *code = generate_ra_code(write_root_array_from_json_func, "StrList",
                                "string", NULL);
  ASSERT(code);
  /* Signature: char ***out */
  ASSERT(strstr(
      code,
      "int StrList_from_json(const char *json, char ***out, size_t *len)"));
  /* Check deep copy loop */
  ASSERT(strstr(code, "json_array_get_string(arr, i)"));
  ASSERT(strstr(code, "strdup(s)"));
  /* cleanup on failure */
  ASSERT(strstr(code, "free((*out)[j])"));
  free(code);
  PASS();
}

TEST test_root_obj_array_from_json(void) {
  /* Array of objects references "MyObj" */
  char *code = generate_ra_code(write_root_array_from_json_func, "ObjList",
                                "object", "MyObj");
  ASSERT(code);
  /* Signature: struct MyObj ***out */
  ASSERT(strstr(code, "int ObjList_from_json(const char *json, struct MyObj "
                      "***out, size_t *len)"));
  /* Recursive parse */
  ASSERT(strstr(
      code,
      "MyObj_from_jsonObject(json_array_get_object(arr, i), &(*out)[i])"));
  free(code);
  PASS();
}

TEST test_root_int_array_to_json(void) {
  char *code = generate_ra_code(write_root_array_to_json_func, "IntList",
                                "integer", NULL);
  ASSERT(code);
  ASSERT(strstr(
      code, "int IntList_to_json(const int *in, size_t len, char **json_out)"));
  ASSERT(strstr(code, "jasprintf(json_out, \"[\")"));
  ASSERT(strstr(code, "jasprintf(json_out, \"%d\", in[i])"));
  free(code);
  PASS();
}

TEST test_root_obj_array_to_json(void) {
  char *code = generate_ra_code(write_root_array_to_json_func, "ObjList",
                                "object", "MyObj");
  ASSERT(code);
  ASSERT(strstr(code, "int ObjList_to_json(struct MyObj **const in, size_t "
                      "len, char **json_out)"));
  ASSERT(strstr(code, "MyObj_to_json(in[i], &tmp)"));
  free(code);
  PASS();
}

TEST test_root_array_cleanup(void) {
  /* String cleanup */
  char *code = generate_ra_code(write_root_array_cleanup_func, "StrList",
                                "string", NULL);
  ASSERT(code);
  ASSERT(strstr(code, "void StrList_cleanup(char **in, size_t len)"));
  ASSERT(strstr(code, "free(in[i])"));
  ASSERT(strstr(code, "free(in)"));
  free(code);

  /* Int cleanup (simple free) */
  code = generate_ra_code(write_root_array_cleanup_func, "IntList", "integer",
                          NULL);
  ASSERT(code);
  ASSERT(strstr(code, "free(in)"));
  free(code);
  PASS();
}

SUITE(root_array_suite) {
  RUN_TEST(test_root_int_array_from_json);
  RUN_TEST(test_root_string_array_from_json);
  RUN_TEST(test_root_obj_array_from_json);
  RUN_TEST(test_root_int_array_to_json);
  RUN_TEST(test_root_obj_array_to_json);
  RUN_TEST(test_root_array_cleanup);
}

#endif /* TEST_CODEGEN_ROOT_ARRAYS_H */
