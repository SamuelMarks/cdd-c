/**
 * @file doc_arazzo.c
 * @brief Arazzo 1.1.0 documentation comment directive parser (`@arazzo`,
 * `@workflow`, `@step`, etc.).
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"

#include "c_cdd_export.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "docstrings/parse/doc.h"
#include "docstrings/parse/doc_internal.h"
#include "functions/parse/str.h"
/* clang-format on */

/**
 * @brief Frees a `DocArazzoParam` array and its elements.
 *
 * @param[in,out] params Array of parameters to free.
 * @param[in] n_params Number of parameters in the array.
 */
static void free_doc_arazzo_params(struct DocArazzoParam *params,
                                   size_t n_params) {
  size_t i;
  for (i = 0; i < n_params; ++i) {
    C_CDD_FREE(params[i].name);
    C_CDD_FREE(params[i].in_loc);
    C_CDD_FREE(params[i].value);
    C_CDD_FREE(params[i].reference);
  }
  C_CDD_FREE(params);
}

/**
 * @brief Frees a `DocArazzoKeyVal` array and its elements.
 *
 * @param[in,out] kvs Array of key-value pairs to free.
 * @param[in] n_kvs Number of key-value pairs in the array.
 */
static void free_doc_arazzo_kvs(struct DocArazzoKeyVal *kvs, size_t n_kvs) {
  size_t i;
  for (i = 0; i < n_kvs; ++i) {
    C_CDD_FREE(kvs[i].key);
    C_CDD_FREE(kvs[i].value);
  }
  C_CDD_FREE(kvs);
}

/**
 * @brief Frees a `DocArazzoAction` array and its elements.
 *
 * @param[in,out] actions Array of actions to free.
 * @param[in] n_actions Number of actions in the array.
 */
static void free_doc_arazzo_actions(struct DocArazzoAction *actions,
                                    size_t n_actions) {
  size_t i;
  for (i = 0; i < n_actions; ++i) {
    C_CDD_FREE(actions[i].name);
    C_CDD_FREE(actions[i].type);
    C_CDD_FREE(actions[i].workflow_id);
    C_CDD_FREE(actions[i].step_id);
    C_CDD_FREE(actions[i].reference);
  }
  C_CDD_FREE(actions);
}

/**
 * @brief Frees all Arazzo-specific fields inside a `DocMetadata` structure.
 *
 * @param[in,out] meta Pointer to the DocMetadata structure whose Arazzo fields
 * should be freed.
 */
void doc_free_arazzo_metadata(struct DocMetadata *meta) {
  size_t i, j;
  if (!meta) {
    return;
  }
  C_CDD_FREE(meta->arazzo_version);
  C_CDD_FREE(meta->self_uri);
  for (i = 0; i < meta->n_arazzo_sources; ++i) {
    C_CDD_FREE(meta->arazzo_sources[i].name);
    C_CDD_FREE(meta->arazzo_sources[i].url);
    C_CDD_FREE(meta->arazzo_sources[i].type);
  }
  C_CDD_FREE(meta->arazzo_sources);
  C_CDD_FREE(meta->workflow_id);
  C_CDD_FREE(meta->workflow_summary);
  for (i = 0; i < meta->n_workflow_depends_on; ++i) {
    C_CDD_FREE(meta->workflow_depends_on[i]);
  }
  C_CDD_FREE(meta->workflow_depends_on);
  C_CDD_FREE(meta->workflow_inputs_json);
  free_doc_arazzo_kvs(meta->workflow_outputs, meta->n_workflow_outputs);
  free_doc_arazzo_params(meta->workflow_params, meta->n_workflow_params);

  for (i = 0; i < meta->n_arazzo_steps; ++i) {
    struct DocArazzoStep *st = &meta->arazzo_steps[i];
    C_CDD_FREE(st->step_id);
    C_CDD_FREE(st->description);
    C_CDD_FREE(st->operation_id);
    C_CDD_FREE(st->operation_path);
    C_CDD_FREE(st->channel_path);
    C_CDD_FREE(st->workflow_id);
    C_CDD_FREE(st->correlation_id);
    C_CDD_FREE(st->action);
    for (j = 0; j < st->n_depends_on; ++j) {
      C_CDD_FREE(st->depends_on[j]);
    }
    C_CDD_FREE(st->depends_on);
    free_doc_arazzo_params(st->params, st->n_params);
    C_CDD_FREE(st->request_body_content_type);
    C_CDD_FREE(st->request_body_payload);
    for (j = 0; j < st->n_replacements; ++j) {
      C_CDD_FREE(st->replacements[j].target);
      C_CDD_FREE(st->replacements[j].value);
    }
    C_CDD_FREE(st->replacements);
    for (j = 0; j < st->n_criteria; ++j) {
      C_CDD_FREE(st->criteria[j].context);
      C_CDD_FREE(st->criteria[j].condition);
      C_CDD_FREE(st->criteria[j].type);
      C_CDD_FREE(st->criteria[j].version);
    }
    C_CDD_FREE(st->criteria);
    free_doc_arazzo_actions(st->on_success, st->n_on_success);
    free_doc_arazzo_actions(st->on_failure, st->n_on_failure);
    free_doc_arazzo_kvs(st->outputs, st->n_outputs);
  }
  C_CDD_FREE(meta->arazzo_steps);

  for (i = 0; i < meta->n_arazzo_components; ++i) {
    C_CDD_FREE(meta->arazzo_components[i].kind);
    C_CDD_FREE(meta->arazzo_components[i].name);
    C_CDD_FREE(meta->arazzo_components[i].json);
  }
  C_CDD_FREE(meta->arazzo_components);

  free_doc_arazzo_kvs(meta->arazzo_extensions, meta->n_arazzo_extensions);
}

/**
 * @brief Appends a comma-delimited list of strings into a `char **` array.
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] list Target string array pointer.
 * @param[in,out] count Target string array count pointer.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_comma_list(const char *line, const char *end,
                                      char ***list, size_t *count) {
  char *rest = NULL;
  char *cursor;
  cdd_c_error_t rc;

  rc = doc_extract_rest(line, end, &rest);
  if (rc != CDD_C_SUCCESS || !rest) {
    return rc;
  }

  cursor = rest;
  while (*cursor) {
    char *comma = strchr(cursor, ',');
    char *item = cursor;
    char *item_end;
    if (comma)
      *comma = '\0';

    while (*item && isspace((unsigned char)*item))
      item++;
    item_end = item + strlen(item);
    while (item_end > item && isspace((unsigned char)*(item_end - 1)))
      item_end--;
    *item_end = '\0';

    if (*item) {
      char **next_arr =
          (char **)C_CDD_REALLOC(*list, (*count + 1) * sizeof(char *));
      if (!next_arr) {
        C_CDD_FREE(rest);
        return CDD_C_ERROR_MEMORY;
      }
      *list = next_arr;
      rc = c_cdd_strdup(item, &(*list)[*count]);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(rest);
        return rc;
      }
      (*count)++;
    }
    if (comma)
      cursor = comma + 1;
    else
      break;
  }

  C_CDD_FREE(rest);
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends a key-value pair to a `DocArazzoKeyVal` array.
 *
 * @param[in] line Start of directive text (`<key> <value>`).
 * @param[in] end End of directive text.
 * @param[in,out] list Target key-value array pointer.
 * @param[in,out] count Target key-value count pointer.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM.
 */
static cdd_c_error_t parse_key_val_line(const char *line, const char *end,
                                        struct DocArazzoKeyVal **list,
                                        size_t *count) {
  const char *cur = line;
  char *key = NULL;
  char *val = NULL;
  struct DocArazzoKeyVal *next_arr;
  cdd_c_error_t rc;

  rc = doc_extract_word(cur, end, &cur, &key);
  if (rc != CDD_C_SUCCESS || !key) {
    return rc;
  }
  rc = doc_extract_rest(cur, end, &val);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(key);
    return rc;
  }
  next_arr = (struct DocArazzoKeyVal *)C_CDD_REALLOC(
      *list, (*count + 1) * sizeof(struct DocArazzoKeyVal));
  if (!next_arr) {
    C_CDD_FREE(key);
    C_CDD_FREE(val);
    return CDD_C_ERROR_MEMORY;
  }
  *list = next_arr;
  (*list)[*count].key = key;
  (*list)[*count].value = val;
  (*count)++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a `@workflowParam` or `@stepParam` directive line.
 *
 * Syntax: `<name> [in:<loc>] [reference:<ref>] <value>`
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] list Target parameter array pointer.
 * @param[in,out] count Target parameter count pointer.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_arazzo_param_line(const char *line, const char *end,
                                             struct DocArazzoParam **list,
                                             size_t *count) {
  const char *cur = line;
  struct DocArazzoParam *next_arr;
  struct DocArazzoParam *p;
  cdd_c_error_t rc;

  next_arr = (struct DocArazzoParam *)C_CDD_REALLOC(
      *list, (*count + 1) * sizeof(struct DocArazzoParam));
  if (!next_arr) {
    return CDD_C_ERROR_MEMORY;
  }
  *list = next_arr;
  p = &(*list)[*count];
  memset(p, 0, sizeof(*p));

  rc = doc_extract_word(cur, end, &cur, &p->name);
  if (rc != CDD_C_SUCCESS || !p->name) {
    return rc;
  }
  (*count)++;

  while (*cur == ' ')
    cur++;

  while (*cur == '[') {
    const char *close_b = cur;
    while (close_b < end && *close_b != ']')
      close_b++;
    if (close_b < end) {
      size_t len = (size_t)(close_b - (cur + 1));
      char *attr = (char *)C_CDD_MALLOC(len + 1);
      if (!attr) {
        return CDD_C_ERROR_MEMORY;
      }
      memcpy(attr, cur + 1, len);
      attr[len] = '\0';
      if (strncmp(attr, "in:", 3) == 0) {
        C_CDD_FREE(p->in_loc);
        rc = c_cdd_strdup(attr + 3, &p->in_loc);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(attr);
          return rc;
        }
      } else if (strncmp(attr, "reference:", 10) == 0) {
        C_CDD_FREE(p->reference);
        rc = c_cdd_strdup(attr + 10, &p->reference);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(attr);
          return rc;
        }
      }
      C_CDD_FREE(attr);
      cur = close_b + 1;
      while (*cur == ' ')
        cur++;
    } else {
      break;
    }
  }

  return doc_extract_rest(cur, end, &p->value);
}

/**
 * @brief Ensures at least one step exists in `out->arazzo_steps` and returns
 * a pointer to the current (last) step.
 *
 * @param[in,out] out Metadata structure.
 * @param[out] out_step Output pointer to current step.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t ensure_current_step(struct DocMetadata *out,
                                         struct DocArazzoStep **out_step) {
  if (out->n_arazzo_steps == 0) {
    struct DocArazzoStep *next_arr =
        (struct DocArazzoStep *)C_CDD_CALLOC(1, sizeof(struct DocArazzoStep));
    if (!next_arr) {
      return CDD_C_ERROR_MEMORY;
    }
    out->arazzo_steps = next_arr;
    if (c_cdd_strdup("step_1", &out->arazzo_steps[0].step_id) !=
        CDD_C_SUCCESS) {
      return CDD_C_ERROR_MEMORY;
    }
    out->n_arazzo_steps = 1;
  }
  *out_step = &out->arazzo_steps[out->n_arazzo_steps - 1];
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses `@sourceDescription <name> <url> [type:<type>]`.
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] out Metadata structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_source_description_line(const char *line,
                                                   const char *end,
                                                   struct DocMetadata *out) {
  const char *cur = line;
  struct DocArazzoSource *next_arr;
  struct DocArazzoSource *src;
  cdd_c_error_t rc;

  next_arr = (struct DocArazzoSource *)C_CDD_REALLOC(
      out->arazzo_sources,
      (out->n_arazzo_sources + 1) * sizeof(struct DocArazzoSource));
  if (!next_arr) {
    return CDD_C_ERROR_MEMORY;
  }
  out->arazzo_sources = next_arr;
  src = &out->arazzo_sources[out->n_arazzo_sources];
  memset(src, 0, sizeof(*src));

  rc = doc_extract_word(cur, end, &cur, &src->name);
  if (rc != CDD_C_SUCCESS || !src->name) {
    return rc;
  }
  out->n_arazzo_sources++;

  rc = doc_extract_word(cur, end, &cur, &src->url);
  if (rc != CDD_C_SUCCESS || !src->url) {
    return rc;
  }

  while (*cur == ' ')
    cur++;
  while (*cur == '[') {
    const char *close_b = cur;
    while (close_b < end && *close_b != ']')
      close_b++;
    if (close_b < end) {
      size_t len = (size_t)(close_b - (cur + 1));
      if (strncmp(cur + 1, "type:", 5) == 0) {
        C_CDD_FREE(src->type);
        src->type = (char *)C_CDD_MALLOC(len - 5 + 1);
        if (!src->type) {
          return CDD_C_ERROR_MEMORY;
        }
        memcpy(src->type, cur + 6, len - 5);
        src->type[len - 5] = '\0';
      }
      cur = close_b + 1;
      while (*cur == ' ')
        cur++;
    } else {
      break;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses `@workflow <workflowId> [summary:<text>] <description>`.
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] out Metadata structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_workflow_line(const char *line, const char *end,
                                         struct DocMetadata *out) {
  const char *cur = line;
  char *wf_id = NULL;
  cdd_c_error_t rc;

  rc = doc_extract_word(cur, end, &cur, &wf_id);
  if (rc != CDD_C_SUCCESS || !wf_id) {
    return rc;
  }
  C_CDD_FREE(out->workflow_id);
  out->workflow_id = wf_id;

  while (*cur == ' ')
    cur++;
  while (*cur == '[') {
    const char *close_b = cur;
    while (close_b < end && *close_b != ']')
      close_b++;
    if (close_b < end) {
      size_t len = (size_t)(close_b - (cur + 1));
      if (strncmp(cur + 1, "summary:", 8) == 0) {
        C_CDD_FREE(out->workflow_summary);
        out->workflow_summary = (char *)C_CDD_MALLOC(len - 8 + 1);
        if (!out->workflow_summary) {
          return CDD_C_ERROR_MEMORY;
        }
        memcpy(out->workflow_summary, cur + 9, len - 8);
        out->workflow_summary[len - 8] = '\0';
      }
      cur = close_b + 1;
      while (*cur == ' ')
        cur++;
    } else {
      break;
    }
  }

  if (cur < end) {
    char *desc = NULL;
    rc = doc_extract_rest(cur, end, &desc);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    C_CDD_FREE(out->description);
    out->description = desc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses `@step <stepId> [operationId:...] [operationPath:...]
 * [channelPath:...] [workflowId:...] [timeout:...] [correlationId:...]
 * [action:...] <description>`.
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] out Metadata structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_step_line(const char *line, const char *end,
                                     struct DocMetadata *out) {
  const char *cur = line;
  struct DocArazzoStep *next_arr;
  struct DocArazzoStep *st;
  cdd_c_error_t rc;

  next_arr = (struct DocArazzoStep *)C_CDD_REALLOC(
      out->arazzo_steps,
      (out->n_arazzo_steps + 1) * sizeof(struct DocArazzoStep));
  if (!next_arr) {
    return CDD_C_ERROR_MEMORY;
  }
  out->arazzo_steps = next_arr;
  st = &out->arazzo_steps[out->n_arazzo_steps];
  memset(st, 0, sizeof(*st));

  rc = doc_extract_word(cur, end, &cur, &st->step_id);
  if (rc != CDD_C_SUCCESS || !st->step_id) {
    return rc;
  }
  out->n_arazzo_steps++;

  while (*cur == ' ')
    cur++;
  while (*cur == '[') {
    const char *close_b = cur;
    while (close_b < end && *close_b != ']')
      close_b++;
    if (close_b < end) {
      size_t len = (size_t)(close_b - (cur + 1));
      char *attr = (char *)C_CDD_MALLOC(len + 1);
      if (!attr) {
        return CDD_C_ERROR_MEMORY;
      }
      memcpy(attr, cur + 1, len);
      attr[len] = '\0';
      if (strncmp(attr, "operationId:", 12) == 0) {
        C_CDD_FREE(st->operation_id);
        rc = c_cdd_strdup(attr + 12, &st->operation_id);
      } else if (strncmp(attr, "operationPath:", 14) == 0) {
        C_CDD_FREE(st->operation_path);
        rc = c_cdd_strdup(attr + 14, &st->operation_path);
      } else if (strncmp(attr, "channelPath:", 12) == 0) {
        C_CDD_FREE(st->channel_path);
        rc = c_cdd_strdup(attr + 12, &st->channel_path);
      } else if (strncmp(attr, "workflowId:", 11) == 0) {
        C_CDD_FREE(st->workflow_id);
        rc = c_cdd_strdup(attr + 11, &st->workflow_id);
      } else if (strncmp(attr, "timeout:", 8) == 0) {
        st->timeout_ms = atoi(attr + 8);
        st->has_timeout = 1;
      } else if (strncmp(attr, "correlationId:", 14) == 0) {
        C_CDD_FREE(st->correlation_id);
        rc = c_cdd_strdup(attr + 14, &st->correlation_id);
      } else if (strncmp(attr, "action:", 7) == 0) {
        C_CDD_FREE(st->action);
        rc = c_cdd_strdup(attr + 7, &st->action);
      }
      C_CDD_FREE(attr);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      cur = close_b + 1;
      while (*cur == ' ')
        cur++;
    } else {
      break;
    }
  }

  return doc_extract_rest(cur, end, &st->description);
}

/**
 * @brief Parses `@stepRequestBody [contentType:<type>] <payload>`.
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] out Metadata structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_step_request_body_line(const char *line,
                                                  const char *end,
                                                  struct DocMetadata *out) {
  const char *cur = line;
  struct DocArazzoStep *st = NULL;
  cdd_c_error_t rc;

  rc = ensure_current_step(out, &st);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  while (*cur == ' ')
    cur++;
  while (*cur == '[') {
    const char *close_b = cur;
    while (close_b < end && *close_b != ']')
      close_b++;
    if (close_b < end) {
      size_t len = (size_t)(close_b - (cur + 1));
      if (strncmp(cur + 1, "contentType:", 12) == 0) {
        C_CDD_FREE(st->request_body_content_type);
        st->request_body_content_type = (char *)C_CDD_MALLOC(len - 12 + 1);
        if (!st->request_body_content_type) {
          return CDD_C_ERROR_MEMORY;
        }
        memcpy(st->request_body_content_type, cur + 13, len - 12);
        st->request_body_content_type[len - 12] = '\0';
      }
      cur = close_b + 1;
      while (*cur == ' ')
        cur++;
    } else {
      break;
    }
  }
  C_CDD_FREE(st->request_body_payload);
  st->request_body_payload = NULL;
  return doc_extract_rest(cur, end, &st->request_body_payload);
}

/**
 * @brief Parses `@stepReplacement <target> <value>`.
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] out Metadata structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_step_replacement_line(const char *line,
                                                 const char *end,
                                                 struct DocMetadata *out) {
  const char *cur = line;
  struct DocArazzoStep *st = NULL;
  struct DocArazzoReplacement *next_arr;
  char *target = NULL;
  char *val = NULL;
  cdd_c_error_t rc;

  rc = ensure_current_step(out, &st);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  rc = doc_extract_word(cur, end, &cur, &target);
  if (rc != CDD_C_SUCCESS || !target) {
    return rc;
  }
  rc = doc_extract_rest(cur, end, &val);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(target);
    return rc;
  }
  next_arr = (struct DocArazzoReplacement *)C_CDD_REALLOC(
      st->replacements,
      (st->n_replacements + 1) * sizeof(struct DocArazzoReplacement));
  if (!next_arr) {
    C_CDD_FREE(target);
    C_CDD_FREE(val);
    return CDD_C_ERROR_MEMORY;
  }
  st->replacements = next_arr;
  st->replacements[st->n_replacements].target = target;
  st->replacements[st->n_replacements].value = val;
  st->n_replacements++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses `@stepCriterion [context:<ctx>] [type:<type>]
 * [version:<ver>] <condition>`.
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] out Metadata structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_step_criterion_line(const char *line,
                                               const char *end,
                                               struct DocMetadata *out) {
  const char *cur = line;
  struct DocArazzoStep *st = NULL;
  struct DocArazzoCriterion *next_arr;
  struct DocArazzoCriterion *crit;
  cdd_c_error_t rc;

  rc = ensure_current_step(out, &st);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  next_arr = (struct DocArazzoCriterion *)C_CDD_REALLOC(
      st->criteria, (st->n_criteria + 1) * sizeof(struct DocArazzoCriterion));
  if (!next_arr) {
    return CDD_C_ERROR_MEMORY;
  }
  st->criteria = next_arr;
  crit = &st->criteria[st->n_criteria];
  memset(crit, 0, sizeof(*crit));
  st->n_criteria++;

  while (*cur == ' ')
    cur++;
  while (*cur == '[') {
    const char *close_b = cur;
    while (close_b < end && *close_b != ']')
      close_b++;
    if (close_b < end) {
      size_t len = (size_t)(close_b - (cur + 1));
      char *attr = (char *)C_CDD_MALLOC(len + 1);
      if (!attr) {
        return CDD_C_ERROR_MEMORY;
      }
      memcpy(attr, cur + 1, len);
      attr[len] = '\0';
      if (strncmp(attr, "context:", 8) == 0) {
        C_CDD_FREE(crit->context);
        rc = c_cdd_strdup(attr + 8, &crit->context);
      } else if (strncmp(attr, "type:", 5) == 0) {
        C_CDD_FREE(crit->type);
        rc = c_cdd_strdup(attr + 5, &crit->type);
      } else if (strncmp(attr, "version:", 8) == 0) {
        C_CDD_FREE(crit->version);
        rc = c_cdd_strdup(attr + 8, &crit->version);
      }
      C_CDD_FREE(attr);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      cur = close_b + 1;
      while (*cur == ' ')
        cur++;
    } else {
      break;
    }
  }
  return doc_extract_rest(cur, end, &crit->condition);
}

/**
 * @brief Parses `@stepOnSuccess` or `@stepOnFailure` directive line.
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] out Metadata structure.
 * @param[in] is_failure Non-zero for `onFailure`, 0 for `onSuccess`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_step_action_line(const char *line, const char *end,
                                            struct DocMetadata *out,
                                            int is_failure) {
  const char *cur = line;
  struct DocArazzoStep *st = NULL;
  struct DocArazzoAction **list_ptr;
  size_t *count_ptr;
  struct DocArazzoAction *next_arr;
  struct DocArazzoAction *act;
  cdd_c_error_t rc;

  rc = ensure_current_step(out, &st);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  list_ptr = is_failure ? &st->on_failure : &st->on_success;
  count_ptr = is_failure ? &st->n_on_failure : &st->n_on_success;

  next_arr = (struct DocArazzoAction *)C_CDD_REALLOC(
      *list_ptr, (*count_ptr + 1) * sizeof(struct DocArazzoAction));
  if (!next_arr) {
    return CDD_C_ERROR_MEMORY;
  }
  *list_ptr = next_arr;
  act = &(*list_ptr)[*count_ptr];
  memset(act, 0, sizeof(*act));

  rc = doc_extract_word(cur, end, &cur, &act->name);
  if (rc != CDD_C_SUCCESS || !act->name) {
    return rc;
  }
  (*count_ptr)++;

  while (*cur == ' ')
    cur++;
  while (*cur == '[') {
    const char *close_b = cur;
    while (close_b < end && *close_b != ']')
      close_b++;
    if (close_b < end) {
      size_t len = (size_t)(close_b - (cur + 1));
      char *attr = (char *)C_CDD_MALLOC(len + 1);
      if (!attr) {
        return CDD_C_ERROR_MEMORY;
      }
      memcpy(attr, cur + 1, len);
      attr[len] = '\0';
      if (strncmp(attr, "type:", 5) == 0) {
        C_CDD_FREE(act->type);
        rc = c_cdd_strdup(attr + 5, &act->type);
      } else if (strncmp(attr, "stepId:", 7) == 0) {
        C_CDD_FREE(act->step_id);
        rc = c_cdd_strdup(attr + 7, &act->step_id);
      } else if (strncmp(attr, "workflowId:", 11) == 0) {
        C_CDD_FREE(act->workflow_id);
        rc = c_cdd_strdup(attr + 11, &act->workflow_id);
      } else if (strncmp(attr, "reference:", 10) == 0) {
        C_CDD_FREE(act->reference);
        rc = c_cdd_strdup(attr + 10, &act->reference);
      } else if (strncmp(attr, "retryAfter:", 11) == 0) {
        act->retry_after = atof(attr + 11);
        act->has_retry_after = 1;
      } else if (strncmp(attr, "retryLimit:", 11) == 0) {
        act->retry_limit = atoi(attr + 11);
        act->has_retry_limit = 1;
      }
      C_CDD_FREE(attr);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      cur = close_b + 1;
      while (*cur == ' ')
        cur++;
    } else {
      break;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses `@arazzoComponent <kind> <name> <json>`.
 *
 * @param[in] line Start of directive text.
 * @param[in] end End of directive text.
 * @param[in,out] out Metadata structure.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_arazzo_component_line(const char *line,
                                                 const char *end,
                                                 struct DocMetadata *out) {
  const char *cur = line;
  char *kind = NULL;
  char *name = NULL;
  char *json = NULL;
  struct DocArazzoComponent *next_arr;
  cdd_c_error_t rc;

  rc = doc_extract_word(cur, end, &cur, &kind);
  if (rc != CDD_C_SUCCESS || !kind) {
    return rc;
  }
  rc = doc_extract_word(cur, end, &cur, &name);
  if (rc != CDD_C_SUCCESS || !name) {
    C_CDD_FREE(kind);
    return rc;
  }
  rc = doc_extract_rest(cur, end, &json);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(kind);
    C_CDD_FREE(name);
    return rc;
  }
  next_arr = (struct DocArazzoComponent *)C_CDD_REALLOC(
      out->arazzo_components,
      (out->n_arazzo_components + 1) * sizeof(struct DocArazzoComponent));
  if (!next_arr) {
    C_CDD_FREE(kind);
    C_CDD_FREE(name);
    C_CDD_FREE(json);
    return CDD_C_ERROR_MEMORY;
  }
  out->arazzo_components = next_arr;
  out->arazzo_components[out->n_arazzo_components].kind = kind;
  out->arazzo_components[out->n_arazzo_components].name = name;
  out->arazzo_components[out->n_arazzo_components].json = json;
  out->n_arazzo_components++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Dispatches an `@arazzo*`, `@workflow*`, `@step*`, or `@x-*` directive
 * line.
 *
 * @param[in] cmd Directive command name without the leading `@` or `\`.
 * @param[in] line Start of the directive argument text on the current line.
 * @param[in] end End of the current line.
 * @param[in,out] out Target DocMetadata structure to populate.
 * @param[out] out_handled Output pointer receiving 1 if recognized, 0
 * otherwise.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t doc_parse_arazzo_directive(const char *cmd, const char *line,
                                         const char *end,
                                         struct DocMetadata *out,
                                         int *out_handled) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  struct DocArazzoStep *st = NULL;

  if (!cmd || !line || !end || !out || !out_handled) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  *out_handled = 1;

  if (strcmp(cmd, "arazzo") == 0) {
    C_CDD_FREE(out->arazzo_version);
    out->arazzo_version = NULL;
    return doc_extract_rest(line, end, &out->arazzo_version);
  }
  if (strcmp(cmd, "self") == 0) {
    C_CDD_FREE(out->self_uri);
    out->self_uri = NULL;
    return doc_extract_rest(line, end, &out->self_uri);
  }
  if (strcmp(cmd, "sourceDescription") == 0 ||
      strcmp(cmd, "sourcedescription") == 0) {
    return parse_source_description_line(line, end, out);
  }
  if (strcmp(cmd, "workflow") == 0) {
    return parse_workflow_line(line, end, out);
  }
  if (strcmp(cmd, "workflowSummary") == 0 ||
      strcmp(cmd, "workflowsummary") == 0) {
    C_CDD_FREE(out->workflow_summary);
    out->workflow_summary = NULL;
    return doc_extract_rest(line, end, &out->workflow_summary);
  }
  if (strcmp(cmd, "workflowDependsOn") == 0 ||
      strcmp(cmd, "workflowdependson") == 0) {
    return parse_comma_list(line, end, &out->workflow_depends_on,
                            &out->n_workflow_depends_on);
  }
  if (strcmp(cmd, "workflowInput") == 0 || strcmp(cmd, "workflowinput") == 0) {
    C_CDD_FREE(out->workflow_inputs_json);
    out->workflow_inputs_json = NULL;
    return doc_extract_rest(line, end, &out->workflow_inputs_json);
  }
  if (strcmp(cmd, "workflowOutput") == 0 ||
      strcmp(cmd, "workflowoutput") == 0) {
    return parse_key_val_line(line, end, &out->workflow_outputs,
                              &out->n_workflow_outputs);
  }
  if (strcmp(cmd, "workflowParam") == 0 || strcmp(cmd, "workflowparam") == 0) {
    return parse_arazzo_param_line(line, end, &out->workflow_params,
                                   &out->n_workflow_params);
  }
  if (strcmp(cmd, "step") == 0) {
    return parse_step_line(line, end, out);
  }
  if (strcmp(cmd, "stepDependsOn") == 0 || strcmp(cmd, "stepdependson") == 0) {
    rc = ensure_current_step(out, &st);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return parse_comma_list(line, end, &st->depends_on, &st->n_depends_on);
  }
  if (strcmp(cmd, "stepParam") == 0 || strcmp(cmd, "stepparam") == 0) {
    rc = ensure_current_step(out, &st);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return parse_arazzo_param_line(line, end, &st->params, &st->n_params);
  }
  if (strcmp(cmd, "stepRequestBody") == 0 ||
      strcmp(cmd, "steprequestbody") == 0) {
    return parse_step_request_body_line(line, end, out);
  }
  if (strcmp(cmd, "stepReplacement") == 0 ||
      strcmp(cmd, "stepreplacement") == 0) {
    return parse_step_replacement_line(line, end, out);
  }
  if (strcmp(cmd, "stepCriterion") == 0 || strcmp(cmd, "stepcriterion") == 0) {
    return parse_step_criterion_line(line, end, out);
  }
  if (strcmp(cmd, "stepOnSuccess") == 0 || strcmp(cmd, "steponsuccess") == 0) {
    return parse_step_action_line(line, end, out, 0);
  }
  if (strcmp(cmd, "stepOnFailure") == 0 || strcmp(cmd, "steponfailure") == 0) {
    return parse_step_action_line(line, end, out, 1);
  }
  if (strcmp(cmd, "stepOutput") == 0 || strcmp(cmd, "stepoutput") == 0) {
    rc = ensure_current_step(out, &st);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return parse_key_val_line(line, end, &st->outputs, &st->n_outputs);
  }
  if (strcmp(cmd, "arazzoComponent") == 0 ||
      strcmp(cmd, "arazzocomponent") == 0) {
    return parse_arazzo_component_line(line, end, out);
  }
  if (strncmp(cmd, "x-", 2) == 0) {
    char *val = NULL;
    struct DocArazzoKeyVal *next_arr;
    rc = doc_extract_rest(line, end, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    next_arr = (struct DocArazzoKeyVal *)C_CDD_REALLOC(
        out->arazzo_extensions,
        (out->n_arazzo_extensions + 1) * sizeof(struct DocArazzoKeyVal));
    if (!next_arr) {
      C_CDD_FREE(val);
      return CDD_C_ERROR_MEMORY;
    }
    out->arazzo_extensions = next_arr;
    rc = c_cdd_strdup(cmd,
                      &out->arazzo_extensions[out->n_arazzo_extensions].key);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(val);
      return rc;
    }
    out->arazzo_extensions[out->n_arazzo_extensions].value = val;
    out->n_arazzo_extensions++;
    return CDD_C_SUCCESS;
  }

  *out_handled = 0;
  return CDD_C_SUCCESS;
}
