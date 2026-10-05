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

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_client_gen_fail = 0;
#endif

/**
 * @brief Executes the openapi client generate operation.
 */
cdd_c_error_t
openapi_client_generate(const struct OpenAPI_Spec *spec,
                        const struct OpenApiClientConfig *config) {
  char *_ast_generate_guard_10 = NULL;
  char *_ast_derive_model_header_11 = NULL;
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
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 24) {
      free(dir_name);
      _rc = CDD_C_ERROR_INVALID_ARGUMENT;
    }
#endif
    if (_rc != CDD_C_SUCCESS) {
      return _rc;
    }
    _rc = get_basename(config->filename_base, &base_name);
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 25)
      _rc = CDD_C_ERROR_INVALID_ARGUMENT;
#endif
    if (_rc != CDD_C_SUCCESS) {
      free(dir_name);
      return _rc;
    }
  }

  {
    char *src_dir = malloc(512);
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 10) {
      free(src_dir);
      src_dir = NULL;
    }
#endif
    if (!src_dir)
      return CDD_C_ERROR_MEMORY;
    CDD_SNPRINTF(src_dir, 512, "%s/src", dir_name);
    {
      cdd_c_error_t rc_cg = makedirs(src_dir);
#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 62)
        rc_cg = CDD_C_ERROR_IO;
#endif
      if (rc_cg != CDD_C_SUCCESS) {
        free(src_dir);
        free(dir_name);
        free(base_name);
        return rc_cg;
      }
    }
    actual_base = malloc(strlen(src_dir) + strlen(base_name) + 2);
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 11 || g_client_gen_fail == 61) {
      free(actual_base);
      actual_base = NULL;
    }
#endif
    if (actual_base) {
      CDD_SNPRINTF(actual_base, strlen(src_dir) + strlen(base_name) + 2,
                   "%s/%s", src_dir, base_name);
    }
    free(src_dir);
  }

  if (!actual_base) {
    actual_base = strdup(config->filename_base);
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 61) {
      free(actual_base);
      actual_base = NULL;
    }
#endif
    if (!actual_base)
      return CDD_C_ERROR_MEMORY;
  }

  /* Prepare filenames */
  h_name = malloc(strlen(actual_base) + 3);   /* .h */
  c_name = malloc(strlen(actual_base) + 3);   /* .c */
  mh_name = malloc(strlen(actual_base) + 10); /* _models.h */
  mc_name = malloc(strlen(actual_base) + 10); /* _models.c */
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 12) {
    free(h_name);
    h_name = NULL;
  }
  if (g_client_gen_fail == 75) {
    free(c_name);
    c_name = NULL;
  }
  if (g_client_gen_fail == 76) {
    free(mh_name);
    mh_name = NULL;
  }
  if (g_client_gen_fail == 77) {
    free(mc_name);
    mc_name = NULL;
  }
#endif
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

#if defined(_MSC_VER)
  if (fopen_s(&hfile, h_name, "w") != 0)
    hfile = NULL;
  if (fopen_s(&cfile, c_name, "w") != 0)
    cfile = NULL;
  if (fopen_s(&mhfile, mh_name, "w") != 0)
    mhfile = NULL;
  if (fopen_s(&mcfile, mc_name, "w") != 0)
    mcfile = NULL;
#else
#if defined(_MSC_VER)
  if (fopen_s(&hfile, h_name, "w") != 0)
    hfile = NULL;
#else
  hfile = fopen(h_name, "w");
#endif
#if defined(_MSC_VER)
  if (fopen_s(&cfile, c_name, "w") != 0)
    cfile = NULL;
#else
  cfile = fopen(c_name, "w");
#endif
#if defined(_MSC_VER)
  if (fopen_s(&mhfile, mh_name, "w") != 0)
    mhfile = NULL;
#else
  mhfile = fopen(mh_name, "w");
#endif
#if defined(_MSC_VER)
  if (fopen_s(&mcfile, mc_name, "w") != 0)
    mcfile = NULL;
#else
  mcfile = fopen(mc_name, "w");
#endif
#endif
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 16) {
    fclose(hfile);
    hfile = NULL;
  }
  if (g_client_gen_fail == 78) {
    fclose(cfile);
    cfile = NULL;
  }
  if (g_client_gen_fail == 79) {
    fclose(mhfile);
    mhfile = NULL;
  }
  if (g_client_gen_fail == 80) {
    fclose(mcfile);
    mcfile = NULL;
  }
#endif
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
    guard = strdup(config->header_guard);
  else
    guard = (generate_guard(config->filename_base, &_ast_generate_guard_10),
             _ast_generate_guard_10);

  if (config->model_header)
    model_h = strdup(config->model_header);
  else
    model_h = (derive_model_header(base_name, &_ast_derive_model_header_11),
               _ast_derive_model_header_11);

  if (config->func_prefix)
    prefix = config->func_prefix;

#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 17) {
    free(guard);
    guard = NULL;
  }
  if (g_client_gen_fail == 81) {
    free(model_h);
    model_h = NULL;
  }
#endif
  if (!guard || !model_h) {
    rc = CDD_C_ERROR_MEMORY;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  model_guard = malloc(strlen(guard) + 8);
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 18) {
    free(model_guard);
    model_guard = NULL;
  }
#endif
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
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 21)
    rc = CDD_C_ERROR_IO;
  else
#endif
    rc = write_header_preamble(hfile, guard, model_h);
  if (rc != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  {
    char *base = NULL;
    {
      cdd_c_error_t rc_cg = get_basename(h_name, &base);
#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 53)
        rc_cg = CDD_C_ERROR_INVALID_ARGUMENT;
#endif
      if (rc_cg != CDD_C_SUCCESS) {
        rc = rc_cg;
        goto cleanup;
      }
    }
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 22)
      rc = CDD_C_ERROR_IO;
    else
#endif
      rc = write_source_preamble(cfile, base);
    free(base);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  /* --- Write Lifecycle --- */
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 23)
    rc = CDD_C_ERROR_IO;
  else
#endif
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
#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 71)
        rc = CDD_C_ERROR_IO;
      else
#endif
        rc = emit_operation(hfile, cfile, path, op, spec, config, prefix);
      if (rc != CDD_C_SUCCESS) {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
    for (j = 0; j < path->n_additional_operations; ++j) {
      struct OpenAPI_Operation *op = &path->additional_operations[j];
#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 72)
        rc = CDD_C_ERROR_IO;
      else
#endif
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

#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 84)
      rc = CDD_C_ERROR_IO;
    else
#endif
      rc = client_gen_emit_url_utils_h(dir_name);
    if (rc != CDD_C_SUCCESS) {
      goto cleanup;
    }

    CDD_SNPRINTF(cpath, 512, "%s/src/url_utils.c", dir_name);
#if defined(_MSC_VER)
    if (fopen_s(&uc, cpath, "w") != 0)
      uc = NULL;
#else
    uc = fopen(cpath, "w");
#endif
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 55) {
      fclose(uc);
      uc = NULL;
    }
#endif
    if (uc) {
#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 82)
        rc = CDD_C_ERROR_IO;
      else
#endif
        rc = client_gen_emit_url_utils_c1(uc);
      if (rc != CDD_C_SUCCESS) {
        fclose(uc);
        goto cleanup;
      }
#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 83)
        rc = CDD_C_ERROR_IO;
      else
#endif
        rc = client_gen_emit_url_utils_c2(uc);
      if (rc != CDD_C_SUCCESS) {
        fclose(uc);
        goto cleanup;
      }
      fclose(uc);
    }
  }

  if (config->create_tests_and_mocks) {
    rc = client_gen_emit_mocks(dir_name, spec);
    if (rc != CDD_C_SUCCESS) {
      goto cleanup;
    }
  }

  if (!config->no_installable_package) {
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 20)
      rc = CDD_C_ERROR_IO;
    else
#endif
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
  free(h_name);
  free(c_name);
  free(mh_name);
  free(mc_name);
  free(guard);
  free(model_h);
  free(model_guard);
  free(dir_name);
  free(base_name);
  free(actual_base);

  return rc;
}
