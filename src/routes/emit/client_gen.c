/**
 * @file client_gen.c
 * @brief Implementation of client code generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/emit/client_gen.h"
#include "routes/emit/client_gen_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include "functions/emit/build_system.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Generate the C Client Library (Header and Source).
 *
 * Performs the following steps:
 * 1. Opens `<basename>.h` and `<basename>.c` for writing.
 * 2. Writes Preamble (Includes, Type Definitions).
 *    - Injects `struct ApiError` definition for global error handling.
 * 3. Iterates through all Paths and Operations in the Spec.
 * 4. For each operation:
 *    - Generates a Doxygen-style docblock in the header.
 *    - Resolves naming convention (Namespace + Tag + Prefix).
 *    - Generates a function prototype in the header.
 *    - Generates the function definition and body in the source.
 * 5. Writes the `_init`, `_cleanup`, and `ApiError` lifecycle functions.
 *
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Configuration options for generation.
 * @return 0 on success, error code (EINVAL, ENOMEM, EIO) on failure.
 */
cdd_c_error_t
openapi_client_generate(const struct OpenAPI_Spec *spec,
                        const struct OpenApiClientConfig *config) {
  FILE *hfile = NULL, *cfile = NULL, *mhfile = NULL, *mcfile = NULL;
  char *h_name = NULL, *c_name = NULL, *mh_name = NULL, *mc_name = NULL;
  char *guard = NULL, *model_h = NULL, *model_guard = NULL;
  const char *prefix = "";
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t i, j;

  char *dir_name = NULL;
  char *base_name = NULL;
  char *actual_base = NULL;

  if (!spec || !config || !config->filename_base)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    cdd_c_error_t _rc;
    _rc = get_dirname(config->filename_base, &dir_name);
    if (_rc != CDD_C_SUCCESS) {
      return _rc;
    }
    _rc = get_basename(config->filename_base, &base_name);
    if (_rc != CDD_C_SUCCESS) {
      C_CDD_FREE(dir_name);
      return _rc;
    }
  }

  {
    char *src_dir = C_CDD_MALLOC(512);
    if (!src_dir) {
      C_CDD_FREE(dir_name);
      C_CDD_FREE(base_name);
      return CDD_C_ERROR_MEMORY;
    }
    CDD_SNPRINTF(src_dir, 512, "%s/src", dir_name);
    rc = makedirs(src_dir);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(src_dir);
      C_CDD_FREE(dir_name);
      C_CDD_FREE(base_name);
      return rc;
    }
    actual_base = C_CDD_MALLOC(strlen(dir_name) + strlen(base_name) + 6);
    if (actual_base) {
      CDD_SNPRINTF(actual_base, strlen(dir_name) + strlen(base_name) + 6,
                   "%s/src/%s", dir_name, base_name);
    }
    C_CDD_FREE(src_dir);
  }

  if (!actual_base) {
    actual_base = C_CDD_STRDUP(config->filename_base);
    if (!actual_base) {
      C_CDD_FREE(dir_name);
      C_CDD_FREE(base_name);
      return CDD_C_ERROR_MEMORY;
    }
  }

  /* Prepare filenames */
  h_name = C_CDD_MALLOC(strlen(actual_base) + 3);   /* .h */
  c_name = C_CDD_MALLOC(strlen(actual_base) + 3);   /* .c */
  mh_name = C_CDD_MALLOC(strlen(actual_base) + 10); /* _models.h */
  mc_name = C_CDD_MALLOC(strlen(actual_base) + 10); /* _models.c */
  if (!h_name || !c_name || !mh_name || !mc_name) {
    rc = CDD_C_ERROR_MEMORY;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(h_name, strlen(actual_base) + 3, "%s.h", actual_base);
  sprintf_s(c_name, strlen(actual_base) + 3, "%s.c", actual_base);
  sprintf_s(mh_name, strlen(actual_base) + 10, "%s_models.h", actual_base);
  sprintf_s(mc_name, strlen(actual_base) + 10, "%s_models.c", actual_base);
#else
  CDD_SNPRINTF(h_name, strlen(actual_base) + 3, "%s.h", actual_base);
  CDD_SNPRINTF(c_name, strlen(actual_base) + 3, "%s.c", actual_base);
  CDD_SNPRINTF(mh_name, strlen(actual_base) + 10, "%s_models.h", actual_base);
  CDD_SNPRINTF(mc_name, strlen(actual_base) + 10, "%s_models.c", actual_base);
#endif

  cdd_vfs_fopen(h_name, "w", &hfile);
  cdd_vfs_fopen(c_name, "w", &cfile);
  cdd_vfs_fopen(mh_name, "w", &mhfile);
  cdd_vfs_fopen(mc_name, "w", &mcfile);
  if (!hfile || !cfile || !mhfile || !mcfile) {
    rc = CDD_C_ERROR_IO;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  /* Prepare configurations */
  if (config->header_guard)
    guard = C_CDD_STRDUP(config->header_guard);
  else
    generate_guard(config->filename_base, &guard);

  if (config->model_header)
    model_h = C_CDD_STRDUP(config->model_header);
  else
    derive_model_header(base_name, &model_h);

  if (config->func_prefix)
    prefix = config->func_prefix;

  if (!guard || !model_h) {
    rc = CDD_C_ERROR_MEMORY;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  model_guard = C_CDD_MALLOC(strlen(guard) + 8);
  if (!model_guard) {
    rc = CDD_C_ERROR_MEMORY;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(model_guard, strlen(guard) + 8, "%s_MODELS", guard);
#else
  CDD_SNPRINTF(model_guard, strlen(guard) + 8, "%s_MODELS", guard);
#endif

  /* --- Write Models Header and Source --- */
  rc = client_gen_emit_models(mhfile, mcfile, model_guard, mh_name, spec);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "goto cleanup at %s:%d\n", __FILE__, __LINE__);
    goto cleanup;
  }

  /* --- Write Client Preamble --- */
  rc = write_header_preamble(hfile, guard, model_h);
  if (rc != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  {
    char *base = NULL;
    rc = get_basename(h_name, &base);
    if (rc != CDD_C_SUCCESS) {
      goto cleanup;
    }
    rc = write_source_preamble(cfile, base);
    C_CDD_FREE(base);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  /* --- Write Lifecycle --- */
  rc = write_lifecycle_funcs(hfile, cfile, prefix, spec);
  if (rc != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  /* --- Iterate Operations --- */
  for (i = 0; i < spec->n_paths; ++i) {
    struct OpenAPI_Path *path = &spec->paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      struct OpenAPI_Operation *op = &path->operations[j];
      rc = emit_operation(hfile, cfile, path, op, spec, config, prefix);
      if (rc != CDD_C_SUCCESS) {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
    for (j = 0; j < path->n_additional_operations; ++j) {
      struct OpenAPI_Operation *op = &path->additional_operations[j];
      rc = emit_operation(hfile, cfile, path, op, spec, config, prefix);
      if (rc != CDD_C_SUCCESS) {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  }

  /* --- Write MCP Adapters --- */
  rc = client_gen_emit_mcp(hfile, cfile, guard, prefix, spec);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "goto cleanup at %s:%d\n", __FILE__, __LINE__);
    goto cleanup;
  }

  if (!config->no_installable_package) {
    char cpath[512];
    FILE *uc = NULL;

    rc = client_gen_emit_url_utils_h(dir_name);
    if (rc != CDD_C_SUCCESS) {
      goto cleanup;
    }

    CDD_SNPRINTF(cpath, 512, "%s/src/url_utils.c", dir_name);
    cdd_vfs_fopen(cpath, "w", &uc);
    if (uc) {
      rc = (cdd_c_error_t)((int)client_gen_emit_url_utils_c1(uc) |
                           (int)client_gen_emit_url_utils_c2(uc));
      fclose(uc);
      if (rc != CDD_C_SUCCESS) {
        goto cleanup;
      }
    }
  }

  if (config->create_tests_and_mocks) {
    rc = client_gen_emit_mocks(dir_name, spec);
    if (rc != CDD_C_SUCCESS) {
      goto cleanup;
    }
  }

  if (!config->no_installable_package) {
    rc = generate_cmake_project(dir_name, base_name,
                                config->create_tests_and_mocks);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

cleanup:
  if (hfile)
    fclose(hfile);
  if (cfile)
    fclose(cfile);
  if (mhfile)
    fclose(mhfile);
  if (mcfile)
    fclose(mcfile);
  C_CDD_FREE(h_name);
  C_CDD_FREE(c_name);
  C_CDD_FREE(mh_name);
  C_CDD_FREE(mc_name);
  C_CDD_FREE(guard);
  C_CDD_FREE(model_h);
  C_CDD_FREE(model_guard);
  C_CDD_FREE(dir_name);
  C_CDD_FREE(base_name);
  C_CDD_FREE(actual_base);

  return rc;
}
