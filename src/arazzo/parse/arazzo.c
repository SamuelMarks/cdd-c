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
  size_t count = json_object_get_count(obj);

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(obj, i);
    if (strncmp(name, "x-", 2) == 0) {
      struct OpenAPI_Any val;
      cdd_c_error_t rc;
      memset(&val, 0, sizeof(val));
      rc = parse_any_value(json_object_get_value_at(obj, i), &val);
      if (rc == CDD_C_SUCCESS)
        rc = arazzo_extension_list_append(ext_list, name, &val);
      free_any_value(&val);
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
  const char *s = json_object_get_string(obj, key);
  if (!s)
    return CDD_C_SUCCESS;
  *out_str = C_CDD_STRDUP(s);
  return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
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
  const JSON_Value *tval = json_object_get_value(obj, type_key);
  cdd_c_error_t rc;

  if (!tval) {
    *out_type = ARAZZO_CRITERION_SIMPLE;
    return CDD_C_SUCCESS;
  }

  if (json_value_get_type(tval) == JSONString) {
    const char *tstr = json_value_get_string(tval);
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

  if (json_value_get_type(tval) == JSONObject) {
    const JSON_Object *et_obj = json_value_get_object(tval);
    const char *dtype = json_object_get_string(et_obj, "type");
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
  size_t n = arr ? json_array_get_count(arr) : 0;
  struct Arazzo_Criterion *items;
  cdd_c_error_t rc;

  if (n == 0)
    return CDD_C_SUCCESS;

  items = (struct Arazzo_Criterion *)C_CDD_CALLOC(
      n, sizeof(struct Arazzo_Criterion));
  if (!items)
    return CDD_C_ERROR_MEMORY;
  *out_arr = items;
  *out_count = n;

  for (i = 0; i < n; ++i) {
    const JSON_Object *cobj = json_array_get_object(arr, i);
    if (!json_object_get_string(cobj, "condition"))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = dup_opt_json_str(cobj, "condition", &items[i].condition);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = dup_opt_json_str(cobj, "context", &items[i].context);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_criterion_or_expr_type(cobj, "type", &items[i].type,
                                      &items[i].expression_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_arazzo_extensions(cobj, &items[i].extensions);
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
  rc = dup_opt_json_str(sobj, "context", &sel->context);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(sobj, "selector", &sel->selector);
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
  const char *ref_str;
  const char *in_str;
  const JSON_Value *vval;
  cdd_c_error_t rc;

  if (!pobj)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  ref_str = json_object_get_string(pobj, "reference");
  if (ref_str) {
    param->reusable.is_reusable = 1;
    param->reusable.reference = C_CDD_STRDUP(ref_str);
    if (!param->reusable.reference)
      return CDD_C_ERROR_MEMORY;
    vval = json_object_get_value(pobj, "value");
    if (vval) {
      param->reusable.has_value_override = 1;
      rc = parse_any_value(vval, &param->reusable.value);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }

  if (!json_object_get_string(pobj, "name") ||
      !json_object_get_value(pobj, "value"))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = dup_opt_json_str(pobj, "name", &param->name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  in_str = json_object_get_string(pobj, "in");
  if (in_str) {
    rc = arazzo_param_in_from_str(in_str, &param->in);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  vval = json_object_get_value(pobj, "value");
  if (json_object_get_string(json_value_get_object(vval), "selector")) {
    param->is_selector = 1;
    rc = parse_selector_obj(json_value_get_object(vval), &param->selector);
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
  size_t n = arr ? json_array_get_count(arr) : 0;
  struct Arazzo_Parameter *items;
  cdd_c_error_t rc;

  if (n == 0)
    return CDD_C_SUCCESS;

  items = (struct Arazzo_Parameter *)C_CDD_CALLOC(
      n, sizeof(struct Arazzo_Parameter));
  if (!items)
    return CDD_C_ERROR_MEMORY;
  *out_arr = items;
  *out_count = n;

  for (i = 0; i < n; ++i) {
    rc = parse_arazzo_param_obj(json_array_get_object(arr, i), &items[i]);
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
  const char *ref_str;
  const char *type_str;
  cdd_c_error_t rc;

  if (!aobj)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  ref_str = json_object_get_string(aobj, "reference");
  if (ref_str) {
    act->reusable.is_reusable = 1;
    act->reusable.reference = C_CDD_STRDUP(ref_str);
    return act->reusable.reference ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }

  type_str = json_object_get_string(aobj, "type");
  if (!json_object_get_string(aobj, "name") || !type_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_action_type_from_str(type_str, &act->type);
  if (rc != CDD_C_SUCCESS || act->type == ARAZZO_ACTION_RETRY)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = dup_opt_json_str(aobj, "name", &act->name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(aobj, "workflowId", &act->workflow_id);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(aobj, "stepId", &act->step_id);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_arazzo_params_array(json_object_get_array(aobj, "parameters"),
                                 &act->parameters, &act->n_parameters);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_criteria_array(json_object_get_array(aobj, "criteria"),
                            &act->criteria, &act->n_criteria);
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
  const char *ref_str;
  const char *type_str;
  cdd_c_error_t rc;

  if (!aobj)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  ref_str = json_object_get_string(aobj, "reference");
  if (ref_str) {
    act->reusable.is_reusable = 1;
    act->reusable.reference = C_CDD_STRDUP(ref_str);
    return act->reusable.reference ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }

  type_str = json_object_get_string(aobj, "type");
  if (!json_object_get_string(aobj, "name") || !type_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_action_type_from_str(type_str, &act->type);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (json_object_has_value_of_type(aobj, "retryAfter", JSONNumber)) {
    act->has_retry_after = 1;
    act->retry_after = json_object_get_number(aobj, "retryAfter");
  }
  if (json_object_has_value_of_type(aobj, "retryLimit", JSONNumber)) {
    act->has_retry_limit = 1;
    act->retry_limit = (int)json_object_get_number(aobj, "retryLimit");
  }

  rc = dup_opt_json_str(aobj, "name", &act->name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(aobj, "workflowId", &act->workflow_id);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(aobj, "stepId", &act->step_id);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_arazzo_params_array(json_object_get_array(aobj, "parameters"),
                                 &act->parameters, &act->n_parameters);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_criteria_array(json_object_get_array(aobj, "criteria"),
                            &act->criteria, &act->n_criteria);
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
  size_t n = out_obj ? json_object_get_count(out_obj) : 0;
  struct Arazzo_OutputMapEntry *items;
  cdd_c_error_t rc;

  if (n == 0)
    return CDD_C_SUCCESS;

  items = (struct Arazzo_OutputMapEntry *)C_CDD_CALLOC(
      n, sizeof(struct Arazzo_OutputMapEntry));
  if (!items)
    return CDD_C_ERROR_MEMORY;
  *out_arr = items;
  *out_count = n;

  for (i = 0; i < n; ++i) {
    const char *k = json_object_get_name(out_obj, i);
    const JSON_Value *v = json_object_get_value_at(out_obj, i);
    items[i].key = C_CDD_STRDUP(k);
    if (!items[i].key)
      return CDD_C_ERROR_MEMORY;

    if (json_value_get_type(v) == JSONString) {
      items[i].expression_or_value = C_CDD_STRDUP(json_value_get_string(v));
      if (!items[i].expression_or_value)
        return CDD_C_ERROR_MEMORY;
      rc = arazzo_runtime_expr_parse(items[i].expression_or_value,
                                     &items[i].parsed_expr);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (json_value_get_type(v) == JSONObject) {
      items[i].is_selector = 1;
      rc = parse_selector_obj(json_value_get_object(v), &items[i].selector);
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
  const JSON_Object *rb_obj;
  const JSON_Array *succ_arr;
  const JSON_Array *fail_arr;
  const char *act_str;
  size_t i;
  size_t scount;
  size_t fcount;
  cdd_c_error_t rc;

  if (!json_object_get_string(sobj, "stepId"))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = dup_opt_json_str(sobj, "stepId", &step->step_id);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(sobj, "description", &step->description);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(sobj, "operationId", &step->operation_id);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(sobj, "operationPath", &step->operation_path);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(sobj, "channelPath", &step->channel_path);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(sobj, "workflowId", &step->workflow_id);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(sobj, "correlationId", &step->correlation_id);
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

  if (json_object_has_value_of_type(sobj, "timeout", JSONNumber)) {
    step->has_timeout = 1;
    step->timeout_ms = (int)json_object_get_number(sobj, "timeout");
  }

  act_str = json_object_get_string(sobj, "action");
  if (act_str) {
    if (strcmp(act_str, "send") == 0)
      step->action = ARAZZO_ASYNC_SEND;
    else if (strcmp(act_str, "receive") == 0)
      step->action = ARAZZO_ASYNC_RECEIVE;
    else
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (json_object_get_array(sobj, "dependsOn")) {
    rc = parse_string_enum_array(json_object_get_array(sobj, "dependsOn"),
                                 &step->depends_on, &step->n_depends_on);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = parse_arazzo_params_array(json_object_get_array(sobj, "parameters"),
                                 &step->parameters, &step->n_parameters);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rb_obj = json_object_get_object(sobj, "requestBody");
  if (rb_obj) {
    const JSON_Array *rep_arr = json_object_get_array(rb_obj, "replacements");
    size_t rcount = json_array_get_count(rep_arr);
    step->has_request_body = 1;
    rc = dup_opt_json_str(rb_obj, "contentType",
                          &step->request_body.content_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (json_object_get_value(rb_obj, "payload")) {
      rc = parse_any_value(json_object_get_value(rb_obj, "payload"),
                           &step->request_body.payload);
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
        const JSON_Object *robj = json_array_get_object(rep_arr, i);
        struct Arazzo_PayloadReplacement *rep =
            &step->request_body.replacements[i];
        if (!json_object_get_string(robj, "target") ||
            !json_object_get_value(robj, "value"))
          return CDD_C_ERROR_INVALID_ARGUMENT;
        rc = dup_opt_json_str(robj, "target", &rep->target);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_criterion_or_expr_type(robj, "targetSelectorType",
                                          &rep->target_selector_type,
                                          &rep->target_expression_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = parse_any_value(json_object_get_value(robj, "value"), &rep->value);
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

  rc = parse_criteria_array(json_object_get_array(sobj, "successCriteria"),
                            &step->success_criteria, &step->n_success_criteria);
  if (rc != CDD_C_SUCCESS)
    return rc;

  succ_arr = json_object_get_array(sobj, "onSuccess");
  scount = json_array_get_count(succ_arr);
  if (scount > 0) {
    step->on_success = (struct Arazzo_SuccessAction *)C_CDD_CALLOC(
        scount, sizeof(struct Arazzo_SuccessAction));
    if (!step->on_success)
      return CDD_C_ERROR_MEMORY;
    step->n_on_success = scount;
    for (i = 0; i < scount; ++i) {
      rc = parse_success_action_object(json_array_get_object(succ_arr, i),
                                       &step->on_success[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  fail_arr = json_object_get_array(sobj, "onFailure");
  fcount = json_array_get_count(fail_arr);
  if (fcount > 0) {
    step->on_failure = (struct Arazzo_FailureAction *)C_CDD_CALLOC(
        fcount, sizeof(struct Arazzo_FailureAction));
    if (!step->on_failure)
      return CDD_C_ERROR_MEMORY;
    step->n_on_failure = fcount;
    for (i = 0; i < fcount; ++i) {
      rc = parse_failure_action_object(json_array_get_object(fail_arr, i),
                                       &step->on_failure[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  rc = parse_outputs_map(json_object_get_object(sobj, "outputs"),
                         &step->outputs, &step->n_outputs);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_arazzo_extensions(sobj, &step->extensions);
}

/**
 * @brief Parses a single `Source Description Object` from `sd_obj`.
 *
 * @param[in] sd_obj Source JSON object.
 * @param[out] sd Destination source description structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
parse_source_description_object(const JSON_Object *sd_obj,
                                struct Arazzo_SourceDescription *sd) {
  const char *tstr = json_object_get_string(sd_obj, "type");
  cdd_c_error_t rc;

  if (!json_object_get_string(sd_obj, "name") ||
      !json_object_get_string(sd_obj, "url"))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (tstr) {
    rc = arazzo_source_type_from_str(tstr, &sd->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    sd->type = ARAZZO_SOURCE_OPENAPI;
  }

  rc = dup_opt_json_str(sd_obj, "name", &sd->name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(sd_obj, "url", &sd->url);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_arazzo_extensions(sd_obj, &sd->extensions);
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
  const JSON_Array *steps_arr;
  const JSON_Array *succ_arr;
  const JSON_Array *fail_arr;
  size_t i;
  size_t n_steps;
  size_t sc;
  size_t fc;
  cdd_c_error_t rc;

  if (!json_object_get_string(wobj, "workflowId"))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  steps_arr = json_object_get_array(wobj, "steps");
  n_steps = json_array_get_count(steps_arr);
  if (n_steps == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = dup_opt_json_str(wobj, "workflowId", &wf->workflow_id);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(wobj, "summary", &wf->summary);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = dup_opt_json_str(wobj, "description", &wf->description);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (json_object_get_object(wobj, "inputs")) {
    wf->has_inputs = 1;
    rc = parse_schema_ref(json_object_get_object(wobj, "inputs"), &wf->inputs,
                          NULL);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (json_object_get_array(wobj, "dependsOn")) {
    rc = parse_string_enum_array(json_object_get_array(wobj, "dependsOn"),
                                 &wf->depends_on, &wf->n_depends_on);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  wf->steps =
      (struct Arazzo_Step *)C_CDD_CALLOC(n_steps, sizeof(struct Arazzo_Step));
  if (!wf->steps)
    return CDD_C_ERROR_MEMORY;
  wf->n_steps = n_steps;

  for (i = 0; i < n_steps; ++i) {
    rc = parse_step_object(json_array_get_object(steps_arr, i), &wf->steps[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  succ_arr = json_object_get_array(wobj, "successActions");
  sc = json_array_get_count(succ_arr);
  if (sc > 0) {
    wf->success_actions = (struct Arazzo_SuccessAction *)C_CDD_CALLOC(
        sc, sizeof(struct Arazzo_SuccessAction));
    if (!wf->success_actions)
      return CDD_C_ERROR_MEMORY;
    wf->n_success_actions = sc;
    for (i = 0; i < sc; ++i) {
      rc = parse_success_action_object(json_array_get_object(succ_arr, i),
                                       &wf->success_actions[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  fail_arr = json_object_get_array(wobj, "failureActions");
  fc = json_array_get_count(fail_arr);
  if (fc > 0) {
    wf->failure_actions = (struct Arazzo_FailureAction *)C_CDD_CALLOC(
        fc, sizeof(struct Arazzo_FailureAction));
    if (!wf->failure_actions)
      return CDD_C_ERROR_MEMORY;
    wf->n_failure_actions = fc;
    for (i = 0; i < fc; ++i) {
      rc = parse_failure_action_object(json_array_get_object(fail_arr, i),
                                       &wf->failure_actions[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  rc = parse_outputs_map(json_object_get_object(wobj, "outputs"), &wf->outputs,
                         &wf->n_outputs);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_arazzo_params_array(json_object_get_array(wobj, "parameters"),
                                 &wf->parameters, &wf->n_parameters);
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
  const JSON_Object *in_obj = json_object_get_object(cobj, "inputs");
  const JSON_Object *pm_obj = json_object_get_object(cobj, "parameters");
  const JSON_Object *sa_obj = json_object_get_object(cobj, "successActions");
  const JSON_Object *fa_obj = json_object_get_object(cobj, "failureActions");
  size_t i;
  size_t n;
  cdd_c_error_t rc;

  n = json_object_get_count(in_obj);
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
      const JSON_Object *sch_obj =
          json_value_get_object(json_object_get_value_at(in_obj, i));
      if (!sch_obj)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      comp->input_keys[i] = C_CDD_STRDUP(json_object_get_name(in_obj, i));
      if (!comp->input_keys[i])
        return CDD_C_ERROR_MEMORY;
      rc = parse_schema_ref(sch_obj, &comp->inputs[i], NULL);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  n = json_object_get_count(pm_obj);
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
      comp->parameter_keys[i] = C_CDD_STRDUP(json_object_get_name(pm_obj, i));
      if (!comp->parameter_keys[i])
        return CDD_C_ERROR_MEMORY;
      rc = parse_arazzo_param_obj(
          json_value_get_object(json_object_get_value_at(pm_obj, i)),
          &comp->parameters[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  n = json_object_get_count(sa_obj);
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
      comp->success_action_keys[i] =
          C_CDD_STRDUP(json_object_get_name(sa_obj, i));
      if (!comp->success_action_keys[i])
        return CDD_C_ERROR_MEMORY;
      rc = parse_success_action_object(
          json_value_get_object(json_object_get_value_at(sa_obj, i)),
          &comp->success_actions[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  n = json_object_get_count(fa_obj);
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
      comp->failure_action_keys[i] =
          C_CDD_STRDUP(json_object_get_name(fa_obj, i));
      if (!comp->failure_action_keys[i])
        return CDD_C_ERROR_MEMORY;
      rc = parse_failure_action_object(
          json_value_get_object(json_object_get_value_at(fa_obj, i)),
          &comp->failure_actions[i]);
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

/**
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

/**
 * @brief Parses an Arazzo v1.1.0 specification from a Parson `JSON_Value`.
 *
 * @param[in] root Root Parson JSON value representing the Arazzo document.
 * @param[out] out_spec Output Arazzo specification structure to populate.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_load_from_json_value(const JSON_Value *root,
                                          struct Arazzo_Spec *out_spec) {
  const JSON_Object *root_obj;
  const JSON_Object *info_obj;
  const JSON_Array *sd_arr;
  const JSON_Array *wf_arr;
  const JSON_Object *comp_obj;
  const char *ver_str;
  size_t i;
  size_t n_sd;
  size_t n_wf;
  cdd_c_error_t rc;

  root_obj = json_value_get_object(root);
  if (!root_obj || !out_spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  ver_str = json_object_get_string(root_obj, "arazzo");
  info_obj = json_object_get_object(root_obj, "info");
  sd_arr = json_object_get_array(root_obj, "sourceDescriptions");
  wf_arr = json_object_get_array(root_obj, "workflows");

  n_sd = json_array_get_count(sd_arr);
  n_wf = json_array_get_count(wf_arr);
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
    arazzo_spec_free(out_spec);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = dup_opt_json_str(root_obj, "$self", &out_spec->self_uri);
  if (rc == CDD_C_SUCCESS)
    rc = dup_opt_json_str(info_obj, "title", &out_spec->info.title);
  if (rc == CDD_C_SUCCESS)
    rc = dup_opt_json_str(info_obj, "summary", &out_spec->info.summary);
  if (rc == CDD_C_SUCCESS)
    rc = dup_opt_json_str(info_obj, "description", &out_spec->info.description);
  if (rc == CDD_C_SUCCESS)
    rc = dup_opt_json_str(info_obj, "version", &out_spec->info.version);
  if (rc == CDD_C_SUCCESS)
    rc = parse_arazzo_extensions(info_obj, &out_spec->info.extensions);
  if (rc == CDD_C_SUCCESS) {
    out_spec->source_descriptions =
        (struct Arazzo_SourceDescription *)C_CDD_CALLOC(
            n_sd, sizeof(struct Arazzo_SourceDescription));
    if (!out_spec->source_descriptions)
      rc = CDD_C_ERROR_MEMORY;
  }
  if (rc != CDD_C_SUCCESS) {
    arazzo_spec_free(out_spec);
    return rc;
  }
  out_spec->n_source_descriptions = n_sd;
  for (i = 0; i < n_sd; ++i) {
    rc = parse_source_description_object(json_array_get_object(sd_arr, i),
                                         &out_spec->source_descriptions[i]);
    if (rc != CDD_C_SUCCESS) {
      arazzo_spec_free(out_spec);
      return rc;
    }
  }

  comp_obj = json_object_get_object(root_obj, "components");
  if (comp_obj) {
    out_spec->has_components = 1;
    rc = parse_components_object(comp_obj, &out_spec->components);
    if (rc != CDD_C_SUCCESS) {
      arazzo_spec_free(out_spec);
      return rc;
    }
  }

  out_spec->workflows = (struct Arazzo_Workflow *)C_CDD_CALLOC(
      n_wf, sizeof(struct Arazzo_Workflow));
  if (!out_spec->workflows) {
    arazzo_spec_free(out_spec);
    return CDD_C_ERROR_MEMORY;
  }
  out_spec->n_workflows = n_wf;
  for (i = 0; i < n_wf; ++i) {
    rc = parse_workflow_object(json_array_get_object(wf_arr, i),
                               &out_spec->workflows[i]);
    if (rc != CDD_C_SUCCESS) {
      arazzo_spec_free(out_spec);
      return rc;
    }
  }

  rc = parse_arazzo_extensions(root_obj, &out_spec->extensions);
  if (rc == CDD_C_SUCCESS)
    rc = arazzo_validate_spec(out_spec);
  if (rc != CDD_C_SUCCESS) {
    arazzo_spec_free(out_spec);
    return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses an Arazzo v1.1.0 specification from a JSON string.
 *
 * @param[in] json_str Raw JSON string containing the Arazzo specification.
 * @param[out] out_spec Output Arazzo specification structure to populate.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_load_from_json(const char *json_str,
                                    struct Arazzo_Spec *out_spec) {
  JSON_Value *root;
  cdd_c_error_t rc;

  if (!json_str || !out_spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  root = json_parse_string(json_str);
  if (!root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_load_from_json_value(root, out_spec);
  json_value_free(root);
  return rc;
}
