/**
 * @file arazzo_client_gen.c
 * @brief Implementation of Frontend C SDK Workflow Generator from Arazzo v1.1.0
 * Specifications.
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

#include "c_cdd/code_writer.h"
#include "c_cdd/safe_crt.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
#include "routes/emit/arazzo_client_gen.h"
/* clang-format on */

/**
 * @brief Formats an `OpenAPI_Any` value into a short string representation for
 * Doxygen annotations.
 *
 * @param[in] val Any value to format.
 * @param[out] buf Output character buffer.
 * @param[in] buf_cap Capacity of `buf`.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t format_any_short(const struct OpenAPI_Any *val, char *buf,
                                      size_t buf_cap) {
  JSON_Value *jv = NULL;
  char *ser = NULL;
  buf[0] = '\0';
  if (val->type == OA_ANY_STRING) {
    CDD_SNPRINTF(buf, buf_cap, "%s", val->string);
    return CDD_C_SUCCESS;
  }
  any_to_json_value(val, &jv);
  if (jv) {
    ser = json_serialize_to_string(jv);
    json_value_free(jv);
  }
  if (ser) {
    CDD_SNPRINTF(buf, buf_cap, "%s", ser);
    json_free_serialized_string(ser);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits the file-level Doxygen header block containing round-trip
 * `@arazzo`, `@self`, `@info*`, `@sourceDescription`, and `@x-*` annotations.
 *
 * @param[in,out] w Code writer instance.
 * @param[in] spec Arazzo specification.
 * @param[in] base_name Base filename.
 * @return `CDD_C_SUCCESS` on success, or sticky writer error code.
 */
static cdd_c_error_t emit_arazzo_file_docblock(cdd_code_writer_t *w,
                                               const struct Arazzo_Spec *spec,
                                               const char *base_name) {
  size_t i;
  char val_buf[256];
  cdd_code_writer_printf(w, "/**\n * @file %s_workflows.h\n", base_name);
  cdd_code_writer_printf(
      w, " * @brief Generated Arazzo v%s C Client SDK Workflows.\n",
      spec->arazzo ? spec->arazzo : "1.1.0");
  cdd_code_writer_printf(w, " * @arazzo %s\n",
                         spec->arazzo ? spec->arazzo : "1.1.0");
  if (spec->self_uri) {
    cdd_code_writer_printf(w, " * @self %s\n", spec->self_uri);
  }
  if (spec->info.title) {
    cdd_code_writer_printf(w, " * @infoTitle %s\n", spec->info.title);
  }
  if (spec->info.version) {
    cdd_code_writer_printf(w, " * @infoVersion %s\n", spec->info.version);
  }
  if (spec->info.summary) {
    cdd_code_writer_printf(w, " * @infoSummary %s\n", spec->info.summary);
  }
  if (spec->info.description) {
    cdd_code_writer_printf(w, " * @infoDescription %s\n",
                           spec->info.description);
  }
  for (i = 0; i < spec->n_source_descriptions; ++i) {
    const struct Arazzo_SourceDescription *sd = &spec->source_descriptions[i];
    const char *sname = sd->name ? sd->name : "default";
    const char *surl = sd->url ? sd->url : "./openapi.json";
    const char *type_str = NULL;
    if (arazzo_source_type_to_str(sd->type, &type_str) == CDD_C_SUCCESS) {
      cdd_code_writer_printf(w, " * @sourceDescription %s %s [type:%s]\n",
                             sname, surl, type_str);
    } else {
      cdd_code_writer_printf(w, " * @sourceDescription %s %s\n", sname, surl);
    }
  }
  for (i = 0; i < spec->extensions.count; ++i) {
    format_any_short(&spec->extensions.items[i].val, val_buf, sizeof(val_buf));
    cdd_code_writer_printf(w, " * @%s %s\n", spec->extensions.items[i].key,
                           val_buf[0] ? val_buf : "true");
  }
  return cdd_code_writer_write(w, " */\n\n");
}

/**
 * @brief Emits the Doxygen annotation block for a single Arazzo workflow and
 * all its steps for lossless round-tripping via `c2arazzo`.
 *
 * @param[in,out] w Code writer instance.
 * @param[in] wf Arazzo workflow.
 * @return `CDD_C_SUCCESS` on success, or sticky writer error code.
 */
static cdd_c_error_t emit_workflow_docblock(cdd_code_writer_t *w,
                                            const struct Arazzo_Workflow *wf) {
  size_t i, j;
  char val_buf[256];
  cdd_code_writer_write(w, "/**\n");
  if (wf->summary) {
    cdd_code_writer_printf(w, " * @workflow %s [summary:%s] %s\n",
                           wf->workflow_id, wf->summary,
                           wf->description ? wf->description : "");
    cdd_code_writer_printf(w, " * @workflowSummary %s\n", wf->summary);
  } else {
    cdd_code_writer_printf(w, " * @workflow %s %s\n",
                           wf->workflow_id ? wf->workflow_id : "workflow",
                           wf->description ? wf->description : "");
  }
  if (wf->n_depends_on > 0) {
    static const char *const k_sep[2] = {"", ", "};
    cdd_code_writer_write(w, " * @workflowDependsOn ");
    for (i = 0; i < wf->n_depends_on; ++i) {
      cdd_code_writer_printf(w, "%s%s", k_sep[i > 0], wf->depends_on[i]);
    }
    cdd_code_writer_write(w, "\n");
  }
  for (i = 0; i < wf->n_parameters; ++i) {
    const struct Arazzo_Parameter *p = &wf->parameters[i];
    if (p->reusable.is_reusable) {
      format_any_short(&p->reusable.value, val_buf, sizeof(val_buf));
      cdd_code_writer_printf(w, " * @workflowParam ref [reference:%s] %s\n",
                             p->reusable.reference, val_buf);
    } else {
      const char *in_str = NULL;
      format_any_short(&p->value, val_buf, sizeof(val_buf));
      if (arazzo_param_in_to_str(p->in, &in_str) == CDD_C_SUCCESS) {
        cdd_code_writer_printf(w, " * @workflowParam %s [in:%s] %s\n", p->name,
                               in_str, val_buf);
      } else {
        cdd_code_writer_printf(w, " * @workflowParam %s %s\n", p->name,
                               val_buf);
      }
    }
  }
  for (i = 0; i < wf->n_outputs; ++i) {
    cdd_code_writer_printf(w, " * @workflowOutput %s %s\n", wf->outputs[i].key,
                           wf->outputs[i].expression_or_value
                               ? wf->outputs[i].expression_or_value
                               : "$statusCode");
  }

  for (i = 0; i < wf->n_steps; ++i) {
    const struct Arazzo_Step *st = &wf->steps[i];
    cdd_code_writer_printf(w, " * @step %s",
                           st->step_id ? st->step_id : "step_1");
    if (st->target_kind == ARAZZO_STEP_TARGET_OPERATION_ID) {
      cdd_code_writer_printf(w, " [operationId:%s]", st->operation_id);
    } else if (st->target_kind == ARAZZO_STEP_TARGET_OPERATION_PATH) {
      cdd_code_writer_printf(w, " [operationPath:%s]", st->operation_path);
    } else if (st->target_kind == ARAZZO_STEP_TARGET_CHANNEL_PATH) {
      cdd_code_writer_printf(w, " [channelPath:%s]", st->channel_path);
    } else if (st->target_kind == ARAZZO_STEP_TARGET_WORKFLOW_ID) {
      cdd_code_writer_printf(w, " [workflowId:%s]", st->workflow_id);
    }
    if (st->has_timeout) {
      cdd_code_writer_printf(w, " [timeout:%d]", st->timeout_ms);
    }
    if (st->correlation_id) {
      cdd_code_writer_printf(w, " [correlationId:%s]", st->correlation_id);
    }
    if (st->action == ARAZZO_ASYNC_SEND) {
      cdd_code_writer_write(w, " [action:send]");
    } else if (st->action == ARAZZO_ASYNC_RECEIVE) {
      cdd_code_writer_write(w, " [action:receive]");
    }
    if (st->description) {
      cdd_code_writer_printf(w, " %s", st->description);
    }
    cdd_code_writer_write(w, "\n");

    if (st->n_depends_on > 0) {
      static const char *const k_sep[2] = {"", ", "};
      cdd_code_writer_write(w, " * @stepDependsOn ");
      for (j = 0; j < st->n_depends_on; ++j) {
        cdd_code_writer_printf(w, "%s%s", k_sep[j > 0], st->depends_on[j]);
      }
      cdd_code_writer_write(w, "\n");
    }

    for (j = 0; j < st->n_parameters; ++j) {
      const struct Arazzo_Parameter *p = &st->parameters[j];
      if (p->reusable.is_reusable) {
        format_any_short(&p->reusable.value, val_buf, sizeof(val_buf));
        cdd_code_writer_printf(w, " * @stepParam ref [reference:%s] %s\n",
                               p->reusable.reference, val_buf);
      } else {
        const char *in_str = NULL;
        format_any_short(&p->value, val_buf, sizeof(val_buf));
        if (arazzo_param_in_to_str(p->in, &in_str) == CDD_C_SUCCESS) {
          cdd_code_writer_printf(w, " * @stepParam %s [in:%s] %s\n", p->name,
                                 in_str, val_buf);
        } else {
          cdd_code_writer_printf(w, " * @stepParam %s %s\n", p->name, val_buf);
        }
      }
    }

    if (st->has_request_body) {
      format_any_short(&st->request_body.payload, val_buf, sizeof(val_buf));
      if (st->request_body.content_type) {
        cdd_code_writer_printf(w, " * @stepRequestBody [contentType:%s] %s\n",
                               st->request_body.content_type,
                               val_buf[0] ? val_buf : "{}");
      } else {
        cdd_code_writer_printf(w, " * @stepRequestBody %s\n",
                               val_buf[0] ? val_buf : "{}");
      }
      for (j = 0; j < st->request_body.n_replacements; ++j) {
        const struct Arazzo_PayloadReplacement *rep =
            &st->request_body.replacements[j];
        format_any_short(&rep->value, val_buf, sizeof(val_buf));
        cdd_code_writer_printf(w, " * @stepReplacement %s %s\n",
                               rep->target ? rep->target : "/id",
                               val_buf[0] ? val_buf : "\"\"");
      }
    }

    for (j = 0; j < st->n_success_criteria; ++j) {
      const struct Arazzo_Criterion *cr = &st->success_criteria[j];
      cdd_code_writer_write(w, " * @stepCriterion");
      if (cr->context) {
        cdd_code_writer_printf(w, " [context:%s]", cr->context);
      }
      if (cr->type == ARAZZO_CRITERION_REGEX) {
        cdd_code_writer_write(w, " [type:regex]");
      } else if (cr->type == ARAZZO_CRITERION_JSONPATH ||
                 cr->type == ARAZZO_CRITERION_EXPRESSION_TYPE) {
        cdd_code_writer_write(w, " [type:jsonpath]");
      } else if (cr->type == ARAZZO_CRITERION_XPATH) {
        cdd_code_writer_write(w, " [type:xpath]");
      }
      cdd_code_writer_printf(
          w, " %s\n", cr->condition ? cr->condition : "$statusCode == 200");
    }

    for (j = 0; j < st->n_on_success; ++j) {
      const struct Arazzo_SuccessAction *sa = &st->on_success[j];
      if (sa->reusable.is_reusable) {
        cdd_code_writer_printf(w, " * @stepOnSuccess ref [reference:%s]\n",
                               sa->reusable.reference);
      } else {
        const char *act_str = NULL;
        arazzo_action_type_to_str(sa->type, &act_str);
        cdd_code_writer_printf(w, " * @stepOnSuccess %s [type:%s]",
                               sa->name ? sa->name : "done",
                               act_str ? act_str : "end");
        if (sa->step_id) {
          cdd_code_writer_printf(w, " [stepId:%s]", sa->step_id);
        }
        if (sa->workflow_id) {
          cdd_code_writer_printf(w, " [workflowId:%s]", sa->workflow_id);
        }
        cdd_code_writer_write(w, "\n");
      }
    }

    for (j = 0; j < st->n_on_failure; ++j) {
      const struct Arazzo_FailureAction *fa = &st->on_failure[j];
      if (fa->reusable.is_reusable) {
        cdd_code_writer_printf(w, " * @stepOnFailure ref [reference:%s]\n",
                               fa->reusable.reference);
      } else {
        const char *act_str = NULL;
        arazzo_action_type_to_str(fa->type, &act_str);
        cdd_code_writer_printf(w, " * @stepOnFailure %s [type:%s]",
                               fa->name ? fa->name : "fail",
                               act_str ? act_str : "end");
        if (fa->step_id) {
          cdd_code_writer_printf(w, " [stepId:%s]", fa->step_id);
        }
        if (fa->workflow_id) {
          cdd_code_writer_printf(w, " [workflowId:%s]", fa->workflow_id);
        }
        if (fa->has_retry_after) {
          cdd_code_writer_printf(w, " [retryAfter:%.2f]", fa->retry_after);
        }
        if (fa->has_retry_limit) {
          cdd_code_writer_printf(w, " [retryLimit:%d]", fa->retry_limit);
        }
        cdd_code_writer_write(w, "\n");
      }
    }

    for (j = 0; j < st->n_outputs; ++j) {
      cdd_code_writer_printf(w, " * @stepOutput %s %s\n", st->outputs[j].key,
                             st->outputs[j].expression_or_value
                                 ? st->outputs[j].expression_or_value
                                 : "$statusCode");
    }
  }
  return cdd_code_writer_write(
      w, " * @param[in,out] clients Bound source description clients.\n"
         " * @param[in] inputs Workflow inputs.\n"
         " * @param[out] outputs Workflow outputs.\n"
         " * @return `CDD_C_SUCCESS` on success.\n"
         " */\n");
}

/**
 * @brief Renders the C Client SDK workflow header (`_workflows.h`) and source
 * (`_workflows.c`) into in-memory strings.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] openapi_spec Optional linked OpenAPI specification (may be NULL).
 * @param[in] base_name Base filename without directory.
 * @param[in] prefix Function name prefix (e.g. `"api_"` or `""`).
 * @param[out] out_hdr Output heap-allocated header content string.
 * @param[out] out_src Output heap-allocated source content string.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t
arazzo_client_render_to_strings(const struct Arazzo_Spec *arazzo_spec,
                                const struct OpenAPI_Spec *openapi_spec,
                                const char *base_name, const char *prefix,
                                char **out_hdr, char **out_src) {
  cdd_code_writer_t wh, wc;
  const char *pfx = prefix ? prefix : "";
  size_t i, j, k;
  cdd_c_error_t rc;

  (void)openapi_spec;

  if (!arazzo_spec || !base_name || !out_hdr || !out_src) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_code_writer_init_buffer(&wh);
  rc = cdd_code_writer_init_buffer(&wc);

  /* Header File */
  emit_arazzo_file_docblock(&wh, arazzo_spec, base_name);
  cdd_code_writer_printf(&wh, "#ifndef C_CDD_%s_WORKFLOWS_H\n", base_name);
  cdd_code_writer_printf(&wh, "#define C_CDD_%s_WORKFLOWS_H\n\n", base_name);
  cdd_code_writer_write(&wh, "/* clang-format "
                             "off */\n");
  cdd_code_writer_write(&wh, "#include <stddef.h>\n");
  cdd_code_writer_write(&wh, "#include \"cdd_c_error.h\"\n");
  cdd_code_writer_write(&wh, "/* clang-format "
                             "on */\n\n");
  cdd_code_writer_write(&wh, "#ifdef __cplusplus\nextern \"C\" {\n#endif /* "
                             "__cplusplus */\n\n");

  /* Workflow Clients struct */
  cdd_code_writer_printf(
      &wh,
      "/**\n * @brief Bound sourceDescription clients for workflows.\n */\n"
      "struct %sWorkflowClients {\n",
      pfx);
  if (arazzo_spec->n_source_descriptions == 0) {
    cdd_code_writer_write(
        &wh, "  void *default_client; /**< Default API client */\n");
  } else {
    for (i = 0; i < arazzo_spec->n_source_descriptions; ++i) {
      const char *sname = arazzo_spec->source_descriptions[i].name
                              ? arazzo_spec->source_descriptions[i].name
                              : "default";
      cdd_code_writer_printf(&wh, "  void *%s_client; /**< %s client */\n",
                             sname, sname);
    }
  }
  cdd_code_writer_write(&wh, "};\n\n");

  /* Source File Preamble */
  emit_arazzo_file_docblock(&wc, arazzo_spec, base_name);
  cdd_code_writer_write(&wc, "/* clang-format "
                             "off */\n");
  cdd_code_writer_write(&wc, "#include <stdio.h>\n");
  cdd_code_writer_write(&wc, "#include <stdlib.h>\n");
  cdd_code_writer_write(&wc, "#include <string.h>\n");
  cdd_code_writer_printf(&wc, "#include \"%s_workflows.h\"\n", base_name);
  cdd_code_writer_write(&wc, "/* clang-format "
                             "on */\n\n");

  /* Workflows */
  for (i = 0; i < arazzo_spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &arazzo_spec->workflows[i];
    const char *wf_id = wf->workflow_id ? wf->workflow_id : "default_workflow";

    /* Inputs struct */
    cdd_code_writer_printf(
        &wh, "/**\n * @brief Inputs for workflow `%s`.\n */\n", wf_id);
    cdd_code_writer_printf(&wh, "struct %sWorkflow_%s_Inputs {\n", pfx, wf_id);
    cdd_code_writer_write(
        &wh, "  const char *raw_json; /**< Raw JSON workflow inputs */\n");
    cdd_code_writer_write(&wh, "};\n\n");

    /* Outputs struct */
    cdd_code_writer_printf(
        &wh, "/**\n * @brief Outputs for workflow `%s`.\n */\n", wf_id);
    cdd_code_writer_printf(&wh, "struct %sWorkflow_%s_Outputs {\n", pfx, wf_id);
    cdd_code_writer_write(
        &wh, "  int last_status_code; /**< Final HTTP status code */\n");
    for (j = 0; j < wf->n_outputs; ++j) {
      cdd_code_writer_printf(&wh, "  char *%s; /**< Output %s */\n",
                             wf->outputs[j].key, wf->outputs[j].key);
    }
    cdd_code_writer_write(&wh, "};\n\n");

    /* Docblock & Prototype */
    emit_workflow_docblock(&wh, wf);
    cdd_code_writer_printf(&wh,
                           "cdd_c_error_t %sworkflow_%s(\n"
                           "    struct %sWorkflowClients *clients,\n"
                           "    const struct %sWorkflow_%s_Inputs *inputs,\n"
                           "    struct %sWorkflow_%s_Outputs *outputs);\n\n",
                           pfx, wf_id, pfx, pfx, wf_id, pfx, wf_id);

    /* Implementation */
    emit_workflow_docblock(&wc, wf);
    cdd_code_writer_printf(&wc,
                           "cdd_c_error_t %sworkflow_%s(\n"
                           "    struct %sWorkflowClients *clients,\n"
                           "    const struct %sWorkflow_%s_Inputs *inputs,\n"
                           "    struct %sWorkflow_%s_Outputs *outputs) {\n",
                           pfx, wf_id, pfx, pfx, wf_id, pfx, wf_id);
    cdd_code_writer_write(&wc, "  int status_code = 200;\n");
    for (j = 0; j < wf->n_steps; ++j) {
      const char *sid = wf->steps[j].step_id ? wf->steps[j].step_id : "step_1";
      cdd_code_writer_printf(&wc, "  int retry_%s = 0;\n", sid);
      cdd_code_writer_printf(&wc, "  int done_%s = 0;\n", sid);
    }
    cdd_code_writer_write(&wc, "  if (!clients || !outputs) {\n"
                               "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                               "  }\n"
                               "  (void)inputs;\n"
                               "  memset(outputs, 0, sizeof(*outputs));\n");

    for (j = 0; j < wf->n_depends_on; ++j) {
      cdd_code_writer_printf(&wc, "  /* Prerequisite workflow: %s */\n",
                             wf->depends_on[j]);
    }

    for (j = 0; j < wf->n_steps; ++j) {
      const struct Arazzo_Step *st = &wf->steps[j];
      const char *sid = st->step_id ? st->step_id : "step_1";
      cdd_code_writer_printf(&wc, "\nstep_%s:\n", sid);
      cdd_code_writer_printf(&wc, "  (void)retry_%s;\n", sid);
      for (k = 0; k < st->n_depends_on; ++k) {
        cdd_code_writer_printf(&wc,
                               "  if (!done_%s) {\n"
                               "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                               "  }\n",
                               st->depends_on[k]);
      }
      if (st->target_kind == ARAZZO_STEP_TARGET_OPERATION_ID) {
        cdd_code_writer_printf(&wc, "  /* Execute operationId: %s */\n",
                               st->operation_id);
      } else if (st->target_kind == ARAZZO_STEP_TARGET_OPERATION_PATH) {
        cdd_code_writer_printf(&wc, "  /* Execute operationPath: %s */\n",
                               st->operation_path);
      } else if (st->target_kind == ARAZZO_STEP_TARGET_CHANNEL_PATH) {
        cdd_code_writer_printf(&wc, "  /* Execute channelPath: %s */\n",
                               st->channel_path);
      } else if (st->target_kind == ARAZZO_STEP_TARGET_WORKFLOW_ID) {
        cdd_code_writer_printf(&wc, "  /* Execute nested workflowId: %s */\n",
                               st->workflow_id);
      }
      if (st->has_timeout) {
        cdd_code_writer_printf(&wc, "  /* Step timeout: %d ms */\n",
                               st->timeout_ms);
      }
      for (k = 0; k < st->n_on_failure; ++k) {
        const struct Arazzo_FailureAction *fa = &st->on_failure[k];
        if (fa->type == ARAZZO_ACTION_RETRY) {
          int lim = fa->has_retry_limit ? fa->retry_limit : 1;
          cdd_code_writer_printf(
              &wc,
              "  if (status_code >= 500 && retry_%s < %d) {\n"
              "    retry_%s++;\n"
              "    goto step_%s;\n"
              "  }\n",
              sid, lim, sid, sid);
        } else if (fa->type == ARAZZO_ACTION_GOTO && fa->step_id) {
          cdd_code_writer_printf(&wc,
                                 "  if (status_code >= 400) {\n"
                                 "    goto step_%s;\n"
                                 "  }\n",
                                 fa->step_id);
        }
      }
      for (k = 0; k < st->n_on_success; ++k) {
        const struct Arazzo_SuccessAction *sa = &st->on_success[k];
        if (sa->type == ARAZZO_ACTION_GOTO && sa->step_id) {
          cdd_code_writer_printf(&wc,
                                 "  if (status_code == 202) {\n"
                                 "    done_%s = 1;\n"
                                 "    goto step_%s;\n"
                                 "  }\n",
                                 sid, sa->step_id);
        }
      }
      cdd_code_writer_printf(&wc, "  done_%s = 1;\n", sid);
      cdd_code_writer_printf(&wc, "  (void)done_%s;\n", sid);
    }

    cdd_code_writer_write(&wc, "\n  outputs->last_status_code = status_code;\n"
                               "  return CDD_C_SUCCESS;\n"
                               "}\n\n");
  }

  cdd_code_writer_write(&wh,
                        "#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n\n");
  cdd_code_writer_printf(&wh, "#endif /* C_CDD_%s_WORKFLOWS_H */\n", base_name);

  rc = cdd_code_writer_take_string(&wh, out_hdr);
  cdd_code_writer_free(&wh);
  if (rc != CDD_C_SUCCESS) {
    cdd_code_writer_free(&wc);
    return rc;
  }
  rc = cdd_code_writer_take_string(&wc, out_src);
  cdd_code_writer_free(&wc);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(*out_hdr);
    *out_hdr = NULL;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C Client SDK workflow header and implementation files
 * (`<base>_workflows.h` and `<base>_workflows.c`) from an Arazzo v1.1.0
 * specification.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] openapi_spec Optional linked OpenAPI specification (may be NULL).
 * @param[in] config Client generation configuration (`filename_base`,
 * `func_prefix`, etc.).
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on NULL,
 * `CDD_C_ERROR_MEMORY` on OOM, or `CDD_C_ERROR_IO` on file failure.
 */
cdd_c_error_t arazzo_client_generate(const struct Arazzo_Spec *arazzo_spec,
                                     const struct OpenAPI_Spec *openapi_spec,
                                     const struct OpenApiClientConfig *config) {
  char *dir_name = NULL;
  char *base_name = NULL;
  char *src_dir = NULL;
  char *h_path = NULL;
  char *c_path = NULL;
  char *hdr_content = NULL;
  char *src_content = NULL;
  size_t path_cap;
  cdd_c_error_t rc;

  if (!arazzo_spec || !config || !config->filename_base) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (openapi_spec) {
    rc = openapi_client_generate(openapi_spec, config);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  }

  rc = get_dirname(config->filename_base, &dir_name);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  rc = get_basename(config->filename_base, &base_name);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(dir_name);
    return rc;
  }

  path_cap = strlen(dir_name) + strlen(base_name) + 32;
  src_dir = (char *)C_CDD_MALLOC(path_cap);
  h_path = (char *)C_CDD_MALLOC(path_cap);
  c_path = (char *)C_CDD_MALLOC(path_cap);
  if (!src_dir || !h_path || !c_path) {
    C_CDD_FREE(dir_name);
    C_CDD_FREE(base_name);
    C_CDD_FREE(src_dir);
    C_CDD_FREE(h_path);
    C_CDD_FREE(c_path);
    return CDD_C_ERROR_MEMORY;
  }

  CDD_SNPRINTF(src_dir, path_cap, "%s/src", dir_name);
  rc = makedirs(src_dir);
  if (rc != CDD_C_SUCCESS) {
    goto cleanup;
  }

  CDD_SNPRINTF(h_path, path_cap, "%s/src/%s_workflows.h", dir_name, base_name);
  CDD_SNPRINTF(c_path, path_cap, "%s/src/%s_workflows.c", dir_name, base_name);

  rc = arazzo_client_render_to_strings(arazzo_spec, openapi_spec, base_name,
                                       config->func_prefix ? config->func_prefix
                                                           : "",
                                       &hdr_content, &src_content);
  if (rc != CDD_C_SUCCESS) {
    goto cleanup;
  }

  rc = fs_write_to_file(h_path, hdr_content);
  if (rc != CDD_C_SUCCESS) {
    goto cleanup;
  }
  rc = fs_write_to_file(c_path, src_content);

cleanup:
  C_CDD_FREE(hdr_content);
  C_CDD_FREE(src_content);
  C_CDD_FREE(dir_name);
  C_CDD_FREE(base_name);
  C_CDD_FREE(src_dir);
  C_CDD_FREE(h_path);
  C_CDD_FREE(c_path);
  return rc;
}
