/**
 * @file arazzo_free.c
 * @brief Lifecycle, cleanup, and enum conversion routines for Arazzo v1.1.0 IR.
 *
 * Implements strict C89 memory management, initialization, extension list
 * helpers, and strongly-typed enum conversions for Arazzo Specification v1.1.0.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "arazzo/parse/arazzo_types.h"
#include "c_cdd/memory.h"
#include "openapi/parse/openapi_internal.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Frees an `Arazzo_ExpressionType` structure's dynamic fields.
 *
 * @param[in,out] et Pointer to the expression type structure.
 */
static void free_arazzo_expression_type(struct Arazzo_ExpressionType *et) {
  C_CDD_FREE(et->version);
  et->version = NULL;
  arazzo_extension_list_free(&et->extensions);
}

/**
 * @brief Frees an `Arazzo_Criterion` structure's dynamic fields.
 *
 * @param[in,out] crit Pointer to the criterion structure.
 */
static void free_arazzo_criterion(struct Arazzo_Criterion *crit) {
  C_CDD_FREE(crit->context);
  C_CDD_FREE(crit->condition);
  free_arazzo_expression_type(&crit->expression_type);
  arazzo_extension_list_free(&crit->extensions);
  memset(crit, 0, sizeof(*crit));
}

/**
 * @brief Frees an `Arazzo_Selector` structure's dynamic fields.
 *
 * @param[in,out] sel Pointer to the selector structure.
 */
static void free_arazzo_selector(struct Arazzo_Selector *sel) {
  C_CDD_FREE(sel->context);
  C_CDD_FREE(sel->selector);
  free_arazzo_expression_type(&sel->expression_type);
  arazzo_extension_list_free(&sel->extensions);
  memset(sel, 0, sizeof(*sel));
}

/**
 * @brief Frees an `Arazzo_Reusable` structure's dynamic fields.
 *
 * @param[in,out] r Pointer to the reusable reference structure.
 */
static void free_arazzo_reusable(struct Arazzo_Reusable *r) {
  C_CDD_FREE(r->reference);
  free_any_value(&r->value);
  memset(r, 0, sizeof(*r));
}

/**
 * @brief Frees an `Arazzo_Parameter` structure's dynamic fields.
 *
 * @param[in,out] param Pointer to the parameter structure.
 */
static void free_arazzo_parameter(struct Arazzo_Parameter *param) {
  C_CDD_FREE(param->name);
  free_any_value(&param->value);
  free_arazzo_selector(&param->selector);
  arazzo_runtime_expr_free(&param->parsed_expr);
  free_arazzo_reusable(&param->reusable);
  arazzo_extension_list_free(&param->extensions);
  memset(param, 0, sizeof(*param));
}

/**
 * @brief Frees an array of `Arazzo_Parameter` structures.
 *
 * @param[in,out] params Array of parameters (may be NULL).
 * @param[in] n_params Number of parameters.
 */
static void free_arazzo_parameters_array(struct Arazzo_Parameter *params,
                                         size_t n_params) {
  size_t i;
  for (i = 0; i < n_params; ++i) {
    free_arazzo_parameter(&params[i]);
  }
  C_CDD_FREE(params);
}

/**
 * @brief Frees an array of `Arazzo_Criterion` structures.
 *
 * @param[in,out] criteria Array of criteria (may be NULL).
 * @param[in] n_criteria Number of criteria.
 */
static void free_arazzo_criteria_array(struct Arazzo_Criterion *criteria,
                                       size_t n_criteria) {
  size_t i;
  for (i = 0; i < n_criteria; ++i) {
    free_arazzo_criterion(&criteria[i]);
  }
  C_CDD_FREE(criteria);
}

/**
 * @brief Frees an `Arazzo_SuccessAction` structure's dynamic fields.
 *
 * @param[in,out] act Pointer to the success action structure.
 */
static void free_arazzo_success_action(struct Arazzo_SuccessAction *act) {
  C_CDD_FREE(act->name);
  C_CDD_FREE(act->workflow_id);
  C_CDD_FREE(act->step_id);
  free_arazzo_parameters_array(act->parameters, act->n_parameters);
  free_arazzo_criteria_array(act->criteria, act->n_criteria);
  free_arazzo_reusable(&act->reusable);
  arazzo_extension_list_free(&act->extensions);
  memset(act, 0, sizeof(*act));
}

/**
 * @brief Frees an `Arazzo_FailureAction` structure's dynamic fields.
 *
 * @param[in,out] act Pointer to the failure action structure.
 */
static void free_arazzo_failure_action(struct Arazzo_FailureAction *act) {
  C_CDD_FREE(act->name);
  C_CDD_FREE(act->workflow_id);
  C_CDD_FREE(act->step_id);
  free_arazzo_parameters_array(act->parameters, act->n_parameters);
  free_arazzo_criteria_array(act->criteria, act->n_criteria);
  free_arazzo_reusable(&act->reusable);
  arazzo_extension_list_free(&act->extensions);
  memset(act, 0, sizeof(*act));
}

/**
 * @brief Frees an array of `Arazzo_OutputMapEntry` structures.
 *
 * @param[in,out] outputs Array of output map entries (may be NULL).
 * @param[in] n_outputs Number of output entries.
 */
static void free_arazzo_outputs_array(struct Arazzo_OutputMapEntry *outputs,
                                      size_t n_outputs) {
  size_t i;
  for (i = 0; i < n_outputs; ++i) {
    C_CDD_FREE(outputs[i].key);
    C_CDD_FREE(outputs[i].expression_or_value);
    free_arazzo_selector(&outputs[i].selector);
    arazzo_runtime_expr_free(&outputs[i].parsed_expr);
  }
  C_CDD_FREE(outputs);
}

/**
 * @brief Frees an `Arazzo_Step` structure's dynamic fields.
 *
 * @param[in,out] step Pointer to the step structure.
 */
static void free_arazzo_step(struct Arazzo_Step *step) {
  size_t i;
  C_CDD_FREE(step->step_id);
  C_CDD_FREE(step->description);
  C_CDD_FREE(step->operation_id);
  C_CDD_FREE(step->operation_path);
  C_CDD_FREE(step->channel_path);
  C_CDD_FREE(step->workflow_id);
  free_arazzo_parameters_array(step->parameters, step->n_parameters);
  C_CDD_FREE(step->request_body.content_type);
  free_any_value(&step->request_body.payload);
  for (i = 0; i < step->request_body.n_replacements; ++i) {
    C_CDD_FREE(step->request_body.replacements[i].target);
    free_arazzo_expression_type(
        &step->request_body.replacements[i].target_expression_type);
    free_any_value(&step->request_body.replacements[i].value);
    arazzo_runtime_expr_free(&step->request_body.replacements[i].parsed_expr);
    arazzo_extension_list_free(&step->request_body.replacements[i].extensions);
  }
  C_CDD_FREE(step->request_body.replacements);
  arazzo_extension_list_free(&step->request_body.extensions);
  free_arazzo_criteria_array(step->success_criteria, step->n_success_criteria);
  for (i = 0; i < step->n_on_success; ++i) {
    free_arazzo_success_action(&step->on_success[i]);
  }
  C_CDD_FREE(step->on_success);
  for (i = 0; i < step->n_on_failure; ++i) {
    free_arazzo_failure_action(&step->on_failure[i]);
  }
  C_CDD_FREE(step->on_failure);
  free_arazzo_outputs_array(step->outputs, step->n_outputs);
  C_CDD_FREE(step->correlation_id);
  free_string_array(step->depends_on, step->n_depends_on);
  arazzo_extension_list_free(&step->extensions);
  memset(step, 0, sizeof(*step));
}

/**
 * @brief Frees an `Arazzo_Workflow` structure's dynamic fields.
 *
 * @param[in,out] wf Pointer to the workflow structure.
 */
static void free_arazzo_workflow(struct Arazzo_Workflow *wf) {
  size_t i;
  C_CDD_FREE(wf->workflow_id);
  C_CDD_FREE(wf->summary);
  C_CDD_FREE(wf->description);
  free_schema_ref_content(&wf->inputs);
  free_string_array(wf->depends_on, wf->n_depends_on);
  for (i = 0; i < wf->n_steps; ++i) {
    free_arazzo_step(&wf->steps[i]);
  }
  C_CDD_FREE(wf->steps);
  for (i = 0; i < wf->n_success_actions; ++i) {
    free_arazzo_success_action(&wf->success_actions[i]);
  }
  C_CDD_FREE(wf->success_actions);
  for (i = 0; i < wf->n_failure_actions; ++i) {
    free_arazzo_failure_action(&wf->failure_actions[i]);
  }
  C_CDD_FREE(wf->failure_actions);
  free_arazzo_outputs_array(wf->outputs, wf->n_outputs);
  free_arazzo_parameters_array(wf->parameters, wf->n_parameters);
  arazzo_extension_list_free(&wf->extensions);
  memset(wf, 0, sizeof(*wf));
}

/**
 * @brief Frees all dynamic strings inside an `Arazzo_RuntimeExpr` structure.
 */
void arazzo_runtime_expr_free(struct Arazzo_RuntimeExpr *expr) {
  if (!expr)
    return;
  C_CDD_FREE(expr->raw);
  C_CDD_FREE(expr->source_name);
  C_CDD_FREE(expr->step_or_workflow_id);
  C_CDD_FREE(expr->property_name);
  C_CDD_FREE(expr->json_pointer);
  memset(expr, 0, sizeof(*expr));
}

/**
 * @brief Deep-copies an `Arazzo_RuntimeExpr` structure.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_runtime_expr_copy(struct Arazzo_RuntimeExpr *dst,
                                       const struct Arazzo_RuntimeExpr *src) {
  if (!dst || !src)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(dst, 0, sizeof(*dst));
  dst->kind = src->kind;
  dst->is_embedded_template = src->is_embedded_template;
  if (src->raw) {
    dst->raw = C_CDD_STRDUP(src->raw);
    if (!dst->raw)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->source_name) {
    dst->source_name = C_CDD_STRDUP(src->source_name);
    if (!dst->source_name) {
      arazzo_runtime_expr_free(dst);
      return CDD_C_ERROR_MEMORY;
    }
  }
  if (src->step_or_workflow_id) {
    dst->step_or_workflow_id = C_CDD_STRDUP(src->step_or_workflow_id);
    if (!dst->step_or_workflow_id) {
      arazzo_runtime_expr_free(dst);
      return CDD_C_ERROR_MEMORY;
    }
  }
  if (src->property_name) {
    dst->property_name = C_CDD_STRDUP(src->property_name);
    if (!dst->property_name) {
      arazzo_runtime_expr_free(dst);
      return CDD_C_ERROR_MEMORY;
    }
  }
  if (src->json_pointer) {
    dst->json_pointer = C_CDD_STRDUP(src->json_pointer);
    if (!dst->json_pointer) {
      arazzo_runtime_expr_free(dst);
      return CDD_C_ERROR_MEMORY;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees an `Arazzo_ExtensionList` and all its contained key/value pairs.
 */
void arazzo_extension_list_free(struct Arazzo_ExtensionList *list) {
  size_t i;
  if (!list)
    return;
  for (i = 0; i < list->count; ++i) {
    C_CDD_FREE(list->items[i].key);
    free_any_value(&list->items[i].val);
  }
  C_CDD_FREE(list->items);
  list->items = NULL;
  list->count = 0;
}

/**
 * @brief Appends a specification extension (`x-*`) to an
 * `Arazzo_ExtensionList`.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_extension_list_append(struct Arazzo_ExtensionList *list,
                                           const char *key,
                                           const struct OpenAPI_Any *val) {
  struct Arazzo_Extension *new_items;
  char *dup_key = NULL;
  cdd_c_error_t rc;

  if (!list || !key || !val || strncmp(key, "x-", 2) != 0 || key[2] == '\0')
    return CDD_C_ERROR_INVALID_ARGUMENT;

  dup_key = C_CDD_STRDUP(key);
  if (!dup_key)
    return CDD_C_ERROR_MEMORY;

  new_items = (struct Arazzo_Extension *)C_CDD_REALLOC(
      list->items, (list->count + 1) * sizeof(struct Arazzo_Extension));
  if (!new_items) {
    C_CDD_FREE(dup_key);
    return CDD_C_ERROR_MEMORY;
  }
  list->items = new_items;
  memset(&list->items[list->count], 0, sizeof(struct Arazzo_Extension));
  list->items[list->count].key = dup_key;

  rc = copy_any_value(&list->items[list->count].val, val);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(dup_key);
    list->items[list->count].key = NULL;
    return rc;
  }

  list->count++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Initializes an `Arazzo_Spec` structure with default values (`1.1.0`).
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_spec_init(struct Arazzo_Spec *spec) {
  if (!spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(spec, 0, sizeof(*spec));
  spec->arazzo = C_CDD_STRDUP("1.1.0");
  if (!spec->arazzo)
    return CDD_C_ERROR_MEMORY;
  spec->version = ARAZZO_VER_1_1_0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees all heap-allocated memory inside an `Arazzo_Spec` structure.
 */
void arazzo_spec_free(struct Arazzo_Spec *spec) {
  size_t i;
  if (!spec)
    return;

  C_CDD_FREE(spec->arazzo);
  C_CDD_FREE(spec->self_uri);
  C_CDD_FREE(spec->info.title);
  C_CDD_FREE(spec->info.summary);
  C_CDD_FREE(spec->info.description);
  C_CDD_FREE(spec->info.version);
  arazzo_extension_list_free(&spec->info.extensions);

  for (i = 0; i < spec->n_source_descriptions; ++i) {
    C_CDD_FREE(spec->source_descriptions[i].name);
    C_CDD_FREE(spec->source_descriptions[i].url);
    if (spec->source_descriptions[i].resolved_openapi) {
      openapi_spec_free(spec->source_descriptions[i].resolved_openapi);
      C_CDD_FREE(spec->source_descriptions[i].resolved_openapi);
    }
    arazzo_extension_list_free(&spec->source_descriptions[i].extensions);
  }
  C_CDD_FREE(spec->source_descriptions);

  for (i = 0; i < spec->n_workflows; ++i) {
    free_arazzo_workflow(&spec->workflows[i]);
  }
  C_CDD_FREE(spec->workflows);

  for (i = 0; i < spec->components.n_inputs; ++i) {
    free_schema_ref_content(&spec->components.inputs[i]);
  }
  C_CDD_FREE(spec->components.inputs);
  free_string_array(spec->components.input_keys, spec->components.n_inputs);

  free_arazzo_parameters_array(spec->components.parameters,
                               spec->components.n_parameters);
  free_string_array(spec->components.parameter_keys,
                    spec->components.n_parameters);

  for (i = 0; i < spec->components.n_success_actions; ++i) {
    free_arazzo_success_action(&spec->components.success_actions[i]);
  }
  C_CDD_FREE(spec->components.success_actions);
  free_string_array(spec->components.success_action_keys,
                    spec->components.n_success_actions);

  for (i = 0; i < spec->components.n_failure_actions; ++i) {
    free_arazzo_failure_action(&spec->components.failure_actions[i]);
  }
  C_CDD_FREE(spec->components.failure_actions);
  free_string_array(spec->components.failure_action_keys,
                    spec->components.n_failure_actions);

  arazzo_extension_list_free(&spec->components.extensions);
  arazzo_extension_list_free(&spec->extensions);
  memset(spec, 0, sizeof(*spec));
}

/**
 * @brief Converts an `Arazzo_SourceType` enum to its canonical string.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unknown.
 */
cdd_c_error_t arazzo_source_type_to_str(enum Arazzo_SourceType type,
                                        const char **out_str) {
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (type) {
  case ARAZZO_SOURCE_OPENAPI:
    *out_str = "openapi";
    return CDD_C_SUCCESS;
  case ARAZZO_SOURCE_ASYNCAPI:
    *out_str = "asyncapi";
    return CDD_C_SUCCESS;
  case ARAZZO_SOURCE_ARAZZO:
    *out_str = "arazzo";
    return CDD_C_SUCCESS;
  default:
    *out_str = NULL;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
}

/**
 * @brief Parses an `Arazzo_SourceType` enum from a string.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unrecognized/NULL.
 */
cdd_c_error_t arazzo_source_type_from_str(const char *str,
                                          enum Arazzo_SourceType *out_type) {
  if (!str || !out_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (strcmp(str, "openapi") == 0) {
    *out_type = ARAZZO_SOURCE_OPENAPI;
    return CDD_C_SUCCESS;
  }
  if (strcmp(str, "asyncapi") == 0) {
    *out_type = ARAZZO_SOURCE_ASYNCAPI;
    return CDD_C_SUCCESS;
  }
  if (strcmp(str, "arazzo") == 0) {
    *out_type = ARAZZO_SOURCE_ARAZZO;
    return CDD_C_SUCCESS;
  }
  *out_type = ARAZZO_SOURCE_UNKNOWN;
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Converts an `Arazzo_ParamIn` enum to its canonical string.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unknown.
 */
cdd_c_error_t arazzo_param_in_to_str(enum Arazzo_ParamIn in_loc,
                                     const char **out_str) {
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (in_loc) {
  case ARAZZO_PARAM_IN_PATH:
    *out_str = "path";
    return CDD_C_SUCCESS;
  case ARAZZO_PARAM_IN_QUERY:
    *out_str = "query";
    return CDD_C_SUCCESS;
  case ARAZZO_PARAM_IN_QUERYSTRING:
    *out_str = "querystring";
    return CDD_C_SUCCESS;
  case ARAZZO_PARAM_IN_HEADER:
    *out_str = "header";
    return CDD_C_SUCCESS;
  case ARAZZO_PARAM_IN_COOKIE:
    *out_str = "cookie";
    return CDD_C_SUCCESS;
  default:
    *out_str = NULL;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
}

/**
 * @brief Parses an `Arazzo_ParamIn` enum from a string.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unrecognized/NULL.
 */
cdd_c_error_t arazzo_param_in_from_str(const char *str,
                                       enum Arazzo_ParamIn *out_in) {
  if (!str || !out_in)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (strcmp(str, "path") == 0) {
    *out_in = ARAZZO_PARAM_IN_PATH;
    return CDD_C_SUCCESS;
  }
  if (strcmp(str, "query") == 0) {
    *out_in = ARAZZO_PARAM_IN_QUERY;
    return CDD_C_SUCCESS;
  }
  if (strcmp(str, "querystring") == 0) {
    *out_in = ARAZZO_PARAM_IN_QUERYSTRING;
    return CDD_C_SUCCESS;
  }
  if (strcmp(str, "header") == 0) {
    *out_in = ARAZZO_PARAM_IN_HEADER;
    return CDD_C_SUCCESS;
  }
  if (strcmp(str, "cookie") == 0) {
    *out_in = ARAZZO_PARAM_IN_COOKIE;
    return CDD_C_SUCCESS;
  }
  *out_in = ARAZZO_PARAM_IN_UNKNOWN;
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Converts an `Arazzo_ActionType` enum to its canonical string.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unknown.
 */
cdd_c_error_t arazzo_action_type_to_str(enum Arazzo_ActionType action,
                                        const char **out_str) {
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (action) {
  case ARAZZO_ACTION_END:
    *out_str = "end";
    return CDD_C_SUCCESS;
  case ARAZZO_ACTION_GOTO:
    *out_str = "goto";
    return CDD_C_SUCCESS;
  case ARAZZO_ACTION_RETRY:
    *out_str = "retry";
    return CDD_C_SUCCESS;
  default:
    *out_str = NULL;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
}

/**
 * @brief Parses an `Arazzo_ActionType` enum from a string.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unrecognized/NULL.
 */
cdd_c_error_t arazzo_action_type_from_str(const char *str,
                                          enum Arazzo_ActionType *out_action) {
  if (!str || !out_action)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (strcmp(str, "end") == 0) {
    *out_action = ARAZZO_ACTION_END;
    return CDD_C_SUCCESS;
  }
  if (strcmp(str, "goto") == 0) {
    *out_action = ARAZZO_ACTION_GOTO;
    return CDD_C_SUCCESS;
  }
  if (strcmp(str, "retry") == 0) {
    *out_action = ARAZZO_ACTION_RETRY;
    return CDD_C_SUCCESS;
  }
  *out_action = ARAZZO_ACTION_UNKNOWN;
  return CDD_C_ERROR_INVALID_ARGUMENT;
}
