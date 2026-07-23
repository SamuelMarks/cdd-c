/**
 * @file makefile_scraper.c
 * @brief Implementation of legacy build script scraper.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/makefile_scraper.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
/* clang-format on */

static enum cdd_c_error my_strdup(const char *s, char **out_val) {
  size_t len;
  char *d;
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = NULL;
  if (!s)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  len = strlen(s) + 1;
  d = (char *)malloc(len);
  if (!d)
    return CDD_C_ERROR_MEMORY;
  memcpy(d, s, len);
  *out_val = d;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the build info init operation.
 */
enum cdd_c_error build_info_init(struct ExtractedBuildInfo *info) {
  if (!info)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  info->source_files = NULL;
  info->source_files_n = 0;
  info->include_dirs = NULL;
  info->include_dirs_n = 0;
  info->compile_defs = NULL;
  info->compile_defs_n = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the build info free operation.
 */
void build_info_free(struct ExtractedBuildInfo *info) {
  size_t i;
  if (!info)
    return;

  if (info->source_files) {
    for (i = 0; i < info->source_files_n; i++) {
      free(info->source_files[i]);
    }
    free(info->source_files);
  }
  if (info->include_dirs) {
    for (i = 0; i < info->include_dirs_n; i++) {
      free(info->include_dirs[i]);
    }
    free(info->include_dirs);
  }
  if (info->compile_defs) {
    for (i = 0; i < info->compile_defs_n; i++) {
      free(info->compile_defs[i]);
    }
    free(info->compile_defs);
  }

  (void)build_info_init(info);
}

/**
 * @brief Adds or sets string to array.
 */
static enum cdd_c_error add_string_to_array(char ***arr, size_t *n,
                                            const char *str) {
  size_t i;
  char **new_arr;
  enum cdd_c_error rc;
  /* check dupes */
  for (i = 0; i < *n; i++) {
    if (strcmp((*arr)[i], str) == 0)
      return CDD_C_SUCCESS;
  }
  new_arr = (char **)realloc(*arr, (*n + 1) * sizeof(char *));
  if (!new_arr)
    return CDD_C_ERROR_MEMORY;
  *arr = new_arr;
  rc = my_strdup(str, &(*arr)[*n]);
  if (rc != CDD_C_SUCCESS)
    return rc;
  (*n)++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the process token operation.
 */
static enum cdd_c_error process_token(struct ExtractedBuildInfo *info,
                                      const char *tok) {
  size_t len = strlen(tok);
  enum cdd_c_error rc = CDD_C_SUCCESS;
  if (len > 2 && tok[len - 2] == '.' && tok[len - 1] == 'c') {
    const char *eq = strchr(tok, '=');
    if (eq) {
      rc = add_string_to_array(&info->source_files, &info->source_files_n,
                               eq + 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      rc = add_string_to_array(&info->source_files, &info->source_files_n, tok);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (len > 2) {
    char *inc = strstr(tok, "-I");
    char *def = strstr(tok, "-D");
    if (inc) {
      rc = add_string_to_array(&info->include_dirs, &info->include_dirs_n,
                               inc + 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (def) {
      rc = add_string_to_array(&info->compile_defs, &info->compile_defs_n,
                               def + 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the scrape makefile operation.
 */
enum cdd_c_error scrape_makefile(struct ExtractedBuildInfo *info,
                                 const char *makefile_content) {
  char *copy;
  char *saveptr;
  char *tok;
  enum cdd_c_error rc;

  if (!info || !makefile_content)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  my_strdup(makefile_content, &copy);
  if (!copy) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

#if defined(_WIN32)
  tok = strtok_s(copy, " \t\n\r\\", &saveptr);
#else
  tok = strtok_r(copy, " \t\n\r\\", &saveptr);
#endif
  while (tok) {
    rc = process_token(info, tok);
    if (rc != CDD_C_SUCCESS) {
      free(copy);
      return rc;
    }
#if defined(_WIN32)
    tok = strtok_s(NULL, " \t\n\r\\", &saveptr);
#else
    tok = strtok_r(NULL, " \t\n\r\\", &saveptr);
#endif
  }

  free(copy);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the scrape configure ac operation.
 */
enum cdd_c_error scrape_configure_ac(
    struct ExtractedBuildInfo *info,
    const char
        *configure_ac_content) { /* Minimal implementation: scan similarly to
                                    Makefile for basic definitions */
  char *copy;
  char *saveptr;
  char *tok;
  enum cdd_c_error rc;

  if (!info || !configure_ac_content)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  my_strdup(configure_ac_content, &copy);
  if (!copy) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

#if defined(_WIN32)
  tok = strtok_s(copy, " \t\n\r\\()[]\",", &saveptr);
#else
  tok = strtok_r(copy, " \t\n\r\\()[]\",", &saveptr);
#endif
  while (tok) {
    rc = process_token(info, tok);
    if (rc != CDD_C_SUCCESS) {
      free(copy);
      return rc;
    }
#if defined(_WIN32)
    tok = strtok_s(NULL, " \t\n\r\\()[]\",", &saveptr);
#else
    tok = strtok_r(NULL, " \t\n\r\\()[]\",", &saveptr);
#endif
  }

  free(copy);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the build info to cmake operation.
 */
enum cdd_c_error build_info_to_cmake(const struct ExtractedBuildInfo *info,
                                     const char *project_name,
                                     char **out_cmake) {
  size_t cap = 4096;
  size_t len = 0;
  char *buf;
  size_t i;

  if (!info || !project_name || !out_cmake)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  buf = (char *)malloc(cap);
  if (!buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(buf + len, cap - len, _TRUNCATE,
                     "cmake_minimum_required(VERSION 3.10)\n"
                     "project(%s C)\n\n ",
                     project_name);
#else
  len += CDD_SNPRINTF(buf + len, cap - len,
                      "cmake_minimum_required(VERSION 3.10)\n"
                      "project(%s C)\n\n ",
                      project_name);
#endif

  if (info->compile_defs_n > 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(buf + len, cap - len, _TRUNCATE,
                       "add_compile_definitions(\n");
#else
    len += CDD_SNPRINTF(buf + len, cap - len, "add_compile_definitions(\n");
#endif
    for (i = 0; i < info->compile_defs_n; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      len += _snprintf_s(buf + len, cap - len, _TRUNCATE, "  %s\n",
                         info->compile_defs[i]);
#else
      len +=
          CDD_SNPRINTF(buf + len, cap - len, "  %s\n", info->compile_defs[i]);
#endif
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(buf + len, cap - len, _TRUNCATE, ")\n\n");
#else
    len += CDD_SNPRINTF(buf + len, cap - len, ")\n\n");
#endif
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(buf + len, cap - len, _TRUNCATE, "add_executable(%s\n",
                     project_name);
#else
  len +=
      CDD_SNPRINTF(buf + len, cap - len, "add_executable(%s\n", project_name);
#endif

  for (i = 0; i < info->source_files_n; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(buf + len, cap - len, _TRUNCATE, "  %s\n",
                       info->source_files[i]);
#else
    len += CDD_SNPRINTF(buf + len, cap - len, "  %s\n", info->source_files[i]);
#endif
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(buf + len, cap - len, _TRUNCATE, ")\n\n");
#else
  len += CDD_SNPRINTF(buf + len, cap - len, ")\n\n");
#endif

  if (info->include_dirs_n > 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(buf + len, cap - len, _TRUNCATE,
                       "target_include_directories(%s PRIVATE\n", project_name);
#else
    len +=
        CDD_SNPRINTF(buf + len, cap - len,
                     "target_include_directories(%s PRIVATE\n", project_name);
#endif
    for (i = 0; i < info->include_dirs_n; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      len += _snprintf_s(buf + len, cap - len, _TRUNCATE, "  %s\n",
                         info->include_dirs[i]);
#else
      len +=
          CDD_SNPRINTF(buf + len, cap - len, "  %s\n", info->include_dirs[i]);
#endif
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(buf + len, cap - len, _TRUNCATE, ")\\n");
#else
    len += CDD_SNPRINTF(buf + len, cap - len, ")\\n");
#endif
  }

  *out_cmake = buf;
  return CDD_C_SUCCESS;
}
