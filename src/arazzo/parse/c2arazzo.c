/**
 * @file c2arazzo.c
 * @brief Implementation of C Source/Header to Arazzo v1.1.0 Specification
 * Extractor.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arazzo/emit/arazzo.h"
#include "arazzo/parse/arazzo.h"
#include "arazzo/parse/arazzo_expr.h"
#include "arazzo/parse/c2arazzo.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "openapi/parse/openapi_internal.h"
#include "openapi/parse/openapi_schema.h"
#include "routes/parse/cli.h"
/* clang-format on */

/**
 * @brief Replaces a destination string with a freshly duplicated copy of `src`.
 *
 * @param[in] src Source string.
 * @param[in,out] dst Destination string pointer (existing value freed).
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t replace_str(const char *src, char **dst) {
  C_CDD_FREE(*dst);
  *dst = NULL;
  return c_cdd_strdup(src, dst);
}

/**
 * @brief Parses a string or JSON snippet into a `struct OpenAPI_Any`.
 *
 * @param[in] raw Raw string or JSON literal.
 * @param[out] out_any Destination `OpenAPI_Any` structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_str_to_any(const char *raw,
                                      struct OpenAPI_Any *out_any) {
  JSON_Value *jv;
  cdd_c_error_t rc;
  memset(out_any, 0, sizeof(*out_any));
  jv = json_parse_string(raw);
  if (jv) {
    rc = parse_any_value(jv, out_any);
    json_value_free(jv);
    return rc;
  }
  out_any->type = OA_ANY_STRING;
  return c_cdd_strdup(raw, &out_any->string);
}

/**
 * @brief Appends a source description if not already present by `name`.
 *
 * @param[in,out] spec Target Arazzo specification.
 * @param[in] sDoc Parsed docstring source description.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t
append_source_description(struct Arazzo_Spec *spec,
                          const struct DocArazzoSource *sDoc) {
  size_t i;
  struct Arazzo_SourceDescription *next_arr;
  struct Arazzo_SourceDescription *sd;
  cdd_c_error_t rc;

  for (i = 0; i < spec->n_source_descriptions; ++i) {
    if (strcmp(spec->source_descriptions[i].name, sDoc->name) == 0) {
      return CDD_C_SUCCESS;
    }
  }

  next_arr = (struct Arazzo_SourceDescription *)C_CDD_REALLOC(
      spec->source_descriptions, (spec->n_source_descriptions + 1) *
                                     sizeof(struct Arazzo_SourceDescription));
  if (!next_arr) {
    return CDD_C_ERROR_MEMORY;
  }
  spec->source_descriptions = next_arr;
  sd = &spec->source_descriptions[spec->n_source_descriptions];
  memset(sd, 0, sizeof(*sd));

  rc = c_cdd_strdup(sDoc->name, &sd->name);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  spec->n_source_descriptions++;

  rc = c_cdd_strdup(sDoc->url, &sd->url);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (sDoc->type) {
    enum Arazzo_SourceType st = ARAZZO_SOURCE_UNKNOWN;
    if (arazzo_source_type_from_str(sDoc->type, &st) == CDD_C_SUCCESS) {
      sd->type = st;
    }
  } else {
    sd->type = ARAZZO_SOURCE_OPENAPI;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Converts a `DocArazzoParam` array into an `Arazzo_Parameter` array.
 *
 * @param[in] src Source docstring parameter array.
 * @param[in] n_src Count of source parameters.
 * @param[out] out_params Output Arazzo parameter array pointer.
 * @param[out] out_count Output parameter count pointer.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t convert_doc_params(const struct DocArazzoParam *src,
                                        size_t n_src,
                                        struct Arazzo_Parameter **out_params,
                                        size_t *out_count) {
  size_t i;
  struct Arazzo_Parameter *arr;
  cdd_c_error_t rc;

  if (n_src == 0) {
    return CDD_C_SUCCESS;
  }
  arr = (struct Arazzo_Parameter *)C_CDD_CALLOC(
      n_src, sizeof(struct Arazzo_Parameter));
  if (!arr) {
    return CDD_C_ERROR_MEMORY;
  }
  *out_params = arr;

  for (i = 0; i < n_src; ++i) {
    struct Arazzo_Parameter *dst = &arr[i];
    if (src[i].reference) {
      dst->reusable.is_reusable = 1;
      rc = c_cdd_strdup(src[i].reference, &dst->reusable.reference);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      if (src[i].value) {
        dst->reusable.has_value_override = 1;
        rc = parse_str_to_any(src[i].value, &dst->reusable.value);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
      }
    } else {
      rc = c_cdd_strdup(src[i].name, &dst->name);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      if (src[i].in_loc) {
        enum Arazzo_ParamIn loc = ARAZZO_PARAM_IN_UNKNOWN;
        if (arazzo_param_in_from_str(src[i].in_loc, &loc) == CDD_C_SUCCESS) {
          dst->in = loc;
        }
      }
      if (src[i].value) {
        rc = parse_str_to_any(src[i].value, &dst->value);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        if (src[i].value[0] == '$' || strchr(src[i].value, '{') != NULL) {
          rc = arazzo_runtime_expr_parse(src[i].value, &dst->parsed_expr);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
      }
    }
    (*out_count)++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Converts a `DocArazzoKeyVal` array into an `Arazzo_OutputMapEntry`
 * array.
 *
 * @param[in] src Source docstring key-value array.
 * @param[in] n_src Count of entries.
 * @param[out] out_entries Output entries pointer.
 * @param[out] out_count Output count pointer.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t
convert_doc_outputs(const struct DocArazzoKeyVal *src, size_t n_src,
                    struct Arazzo_OutputMapEntry **out_entries,
                    size_t *out_count) {
  size_t i;
  struct Arazzo_OutputMapEntry *arr;
  cdd_c_error_t rc;

  if (n_src == 0) {
    return CDD_C_SUCCESS;
  }
  arr = (struct Arazzo_OutputMapEntry *)C_CDD_CALLOC(
      n_src, sizeof(struct Arazzo_OutputMapEntry));
  if (!arr) {
    return CDD_C_ERROR_MEMORY;
  }
  *out_entries = arr;

  for (i = 0; i < n_src; ++i) {
    rc = c_cdd_strdup(src[i].key, &arr[i].key);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    (*out_count)++;
    if (src[i].value) {
      rc = c_cdd_strdup(src[i].value, &arr[i].expression_or_value);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      rc = arazzo_runtime_expr_parse(src[i].value, &arr[i].parsed_expr);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Converts a single `DocArazzoStep` into an `Arazzo_Step`.
 *
 * @param[in] src Source docstring step.
 * @param[out] dst Destination Arazzo step.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t convert_doc_step(const struct DocArazzoStep *src,
                                      struct Arazzo_Step *dst) {
  size_t i;
  cdd_c_error_t rc;

  rc = c_cdd_strdup(src->step_id, &dst->step_id);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (src->description) {
    rc = c_cdd_strdup(src->description, &dst->description);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  }
  if (src->operation_id) {
    dst->target_kind = ARAZZO_STEP_TARGET_OPERATION_ID;
    rc = c_cdd_strdup(src->operation_id, &dst->operation_id);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  } else if (src->operation_path) {
    dst->target_kind = ARAZZO_STEP_TARGET_OPERATION_PATH;
    rc = c_cdd_strdup(src->operation_path, &dst->operation_path);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  } else if (src->channel_path) {
    dst->target_kind = ARAZZO_STEP_TARGET_CHANNEL_PATH;
    rc = c_cdd_strdup(src->channel_path, &dst->channel_path);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  } else if (src->workflow_id) {
    dst->target_kind = ARAZZO_STEP_TARGET_WORKFLOW_ID;
    rc = c_cdd_strdup(src->workflow_id, &dst->workflow_id);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  } else {
    dst->target_kind = ARAZZO_STEP_TARGET_OPERATION_ID;
    rc = c_cdd_strdup(src->step_id, &dst->operation_id);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  }

  if (src->has_timeout) {
    dst->has_timeout = 1;
    dst->timeout_ms = src->timeout_ms;
  }
  if (src->correlation_id) {
    rc = c_cdd_strdup(src->correlation_id, &dst->correlation_id);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  }
  if (src->action) {
    if (strcmp(src->action, "send") == 0) {
      dst->action = ARAZZO_ASYNC_SEND;
    } else if (strcmp(src->action, "receive") == 0) {
      dst->action = ARAZZO_ASYNC_RECEIVE;
    }
  }

  if (src->n_depends_on > 0) {
    dst->depends_on = (char **)C_CDD_CALLOC(src->n_depends_on, sizeof(char *));
    if (!dst->depends_on) {
      return CDD_C_ERROR_MEMORY;
    }
    for (i = 0; i < src->n_depends_on; ++i) {
      rc = c_cdd_strdup(src->depends_on[i], &dst->depends_on[i]);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      dst->n_depends_on++;
    }
  }

  rc = convert_doc_params(src->params, src->n_params, &dst->parameters,
                          &dst->n_parameters);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  if (src->request_body_content_type || src->request_body_payload ||
      src->n_replacements > 0) {
    dst->has_request_body = 1;
    if (src->request_body_content_type) {
      rc = c_cdd_strdup(src->request_body_content_type,
                        &dst->request_body.content_type);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
    }
    if (src->request_body_payload) {
      rc = parse_str_to_any(src->request_body_payload,
                            &dst->request_body.payload);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
    }
    if (src->n_replacements > 0) {
      dst->request_body.replacements =
          (struct Arazzo_PayloadReplacement *)C_CDD_CALLOC(
              src->n_replacements, sizeof(struct Arazzo_PayloadReplacement));
      if (!dst->request_body.replacements) {
        return CDD_C_ERROR_MEMORY;
      }
      for (i = 0; i < src->n_replacements; ++i) {
        struct Arazzo_PayloadReplacement *rep =
            &dst->request_body.replacements[i];
        rc = c_cdd_strdup(src->replacements[i].target, &rep->target);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        rep->target_selector_type = (src->replacements[i].target[0] == '$')
                                        ? ARAZZO_CRITERION_JSONPATH
                                        : ARAZZO_CRITERION_SIMPLE;
        dst->request_body.n_replacements++;
        if (src->replacements[i].value) {
          rc = parse_str_to_any(src->replacements[i].value, &rep->value);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
          if (src->replacements[i].value[0] == '$' ||
              strchr(src->replacements[i].value, '{') != NULL) {
            rc = arazzo_runtime_expr_parse(src->replacements[i].value,
                                           &rep->parsed_expr);
            if (rc != CDD_C_SUCCESS) {
              return rc;
            }
          }
        }
      }
    }
  }

  if (src->n_criteria > 0) {
    dst->success_criteria = (struct Arazzo_Criterion *)C_CDD_CALLOC(
        src->n_criteria, sizeof(struct Arazzo_Criterion));
    if (!dst->success_criteria) {
      return CDD_C_ERROR_MEMORY;
    }
    for (i = 0; i < src->n_criteria; ++i) {
      struct Arazzo_Criterion *cr = &dst->success_criteria[i];
      rc = c_cdd_strdup(src->criteria[i].condition, &cr->condition);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      dst->n_success_criteria++;
      if (src->criteria[i].context) {
        rc = c_cdd_strdup(src->criteria[i].context, &cr->context);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
      }
      if (src->criteria[i].type) {
        if (strcmp(src->criteria[i].type, "regex") == 0) {
          cr->type = ARAZZO_CRITERION_REGEX;
        } else if (strcmp(src->criteria[i].type, "jsonpath") == 0) {
          if (src->criteria[i].version) {
            cr->type = ARAZZO_CRITERION_EXPRESSION_TYPE;
            cr->expression_type.type = ARAZZO_DIALECT_JSONPATH;
            rc = c_cdd_strdup(src->criteria[i].version,
                              &cr->expression_type.version);
            if (rc != CDD_C_SUCCESS) {
              return rc;
            }
          } else {
            cr->type = ARAZZO_CRITERION_JSONPATH;
          }
        } else if (strcmp(src->criteria[i].type, "xpath") == 0) {
          cr->type = ARAZZO_CRITERION_XPATH;
        }
      }
    }
  }

  if (src->n_on_success > 0) {
    dst->on_success = (struct Arazzo_SuccessAction *)C_CDD_CALLOC(
        src->n_on_success, sizeof(struct Arazzo_SuccessAction));
    if (!dst->on_success) {
      return CDD_C_ERROR_MEMORY;
    }
    for (i = 0; i < src->n_on_success; ++i) {
      struct Arazzo_SuccessAction *sa = &dst->on_success[i];
      if (src->on_success[i].reference) {
        sa->reusable.is_reusable = 1;
        rc =
            c_cdd_strdup(src->on_success[i].reference, &sa->reusable.reference);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
      } else {
        rc = c_cdd_strdup(src->on_success[i].name, &sa->name);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        sa->type = ARAZZO_ACTION_END;
        if (src->on_success[i].type) {
          rc = arazzo_action_type_from_str(src->on_success[i].type, &sa->type);
          if (rc != CDD_C_SUCCESS) {
            sa->type = ARAZZO_ACTION_END;
          }
        }
        if (src->on_success[i].step_id) {
          rc = c_cdd_strdup(src->on_success[i].step_id, &sa->step_id);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
        if (src->on_success[i].workflow_id) {
          rc = c_cdd_strdup(src->on_success[i].workflow_id, &sa->workflow_id);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
      }
      dst->n_on_success++;
    }
  }

  if (src->n_on_failure > 0) {
    dst->on_failure = (struct Arazzo_FailureAction *)C_CDD_CALLOC(
        src->n_on_failure, sizeof(struct Arazzo_FailureAction));
    if (!dst->on_failure) {
      return CDD_C_ERROR_MEMORY;
    }
    for (i = 0; i < src->n_on_failure; ++i) {
      struct Arazzo_FailureAction *fa = &dst->on_failure[i];
      if (src->on_failure[i].reference) {
        fa->reusable.is_reusable = 1;
        rc =
            c_cdd_strdup(src->on_failure[i].reference, &fa->reusable.reference);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
      } else {
        rc = c_cdd_strdup(src->on_failure[i].name, &fa->name);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        fa->type = ARAZZO_ACTION_END;
        if (src->on_failure[i].type) {
          rc = arazzo_action_type_from_str(src->on_failure[i].type, &fa->type);
          if (rc != CDD_C_SUCCESS) {
            fa->type = ARAZZO_ACTION_END;
          }
        }
        if (src->on_failure[i].step_id) {
          rc = c_cdd_strdup(src->on_failure[i].step_id, &fa->step_id);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
        if (src->on_failure[i].workflow_id) {
          rc = c_cdd_strdup(src->on_failure[i].workflow_id, &fa->workflow_id);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
        if (src->on_failure[i].has_retry_after) {
          fa->has_retry_after = 1;
          fa->retry_after = src->on_failure[i].retry_after;
        }
        if (src->on_failure[i].has_retry_limit) {
          fa->has_retry_limit = 1;
          fa->retry_limit = src->on_failure[i].retry_limit;
        }
      }
      dst->n_on_failure++;
    }
  }

  return convert_doc_outputs(src->outputs, src->n_outputs, &dst->outputs,
                             &dst->n_outputs);
}

/**
 * @brief Applies parsed `@arazzo*` and `@workflow*` docstring metadata onto an
 * `Arazzo_Spec`.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t c2arazzo_apply_doc_metadata(const struct DocMetadata *meta,
                                          struct Arazzo_Spec *spec) {
  size_t i;
  cdd_c_error_t rc;

  if (!meta || !spec) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (meta->arazzo_version) {
    rc = replace_str(meta->arazzo_version, &spec->arazzo);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->self_uri) {
    rc = replace_str(meta->self_uri, &spec->self_uri);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_title) {
    rc = replace_str(meta->info_title, &spec->info.title);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_version) {
    rc = replace_str(meta->info_version, &spec->info.version);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_summary) {
    rc = replace_str(meta->info_summary, &spec->info.summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_description) {
    rc = replace_str(meta->info_description, &spec->info.description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  for (i = 0; i < meta->n_arazzo_sources; ++i) {
    rc = append_source_description(spec, &meta->arazzo_sources[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  for (i = 0; i < meta->n_arazzo_extensions; ++i) {
    struct OpenAPI_Any ext_val;
    rc = parse_str_to_any(meta->arazzo_extensions[i].value
                              ? meta->arazzo_extensions[i].value
                              : "true",
                          &ext_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = arazzo_extension_list_append(&spec->extensions,
                                      meta->arazzo_extensions[i].key, &ext_val);
    free_any_value(&ext_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (meta->workflow_id) {
    struct Arazzo_Workflow *wf = NULL;
    size_t k;
    for (k = 0; k < spec->n_workflows; ++k) {
      if (strcmp(spec->workflows[k].workflow_id, meta->workflow_id) == 0) {
        wf = &spec->workflows[k];
        break;
      }
    }
    if (!wf) {
      struct Arazzo_Workflow *next_wfs =
          (struct Arazzo_Workflow *)C_CDD_REALLOC(
              spec->workflows,
              (spec->n_workflows + 1) * sizeof(struct Arazzo_Workflow));
      if (!next_wfs) {
        return CDD_C_ERROR_MEMORY;
      }
      spec->workflows = next_wfs;
      wf = &spec->workflows[spec->n_workflows];
      memset(wf, 0, sizeof(*wf));

      rc = c_cdd_strdup(meta->workflow_id, &wf->workflow_id);
      if (rc != CDD_C_SUCCESS)
        return rc;
      spec->n_workflows++;

      if (meta->workflow_summary) {
        rc = c_cdd_strdup(meta->workflow_summary, &wf->summary);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (meta->description) {
        rc = c_cdd_strdup(meta->description, &wf->description);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (meta->workflow_inputs_json) {
        JSON_Value *in_val = json_parse_string(meta->workflow_inputs_json);
        if (json_value_get_object(in_val)) {
          if (parse_schema_ref(json_value_get_object(in_val), &wf->inputs,
                               NULL) == CDD_C_SUCCESS) {
            wf->has_inputs = 1;
          }
        }
        json_value_free(in_val);
      }
      if (meta->n_workflow_depends_on > 0) {
        wf->depends_on =
            (char **)C_CDD_CALLOC(meta->n_workflow_depends_on, sizeof(char *));
        if (!wf->depends_on)
          return CDD_C_ERROR_MEMORY;
        for (i = 0; i < meta->n_workflow_depends_on; ++i) {
          rc = c_cdd_strdup(meta->workflow_depends_on[i], &wf->depends_on[i]);
          if (rc != CDD_C_SUCCESS)
            return rc;
          wf->n_depends_on++;
        }
      }
      rc = convert_doc_params(meta->workflow_params, meta->n_workflow_params,
                              &wf->parameters, &wf->n_parameters);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = convert_doc_outputs(meta->workflow_outputs, meta->n_workflow_outputs,
                               &wf->outputs, &wf->n_outputs);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }

    if (wf->n_steps == 0 && meta->n_arazzo_steps > 0) {
      wf->steps = (struct Arazzo_Step *)C_CDD_CALLOC(
          meta->n_arazzo_steps, sizeof(struct Arazzo_Step));
      if (!wf->steps)
        return CDD_C_ERROR_MEMORY;
      for (i = 0; i < meta->n_arazzo_steps; ++i) {
        rc = convert_doc_step(&meta->arazzo_steps[i], &wf->steps[i]);
        if (rc != CDD_C_SUCCESS)
          return rc;
        wf->n_steps++;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a workflow with `wf_id` already exists in `spec`.
 *
 * @param[in] spec Arazzo specification.
 * @param[in] wf_id Workflow identifier to look up.
 * @param[out] out_found Set to 1 if found, 0 otherwise.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t find_workflow_by_id(const struct Arazzo_Spec *spec,
                                         const char *wf_id, int *out_found) {
  size_t i;
  *out_found = 0;
  for (i = 0; i < spec->n_workflows; ++i) {
    if (strcmp(spec->workflows[i].workflow_id, wf_id) == 0) {
      *out_found = 1;
      break;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts undocumented workflows from function signatures/bodies
 * (`<prefix>workflow_<workflowId>` or `TEST test_workflow_<workflowId>`) and
 * scans step labels (`step_<stepId>:`) inside the function body.
 *
 * @param[in] func_node CST node of the function.
 * @param[out] spec Target Arazzo specification.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t
extract_workflow_from_function_node(const struct CstNode *func_node,
                                    struct Arazzo_Spec *spec) {
  struct TokenList *tl = NULL;
  az_span span;
  size_t i;
  char *wf_id = NULL;
  int found = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  span = az_span_create((uint8_t *)(size_t)func_node->start, func_node->length);
  if (tokenize(span, &tl) != 0) {
    return CDD_C_SUCCESS;
  }

  for (i = 0; tl->tokens[i].kind != TOKEN_LBRACE; ++i) {
    if (tl->tokens[i].kind == TOKEN_IDENTIFIER) {
      size_t k;
      for (k = 0; k + 9 < tl->tokens[i].length; ++k) {
        if (memcmp(tl->tokens[i].start + k, "workflow_", 9) == 0) {
          size_t id_len = tl->tokens[i].length - (k + 9);
          wf_id = (char *)C_CDD_MALLOC(id_len + 1);
          if (!wf_id) {
            free_token_list(tl);
            return CDD_C_ERROR_MEMORY;
          }
          memcpy(wf_id, tl->tokens[i].start + k + 9, id_len);
          wf_id[id_len] = '\0';
          break;
        }
      }
      if (wf_id)
        break;
    }
  }

  if (wf_id) {
    find_workflow_by_id(spec, wf_id, &found);
    if (!found) {
      struct Arazzo_Workflow *next_wfs =
          (struct Arazzo_Workflow *)C_CDD_REALLOC(
              spec->workflows,
              (spec->n_workflows + 1) * sizeof(struct Arazzo_Workflow));
      if (!next_wfs) {
        C_CDD_FREE(wf_id);
        free_token_list(tl);
        return CDD_C_ERROR_MEMORY;
      }
      spec->workflows = next_wfs;
      {
        struct Arazzo_Workflow *wf = &spec->workflows[spec->n_workflows];
        memset(wf, 0, sizeof(*wf));
        wf->workflow_id = wf_id;
        wf_id = NULL;
        spec->n_workflows++;

        for (i = 0; i + 1 < tl->size; ++i) {
          if (tl->tokens[i].kind == TOKEN_IDENTIFIER &&
              tl->tokens[i].length > 5 &&
              memcmp(tl->tokens[i].start, "step_", 5) == 0 &&
              tl->tokens[i + 1].kind == TOKEN_COLON) {
            size_t slen = tl->tokens[i].length - 5;
            struct Arazzo_Step *next_steps =
                (struct Arazzo_Step *)C_CDD_REALLOC(
                    wf->steps, (wf->n_steps + 1) * sizeof(struct Arazzo_Step));
            if (!next_steps) {
              free_token_list(tl);
              return CDD_C_ERROR_MEMORY;
            }
            wf->steps = next_steps;
            memset(&wf->steps[wf->n_steps], 0, sizeof(struct Arazzo_Step));
            wf->steps[wf->n_steps].step_id = (char *)C_CDD_MALLOC(slen + 1);
            if (!wf->steps[wf->n_steps].step_id) {
              free_token_list(tl);
              return CDD_C_ERROR_MEMORY;
            }
            memcpy(wf->steps[wf->n_steps].step_id, tl->tokens[i].start + 5,
                   slen);
            wf->steps[wf->n_steps].step_id[slen] = '\0';
            wf->steps[wf->n_steps].target_kind =
                ARAZZO_STEP_TARGET_OPERATION_ID;
            rc = c_cdd_strdup(wf->steps[wf->n_steps].step_id,
                              &wf->steps[wf->n_steps].operation_id);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(wf->steps[wf->n_steps].step_id);
              free_token_list(tl);
              return rc;
            }
            wf->n_steps++;
          }
        }

        if (wf->n_steps == 0) {
          wf->steps =
              (struct Arazzo_Step *)C_CDD_CALLOC(1, sizeof(struct Arazzo_Step));
          if (!wf->steps) {
            free_token_list(tl);
            return CDD_C_ERROR_MEMORY;
          }
          rc = c_cdd_strdup("step_1", &wf->steps[0].step_id);
          if (rc != CDD_C_SUCCESS) {
            free_token_list(tl);
            return rc;
          }
          wf->steps[0].target_kind = ARAZZO_STEP_TARGET_OPERATION_ID;
          rc = c_cdd_strdup(wf->workflow_id, &wf->steps[0].operation_id);
          if (rc != CDD_C_SUCCESS) {
            free_token_list(tl);
            return rc;
          }
          wf->n_steps = 1;
        }
      }
    }
  }

  C_CDD_FREE(wf_id);
  free_token_list(tl);
  return rc;
}

/**
 * @brief Ensures default `info` and `sourceDescriptions` fields are populated
 * on an extracted `Arazzo_Spec`.
 *
 * @param[in,out] spec Target Arazzo specification.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t ensure_spec_defaults(struct Arazzo_Spec *spec) {
  cdd_c_error_t rc;
  if (!spec->info.title) {
    rc = c_cdd_strdup("Extracted Arazzo Workflow Specification",
                      &spec->info.title);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (!spec->info.version) {
    rc = c_cdd_strdup("1.0.0", &spec->info.version);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (spec->n_source_descriptions == 0) {
    char name_buf[16];
    char url_buf[32];
    char type_buf[16];
    struct DocArazzoSource def_src;
    memcpy(name_buf, "default", 8);
    memcpy(url_buf, "./openapi.json", 15);
    memcpy(type_buf, "openapi", 8);
    def_src.name = name_buf;
    def_src.url = url_buf;
    def_src.type = type_buf;
    rc = append_source_description(spec, &def_src);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (spec->n_workflows == 0) {
    spec->workflows = (struct Arazzo_Workflow *)C_CDD_CALLOC(
        1, sizeof(struct Arazzo_Workflow));
    if (!spec->workflows)
      return CDD_C_ERROR_MEMORY;
    spec->n_workflows = 1;
    rc = c_cdd_strdup("default_workflow", &spec->workflows[0].workflow_id);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  {
    size_t i;
    for (i = 0; i < spec->n_workflows; ++i) {
      if (spec->workflows[i].n_steps == 0) {
        spec->workflows[i].steps =
            (struct Arazzo_Step *)C_CDD_CALLOC(1, sizeof(struct Arazzo_Step));
        if (!spec->workflows[i].steps)
          return CDD_C_ERROR_MEMORY;
        spec->workflows[i].n_steps = 1;
        rc = c_cdd_strdup("step_1", &spec->workflows[i].steps[0].step_id);
        if (rc != CDD_C_SUCCESS)
          return rc;
        spec->workflows[i].steps[0].target_kind =
            ARAZZO_STEP_TARGET_OPERATION_ID;
        rc = c_cdd_strdup("default_op",
                          &spec->workflows[i].steps[0].operation_id);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses an in-memory C source or header string and merges discovered
 * Arazzo workflows and metadata into `spec`.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t c2arazzo_parse_source(const char *source,
                                    struct Arazzo_Spec *spec) {
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!source || !spec) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (!spec->arazzo) {
    rc = arazzo_spec_init(spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (tokenize(az_span_create_from_str(source), &tokens) != 0) {
    return CDD_C_ERROR_MEMORY;
  }
  rc = parse_tokens(tokens, &cst);
  if (rc != CDD_C_SUCCESS) {
    free_token_list(tokens);
    return rc;
  }

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_COMMENT) {
      struct DocMetadata meta;
      char *doc_text = (char *)C_CDD_MALLOC(cst.nodes[i].length + 1);
      if (!doc_text) {
        rc = CDD_C_ERROR_MEMORY;
        goto cleanup;
      }
      memcpy(doc_text, cst.nodes[i].start, cst.nodes[i].length);
      doc_text[cst.nodes[i].length] = '\0';

      doc_metadata_init(&meta);
      rc = doc_parse_block(doc_text, &meta);
      if (rc == CDD_C_SUCCESS) {
        rc = c2arazzo_apply_doc_metadata(&meta, spec);
      }
      doc_metadata_free(&meta);
      C_CDD_FREE(doc_text);
      if (rc != CDD_C_SUCCESS) {
        goto cleanup;
      }
    } else if (cst.nodes[i].kind == CST_NODE_FUNCTION) {
      rc = extract_workflow_from_function_node(&cst.nodes[i], spec);
      if (rc != CDD_C_SUCCESS) {
        goto cleanup;
      }
    }
  }

  rc = ensure_spec_defaults(spec);

cleanup:
  free_cst_node_list(&cst);
  free_token_list(tokens);
  return rc;
}

/**
 * @brief Parses a single C source or header file and merges discovered Arazzo
 * workflows into `spec`.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t c2arazzo_parse_file(const char *path, struct Arazzo_Spec *spec) {
  char *content = NULL;
  size_t sz = 0;
  cdd_c_error_t rc;

  if (!path || !spec) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = read_to_file(path, "r", &content, &sz);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  rc = c2arazzo_parse_source(content, spec);
  C_CDD_FREE(content);
  return rc;
}

/**
 * @brief Directory walker callback for `c2arazzo_parse_dir`.
 *
 * @param[in] path File path encountered.
 * @param[in,out] user_data Pointer to `struct Arazzo_Spec`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t c2arazzo_walker_cb(const char *path, void *user_data) {
  struct Arazzo_Spec *spec = (struct Arazzo_Spec *)user_data;
  int is_src = 0;

  c2openapi_is_source_file(path, &is_src);
  if (!is_src) {
    return CDD_C_SUCCESS;
  }
  return c2arazzo_parse_file(path, spec);
}

/**
 * @brief Recursively scans a directory (or single file) of C code to build an
 * `Arazzo_Spec`.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t c2arazzo_parse_dir(const char *dir_path,
                                 struct Arazzo_Spec *spec) {
  int is_src = 0;
  cdd_c_error_t rc;

  if (!dir_path || !spec) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (!spec->arazzo) {
    rc = arazzo_spec_init(spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  c2openapi_is_source_file(dir_path, &is_src);
  if (is_src) {
    return c2arazzo_parse_file(dir_path, spec);
  }
  rc = walk_directory(dir_path, c2arazzo_walker_cb, spec);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  return ensure_spec_defaults(spec);
}

/**
 * @brief CLI entry point for `c2arazzo` / `to_arazzo`.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t c2arazzo_cli_main(int argc, char **argv) {
  const char *input = NULL;
  const char *output = NULL;
  const char *self_uri = NULL;
  struct Arazzo_Spec spec;
  char *json = NULL;
  int i;
  cdd_c_error_t rc;

  if (argc < 1 || !argv) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
      if (i + 1 >= argc)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      input = argv[++i];
    } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
      if (i + 1 >= argc)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      output = argv[++i];
    } else if (strcmp(argv[i], "--self") == 0 || strcmp(argv[i], "-s") == 0) {
      if (i + 1 >= argc)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      self_uri = argv[++i];
    } else if (argv[i][0] != '-') {
      if (!input)
        input = argv[i];
      else if (!output)
        output = argv[i];
    }
  }

  if (!input) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (!output) {
    output = "arazzo.json";
  }

  memset(&spec, 0, sizeof(spec));
  rc = arazzo_spec_init(&spec);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  rc = c2arazzo_parse_dir(input, &spec);
  if (rc != CDD_C_SUCCESS) {
    arazzo_spec_free(&spec);
    return rc;
  }

  if (self_uri) {
    rc = replace_str(self_uri, &spec.self_uri);
    if (rc != CDD_C_SUCCESS) {
      arazzo_spec_free(&spec);
      return rc;
    }
  }

  rc = arazzo_write_spec_to_json(&spec, &json);
  arazzo_spec_free(&spec);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  rc = fs_write_to_file(output, json);
  C_CDD_FREE(json);
  return rc;
}
