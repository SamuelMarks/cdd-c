/**
 * @file vcpkg_integration.c
 * @brief Implementation of Vcpkg Integration Generator.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/tokenizer.h"
#include "functions/parse/vcpkg_integration.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
/* clang-format on */

static enum cdd_c_error my_strdup(const char *s, char **out_val) {
  size_t len;
  char *d;
  if (!out_val)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *out_val = NULL;
  if (!s)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  len = strlen(s) + 1;
  d = (char *)malloc(len);
  if (!d)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */
  memcpy(d, s, len);
  *out_val = d;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the vcpkg builder init operation.
 */
enum cdd_c_error vcpkg_builder_init(struct VcpkgManifestBuilder *builder,
                                    const char *project_name,
                                    const char *version_string,
                                    const char *description) {
  if (!builder || !project_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  my_strdup(project_name, &builder->project_name);
  if (version_string)
    my_strdup(version_string, &builder->version_string);
  else
    my_strdup("0.0.1", &builder->version_string);
  if (description)
    my_strdup(description, &builder->description);
  else
    my_strdup("", &builder->description);
  builder->deps = NULL;
  builder->deps_count = 0;
  builder->deps_capacity = 0;

  if (!builder->project_name || !builder->version_string ||
      !builder->description) {
    /* LCOV_EXCL_START */
    vcpkg_builder_free(builder);
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the vcpkg builder free operation.
 */
void vcpkg_builder_free(struct VcpkgManifestBuilder *builder) {
  size_t i;
  if (!builder)
    /* LCOV_EXCL_START */
    return;
  /* LCOV_EXCL_STOP */

  if (builder->project_name)
    free(builder->project_name);
  if (builder->version_string)
    free(builder->version_string);
  if (builder->description)
    free(builder->description);

  if (builder->deps) {
    for (i = 0; i < builder->deps_count; i++) {
      if (builder->deps[i].name)
        free(builder->deps[i].name);
    }
    free(builder->deps);
  }
}

/**
 * @brief Executes the vcpkg builder add dep operation.
 */
enum cdd_c_error vcpkg_builder_add_dep(struct VcpkgManifestBuilder *builder,
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
    struct VcpkgDependency *new_deps = (struct VcpkgDependency *)realloc(
        builder->deps, new_cap * sizeof(struct VcpkgDependency));
    if (!new_deps) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    builder->deps = new_deps;
    builder->deps_capacity = new_cap;
  }

  my_strdup(dep_name, &builder->deps[builder->deps_count].name);
  if (!builder->deps[builder->deps_count].name)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */
  builder->deps_count++;

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the vcpkg builder scan source operation.
 */
enum cdd_c_error vcpkg_builder_scan_source(struct VcpkgManifestBuilder *builder,
                                           const char *file_content) {
  struct TokenList *tokens = NULL;
  int res;
  size_t i;

  if (!builder || !file_content)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  res = tokenize(az_span_create_from_str((char *)file_content), &tokens);
  if (res != 0)
    /* LCOV_EXCL_START */
    return res;
  /* LCOV_EXCL_STOP */

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_HASH) {
      size_t j = i + 1;
      while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE)
        j++;
      if (j < tokens->size && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        int is_inc = 0;
        token_matches_string(&tokens->tokens[j], "include", &is_inc);
        if (is_inc) {
          size_t k = j + 1;
          while (k < tokens->size && tokens->tokens[k].kind == TOKEN_WHITESPACE)
            k++;

          /* Need to parse until newline to capture <pthread.h> because < and >
           * are tokens */
          if (k < tokens->size) {
            size_t end_inc = k;
            while (end_inc < tokens->size &&
                   tokens->tokens[end_inc].kind != TOKEN_WHITESPACE &&
                   tokens->tokens[end_inc].start[0] != '\n') {
              end_inc++;
            }

            if (end_inc > k) {
              const char *inc_start = (const char *)tokens->tokens[k].start;
              const char *inc_end =
                  (const char *)tokens->tokens[end_inc - 1].start +
                  tokens->tokens[end_inc - 1].length;
              size_t inc_len = (size_t)(inc_end - inc_start);
              char *inc_str = (char *)malloc(inc_len + 1);
              if (inc_str) {
                memcpy(inc_str, inc_start, inc_len);
                inc_str[inc_len] = '\0';

                if (strstr(inc_str, "pthread.h"))
                  vcpkg_builder_add_dep(builder, "pthreads");
                if (strstr(inc_str, "dirent.h"))
                  vcpkg_builder_add_dep(builder, "dirent");
                if (strstr(inc_str, "zlib.h"))
                  vcpkg_builder_add_dep(builder, "zlib");

                free(inc_str);
              }
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
 * @brief Executes the vcpkg builder generate operation.
 */
enum cdd_c_error
vcpkg_builder_generate(const struct VcpkgManifestBuilder *builder,
                       char **out_json) {
  char *json = NULL;
  size_t cap = 1024;
  size_t len = 0;
  size_t i;

  if (!builder || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  json = (char *)malloc(cap);
  if (!json) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  /* Basic manual string builder without heavy dependencies */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(json + len, cap - len, _TRUNCATE,
                     "{\n  \"name\": \"%s\",\n  \"version-string\": \"%s\",\n  "
                     "\"description\": \"%s\"",
                     builder->project_name, builder->version_string,
                     builder->description);
#else
  len += CDD_SNPRINTF(
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
    len += CDD_SNPRINTF(json + len, cap - len, ",\n  \"dependencies\": [\n");
#endif
    for (i = 0; i < builder->deps_count; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      len += _snprintf_s(json + len, cap - len, _TRUNCATE, "    \"%s\"%s\\n",
                         builder->deps[i].name,
                         i == builder->deps_count - 1 ? "" : ",");
#else
      len += CDD_SNPRINTF(json + len, cap - len, "    \"%s\"%s\\n",
                          builder->deps[i].name,
                          i == builder->deps_count - 1 ? "" : ",");
#endif
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(json + len, cap - len, _TRUNCATE, "  ]\\n");
#else
    len += CDD_SNPRINTF(json + len, cap - len, "  ]\\n");
#endif
  } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(json + len, cap - len, _TRUNCATE, "\\n");
#else
    /* LCOV_EXCL_START */
    len += CDD_SNPRINTF(json + len, cap - len, "\\n");
/* LCOV_EXCL_STOP */
#endif
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(json + len, cap - len, _TRUNCATE, "}\\n");
#else
  len += CDD_SNPRINTF(json + len, cap - len, "}\\n");
#endif

  *out_json = json;
  return CDD_C_SUCCESS;
}
