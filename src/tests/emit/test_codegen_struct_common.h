/**
 * @file test_codegen_struct_common.h
 * @brief Common test fixtures and helper functions for struct codegen and
 * reflection tests.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_COMMON_H
#define TEST_CODEGEN_STRUCT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_test_helpers_export.h"
#include "c_cdd_export.h"
#include "c_cdd/memory.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/struct.h"
/* clang-format on */

/**
 * @brief Global test temporary file helper.
 * @return Temporary FILE pointer or NULL on failure.
 */
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);

/** @brief Test definition g_cdd_fail_str_after_last. */
extern C_CDD_EXPORT int g_cdd_fail_str_after_last;

/**
 * @brief Helper function setup_struct_fields.
 * @param[in,out] sf Parameter sf.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t setup_struct_fields(struct StructFields *sf) {
  cdd_c_error_t rc;
  rc = struct_fields_init(sf);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = struct_fields_add(sf, "id", "integer", NULL, "0", NULL);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = struct_fields_add(sf, "name", "string", NULL, "\"test\"", NULL);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Visitor callback used to test cdd_reflect_visit_fields.
 * @param[in] field Field reflection descriptor.
 * @param[in] field_ptr Pointer to field value in struct instance.
 * @param[in,out] user_data Counter / stop-after-1 flag.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t test_reflect_visitor_cb(const struct cdd_field_info *field,
                                             const void *field_ptr,
                                             void *user_data) {
  int *cnt = (int *)user_data;
  if (!field || !cnt)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->name && strcmp(field->name, "fail_me") == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->bit_width == 0 && !field_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->bit_width > 0 && field_ptr != NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (*cnt < 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  (*cnt)++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Mutable visitor callback used to test cdd_reflect_visit_fields_mut.
 * @param[in] field Field reflection descriptor.
 * @param[in,out] field_ptr Mutable pointer to field value in struct instance.
 * @param[in,out] user_data Counter / stop-after-1 flag.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t
test_reflect_mut_visitor_cb(const struct cdd_field_info *field, void *field_ptr,
                            void *user_data) {
  int *cnt = (int *)user_data;
  if (!field || !cnt)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (*cnt < 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (field->bit_width == 0 && !field->is_pointer && field_ptr &&
      field->kind == CDD_FIELD_KIND_INTEGER && field->size == sizeof(int)) {
    *(int *)field_ptr += 10;
  }
  (*cnt)++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Dummy bit-field getter for testing cdd_reflect_get_int.
 * @param[in] obj Struct instance pointer.
 * @param[out] out_val Output integer value.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t test_bf_get(const void *obj, int *out_val) {
  if (!obj || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = 7;
  return CDD_C_SUCCESS;
}

/**
 * @brief Failing bit-field getter for testing error propagation.
 * @param[in] obj Struct instance pointer.
 * @param[out] out_val Output integer value.
 * @return CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t test_bf_get_fail(const void *obj, int *out_val) {
  if (!obj && !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Dummy bit-field setter for testing cdd_reflect_set_int.
 * @param[in,out] obj Struct instance pointer.
 * @param[in] val New integer value.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t test_bf_set(void *obj, int val) {
  *(int *)obj = val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Dummy invoke trampoline for reflection function invocation tests.
 *
 * @param[in] args Array of argument pointers.
 * @param[out] ret_out Output return value pointer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t dummy_add_invoke_trampoline(const void *const *args,
                                                 void *ret_out) {
  int a, b;
  if (!args || !args[0] || !args[1])
    return CDD_C_ERROR_INVALID_ARGUMENT;
  a = *(const int *)args[0];
  b = *(const int *)args[1];
  if (ret_out)
    *(int *)ret_out = a + b;
  return CDD_C_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_STRUCT_COMMON_H */
