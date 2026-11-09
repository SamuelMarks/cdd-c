/**
 * @file arazzo.c
 * @brief Implementation of Arazzo Specification v1.1.0 parser and validator.
 *
 * Parses Arazzo v1.1.0 JSON trees and strings into `struct Arazzo_Spec`,
 * extracts `^x-` specification extensions, resolves `$components.*` reusable
 * references, and validates semantic invariants in strict C89.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "arazzo/parse/arazzo.h"
#include "arazzo/parse/arazzo_expr.h"
#include "c_cdd/memory.h"
#include "openapi/parse/openapi_internal.h"
#include "openapi/parse/openapi_schema.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Extracts all `^x-` specification extensions from a `JSON_Object` into
 * `ext_list`.
 *
 * @param[in] obj Source JSON object.
 * @param[in,out] ext_list Destination extension list.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t
parse_arazzo_extensions(const JSON_Object *obj,
                        struct Arazzo_ExtensionList *ext_list) {
  size_t i;
  size_t count = 0;

  g_cdd_last_alloc_err = cdd_json_object_get_count(obj, &count);
  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_name(obj, i, &name);
    if (strncmp(name, "x-", 2) == 0) {
      JSON_Value *val_at = NULL;
      struct OpenAPI_Any val;
      cdd_c_error_t rc;
      memset(&val, 0, sizeof(val));
      g_cdd_last_alloc_err = cdd_json_object_get_value_at(obj, i, &val_at);
      rc = parse_any_value(val_at, &val);
      if (rc == CDD_C_SUCCESS)
        rc = arazzo_extension_list_append(ext_list, name, &val);
      g_cdd_last_alloc_err = free_any_value(&val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Duplicates an optional string field from a `JSON_Object` into
 * `*out_str`.
 *
 * @param[in] obj Source JSON object.
 * @param[in] key Property name.
 * @param[out] out_str Destination string pointer.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t dup_opt_json_str(const JSON_Object *obj, const char *key,
                                      char **out_str) {
  const char *s = NULL;
  g_cdd_last_alloc_err = cdd_json_object_get_string(obj, key, &s);
  if (!s)
    return CDD_C_SUCCESS;
  *out_str = C_CDD_STRDUP(s);
  return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/** @brief Reflected fields for `Arazzo_Info`. */
static const struct cdd_field_info g_arazzo_info_fields[] = {
    {"title",
     "title",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Info, title),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"summary",
     "summary",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Info, summary),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Info, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"version",
     "version",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Info, version),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info Arazzo_Info_type_info = {
    "Arazzo_Info",
    sizeof(struct Arazzo_Info),
    0,
    g_arazzo_info_fields,
    sizeof(g_arazzo_info_fields) / sizeof(g_arazzo_info_fields[0]),
    0};

/** @brief Reflected fields for `Arazzo_SourceDescription`. */
static const struct cdd_field_info g_arazzo_source_description_fields[] = {
    {"name",
     "name",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_SourceDescription, name),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"url",
     "url",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_SourceDescription, url),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info Arazzo_SourceDescription_type_info = {
    "Arazzo_SourceDescription",
    sizeof(struct Arazzo_SourceDescription),
    0,
    g_arazzo_source_description_fields,
    sizeof(g_arazzo_source_description_fields) /
        sizeof(g_arazzo_source_description_fields[0]),
    0};

/** @brief Reflected fields for `Arazzo_Criterion`. */
static const struct cdd_field_info g_arazzo_criterion_fields[] = {
    {"condition",
     "condition",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Criterion, condition),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"context",
     "context",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Criterion, context),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info Arazzo_Criterion_type_info = {
    "Arazzo_Criterion",
    sizeof(struct Arazzo_Criterion),
    0,
    g_arazzo_criterion_fields,
    sizeof(g_arazzo_criterion_fields) / sizeof(g_arazzo_criterion_fields[0]),
    0};

/** @brief Reflected fields for `Arazzo_Selector`. */
static const struct cdd_field_info g_arazzo_selector_fields[] = {
    {"context",
     "context",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Selector, context),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"selector",
     "selector",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Selector, selector),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info Arazzo_Selector_type_info = {
    "Arazzo_Selector",
    sizeof(struct Arazzo_Selector),
    0,
    g_arazzo_selector_fields,
    sizeof(g_arazzo_selector_fields) / sizeof(g_arazzo_selector_fields[0]),
    0};

/** @brief Reflected fields for `Arazzo_Reusable`. */
static const struct cdd_field_info g_arazzo_reusable_fields[] = {
    {"reference",
     "reference",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Reusable, reference),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info Arazzo_Reusable_type_info = {
    "Arazzo_Reusable",
    sizeof(struct Arazzo_Reusable),
    0,
    g_arazzo_reusable_fields,
    sizeof(g_arazzo_reusable_fields) / sizeof(g_arazzo_reusable_fields[0]),
    0};

/** @brief Reflected fields for `Arazzo_SuccessAction`. */
static const struct cdd_field_info g_arazzo_success_action_fields[] = {
    {"name",
     "name",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_SuccessAction, name),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"workflow_id",
     "workflowId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_SuccessAction, workflow_id),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"step_id",
     "stepId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_SuccessAction, step_id),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info Arazzo_SuccessAction_type_info = {
    "Arazzo_SuccessAction",
    sizeof(struct Arazzo_SuccessAction),
    0,
    g_arazzo_success_action_fields,
    sizeof(g_arazzo_success_action_fields) /
        sizeof(g_arazzo_success_action_fields[0]),
    0};

/** @brief Reflected fields for `Arazzo_FailureAction`. */
static const struct cdd_field_info g_arazzo_failure_action_fields[] = {
    {"name",
     "name",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_FailureAction, name),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"workflow_id",
     "workflowId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_FailureAction, workflow_id),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"step_id",
     "stepId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_FailureAction, step_id),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info Arazzo_FailureAction_type_info = {
    "Arazzo_FailureAction",
    sizeof(struct Arazzo_FailureAction),
    0,
    g_arazzo_failure_action_fields,
    sizeof(g_arazzo_failure_action_fields) /
        sizeof(g_arazzo_failure_action_fields[0]),
    0};

/** @brief Reflected fields for `Arazzo_Step`. */
static const struct cdd_field_info g_arazzo_step_fields[] = {
    {"step_id",
     "stepId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Step, step_id),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Step, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"operation_id",
     "operationId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Step, operation_id),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"operation_path",
     "operationPath",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Step, operation_path),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"channel_path",
     "channelPath",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Step, channel_path),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"workflow_id",
     "workflowId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Step, workflow_id),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"correlation_id",
     "correlationId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Step, correlation_id),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info Arazzo_Step_type_info = {
    "Arazzo_Step",
    sizeof(struct Arazzo_Step),
    0,
    g_arazzo_step_fields,
    sizeof(g_arazzo_step_fields) / sizeof(g_arazzo_step_fields[0]),
    0};

/** @brief Reflected fields for `Arazzo_Workflow`. */
static const struct cdd_field_info g_arazzo_workflow_fields[] = {
    {"workflow_id",
     "workflowId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Workflow, workflow_id),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"summary",
     "summary",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Workflow, summary),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct Arazzo_Workflow, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info Arazzo_Workflow_type_info = {
    "Arazzo_Workflow",
    sizeof(struct Arazzo_Workflow),
    0,
    g_arazzo_workflow_fields,
    sizeof(g_arazzo_workflow_fields) / sizeof(g_arazzo_workflow_fields[0]),
    0};

/**
 * @brief Table-driven helper to populate reflected fields from a `JSON_Object`
 * using `cdd_reflect_from_json_object`.
 *
 * @param[in] obj Source JSON object.
 * @param[in] ti Runtime reflection descriptor.
 * @param[in,out] target Target C struct pointer.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t
arazzo_parse_reflected_strings(const JSON_Object *obj,
                               const struct cdd_type_info *ti, void *target) {
  return cdd_reflect_from_json_object(obj, ti, target);
}

/**
 * @brief Parses a Criterion / ExpressionType `type` field (string or Expression
 * Type Object).
 *
 * @param[in] obj Parent JSON object (`Criterion` or `Selector` or
 * `PayloadReplacement`).
 * @param[in] type_key Key name (`"type"` or `"targetSelectorType"`).
 * @param[out] out_type Output criterion/selector type enum.
 * @param[out] out_et Output `Arazzo_ExpressionType` structure.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on bad
 * dialect, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t
parse_criterion_or_expr_type(const JSON_Object *obj, const char *type_key,
                             enum Arazzo_CriterionType *out_type,
                             struct Arazzo_ExpressionType *out_et) {
  JSON_Value *tval = NULL;
  JSON_Value_Type vtype = JSONError;
  cdd_c_error_t rc;

  g_cdd_last_alloc_err = cdd_json_object_get_value(obj, type_key, &tval);
  if (!tval) {
    *out_type = ARAZZO_CRITERION_SIMPLE;
    return CDD_C_SUCCESS;
  }

  g_cdd_last_alloc_err = cdd_json_value_get_type(tval, &vtype);
  if (vtype == JSONString) {
    const char *tstr = NULL;
    g_cdd_last_alloc_err = cdd_json_value_get_string(tval, &tstr);
    if (strcmp(tstr, "simple") == 0)
      *out_type = ARAZZO_CRITERION_SIMPLE;
    else if (strcmp(tstr, "regex") == 0)
      *out_type = ARAZZO_CRITERION_REGEX;
    else if (strcmp(tstr, "jsonpath") == 0)
      *out_type = ARAZZO_CRITERION_JSONPATH;
    else if (strcmp(tstr, "xpath") == 0)
      *out_type = ARAZZO_CRITERION_XPATH;
    else
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return CDD_C_SUCCESS;
  }

  if (vtype == JSONObject) {
    JSON_Object *et_obj = NULL;
    const char *dtype = NULL;
    g_cdd_last_alloc_err = cdd_json_value_get_object(tval, &et_obj);
    g_cdd_last_alloc_err = cdd_json_object_get_string(et_obj, "type", &dtype);
    if (!dtype)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    *out_type = ARAZZO_CRITERION_EXPRESSION_TYPE;
    if (strcmp(dtype, "jsonpath") == 0)
      out_et->type = ARAZZO_DIALECT_JSONPATH;
    else if (strcmp(dtype, "xpath") == 0)
      out_et->type = ARAZZO_DIALECT_XPATH;
    else if (strcmp(dtype, "jsonpointer") == 0)
      out_et->type = ARAZZO_DIALECT_JSONPOINTER;
    else
      return CDD_C_ERROR_INVALID_ARGUMENT;

    rc = dup_opt_json_str(et_obj, "version", &out_et->version);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return parse_arazzo_extensions(et_obj, &out_et->extensions);
  }

  return CDD_C_ERROR_INVALID_ARGUMENT;
}

cdd_c_error_t
Arazzo_Criterion_from_jsonObject(const JSON_Object *cobj,
                                 struct Arazzo_Criterion *out_crit) {
  const char *cond = NULL;
  cdd_c_error_t rc;
  g_cdd_last_alloc_err = cdd_json_object_get_string(cobj, "condition", &cond);
  if (!cobj || !out_crit || !cond)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = arazzo_parse_reflected_strings(cobj, &Arazzo_Criterion_type_info,
                                      out_crit);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_criterion_or_expr_type(cobj, "type", &out_crit->type,
                                    &out_crit->expression_type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_arazzo_extensions(cobj, &out_crit->extensions);
}

/**
 * @brief Parses a `JSON_Array` of Criterion Objects.
 *
 * @param[in] arr Source JSON array (may be NULL).
 * @param[out] out_arr Output allocated array of `Arazzo_Criterion`.
 * @param[out] out_count Output count of criteria.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t parse_criteria_array(const JSON_Array *arr,
                                          struct Arazzo_Criterion **out_arr,
                                          size_t *out_count) {
  size_t i;
  size_t n = 0;
  struct Arazzo_Criterion *items;
  cdd_c_error_t rc;

  if (arr)
    g_cdd_last_alloc_err = cdd_json_array_get_count(arr, &n);
  if (n == 0)
    return CDD_C_SUCCESS;

  items = (struct Arazzo_Criterion *)C_CDD_CALLOC(
      n, sizeof(struct Arazzo_Criterion));
  if (!items)
    return CDD_C_ERROR_MEMORY;
  *out_arr = items;
  *out_count = n;

  for (i = 0; i < n; ++i) {
    JSON_Object *elem_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_array_get_object(arr, i, &elem_obj);
    rc = Arazzo_Criterion_from_jsonObject(elem_obj, &items[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a `Selector Object` from `sobj` into `sel`.
 *
 * @param[in] sobj Source JSON object.
 * @param[out] sel Destination selector structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t parse_selector_obj(const JSON_Object *sobj,
                                        struct Arazzo_Selector *sel) {
  cdd_c_error_t rc;
  rc = arazzo_parse_reflected_strings(sobj, &Arazzo_Selector_type_info, sel);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_criterion_or_expr_type(sobj, "type", &sel->type,
                                    &sel->expression_type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_arazzo_extensions(sobj, &sel->extensions);
}

/**
 * @brief Parses a single `Arazzo_Parameter` (or Reusable Parameter Object) from
 * `pobj`.
 *
 * @param[in] pobj Source JSON object.
 * @param[out] param Destination parameter structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t parse_arazzo_param_obj(const JSON_Object *pobj,
                                            struct Arazzo_Parameter *param) {
  const char *ref_str = NULL;
  const char *in_str = NULL;
  const char *name_str = NULL;
  const char *sel_str = NULL;
  JSON_Value *vval = NULL;
  JSON_Object *vobj = NULL;
  cdd_c_error_t rc;

  if (!pobj)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err =
      cdd_json_object_get_string(pobj, "reference", &ref_str);
  if (ref_str) {
    param->reusable.is_reusable = 1;
    rc = arazzo_parse_reflected_strings(pobj, &Arazzo_Reusable_type_info,
                                        &param->reusable);
    if (rc != CDD_C_SUCCESS)
      return rc;
    g_cdd_last_alloc_err = cdd_json_object_get_value(pobj, "value", &vval);
    if (vval) {
      param->reusable.has_value_override = 1;
      rc = parse_any_value(vval, &param->reusable.value);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }

  g_cdd_last_alloc_err = cdd_json_object_get_string(pobj, "name", &name_str);
  g_cdd_last_alloc_err = cdd_json_object_get_value(pobj, "value", &vval);
  if (!name_str || !vval)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = dup_opt_json_str(pobj, "name", &param->name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  g_cdd_last_alloc_err = cdd_json_object_get_string(pobj, "in", &in_str);
  if (in_str) {
    rc = arazzo_param_in_from_str(in_str, &param->in);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  g_cdd_last_alloc_err = cdd_json_value_get_object(vval, &vobj);
  g_cdd_last_alloc_err = cdd_json_object_get_string(vobj, "selector", &sel_str);
  if (sel_str) {
    param->is_selector = 1;
    rc = parse_selector_obj(vobj, &param->selector);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    rc = parse_any_value(vval, &param->value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (param->value.type == OA_ANY_STRING) {
      rc = arazzo_runtime_expr_parse(param->value.string, &param->parsed_expr);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return parse_arazzo_extensions(pobj, &param->extensions);
}

/**
 * @brief Parses a `JSON_Array` of Parameter Objects.
 *
 * @param[in] arr Source JSON array (may be NULL).
 * @param[out] out_arr Output allocated array of `Arazzo_Parameter`.
 * @param[out] out_count Output count of parameters.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
parse_arazzo_params_array(const JSON_Array *arr,
                          struct Arazzo_Parameter **out_arr,
                          size_t *out_count) {
  size_t i;
  size_t n = 0;
  struct Arazzo_Parameter *items;
  cdd_c_error_t rc;

  if (arr)
    g_cdd_last_alloc_err = cdd_json_array_get_count(arr, &n);
  if (n == 0)
    return CDD_C_SUCCESS;

  items = (struct Arazzo_Parameter *)C_CDD_CALLOC(
      n, sizeof(struct Arazzo_Parameter));
  if (!items)
    return CDD_C_ERROR_MEMORY;
  *out_arr = items;
  *out_count = n;

  for (i = 0; i < n; ++i) {
    JSON_Object *elem_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_array_get_object(arr, i, &elem_obj);
    rc = parse_arazzo_param_obj(elem_obj, &items[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a single `Arazzo_SuccessAction` (or Reusable reference).
 *
 * @param[in] aobj Source JSON object.
 * @param[out] act Destination success action structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
parse_success_action_object(const JSON_Object *aobj,
                            struct Arazzo_SuccessAction *act) {
  const char *ref_str = NULL;
  const char *type_str = NULL;
  const char *name_str = NULL;
  JSON_Array *params_arr = NULL;
  JSON_Array *crit_arr = NULL;
  cdd_c_error_t rc;

  if (!aobj)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err =
      cdd_json_object_get_string(aobj, "reference", &ref_str);
  if (ref_str) {
    act->reusable.is_reusable = 1;
    return arazzo_parse_reflected_strings(aobj, &Arazzo_Reusable_type_info,
                                          &act->reusable);
  }

  g_cdd_last_alloc_err = cdd_json_object_get_string(aobj, "type", &type_str);
  g_cdd_last_alloc_err = cdd_json_object_get_string(aobj, "name", &name_str);
  if (!name_str || !type_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_action_type_from_str(type_str, &act->type);
  if (rc != CDD_C_SUCCESS || act->type == ARAZZO_ACTION_RETRY)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_parse_reflected_strings(aobj, &Arazzo_SuccessAction_type_info,
                                      act);
  if (rc != CDD_C_SUCCESS)
    return rc;
  g_cdd_last_alloc_err =
      cdd_json_object_get_array(aobj, "parameters", &params_arr);
  rc = parse_arazzo_params_array(params_arr, &act->parameters,
                                 &act->n_parameters);
  if (rc != CDD_C_SUCCESS)
    return rc;
  g_cdd_last_alloc_err = cdd_json_object_get_array(aobj, "criteria", &crit_arr);
  rc = parse_criteria_array(crit_arr, &act->criteria, &act->n_criteria);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_arazzo_extensions(aobj, &act->extensions);
}

/**
 * @brief Parses a single `Arazzo_FailureAction` (or Reusable reference).
 *
 * @param[in] aobj Source JSON object.
 * @param[out] act Destination failure action structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
parse_failure_action_object(const JSON_Object *aobj,
                            struct Arazzo_FailureAction *act) {
  const char *ref_str = NULL;
  const char *type_str = NULL;
  const char *name_str = NULL;
  JSON_Array *params_arr = NULL;
  JSON_Array *crit_arr = NULL;
  int hv = 0;
  double dv = 0.0;
  cdd_c_error_t rc;

  if (!aobj)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err =
      cdd_json_object_get_string(aobj, "reference", &ref_str);
  if (ref_str) {
    act->reusable.is_reusable = 1;
    return arazzo_parse_reflected_strings(aobj, &Arazzo_Reusable_type_info,
                                          &act->reusable);
  }

  g_cdd_last_alloc_err = cdd_json_object_get_string(aobj, "type", &type_str);
  g_cdd_last_alloc_err = cdd_json_object_get_string(aobj, "name", &name_str);
  if (!name_str || !type_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_action_type_from_str(type_str, &act->type);
  if (rc != CDD_C_SUCCESS)
    return rc;

  g_cdd_last_alloc_err =
      cdd_json_object_has_value_of_type(aobj, "retryAfter", JSONNumber, &hv);
  if (hv) {
    act->has_retry_after = 1;
    g_cdd_last_alloc_err =
        cdd_json_object_get_number(aobj, "retryAfter", &act->retry_after);
  }
  hv = 0;
  g_cdd_last_alloc_err =
      cdd_json_object_has_value_of_type(aobj, "retryLimit", JSONNumber, &hv);
  if (hv) {
    act->has_retry_limit = 1;
    g_cdd_last_alloc_err = cdd_json_object_get_number(aobj, "retryLimit", &dv);
    act->retry_limit = (int)dv;
  }

  rc = arazzo_parse_reflected_strings(aobj, &Arazzo_FailureAction_type_info,
                                      act);
  if (rc != CDD_C_SUCCESS)
    return rc;
  g_cdd_last_alloc_err =
      cdd_json_object_get_array(aobj, "parameters", &params_arr);
  rc = parse_arazzo_params_array(params_arr, &act->parameters,
                                 &act->n_parameters);
  if (rc != CDD_C_SUCCESS)
    return rc;
  g_cdd_last_alloc_err = cdd_json_object_get_array(aobj, "criteria", &crit_arr);
  rc = parse_criteria_array(crit_arr, &act->criteria, &act->n_criteria);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_arazzo_extensions(aobj, &act->extensions);
}

/**
 * @brief Parses an `outputs` JSON object map (`key` -> expression or `Selector
 * Object`).
 *
 * @param[in] out_obj Source JSON object (may be NULL).
 * @param[out] out_arr Output allocated array of `Arazzo_OutputMapEntry`.
 * @param[out] out_count Output count of entries.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t parse_outputs_map(const JSON_Object *out_obj,
                                       struct Arazzo_OutputMapEntry **out_arr,
                                       size_t *out_count) {
  size_t i;
  size_t n = 0;
  struct Arazzo_OutputMapEntry *items;
  cdd_c_error_t rc;

  if (out_obj)
    g_cdd_last_alloc_err = cdd_json_object_get_count(out_obj, &n);
  if (n == 0)
    return CDD_C_SUCCESS;

  items = (struct Arazzo_OutputMapEntry *)C_CDD_CALLOC(
      n, sizeof(struct Arazzo_OutputMapEntry));
  if (!items)
    return CDD_C_ERROR_MEMORY;
  *out_arr = items;
  *out_count = n;

  for (i = 0; i < n; ++i) {
    const char *k = NULL;
    JSON_Value *v = NULL;
    JSON_Value_Type vtype = JSONError;
    g_cdd_last_alloc_err = cdd_json_object_get_name(out_obj, i, &k);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(out_obj, i, &v);
    items[i].key = C_CDD_STRDUP(k);
    if (!items[i].key)
      return CDD_C_ERROR_MEMORY;

    g_cdd_last_alloc_err = cdd_json_value_get_type(v, &vtype);
    if (vtype == JSONString) {
      const char *vs = NULL;
      g_cdd_last_alloc_err = cdd_json_value_get_string(v, &vs);
      items[i].expression_or_value = C_CDD_STRDUP(vs);
      if (!items[i].expression_or_value)
        return CDD_C_ERROR_MEMORY;
      rc = arazzo_runtime_expr_parse(items[i].expression_or_value,
                                     &items[i].parsed_expr);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (vtype == JSONObject) {
      JSON_Object *vobj = NULL;
      items[i].is_selector = 1;
      g_cdd_last_alloc_err = cdd_json_value_get_object(v, &vobj);
      rc = parse_selector_obj(vobj, &items[i].selector);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a single `Step Object` from `sobj`.
 *
 * @param[in] sobj Source JSON object.
 * @param[out] step Destination step structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t parse_step_object(const JSON_Object *sobj,
                                       struct Arazzo_Step *step) {
  int target_count = 0;
  JSON_Object *rb_obj = NULL;
  JSON_Object *out_obj = NULL;
  JSON_Array *dep_arr = NULL;
  JSON_Array *params_arr = NULL;
  JSON_Array *scrit_arr = NULL;
  JSON_Array *succ_arr = NULL;
  JSON_Array *fail_arr = NULL;
  const char *step_id_str = NULL;
  const char *act_str = NULL;
  int hv = 0;
  double dv = 0.0;
  size_t i;
  size_t scount = 0;
  size_t fcount = 0;
  cdd_c_error_t rc;

  g_cdd_last_alloc_err =
      cdd_json_object_get_string(sobj, "stepId", &step_id_str);
  if (!step_id_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_parse_reflected_strings(sobj, &Arazzo_Step_type_info, step);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (step->operation_id) {
    step->target_kind = ARAZZO_STEP_TARGET_OPERATION_ID;
    ++target_count;
  }
  if (step->operation_path) {
    step->target_kind = ARAZZO_STEP_TARGET_OPERATION_PATH;
    ++target_count;
  }
  if (step->channel_path) {
    step->target_kind = ARAZZO_STEP_TARGET_CHANNEL_PATH;
    ++target_count;
  }
  if (step->workflow_id) {
    step->target_kind = ARAZZO_STEP_TARGET_WORKFLOW_ID;
    ++target_count;
  }
  if (target_count != 1)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err =
      cdd_json_object_has_value_of_type(sobj, "timeout", JSONNumber, &hv);
  if (hv) {
    step->has_timeout = 1;
    g_cdd_last_alloc_err = cdd_json_object_get_number(sobj, "timeout", &dv);
    step->timeout_ms = (int)dv;
  }

  g_cdd_last_alloc_err = cdd_json_object_get_string(sobj, "action", &act_str);
  if (act_str) {
    if (strcmp(act_str, "send") == 0)
      step->action = ARAZZO_ASYNC_SEND;
    else if (strcmp(act_str, "receive") == 0)
      step->action = ARAZZO_ASYNC_RECEIVE;
    else
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  g_cdd_last_alloc_err = cdd_json_object_get_array(sobj, "dependsOn", &dep_arr);
  if (dep_arr) {
    rc = parse_string_enum_array(dep_arr, &step->depends_on,
                                 &step->n_depends_on);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  g_cdd_last_alloc_err =
      cdd_json_object_get_array(sobj, "parameters", &params_arr);
  rc = parse_arazzo_params_array(params_arr, &step->parameters,
                                 &step->n_parameters);
  if (rc != CDD_C_SUCCESS)
    return rc;

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(sobj, "requestBody", &rb_obj);
  if (rb_obj) {
    JSON_Array *rep_arr = NULL;
    JSON_Value *pay_val = NULL;
    size_t rcount = 0;
    g_cdd_last_alloc_err =
        cdd_json_object_get_array(rb_obj, "replacements", &rep_arr);
    g_cdd_last_alloc_err = cdd_json_array_get_count(rep_arr, &rcount);
    step->has_request_body = 1;
    rc = dup_opt_json_str(rb_obj, "contentType",
                          &step->request_body.content_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    g_cdd_last_alloc_err =
        cdd_json_object_get_value(rb_obj, "payload", &pay_val);
    if (pay_val) {
      rc = parse_any_value(pay_val, &step->request_body.payload);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (rcount > 0) {
      step->request_body.replacements =
          (struct Arazzo_PayloadReplacement *)C_CDD_CALLOC(
              rcount, sizeof(struct Arazzo_PayloadReplacement));
      if (!step->request_body.replacements)
        return CDD_C_ERROR_MEMORY;
      step->request_body.n_replacements = rcount;
      for (i = 0; i < rcount; ++i) {
        JSON_Object *robj = NULL;
        const char *tgt_str = NULL;
        JSON_Value *rval = NULL;
        struct Arazzo_PayloadReplacement *rep =
            &step->request_body.replacements[i];
        g_cdd_last_alloc_err = cdd_json_array_get_object(rep_arr, i, &robj);
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(robj, "target", &tgt_str);
        g_cdd_last_alloc_err = cdd_json_object_get_value(robj, "value", &rval);
        if (!tgt_str || !rval)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        rc = dup_opt_json_str(robj, "target", &rep->target);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_criterion_or_expr_type(robj, "targetSelectorType",
                                          &rep->target_selector_type,
                                          &rep->target_expression_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_any_value(rval, &rep->value);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_arazzo_extensions(robj, &rep->extensions);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    rc = parse_arazzo_extensions(rb_obj, &step->request_body.extensions);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  g_cdd_last_alloc_err =
      cdd_json_object_get_array(sobj, "successCriteria", &scrit_arr);
  rc = parse_criteria_array(scrit_arr, &step->success_criteria,
                            &step->n_success_criteria);
  if (rc != CDD_C_SUCCESS)
    return rc;

  g_cdd_last_alloc_err =
      cdd_json_object_get_array(sobj, "onSuccess", &succ_arr);
  g_cdd_last_alloc_err = cdd_json_array_get_count(succ_arr, &scount);
  if (scount > 0) {
    step->on_success = (struct Arazzo_SuccessAction *)C_CDD_CALLOC(
        scount, sizeof(struct Arazzo_SuccessAction));
    if (!step->on_success)
      return CDD_C_ERROR_MEMORY;
    step->n_on_success = scount;
    for (i = 0; i < scount; ++i) {
      JSON_Object *elem_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_array_get_object(succ_arr, i, &elem_obj);
      rc = parse_success_action_object(elem_obj, &step->on_success[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  g_cdd_last_alloc_err =
      cdd_json_object_get_array(sobj, "onFailure", &fail_arr);
  g_cdd_last_alloc_err = cdd_json_array_get_count(fail_arr, &fcount);
  if (fcount > 0) {
    step->on_failure = (struct Arazzo_FailureAction *)C_CDD_CALLOC(
        fcount, sizeof(struct Arazzo_FailureAction));
    if (!step->on_failure)
      return CDD_C_ERROR_MEMORY;
    step->n_on_failure = fcount;
    for (i = 0; i < fcount; ++i) {
      JSON_Object *elem_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_array_get_object(fail_arr, i, &elem_obj);
      rc = parse_failure_action_object(elem_obj, &step->on_failure[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  g_cdd_last_alloc_err = cdd_json_object_get_object(sobj, "outputs", &out_obj);
  rc = parse_outputs_map(out_obj, &step->outputs, &step->n_outputs);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_arazzo_extensions(sobj, &step->extensions);
}

/**
 * @brief Parses a single `Source Description Object` from `sd_obj`.
 *
 * @param[in] sd_obj Source JSON object.
 * @param[out] out_sd Destination source description structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
cdd_c_error_t Arazzo_SourceDescription_from_jsonObject(
    const JSON_Object *sd_obj, struct Arazzo_SourceDescription *out_sd) {
  const char *tstr = NULL;
  const char *name_str = NULL;
  const char *url_str = NULL;
  cdd_c_error_t rc;

  g_cdd_last_alloc_err = cdd_json_object_get_string(sd_obj, "type", &tstr);
  g_cdd_last_alloc_err = cdd_json_object_get_string(sd_obj, "name", &name_str);
  g_cdd_last_alloc_err = cdd_json_object_get_string(sd_obj, "url", &url_str);
  if (!sd_obj || !out_sd || !name_str || !url_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (tstr) {
    rc = arazzo_source_type_from_str(tstr, &out_sd->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    out_sd->type = ARAZZO_SOURCE_OPENAPI;
  }

  rc = arazzo_parse_reflected_strings(
      sd_obj, &Arazzo_SourceDescription_type_info, out_sd);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_arazzo_extensions(sd_obj, &out_sd->extensions);
}

cdd_c_error_t Arazzo_Info_from_jsonObject(const JSON_Object *info_obj,
                                          struct Arazzo_Info *out_info) {
  cdd_c_error_t rc;
  if (!info_obj || !out_info)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = arazzo_parse_reflected_strings(info_obj, &Arazzo_Info_type_info,
                                      out_info);
  if (rc == CDD_C_SUCCESS)
    rc = parse_arazzo_extensions(info_obj, &out_info->extensions);
  return rc;
}

/**
 * @brief Parses a single `Workflow Object` from `wobj`.
 *
 * @param[in] wobj Source JSON object.
 * @param[out] wf Destination workflow structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t parse_workflow_object(const JSON_Object *wobj,
                                           struct Arazzo_Workflow *wf) {
  const char *wf_id_str = NULL;
  JSON_Object *in_obj = NULL;
  JSON_Object *out_obj = NULL;
  JSON_Array *dep_arr = NULL;
  JSON_Array *params_arr = NULL;
  JSON_Array *steps_arr = NULL;
  JSON_Array *succ_arr = NULL;
  JSON_Array *fail_arr = NULL;
  size_t i;
  size_t n_steps = 0;
  size_t sc = 0;
  size_t fc = 0;
  cdd_c_error_t rc;

  g_cdd_last_alloc_err =
      cdd_json_object_get_string(wobj, "workflowId", &wf_id_str);
  if (!wf_id_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_array(wobj, "steps", &steps_arr);
  g_cdd_last_alloc_err = cdd_json_array_get_count(steps_arr, &n_steps);
  if (n_steps == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_parse_reflected_strings(wobj, &Arazzo_Workflow_type_info, wf);
  if (rc != CDD_C_SUCCESS)
    return rc;

  g_cdd_last_alloc_err = cdd_json_object_get_object(wobj, "inputs", &in_obj);
  if (in_obj) {
    wf->has_inputs = 1;
    rc = parse_schema_ref(in_obj, &wf->inputs, NULL);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  g_cdd_last_alloc_err = cdd_json_object_get_array(wobj, "dependsOn", &dep_arr);
  if (dep_arr) {
    rc = parse_string_enum_array(dep_arr, &wf->depends_on, &wf->n_depends_on);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  wf->steps =
      (struct Arazzo_Step *)C_CDD_CALLOC(n_steps, sizeof(struct Arazzo_Step));
  if (!wf->steps)
    return CDD_C_ERROR_MEMORY;
  wf->n_steps = n_steps;

  for (i = 0; i < n_steps; ++i) {
    JSON_Object *st_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_array_get_object(steps_arr, i, &st_obj);
    rc = parse_step_object(st_obj, &wf->steps[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  g_cdd_last_alloc_err =
      cdd_json_object_get_array(wobj, "successActions", &succ_arr);
  g_cdd_last_alloc_err = cdd_json_array_get_count(succ_arr, &sc);
  if (sc > 0) {
    wf->success_actions = (struct Arazzo_SuccessAction *)C_CDD_CALLOC(
        sc, sizeof(struct Arazzo_SuccessAction));
    if (!wf->success_actions)
      return CDD_C_ERROR_MEMORY;
    wf->n_success_actions = sc;
    for (i = 0; i < sc; ++i) {
      JSON_Object *elem_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_array_get_object(succ_arr, i, &elem_obj);
      rc = parse_success_action_object(elem_obj, &wf->success_actions[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  g_cdd_last_alloc_err =
      cdd_json_object_get_array(wobj, "failureActions", &fail_arr);
  g_cdd_last_alloc_err = cdd_json_array_get_count(fail_arr, &fc);
  if (fc > 0) {
    wf->failure_actions = (struct Arazzo_FailureAction *)C_CDD_CALLOC(
        fc, sizeof(struct Arazzo_FailureAction));
    if (!wf->failure_actions)
      return CDD_C_ERROR_MEMORY;
    wf->n_failure_actions = fc;
    for (i = 0; i < fc; ++i) {
      JSON_Object *elem_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_array_get_object(fail_arr, i, &elem_obj);
      rc = parse_failure_action_object(elem_obj, &wf->failure_actions[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  g_cdd_last_alloc_err = cdd_json_object_get_object(wobj, "outputs", &out_obj);
  rc = parse_outputs_map(out_obj, &wf->outputs, &wf->n_outputs);
  if (rc != CDD_C_SUCCESS)
    return rc;
  g_cdd_last_alloc_err =
      cdd_json_object_get_array(wobj, "parameters", &params_arr);
  rc =
      parse_arazzo_params_array(params_arr, &wf->parameters, &wf->n_parameters);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_arazzo_extensions(wobj, &wf->extensions);
}

/**
 * @brief Parses the root `components` object into `spec->components`.
 *
 * @param[in] cobj Source `components` JSON object.
 * @param[in,out] comp Destination `Arazzo_Components` structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t parse_components_object(const JSON_Object *cobj,
                                             struct Arazzo_Components *comp) {
  JSON_Object *in_obj = NULL;
  JSON_Object *pm_obj = NULL;
  JSON_Object *sa_obj = NULL;
  JSON_Object *fa_obj = NULL;
  size_t i;
  size_t n = 0;
  cdd_c_error_t rc;

  g_cdd_last_alloc_err = cdd_json_object_get_object(cobj, "inputs", &in_obj);
  g_cdd_last_alloc_err =
      cdd_json_object_get_object(cobj, "parameters", &pm_obj);
  g_cdd_last_alloc_err =
      cdd_json_object_get_object(cobj, "successActions", &sa_obj);
  g_cdd_last_alloc_err =
      cdd_json_object_get_object(cobj, "failureActions", &fa_obj);

  g_cdd_last_alloc_err = cdd_json_object_get_count(in_obj, &n);
  if (n > 0) {
    comp->input_keys = (char **)C_CDD_CALLOC(n, sizeof(char *));
    if (!comp->input_keys)
      return CDD_C_ERROR_MEMORY;
    comp->inputs = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        n, sizeof(struct OpenAPI_SchemaRef));
    if (!comp->inputs)
      return CDD_C_ERROR_MEMORY;
    comp->n_inputs = n;
    for (i = 0; i < n; ++i) {
      JSON_Value *val_at = NULL;
      JSON_Object *sch_obj = NULL;
      const char *kname = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_value_at(in_obj, i, &val_at);
      g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &sch_obj);
      if (!sch_obj)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      g_cdd_last_alloc_err = cdd_json_object_get_name(in_obj, i, &kname);
      comp->input_keys[i] = C_CDD_STRDUP(kname);
      if (!comp->input_keys[i])
        return CDD_C_ERROR_MEMORY;
      rc = parse_schema_ref(sch_obj, &comp->inputs[i], NULL);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  n = 0;
  g_cdd_last_alloc_err = cdd_json_object_get_count(pm_obj, &n);
  if (n > 0) {
    comp->parameter_keys = (char **)C_CDD_CALLOC(n, sizeof(char *));
    if (!comp->parameter_keys)
      return CDD_C_ERROR_MEMORY;
    comp->parameters = (struct Arazzo_Parameter *)C_CDD_CALLOC(
        n, sizeof(struct Arazzo_Parameter));
    if (!comp->parameters)
      return CDD_C_ERROR_MEMORY;
    comp->n_parameters = n;
    for (i = 0; i < n; ++i) {
      JSON_Value *val_at = NULL;
      JSON_Object *p_obj = NULL;
      const char *kname = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_name(pm_obj, i, &kname);
      comp->parameter_keys[i] = C_CDD_STRDUP(kname);
      if (!comp->parameter_keys[i])
        return CDD_C_ERROR_MEMORY;
      g_cdd_last_alloc_err = cdd_json_object_get_value_at(pm_obj, i, &val_at);
      g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &p_obj);
      rc = parse_arazzo_param_obj(p_obj, &comp->parameters[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  n = 0;
  g_cdd_last_alloc_err = cdd_json_object_get_count(sa_obj, &n);
  if (n > 0) {
    comp->success_action_keys = (char **)C_CDD_CALLOC(n, sizeof(char *));
    if (!comp->success_action_keys)
      return CDD_C_ERROR_MEMORY;
    comp->success_actions = (struct Arazzo_SuccessAction *)C_CDD_CALLOC(
        n, sizeof(struct Arazzo_SuccessAction));
    if (!comp->success_actions)
      return CDD_C_ERROR_MEMORY;
    comp->n_success_actions = n;
    for (i = 0; i < n; ++i) {
      JSON_Value *val_at = NULL;
      JSON_Object *s_obj = NULL;
      const char *kname = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_name(sa_obj, i, &kname);
      comp->success_action_keys[i] = C_CDD_STRDUP(kname);
      if (!comp->success_action_keys[i])
        return CDD_C_ERROR_MEMORY;
      g_cdd_last_alloc_err = cdd_json_object_get_value_at(sa_obj, i, &val_at);
      g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &s_obj);
      rc = parse_success_action_object(s_obj, &comp->success_actions[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  n = 0;
  g_cdd_last_alloc_err = cdd_json_object_get_count(fa_obj, &n);
  if (n > 0) {
    comp->failure_action_keys = (char **)C_CDD_CALLOC(n, sizeof(char *));
    if (!comp->failure_action_keys)
      return CDD_C_ERROR_MEMORY;
    comp->failure_actions = (struct Arazzo_FailureAction *)C_CDD_CALLOC(
        n, sizeof(struct Arazzo_FailureAction));
    if (!comp->failure_actions)
      return CDD_C_ERROR_MEMORY;
    comp->n_failure_actions = n;
    for (i = 0; i < n; ++i) {
      JSON_Value *val_at = NULL;
      JSON_Object *f_obj = NULL;
      const char *kname = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_name(fa_obj, i, &kname);
      comp->failure_action_keys[i] = C_CDD_STRDUP(kname);
      if (!comp->failure_action_keys[i])
        return CDD_C_ERROR_MEMORY;
      g_cdd_last_alloc_err = cdd_json_object_get_value_at(fa_obj, i, &val_at);
      g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &f_obj);
      rc = parse_failure_action_object(f_obj, &comp->failure_actions[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return parse_arazzo_extensions(cobj, &comp->extensions);
}

/**
 * @brief Validates a `$components.<section>.<key>` reusable reference against
 * a component key array.
 *
 * @param[in] ref Reusable reference string.
 * @param[in] prefix Expected prefix (e.g., `"$components.parameters."`).
 * @param[in] keys Component keys array.
 * @param[in] n_keys Number of component keys.
 * @return `CDD_C_SUCCESS` if found, `CDD_C_ERROR_INVALID_ARGUMENT` otherwise.
 */
static cdd_c_error_t validate_component_ref(const char *ref, const char *prefix,
                                            char *const *keys, size_t n_keys) {
  size_t plen = strlen(prefix);
  size_t m;
  if (strncmp(ref, prefix, plen) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (m = 0; m < n_keys; ++m) {
    if (strcmp(keys[m], ref + plen) == 0)
      return CDD_C_SUCCESS;
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/*
 * @brief Validates structural and semantic invariants of an `Arazzo_Spec`.
 *
 * @param[in] spec Parsed Arazzo specification to validate.
 * @return `CDD_C_SUCCESS` if valid, `CDD_C_ERROR_INVALID_ARGUMENT` otherwise.
 */
cdd_c_error_t arazzo_validate_spec(const struct Arazzo_Spec *spec) {
  size_t i;
  size_t j;
  size_t k;

  if (!spec || !spec->arazzo || !spec->info.title || !spec->info.version ||
      spec->n_source_descriptions == 0 || spec->n_workflows == 0 ||
      (strncmp(spec->arazzo, "1.0.", 4) != 0 &&
       strncmp(spec->arazzo, "1.1.", 4) != 0))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &spec->workflows[i];
    if (!wf->workflow_id || wf->n_steps == 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;

    for (j = i + 1; j < spec->n_workflows; ++j) {
      if (strcmp(wf->workflow_id, spec->workflows[j].workflow_id) == 0)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    for (j = 0; j < wf->n_depends_on; ++j) {
      int found_wf = 0;
      if (strcmp(wf->depends_on[j], wf->workflow_id) == 0)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      for (k = 0; k < spec->n_workflows; ++k) {
        if (strcmp(spec->workflows[k].workflow_id, wf->depends_on[j]) == 0) {
          found_wf = 1;
          break;
        }
      }
      if (!found_wf)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    for (j = 0; j < wf->n_steps; ++j) {
      const struct Arazzo_Step *st = &wf->steps[j];
      if (!st->step_id || st->target_kind == ARAZZO_STEP_TARGET_NONE)
        return CDD_C_ERROR_INVALID_ARGUMENT;

      for (k = j + 1; k < wf->n_steps; ++k) {
        if (strcmp(st->step_id, wf->steps[k].step_id) == 0)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }

      for (k = 0; k < st->n_depends_on; ++k) {
        size_t m;
        int found_step = 0;
        if (strcmp(st->depends_on[k], st->step_id) == 0)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        for (m = 0; m < wf->n_steps; ++m) {
          if (strcmp(wf->steps[m].step_id, st->depends_on[k]) == 0) {
            found_step = 1;
            break;
          }
        }
        if (!found_step)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }

      for (k = 0; k < st->n_parameters; ++k) {
        if (st->parameters[k].reusable.is_reusable &&
            validate_component_ref(
                st->parameters[k].reusable.reference, "$components.parameters.",
                spec->components.parameter_keys,
                spec->components.n_parameters) != CDD_C_SUCCESS)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }

      for (k = 0; k < st->n_on_success; ++k) {
        if (st->on_success[k].reusable.is_reusable &&
            validate_component_ref(st->on_success[k].reusable.reference,
                                   "$components.successActions.",
                                   spec->components.success_action_keys,
                                   spec->components.n_success_actions) !=
                CDD_C_SUCCESS)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }

      for (k = 0; k < st->n_on_failure; ++k) {
        if (st->on_failure[k].reusable.is_reusable &&
            validate_component_ref(st->on_failure[k].reusable.reference,
                                   "$components.failureActions.",
                                   spec->components.failure_action_keys,
                                   spec->components.n_failure_actions) !=
                CDD_C_SUCCESS)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Parses an Arazzo v1.1.0 specification from a Parson `JSON_Value`.
 *
 * @param[in] root Root Parson JSON value representing the Arazzo document.
 * @param[out] out_spec Output Arazzo specification structure to populate.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_load_from_json_value(const JSON_Value *root,
                                          struct Arazzo_Spec *out_spec) {
  JSON_Object *root_obj = NULL;
  JSON_Object *info_obj = NULL;
  JSON_Array *sd_arr = NULL;
  JSON_Array *wf_arr = NULL;
  JSON_Object *comp_obj = NULL;
  const char *ver_str = NULL;
  size_t i;
  size_t n_sd = 0;
  size_t n_wf = 0;
  cdd_c_error_t rc;

  g_cdd_last_alloc_err = cdd_json_value_get_object(root, &root_obj);
  if (!root_obj || !out_spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  g_cdd_last_alloc_err =
      cdd_json_object_get_string(root_obj, "arazzo", &ver_str);
  g_cdd_last_alloc_err =
      cdd_json_object_get_object(root_obj, "info", &info_obj);
  g_cdd_last_alloc_err =
      cdd_json_object_get_array(root_obj, "sourceDescriptions", &sd_arr);
  g_cdd_last_alloc_err =
      cdd_json_object_get_array(root_obj, "workflows", &wf_arr);

  g_cdd_last_alloc_err = cdd_json_array_get_count(sd_arr, &n_sd);
  g_cdd_last_alloc_err = cdd_json_array_get_count(wf_arr, &n_wf);
  if (!ver_str || !info_obj || n_sd == 0 || n_wf == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(out_spec, 0, sizeof(*out_spec));
  out_spec->arazzo = C_CDD_STRDUP(ver_str);
  if (!out_spec->arazzo)
    return CDD_C_ERROR_MEMORY;

  if (strcmp(ver_str, "1.0.0") == 0)
    out_spec->version = ARAZZO_VER_1_0_0;
  else if (strcmp(ver_str, "1.0.1") == 0)
    out_spec->version = ARAZZO_VER_1_0_1;
  else if (strncmp(ver_str, "1.1.", 4) == 0)
    out_spec->version = ARAZZO_VER_1_1_0;
  else {
    g_cdd_last_alloc_err = arazzo_spec_free(out_spec);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = dup_opt_json_str(root_obj, "$self", &out_spec->self_uri);
  if (rc == CDD_C_SUCCESS)
    rc = Arazzo_Info_from_jsonObject(info_obj, &out_spec->info);
  if (rc == CDD_C_SUCCESS) {
    out_spec->source_descriptions =
        (struct Arazzo_SourceDescription *)C_CDD_CALLOC(
            n_sd, sizeof(struct Arazzo_SourceDescription));
    if (!out_spec->source_descriptions)
      rc = CDD_C_ERROR_MEMORY;
  }
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = arazzo_spec_free(out_spec);
    return rc;
  }
  out_spec->n_source_descriptions = n_sd;
  for (i = 0; i < n_sd; ++i) {
    JSON_Object *elem_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_array_get_object(sd_arr, i, &elem_obj);
    rc = Arazzo_SourceDescription_from_jsonObject(
        elem_obj, &out_spec->source_descriptions[i]);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = arazzo_spec_free(out_spec);
      return rc;
    }
  }

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(root_obj, "components", &comp_obj);
  if (comp_obj) {
    out_spec->has_components = 1;
    rc = parse_components_object(comp_obj, &out_spec->components);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = arazzo_spec_free(out_spec);
      return rc;
    }
  }

  out_spec->workflows = (struct Arazzo_Workflow *)C_CDD_CALLOC(
      n_wf, sizeof(struct Arazzo_Workflow));
  if (!out_spec->workflows) {
    g_cdd_last_alloc_err = arazzo_spec_free(out_spec);
    return CDD_C_ERROR_MEMORY;
  }
  out_spec->n_workflows = n_wf;
  for (i = 0; i < n_wf; ++i) {
    JSON_Object *elem_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_array_get_object(wf_arr, i, &elem_obj);
    rc = parse_workflow_object(elem_obj, &out_spec->workflows[i]);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = arazzo_spec_free(out_spec);
      return rc;
    }
  }

  rc = parse_arazzo_extensions(root_obj, &out_spec->extensions);
  if (rc == CDD_C_SUCCESS)
    rc = arazzo_validate_spec(out_spec);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = arazzo_spec_free(out_spec);
    return rc;
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Parses an Arazzo v1.1.0 specification from a JSON string.
 *
 * @param[in] json_str Raw JSON string containing the Arazzo specification.
 * @param[out] out_spec Output Arazzo specification structure to populate.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_load_from_json(const char *json_str,
                                    struct Arazzo_Spec *out_spec) {
  JSON_Value *root = NULL;
  cdd_c_error_t rc;

  if (!json_str || !out_spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (cdd_json_parse_string(json_str, &root) != CDD_C_SUCCESS)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_load_from_json_value(root, out_spec);
  g_cdd_last_alloc_err = cdd_json_value_free(root);
  return rc;
}
