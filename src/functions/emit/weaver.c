/**
 * @file weaver.c
 * @brief Implementation of the High-level Weaver Engine API.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd_export.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/weaver.h"
/* clang-format on */

/**
 * @brief Executes the weaver wrap ifdef operation.
 */
enum cdd_c_error weaver_wrap_ifdef(struct PatchList *patches,
                                   const struct TokenList *tokens,
                                   size_t start_idx, size_t end_idx,
                                   const char *condition,
                                   const char *false_code) {
  char *ifdef_str;
  char *endif_str;
  int res;
  size_t ifdef_len;
  size_t endif_len;

  if (!patches || !tokens || !condition || start_idx > end_idx ||
      end_idx > tokens->size) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* Construct `#ifdef <condition>
` */
  ifdef_len = strlen("#ifdef ") + strlen(condition) + 20;

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
      ifdef_str = NULL;
    else
      ifdef_str = (char *)malloc(ifdef_len);
  }
#else
  ifdef_str = (char *)malloc(ifdef_len);
#endif

  if (!ifdef_str) {
    return CDD_C_ERROR_MEMORY;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(ifdef_str, ifdef_len, "#ifdef %s\\n", condition);
#else
  sprintf(ifdef_str, "#ifdef %s\\n", condition);
#endif

  res = patch_list_add(patches, start_idx, start_idx, ifdef_str);
  if (res != 0) {
    /* patch_list_add frees on failure */
    /* LCOV_EXCL_START */
    return res;
    /* LCOV_EXCL_STOP */
  }

  /* Construct `#else
<false_code>
#endif
` or just `#endif
` */
  if (false_code) {
    endif_len =
        strlen("#else\\n") + strlen(false_code) + strlen("\\n#endif\\n") + 20;

#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT int g_cdd_fail_alloc;
      if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
        /* LCOV_EXCL_START */
        endif_str = NULL;
      /* LCOV_EXCL_STOP */
      else
        endif_str = (char *)malloc(endif_len);
    }
#else
    endif_str = (char *)malloc(endif_len);
#endif

    if (!endif_str) {
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(endif_str, endif_len, "#else\n%s\n#endif \\n ", false_code);
#else
    sprintf(endif_str, "#else\n%s\n#endif \\n ", false_code);
#endif
  } else {
    endif_len = strlen("#endif\\n") + 20;

#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT int g_cdd_fail_alloc;
      if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
        /* LCOV_EXCL_START */
        endif_str = NULL;
      /* LCOV_EXCL_STOP */
      else
        endif_str = (char *)malloc(endif_len);
    }
#else
    endif_str = (char *)malloc(endif_len);
#endif

    if (!endif_str) {
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strcpy_s(endif_str, endif_len, "#endif\\n");
#else
    strcpy(endif_str, "#endif\\n");
#endif
  }

  res = patch_list_add(patches, end_idx, end_idx, endif_str);
  if (res != 0) {
    /* patch_list_add frees on failure */
    /* LCOV_EXCL_START */
    return res;
    /* LCOV_EXCL_STOP */
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the weaver inject msvc headers operation.
 */
enum cdd_c_error weaver_inject_msvc_headers(struct PatchList *patches,
                                            const struct TokenList *tokens,
                                            int include_windows_h,
                                            int include_winsock2_h) {
  size_t i;
  size_t last_include_idx = 0;
  int found_include = 0;
  size_t insert_idx = 0;
  size_t len;
  char *str;
  int res;

  if (!patches || !tokens) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (!include_windows_h && !include_winsock2_h) {
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_HASH) {
      size_t j = i + 1;
      while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE) {
        /* LCOV_EXCL_START */
        j++;
        /* LCOV_EXCL_STOP */
      }
      if (j < tokens->size && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        int is_include = 0;
        token_matches_string(&tokens->tokens[j], "include", &is_include);
        if (is_include) {
          last_include_idx = j;
          found_include = 1;
        }
      }
    }
  }

  if (found_include) {
    size_t j = last_include_idx + 1;
    while (j < tokens->size) {
      if (tokens->tokens[j].kind == TOKEN_WHITESPACE) {
        const uint8_t *s = tokens->tokens[j].start;
        size_t l = tokens->tokens[j].length;
        int has_nl = 0;
        size_t k;
        for (k = 0; k < l; ++k) {
          if (s[k] == '\n') {
            has_nl = 1;
            break;
          }
        }
        if (has_nl) {
          insert_idx = j + 1;
          break;
        }
      }
      j++;
    }
    if (j == tokens->size) {
      /* LCOV_EXCL_START */
      insert_idx = tokens->size;
      /* LCOV_EXCL_STOP */
    }
  }

  len = 256;

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
      str = NULL;
    else
      str = (char *)malloc(len);
  }
#else
  str = (char *)malloc(len);
#endif

  if (!str) {
    return CDD_C_ERROR_MEMORY;
  }
  str[0] = '\0';

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strcat_s(str, len, "#ifdef _MSC_VER\\n");
  if (include_winsock2_h) {
    strcat_s(str, len,
             "#ifndef WIN32_LEAN_AND_MEAN\n#define WIN32_LEAN_AND_MEAN\n#endif "
             "\\n ");
    strcat_s(str, len, "#include <winsock2.h>\\n");
  }
  if (include_windows_h) {
    if (!include_winsock2_h) {
      strcat_s(str, len,
               "#ifndef WIN32_LEAN_AND_MEAN\n#define "
               "WIN32_LEAN_AND_MEAN\n#endif \\n ");
    }
    strcat_s(str, len, "#ifndef NOMINMAX\n#define NOMINMAX\n#endif \\n ");
    strcat_s(str, len, "\\n");
  }
  strcat_s(str, len, "#endif\n\\n");
#else
  strcat(str, "#ifdef _MSC_VER\\n");
  if (include_winsock2_h) {
    strcat(str, "#ifndef WIN32_LEAN_AND_MEAN\n#define "
                "WIN32_LEAN_AND_MEAN\n#endif \\n ");
    strcat(str, "#include <winsock2.h>\\n");
  }
  if (include_windows_h) {
    if (!include_winsock2_h) {
      /* LCOV_EXCL_START */
      strcat(str, "#ifndef WIN32_LEAN_AND_MEAN\n#define "
                  /* LCOV_EXCL_STOP */
                  "WIN32_LEAN_AND_MEAN\n#endif \\n ");
    }
    strcat(str, "#ifndef NOMINMAX\n#define NOMINMAX\n#endif \\n ");
    strcat(str, "\\n");
  }
  strcat(str, "#endif\n\\n");
#endif

  res = patch_list_add(patches, insert_idx, insert_idx, str);
  return res;
}

/**
 * @brief Executes the weaver vla to alloca operation.
 */
enum cdd_c_error weaver_vla_to_alloca(struct PatchList *patches,
                                      const struct TokenList *tokens,
                                      size_t start_idx, size_t end_idx,
                                      const char *type_str,
                                      const char *var_name,
                                      const char *size_expr, int interactive) {
  char *str;
  size_t len;
  int res;

  if (!patches || !tokens || !type_str || !var_name || !size_expr ||
      start_idx >= end_idx || end_idx > tokens->size) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (interactive) {
    char user_input[16];
    /* LCOV_EXCL_START */
    printf("\nInteractive Review:\\n");
    printf("Detected VLA: `%s %s[%s];`\\n", type_str, var_name, size_expr);
    printf("Transform to: `%s *%s = (%s *)_alloca((%s) * sizeof(%s));`\\n",
           /* LCOV_EXCL_STOP */
           type_str, var_name, type_str, size_expr, type_str);
    /* LCOV_EXCL_START */
    printf("Apply this transformation? [Y/n] ");
    fflush(stdout);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    if (fgets(user_input, sizeof(user_input), stdin)) {
      if (user_input[0] == 'n' || user_input[0] == 'N') {
        printf("Skipped.\\n");
        return CDD_C_SUCCESS;
        /* LCOV_EXCL_STOP */
      }
    }
  }

  /* Calculate length:
   * type_str + " *" + var_name + " = (" + type_str + " *)_alloca((" +
   * size_expr + ") * sizeof(" + type_str + "));" + null terminator
   */
  len = strlen(type_str) + 2 + strlen(var_name) + 4 + strlen(type_str) + 13 +
        strlen(size_expr) + 11 + strlen(type_str) + 4 + 100;

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
      str = NULL;
    else
      str = (char *)malloc(len);
  }
#else
  str = (char *)malloc(len);
#endif

  if (!str) {
    return CDD_C_ERROR_MEMORY;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(str, len, "%s *%s = (%s *)_alloca((%s) * sizeof(%s));", type_str,
            var_name, type_str, size_expr, type_str);
#else
  sprintf(str, "%s *%s = (%s *)_alloca((%s) * sizeof(%s));", type_str, var_name,
          type_str, size_expr, type_str);
#endif

  res = patch_list_add(patches, start_idx, end_idx, str);
  return res;
}
