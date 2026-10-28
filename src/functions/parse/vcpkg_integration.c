/**
 * @file vcpkg_integration.c
 * @brief Implementation of Vcpkg Integration Generator.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/memory.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include "functions/parse/tokenizer.h"
#include "functions/parse/vcpkg_integration.h"
/* clang-format on */

/**
 * @brief Duplicates a null-terminated string into `*out_val`.
 *
 * @param[in] s Source string to duplicate.
 * @param[out] out_val Output pointer receiving the newly allocated copy.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t my_strdup(const char *s, char **out_val) {
  size_t len;
  char *d;
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = NULL;
  if (!s)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  len = strlen(s) + 1;
  d = (char *)C_CDD_MALLOC(len);
  if (!d)
    return CDD_C_ERROR_MEMORY;
  memcpy(d, s, len);
  *out_val = d;
  return CDD_C_SUCCESS;
}

/**
 * @brief Initializes a vcpkg manifest builder context.
 *
 * @param[out] builder Pointer to the VcpkgManifestBuilder structure to
 * initialize.
 * @param[in] project_name Package name for `vcpkg.json`.
 * @param[in] version_string Optional package version string (defaults to
 * `"0.0.1"` if NULL).
 * @param[in] description Optional package description string (may be NULL).
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t vcpkg_builder_init(struct VcpkgManifestBuilder *builder,
                                 const char *project_name,
                                 const char *version_string,
                                 const char *description) {
  if (!builder || !project_name) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  builder->project_name = NULL;
  builder->version_string = NULL;
  builder->description = NULL;
  builder->deps = NULL;
  builder->deps_count = 0;
  builder->deps_capacity = 0;

  {
    cdd_c_error_t rc_vc = my_strdup(project_name, &builder->project_name);
    if (rc_vc != CDD_C_SUCCESS) {
      vcpkg_builder_free(builder);
      return rc_vc;
    }
  }
  if (version_string) {
    cdd_c_error_t rc_vc = my_strdup(version_string, &builder->version_string);
    if (rc_vc != CDD_C_SUCCESS) {
      vcpkg_builder_free(builder);
      return rc_vc;
    }
  } else {
    cdd_c_error_t rc_vc = my_strdup("0.0.1", &builder->version_string);
    if (rc_vc != CDD_C_SUCCESS) {
      vcpkg_builder_free(builder);
      return rc_vc;
    }
  }
  if (description) {
    cdd_c_error_t rc_vc = my_strdup(description, &builder->description);
    if (rc_vc != CDD_C_SUCCESS) {
      vcpkg_builder_free(builder);
      return rc_vc;
    }
  } else {
    cdd_c_error_t rc_vc = my_strdup("", &builder->description);
    if (rc_vc != CDD_C_SUCCESS) {
      vcpkg_builder_free(builder);
      return rc_vc;
    }
  }
  builder->deps = NULL;
  builder->deps_count = 0;
  builder->deps_capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Releases memory held by a vcpkg manifest builder context.
 *
 * @param[in,out] builder Pointer to the VcpkgManifestBuilder structure to free.
 */
void vcpkg_builder_free(struct VcpkgManifestBuilder *builder) {
  size_t i;
  if (!builder)
    return;

  if (builder->project_name)
    C_CDD_FREE(builder->project_name);
  if (builder->version_string)
    C_CDD_FREE(builder->version_string);
  if (builder->description)
    C_CDD_FREE(builder->description);

  if (builder->deps) {
    for (i = 0; i < builder->deps_count; i++) {
      if (builder->deps[i].name)
        C_CDD_FREE(builder->deps[i].name);
    }
    C_CDD_FREE(builder->deps);
  }
}

/**
 * @brief Adds a package dependency to a vcpkg manifest builder.
 *
 * @param[in,out] builder VcpkgManifestBuilder context to update.
 * @param[in] dep_name Vcpkg dependency package name to add.
 * @return 0 on success, ENOMEM on failure.
 */
cdd_c_error_t vcpkg_builder_add_dep(struct VcpkgManifestBuilder *builder,
                                    const char *dep_name) {
  size_t i;
  if (!builder || !dep_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Prevent duplicates */
  for (i = 0; i < builder->deps_count; i++) {
    if (strcmp(builder->deps[i].name, dep_name) == 0) {
      return CDD_C_SUCCESS; /* Already added */
    }
  }

  if (builder->deps_count >= builder->deps_capacity) {
    size_t new_cap =
        builder->deps_capacity == 0 ? 4 : builder->deps_capacity * 2;
    struct VcpkgDependency *new_deps = (struct VcpkgDependency *)C_CDD_REALLOC(
        builder->deps, new_cap * sizeof(struct VcpkgDependency));
    if (!new_deps) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    builder->deps = new_deps;
    builder->deps_capacity = new_cap;
  }

  {
    cdd_c_error_t rc_vc =
        my_strdup(dep_name, &builder->deps[builder->deps_count].name);
    if (rc_vc != CDD_C_SUCCESS)
      return rc_vc;
  }
  builder->deps_count++;

  return CDD_C_SUCCESS;
}

/**
 * @brief Scans C source includes to infer required vcpkg dependencies.
 *
 * @param[in,out] builder VcpkgManifestBuilder context to update.
 * @param[in] file_content Raw C source or header file content to scan.
 * @return 0 on success.
 */
cdd_c_error_t vcpkg_builder_scan_source(struct VcpkgManifestBuilder *builder,
                                        const char *file_content) {
  struct TokenList *tokens = NULL;
  cdd_c_error_t res;
  size_t i;

  if (!builder || !file_content)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  res = tokenize(az_span_create_from_str(file_content), &tokens);
  if (res != 0)
    return res;

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_HASH) {
      size_t j = i + 1;
      while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE)
        j++;
      if (j < tokens->size && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        int is_inc = 0;
        res = token_matches_string(&tokens->tokens[j], "include", &is_inc);
        if (is_inc) {
          size_t k = j + 1;
          while (k < tokens->size && tokens->tokens[k].kind == TOKEN_WHITESPACE)
            k++;

          /* Need to parse until newline to capture <pthread.h> because < and >
           * are tokens */
          if (k < tokens->size) {
            size_t end_inc = k;
            while (end_inc < tokens->size &&
                   tokens->tokens[end_inc].kind != TOKEN_WHITESPACE) {
              end_inc++;
            }

            {
              const char *inc_start = (const char *)tokens->tokens[k].start;
              const char *inc_end =
                  (const char *)tokens->tokens[end_inc - 1].start +
                  tokens->tokens[end_inc - 1].length;
              size_t inc_len = (size_t)(inc_end - inc_start);
              char *inc_str = (char *)C_CDD_MALLOC(inc_len + 1);
              if (!inc_str) {
                free_token_list(tokens);
                return CDD_C_ERROR_MEMORY;
              }
              memcpy(inc_str, inc_start, inc_len);
              inc_str[inc_len] = '\0';

              if (strstr(inc_str, "pthread.h")) {
                cdd_c_error_t rc = vcpkg_builder_add_dep(builder, "pthreads");
                if (rc != CDD_C_SUCCESS) {
                  C_CDD_FREE(inc_str);
                  free_token_list(tokens);
                  return rc;
                }
              }
              if (strstr(inc_str, "dirent.h")) {
                cdd_c_error_t rc = vcpkg_builder_add_dep(builder, "dirent");
                if (rc != CDD_C_SUCCESS) {
                  C_CDD_FREE(inc_str);
                  free_token_list(tokens);
                  return rc;
                }
              }
              if (strstr(inc_str, "zlib.h")) {
                cdd_c_error_t rc = vcpkg_builder_add_dep(builder, "zlib");
                if (rc != CDD_C_SUCCESS) {
                  C_CDD_FREE(inc_str);
                  free_token_list(tokens);
                  return rc;
                }
              }

              C_CDD_FREE(inc_str);
            }
          }
        }
      }
    }
  }
  free_token_list(tokens);
  return CDD_C_SUCCESS;
}

/**
 * @brief Serializes a vcpkg.json manifest file from collected dependencies.
 *
 * @param[in] builder VcpkgManifestBuilder context to serialize.
 * @param[out] out_json Output pointer receiving the newly allocated
 * `vcpkg.json` string.
 * @return 0 on success, ENOMEM on failure.
 */
cdd_c_error_t vcpkg_builder_generate(const struct VcpkgManifestBuilder *builder,
                                     char **out_json) {
  char *json = NULL;
  size_t cap = 1024;
  size_t len = 0;
  size_t i;

  if (!builder || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  json = (char *)C_CDD_MALLOC(cap);
  if (!json) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  /* Basic manual string builder without heavy dependencies */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(json + len, cap - len, _TRUNCATE,
                     "{\n  \"name\": \"%s\",\n  \"version-string\": \"%s\",\n  "
                     "\"description\": \"%s\"",
                     builder->project_name, builder->version_string,
                     builder->description);
#else
  len += (size_t)CDD_SNPRINTF(
      json + len, cap - len,
      "{\n  \"name\": \"%s\",\n  \"version-string\": \"%s\",\n  "
      "\"description\": \"%s\"",
      builder->project_name, builder->version_string, builder->description);
#endif

  if (builder->deps_count > 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(json + len, cap - len, _TRUNCATE,
                       ",\n  \"dependencies\": [\n");
#else
    len += (size_t)CDD_SNPRINTF(json + len, cap - len,
                                ",\n  \"dependencies\": [\n");
#endif
    for (i = 0; i < builder->deps_count; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      len += _snprintf_s(json + len, cap - len, _TRUNCATE, "    \"%s\"%s\\n",
                         builder->deps[i].name,
                         i == builder->deps_count - 1 ? "" : ",");
#else
      len += (size_t)CDD_SNPRINTF(json + len, cap - len, "    \"%s\"%s\\n",
                                  builder->deps[i].name,
                                  i == builder->deps_count - 1 ? "" : ",");
#endif
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(json + len, cap - len, _TRUNCATE, "  ]\\n");
#else
    len += (size_t)CDD_SNPRINTF(json + len, cap - len, "  ]\\n");
#endif
  } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(json + len, cap - len, _TRUNCATE, "\\n");
#else
    len += (size_t)CDD_SNPRINTF(json + len, cap - len, "\\n");
#endif
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(json + len, cap - len, _TRUNCATE, "}\n");
#else
  len += (size_t)CDD_SNPRINTF(json + len, cap - len, "}\n");
#endif

  *out_json = json;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/**
 * @brief Unit test helper verifying `my_strdup` error paths in
 * `vcpkg_integration.c`.
 *
 * @return `CDD_C_SUCCESS` if both NULL checks return
 * `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
C_CDD_EXPORT cdd_c_error_t test_vcpkg_my_strdup_errors(void);
C_CDD_EXPORT cdd_c_error_t test_vcpkg_my_strdup_errors(void) {
  char *out = NULL;
  cdd_c_error_t err1 = my_strdup(NULL, &out);
  cdd_c_error_t err2 = my_strdup("a", NULL);
  return (cdd_c_error_t)((err1 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err2 ^ CDD_C_ERROR_INVALID_ARGUMENT));
}
#endif
