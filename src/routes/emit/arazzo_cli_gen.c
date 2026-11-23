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
  memset(&w, 0, sizeof(w));
  *out_cli_src = NULL;

  rc = cdd_code_writer_init_buffer(&w);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_printf(&w, "/**\n * @file %s_workflow_cli.c\n",
                              base_name);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_printf(
      &w, " * @brief Generated CLI Workflow Runner reusing `%s_workflows.h`.\n",
      base_name);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_printf(&w, " * @arazzo %s\n",
                              arazzo_spec->arazzo ? arazzo_spec->arazzo
                                                  : "1.1.0");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (arazzo_spec->info.title) {
    rc = cdd_code_writer_printf(&w, " * @infoTitle %s\n",
                                arazzo_spec->info.title);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (arazzo_spec->info.version) {
    rc = cdd_code_writer_printf(&w, " * @infoVersion %s\n",
                                arazzo_spec->info.version);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  rc = cdd_code_writer_write(&w, " */\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(&w, "/* clang-format "
                                 "off */\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(&w, "#include <stdio.h>\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(&w, "#include <stdlib.h>\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(&w, "#include <string.h>\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_printf(&w, "#include \"%s_workflows.h\"\n", base_name);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(&w, "/* clang-format "
                                 "on */\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "/**\n * @brief Prints available Arazzo CLI workflows.\n */\n"
          "static cdd_c_error_t print_workflow_cli_help(void) {\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_printf(
      &w, "  printf(\"%s v%s\\n\", \"%s\", \"%s\");\n", "%s", "%s",
      arazzo_spec->info.title ? arazzo_spec->info.title : "Arazzo CLI",
      arazzo_spec->info.version ? arazzo_spec->info.version : "1.0.0");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(
      &w,
      "  printf(\"Usage: cli [options] workflow <workflowId> [--input "
      "<json>]\\n\");\n"
      "  printf(\"Options:\\n\");\n"
      "  printf(\"  -i, --input <json>       Workflow inputs JSON "
      "object\\n\");\n"
      "  printf(\"  --dry-run                Validate without "
      "executing\\n\");\n"
      "  printf(\"  -v, --verbose            Enable verbose logging\\n\");\n"
      "  printf(\"  -h, --help               Print help\\n\");\n"
      "  printf(\"  -V, --version            Print version\\n\");\n"
      "  printf(\"Workflows:\\n\");\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  for (i = 0; i < arazzo_spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &arazzo_spec->workflows[i];
    rc = cdd_code_writer_printf(&w, "  printf(\"  %-24s %s\\n\");\n",
                                wf->workflow_id ? wf->workflow_id : "workflow",
                                wf->summary ? wf->summary : "");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  rc = cdd_code_writer_write(&w, "  return CDD_C_SUCCESS;\n}\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "/** @brief Parsed CLI options for the Arazzo workflow runner. */\n"
          "struct ArazzoCliOptions {\n"
          "  const char *wf_id;\n"
          "  const char *input_json;\n"
          "  int dry_run;\n"
          "  int verbose;\n"
          "  int want_help;\n"
          "  int want_version;\n"
          "  int want_wf_help;\n"
          "};\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(
      &w, "/**\n"
          " * @brief Parses CLI arguments into `struct ArazzoCliOptions`.\n"
          " *\n"
          " * @param[in] argc Argument count.\n"
          " * @param[in] argv Argument vector.\n"
          " * @param[out] out_opts Output options struct.\n"
          " * @return `CDD_C_SUCCESS` on success, or error code on failure.\n"
          " */\n"
          "static cdd_c_error_t parse_workflow_cli_options(\n"
          "    int argc, char **argv, struct ArazzoCliOptions *out_opts) {\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(
      &w, "  int i;\n"
          "  if (!argv || !out_opts) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  memset(out_opts, 0, sizeof(*out_opts));\n"
          "  out_opts->input_json = \"{}\";\n"
          "  if (argc < 2 || strcmp(argv[1], \"--help\") == 0 ||\n"
          "      strcmp(argv[1], \"-h\") == 0) {\n"
          "    out_opts->want_help = 1;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(
      &w, "  if (strcmp(argv[1], \"--version\") == 0 ||\n"
          "      strcmp(argv[1], \"-V\") == 0) {\n"
          "    out_opts->want_version = 1;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "  if (strcmp(argv[1], \"workflow\") == 0 && argc >= 3) {\n"
          "    out_opts->wf_id = argv[2];\n"
          "    i = 3;\n"
          "  } else {\n"
          "    out_opts->wf_id = argv[1];\n"
          "    i = 2;\n"
          "  }\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(
      &w, "  for (; i < argc; ++i) {\n"
          "    if (strcmp(argv[i], \"--help\") == 0 ||\n"
          "        strcmp(argv[i], \"-h\") == 0) {\n"
          "      out_opts->want_wf_help = 1;\n"
          "    } else if (strcmp(argv[i], \"--dry-run\") == 0) {\n"
          "      out_opts->dry_run = 1;\n"
          "    } else if (strcmp(argv[i], \"--verbose\") == 0 ||\n"
          "               strcmp(argv[i], \"-v\") == 0) {\n"
          "      out_opts->verbose++;\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(
      &w, "    } else if ((strcmp(argv[i], \"--input\") == 0 ||\n"
          "                strcmp(argv[i], \"-i\") == 0) && i + 1 < argc) {\n"
          "      out_opts->input_json = argv[++i];\n"
          "    }\n"
          "  }\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n\n"
          "/**\n * @brief CLI entry point dispatching Arazzo workflows.\n */\n"
          "int main(int argc, char **argv) {\n"
          "  struct ArazzoCliOptions opts;\n"
          "  const char *wf_id;\n"
          "  const char *input_json;\n"
          "  int dry_run;\n"
          "  int verbose;\n"
          "  int want_wf_help;\n"
          "  cdd_c_error_t parse_rc;\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_printf(&w, "  struct %sWorkflowClients clients;\n", pfx);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(
      &w, "  memset(&clients, 0, sizeof(clients));\n"
          "  parse_rc = parse_workflow_cli_options(argc, argv, &opts);\n"
          "  if (parse_rc != CDD_C_SUCCESS) return (int)parse_rc;\n"
          "  if (opts.want_help) {\n"
          "    return (int)print_workflow_cli_help();\n"
          "  }\n"
          "  if (opts.want_version) {\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_printf(
      &w, "    printf(\"%s v%s\\n\", \"%s\", \"%s\");\n", "%s", "%s",
      arazzo_spec->info.title ? arazzo_spec->info.title : "Arazzo CLI",
      arazzo_spec->info.version ? arazzo_spec->info.version : "1.0.0");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_code_writer_write(&w, "    return 0;\n"
                                 "  }\n"
                                 "  wf_id = opts.wf_id;\n"
                                 "  input_json = opts.input_json;\n"
                                 "  dry_run = opts.dry_run;\n"
                                 "  verbose = opts.verbose;\n"
                                 "  want_wf_help = opts.want_wf_help;\n"
                                 "  if (verbose > 1 && input_json) {}\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < arazzo_spec->n_workflows; ++i) {
    const struct Arazzo_Workflow *wf = &arazzo_spec->workflows[i];
    const char *wf_id = wf->workflow_id ? wf->workflow_id : "default_workflow";
    size_t p_idx;
    rc = cdd_code_writer_printf(&w,
                                "  if (strcmp(wf_id, \"%s\") == 0) {\n"
                                "    struct %sWorkflow_%s_Inputs in_s;\n"
                                "    struct %sWorkflow_%s_Outputs out_s;\n"
                                "    cdd_c_error_t rc;\n"
                                "    if (want_wf_help) {\n"
                                "      printf(\"Workflow: %s\\n\");\n",
                                wf_id, pfx, wf_id, pfx, wf_id, wf_id);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (wf->summary) {
      rc = cdd_code_writer_printf(&w, "      printf(\"Summary: %s\\n\");\n",
                                  wf->summary);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (wf->description) {
      rc = cdd_code_writer_printf(&w, "      printf(\"Description: %s\\n\");\n",
                                  wf->description);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (wf->n_depends_on > 0) {
      rc = cdd_code_writer_write(&w, "      printf(\"DependsOn:\\n\");\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      for (p_idx = 0; p_idx < wf->n_depends_on; ++p_idx) {
        if (wf->depends_on[p_idx]) {
          rc = cdd_code_writer_printf(&w, "      printf(\"  - %s\\n\");\n",
                                      wf->depends_on[p_idx]);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
    }
    if (wf->n_parameters > 0) {
      rc = cdd_code_writer_write(&w, "      printf(\"Inputs:\\n\");\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      for (p_idx = 0; p_idx < wf->n_parameters; ++p_idx) {
        const char *pn = wf->parameters[p_idx].name;
        if (pn) {
          rc = cdd_code_writer_printf(
              &w, "      printf(\"  --%s <val>\\n\");\n", pn);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
    }
    if (wf->n_outputs > 0) {
      rc = cdd_code_writer_write(&w, "      printf(\"Outputs:\\n\");\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      for (p_idx = 0; p_idx < wf->n_outputs; ++p_idx) {
        const char *ok = wf->outputs[p_idx].key;
        if (ok) {
          rc = cdd_code_writer_printf(&w, "      printf(\"  - %s\\n\");\n", ok);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
    }
    rc = cdd_code_writer_write(&w, "      return 0;\n"
                                   "    }\n"
                                   "    if (dry_run) {\n"
                                   "      return 0;\n"
                                   "    }\n"
                                   "    memset(&in_s, 0, sizeof(in_s));\n"
                                   "    memset(&out_s, 0, sizeof(out_s));\n"
                                   "    in_s.raw_json = input_json;\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_code_writer_printf(
        &w,
        "    rc = %sworkflow_%s(&clients, &in_s, &out_s);\n"
        "    if (rc != CDD_C_SUCCESS) return (int)rc;\n"
        "    printf(\"{\\\"workflowId\\\":\\\"%s\\\","
        "\\\"statusCode\\\":%%d}\\n\", "
        "out_s.last_status_code);\n"
        "    return 0;\n"
        "  }\n",
        pfx, wf_id, wf_id);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = cdd_code_writer_write(
      &w, "  return (int)CDD_C_ERROR_INVALID_ARGUMENT;\n}\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_take_string(&w, out_cli_src);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

cleanup: {
  cdd_c_error_t rc_free = cdd_code_writer_free(&w);
  return cdd_c_keep_err(rc, rc_free);
}
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
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(cli_path);
    C_CDD_FREE(dir_name);
    C_CDD_FREE(base_name);
    return rc;
  }
  rc = fs_write_to_file(cli_path, cli_content);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(cli_content);
    C_CDD_FREE(cli_path);
    C_CDD_FREE(dir_name);
    C_CDD_FREE(base_name);
    return rc;
  }
  C_CDD_FREE(cli_content);
  C_CDD_FREE(cli_path);
  C_CDD_FREE(dir_name);
  C_CDD_FREE(base_name);
  return CDD_C_SUCCESS;
}
