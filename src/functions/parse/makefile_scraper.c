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
/* clang-format on */

static /**
        * @brief Executes the my strdup operation.
        */
    int
    my_strdup(const char *s, char **_out_val) {
  size_t len = strlen(s) + 1;
  char *d = (char *)malloc(len);
  if (!d)
    return NULL;
  memcpy(d, s, len);
  return d;
}

/**
 * @brief Executes the build info init operation.
 */
void build_info_init(struct ExtractedBuildInfo *info) {
  if (!info)
    return;
  info->source_files = NULL;
  info->source_files_n = 0;
  info->include_dirs = NULL;
  info->include_dirs_n = 0;
  info->compile_defs = NULL;
  info->compile_defs_n = 0;
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

  build_info_init(info);
}

static /**
        * @brief Adds or sets string to array.
        */
    void
    add_string_to_array(char ***arr, size_t *n, const char *str) {
  size_t i;
  /* check dupes */
  for (i = 0; i < *n; i++) {
    if (strcmp((*arr)[i], str) == 0)
      return;
  }
  *arr = (char **)realloc(*arr, (*n + 1) * sizeof(char *));
  (*arr)[*n] = my_strdup(str);
  (*n)++;
}

static /**
        * @brief Executes the process token operation.
        */
    void
    process_token(struct ExtractedBuildInfo *info, const char *tok) {
  size_t len = strlen(tok);
  if (len > 2 && tok[len - 2] == '.' && tok[len - 1] == 'c') {
    const char *eq = strchr(tok, '=');
    if (eq) {
      add_string_to_array(&info->source_files, &info->source_files_n, eq + 1);
    } else {
      add_string_to_array(&info->source_files, &info->source_files_n, tok);
    }
  }

  if (len > 2) {
    char *inc = strstr(tok, "-I");
    char *def = strstr(tok, "-D");
    if (inc) {
      add_string_to_array(&info->include_dirs, &info->include_dirs_n, inc + 2);
    }
    if (def) {
      add_string_to_array(&info->compile_defs, &info->compile_defs_n, def + 2);
    }
  }
}

/**
 * @brief Executes the scrape makefile operation.
 */
int scrape_makefile(struct ExtractedBuildInfo *info,
                    const char *makefile_content) {
  char *copy;
  char *saveptr;
  char *tok;

  if (!info || !makefile_content)
    return EINVAL;

  copy = my_strdup(makefile_content);
  if (!copy)
    return ENOMEM;

  tok = strtok_s(copy, " \t
\r\\", &saveptr);
  while (tok) {
    process_token(info, tok);
    tok = strtok_s(NULL, " \t
\r\\", &saveptr);
  }

  free(copy);
  return 0;
}

/**
 * @brief Executes the scrape configure ac operation.
 */
int scrape_configure_ac(
    struct ExtractedBuildInfo *info,
    const char
        *configure_ac_content) { /* Minimal implementation: scan similarly to
                                    Makefile for basic definitions */
  char *copy;
  char *saveptr;
  char *tok;

  if (!info || !configure_ac_content)
    return EINVAL;

  copy = my_strdup(configure_ac_content);
  if (!copy)
    return ENOMEM;

  tok = strtok_s(copy, " \t
\r\\", &saveptr);
  while (tok) {
    process_token(info, tok);
    tok = strtok_s(NULL, " \t
\r\\", &saveptr);
  }

  free(copy);
  return 0;
}

/**
 * @brief Executes the build info to cmake operation.
 */
int build_info_to_cmake(const struct ExtractedBuildInfo *info,
                        const char *project_name, char **out_cmake) {
  size_t cap = 4096;
  size_t len = 0;
  char *buf;
  size_t i;

  if (!info || !project_name || !out_cmake)
    return EINVAL;

  buf = (char *)malloc(cap);
  if (!buf)
    return ENOMEM;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(buf + len, cap - len, _TRUNCATE,
                     "cmake_minimum_required(VERSION 3.10)
                     "
                     "project(%s C)
\\n ",
                     project_name);
#else
  len += snprintf(buf + len, cap - len,
                  "cmake_minimum_required(VERSION 3.10)
                  "
                  "project(%s C)
\\n ",
                  project_name);
#endif

  if (info->compile_defs_n > 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(buf + len, cap - len, _TRUNCATE,
                       "add_compile_definitions(\\n");
#else
    len += snprintf(buf + len, cap - len, "add_compile_definitions(\\n");
#endif
    for (i = 0; i < info->compile_defs_n; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      len += _snprintf_s(buf + len, cap - len, _TRUNCATE, "  %s\\n",
                         info->compile_defs[i]);
#else
      len += snprintf(buf + len, cap - len, "  %s\\n", info->compile_defs[i]);
#endif
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(buf + len, cap - len, _TRUNCATE, ")
\\n");
#else
    len += snprintf(buf + len, cap - len, ")
\\n");
#endif
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(buf + len, cap - len, _TRUNCATE, "add_executable(%s\\n",
                     project_name);
#else
  len += snprintf(buf + len, cap - len, "add_executable(%s\\n", project_name);
#endif

  for (i = 0; i < info->source_files_n; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(buf + len, cap - len, _TRUNCATE, "  %s\\n",
                       info->source_files[i]);
#else
    len += snprintf(buf + len, cap - len, "  %s\\n", info->source_files[i]);
#endif
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  len += _snprintf_s(buf + len, cap - len, _TRUNCATE, ")
\\n");
#else
  len += snprintf(buf + len, cap - len, ")
\\n");
#endif

  if (info->include_dirs_n > 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len +=
        _snprintf_s(buf + len, cap - len, _TRUNCATE,
                    "target_include_directories(%s PRIVATE\\n", project_name);
#else
    len += snprintf(buf + len, cap - len,
                    "target_include_directories(%s PRIVATE\\n", project_name);
#endif
    for (i = 0; i < info->include_dirs_n; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      len += _snprintf_s(buf + len, cap - len, _TRUNCATE, "  %s\\n",
                         info->include_dirs[i]);
#else
      len += snprintf(buf + len, cap - len, "  %s\\n", info->include_dirs[i]);
#endif
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    len += _snprintf_s(buf + len, cap - len, _TRUNCATE, ")\\n");
#else
    len += snprintf(buf + len, cap - len, ")\\n");
#endif
  }

  *out_cmake = buf;
  return 0;
}
