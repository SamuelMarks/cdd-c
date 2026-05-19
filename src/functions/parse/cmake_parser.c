/**
 * @file cmake_parser.c
 * @brief Implementation of basic CMakeLists.txt structure manipulation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/cmake_parser.h"
#include "c_cdd/log.h"
/* clang-format on */

static int my_strdup(const char *s, char **out_val) {
  size_t len;
  char *d;
  if (!out_val)
    return EINVAL;
  *out_val = NULL;
  if (!s)
    return EINVAL;
  len = strlen(s) + 1;
  d = (char *)malloc(len);
  if (!d)
    return ENOMEM;
  memcpy(d, s, len);
  *out_val = d;
  return 0;
}

/**
 * @brief Executes the cmake modifier init operation.
 */
int cmake_modifier_init(struct CMakeModifier *mod, const char *filepath,
                        const char *target_name) {
  if (!mod || !filepath)
    return EINVAL;

  my_strdup(filepath, &mod->filepath);
  if (target_name)
    my_strdup(target_name, &mod->target_name);
  else
    mod->target_name = NULL;
  mod->compile_opts = NULL;
  mod->compile_opts_n = 0;
  mod->link_libs = NULL;
  mod->link_libs_n = 0;

  if (!mod->filepath) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  return 0;
}

/**
 * @brief Executes the cmake modifier add compile opt operation.
 */
int cmake_modifier_add_compile_opt(struct CMakeModifier *mod, const char *opt) {
  if (!mod || !opt)
    return EINVAL;

  mod->compile_opts = (char **)realloc(
      mod->compile_opts, (mod->compile_opts_n + 1) * sizeof(char *));
  if (!mod->compile_opts) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  my_strdup(opt, &mod->compile_opts[mod->compile_opts_n]);
  if (!mod->compile_opts[mod->compile_opts_n])
    return ENOMEM;

  mod->compile_opts_n++;
  return 0;
}

/**
 * @brief Executes the cmake modifier add link lib operation.
 */
int cmake_modifier_add_link_lib(struct CMakeModifier *mod, const char *lib) {
  if (!mod || !lib)
    return EINVAL;

  mod->link_libs =
      (char **)realloc(mod->link_libs, (mod->link_libs_n + 1) * sizeof(char *));
  if (!mod->link_libs) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  my_strdup(lib, &mod->link_libs[mod->link_libs_n]);
  if (!mod->link_libs[mod->link_libs_n])
    return ENOMEM;

  mod->link_libs_n++;
  return 0;
}

/**
 * @brief Executes the cmake modifier free operation.
 */
void cmake_modifier_free(struct CMakeModifier *mod) {
  size_t i;
  if (!mod)
    return;

  free(mod->filepath);
  if (mod->target_name)
    free(mod->target_name);

  if (mod->compile_opts) {
    for (i = 0; i < mod->compile_opts_n; i++) {
      free(mod->compile_opts[i]);
    }
    free(mod->compile_opts);
  }

  if (mod->link_libs) {
    for (i = 0; i < mod->link_libs_n; i++) {
      free(mod->link_libs[i]);
    }
    free(mod->link_libs);
  }
}

/**
 * @brief Executes the read file to string operation.
 */
static int read_file_to_string(const char *filename, size_t *out_len,
                               char **out_val) {
  FILE *f;
  char *buf = NULL;
  long size;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (fopen_s(&f, filename, "rb") != 0)
    return EINVAL;
#else
#if defined(_MSC_VER)
  fopen_s(&f, filename, "rb");
#else
#if defined(_MSC_VER)
  fopen_s(&f, filename, "rb");
#else
  f = fopen(filename, "rb");
#endif
#endif
  if (!f) {
    *out_val = NULL;
    return 0;
  }
#endif

  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (size < 0) {
    fclose(f);
    {
      *out_val = NULL;
      return 0;
    }
  }

  buf = (char *)malloc((size_t)size + 1);
  if (!buf) {
    fclose(f);
    {
      *out_val = NULL;
      return 0;
    }
  }

  if (fread(buf, 1, (size_t)size, f) != (size_t)size) {
    free(buf);
    fclose(f);
    {
      *out_val = NULL;
      return 0;
    }
  }
  buf[size] = '\0';
  if (out_len)
    *out_len = (size_t)size;
  fclose(f);
  {
    *out_val = buf;
    return 0;
  }
}

/* Simple append implementation ignoring sophisticated replacement for now */
/**
 * @brief Executes the cmake modifier apply diff operation.
 */
int cmake_modifier_apply_diff(const struct CMakeModifier *mod,
                              char **out_diff) {
  size_t len = 0;
  char *src = NULL;
  char *diff;
  size_t diff_cap = 2048;
  size_t diff_len = 0;
  size_t i;
  int lines_count = 0;
  char *str_buf;
  size_t str_buf_len = 0;
  read_file_to_string(mod->filepath, &len, &src);

  if (!src) {
    /* If file doesn't exist, we just simulate appending to empty */
    my_strdup("", &src);
    len = 0;
  }

  for (i = 0; i < len; i++) {
    if (src[i] == '\n')
      lines_count++;
  }
  if (len > 0 && src[len - 1] != '\n')
    lines_count++;
  if (lines_count == 0)
    lines_count = 1; /* For empty files */

  diff = (char *)malloc(diff_cap);
  if (!diff) {
    free(src);
    return ENOMEM;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  diff_len += _snprintf_s(diff + diff_len, diff_cap - diff_len, _TRUNCATE,
                          "--- %s\n+++ %s\n", mod->filepath, mod->filepath);
#else
  diff_len += snprintf(diff + diff_len, diff_cap - diff_len, "--- %s\n+++ %s\n",
                       mod->filepath, mod->filepath);
#endif

  str_buf = (char *)malloc(1024);
  if (!str_buf) {
    free(src);
    free(diff);
    return ENOMEM;
  }
  str_buf[0] = '\0';

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  str_buf_len += _snprintf_s(str_buf + str_buf_len, 1024 - str_buf_len,
                             _TRUNCATE, "if(MSVC)\n");
#else
  str_buf_len +=
      snprintf(str_buf + str_buf_len, 1024 - str_buf_len, "if(MSVC)\n");
#endif

  if (mod->compile_opts_n > 0) {
    if (mod->target_name) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      str_buf_len += _snprintf_s(
          str_buf + str_buf_len, 1024 - str_buf_len, _TRUNCATE,
          "    target_compile_options(%s PRIVATE", mod->target_name);
#else
      str_buf_len +=
          snprintf(str_buf + str_buf_len, 1024 - str_buf_len,
                   "    target_compile_options(%s PRIVATE", mod->target_name);
#endif
    } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      str_buf_len += _snprintf_s(str_buf + str_buf_len, 1024 - str_buf_len,
                                 _TRUNCATE, "    add_compile_options(");
#else
      str_buf_len += snprintf(str_buf + str_buf_len, 1024 - str_buf_len,
                              "    add_compile_options(");
#endif
    }
    for (i = 0; i < mod->compile_opts_n; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      str_buf_len += _snprintf_s(str_buf + str_buf_len, 1024 - str_buf_len,
                                 _TRUNCATE, " %s", mod->compile_opts[i]);
#else
      str_buf_len += snprintf(str_buf + str_buf_len, 1024 - str_buf_len, " %s",
                              mod->compile_opts[i]);
#endif
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    str_buf_len += _snprintf_s(str_buf + str_buf_len, 1024 - str_buf_len,
                               _TRUNCATE, ")\n");
#else
    str_buf_len += snprintf(str_buf + str_buf_len, 1024 - str_buf_len, ")\n");
#endif
  }

  if (mod->link_libs_n > 0) {
    if (mod->target_name) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      str_buf_len +=
          _snprintf_s(str_buf + str_buf_len, 1024 - str_buf_len, _TRUNCATE,
                      "    target_link_libraries(%s PRIVATE", mod->target_name);
#else
      str_buf_len +=
          snprintf(str_buf + str_buf_len, 1024 - str_buf_len,
                   "    target_link_libraries(%s PRIVATE", mod->target_name);
#endif
    } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      str_buf_len += _snprintf_s(str_buf + str_buf_len, 1024 - str_buf_len,
                                 _TRUNCATE, "    link_libraries(");
#else
      str_buf_len += snprintf(str_buf + str_buf_len, 1024 - str_buf_len,
                              "    link_libraries(");
#endif
    }
    for (i = 0; i < mod->link_libs_n; i++) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      str_buf_len += _snprintf_s(str_buf + str_buf_len, 1024 - str_buf_len,
                                 _TRUNCATE, " %s", mod->link_libs[i]);
#else
      str_buf_len += snprintf(str_buf + str_buf_len, 1024 - str_buf_len, " %s",
                              mod->link_libs[i]);
#endif
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    str_buf_len += _snprintf_s(str_buf + str_buf_len, 1024 - str_buf_len,
                               _TRUNCATE, ")\n");
#else
    str_buf_len += snprintf(str_buf + str_buf_len, 1024 - str_buf_len, ")\n");
#endif
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  str_buf_len += _snprintf_s(str_buf + str_buf_len, 1024 - str_buf_len,
                             _TRUNCATE, "endif()\n");
#else
  str_buf_len +=
      snprintf(str_buf + str_buf_len, 1024 - str_buf_len, "endif()\n");
#endif

  {
    /* Hunk appended at EOF */
    int new_lines = 0;
    for (i = 0; i < str_buf_len; i++) {
      if (str_buf[i] == '\n')
        new_lines++;
    }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    diff_len += _snprintf_s(diff + diff_len, diff_cap - diff_len, _TRUNCATE,
                            "@@ -%d,0 +%d,%d @@\n", lines_count, lines_count,
                            new_lines);
#else
    diff_len +=
        snprintf(diff + diff_len, diff_cap - diff_len, "@@ -%d,0 +%d,%d @@\n",
                 lines_count, lines_count, new_lines);
#endif

    /* Append lines */
    {
      char *line_start = str_buf;
      while (*line_start) {
        char *nl = strchr(line_start, '\n');
        if (nl) {
          int l_len = (int)(nl - line_start);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          diff_len += _snprintf_s(diff + diff_len, diff_cap - diff_len,
                                  _TRUNCATE, "+%.*s\n", l_len, line_start);
#else
          diff_len += snprintf(diff + diff_len, diff_cap - diff_len, "+%.*s\n",
                               l_len, line_start);
#endif
          line_start = nl + 1;
        } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          diff_len += _snprintf_s(diff + diff_len, diff_cap - diff_len,
                                  _TRUNCATE, "+%s\n", line_start);
#else
          diff_len += snprintf(diff + diff_len, diff_cap - diff_len, "+%s\n",
                               line_start);
#endif
          break;
        }
      }
    }
  }

  *out_diff = diff;
  free(str_buf);
  free(src);

  return 0;
}
