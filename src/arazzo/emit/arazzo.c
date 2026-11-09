/**
 * @file arazzo.c
 * @brief Implementation of Arazzo Specification v1.1.0 JSON emitter.
 *
 * Serializes `struct Arazzo_Spec` into canonical Arazzo v1.1.0 `JSON_Value *`
 * trees and formatted JSON strings in strict C89 with full error enum
 * percolation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "arazzo/emit/arazzo.h"
#include "arazzo/parse/arazzo.h"
#include "c_cdd/memory.h"
#include "openapi/emit/openapi.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Accumulates a `cdd_c_error_t` into a sticky `cdd_c_error_t`
 * state.
 *
 * @param[in] st Error status (`CDD_C_SUCCESS` or error code).
 * @param[in,out] rc Sticky error code updated to `CDD_C_ERROR_MEMORY` on
 * failure.
 * @return The updated `*rc`.
 */
static cdd_c_error_t arazzo_check_json(cdd_c_error_t st, cdd_c_error_t *rc) {
  if (st != CDD_C_SUCCESS)
    *rc = st;
  return *rc;
}

/**
 * @brief Accumulates a `cdd_c_error_t` sub-result into a sticky `cdd_c_error_t`
 * state.
 *
 * @param[in] sub_rc Sub-operation error code.
 * @param[in,out] rc Sticky error code updated when `sub_rc != CDD_C_SUCCESS`.
 * @return The updated `*rc`.
 */
static cdd_c_error_t arazzo_check_rc(cdd_c_error_t sub_rc, cdd_c_error_t *rc) {
  if (sub_rc != CDD_C_SUCCESS)
    *rc = sub_rc;
  return *rc;
}

/**
 * @brief Safely attaches `val` to `obj[name]`, freeing `val` and recording
 * `CDD_C_ERROR_MEMORY` into `rc` on failure.
 *
 * @param[in,out] obj Target JSON object.
 * @param[in] name Property key.
 * @param[in] val Value to attach (ownership transferred or freed).
 * @param[in,out] rc Sticky error code.
 * @return The updated `*rc`.
 */
static cdd_c_error_t arazzo_set_val(JSON_Object *obj, const char *name,
                                    JSON_Value *val, cdd_c_error_t *rc) {
  if (!val || cdd_json_object_set_value_safe(obj, name, val) != CDD_C_SUCCESS)
    *rc = CDD_C_ERROR_MEMORY;
  return *rc;
}

/**
 * @brief Safely appends `val` to `arr`, freeing `val` and recording
 * `CDD_C_ERROR_MEMORY` into `rc` on failure.
 *
 * @param[in,out] arr Target JSON array.
 * @param[in] val Value to append (ownership transferred or freed).
 * @param[in,out] rc Sticky error code.
 * @return The updated `*rc`.
 */
static cdd_c_error_t arazzo_append_val(JSON_Array *arr, JSON_Value *val,
                                       cdd_c_error_t *rc) {
  if (!val || cdd_json_array_append_value_safe(arr, val) != CDD_C_SUCCESS)
    *rc = CDD_C_ERROR_MEMORY;
  return *rc;
}

/**
 * @brief Serializes an `Arazzo_ExtensionList` into `target_obj`.
 *
 * @param[in,out] target_obj Target JSON object.
 * @param[in] ext_list Source extension list.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_arazzo_extensions(JSON_Object *target_obj,
                        const struct Arazzo_ExtensionList *ext_list) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  for (i = 0; i < ext_list->count; ++i) {
    JSON_Value *val = NULL;
    g_cdd_last_alloc_err =
        arazzo_check_rc(any_to_json_value(&ext_list->items[i].val, &val), &rc);
    g_cdd_last_alloc_err =
        arazzo_set_val(target_obj, ext_list->items[i].key, val, &rc);
  }
  return rc;
}

/**
 * @brief Serializes a Criterion or Selector `type` / `expression_type` into
 * `obj`.
 *
 * @param[in,out] obj Target JSON object.
 * @param[in] key Property key (`"type"` or `"targetSelectorType"`).
 * @param[in] type Criterion/selector type enum.
 * @param[in] et Expression Type Object when `type ==
 * ARAZZO_CRITERION_EXPRESSION_TYPE`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_criterion_or_expr_type(JSON_Object *obj, const char *key,
                             enum Arazzo_CriterionType type,
                             const struct Arazzo_ExpressionType *et) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (type == ARAZZO_CRITERION_REGEX) {
    g_cdd_last_alloc_err =
        arazzo_check_json(cdd_json_object_set_string(obj, key, "regex"), &rc);
  } else if (type == ARAZZO_CRITERION_JSONPATH) {
    g_cdd_last_alloc_err = arazzo_check_json(
        cdd_json_object_set_string(obj, key, "jsonpath"), &rc);
  } else if (type == ARAZZO_CRITERION_XPATH) {
    g_cdd_last_alloc_err =
        arazzo_check_json(cdd_json_object_set_string(obj, key, "xpath"), &rc);
  } else if (type == ARAZZO_CRITERION_EXPRESSION_TYPE) {
    JSON_Value *et_val = NULL;
    JSON_Object *et_obj = NULL;
    const char *dstr = "jsonpointer";
    g_cdd_last_alloc_err = cdd_json_value_init_object(&et_val);
    g_cdd_last_alloc_err = cdd_json_value_get_object(et_val, &et_obj);
    if (et->type == ARAZZO_DIALECT_JSONPATH)
      dstr = "jsonpath";
    else if (et->type == ARAZZO_DIALECT_XPATH)
      dstr = "xpath";
    g_cdd_last_alloc_err = arazzo_check_json(
        cdd_json_object_set_string(et_obj, "type", dstr), &rc);
    if (et->version)
      g_cdd_last_alloc_err = arazzo_check_json(
          cdd_json_object_set_string(et_obj, "version", et->version), &rc);
    g_cdd_last_alloc_err =
        arazzo_check_rc(write_arazzo_extensions(et_obj, &et->extensions), &rc);
    g_cdd_last_alloc_err = arazzo_set_val(obj, key, et_val, &rc);
  }
  return rc;
}

/**
 * @brief Table-driven helper to serialize reflected fields into a `JSON_Object`
 * using `cdd_reflect_to_json_object`.
 *
 * @param[in,out] obj Destination JSON object.
 * @param[in] ti Runtime reflection descriptor.
 * @param[in] src Source C struct pointer.
 * @param[in,out] rc Sticky error code.
 * @return The updated `*rc`.
 */
static cdd_c_error_t
arazzo_emit_reflected_strings(JSON_Object *obj, const struct cdd_type_info *ti,
                              const void *src, cdd_c_error_t *rc) {
  return arazzo_check_rc(cdd_reflect_to_json_object(src, ti, obj), rc);
}

cdd_c_error_t
Arazzo_Criterion_to_jsonObject(const struct Arazzo_Criterion *crit,
                               JSON_Object *cobj) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!crit || !cobj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
      cobj, &Arazzo_Criterion_type_info, crit, &rc);
  g_cdd_last_alloc_err =
      arazzo_check_rc(write_criterion_or_expr_type(cobj, "type", crit->type,
                                                   &crit->expression_type),
                      &rc);
  g_cdd_last_alloc_err =
      arazzo_check_rc(write_arazzo_extensions(cobj, &crit->extensions), &rc);
  return rc;
}

cdd_c_error_t Arazzo_Info_to_jsonObject(const struct Arazzo_Info *info,
                                        JSON_Object *info_obj) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!info || !info_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
      info_obj, &Arazzo_Info_type_info, info, &rc);
  g_cdd_last_alloc_err = arazzo_check_rc(
      write_arazzo_extensions(info_obj, &info->extensions), &rc);
  return rc;
}

cdd_c_error_t Arazzo_SourceDescription_to_jsonObject(
    const struct Arazzo_SourceDescription *sd, JSON_Object *sd_obj) {
  const char *tstr = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!sd || !sd_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
      sd_obj, &Arazzo_SourceDescription_type_info, sd, &rc);
  if (arazzo_source_type_to_str(sd->type, &tstr) == CDD_C_SUCCESS)
    g_cdd_last_alloc_err = arazzo_check_json(
        cdd_json_object_set_string(sd_obj, "type", tstr), &rc);
  g_cdd_last_alloc_err =
      arazzo_check_rc(write_arazzo_extensions(sd_obj, &sd->extensions), &rc);
  return rc;
}

/**
 * @brief Serializes an array of `Arazzo_Criterion` into `parent` under `key`.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Array property name.
 * @param[in] criteria Criteria array.
 * @param[in] n_criteria Number of criteria.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_criteria_array(JSON_Object *parent, const char *key,
                     const struct Arazzo_Criterion *criteria,
                     size_t n_criteria) {
  JSON_Value *arr_val = NULL;
  JSON_Array *arr = NULL;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (n_criteria == 0)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_array(&arr_val);
  g_cdd_last_alloc_err = cdd_json_value_get_array(arr_val, &arr);
  for (i = 0; i < n_criteria; ++i) {
    JSON_Value *cval = NULL;
    JSON_Object *cobj = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&cval);
    g_cdd_last_alloc_err = cdd_json_value_get_object(cval, &cobj);
    g_cdd_last_alloc_err = arazzo_check_rc(
        Arazzo_Criterion_to_jsonObject(&criteria[i], cobj), &rc);
    g_cdd_last_alloc_err = arazzo_append_val(arr, cval, &rc);
  }
  g_cdd_last_alloc_err = arazzo_set_val(parent, key, arr_val, &rc);
  return rc;
}

/**
 * @brief Serializes a single `Arazzo_Parameter` into a `JSON_Value *` object.
 *
 * @param[in] param Source parameter structure.
 * @param[out] out_val Output `JSON_Value *` object.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_arazzo_param_obj(const struct Arazzo_Parameter *param,
                       JSON_Value **out_val) {
  JSON_Value *pval = NULL;
  JSON_Object *pobj = NULL;
  const char *in_str = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&pval);
  g_cdd_last_alloc_err = cdd_json_value_get_object(pval, &pobj);

  if (param->reusable.is_reusable) {
    g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
        pobj, &Arazzo_Reusable_type_info, &param->reusable, &rc);
    if (param->reusable.has_value_override) {
      JSON_Value *ov = NULL;
      g_cdd_last_alloc_err =
          arazzo_check_rc(any_to_json_value(&param->reusable.value, &ov), &rc);
      g_cdd_last_alloc_err = arazzo_set_val(pobj, "value", ov, &rc);
    }
    *out_val = pval;
    return rc;
  }

  g_cdd_last_alloc_err = arazzo_check_json(
      cdd_json_object_set_string(pobj, "name", param->name), &rc);
  if (arazzo_param_in_to_str(param->in, &in_str) == CDD_C_SUCCESS)
    g_cdd_last_alloc_err =
        arazzo_check_json(cdd_json_object_set_string(pobj, "in", in_str), &rc);

  if (param->is_selector) {
    JSON_Value *sval = NULL;
    JSON_Object *sobj = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&sval);
    g_cdd_last_alloc_err = cdd_json_value_get_object(sval, &sobj);
    g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
        sobj, &Arazzo_Selector_type_info, &param->selector, &rc);
    g_cdd_last_alloc_err = arazzo_check_rc(
        write_criterion_or_expr_type(sobj, "type", param->selector.type,
                                     &param->selector.expression_type),
        &rc);
    g_cdd_last_alloc_err = arazzo_check_rc(
        write_arazzo_extensions(sobj, &param->selector.extensions), &rc);
    g_cdd_last_alloc_err = arazzo_set_val(pobj, "value", sval, &rc);
  } else {
    JSON_Value *vval = NULL;
    g_cdd_last_alloc_err =
        arazzo_check_rc(any_to_json_value(&param->value, &vval), &rc);
    g_cdd_last_alloc_err = arazzo_set_val(pobj, "value", vval, &rc);
  }

  g_cdd_last_alloc_err =
      arazzo_check_rc(write_arazzo_extensions(pobj, &param->extensions), &rc);
  *out_val = pval;
  return rc;
}

/**
 * @brief Serializes an array of `Arazzo_Parameter` into `parent` under `key`.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property name.
 * @param[in] params Parameters array.
 * @param[in] n_params Number of parameters.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_parameters_array(JSON_Object *parent, const char *key,
                       const struct Arazzo_Parameter *params, size_t n_params) {
  JSON_Value *arr_val = NULL;
  JSON_Array *arr = NULL;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (n_params == 0)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_array(&arr_val);
  g_cdd_last_alloc_err = cdd_json_value_get_array(arr_val, &arr);
  for (i = 0; i < n_params; ++i) {
    JSON_Value *pval = NULL;
    g_cdd_last_alloc_err =
        arazzo_check_rc(write_arazzo_param_obj(&params[i], &pval), &rc);
    g_cdd_last_alloc_err = arazzo_append_val(arr, pval, &rc);
  }
  g_cdd_last_alloc_err = arazzo_set_val(parent, key, arr_val, &rc);
  return rc;
}

/**
 * @brief Serializes a single `Arazzo_SuccessAction` into a `JSON_Value *`
 * object.
 *
 * @param[in] act Source success action.
 * @param[out] out_val Output `JSON_Value *` object.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_success_action_object(const struct Arazzo_SuccessAction *act,
                            JSON_Value **out_val) {
  JSON_Value *aval = NULL;
  JSON_Object *aobj = NULL;
  const char *tstr = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&aval);
  g_cdd_last_alloc_err = cdd_json_value_get_object(aval, &aobj);

  if (act->reusable.is_reusable) {
    g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
        aobj, &Arazzo_Reusable_type_info, &act->reusable, &rc);
    *out_val = aval;
    return rc;
  }

  g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
      aobj, &Arazzo_SuccessAction_type_info, act, &rc);
  if (arazzo_action_type_to_str(act->type, &tstr) == CDD_C_SUCCESS)
    g_cdd_last_alloc_err =
        arazzo_check_json(cdd_json_object_set_string(aobj, "type", tstr), &rc);

  g_cdd_last_alloc_err = arazzo_check_rc(
      write_parameters_array(aobj, "parameters", act->parameters,
                             act->n_parameters),
      &rc);
  g_cdd_last_alloc_err = arazzo_check_rc(
      write_criteria_array(aobj, "criteria", act->criteria, act->n_criteria),
      &rc);
  g_cdd_last_alloc_err =
      arazzo_check_rc(write_arazzo_extensions(aobj, &act->extensions), &rc);
  *out_val = aval;
  return rc;
}

/**
 * @brief Serializes a single `Arazzo_FailureAction` into a `JSON_Value *`
 * object.
 *
 * @param[in] act Source failure action.
 * @param[out] out_val Output `JSON_Value *` object.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_failure_action_object(const struct Arazzo_FailureAction *act,
                            JSON_Value **out_val) {
  JSON_Value *aval = NULL;
  JSON_Object *aobj = NULL;
  const char *tstr = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&aval);
  g_cdd_last_alloc_err = cdd_json_value_get_object(aval, &aobj);

  if (act->reusable.is_reusable) {
    g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
        aobj, &Arazzo_Reusable_type_info, &act->reusable, &rc);
    *out_val = aval;
    return rc;
  }

  g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
      aobj, &Arazzo_FailureAction_type_info, act, &rc);
  if (arazzo_action_type_to_str(act->type, &tstr) == CDD_C_SUCCESS)
    g_cdd_last_alloc_err =
        arazzo_check_json(cdd_json_object_set_string(aobj, "type", tstr), &rc);
  if (act->has_retry_after)
    g_cdd_last_alloc_err = arazzo_check_json(
        cdd_json_object_set_number(aobj, "retryAfter", act->retry_after), &rc);
  if (act->has_retry_limit)
    g_cdd_last_alloc_err =
        arazzo_check_json(cdd_json_object_set_number(aobj, "retryLimit",
                                                     (double)act->retry_limit),
                          &rc);

  g_cdd_last_alloc_err = arazzo_check_rc(
      write_parameters_array(aobj, "parameters", act->parameters,
                             act->n_parameters),
      &rc);
  g_cdd_last_alloc_err = arazzo_check_rc(
      write_criteria_array(aobj, "criteria", act->criteria, act->n_criteria),
      &rc);
  g_cdd_last_alloc_err =
      arazzo_check_rc(write_arazzo_extensions(aobj, &act->extensions), &rc);
  *out_val = aval;
  return rc;
}

/**
 * @brief Serializes an `outputs` map (`Arazzo_OutputMapEntry` array) into
 * `parent`.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] outputs Array of output map entries.
 * @param[in] n_outputs Number of output entries.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_outputs_map(JSON_Object *parent,
                  const struct Arazzo_OutputMapEntry *outputs,
                  size_t n_outputs) {
  JSON_Value *out_val = NULL;
  JSON_Object *out_obj = NULL;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (n_outputs == 0)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&out_val);
  g_cdd_last_alloc_err = cdd_json_value_get_object(out_val, &out_obj);
  for (i = 0; i < n_outputs; ++i) {
    if (outputs[i].is_selector) {
      JSON_Value *sval = NULL;
      JSON_Object *sobj = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&sval);
      g_cdd_last_alloc_err = cdd_json_value_get_object(sval, &sobj);
      g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
          sobj, &Arazzo_Selector_type_info, &outputs[i].selector, &rc);
      g_cdd_last_alloc_err = arazzo_check_rc(
          write_criterion_or_expr_type(sobj, "type", outputs[i].selector.type,
                                       &outputs[i].selector.expression_type),
          &rc);
      g_cdd_last_alloc_err = arazzo_check_rc(
          write_arazzo_extensions(sobj, &outputs[i].selector.extensions), &rc);
      g_cdd_last_alloc_err = arazzo_set_val(out_obj, outputs[i].key, sval, &rc);
    } else {
      g_cdd_last_alloc_err = arazzo_check_json(
          cdd_json_object_set_string(out_obj, outputs[i].key,
                                     outputs[i].expression_or_value),
          &rc);
    }
  }
  g_cdd_last_alloc_err = arazzo_set_val(parent, "outputs", out_val, &rc);
  return rc;
}

/**
 * @brief Serializes a string array (`dependsOn`) into `parent` under `key`.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property name.
 * @param[in] items Array of strings.
 * @param[in] count Number of strings.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t write_string_list_field(JSON_Object *parent,
                                             const char *key,
                                             char *const *items, size_t count) {
  JSON_Value *aval = NULL;
  JSON_Array *arr = NULL;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (count == 0)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_value_init_array(&aval);
  g_cdd_last_alloc_err = cdd_json_value_get_array(aval, &arr);
  for (i = 0; i < count; ++i) {
    g_cdd_last_alloc_err =
        arazzo_check_json(cdd_json_array_append_string(arr, items[i]), &rc);
  }
  g_cdd_last_alloc_err = arazzo_set_val(parent, key, aval, &rc);
  return rc;
}

/**
 * @brief Serializes a single `Arazzo_Step` into `sobj`.
 *
 * @param[in,out] sobj Target step JSON object.
 * @param[in] step Source step structure.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t write_step_object(JSON_Object *sobj,
                                       const struct Arazzo_Step *step) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      arazzo_emit_reflected_strings(sobj, &Arazzo_Step_type_info, step, &rc);
  if (step->has_timeout)
    g_cdd_last_alloc_err = arazzo_check_json(
        cdd_json_object_set_number(sobj, "timeout", (double)step->timeout_ms),
        &rc);
  if (step->action == ARAZZO_ASYNC_SEND)
    g_cdd_last_alloc_err = arazzo_check_json(
        cdd_json_object_set_string(sobj, "action", "send"), &rc);
  else if (step->action == ARAZZO_ASYNC_RECEIVE)
    g_cdd_last_alloc_err = arazzo_check_json(
        cdd_json_object_set_string(sobj, "action", "receive"), &rc);

  g_cdd_last_alloc_err = arazzo_check_rc(
      write_string_list_field(sobj, "dependsOn", step->depends_on,
                              step->n_depends_on),
      &rc);
  g_cdd_last_alloc_err = arazzo_check_rc(
      write_parameters_array(sobj, "parameters", step->parameters,
                             step->n_parameters),
      &rc);

  if (step->has_request_body) {
    JSON_Value *rb_val = NULL;
    JSON_Object *rb_obj = NULL;
    JSON_Value *pay_val = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&rb_val);
    g_cdd_last_alloc_err = cdd_json_value_get_object(rb_val, &rb_obj);
    if (step->request_body.content_type)
      g_cdd_last_alloc_err = arazzo_check_json(
          cdd_json_object_set_string(rb_obj, "contentType",
                                     step->request_body.content_type),
          &rc);
    g_cdd_last_alloc_err = arazzo_check_rc(
        any_to_json_value(&step->request_body.payload, &pay_val), &rc);
    g_cdd_last_alloc_err = arazzo_set_val(rb_obj, "payload", pay_val, &rc);
    if (step->request_body.n_replacements > 0) {
      JSON_Value *rarr_val = NULL;
      JSON_Array *rarr = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_array(&rarr_val);
      g_cdd_last_alloc_err = cdd_json_value_get_array(rarr_val, &rarr);
      for (i = 0; i < step->request_body.n_replacements; ++i) {
        const struct Arazzo_PayloadReplacement *rep =
            &step->request_body.replacements[i];
        JSON_Value *rval = NULL;
        JSON_Object *robj = NULL;
        JSON_Value *rv = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&rval);
        g_cdd_last_alloc_err = cdd_json_value_get_object(rval, &robj);
        g_cdd_last_alloc_err = arazzo_check_json(
            cdd_json_object_set_string(robj, "target", rep->target), &rc);
        g_cdd_last_alloc_err = arazzo_check_rc(
            write_criterion_or_expr_type(robj, "targetSelectorType",
                                         rep->target_selector_type,
                                         &rep->target_expression_type),
            &rc);
        g_cdd_last_alloc_err =
            arazzo_check_rc(any_to_json_value(&rep->value, &rv), &rc);
        g_cdd_last_alloc_err = arazzo_set_val(robj, "value", rv, &rc);
        g_cdd_last_alloc_err = arazzo_check_rc(
            write_arazzo_extensions(robj, &rep->extensions), &rc);
        g_cdd_last_alloc_err = arazzo_append_val(rarr, rval, &rc);
      }
      g_cdd_last_alloc_err =
          arazzo_set_val(rb_obj, "replacements", rarr_val, &rc);
    }
    g_cdd_last_alloc_err = arazzo_check_rc(
        write_arazzo_extensions(rb_obj, &step->request_body.extensions), &rc);
    g_cdd_last_alloc_err = arazzo_set_val(sobj, "requestBody", rb_val, &rc);
  }

  g_cdd_last_alloc_err = arazzo_check_rc(
      write_criteria_array(sobj, "successCriteria", step->success_criteria,
                           step->n_success_criteria),
      &rc);

  if (step->n_on_success > 0) {
    JSON_Value *sa_val = NULL;
    JSON_Array *sa_arr = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_array(&sa_val);
    g_cdd_last_alloc_err = cdd_json_value_get_array(sa_val, &sa_arr);
    for (i = 0; i < step->n_on_success; ++i) {
      JSON_Value *item = NULL;
      g_cdd_last_alloc_err = arazzo_check_rc(
          write_success_action_object(&step->on_success[i], &item), &rc);
      g_cdd_last_alloc_err = arazzo_append_val(sa_arr, item, &rc);
    }
    g_cdd_last_alloc_err = arazzo_set_val(sobj, "onSuccess", sa_val, &rc);
  }

  if (step->n_on_failure > 0) {
    JSON_Value *fa_val = NULL;
    JSON_Array *fa_arr = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_array(&fa_val);
    g_cdd_last_alloc_err = cdd_json_value_get_array(fa_val, &fa_arr);
    for (i = 0; i < step->n_on_failure; ++i) {
      JSON_Value *item = NULL;
      g_cdd_last_alloc_err = arazzo_check_rc(
          write_failure_action_object(&step->on_failure[i], &item), &rc);
      g_cdd_last_alloc_err = arazzo_append_val(fa_arr, item, &rc);
    }
    g_cdd_last_alloc_err = arazzo_set_val(sobj, "onFailure", fa_val, &rc);
  }

  g_cdd_last_alloc_err = arazzo_check_rc(
      write_outputs_map(sobj, step->outputs, step->n_outputs), &rc);
  g_cdd_last_alloc_err =
      arazzo_check_rc(write_arazzo_extensions(sobj, &step->extensions), &rc);
  return rc;
}

/*
 * @brief Serializes an `Arazzo_Spec` structure into a Parson `JSON_Value` tree.
 *
 * @param[in] spec Parsed Arazzo specification to serialize.
 * @param[out] out_root Output pointer receiving the newly created Parson
 * JSON_Value.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_write_spec_to_json_value(const struct Arazzo_Spec *spec,
                                              JSON_Value **out_root) {
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Value *info_val = NULL;
  JSON_Object *info_obj = NULL;
  JSON_Value *sd_arr_val = NULL;
  JSON_Array *sd_arr = NULL;
  JSON_Value *wf_arr_val = NULL;
  JSON_Array *wf_arr = NULL;
  size_t i;
  size_t j;
  cdd_c_error_t rc;

  if (!out_root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_validate_spec(spec);
  if (rc != CDD_C_SUCCESS)
    return rc;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&root_val);
  g_cdd_last_alloc_err = cdd_json_value_get_object(root_val, &root_obj);

  g_cdd_last_alloc_err = arazzo_check_json(
      cdd_json_object_set_string(root_obj, "arazzo", spec->arazzo), &rc);
  if (spec->self_uri)
    g_cdd_last_alloc_err = arazzo_check_json(
        cdd_json_object_set_string(root_obj, "$self", spec->self_uri), &rc);

  g_cdd_last_alloc_err = cdd_json_value_init_object(&info_val);
  g_cdd_last_alloc_err = cdd_json_value_get_object(info_val, &info_obj);
  g_cdd_last_alloc_err =
      arazzo_check_rc(Arazzo_Info_to_jsonObject(&spec->info, info_obj), &rc);
  g_cdd_last_alloc_err = arazzo_set_val(root_obj, "info", info_val, &rc);

  g_cdd_last_alloc_err = cdd_json_value_init_array(&sd_arr_val);
  g_cdd_last_alloc_err = cdd_json_value_get_array(sd_arr_val, &sd_arr);
  for (i = 0; i < spec->n_source_descriptions; ++i) {
    JSON_Value *sd_val = NULL;
    JSON_Object *sd_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&sd_val);
    g_cdd_last_alloc_err = cdd_json_value_get_object(sd_val, &sd_obj);
    g_cdd_last_alloc_err =
        arazzo_check_rc(Arazzo_SourceDescription_to_jsonObject(
                            &spec->source_descriptions[i], sd_obj),
                        &rc);
    g_cdd_last_alloc_err = arazzo_append_val(sd_arr, sd_val, &rc);
  }
  g_cdd_last_alloc_err =
      arazzo_set_val(root_obj, "sourceDescriptions", sd_arr_val, &rc);

  g_cdd_last_alloc_err = cdd_json_value_init_array(&wf_arr_val);
  g_cdd_last_alloc_err = cdd_json_value_get_array(wf_arr_val, &wf_arr);
  for (i = 0; i < spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &spec->workflows[i];
    JSON_Value *wval = NULL;
    JSON_Object *wobj = NULL;
    JSON_Value *sarr_val = NULL;
    JSON_Array *sarr = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&wval);
    g_cdd_last_alloc_err = cdd_json_value_get_object(wval, &wobj);
    g_cdd_last_alloc_err = cdd_json_value_init_array(&sarr_val);
    g_cdd_last_alloc_err = cdd_json_value_get_array(sarr_val, &sarr);

    g_cdd_last_alloc_err = arazzo_emit_reflected_strings(
        wobj, &Arazzo_Workflow_type_info, wf, &rc);
    if (wf->has_inputs)
      g_cdd_last_alloc_err =
          arazzo_check_rc(write_schema_ref(wobj, "inputs", &wf->inputs), &rc);
    g_cdd_last_alloc_err = arazzo_check_rc(
        write_string_list_field(wobj, "dependsOn", wf->depends_on,
                                wf->n_depends_on),
        &rc);

    for (j = 0; j < wf->n_steps; ++j) {
      JSON_Value *st_val = NULL;
      JSON_Object *st_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&st_val);
      g_cdd_last_alloc_err = cdd_json_value_get_object(st_val, &st_obj);
      g_cdd_last_alloc_err =
          arazzo_check_rc(write_step_object(st_obj, &wf->steps[j]), &rc);
      g_cdd_last_alloc_err = arazzo_append_val(sarr, st_val, &rc);
    }
    g_cdd_last_alloc_err = arazzo_set_val(wobj, "steps", sarr_val, &rc);

    if (wf->n_success_actions > 0) {
      JSON_Value *sa_val = NULL;
      JSON_Array *sa_arr = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_array(&sa_val);
      g_cdd_last_alloc_err = cdd_json_value_get_array(sa_val, &sa_arr);
      for (j = 0; j < wf->n_success_actions; ++j) {
        JSON_Value *item = NULL;
        g_cdd_last_alloc_err = arazzo_check_rc(
            write_success_action_object(&wf->success_actions[j], &item), &rc);
        g_cdd_last_alloc_err = arazzo_append_val(sa_arr, item, &rc);
      }
      g_cdd_last_alloc_err =
          arazzo_set_val(wobj, "successActions", sa_val, &rc);
    }

    if (wf->n_failure_actions > 0) {
      JSON_Value *fa_val = NULL;
      JSON_Array *fa_arr = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_array(&fa_val);
      g_cdd_last_alloc_err = cdd_json_value_get_array(fa_val, &fa_arr);
      for (j = 0; j < wf->n_failure_actions; ++j) {
        JSON_Value *item = NULL;
        g_cdd_last_alloc_err = arazzo_check_rc(
            write_failure_action_object(&wf->failure_actions[j], &item), &rc);
        g_cdd_last_alloc_err = arazzo_append_val(fa_arr, item, &rc);
      }
      g_cdd_last_alloc_err =
          arazzo_set_val(wobj, "failureActions", fa_val, &rc);
    }

    g_cdd_last_alloc_err = arazzo_check_rc(
        write_outputs_map(wobj, wf->outputs, wf->n_outputs), &rc);
    g_cdd_last_alloc_err = arazzo_check_rc(
        write_parameters_array(wobj, "parameters", wf->parameters,
                               wf->n_parameters),
        &rc);
    g_cdd_last_alloc_err =
        arazzo_check_rc(write_arazzo_extensions(wobj, &wf->extensions), &rc);
    g_cdd_last_alloc_err = arazzo_append_val(wf_arr, wval, &rc);
  }
  g_cdd_last_alloc_err = arazzo_set_val(root_obj, "workflows", wf_arr_val, &rc);

  if (spec->has_components) {
    JSON_Value *cval = NULL;
    JSON_Object *cobj = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&cval);
    g_cdd_last_alloc_err = cdd_json_value_get_object(cval, &cobj);
    if (spec->components.n_inputs > 0) {
      JSON_Value *in_val = NULL;
      JSON_Object *in_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&in_val);
      g_cdd_last_alloc_err = cdd_json_value_get_object(in_val, &in_obj);
      for (i = 0; i < spec->components.n_inputs; ++i) {
        g_cdd_last_alloc_err = arazzo_check_rc(
            write_schema_ref(in_obj, spec->components.input_keys[i],
                             &spec->components.inputs[i]),
            &rc);
      }
      g_cdd_last_alloc_err = arazzo_set_val(cobj, "inputs", in_val, &rc);
    }
    if (spec->components.n_parameters > 0) {
      JSON_Value *pm_val = NULL;
      JSON_Object *pm_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&pm_val);
      g_cdd_last_alloc_err = cdd_json_value_get_object(pm_val, &pm_obj);
      for (i = 0; i < spec->components.n_parameters; ++i) {
        JSON_Value *item = NULL;
        g_cdd_last_alloc_err = arazzo_check_rc(
            write_arazzo_param_obj(&spec->components.parameters[i], &item),
            &rc);
        g_cdd_last_alloc_err = arazzo_set_val(
            pm_obj, spec->components.parameter_keys[i], item, &rc);
      }
      g_cdd_last_alloc_err = arazzo_set_val(cobj, "parameters", pm_val, &rc);
    }
    if (spec->components.n_success_actions > 0) {
      JSON_Value *sa_val = NULL;
      JSON_Object *sa_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&sa_val);
      g_cdd_last_alloc_err = cdd_json_value_get_object(sa_val, &sa_obj);
      for (i = 0; i < spec->components.n_success_actions; ++i) {
        JSON_Value *item = NULL;
        g_cdd_last_alloc_err =
            arazzo_check_rc(write_success_action_object(
                                &spec->components.success_actions[i], &item),
                            &rc);
        g_cdd_last_alloc_err = arazzo_set_val(
            sa_obj, spec->components.success_action_keys[i], item, &rc);
      }
      g_cdd_last_alloc_err =
          arazzo_set_val(cobj, "successActions", sa_val, &rc);
    }
    if (spec->components.n_failure_actions > 0) {
      JSON_Value *fa_val = NULL;
      JSON_Object *fa_obj = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&fa_val);
      g_cdd_last_alloc_err = cdd_json_value_get_object(fa_val, &fa_obj);
      for (i = 0; i < spec->components.n_failure_actions; ++i) {
        JSON_Value *item = NULL;
        g_cdd_last_alloc_err =
            arazzo_check_rc(write_failure_action_object(
                                &spec->components.failure_actions[i], &item),
                            &rc);
        g_cdd_last_alloc_err = arazzo_set_val(
            fa_obj, spec->components.failure_action_keys[i], item, &rc);
      }
      g_cdd_last_alloc_err =
          arazzo_set_val(cobj, "failureActions", fa_val, &rc);
    }
    g_cdd_last_alloc_err = arazzo_check_rc(
        write_arazzo_extensions(cobj, &spec->components.extensions), &rc);
    g_cdd_last_alloc_err = arazzo_set_val(root_obj, "components", cval, &rc);
  }

  g_cdd_last_alloc_err = arazzo_check_rc(
      write_arazzo_extensions(root_obj, &spec->extensions), &rc);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = cdd_json_value_free(root_val);
    *out_root = NULL;
    return rc;
  }
  *out_root = root_val;
  return CDD_C_SUCCESS;
}

/*
 * @brief Serializes an `Arazzo_Spec` structure into a formatted JSON string.
 *
 * @param[in] spec Parsed Arazzo specification to serialize.
 * @param[out] out_json Output pointer receiving the newly allocated JSON
 * string.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_write_spec_to_json(const struct Arazzo_Spec *spec,
                                        char **out_json) {
  JSON_Value *root = NULL;
  char *ser = NULL;
  cdd_c_error_t rc;

  if (!spec || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_write_spec_to_json_value(spec, &root);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_json_serialize_to_string_pretty(root, &ser);
  g_cdd_last_alloc_err = cdd_json_value_free(root);
  *out_json = C_CDD_STRDUP(ser);
  g_cdd_last_alloc_err = cdd_json_free_serialized_string(ser);
  return *out_json ? rc : CDD_C_ERROR_MEMORY;
}
