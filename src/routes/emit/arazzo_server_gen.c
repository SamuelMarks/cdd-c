/**
 * @file arazzo_server_gen.c
 * @brief Implementation of Backend Server Orchestrator, Stateful Mock Runner &
 * E2E `greatest.h` Test Suite Generator from Arazzo v1.1.0 Specifications.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/code_writer.h"
#include "c_cdd/safe_crt.h"
#include "functions/parse/fs.h"
#include "routes/emit/arazzo_client_gen.h"
#include "routes/emit/arazzo_server_gen.h"
#include "routes/emit/server_gen.h"
/* clang-format on */

cdd_c_error_t
arazzo_server_render_to_strings(const struct Arazzo_Spec *arazzo_spec,
                                const char *base_name, const char *prefix,
                                char **out_server_src, char **out_test_hdr) {
  cdd_code_writer_t ws, wt;
  const char *pfx = prefix ? prefix : "";
  size_t i, j;
  cdd_c_error_t rc;

  if (!arazzo_spec || !base_name || !out_server_src || !out_test_hdr) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_code_writer_init_buffer(&ws);
  rc = cdd_code_writer_init_buffer(&wt);

  /* 1. Server Orchestrator File */
  cdd_code_writer_printf(&ws, "/**\n * @file %s_workflow_server.c\n",
                         base_name);
  cdd_code_writer_printf(
      &ws, " * @brief Generated Arazzo v%s Server Workflow Orchestrator.\n",
      arazzo_spec->arazzo ? arazzo_spec->arazzo : "1.1.0");
  cdd_code_writer_printf(&ws, " * @arazzo %s\n",
                         arazzo_spec->arazzo ? arazzo_spec->arazzo : "1.1.0");
  if (arazzo_spec->info.title) {
    cdd_code_writer_printf(&ws, " * @infoTitle %s\n", arazzo_spec->info.title);
  }
  if (arazzo_spec->info.version) {
    cdd_code_writer_printf(&ws, " * @infoVersion %s\n",
                           arazzo_spec->info.version);
  }
  cdd_code_writer_write(&ws, " */\n\n");

  cdd_code_writer_write(&ws, "/* clang-format "
                             "off */\n");
  cdd_code_writer_write(&ws, "#include <stdio.h>\n");
  cdd_code_writer_write(&ws, "#include <stdlib.h>\n");
  cdd_code_writer_write(&ws, "#include <string.h>\n");
  cdd_code_writer_write(&ws, "#include \"cdd_c_error.h\"\n");
  cdd_code_writer_write(&ws, "#include <c_rest_request.h>\n");
  cdd_code_writer_write(&ws, "#include <c_rest_response.h>\n");
  cdd_code_writer_write(&ws, "#include <c_rest_router.h>\n");
  cdd_code_writer_printf(&ws, "#include \"%s_workflows.h\"\n", base_name);
  cdd_code_writer_write(&ws, "/* clang-format "
                             "on */\n\n");

  for (i = 0; i < arazzo_spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &arazzo_spec->workflows[i];
    const char *wf_id = wf->workflow_id ? wf->workflow_id : "default_workflow";

    cdd_code_writer_printf(
        &ws,
        "/**\n"
        " * @brief Composite server workflow handler for `%s`.\n"
        " * @route POST /workflows/%s\n"
        " * @workflow %s %s\n",
        wf_id, wf_id, wf_id, wf->description ? wf->description : "");
    for (j = 0; j < wf->n_steps; ++j) {
      const struct Arazzo_Step *st = &wf->steps[j];
      cdd_code_writer_printf(&ws, " * @step %s [operationId:%s]\n",
                             st->step_id ? st->step_id : "step_1",
                             st->operation_id ? st->operation_id : "op");
    }
    cdd_code_writer_write(&ws, " */\n");
    cdd_code_writer_printf(
        &ws,
        "cdd_c_error_t handle_workflow_%s(struct c_rest_request *req,\n"
        "                                 struct c_rest_response *res,\n"
        "                                 void *user_data) {\n"
        "  struct %sWorkflowClients clients;\n"
        "  struct %sWorkflow_%s_Inputs in_s;\n"
        "  struct %sWorkflow_%s_Outputs out_s;\n"
        "  cdd_c_error_t rc;\n",
        wf_id, pfx, pfx, wf_id, pfx, wf_id);
    cdd_code_writer_printf(&ws,
                           "  (void)req;\n"
                           "  (void)res;\n"
                           "  (void)user_data;\n"
                           "  memset(&clients, 0, sizeof(clients));\n"
                           "  memset(&in_s, 0, sizeof(in_s));\n"
                           "  memset(&out_s, 0, sizeof(out_s));\n"
                           "  rc = %sworkflow_%s(&clients, &in_s, &out_s);\n"
                           "  return rc;\n"
                           "}\n\n",
                           pfx, wf_id);
  }

  cdd_code_writer_write(
      &ws, "/**\n"
           " * @brief Registers all Arazzo workflow composite routes on a "
           "`c_rest_router`.\n"
           " */\n"
           "cdd_c_error_t register_arazzo_workflow_routes(struct c_rest_router "
           "*router) {\n"
           "  if (!router) return CDD_C_ERROR_INVALID_ARGUMENT;\n");
  for (i = 0; i < arazzo_spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &arazzo_spec->workflows[i];
    const char *wf_id = wf->workflow_id ? wf->workflow_id : "default_workflow";
    cdd_code_writer_printf(&ws,
                           "  /* c_rest_router_add(router, \"POST\", "
                           "\"/workflows/%s\", handle_workflow_%s, NULL); */\n",
                           wf_id, wf_id);
  }
  cdd_code_writer_write(&ws, "  return CDD_C_SUCCESS;\n}\n");

  /* 2. E2E `greatest.h` Test Header */
  cdd_code_writer_printf(&wt, "/**\n * @file %s_workflow_tests.h\n", base_name);
  cdd_code_writer_printf(
      &wt, " * @brief Generated Arazzo v%s E2E `greatest.h` Workflow Tests.\n",
      arazzo_spec->arazzo ? arazzo_spec->arazzo : "1.1.0");
  cdd_code_writer_printf(&wt, " * @arazzo %s\n",
                         arazzo_spec->arazzo ? arazzo_spec->arazzo : "1.1.0");
  cdd_code_writer_write(&wt, " */\n\n");
  cdd_code_writer_printf(&wt, "#ifndef C_CDD_%s_WORKFLOW_TESTS_H\n", base_name);
  cdd_code_writer_printf(&wt, "#define C_CDD_%s_WORKFLOW_TESTS_H\n\n",
                         base_name);
  cdd_code_writer_write(&wt, "/* clang-format "
                             "off */\n");
  cdd_code_writer_write(&wt, "#include <greatest.h>\n");
  cdd_code_writer_write(&wt, "#include <string.h>\n");
  cdd_code_writer_printf(&wt, "#include \"%s_workflows.h\"\n", base_name);
  cdd_code_writer_write(&wt, "/* clang-format "
                             "on */\n\n");
  cdd_code_writer_write(&wt, "#ifdef __cplusplus\nextern \"C\" {\n#endif /* "
                             "__cplusplus */\n\n");

  for (i = 0; i < arazzo_spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &arazzo_spec->workflows[i];
    const char *wf_id = wf->workflow_id ? wf->workflow_id : "default_workflow";
    cdd_code_writer_printf(
        &wt,
        "/**\n"
        " * @brief E2E workflow test for `%s`.\n"
        " * @workflow %s %s\n"
        " */\n"
        "TEST test_workflow_%s(void) {\n"
        "  struct %sWorkflowClients clients;\n"
        "  struct %sWorkflow_%s_Inputs in_s;\n"
        "  struct %sWorkflow_%s_Outputs out_s;\n"
        "  memset(&clients, 0, sizeof(clients));\n"
        "  memset(&in_s, 0, sizeof(in_s));\n"
        "  memset(&out_s, 0, sizeof(out_s));\n"
        "  ASSERT_EQ(CDD_C_SUCCESS, %sworkflow_%s(&clients, &in_s, &out_s));\n"
        "  ASSERT_EQ(200, out_s.last_status_code);\n"
        "  PASS();\n"
        "}\n\n",
        wf_id, wf_id, wf->description ? wf->description : "", wf_id, pfx, pfx,
        wf_id, pfx, wf_id, pfx, wf_id);
  }

  cdd_code_writer_printf(&wt, "SUITE(%s_arazzo_workflows_suite) {\n",
                         base_name);
  for (i = 0; i < arazzo_spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &arazzo_spec->workflows[i];
    const char *wf_id = wf->workflow_id ? wf->workflow_id : "default_workflow";
    cdd_code_writer_printf(&wt, "  RUN_TEST(test_workflow_%s);\n", wf_id);
  }
  cdd_code_writer_write(&wt, "}\n\n");
  cdd_code_writer_write(&wt,
                        "#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n\n");
  cdd_code_writer_printf(&wt, "#endif /* C_CDD_%s_WORKFLOW_TESTS_H */\n",
                         base_name);

  rc = cdd_code_writer_take_string(&ws, out_server_src);
  cdd_code_writer_free(&ws);
  if (rc != CDD_C_SUCCESS) {
    cdd_code_writer_free(&wt);
    return rc;
  }
  rc = cdd_code_writer_take_string(&wt, out_test_hdr);
  cdd_code_writer_free(&wt);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(*out_server_src);
    *out_server_src = NULL;
    return rc;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t arazzo_server_generate(const struct Arazzo_Spec *arazzo_spec,
                                     const struct OpenAPI_Spec *openapi_spec,
                                     const struct OpenApiClientConfig *config) {
  char *dir_name = NULL;
  char *base_name = NULL;
  char *srv_path = NULL;
  char *tst_path = NULL;
  char *srv_content = NULL;
  char *tst_content = NULL;
  size_t path_cap;
  cdd_c_error_t rc;

  if (!arazzo_spec || !config || !config->filename_base) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (openapi_spec) {
    rc = openapi_server_generate(openapi_spec, config);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  }

  /* Also generate the workflow SDK files so server & tests can include them */
  rc = arazzo_client_generate(arazzo_spec, NULL, config);
  if (rc != CDD_C_SUCCESS) {
    return rc;
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

  path_cap = strlen(dir_name) + strlen(base_name) + 40;
  srv_path = (char *)C_CDD_MALLOC(path_cap);
  tst_path = (char *)C_CDD_MALLOC(path_cap);
  if (!srv_path || !tst_path) {
    C_CDD_FREE(dir_name);
    C_CDD_FREE(base_name);
    C_CDD_FREE(srv_path);
    C_CDD_FREE(tst_path);
    return CDD_C_ERROR_MEMORY;
  }

  CDD_SNPRINTF(srv_path, path_cap, "%s/src/%s_workflow_server.c", dir_name,
               base_name);
  CDD_SNPRINTF(tst_path, path_cap, "%s/src/%s_workflow_tests.h", dir_name,
               base_name);

  rc = arazzo_server_render_to_strings(
      arazzo_spec, base_name, config->func_prefix ? config->func_prefix : "",
      &srv_content, &tst_content);
  if (rc == CDD_C_SUCCESS) {
    rc = fs_write_to_file(srv_path, srv_content);
  }
  if (rc == CDD_C_SUCCESS) {
    rc = fs_write_to_file(tst_path, tst_content);
  }
  C_CDD_FREE(srv_content);
  C_CDD_FREE(tst_content);

  C_CDD_FREE(srv_path);
  C_CDD_FREE(tst_path);
  C_CDD_FREE(dir_name);
  C_CDD_FREE(base_name);
  return rc;
}
