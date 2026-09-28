/* clang-format off */
#include "c_cdd/memory.h"
/**
 * @file doc_operations.c
 * @brief Operation parameters, responses, headers, and request body parsing.
 *
 * @author Samuel Marks
 */

#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd_export.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "docstrings/parse/doc.h"
#include "docstrings/parse/doc_internal.h"
#include "functions/parse/str.h"
/* clang-format on */

cdd_c_error_t doc_parse_param_line(const char *line, const char *end,
                                   struct DocMetadata *out) {
  cdd_c_error_t rc_opt;
  struct DocParam *new_params;
  struct DocParam *p;
  const char *cur = line;
  cdd_c_error_t rc;

  /* Realloc array */
  new_params = (struct DocParam *)C_CDD_REALLOC(
      out->params, (out->n_params + 1) * sizeof(struct DocParam));
  if (!new_params) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  out->params = new_params;
  p = &out->params[out->n_params];
  memset(p, 0, sizeof(*p));

  /* 1. Name */
  rc = doc_extract_word(cur, end, &cur, &p->name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!p->name) {
    /* Malformed param line, ignore but don't crash */
    return CDD_C_SUCCESS;
  }
  out->n_params++;

  /* 2. Check for Attributes [key:val] or [required] */
  rc = doc_skip_ws(cur, &cur);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      /* Extract content inside [] */
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)(size_t)C_CDD_MALLOC(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        if (strncmp(attr, "in:", 3) == 0) {
          rc = c_cdd_strdup(attr + 3, &p->in_loc);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
        } else if (strcmp(attr, "required") == 0) {
          p->required = 1;
        } else if (strncmp(attr, "contentType:", 12) == 0) {
          if (p->content_type)
            C_CDD_FREE(p->content_type);
          rc = c_cdd_strdup(attr + 12, &p->content_type);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
        } else if (strncmp(attr, "format:", 7) == 0 ||
                   strncmp(attr, "format=", 7) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 7, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (p->format)
              C_CDD_FREE(p->format);
            rc = c_cdd_strdup(val, &p->format);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "style:", 6) == 0) {
          enum DocParamStyle style = DOC_PARAM_STYLE_UNSET;
          if (doc_parse_style_text(attr + 6, &style) == CDD_C_SUCCESS) {
            p->style = style;
            p->style_set = 1;
          }
        } else {
          rc_opt = doc_parse_optional_bool_attr(attr, "explode",
                                                &p->explode_set, &p->explode);
          if (rc_opt == CDD_C_ERROR_MEMORY) {
            C_CDD_FREE(attr);
            return rc_opt;
          }
          rc_opt = doc_parse_optional_bool_attr(attr, "allowReserved",
                                                &p->allow_reserved_set,
                                                &p->allow_reserved);
          if (rc_opt == CDD_C_ERROR_MEMORY) {
            C_CDD_FREE(attr);
            return rc_opt;
          }
          rc_opt = doc_parse_optional_bool_attr(attr, "allowEmptyValue",
                                                &p->allow_empty_value_set,
                                                &p->allow_empty_value);
          if (rc_opt == CDD_C_ERROR_MEMORY) {
            C_CDD_FREE(attr);
            return rc_opt;
          }
          if (strcmp(attr, "itemSchema") == 0 ||
              strcmp(attr, "itemSchema:true") == 0 ||
              strcmp(attr, "itemSchema=true") == 0) {
            p->item_schema = 1;
          }
          rc_opt = doc_parse_optional_bool_attr(
              attr, "deprecated", &p->deprecated_set, &p->deprecated);
          if (rc_opt == CDD_C_ERROR_MEMORY) {
            C_CDD_FREE(attr);
            return rc_opt;
          }
          if (doc_parse_optional_example_attr(attr, &p->example) == ENOMEM) {
            C_CDD_FREE(attr);
            return CDD_C_ERROR_MEMORY;
          }
        }
        C_CDD_FREE(attr);
      }
      cur = close_bracket + 1;
      rc = doc_skip_ws(cur, &cur);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      break; /* Unbalanced */
    }
  }

  /* 3. Description */
  rc = doc_extract_rest(cur, end, &p->description);
  return rc;
}

cdd_c_error_t doc_parse_return_line(const char *line, const char *end,
                                    struct DocMetadata *out) {
  struct DocResponse *new_resps;
  struct DocResponse *r;
  const char *cur = line;
  cdd_c_error_t rc;

  new_resps = (struct DocResponse *)C_CDD_REALLOC(
      out->returns, (out->n_returns + 1) * sizeof(struct DocResponse));
  if (!new_resps) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  out->returns = new_resps;
  r = &out->returns[out->n_returns];
  memset(r, 0, sizeof(*r));

  /* 1. Code */
  rc = doc_extract_word(cur, end, &cur, &r->code);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!r->code) {
    return CDD_C_SUCCESS;
  }
  out->n_returns++;

  /* 2. Optional Attributes [key:val] */
  rc = doc_skip_ws(cur, &cur);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)(size_t)C_CDD_MALLOC(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        if (strncmp(attr, "contentType:", 12) == 0 ||
            strncmp(attr, "contentType=", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (r->content_type)
              C_CDD_FREE(r->content_type);
            rc = c_cdd_strdup(val, &r->content_type);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "summary:", 8) == 0 ||
                   strncmp(attr, "summary=", 8) == 0) {
          char *val = NULL;
          rc = doc_trim_segment((char *)(size_t)(attr + 8), &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (r->summary)
              C_CDD_FREE(r->summary);
            rc = c_cdd_strdup(val, &r->summary);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strcmp(attr, "itemSchema") == 0 ||
                   strcmp(attr, "itemSchema:true") == 0 ||
                   strcmp(attr, "itemSchema=true") == 0) {
          r->item_schema = 1;
        } else if (doc_parse_optional_example_attr(attr, &r->example) ==
                   ENOMEM) {
          C_CDD_FREE(attr);
          return CDD_C_ERROR_MEMORY;
        }
        C_CDD_FREE(attr);
      }
      cur = close_bracket + 1;
      rc = doc_skip_ws(cur, &cur);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      break; /* Unbalanced */
    }
  }

  /* 3. Description */
  rc = doc_extract_rest(cur, end, &r->description);
  return rc;
}

cdd_c_error_t doc_parse_response_header_line(const char *line, const char *end,
                                             struct DocMetadata *out) {
  cdd_c_error_t rc_opt;
  struct DocResponseHeader *new_headers;
  struct DocResponseHeader *h;
  const char *cur = line;
  cdd_c_error_t rc;

  new_headers = (struct DocResponseHeader *)C_CDD_REALLOC(
      out->response_headers,
      (out->n_response_headers + 1) * sizeof(struct DocResponseHeader));
  if (!new_headers) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  out->response_headers = new_headers;
  h = &out->response_headers[out->n_response_headers];
  memset(h, 0, sizeof(*h));

  rc = doc_extract_word(cur, end, &cur, &h->code);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!h->code)
    return CDD_C_SUCCESS;

  rc = doc_extract_word(cur, end, &cur, &h->name);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(h->code);
    h->code = NULL;
    return rc;
  }
  if (!h->name) {
    C_CDD_FREE(h->code);
    h->code = NULL;
    return CDD_C_SUCCESS;
  }
  out->n_response_headers++;

  rc = doc_skip_ws(cur, &cur);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)(size_t)C_CDD_MALLOC(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        if (strncmp(attr, "type:", 5) == 0) {
          if (h->type)
            C_CDD_FREE(h->type);
          rc = c_cdd_strdup(attr + 5, &h->type);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
        } else if (strncmp(attr, "format:", 7) == 0 ||
                   strncmp(attr, "format=", 7) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 7, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (h->format)
              C_CDD_FREE(h->format);
            rc = c_cdd_strdup(val, &h->format);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "contentType:", 12) == 0 ||
                   strncmp(attr, "contentType=", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (h->content_type)
              C_CDD_FREE(h->content_type);
            rc = c_cdd_strdup(val, &h->content_type);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "content:", 8) == 0 ||
                   strncmp(attr, "content=", 8) == 0) {
          char *val = NULL;
          rc = doc_trim_segment((char *)(size_t)(attr + 8), &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (h->content_type)
              C_CDD_FREE(h->content_type);
            rc = c_cdd_strdup(val, &h->content_type);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (doc_parse_optional_example_attr(attr, &h->example) ==
                   ENOMEM) {
          C_CDD_FREE(attr);
          return CDD_C_ERROR_MEMORY;
        } else {
          rc_opt = doc_parse_optional_bool_attr(attr, "required",
                                                &h->required_set, &h->required);
          if (rc_opt == CDD_C_ERROR_MEMORY) {
            C_CDD_FREE(attr);
            return rc_opt;
          }
        }
        C_CDD_FREE(attr);
      }
      cur = close_bracket + 1;
      rc = doc_skip_ws(cur, &cur);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      break;
    }
  }

  rc = doc_extract_rest(cur, end, &h->description);
  return rc;
}

cdd_c_error_t doc_parse_link_line(const char *line, const char *end,
                                  struct DocMetadata *out) {
  struct DocLink *new_links;
  struct DocLink *link;
  const char *cur = line;
  cdd_c_error_t rc;

  new_links = (struct DocLink *)C_CDD_REALLOC(
      out->links, (out->n_links + 1) * sizeof(struct DocLink));
  if (!new_links) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  out->links = new_links;
  link = &out->links[out->n_links];
  memset(link, 0, sizeof(*link));

  rc = doc_extract_word(cur, end, &cur, &link->code);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!link->code)
    return CDD_C_SUCCESS;

  rc = doc_extract_word(cur, end, &cur, &link->name);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(link->code);
    link->code = NULL;
    return rc;
  }
  if (!link->name) {
    C_CDD_FREE(link->code);
    link->code = NULL;
    return CDD_C_SUCCESS;
  }

  rc = doc_skip_ws(cur, &cur);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)(size_t)C_CDD_MALLOC(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        if (strncmp(attr, "operationId:", 12) == 0 ||
            strncmp(attr, "operationId=", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (link->operation_id)
              C_CDD_FREE(link->operation_id);
            rc = c_cdd_strdup(val, &link->operation_id);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "operationRef:", 13) == 0 ||
                   strncmp(attr, "operationRef=", 13) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 13, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (link->operation_ref)
              C_CDD_FREE(link->operation_ref);
            rc = c_cdd_strdup(val, &link->operation_ref);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "parameters:", 11) == 0 ||
                   strncmp(attr, "parameters=", 11) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 11, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (link->parameters_json)
              C_CDD_FREE(link->parameters_json);
            rc = c_cdd_strdup(val, &link->parameters_json);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "requestBody:", 12) == 0 ||
                   strncmp(attr, "requestBody=", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (link->request_body_json)
              C_CDD_FREE(link->request_body_json);
            rc = c_cdd_strdup(val, &link->request_body_json);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "summary:", 8) == 0 ||
                   strncmp(attr, "summary=", 8) == 0) {
          char *val = NULL;
          rc = doc_trim_segment((char *)(size_t)(attr + 8), &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (link->summary)
              C_CDD_FREE(link->summary);
            rc = c_cdd_strdup(val, &link->summary);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "serverUrl:", 10) == 0 ||
                   strncmp(attr, "serverUrl=", 10) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 10, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (link->server_url)
              C_CDD_FREE(link->server_url);
            rc = c_cdd_strdup(val, &link->server_url);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "serverName:", 11) == 0 ||
                   strncmp(attr, "serverName=", 11) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 11, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (link->server_name)
              C_CDD_FREE(link->server_name);
            rc = c_cdd_strdup(val, &link->server_name);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "serverDescription:", 18) == 0 ||
                   strncmp(attr, "serverDescription=", 18) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 18, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (link->server_description)
              C_CDD_FREE(link->server_description);
            rc = c_cdd_strdup(val, &link->server_description);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "description:", 12) == 0 ||
                   strncmp(attr, "description=", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (link->description)
              C_CDD_FREE(link->description);
            rc = c_cdd_strdup(val, &link->description);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        }

        C_CDD_FREE(attr);
      }
      cur = close_bracket + 1;
      rc = doc_skip_ws(cur, &cur);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      break;
    }
  }

  if (!link->description) {
    rc = doc_extract_rest(cur, end, &link->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  out->n_links++;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_request_body_line(const char *line, const char *end,
                                          struct DocMetadata *out) {
  cdd_c_error_t rc_opt;
  const char *cur = line;
  struct DocRequestBody *new_arr;
  struct DocRequestBody *entry;
  char *content_type = NULL;
  char *description = NULL;
  char *example = NULL;
  int required_set = 0;
  int required_val = 0;
  int item_schema = 0;
  cdd_c_error_t rc;

  rc = doc_skip_ws(cur, &cur);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)(size_t)C_CDD_MALLOC(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        rc_opt = doc_parse_optional_bool_attr(attr, "required", &required_set,
                                              &required_val);
        if (rc_opt == CDD_C_ERROR_MEMORY) {
          C_CDD_FREE(attr);
          return rc_opt;
        }
        if (strncmp(attr, "contentType:", 12) == 0 ||
            strncmp(attr, "contentType=", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (content_type)
              C_CDD_FREE(content_type);
            rc = c_cdd_strdup(val, &content_type);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "content:", 8) == 0 ||
                   strncmp(attr, "content=", 8) == 0) {
          char *val = NULL;
          rc = doc_trim_segment((char *)(size_t)(attr + 8), &val);
          if (rc == CDD_C_SUCCESS && *val) {
            if (content_type)
              C_CDD_FREE(content_type);
            rc = c_cdd_strdup(val, &content_type);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strcmp(attr, "itemSchema") == 0 ||
                   strcmp(attr, "itemSchema:true") == 0 ||
                   strcmp(attr, "itemSchema=true") == 0) {
          item_schema = 1;
        } else if (doc_parse_optional_example_attr(attr, &example) == ENOMEM) {
          C_CDD_FREE(attr);
          if (content_type)
            C_CDD_FREE(content_type);
          return CDD_C_ERROR_MEMORY;
        }

        C_CDD_FREE(attr);
      }
      cur = close_bracket + 1;
      rc = doc_skip_ws(cur, &cur);
      if (rc != CDD_C_SUCCESS) {
        if (content_type)
          C_CDD_FREE(content_type);
        if (example)
          C_CDD_FREE(example);
        return rc;
      }
    } else {
      break;
    }
  }

  rc = doc_extract_rest(cur, end, &description);
  if (rc != CDD_C_SUCCESS) {
    if (content_type)
      C_CDD_FREE(content_type);
    if (example)
      C_CDD_FREE(example);
    return rc;
  }

  new_arr = (struct DocRequestBody *)C_CDD_REALLOC(
      out->request_bodies,
      (out->n_request_bodies + 1) * sizeof(struct DocRequestBody));
  if (!new_arr) {
    if (content_type)
      C_CDD_FREE(content_type);
    if (description)
      C_CDD_FREE(description);
    if (example)
      C_CDD_FREE(example);
    return CDD_C_ERROR_MEMORY;
  }
  out->request_bodies = new_arr;
  entry = &out->request_bodies[out->n_request_bodies];
  memset(entry, 0, sizeof(*entry));
  entry->content_type = content_type;
  entry->description = description;
  entry->example = example;
  entry->item_schema = item_schema;
  out->n_request_bodies++;

  if (required_set) {
    out->request_body_required_set = 1;
    out->request_body_required = required_val ? 1 : 0;
  }

  if (entry->content_type) {
    if (out->request_body_content_type)
      C_CDD_FREE(out->request_body_content_type);
    rc = c_cdd_strdup(entry->content_type, &out->request_body_content_type);
    if (rc != CDD_C_SUCCESS || !out->request_body_content_type) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
  }
  if (entry->description) {
    if (out->request_body_description)
      C_CDD_FREE(out->request_body_description);
    rc = c_cdd_strdup(entry->description, &out->request_body_description);
    if (rc != CDD_C_SUCCESS || !out->request_body_description) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
  }
  return CDD_C_SUCCESS;
}
