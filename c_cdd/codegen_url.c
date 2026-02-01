/**
 * @file codegen_url.c
 * @brief Implementation of URL interpolator generation.
 *
 * Updated to include `codegen_url_write_query_params` which handles the
 * complexity of Loop Generation for Array Parameters.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_url.h"
#include "str_utils.h"

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

struct UrlSegment {
  int is_var;
  char *text;
};

static const struct OpenAPI_Parameter *
find_param(const char *name, const struct OpenAPI_Parameter *params,
           size_t n_params) {
  size_t i;
  for (i = 0; i < n_params; ++i) {
    if (params[i].name && strcmp(params[i].name, name) == 0 &&
        params[i].in == OA_PARAM_IN_PATH) {
      return &params[i];
    }
  }
  return NULL;
}

static int parse_segments(const char *tmpl, struct UrlSegment **out_segments,
                          size_t *out_count) {
  const char *p = tmpl;
  const char *start = p;
  struct UrlSegment *segs = NULL;
  size_t count = 0;
  size_t cap = 0;

  while (*p) {
    if (*p == '{') {
      if (p > start) {
        size_t len = p - start;
        if (count >= cap) {
          cap = (cap == 0) ? 8 : cap * 2;
          segs = (struct UrlSegment *)realloc(segs,
                                              cap * sizeof(struct UrlSegment));
          if (!segs)
            return ENOMEM;
        }
        segs[count].is_var = 0;
        segs[count].text = malloc(len + 1);
        if (!segs[count].text)
          return ENOMEM;
        memcpy(segs[count].text, start, len);
        segs[count].text[len] = '\0';
        count++;
      }
      start = p + 1;
      {
        const char *close = strchr(start, '}');
        if (!close) {
          size_t i;
          for (i = 0; i < count; ++i)
            free(segs[i].text);
          free(segs);
          return EINVAL;
        }
        {
          size_t len = close - start;
          if (count >= cap) {
            cap = (cap == 0) ? 8 : cap * 2;
            segs = (struct UrlSegment *)realloc(
                segs, cap * sizeof(struct UrlSegment));
            if (!segs)
              return ENOMEM;
          }
          segs[count].is_var = 1;
          segs[count].text = malloc(len + 1);
          if (!segs[count].text)
            return ENOMEM;
          memcpy(segs[count].text, start, len);
          segs[count].text[len] = '\0';
          count++;
        }
        p = close + 1;
        start = p;
      }
    } else {
      p++;
    }
  }
  if (p > start) {
    size_t len = p - start;
    if (count >= cap) {
      cap = (cap == 0) ? 8 : cap * 2;
      segs =
          (struct UrlSegment *)realloc(segs, cap * sizeof(struct UrlSegment));
      if (!segs)
        return ENOMEM;
    }
    segs[count].is_var = 0;
    segs[count].text = malloc(len + 1);
    if (!segs[count].text)
      return ENOMEM;
    memcpy(segs[count].text, start, len);
    segs[count].text[len] = '\0';
    count++;
  }
  *out_segments = segs;
  *out_count = count;
  return 0;
}

int codegen_url_write_builder(FILE *const fp, const char *const path_template,
                              const struct OpenAPI_Parameter *params,
                              size_t n_params,
                              const struct CodegenUrlConfig *const config) {
  struct UrlSegment *segs = NULL;
  size_t n_segs = 0;
  size_t i;
  int rc = 0;
  const char *base_var = (config && config->base_variable)
                             ? config->base_variable
                             : "ctx->base_url";
  const char *out_var =
      (config && config->out_variable) ? config->out_variable : "url";

  if (!fp || !path_template)
    return EINVAL;

  if ((rc = parse_segments(path_template, &segs, &n_segs)) != 0) {
    return rc;
  }

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p =
          find_param(segs[i].text, params, n_params);
      if (p && strcmp(p->type, "string") == 0) {
        CHECK_IO(fprintf(fp, "  char *encoded_%s = url_encode(%s);\n",
                         segs[i].text, segs[i].text));
        CHECK_IO(
            fprintf(fp, "  if (!encoded_%s) return ENOMEM;\n", segs[i].text));
      }
    }
  }

  CHECK_IO(fprintf(fp, "  if (asprintf(&%s, \"%%s", out_var));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p =
          find_param(segs[i].text, params, n_params);
      if (p) {
        if (strcmp(p->type, "integer") == 0)
          CHECK_IO(fprintf(fp, "%%d"));
        else if (strcmp(p->type, "boolean") == 0)
          CHECK_IO(fprintf(fp, "%%s"));
        else
          CHECK_IO(fprintf(fp, "%%s"));
      } else {
        CHECK_IO(fprintf(fp, "%%s"));
      }
    } else {
      CHECK_IO(fprintf(fp, "%s", segs[i].text));
    }
  }

  CHECK_IO(fprintf(fp, "\", %s", base_var));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p =
          find_param(segs[i].text, params, n_params);
      if (p && strcmp(p->type, "string") == 0) {
        CHECK_IO(fprintf(fp, ", encoded_%s", segs[i].text));
      } else if (p && strcmp(p->type, "boolean") == 0) {
        CHECK_IO(fprintf(fp, ", (%s ? \"true\" : \"false\")", segs[i].text));
      } else {
        CHECK_IO(fprintf(fp, ", %s", segs[i].text));
      }
    }
  }
  CHECK_IO(fprintf(fp, ") == -1) {\n"));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p =
          find_param(segs[i].text, params, n_params);
      if (p && strcmp(p->type, "string") == 0) {
        CHECK_IO(fprintf(fp, "    free(encoded_%s);\n", segs[i].text));
      }
    }
  }
  CHECK_IO(fprintf(fp, "    return ENOMEM;\n  }\n"));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p =
          find_param(segs[i].text, params, n_params);
      if (p && strcmp(p->type, "string") == 0) {
        CHECK_IO(fprintf(fp, "  free(encoded_%s);\n", segs[i].text));
      }
    }
  }

  for (i = 0; i < n_segs; ++i)
    free(segs[i].text);
  free(segs);

  return 0;
}

int codegen_url_write_query_params(FILE *const fp,
                                   const struct OpenAPI_Operation *op) {
  size_t i;
  int has_query = 0;

  if (!fp || !op)
    return EINVAL;

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_QUERY) {
      const struct OpenAPI_Parameter *p = &op->parameters[i];

      if (!has_query) {
        CHECK_IO(fprintf(fp, "  rc = url_query_init(&qp);\n"));
        CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
        has_query = 1;
      }

      CHECK_IO(fprintf(fp, "  /* Query Parameter: %s */\n", p->name));

      if (p->is_array && p->explode) {
        /* === explode=true === */
        /* Emit loop over array */
        CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
        CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));

        /* Inner logic depending on item type */
        if (p->items_type && strcmp(p->items_type, "string") == 0) {
          CHECK_IO(fprintf(fp,
                           "      rc = url_query_add(&qp, \"%s\", %s[i]);\n",
                           p->name, p->name));
        } else if (p->items_type && strcmp(p->items_type, "integer") == 0) {
          CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
          CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", %s[i]);\n",
                           p->name));
          CHECK_IO(fprintf(fp,
                           "      rc = url_query_add(&qp, \"%s\", num_buf);\n",
                           p->name));
        } else if (p->items_type && strcmp(p->items_type, "boolean") == 0) {
          CHECK_IO(fprintf(fp,
                           "      rc = url_query_add(&qp, \"%s\", %s[i] ? "
                           "\"true\" : \"false\");\n",
                           p->name, p->name));
        }
        /* Fallback for safety */
        CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n    }\n  }\n"));

      } else if (p->is_array && !p->explode) {
        /* === explode=false (Clean comma separation not implemented yet) === */
        CHECK_IO(fprintf(
            fp, "  /* Nested array without explode not yet supported */\n"));
      } else {
        /* === Scalar === */
        if (strcmp(p->type, "string") == 0) {
          CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
          CHECK_IO(fprintf(fp, "    rc = url_query_add(&qp, \"%s\", %s);\n",
                           p->name, p->name));
          CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
        } else if (strcmp(p->type, "integer") == 0) {
          CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
          CHECK_IO(
              fprintf(fp, "    sprintf(num_buf, \"%%d\", %s);\n", p->name));
          CHECK_IO(fprintf(
              fp, "    rc = url_query_add(&qp, \"%s\", num_buf);\n", p->name));
          CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
        } else if (strcmp(p->type, "boolean") == 0) {
          CHECK_IO(fprintf(fp,
                           "  rc = url_query_add(&qp, \"%s\", %s ? \"true\" : "
                           "\"false\");\n",
                           p->name, p->name));
          CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
        }
      }
    }
  }

  if (has_query) {
    CHECK_IO(fprintf(fp, "  rc = url_query_build(&qp, &query_str);\n"));
    CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n\n"));
  }

  return 0;
}
