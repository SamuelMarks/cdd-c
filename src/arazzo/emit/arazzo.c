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
 * @brief Frees a Parson serialized string via a `*_free`-named helper.
 *
 * @param[in,out] str Serialized JSON string.
 * @return `CDD_C_SUCCESS`.
 */
static cdd_c_error_t arazzo_serialized_str_free(char *str) {
  return cdd_json_free_serialized_string(str);
}

/**
 * @brief Allocates a new Parson `JSON_Value *` object and retrieves its
 * underlying `JSON_Object *`.
 *
 * @param[out] out_val Output `JSON_Value *` object.
 * @param[out] out_obj Output `JSON_Object *` handle.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM/fault injection.
 */
static cdd_c_error_t arazzo_new_json_object(JSON_Value **out_val,
                                            JSON_Object **out_obj) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc = cdd_json_value_init_object(&v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_value_get_object(v, out_obj);
  if (rc != CDD_C_SUCCESS) {
    *out_obj = NULL;
    return cdd_c_keep_err(rc, cdd_json_value_free(v));
  }
  *out_val = v;
  return CDD_C_SUCCESS;
}

/**
 * @brief Allocates a new Parson `JSON_Value *` array and retrieves its
 * underlying `JSON_Array *`.
 *
 * @param[out] out_val Output `JSON_Value *` array.
 * @param[out] out_arr Output `JSON_Array *` handle.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM/fault injection.
 */
static cdd_c_error_t arazzo_new_json_array(JSON_Value **out_val,
                                           JSON_Array **out_arr) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc = cdd_json_value_init_array(&v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_value_get_array(v, out_arr);
  if (rc != CDD_C_SUCCESS) {
    *out_arr = NULL;
    return cdd_c_keep_err(rc, cdd_json_value_free(v));
  }
  *out_val = v;
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves an optional `Arazzo_SourceType` canonical string, setting
 * `*out_str` to NULL when unset (`ARAZZO_SOURCE_UNKNOWN`).
 *
 * @param[in] type Source type enum.
 * @param[out] out_str Canonical string or NULL.
 * @return `CDD_C_SUCCESS`, or error code on fault injection.
 */
static cdd_c_error_t arazzo_opt_source_type_str(enum Arazzo_SourceType type,
                                                const char **out_str) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_str = NULL;
  rc = arazzo_source_type_to_str(type, out_str);
  if (rc == CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves an optional `Arazzo_ParamIn` canonical string, setting
 * `*out_str` to NULL when unset (`ARAZZO_PARAM_IN_UNKNOWN`).
 *
 * @param[in] in_loc Parameter location enum.
 * @param[out] out_str Canonical string or NULL.
 * @return `CDD_C_SUCCESS`, or error code on fault injection.
 */
static cdd_c_error_t arazzo_opt_param_in_str(enum Arazzo_ParamIn in_loc,
                                             const char **out_str) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_str = NULL;
  rc = arazzo_param_in_to_str(in_loc, out_str);
  if (rc == CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves an optional `Arazzo_ActionType` canonical string, setting
 * `*out_str` to NULL when unset (`ARAZZO_ACTION_UNKNOWN`).
 *
 * @param[in] action Action type enum.
 * @param[out] out_str Canonical string or NULL.
 * @return `CDD_C_SUCCESS`, or error code on fault injection.
 */
static cdd_c_error_t arazzo_opt_action_type_str(enum Arazzo_ActionType action,
                                                const char **out_str) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_str = NULL;
  rc = arazzo_action_type_to_str(action, out_str);
  if (rc == CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
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
  cdd_c_error_t rc;
  for (i = 0; i < ext_list->count; ++i) {
    JSON_Value *val = NULL;
    rc = any_to_json_value(&ext_list->items[i].val, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc =
        cdd_json_object_set_value_safe(target_obj, ext_list->items[i].key, val);
    if (rc != CDD_C_SUCCESS)
      return rc;
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
  cdd_c_error_t rc;
  if (type == ARAZZO_CRITERION_REGEX) {
    return cdd_json_object_set_string(obj, key, "regex");
  }
  if (type == ARAZZO_CRITERION_JSONPATH) {
    return cdd_json_object_set_string(obj, key, "jsonpath");
  }
  if (type == ARAZZO_CRITERION_XPATH) {
    return cdd_json_object_set_string(obj, key, "xpath");
  }
  if (type == ARAZZO_CRITERION_EXPRESSION_TYPE) {
    JSON_Value *et_val = NULL;
    JSON_Object *et_obj = NULL;
    const char *dstr = "jsonpointer";
    rc = arazzo_new_json_object(&et_val, &et_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (et->type == ARAZZO_DIALECT_JSONPATH)
      dstr = "jsonpath";
    else if (et->type == ARAZZO_DIALECT_XPATH)
      dstr = "xpath";
    rc = cdd_json_object_set_string(et_obj, "type", dstr);
    if (rc != CDD_C_SUCCESS)
      goto err_et;
    if (et->version) {
      rc = cdd_json_object_set_string(et_obj, "version", et->version);
      if (rc != CDD_C_SUCCESS)
        goto err_et;
    }
    rc = write_arazzo_extensions(et_obj, &et->extensions);
    if (rc != CDD_C_SUCCESS)
      goto err_et;
    return cdd_json_object_set_value_safe(obj, key, et_val);
  err_et:;
    return cdd_c_keep_err(rc, cdd_json_value_free(et_val));
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t
Arazzo_Criterion_to_jsonObject(const struct Arazzo_Criterion *crit,
                               JSON_Object *cobj) {
  cdd_c_error_t rc;
  if (!crit || !cobj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_to_json_object(crit, &Arazzo_Criterion_type_info, cobj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_criterion_or_expr_type(cobj, "type", crit->type,
                                    &crit->expression_type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return write_arazzo_extensions(cobj, &crit->extensions);
}

cdd_c_error_t Arazzo_Info_to_jsonObject(const struct Arazzo_Info *info,
                                        JSON_Object *info_obj) {
  cdd_c_error_t rc;
  if (!info || !info_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_to_json_object(info, &Arazzo_Info_type_info, info_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return write_arazzo_extensions(info_obj, &info->extensions);
}

cdd_c_error_t Arazzo_SourceDescription_to_jsonObject(
    const struct Arazzo_SourceDescription *sd, JSON_Object *sd_obj) {
  const char *tstr = NULL;
  cdd_c_error_t rc;
  if (!sd || !sd_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_reflect_to_json_object(sd, &Arazzo_SourceDescription_type_info,
                                  sd_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = arazzo_opt_source_type_str(sd->type, &tstr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (tstr) {
    rc = cdd_json_object_set_string(sd_obj, "type", tstr);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return write_arazzo_extensions(sd_obj, &sd->extensions);
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
  cdd_c_error_t rc;

  if (n_criteria == 0)
    return CDD_C_SUCCESS;

  rc = arazzo_new_json_array(&arr_val, &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < n_criteria; ++i) {
    JSON_Value *cval = NULL;
    JSON_Object *cobj = NULL;
    rc = arazzo_new_json_object(&cval, &cobj);
    if (rc != CDD_C_SUCCESS)
      goto err_ca;
    rc = Arazzo_Criterion_to_jsonObject(&criteria[i], cobj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(cval),
                                               cdd_json_value_free(arr_val)));
    }
    rc = cdd_json_array_append_value_safe(arr, cval);
    if (rc != CDD_C_SUCCESS)
      goto err_ca;
  }
  return cdd_json_object_set_value_safe(parent, key, arr_val);

err_ca:;
  return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
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
  cdd_c_error_t rc;

  rc = arazzo_new_json_object(&pval, &pobj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (param->reusable.is_reusable) {
    rc = cdd_reflect_to_json_object(&param->reusable,
                                    &Arazzo_Reusable_type_info, pobj);
    if (rc != CDD_C_SUCCESS)
      goto err_pm;
    if (param->reusable.has_value_override) {
      JSON_Value *ov = NULL;
      rc = any_to_json_value(&param->reusable.value, &ov);
      if (rc != CDD_C_SUCCESS)
        goto err_pm;
      rc = cdd_json_object_set_value_safe(pobj, "value", ov);
      if (rc != CDD_C_SUCCESS)
        goto err_pm;
    }
    *out_val = pval;
    return CDD_C_SUCCESS;
  }

  rc = cdd_json_object_set_string(pobj, "name", param->name);
  if (rc != CDD_C_SUCCESS)
    goto err_pm;
  rc = arazzo_opt_param_in_str(param->in, &in_str);
  if (rc != CDD_C_SUCCESS)
    goto err_pm;
  if (in_str) {
    rc = cdd_json_object_set_string(pobj, "in", in_str);
    if (rc != CDD_C_SUCCESS)
      goto err_pm;
  }

  if (param->is_selector) {
    JSON_Value *sval = NULL;
    JSON_Object *sobj = NULL;
    rc = arazzo_new_json_object(&sval, &sobj);
    if (rc != CDD_C_SUCCESS)
      goto err_pm;
    rc = cdd_reflect_to_json_object(&param->selector,
                                    &Arazzo_Selector_type_info, sobj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(sval),
                                               cdd_json_value_free(pval)));
    }
    rc = write_criterion_or_expr_type(sobj, "type", param->selector.type,
                                      &param->selector.expression_type);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(sval),
                                               cdd_json_value_free(pval)));
    }
    rc = write_arazzo_extensions(sobj, &param->selector.extensions);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(sval),
                                               cdd_json_value_free(pval)));
    }
    rc = cdd_json_object_set_value_safe(pobj, "value", sval);
    if (rc != CDD_C_SUCCESS)
      goto err_pm;
  } else {
    JSON_Value *vval = NULL;
    rc = any_to_json_value(&param->value, &vval);
    if (rc != CDD_C_SUCCESS)
      goto err_pm;
    rc = cdd_json_object_set_value_safe(pobj, "value", vval);
    if (rc != CDD_C_SUCCESS)
      goto err_pm;
  }

  rc = write_arazzo_extensions(pobj, &param->extensions);
  if (rc != CDD_C_SUCCESS)
    goto err_pm;
  *out_val = pval;
  return CDD_C_SUCCESS;

err_pm:;
  return cdd_c_keep_err(rc, cdd_json_value_free(pval));
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
  cdd_c_error_t rc;

  if (n_params == 0)
    return CDD_C_SUCCESS;

  rc = arazzo_new_json_array(&arr_val, &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < n_params; ++i) {
    JSON_Value *pval = NULL;
    rc = write_arazzo_param_obj(&params[i], &pval);
    if (rc != CDD_C_SUCCESS)
      goto err_pa;
    rc = cdd_json_array_append_value_safe(arr, pval);
    if (rc != CDD_C_SUCCESS)
      goto err_pa;
  }
  return cdd_json_object_set_value_safe(parent, key, arr_val);

err_pa:;
  return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
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
  cdd_c_error_t rc;

  rc = arazzo_new_json_object(&aval, &aobj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (act->reusable.is_reusable) {
    rc = cdd_reflect_to_json_object(&act->reusable, &Arazzo_Reusable_type_info,
                                    aobj);
    if (rc != CDD_C_SUCCESS)
      goto err_sa;
    *out_val = aval;
    return CDD_C_SUCCESS;
  }

  rc = cdd_reflect_to_json_object(act, &Arazzo_SuccessAction_type_info, aobj);
  if (rc != CDD_C_SUCCESS)
    goto err_sa;
  rc = arazzo_opt_action_type_str(act->type, &tstr);
  if (rc != CDD_C_SUCCESS)
    goto err_sa;
  if (tstr) {
    rc = cdd_json_object_set_string(aobj, "type", tstr);
    if (rc != CDD_C_SUCCESS)
      goto err_sa;
  }

  rc = write_parameters_array(aobj, "parameters", act->parameters,
                              act->n_parameters);
  if (rc != CDD_C_SUCCESS)
    goto err_sa;
  rc = write_criteria_array(aobj, "criteria", act->criteria, act->n_criteria);
  if (rc != CDD_C_SUCCESS)
    goto err_sa;
  rc = write_arazzo_extensions(aobj, &act->extensions);
  if (rc != CDD_C_SUCCESS)
    goto err_sa;
  *out_val = aval;
  return CDD_C_SUCCESS;

err_sa:;
  return cdd_c_keep_err(rc, cdd_json_value_free(aval));
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
  cdd_c_error_t rc;

  rc = arazzo_new_json_object(&aval, &aobj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (act->reusable.is_reusable) {
    rc = cdd_reflect_to_json_object(&act->reusable, &Arazzo_Reusable_type_info,
                                    aobj);
    if (rc != CDD_C_SUCCESS)
      goto err_fa;
    *out_val = aval;
    return CDD_C_SUCCESS;
  }

  rc = cdd_reflect_to_json_object(act, &Arazzo_FailureAction_type_info, aobj);
  if (rc != CDD_C_SUCCESS)
    goto err_fa;
  rc = arazzo_opt_action_type_str(act->type, &tstr);
  if (rc != CDD_C_SUCCESS)
    goto err_fa;
  if (tstr) {
    rc = cdd_json_object_set_string(aobj, "type", tstr);
    if (rc != CDD_C_SUCCESS)
      goto err_fa;
  }
  if (act->has_retry_after) {
    rc = cdd_json_object_set_number(aobj, "retryAfter", act->retry_after);
    if (rc != CDD_C_SUCCESS)
      goto err_fa;
  }
  if (act->has_retry_limit) {
    rc = cdd_json_object_set_number(aobj, "retryLimit",
                                    (double)act->retry_limit);
    if (rc != CDD_C_SUCCESS)
      goto err_fa;
  }

  rc = write_parameters_array(aobj, "parameters", act->parameters,
                              act->n_parameters);
  if (rc != CDD_C_SUCCESS)
    goto err_fa;
  rc = write_criteria_array(aobj, "criteria", act->criteria, act->n_criteria);
  if (rc != CDD_C_SUCCESS)
    goto err_fa;
  rc = write_arazzo_extensions(aobj, &act->extensions);
  if (rc != CDD_C_SUCCESS)
    goto err_fa;
  *out_val = aval;
  return CDD_C_SUCCESS;

err_fa:;
  return cdd_c_keep_err(rc, cdd_json_value_free(aval));
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
  cdd_c_error_t rc;

  if (n_outputs == 0)
    return CDD_C_SUCCESS;

  rc = arazzo_new_json_object(&out_val, &out_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < n_outputs; ++i) {
    if (outputs[i].is_selector) {
      JSON_Value *sval = NULL;
      JSON_Object *sobj = NULL;
      rc = arazzo_new_json_object(&sval, &sobj);
      if (rc != CDD_C_SUCCESS)
        goto err_om;
      rc = cdd_reflect_to_json_object(&outputs[i].selector,
                                      &Arazzo_Selector_type_info, sobj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(sval),
                                                 cdd_json_value_free(out_val)));
      }
      rc = write_criterion_or_expr_type(sobj, "type", outputs[i].selector.type,
                                        &outputs[i].selector.expression_type);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(sval),
                                                 cdd_json_value_free(out_val)));
      }
      rc = write_arazzo_extensions(sobj, &outputs[i].selector.extensions);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(sval),
                                                 cdd_json_value_free(out_val)));
      }
      rc = cdd_json_object_set_value_safe(out_obj, outputs[i].key, sval);
      if (rc != CDD_C_SUCCESS)
        goto err_om;
    } else {
      rc = cdd_json_object_set_string(out_obj, outputs[i].key,
                                      outputs[i].expression_or_value);
      if (rc != CDD_C_SUCCESS)
        goto err_om;
    }
  }
  return cdd_json_object_set_value_safe(parent, "outputs", out_val);

err_om:;
  return cdd_c_keep_err(rc, cdd_json_value_free(out_val));
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
  cdd_c_error_t rc;
  if (count == 0)
    return CDD_C_SUCCESS;
  rc = arazzo_new_json_array(&aval, &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < count; ++i) {
    rc = cdd_json_array_append_string(arr, items[i]);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(aval));
    }
  }
  return cdd_json_object_set_value_safe(parent, key, aval);
}

/**
 * @brief Serializes an array of `Arazzo_SuccessAction` into `parent` under
 * `key`.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property name (`"onSuccess"` or `"successActions"`).
 * @param[in] actions Success actions array.
 * @param[in] n_actions Number of actions.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_success_actions_array(JSON_Object *parent, const char *key,
                            const struct Arazzo_SuccessAction *actions,
                            size_t n_actions) {
  JSON_Value *sa_val = NULL;
  JSON_Array *sa_arr = NULL;
  size_t i;
  cdd_c_error_t rc;
  if (n_actions == 0)
    return CDD_C_SUCCESS;
  rc = arazzo_new_json_array(&sa_val, &sa_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < n_actions; ++i) {
    JSON_Value *item = NULL;
    rc = write_success_action_object(&actions[i], &item);
    if (rc != CDD_C_SUCCESS)
      goto err_saa;
    rc = cdd_json_array_append_value_safe(sa_arr, item);
    if (rc != CDD_C_SUCCESS)
      goto err_saa;
  }
  return cdd_json_object_set_value_safe(parent, key, sa_val);

err_saa:;
  return cdd_c_keep_err(rc, cdd_json_value_free(sa_val));
}

/**
 * @brief Serializes an array of `Arazzo_FailureAction` into `parent` under
 * `key`.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property name (`"onFailure"` or `"failureActions"`).
 * @param[in] actions Failure actions array.
 * @param[in] n_actions Number of actions.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
write_failure_actions_array(JSON_Object *parent, const char *key,
                            const struct Arazzo_FailureAction *actions,
                            size_t n_actions) {
  JSON_Value *fa_val = NULL;
  JSON_Array *fa_arr = NULL;
  size_t i;
  cdd_c_error_t rc;
  if (n_actions == 0)
    return CDD_C_SUCCESS;
  rc = arazzo_new_json_array(&fa_val, &fa_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < n_actions; ++i) {
    JSON_Value *item = NULL;
    rc = write_failure_action_object(&actions[i], &item);
    if (rc != CDD_C_SUCCESS)
      goto err_faa;
    rc = cdd_json_array_append_value_safe(fa_arr, item);
    if (rc != CDD_C_SUCCESS)
      goto err_faa;
  }
  return cdd_json_object_set_value_safe(parent, key, fa_val);

err_faa:;
  return cdd_c_keep_err(rc, cdd_json_value_free(fa_val));
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
  cdd_c_error_t rc;

  rc = cdd_reflect_to_json_object(step, &Arazzo_Step_type_info, sobj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (step->has_timeout) {
    rc = cdd_json_object_set_number(sobj, "timeout", (double)step->timeout_ms);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (step->action == ARAZZO_ASYNC_SEND) {
    rc = cdd_json_object_set_string(sobj, "action", "send");
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (step->action == ARAZZO_ASYNC_RECEIVE) {
    rc = cdd_json_object_set_string(sobj, "action", "receive");
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = write_string_list_field(sobj, "dependsOn", step->depends_on,
                               step->n_depends_on);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_parameters_array(sobj, "parameters", step->parameters,
                              step->n_parameters);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (step->has_request_body) {
    JSON_Value *rb_val = NULL;
    JSON_Object *rb_obj = NULL;
    JSON_Value *pay_val = NULL;
    rc = arazzo_new_json_object(&rb_val, &rb_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_reflect_to_json_object(&step->request_body,
                                    &Arazzo_RequestBody_type_info, rb_obj);
    if (rc != CDD_C_SUCCESS)
      goto err_rb;
    rc = any_to_json_value(&step->request_body.payload, &pay_val);
    if (rc != CDD_C_SUCCESS)
      goto err_rb;
    rc = cdd_json_object_set_value_safe(rb_obj, "payload", pay_val);
    if (rc != CDD_C_SUCCESS)
      goto err_rb;
    if (step->request_body.n_replacements > 0) {
      JSON_Value *rarr_val = NULL;
      JSON_Array *rarr = NULL;
      rc = arazzo_new_json_array(&rarr_val, &rarr);
      if (rc != CDD_C_SUCCESS)
        goto err_rb;
      for (i = 0; i < step->request_body.n_replacements; ++i) {
        const struct Arazzo_PayloadReplacement *rep =
            &step->request_body.replacements[i];
        JSON_Value *rval = NULL;
        JSON_Object *robj = NULL;
        JSON_Value *rv = NULL;
        rc = arazzo_new_json_object(&rval, &robj);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc,
                                cdd_c_keep_err(cdd_json_value_free(rarr_val),
                                               cdd_json_value_free(rb_val)));
        }
        rc = cdd_reflect_to_json_object(
            rep, &Arazzo_PayloadReplacement_type_info, robj);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(cdd_json_value_free(rval),
                                 cdd_c_keep_err(cdd_json_value_free(rarr_val),
                                                cdd_json_value_free(rb_val))));
        }
        rc = write_criterion_or_expr_type(robj, "targetSelectorType",
                                          rep->target_selector_type,
                                          &rep->target_expression_type);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(cdd_json_value_free(rval),
                                 cdd_c_keep_err(cdd_json_value_free(rarr_val),
                                                cdd_json_value_free(rb_val))));
        }
        rc = any_to_json_value(&rep->value, &rv);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(cdd_json_value_free(rval),
                                 cdd_c_keep_err(cdd_json_value_free(rarr_val),
                                                cdd_json_value_free(rb_val))));
        }
        rc = cdd_json_object_set_value_safe(robj, "value", rv);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(cdd_json_value_free(rval),
                                 cdd_c_keep_err(cdd_json_value_free(rarr_val),
                                                cdd_json_value_free(rb_val))));
        }
        rc = write_arazzo_extensions(robj, &rep->extensions);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(cdd_json_value_free(rval),
                                 cdd_c_keep_err(cdd_json_value_free(rarr_val),
                                                cdd_json_value_free(rb_val))));
        }
        rc = cdd_json_array_append_value_safe(rarr, rval);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc,
                                cdd_c_keep_err(cdd_json_value_free(rarr_val),
                                               cdd_json_value_free(rb_val)));
        }
      }
      rc = cdd_json_object_set_value_safe(rb_obj, "replacements", rarr_val);
      if (rc != CDD_C_SUCCESS)
        goto err_rb;
    }
    rc = write_arazzo_extensions(rb_obj, &step->request_body.extensions);
    if (rc != CDD_C_SUCCESS)
      goto err_rb;
    rc = cdd_json_object_set_value_safe(sobj, "requestBody", rb_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    goto after_rb;
  err_rb:;
    return cdd_c_keep_err(rc, cdd_json_value_free(rb_val));
  after_rb:;
  }

  rc = write_criteria_array(sobj, "successCriteria", step->success_criteria,
                            step->n_success_criteria);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_success_actions_array(sobj, "onSuccess", step->on_success,
                                   step->n_on_success);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_failure_actions_array(sobj, "onFailure", step->on_failure,
                                   step->n_on_failure);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_outputs_map(sobj, step->outputs, step->n_outputs);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return write_arazzo_extensions(sobj, &step->extensions);
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
  *out_root = NULL;

  rc = arazzo_validate_spec(spec);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = arazzo_new_json_object(&root_val, &root_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_json_object_set_string(root_obj, "arazzo", spec->arazzo);
  if (rc != CDD_C_SUCCESS)
    goto err_root;
  if (spec->self_uri) {
    rc = cdd_json_object_set_string(root_obj, "$self", spec->self_uri);
    if (rc != CDD_C_SUCCESS)
      goto err_root;
  }

  rc = arazzo_new_json_object(&info_val, &info_obj);
  if (rc != CDD_C_SUCCESS)
    goto err_root;
  rc = Arazzo_Info_to_jsonObject(&spec->info, info_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(info_val),
                                             cdd_json_value_free(root_val)));
  }
  rc = cdd_json_object_set_value_safe(root_obj, "info", info_val);
  if (rc != CDD_C_SUCCESS)
    goto err_root;

  rc = arazzo_new_json_array(&sd_arr_val, &sd_arr);
  if (rc != CDD_C_SUCCESS)
    goto err_root;
  for (i = 0; i < spec->n_source_descriptions; ++i) {
    JSON_Value *sd_val = NULL;
    JSON_Object *sd_obj = NULL;
    rc = arazzo_new_json_object(&sd_val, &sd_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(sd_arr_val),
                                               cdd_json_value_free(root_val)));
    }
    rc = Arazzo_SourceDescription_to_jsonObject(&spec->source_descriptions[i],
                                                sd_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(cdd_json_value_free(sd_val),
                             cdd_c_keep_err(cdd_json_value_free(sd_arr_val),
                                            cdd_json_value_free(root_val))));
    }
    rc = cdd_json_array_append_value_safe(sd_arr, sd_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(sd_arr_val),
                                               cdd_json_value_free(root_val)));
    }
  }
  rc = cdd_json_object_set_value_safe(root_obj, "sourceDescriptions",
                                      sd_arr_val);
  if (rc != CDD_C_SUCCESS)
    goto err_root;

  rc = arazzo_new_json_array(&wf_arr_val, &wf_arr);
  if (rc != CDD_C_SUCCESS)
    goto err_root;
  for (i = 0; i < spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &spec->workflows[i];
    JSON_Value *wval = NULL;
    JSON_Object *wobj = NULL;
    JSON_Value *sarr_val = NULL;
    JSON_Array *sarr = NULL;
    rc = arazzo_new_json_object(&wval, &wobj);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_arr;
    rc = arazzo_new_json_array(&sarr_val, &sarr);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(cdd_json_value_free(wval),
                             cdd_c_keep_err(cdd_json_value_free(wf_arr_val),
                                            cdd_json_value_free(root_val))));
    }

    rc = cdd_reflect_to_json_object(wf, &Arazzo_Workflow_type_info, wobj);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_step;
    if (wf->has_inputs) {
      rc = write_schema_ref(wobj, "inputs", &wf->inputs);
      if (rc != CDD_C_SUCCESS)
        goto err_wf_step;
    }
    rc = write_string_list_field(wobj, "dependsOn", wf->depends_on,
                                 wf->n_depends_on);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_step;

    for (j = 0; j < wf->n_steps; ++j) {
      JSON_Value *st_val = NULL;
      JSON_Object *st_obj = NULL;
      rc = arazzo_new_json_object(&st_val, &st_obj);
      if (rc != CDD_C_SUCCESS)
        goto err_wf_step;
      rc = write_step_object(st_obj, &wf->steps[j]);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(
            rc, cdd_c_keep_err(
                    cdd_json_value_free(st_val),
                    cdd_c_keep_err(
                        cdd_json_value_free(sarr_val),
                        cdd_c_keep_err(
                            cdd_json_value_free(wval),
                            cdd_c_keep_err(cdd_json_value_free(wf_arr_val),
                                           cdd_json_value_free(root_val))))));
      }
      rc = cdd_json_array_append_value_safe(sarr, st_val);
      if (rc != CDD_C_SUCCESS)
        goto err_wf_step;
    }
    rc = cdd_json_object_set_value_safe(wobj, "steps", sarr_val);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_item;
    sarr_val = NULL;

    rc = write_success_actions_array(
        wobj, "successActions", wf->success_actions, wf->n_success_actions);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_item;
    rc = write_failure_actions_array(
        wobj, "failureActions", wf->failure_actions, wf->n_failure_actions);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_item;
    rc = write_outputs_map(wobj, wf->outputs, wf->n_outputs);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_item;
    rc = write_parameters_array(wobj, "parameters", wf->parameters,
                                wf->n_parameters);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_item;
    rc = write_arazzo_extensions(wobj, &wf->extensions);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_item;
    rc = cdd_json_array_append_value_safe(wf_arr, wval);
    if (rc != CDD_C_SUCCESS)
      goto err_wf_arr;
    continue;

  err_wf_step:;
    return cdd_c_keep_err(
        rc, cdd_c_keep_err(
                cdd_json_value_free(sarr_val),
                cdd_c_keep_err(cdd_json_value_free(wval),
                               cdd_c_keep_err(cdd_json_value_free(wf_arr_val),
                                              cdd_json_value_free(root_val)))));
  err_wf_item:;
    return cdd_c_keep_err(
        rc, cdd_c_keep_err(cdd_json_value_free(wval),
                           cdd_c_keep_err(cdd_json_value_free(wf_arr_val),
                                          cdd_json_value_free(root_val))));
  }
  rc = cdd_json_object_set_value_safe(root_obj, "workflows", wf_arr_val);
  if (rc != CDD_C_SUCCESS)
    goto err_root;
  wf_arr_val = NULL;

  if (spec->has_components) {
    JSON_Value *cval = NULL;
    JSON_Object *cobj = NULL;
    rc = arazzo_new_json_object(&cval, &cobj);
    if (rc != CDD_C_SUCCESS)
      goto err_root;
    if (spec->components.n_inputs > 0) {
      JSON_Value *in_val = NULL;
      JSON_Object *in_obj = NULL;
      rc = arazzo_new_json_object(&in_val, &in_obj);
      if (rc != CDD_C_SUCCESS)
        goto err_comp;
      for (i = 0; i < spec->components.n_inputs; ++i) {
        rc = write_schema_ref(in_obj, spec->components.input_keys[i],
                              &spec->components.inputs[i]);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc,
              cdd_c_keep_err(cdd_json_value_free(in_val),
                             cdd_c_keep_err(cdd_json_value_free(cval),
                                            cdd_json_value_free(root_val))));
        }
      }
      rc = cdd_json_object_set_value_safe(cobj, "inputs", in_val);
      if (rc != CDD_C_SUCCESS)
        goto err_comp;
    }
    if (spec->components.n_parameters > 0) {
      JSON_Value *pm_val = NULL;
      JSON_Object *pm_obj = NULL;
      rc = arazzo_new_json_object(&pm_val, &pm_obj);
      if (rc != CDD_C_SUCCESS)
        goto err_comp;
      for (i = 0; i < spec->components.n_parameters; ++i) {
        JSON_Value *item = NULL;
        rc = write_arazzo_param_obj(&spec->components.parameters[i], &item);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc,
              cdd_c_keep_err(cdd_json_value_free(pm_val),
                             cdd_c_keep_err(cdd_json_value_free(cval),
                                            cdd_json_value_free(root_val))));
        }
        rc = cdd_json_object_set_value_safe(
            pm_obj, spec->components.parameter_keys[i], item);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc,
              cdd_c_keep_err(cdd_json_value_free(pm_val),
                             cdd_c_keep_err(cdd_json_value_free(cval),
                                            cdd_json_value_free(root_val))));
        }
      }
      rc = cdd_json_object_set_value_safe(cobj, "parameters", pm_val);
      if (rc != CDD_C_SUCCESS)
        goto err_comp;
    }
    if (spec->components.n_success_actions > 0) {
      JSON_Value *sa_val = NULL;
      JSON_Object *sa_obj = NULL;
      rc = arazzo_new_json_object(&sa_val, &sa_obj);
      if (rc != CDD_C_SUCCESS)
        goto err_comp;
      for (i = 0; i < spec->components.n_success_actions; ++i) {
        JSON_Value *item = NULL;
        rc = write_success_action_object(&spec->components.success_actions[i],
                                         &item);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc,
              cdd_c_keep_err(cdd_json_value_free(sa_val),
                             cdd_c_keep_err(cdd_json_value_free(cval),
                                            cdd_json_value_free(root_val))));
        }
        rc = cdd_json_object_set_value_safe(
            sa_obj, spec->components.success_action_keys[i], item);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc,
              cdd_c_keep_err(cdd_json_value_free(sa_val),
                             cdd_c_keep_err(cdd_json_value_free(cval),
                                            cdd_json_value_free(root_val))));
        }
      }
      rc = cdd_json_object_set_value_safe(cobj, "successActions", sa_val);
      if (rc != CDD_C_SUCCESS)
        goto err_comp;
    }
    if (spec->components.n_failure_actions > 0) {
      JSON_Value *fa_val = NULL;
      JSON_Object *fa_obj = NULL;
      rc = arazzo_new_json_object(&fa_val, &fa_obj);
      if (rc != CDD_C_SUCCESS)
        goto err_comp;
      for (i = 0; i < spec->components.n_failure_actions; ++i) {
        JSON_Value *item = NULL;
        rc = write_failure_action_object(&spec->components.failure_actions[i],
                                         &item);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc,
              cdd_c_keep_err(cdd_json_value_free(fa_val),
                             cdd_c_keep_err(cdd_json_value_free(cval),
                                            cdd_json_value_free(root_val))));
        }
        rc = cdd_json_object_set_value_safe(
            fa_obj, spec->components.failure_action_keys[i], item);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc,
              cdd_c_keep_err(cdd_json_value_free(fa_val),
                             cdd_c_keep_err(cdd_json_value_free(cval),
                                            cdd_json_value_free(root_val))));
        }
      }
      rc = cdd_json_object_set_value_safe(cobj, "failureActions", fa_val);
      if (rc != CDD_C_SUCCESS)
        goto err_comp;
    }
    rc = write_arazzo_extensions(cobj, &spec->components.extensions);
    if (rc != CDD_C_SUCCESS)
      goto err_comp;
    rc = cdd_json_object_set_value_safe(root_obj, "components", cval);
    if (rc != CDD_C_SUCCESS)
      goto err_root;
    goto after_comp;
  err_comp:;
    return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(cval),
                                             cdd_json_value_free(root_val)));
  after_comp:;
  }

  rc = write_arazzo_extensions(root_obj, &spec->extensions);
  if (rc != CDD_C_SUCCESS)
    goto err_root;
  *out_root = root_val;
  return CDD_C_SUCCESS;

err_wf_arr:;
  return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(wf_arr_val),
                                           cdd_json_value_free(root_val)));
err_root:;
  return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
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
  *out_json = NULL;

  rc = arazzo_write_spec_to_json_value(spec, &root);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_json_serialize_to_string_pretty(root, &ser);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }
  *out_json = C_CDD_STRDUP(ser);
  if (!*out_json) {
    return cdd_c_keep_err(CDD_C_ERROR_MEMORY,
                          cdd_c_keep_err(arazzo_serialized_str_free(ser),
                                         cdd_json_value_free(root)));
  }
  return cdd_c_keep_err(arazzo_serialized_str_free(ser),
                        cdd_json_value_free(root));
}
