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
  for (i = 0; i < ext_list->count; ++i) {
    JSON_Value *val = NULL;
    any_to_json_value(&ext_list->items[i].val, &val);
    json_object_set_value(target_obj, ext_list->items[i].key, val);
  }
  return CDD_C_SUCCESS;
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
  if (type == ARAZZO_CRITERION_REGEX) {
    json_object_set_string(obj, key, "regex");
  } else if (type == ARAZZO_CRITERION_JSONPATH) {
    json_object_set_string(obj, key, "jsonpath");
  } else if (type == ARAZZO_CRITERION_XPATH) {
    json_object_set_string(obj, key, "xpath");
  } else if (type == ARAZZO_CRITERION_EXPRESSION_TYPE) {
    JSON_Value *et_val = json_value_init_object();
    JSON_Object *et_obj = json_value_get_object(et_val);
    const char *dstr = "jsonpointer";
    if (et->type == ARAZZO_DIALECT_JSONPATH)
      dstr = "jsonpath";
    else if (et->type == ARAZZO_DIALECT_XPATH)
      dstr = "xpath";
    json_object_set_string(et_obj, "type", dstr);
    if (et->version)
      json_object_set_string(et_obj, "version", et->version);
    write_arazzo_extensions(et_obj, &et->extensions);
    json_object_set_value(obj, key, et_val);
  }
  return CDD_C_SUCCESS;
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
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (n_criteria == 0)
    return CDD_C_SUCCESS;

  arr_val = json_value_init_array();
  arr = json_value_get_array(arr_val);
  for (i = 0; i < n_criteria; ++i) {
    JSON_Value *cval = json_value_init_object();
    JSON_Object *cobj = json_value_get_object(cval);
    if (criteria[i].context)
      json_object_set_string(cobj, "context", criteria[i].context);
    json_object_set_string(cobj, "condition", criteria[i].condition);
    write_criterion_or_expr_type(cobj, "type", criteria[i].type,
                                 &criteria[i].expression_type);
    write_arazzo_extensions(cobj, &criteria[i].extensions);
    json_array_append_value(arr, cval);
  }
  json_object_set_value(parent, key, arr_val);
  return CDD_C_SUCCESS;
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
  JSON_Value *pval = json_value_init_object();
  JSON_Object *pobj = json_value_get_object(pval);
  const char *in_str = NULL;

  if (param->reusable.is_reusable) {
    json_object_set_string(pobj, "reference", param->reusable.reference);
    if (param->reusable.has_value_override) {
      JSON_Value *ov = NULL;
      any_to_json_value(&param->reusable.value, &ov);
      json_object_set_value(pobj, "value", ov);
    }
    *out_val = pval;
    return CDD_C_SUCCESS;
  }

  json_object_set_string(pobj, "name", param->name);
  if (arazzo_param_in_to_str(param->in, &in_str) == CDD_C_SUCCESS)
    json_object_set_string(pobj, "in", in_str);

  if (param->is_selector) {
    JSON_Value *sval = json_value_init_object();
    JSON_Object *sobj = json_value_get_object(sval);
    if (param->selector.context)
      json_object_set_string(sobj, "context", param->selector.context);
    json_object_set_string(sobj, "selector", param->selector.selector);
    write_criterion_or_expr_type(sobj, "type", param->selector.type,
                                 &param->selector.expression_type);
    write_arazzo_extensions(sobj, &param->selector.extensions);
    json_object_set_value(pobj, "value", sval);
  } else {
    JSON_Value *vval = NULL;
    any_to_json_value(&param->value, &vval);
    json_object_set_value(pobj, "value", vval);
  }

  write_arazzo_extensions(pobj, &param->extensions);
  *out_val = pval;
  return CDD_C_SUCCESS;
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
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (n_params == 0)
    return CDD_C_SUCCESS;

  arr_val = json_value_init_array();
  arr = json_value_get_array(arr_val);
  for (i = 0; i < n_params; ++i) {
    JSON_Value *pval = NULL;
    write_arazzo_param_obj(&params[i], &pval);
    json_array_append_value(arr, pval);
  }
  json_object_set_value(parent, key, arr_val);
  return CDD_C_SUCCESS;
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
  JSON_Value *aval = json_value_init_object();
  JSON_Object *aobj = json_value_get_object(aval);
  const char *tstr = NULL;

  if (act->reusable.is_reusable) {
    json_object_set_string(aobj, "reference", act->reusable.reference);
    *out_val = aval;
    return CDD_C_SUCCESS;
  }

  json_object_set_string(aobj, "name", act->name);
  if (arazzo_action_type_to_str(act->type, &tstr) == CDD_C_SUCCESS)
    json_object_set_string(aobj, "type", tstr);
  if (act->workflow_id)
    json_object_set_string(aobj, "workflowId", act->workflow_id);
  if (act->step_id)
    json_object_set_string(aobj, "stepId", act->step_id);

  write_parameters_array(aobj, "parameters", act->parameters,
                         act->n_parameters);
  write_criteria_array(aobj, "criteria", act->criteria, act->n_criteria);
  write_arazzo_extensions(aobj, &act->extensions);
  *out_val = aval;
  return CDD_C_SUCCESS;
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
  JSON_Value *aval = json_value_init_object();
  JSON_Object *aobj = json_value_get_object(aval);
  const char *tstr = NULL;

  if (act->reusable.is_reusable) {
    json_object_set_string(aobj, "reference", act->reusable.reference);
    *out_val = aval;
    return CDD_C_SUCCESS;
  }

  json_object_set_string(aobj, "name", act->name);
  if (arazzo_action_type_to_str(act->type, &tstr) == CDD_C_SUCCESS)
    json_object_set_string(aobj, "type", tstr);
  if (act->workflow_id)
    json_object_set_string(aobj, "workflowId", act->workflow_id);
  if (act->step_id)
    json_object_set_string(aobj, "stepId", act->step_id);
  if (act->has_retry_after)
    json_object_set_number(aobj, "retryAfter", act->retry_after);
  if (act->has_retry_limit)
    json_object_set_number(aobj, "retryLimit", (double)act->retry_limit);

  write_parameters_array(aobj, "parameters", act->parameters,
                         act->n_parameters);
  write_criteria_array(aobj, "criteria", act->criteria, act->n_criteria);
  write_arazzo_extensions(aobj, &act->extensions);
  *out_val = aval;
  return CDD_C_SUCCESS;
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
  JSON_Value *out_val;
  JSON_Object *out_obj;
  size_t i;

  if (n_outputs == 0)
    return CDD_C_SUCCESS;

  out_val = json_value_init_object();
  out_obj = json_value_get_object(out_val);
  for (i = 0; i < n_outputs; ++i) {
    if (outputs[i].is_selector) {
      JSON_Value *sval = json_value_init_object();
      JSON_Object *sobj = json_value_get_object(sval);
      if (outputs[i].selector.context)
        json_object_set_string(sobj, "context", outputs[i].selector.context);
      json_object_set_string(sobj, "selector", outputs[i].selector.selector);
      write_criterion_or_expr_type(sobj, "type", outputs[i].selector.type,
                                   &outputs[i].selector.expression_type);
      write_arazzo_extensions(sobj, &outputs[i].selector.extensions);
      json_object_set_value(out_obj, outputs[i].key, sval);
    } else {
      json_object_set_string(out_obj, outputs[i].key,
                             outputs[i].expression_or_value);
    }
  }
  json_object_set_value(parent, "outputs", out_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Serializes a string array (`dependsOn`) into `parent` under `key`.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property name.
 * @param[in] items Array of strings.
 * @param[in] count Number of strings.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t write_string_list_field(JSON_Object *parent,
                                             const char *key,
                                             char *const *items, size_t count) {
  JSON_Value *aval;
  JSON_Array *arr;
  size_t i;
  if (count == 0)
    return CDD_C_SUCCESS;
  aval = json_value_init_array();
  arr = json_value_get_array(aval);
  for (i = 0; i < count; ++i) {
    json_array_append_string(arr, items[i]);
  }
  json_object_set_value(parent, key, aval);
  return CDD_C_SUCCESS;
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

  json_object_set_string(sobj, "stepId", step->step_id);
  if (step->description)
    json_object_set_string(sobj, "description", step->description);
  if (step->operation_id)
    json_object_set_string(sobj, "operationId", step->operation_id);
  if (step->operation_path)
    json_object_set_string(sobj, "operationPath", step->operation_path);
  if (step->channel_path)
    json_object_set_string(sobj, "channelPath", step->channel_path);
  if (step->workflow_id)
    json_object_set_string(sobj, "workflowId", step->workflow_id);
  if (step->has_timeout)
    json_object_set_number(sobj, "timeout", (double)step->timeout_ms);
  if (step->correlation_id)
    json_object_set_string(sobj, "correlationId", step->correlation_id);
  if (step->action == ARAZZO_ASYNC_SEND)
    json_object_set_string(sobj, "action", "send");
  else if (step->action == ARAZZO_ASYNC_RECEIVE)
    json_object_set_string(sobj, "action", "receive");

  write_string_list_field(sobj, "dependsOn", step->depends_on,
                          step->n_depends_on);
  write_parameters_array(sobj, "parameters", step->parameters,
                         step->n_parameters);

  if (step->has_request_body) {
    JSON_Value *rb_val = json_value_init_object();
    JSON_Object *rb_obj = json_value_get_object(rb_val);
    JSON_Value *pay_val = NULL;
    if (step->request_body.content_type)
      json_object_set_string(rb_obj, "contentType",
                             step->request_body.content_type);
    any_to_json_value(&step->request_body.payload, &pay_val);
    json_object_set_value(rb_obj, "payload", pay_val);
    if (step->request_body.n_replacements > 0) {
      JSON_Value *rarr_val = json_value_init_array();
      JSON_Array *rarr = json_value_get_array(rarr_val);
      for (i = 0; i < step->request_body.n_replacements; ++i) {
        const struct Arazzo_PayloadReplacement *rep =
            &step->request_body.replacements[i];
        JSON_Value *rval = json_value_init_object();
        JSON_Object *robj = json_value_get_object(rval);
        JSON_Value *rv = NULL;
        json_object_set_string(robj, "target", rep->target);
        write_criterion_or_expr_type(robj, "targetSelectorType",
                                     rep->target_selector_type,
                                     &rep->target_expression_type);
        any_to_json_value(&rep->value, &rv);
        json_object_set_value(robj, "value", rv);
        write_arazzo_extensions(robj, &rep->extensions);
        json_array_append_value(rarr, rval);
      }
      json_object_set_value(rb_obj, "replacements", rarr_val);
    }
    write_arazzo_extensions(rb_obj, &step->request_body.extensions);
    json_object_set_value(sobj, "requestBody", rb_val);
  }

  write_criteria_array(sobj, "successCriteria", step->success_criteria,
                       step->n_success_criteria);

  if (step->n_on_success > 0) {
    JSON_Value *sa_val = json_value_init_array();
    JSON_Array *sa_arr = json_value_get_array(sa_val);
    for (i = 0; i < step->n_on_success; ++i) {
      JSON_Value *item = NULL;
      write_success_action_object(&step->on_success[i], &item);
      json_array_append_value(sa_arr, item);
    }
    json_object_set_value(sobj, "onSuccess", sa_val);
  }

  if (step->n_on_failure > 0) {
    JSON_Value *fa_val = json_value_init_array();
    JSON_Array *fa_arr = json_value_get_array(fa_val);
    for (i = 0; i < step->n_on_failure; ++i) {
      JSON_Value *item = NULL;
      write_failure_action_object(&step->on_failure[i], &item);
      json_array_append_value(fa_arr, item);
    }
    json_object_set_value(sobj, "onFailure", fa_val);
  }

  write_outputs_map(sobj, step->outputs, step->n_outputs);
  write_arazzo_extensions(sobj, &step->extensions);
  return CDD_C_SUCCESS;
}

/**
 * @brief Serializes an `Arazzo_Spec` structure into a Parson `JSON_Value` tree.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_write_spec_to_json_value(const struct Arazzo_Spec *spec,
                                              JSON_Value **out_root) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Value *info_val;
  JSON_Object *info_obj;
  JSON_Value *sd_arr_val;
  JSON_Array *sd_arr;
  JSON_Value *wf_arr_val;
  JSON_Array *wf_arr;
  size_t i;
  size_t j;
  cdd_c_error_t rc;

  if (!out_root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_validate_spec(spec);
  if (rc != CDD_C_SUCCESS)
    return rc;

  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);

  json_object_set_string(root_obj, "arazzo", spec->arazzo);
  if (spec->self_uri)
    json_object_set_string(root_obj, "$self", spec->self_uri);

  info_val = json_value_init_object();
  info_obj = json_value_get_object(info_val);
  json_object_set_string(info_obj, "title", spec->info.title);
  if (spec->info.summary)
    json_object_set_string(info_obj, "summary", spec->info.summary);
  if (spec->info.description)
    json_object_set_string(info_obj, "description", spec->info.description);
  json_object_set_string(info_obj, "version", spec->info.version);
  write_arazzo_extensions(info_obj, &spec->info.extensions);
  json_object_set_value(root_obj, "info", info_val);

  sd_arr_val = json_value_init_array();
  sd_arr = json_value_get_array(sd_arr_val);
  for (i = 0; i < spec->n_source_descriptions; ++i) {
    JSON_Value *sd_val = json_value_init_object();
    JSON_Object *sd_obj = json_value_get_object(sd_val);
    const char *tstr = NULL;
    json_object_set_string(sd_obj, "name", spec->source_descriptions[i].name);
    json_object_set_string(sd_obj, "url", spec->source_descriptions[i].url);
    if (arazzo_source_type_to_str(spec->source_descriptions[i].type, &tstr) ==
        CDD_C_SUCCESS)
      json_object_set_string(sd_obj, "type", tstr);
    write_arazzo_extensions(sd_obj, &spec->source_descriptions[i].extensions);
    json_array_append_value(sd_arr, sd_val);
  }
  json_object_set_value(root_obj, "sourceDescriptions", sd_arr_val);

  wf_arr_val = json_value_init_array();
  wf_arr = json_value_get_array(wf_arr_val);
  for (i = 0; i < spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &spec->workflows[i];
    JSON_Value *wval = json_value_init_object();
    JSON_Object *wobj = json_value_get_object(wval);
    JSON_Value *sarr_val = json_value_init_array();
    JSON_Array *sarr = json_value_get_array(sarr_val);

    json_object_set_string(wobj, "workflowId", wf->workflow_id);
    if (wf->summary)
      json_object_set_string(wobj, "summary", wf->summary);
    if (wf->description)
      json_object_set_string(wobj, "description", wf->description);
    if (wf->has_inputs)
      write_schema_ref(wobj, "inputs", &wf->inputs);
    write_string_list_field(wobj, "dependsOn", wf->depends_on,
                            wf->n_depends_on);

    for (j = 0; j < wf->n_steps; ++j) {
      JSON_Value *st_val = json_value_init_object();
      write_step_object(json_value_get_object(st_val), &wf->steps[j]);
      json_array_append_value(sarr, st_val);
    }
    json_object_set_value(wobj, "steps", sarr_val);

    if (wf->n_success_actions > 0) {
      JSON_Value *sa_val = json_value_init_array();
      JSON_Array *sa_arr = json_value_get_array(sa_val);
      for (j = 0; j < wf->n_success_actions; ++j) {
        JSON_Value *item = NULL;
        write_success_action_object(&wf->success_actions[j], &item);
        json_array_append_value(sa_arr, item);
      }
      json_object_set_value(wobj, "successActions", sa_val);
    }

    if (wf->n_failure_actions > 0) {
      JSON_Value *fa_val = json_value_init_array();
      JSON_Array *fa_arr = json_value_get_array(fa_val);
      for (j = 0; j < wf->n_failure_actions; ++j) {
        JSON_Value *item = NULL;
        write_failure_action_object(&wf->failure_actions[j], &item);
        json_array_append_value(fa_arr, item);
      }
      json_object_set_value(wobj, "failureActions", fa_val);
    }

    write_outputs_map(wobj, wf->outputs, wf->n_outputs);
    write_parameters_array(wobj, "parameters", wf->parameters,
                           wf->n_parameters);
    write_arazzo_extensions(wobj, &wf->extensions);
    json_array_append_value(wf_arr, wval);
  }
  json_object_set_value(root_obj, "workflows", wf_arr_val);

  if (spec->has_components) {
    JSON_Value *cval = json_value_init_object();
    JSON_Object *cobj = json_value_get_object(cval);
    if (spec->components.n_inputs > 0) {
      JSON_Value *in_val = json_value_init_object();
      JSON_Object *in_obj = json_value_get_object(in_val);
      for (i = 0; i < spec->components.n_inputs; ++i) {
        write_schema_ref(in_obj, spec->components.input_keys[i],
                         &spec->components.inputs[i]);
      }
      json_object_set_value(cobj, "inputs", in_val);
    }
    if (spec->components.n_parameters > 0) {
      JSON_Value *pm_val = json_value_init_object();
      JSON_Object *pm_obj = json_value_get_object(pm_val);
      for (i = 0; i < spec->components.n_parameters; ++i) {
        JSON_Value *item = NULL;
        write_arazzo_param_obj(&spec->components.parameters[i], &item);
        json_object_set_value(pm_obj, spec->components.parameter_keys[i], item);
      }
      json_object_set_value(cobj, "parameters", pm_val);
    }
    if (spec->components.n_success_actions > 0) {
      JSON_Value *sa_val = json_value_init_object();
      JSON_Object *sa_obj = json_value_get_object(sa_val);
      for (i = 0; i < spec->components.n_success_actions; ++i) {
        JSON_Value *item = NULL;
        write_success_action_object(&spec->components.success_actions[i],
                                    &item);
        json_object_set_value(sa_obj, spec->components.success_action_keys[i],
                              item);
      }
      json_object_set_value(cobj, "successActions", sa_val);
    }
    if (spec->components.n_failure_actions > 0) {
      JSON_Value *fa_val = json_value_init_object();
      JSON_Object *fa_obj = json_value_get_object(fa_val);
      for (i = 0; i < spec->components.n_failure_actions; ++i) {
        JSON_Value *item = NULL;
        write_failure_action_object(&spec->components.failure_actions[i],
                                    &item);
        json_object_set_value(fa_obj, spec->components.failure_action_keys[i],
                              item);
      }
      json_object_set_value(cobj, "failureActions", fa_val);
    }
    write_arazzo_extensions(cobj, &spec->components.extensions);
    json_object_set_value(root_obj, "components", cval);
  }

  write_arazzo_extensions(root_obj, &spec->extensions);
  *out_root = root_val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Serializes an `Arazzo_Spec` structure into a formatted JSON string.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_write_spec_to_json(const struct Arazzo_Spec *spec,
                                        char **out_json) {
  JSON_Value *root = NULL;
  char *ser;
  cdd_c_error_t rc;

  if (!spec || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_write_spec_to_json_value(spec, &root);
  if (rc != CDD_C_SUCCESS)
    return rc;

  ser = json_serialize_to_string_pretty(root);
  json_value_free(root);
  *out_json = C_CDD_STRDUP(ser);
  json_free_serialized_string(ser);
  return *out_json ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}
