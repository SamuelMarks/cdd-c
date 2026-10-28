/**
 * @file arazzo_cli_gen.c
 * @brief Implementation of Frontend SDK CLI Workflow Runner Generator from
 * Arazzo v1.1.0 Specifications.
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
#include "routes/emit/arazzo_cli_gen.h"
#include "routes/emit/arazzo_client_gen.h"
/* clang-format on */

/**
 * @brief Renders the CLI workflow runner C source into an in-memory string.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] base_name Base filename without directory.
 * @param[in] prefix Function name prefix (e.g. `"api_"` or `""`).
 * @param[out] out_cli_src Output heap-allocated CLI source string.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_cli_render_to_string(const struct Arazzo_Spec *arazzo_spec,
                                          const char *base_name,
                                          const char *prefix,
                                          char **out_cli_src) {
  cdd_code_writer_t w;
  const char *pfx = prefix ? prefix : "";
  size_t i;
  cdd_c_error_t rc;

  if (!arazzo_spec || !base_name || !out_cli_src) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_code_writer_init_buffer(&w);

  cdd_code_writer_printf(&w, "/**\n * @file %s_workflow_cli.c\n", base_name);
  cdd_code_writer_printf(
      &w, " * @brief Generated CLI Workflow Runner reusing `%s_workflows.h`.\n",
      base_name);
  cdd_code_writer_printf(&w, " * @arazzo %s\n",
                         arazzo_spec->arazzo ? arazzo_spec->arazzo : "1.1.0");
  if (arazzo_spec->info.title) {
    cdd_code_writer_printf(&w, " * @infoTitle %s\n", arazzo_spec->info.title);
  }
  if (arazzo_spec->info.version) {
    cdd_code_writer_printf(&w, " * @infoVersion %s\n",
                           arazzo_spec->info.version);
  }
  cdd_code_writer_write(&w, " */\n\n");

  cdd_code_writer_write(&w, "/* clang-format "
                            "off */\n");
  cdd_code_writer_write(&w, "#include <stdio.h>\n");
  cdd_code_writer_write(&w, "#include <stdlib.h>\n");
  cdd_code_writer_write(&w, "#include <string.h>\n");
  cdd_code_writer_printf(&w, "#include \"%s_workflows.h\"\n", base_name);
  cdd_code_writer_write(&w, "/* clang-format "
                            "on */\n\n");

  cdd_code_writer_write(
      &w, "/**\n * @brief Prints available Arazzo CLI workflows.\n */\n"
          "static cdd_c_error_t print_workflow_cli_help(void) {\n");
  cdd_code_writer_printf(
      &w, "  printf(\"%s v%s\\n\", \"%s\", \"%s\");\n", "%s", "%s",
      arazzo_spec->info.title ? arazzo_spec->info.title : "Arazzo CLI",
      arazzo_spec->info.version ? arazzo_spec->info.version : "1.0.0");
  cdd_code_writer_write(
      &w,
      "  printf(\"Usage: cli workflow <workflowId> [--input <json>]\\n\");\n"
      "  printf(\"Workflows:\\n\");\n");
  for (i = 0; i < arazzo_spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &arazzo_spec->workflows[i];
    cdd_code_writer_printf(&w, "  printf(\"  %-24s %s\\n\");\n",
                           wf->workflow_id ? wf->workflow_id : "workflow",
                           wf->summary ? wf->summary : "");
  }
  cdd_code_writer_write(&w, "  return CDD_C_SUCCESS;\n}\n\n");

  cdd_code_writer_write(
      &w, "/**\n * @brief CLI entry point dispatching Arazzo workflows.\n */\n"
          "int main(int argc, char **argv) {\n"
          "  const char *wf_id = NULL;\n"
          "  const char *input_json = \"{}\";\n"
          "  int i;\n");
  cdd_code_writer_printf(&w, "  struct %sWorkflowClients clients;\n", pfx);
  cdd_code_writer_write(
      &w, "  memset(&clients, 0, sizeof(clients));\n"
          "  if (argc < 2 || strcmp(argv[1], \"--help\") == 0 ||\n"
          "      strcmp(argv[1], \"-h\") == 0) {\n"
          "    return (int)print_workflow_cli_help();\n"
          "  }\n"
          "  if (strcmp(argv[1], \"workflow\") == 0 && argc >= 3) {\n"
          "    wf_id = argv[2];\n"
          "    i = 3;\n"
          "  } else {\n"
          "    wf_id = argv[1];\n"
          "    i = 2;\n"
          "  }\n"
          "  for (; i < argc; ++i) {\n"
          "    if ((strcmp(argv[i], \"--input\") == 0 ||\n"
          "         strcmp(argv[i], \"-i\") == 0) && i + 1 < argc) {\n"
          "      input_json = argv[++i];\n"
          "    }\n"
          "  }\n"
          "  (void)input_json;\n");

  for (i = 0; i < arazzo_spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &arazzo_spec->workflows[i];
    const char *wf_id = wf->workflow_id ? wf->workflow_id : "default_workflow";
    cdd_code_writer_printf(&w,
                           "  if (strcmp(wf_id, \"%s\") == 0) {\n"
                           "    struct %sWorkflow_%s_Inputs in_s;\n"
                           "    struct %sWorkflow_%s_Outputs out_s;\n"
                           "    cdd_c_error_t rc;\n"
                           "    memset(&in_s, 0, sizeof(in_s));\n"
                           "    memset(&out_s, 0, sizeof(out_s));\n"
                           "    rc = %sworkflow_%s(&clients, &in_s, &out_s);\n"
                           "    if (rc != CDD_C_SUCCESS) return (int)rc;\n"
                           "    printf(\"{\\\"workflowId\\\":\\\"%s\\\","
                           "\\\"statusCode\\\":%%d}\\n\", "
                           "out_s.last_status_code);\n"
                           "    return 0;\n"
                           "  }\n",
                           wf_id, pfx, wf_id, pfx, wf_id, pfx, wf_id, wf_id);
  }

  cdd_code_writer_write(&w, "  return (int)CDD_C_ERROR_INVALID_ARGUMENT;\n}\n");

  rc = cdd_code_writer_take_string(&w, out_cli_src);
  cdd_code_writer_free(&w);
  return rc;
}

/**
 * @brief Generates the C SDK workflow files and the CLI workflow runner source
 * file (`<base>_workflow_cli.c`) from an Arazzo v1.1.0 specification.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] openapi_spec Optional linked OpenAPI specification (may be NULL).
 * @param[in] config Client generation configuration (`filename_base`,
 * `func_prefix`, etc.).
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on NULL,
 * `CDD_C_ERROR_MEMORY` on OOM, or `CDD_C_ERROR_IO` on file failure.
 */
cdd_c_error_t arazzo_cli_generate(const struct Arazzo_Spec *arazzo_spec,
                                  const struct OpenAPI_Spec *openapi_spec,
                                  const struct OpenApiClientConfig *config) {
  char *dir_name = NULL;
  char *base_name = NULL;
  char *cli_path = NULL;
  char *cli_content = NULL;
  size_t path_cap;
  cdd_c_error_t rc;

  if (!arazzo_spec || !config || !config->filename_base) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* Generate the underlying SDK workflows first so the CLI can reuse them */
  rc = arazzo_client_generate(arazzo_spec, openapi_spec, config);
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

  path_cap = strlen(dir_name) + strlen(base_name) + 32;
  cli_path = (char *)C_CDD_MALLOC(path_cap);
  if (!cli_path) {
    C_CDD_FREE(dir_name);
    C_CDD_FREE(base_name);
    return CDD_C_ERROR_MEMORY;
  }
  CDD_SNPRINTF(cli_path, path_cap, "%s/src/%s_workflow_cli.c", dir_name,
               base_name);

  rc = arazzo_cli_render_to_string(
      arazzo_spec, base_name, config->func_prefix ? config->func_prefix : "",
      &cli_content);
  if (rc == CDD_C_SUCCESS) {
    rc = fs_write_to_file(cli_path, cli_content);
    C_CDD_FREE(cli_content);
  }

  C_CDD_FREE(cli_path);
  C_CDD_FREE(dir_name);
  C_CDD_FREE(base_name);
  return rc;
}
