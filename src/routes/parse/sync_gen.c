/**
 * @file sync_gen.c
 * @brief Internal generator routines for API sync.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "functions/emit/client_sig.h"
#include "functions/parse/str.h"
#include "routes/emit/url.h"
#include "routes/parse/sync.h"
#include "routes/parse/sync_gen.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_make_tmpfile;
extern C_CDD_EXPORT int g_cdd_fail_generate_expected_sig;
extern C_CDD_EXPORT int g_cdd_fail_generate_expected_url;
extern C_CDD_EXPORT int g_cdd_fail_codegen_write;
#endif

/**
 * @brief Creates a temporary file in memory or system temp.
 *
 * @param[out] out_file Pointer to receive the opened temporary file.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t make_cdd_tmpfile(FILE **out_file) {
#if !defined(__wasm__) && !defined(__wasm32__)
  FILE *f = NULL;
#endif
  if (!out_file)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#if defined(__wasm__) || defined(__wasm32__)
  *out_file = NULL;
  return CDD_C_ERROR_IO;
#elif defined(_MSC_VER)
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_make_tmpfile) {
    g_cdd_fail_make_tmpfile = 0;
    *out_file = NULL;
    return CDD_C_ERROR_IO;
  }
#endif
  if (tmpfile_s(&f) != 0 || !f) {
    *out_file = NULL;
    return CDD_C_ERROR_IO;
  }
  *out_file = f;
  return CDD_C_SUCCESS;
#else
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_make_tmpfile) {
    g_cdd_fail_make_tmpfile = 0;
    f = NULL;
  } else
#endif
    f = tmpfile();
  if (!f) {
    *out_file = NULL;
    return CDD_C_ERROR_IO;
  }
  *out_file = f;
  return CDD_C_SUCCESS;
#endif
}

/**
 * @brief Generate expected signature string for an OpenAPI operation.
 *
 * @param[in] op Operation specification.
 * @param[in] cfg Synchronization configuration.
 * @param[out] _out_val Pointer to receive allocated signature string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t generate_expected_sig(const struct OpenAPI_Operation *op,
                                    const struct ApiSyncConfig *cfg,
                                    char **_out_val) {
  FILE *tmp = NULL;
  long sz;
  char *buf;
  struct CodegenSigConfig sig_cfg;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!op || !cfg)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = make_cdd_tmpfile(&tmp);
  if (rc != CDD_C_SUCCESS)
    return rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_generate_expected_sig) {
    g_cdd_fail_generate_expected_sig = 0;
    fclose(tmp);
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  memset(&sig_cfg, 0, sizeof(sig_cfg));
  sig_cfg.prefix = cfg->func_prefix;
  sig_cfg.include_semicolon = 0;

  rc = codegen_client_write_signature(tmp, op, &sig_cfg);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_codegen_write) {
    g_cdd_fail_codegen_write = 0;
    rc = CDD_C_ERROR_IO;
  }
#endif
  if (rc != CDD_C_SUCCESS) {
    fclose(tmp);
    return CDD_C_ERROR_IO;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  buf = (char *)C_CDD_MALLOC((size_t)sz + 1);
  if (!buf) {
    fclose(tmp);
    return CDD_C_ERROR_MEMORY;
  }

  {
    size_t len;
    size_t bytes_read = fread(buf, 1, (size_t)sz, tmp);
    buf[bytes_read] = '\0';
    c_cdd_str_trim_trailing_whitespace(buf);
    len = strlen(buf);
#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT int g_cdd_sync_sig_no_brace;
      if (g_cdd_sync_sig_no_brace) {
        g_cdd_sync_sig_no_brace = 0;
        buf[len - 1] = ')';
      }
    }
#endif
    if (buf[len - 1] == '{')
      buf[len - 1] = '\0';
    c_cdd_str_trim_trailing_whitespace(buf);
  }
  fclose(tmp);
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generate Query parameters block for an OpenAPI operation.
 *
 * @param[in] op Operation specification.
 * @param[out] _out_val Pointer to receive allocated query block string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t generate_expected_query(const struct OpenAPI_Operation *op,
                                      char **_out_val) {
  FILE *tmp = NULL;
  long sz;
  char *buf;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!op)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = make_cdd_tmpfile(&tmp);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = codegen_url_write_query_params(tmp, op, 0);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_codegen_write) {
    g_cdd_fail_codegen_write = 0;
    rc = CDD_C_ERROR_IO;
  }
#endif
  if (rc != CDD_C_SUCCESS) {
    fclose(tmp);
    return CDD_C_ERROR_IO;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  buf = (char *)C_CDD_MALLOC((size_t)sz + 1);
  if (!buf) {
    fclose(tmp);
    return CDD_C_ERROR_MEMORY;
  }

  {
    size_t bytes_read = fread(buf, 1, (size_t)sz, tmp);
    buf[bytes_read] = '\0';
  }
  fclose(tmp);
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generate Header parameters block line by line.
 *
 * @param[in] p Parameter specification.
 * @param[out] _out_val Pointer to receive allocated header line string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t generate_expected_header_line(const struct OpenAPI_Parameter *p,
                                            char **_out_val) {
  char *buf;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!p || !p->name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  buf = (char *)C_CDD_MALLOC(512);
  if (!buf)
    return CDD_C_ERROR_MEMORY;

  if (p->type && strcmp(p->type, "string") == 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, 512,
              "  /* Header Parameter: %s */\n  if (%s) {\n    rc = "
              "http_headers_add(&req.headers, \"%s\", %s);\n    if "
              "(rc != CDD_C_SUCCESS) goto cleanup;\n  }\n",
              p->name, p->name, p->name, p->name);
#else
    CDD_SNPRINTF(buf, 512,
                 "  /* Header Parameter: %s */\n  if (%s) {\n    rc = "
                 "http_headers_add(&req.headers, \"%s\", %s);\n    if (rc != "
                 "CDD_C_SUCCESS) "
                 "goto cleanup;\n  }\n",
                 p->name, p->name, p->name, p->name);
#endif
  } else if (p->type && strcmp(p->type, "integer") == 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, 512,
              "  /* Header Parameter: %s */\n  {\n    char num_buf[32];\n    "
              "sprintf_s(num_buf, sizeof(num_buf), \"%%d\", %s);\n    rc = "
              "http_headers_add(&req.headers, \"%s\", num_buf);\n    if (rc != "
              "0) goto cleanup;\n  }\n",
              p->name, p->name, p->name);
#else
    CDD_SNPRINTF(
        buf, 512,
        "  /* Header Parameter: %s */\n  {\n    char num_buf[32];\n    "
        "CDD_SNPRINTF(num_buf, sizeof(num_buf), \"%%d\", %s);\n    rc = "
        "http_headers_add(&req.headers, \"%s\", num_buf);\n    if (rc != "
        "0) goto cleanup;\n  }\n",
        p->name, p->name, p->name);
#endif
  } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, 512,
              "  /* Header Parameter: %s (Type unhandled in sync) */\n",
              p->name);
#else
    CDD_SNPRINTF(buf, 512,
                 "  /* Header Parameter: %s (Type unhandled in sync) */\n",
                 p->name);
#endif
  }
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generate URL builder logic for an OpenAPI operation.
 *
 * @param[in] path Path template string.
 * @param[in] op Operation specification.
 * @param[in] cfg Synchronization configuration.
 * @param[out] _out_val Pointer to receive allocated URL builder string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t generate_expected_url(const char *path,
                                    const struct OpenAPI_Operation *op,
                                    const struct ApiSyncConfig *cfg,
                                    char **_out_val) {
  FILE *tmp = NULL;
  long sz;
  char *buf;
  struct CodegenUrlConfig url_cfg;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!path || !op || !cfg)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = make_cdd_tmpfile(&tmp);
  if (rc != CDD_C_SUCCESS)
    return rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_generate_expected_url) {
    g_cdd_fail_generate_expected_url = 0;
    fclose(tmp);
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  memset(&url_cfg, 0, sizeof(url_cfg));
  url_cfg.out_variable = cfg->url_var_name ? cfg->url_var_name : "url";

  rc = codegen_url_write_builder(tmp, path, op->parameters, op->n_parameters,
                                 &url_cfg);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_codegen_write) {
    g_cdd_fail_codegen_write = 0;
    rc = CDD_C_ERROR_IO;
  }
#endif
  if (rc != CDD_C_SUCCESS) {
    fclose(tmp);
    return CDD_C_ERROR_IO;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  buf = (char *)C_CDD_MALLOC((size_t)sz + 1);
  if (!buf) {
    fclose(tmp);
    return CDD_C_ERROR_MEMORY;
  }

  {
    size_t bytes_read = fread(buf, 1, (size_t)sz, tmp);
    buf[bytes_read] = '\0';
  }
  fclose(tmp);
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
cdd_c_error_t cdd_test_sync_make_cdd_tmpfile(FILE **out_file) {
  return make_cdd_tmpfile(out_file);
}

cdd_c_error_t
cdd_test_sync_generate_expected_sig(const struct OpenAPI_Operation *op,
                                    const struct ApiSyncConfig *cfg,
                                    char **out_val) {
  return generate_expected_sig(op, cfg, out_val);
}

cdd_c_error_t
cdd_test_sync_generate_expected_query(const struct OpenAPI_Operation *op,
                                      char **out_val) {
  return generate_expected_query(op, out_val);
}

cdd_c_error_t
cdd_test_sync_generate_expected_header_line(const struct OpenAPI_Parameter *p,
                                            char **out_val) {
  return generate_expected_header_line(p, out_val);
}

cdd_c_error_t cdd_test_sync_generate_expected_url(
    const char *path, const struct OpenAPI_Operation *op,
    const struct ApiSyncConfig *cfg, char **out_val) {
  return generate_expected_url(path, op, cfg, out_val);
}

#endif /* CDD_BUILD_TESTS */
