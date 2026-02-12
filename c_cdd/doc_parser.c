/**
 * @file doc_parser.c
 * @brief Implementation of the documentation comment parser.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doc_parser.h"
#include "str_utils.h"

/* --- Helpers --- */

/**
 * @brief Check if a character is a newline.
 */
static int is_eol(char c) { return c == '\n' || c == '\r'; }

/**
 * @brief Skip whitespace in a string buffer.
 */
static const char *skip_ws(const char *p) {
  while (*p && isspace((unsigned char)*p) && !is_eol(*p)) {
    p++;
  }
  return p;
}

/**
 * @brief Extract the next word from the string.
 *
 * @param str Input string pointer.
 * @param end Pointer to end of string (or newline).
 * @param next_out [out] Pointer to where the scan stopped.
 * @return Allocated string containing the word, or NULL on failure.
 */
static char *extract_word(const char *str, const char *end,
                          const char **next_out) {
  const char *p = skip_ws(str);
  const char *word_start = p;
  size_t len;
  char *res;

  while (p < end && !isspace((unsigned char)*p)) {
    p++;
  }

  len = (size_t)(p - word_start);
  if (len == 0) {
    if (next_out)
      *next_out = p;
    return NULL;
  }

  res = (char *)malloc(len + 1);
  if (!res)
    return NULL;

  memcpy(res, word_start, len);
  res[len] = '\0';

  if (next_out)
    *next_out = p;
  return res;
}

/**
 * @brief Extract the remainder of the line as text.
 * Trims leading/trailing whitespace.
 */
static char *extract_rest(const char *str, const char *end) {
  const char *p = skip_ws(str);
  const char *e = end;
  size_t len;
  char *res;

  /* Trim trailing */
  while (e > p && isspace((unsigned char)*(e - 1))) {
    e--;
  }

  len = (size_t)(e - p);
  if (len == 0)
    return NULL;

  res = (char *)malloc(len + 1);
  if (!res)
    return NULL;

  memcpy(res, p, len);
  res[len] = '\0';
  return res;
}

static int add_tag(struct DocMetadata *out, const char *tag) {
  char **new_tags;
  if (!out || !tag || !*tag)
    return 0;
  new_tags = (char **)realloc(out->tags, (out->n_tags + 1) * sizeof(char *));
  if (!new_tags)
    return ENOMEM;
  out->tags = new_tags;
  out->tags[out->n_tags] = c_cdd_strdup(tag);
  if (!out->tags[out->n_tags])
    return ENOMEM;
  out->n_tags++;
  return 0;
}

static char *trim_segment(char *s) {
  char *start;
  char *end;
  if (!s)
    return NULL;
  start = s;
  while (*start && isspace((unsigned char)*start))
    start++;
  end = start + strlen(start);
  while (end > start && isspace((unsigned char)*(end - 1)))
    end--;
  *end = '\0';
  return start;
}

static int parse_tags_line(const char *line, const char *end,
                           struct DocMetadata *out) {
  char *rest;
  char *cursor;
  int rc = 0;

  if (!out)
    return EINVAL;

  rest = extract_rest(line, end);
  if (!rest)
    return 0;

  cursor = rest;
  while (cursor && *cursor) {
    char *comma = strchr(cursor, ',');
    if (comma)
      *comma = '\0';
    {
      char *tag = trim_segment(cursor);
      if (tag && *tag) {
        rc = add_tag(out, tag);
        if (rc != 0)
          break;
      }
    }
    if (!comma)
      break;
    cursor = comma + 1;
  }

  free(rest);
  return rc;
}

static int parse_bool_text(const char *s, int *out) {
  if (!s || !out)
    return 0;
  if (strcasecmp(s, "true") == 0 || strcmp(s, "1") == 0 ||
      strcasecmp(s, "yes") == 0) {
    *out = 1;
    return 1;
  }
  if (strcasecmp(s, "false") == 0 || strcmp(s, "0") == 0 ||
      strcasecmp(s, "no") == 0) {
    *out = 0;
    return 1;
  }
  return 0;
}

static int parse_style_text(const char *s, enum DocParamStyle *out) {
  if (!s || !out)
    return 0;
  if (strcasecmp(s, "form") == 0) {
    *out = DOC_PARAM_STYLE_FORM;
    return 1;
  }
  if (strcasecmp(s, "simple") == 0) {
    *out = DOC_PARAM_STYLE_SIMPLE;
    return 1;
  }
  if (strcasecmp(s, "matrix") == 0) {
    *out = DOC_PARAM_STYLE_MATRIX;
    return 1;
  }
  if (strcasecmp(s, "label") == 0) {
    *out = DOC_PARAM_STYLE_LABEL;
    return 1;
  }
  if (strcasecmp(s, "spaceDelimited") == 0) {
    *out = DOC_PARAM_STYLE_SPACE_DELIMITED;
    return 1;
  }
  if (strcasecmp(s, "pipeDelimited") == 0) {
    *out = DOC_PARAM_STYLE_PIPE_DELIMITED;
    return 1;
  }
  if (strcasecmp(s, "deepObject") == 0) {
    *out = DOC_PARAM_STYLE_DEEP_OBJECT;
    return 1;
  }
  if (strcasecmp(s, "cookie") == 0) {
    *out = DOC_PARAM_STYLE_COOKIE;
    return 1;
  }
  return 0;
}

static void parse_optional_bool_attr(const char *attr, const char *key,
                                     int *out_set, int *out_val) {
  size_t key_len;
  int parsed;

  if (!attr || !key || !out_set || !out_val)
    return;

  key_len = strlen(key);
  if (strcmp(attr, key) == 0) {
    *out_set = 1;
    *out_val = 1;
    return;
  }
  if (strncmp(attr, key, key_len) == 0 && attr[key_len] == ':') {
    int value = 0;
    parsed = parse_bool_text(attr + key_len + 1, &value);
    if (parsed) {
      *out_set = 1;
      *out_val = value;
    }
  }
}

static int parse_deprecated_line(const char *line, const char *end,
                                 struct DocMetadata *out) {
  char *rest;
  int value = 1;

  if (!out)
    return EINVAL;

  out->deprecated_set = 1;
  rest = extract_rest(line, end);
  if (!rest) {
    out->deprecated = 1;
    return 0;
  }
  if (parse_bool_text(rest, &value))
    out->deprecated = value;
  else
    out->deprecated = 1;
  free(rest);
  return 0;
}

static int parse_external_docs_line(const char *line, const char *end,
                                    struct DocMetadata *out) {
  const char *cur = line;
  char *url;
  char *desc;

  if (!out)
    return EINVAL;

  url = extract_word(cur, end, &cur);
  if (!url)
    return 0;

  if (out->external_docs_url)
    free(out->external_docs_url);
  out->external_docs_url = url;

  desc = extract_rest(cur, end);
  if (out->external_docs_description)
    free(out->external_docs_description);
  out->external_docs_description = desc;

  return 0;
}

/* --- Core Logic --- */

int doc_metadata_init(struct DocMetadata *const meta) {
  if (!meta)
    return EINVAL;
  memset(meta, 0, sizeof(*meta));
  return 0;
}

void doc_metadata_free(struct DocMetadata *const meta) {
  size_t i;
  if (!meta)
    return;

  if (meta->route)
    free(meta->route);
  if (meta->verb)
    free(meta->verb);
  if (meta->operation_id)
    free(meta->operation_id);
  if (meta->summary)
    free(meta->summary);
  if (meta->description)
    free(meta->description);
  if (meta->external_docs_url)
    free(meta->external_docs_url);
  if (meta->external_docs_description)
    free(meta->external_docs_description);
  if (meta->tags) {
    for (i = 0; i < meta->n_tags; ++i) {
      free(meta->tags[i]);
    }
    free(meta->tags);
  }

  if (meta->params) {
    for (i = 0; i < meta->n_params; ++i) {
      free(meta->params[i].name);
      free(meta->params[i].in_loc);
      free(meta->params[i].description);
      if (meta->params[i].content_type)
        free(meta->params[i].content_type);
    }
    free(meta->params);
  }

  if (meta->returns) {
    for (i = 0; i < meta->n_returns; ++i) {
      free(meta->returns[i].code);
      free(meta->returns[i].description);
      if (meta->returns[i].content_type)
        free(meta->returns[i].content_type);
    }
    free(meta->returns);
  }

  if (meta->security) {
    for (i = 0; i < meta->n_security; ++i) {
      size_t s;
      free(meta->security[i].scheme);
      if (meta->security[i].scopes) {
        for (s = 0; s < meta->security[i].n_scopes; ++s)
          free(meta->security[i].scopes[s]);
        free(meta->security[i].scopes);
      }
    }
    free(meta->security);
  }

  if (meta->servers) {
    for (i = 0; i < meta->n_servers; ++i) {
      free(meta->servers[i].url);
      free(meta->servers[i].name);
      free(meta->servers[i].description);
    }
    free(meta->servers);
  }

  if (meta->request_body_description)
    free(meta->request_body_description);
  if (meta->request_body_content_type)
    free(meta->request_body_content_type);

  memset(meta, 0, sizeof(*meta));
}

static int parse_param_line(const char *line, const char *end,
                            struct DocMetadata *out) {
  struct DocParam *new_params;
  struct DocParam *p;
  const char *cur = line;

  /* Realloc array */
  new_params = (struct DocParam *)realloc(
      out->params, (out->n_params + 1) * sizeof(struct DocParam));
  if (!new_params)
    return ENOMEM;
  out->params = new_params;
  p = &out->params[out->n_params];
  memset(p, 0, sizeof(*p));

  /* 1. Name */
  p->name = extract_word(cur, end, &cur);
  if (!p->name) {
    /* Malformed param line, ignore but don't crash */
    return 0;
  }

  /* 2. Check for Attributes [key:val] or [required] */
  cur = skip_ws(cur);
  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      /* Extract content inside [] */
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)malloc(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        if (strncmp(attr, "in:", 3) == 0) {
          p->in_loc = c_cdd_strdup(attr + 3);
        } else if (strcmp(attr, "required") == 0) {
          p->required = 1;
        } else if (strncmp(attr, "contentType:", 12) == 0) {
          if (p->content_type)
            free(p->content_type);
          p->content_type = c_cdd_strdup(attr + 12);
        } else if (strncmp(attr, "style:", 6) == 0) {
          enum DocParamStyle style = DOC_PARAM_STYLE_UNSET;
          if (parse_style_text(attr + 6, &style)) {
            p->style = style;
            p->style_set = 1;
          }
        } else {
          parse_optional_bool_attr(attr, "explode", &p->explode_set,
                                   &p->explode);
          parse_optional_bool_attr(attr, "allowReserved",
                                   &p->allow_reserved_set, &p->allow_reserved);
          parse_optional_bool_attr(attr, "allowEmptyValue",
                                   &p->allow_empty_value_set,
                                   &p->allow_empty_value);
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = skip_ws(cur);
    } else {
      break; /* Unbalanced */
    }
  }

  /* 3. Description */
  p->description = extract_rest(cur, end);

  out->n_params++;
  return 0;
}

static int parse_return_line(const char *line, const char *end,
                             struct DocMetadata *out) {
  struct DocResponse *new_resps;
  struct DocResponse *r;
  const char *cur = line;

  new_resps = (struct DocResponse *)realloc(
      out->returns, (out->n_returns + 1) * sizeof(struct DocResponse));
  if (!new_resps)
    return ENOMEM;
  out->returns = new_resps;
  r = &out->returns[out->n_returns];
  memset(r, 0, sizeof(*r));

  /* 1. Code */
  r->code = extract_word(cur, end, &cur);
  if (!r->code) {
    return 0;
  }

  /* 2. Optional Attributes [key:val] */
  cur = skip_ws(cur);
  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)malloc(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        if (strncmp(attr, "contentType:", 12) == 0) {
          if (r->content_type)
            free(r->content_type);
          r->content_type = c_cdd_strdup(attr + 12);
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = skip_ws(cur);
    } else {
      break; /* Unbalanced */
    }
  }

  /* 3. Description */
  r->description = extract_rest(cur, end);

  out->n_returns++;
  return 0;
}

static int split_scopes(const char *input, char ***out_scopes,
                        size_t *out_count) {
  char *buf;
  char *token;
  char *saveptr = NULL;
  char **scopes = NULL;
  size_t n = 0;

  if (!out_scopes || !out_count)
    return EINVAL;
  *out_scopes = NULL;
  *out_count = 0;
  if (!input || !*input)
    return 0;

  buf = c_cdd_strdup(input);
  if (!buf)
    return ENOMEM;

  token = strtok_r(buf, ", \t", &saveptr);
  while (token) {
    char *trimmed = trim_segment(token);
    char **new_scopes;
    if (!trimmed || !*trimmed) {
      token = strtok_r(NULL, ", \t", &saveptr);
      continue;
    }
    new_scopes = (char **)realloc(scopes, (n + 1) * sizeof(char *));
    if (!new_scopes) {
      size_t i;
      for (i = 0; i < n; ++i)
        free(scopes[i]);
      free(scopes);
      free(buf);
      return ENOMEM;
    }
    scopes = new_scopes;
    scopes[n] = c_cdd_strdup(trimmed);
    if (!scopes[n]) {
      size_t i;
      for (i = 0; i < n; ++i)
        free(scopes[i]);
      free(scopes);
      free(buf);
      return ENOMEM;
    }
    n++;
    token = strtok_r(NULL, ", \t", &saveptr);
  }

  free(buf);
  *out_scopes = scopes;
  *out_count = n;
  return 0;
}

static int parse_security_line(const char *line, const char *end,
                               struct DocMetadata *out) {
  const char *cur = line;
  char *scheme;
  char *rest;
  char **scopes = NULL;
  size_t n_scopes = 0;
  struct DocSecurityRequirement *new_reqs;
  struct DocSecurityRequirement *req;
  int rc;

  if (!out)
    return EINVAL;

  scheme = extract_word(cur, end, &cur);
  if (!scheme)
    return 0;

  rest = extract_rest(cur, end);
  rc = split_scopes(rest, &scopes, &n_scopes);
  if (rc != 0) {
    free(scheme);
    if (rest)
      free(rest);
    return rc;
  }

  new_reqs = (struct DocSecurityRequirement *)realloc(
      out->security,
      (out->n_security + 1) * sizeof(struct DocSecurityRequirement));
  if (!new_reqs) {
    size_t i;
    for (i = 0; i < n_scopes; ++i)
      free(scopes[i]);
    free(scopes);
    free(scheme);
    if (rest)
      free(rest);
    return ENOMEM;
  }
  out->security = new_reqs;
  req = &out->security[out->n_security];
  memset(req, 0, sizeof(*req));
  req->scheme = scheme;
  req->scopes = scopes;
  req->n_scopes = n_scopes;
  out->n_security++;

  if (rest)
    free(rest);
  return 0;
}

static char *find_key_token(char *s, const char *key, size_t *key_len) {
  char *p;
  size_t klen;
  if (!s || !key)
    return NULL;
  klen = strlen(key);
  p = strstr(s, key);
  while (p) {
    if ((p == s || isspace((unsigned char)p[-1])) &&
        (p[klen] == '=' || p[klen] == ':')) {
      if (key_len)
        *key_len = klen + 1;
      return p;
    }
    p = strstr(p + klen, key);
  }
  return NULL;
}

static int parse_server_line(const char *line, const char *end,
                             struct DocMetadata *out) {
  const char *cur = line;
  char *url;
  char *rest;
  char *name = NULL;
  char *desc = NULL;
  struct DocServer *new_servers;
  struct DocServer *srv;

  if (!out)
    return EINVAL;

  url = extract_word(cur, end, &cur);
  if (!url)
    return 0;

  rest = extract_rest(cur, end);
  if (rest && *rest) {
    size_t name_key_len = 0;
    size_t desc_key_len = 0;
    char *name_key = find_key_token(rest, "name", &name_key_len);
    char *desc_key = find_key_token(rest, "description", &desc_key_len);

    if (name_key) {
      char *name_start = name_key + name_key_len;
      char *name_end = NULL;
      char saved;
      if (desc_key && desc_key > name_start) {
        name_end = desc_key;
      } else {
        name_end = name_start + strcspn(name_start, " \t");
      }
      saved = *name_end;
      *name_end = '\0';
      {
        char *trimmed = trim_segment(name_start);
        if (trimmed && *trimmed)
          name = c_cdd_strdup(trimmed);
      }
      *name_end = saved;
    }
    if (desc_key) {
      char *desc_start = desc_key + desc_key_len;
      char *trimmed = trim_segment(desc_start);
      if (trimmed && *trimmed)
        desc = c_cdd_strdup(trimmed);
    }
    if (!name_key && !desc_key) {
      char *trimmed = trim_segment(rest);
      if (trimmed && *trimmed)
        desc = c_cdd_strdup(trimmed);
    }
  }

  new_servers = (struct DocServer *)realloc(
      out->servers, (out->n_servers + 1) * sizeof(struct DocServer));
  if (!new_servers) {
    free(url);
    if (name)
      free(name);
    if (desc)
      free(desc);
    if (rest)
      free(rest);
    return ENOMEM;
  }
  out->servers = new_servers;
  srv = &out->servers[out->n_servers];
  memset(srv, 0, sizeof(*srv));
  srv->url = url;
  srv->name = name;
  srv->description = desc;
  out->n_servers++;

  if (rest)
    free(rest);
  return 0;
}

static int parse_request_body_line(const char *line, const char *end,
                                   struct DocMetadata *out) {
  const char *cur = line;

  if (!out)
    return EINVAL;

  cur = skip_ws(cur);
  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)malloc(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        parse_optional_bool_attr(attr, "required",
                                 &out->request_body_required_set,
                                 &out->request_body_required);
        if (strncmp(attr, "contentType:", 12) == 0 ||
            strncmp(attr, "contentType=", 12) == 0) {
          char *val = trim_segment(attr + 12);
          if (val && *val) {
            if (out->request_body_content_type)
              free(out->request_body_content_type);
            out->request_body_content_type = c_cdd_strdup(val);
          }
        } else if (strncmp(attr, "content:", 8) == 0 ||
                   strncmp(attr, "content=", 8) == 0) {
          char *val = trim_segment(attr + 8);
          if (val && *val) {
            if (out->request_body_content_type)
              free(out->request_body_content_type);
            out->request_body_content_type = c_cdd_strdup(val);
          }
        }

        free(attr);
      }
      cur = close_bracket + 1;
      cur = skip_ws(cur);
    } else {
      break;
    }
  }

  if (out->request_body_description)
    free(out->request_body_description);
  out->request_body_description = extract_rest(cur, end);
  return 0;
}

static int parse_route_line(const char *line, const char *end,
                            struct DocMetadata *out) {
  const char *cur = line;
  char *word1 = extract_word(cur, end, &cur);
  char *word2 = NULL;

  if (!word1)
    return 0;

  /* Check if word1 is a Verb or Path */
  /* Heuristic: Starts with / is path. Uppercase is verb. */
  if (word1[0] == '/') {
    /* No verb specified */
    if (out->route)
      free(out->route);
    out->route = word1;
  } else {
    /* Assume verb */
    if (out->verb)
      free(out->verb);
    out->verb = word1;

    /* Next word should be path */
    word2 = extract_word(cur, end, &cur);
    if (word2) {
      if (out->route)
        free(out->route);
      out->route = word2;
    }
  }
  return 0;
}

int doc_parse_block(const char *const comment, struct DocMetadata *const out) {
  const char *p = comment;
  int rc = 0;

  if (!comment || !out)
    return EINVAL;

  /* Skip initial marker if present (e.g. "/**" or "///") via simple scan loop
   */
  while (*p) {
    const char *line_end;
    const char *scan;

    /* Find end of current line */
    line_end = p;
    while (*line_end && !is_eol(*line_end)) {
      line_end++;
    }

    /* Analyze line content */
    scan = p;

    /* Skip leading whitespace */
    while (scan < line_end && isspace((unsigned char)*scan))
      scan++;

    /* Skip Comment Decorators */
    if (scan < line_end && *scan == '/') {
      scan++;
      if (scan < line_end && *scan == '*') {
        /* Block comment start */
        scan++;
        while (scan < line_end && *scan == '*')
          scan++;
      } else if (scan < line_end && *scan == '/') {
        /* Line comment start */
        scan++;
        while (scan < line_end && *scan == '/')
          scan++;
      }
    } else if (scan < line_end && *scan == '*') {
      /* Continuation line start */
      scan++;
      /* Also skip closing slash if present */
      if (scan < line_end && *scan == '/')
        scan++;
    }

    /* Skip whitespace again after decorators */
    while (scan < line_end && isspace((unsigned char)*scan))
      scan++;

    /* Check for Directive (@ or \) */
    if (scan < line_end && (*scan == '@' || *scan == '\\')) {
      const char *cmd_start = scan + 1;
      const char *cmd_end = cmd_start;
      char *cmd = NULL;

      while (cmd_end < line_end && isalpha((unsigned char)*cmd_end)) {
        cmd_end++;
      }

      {
        size_t cmd_len = (size_t)(cmd_end - cmd_start);
        cmd = (char *)malloc(cmd_len + 1);
        if (!cmd) {
          rc = ENOMEM;
          goto cleanup;
        }
        memcpy(cmd, cmd_start, cmd_len);
        cmd[cmd_len] = '\0';

        /* Dispatch */
        if (strcmp(cmd, "route") == 0) {
          rc = parse_route_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "param") == 0) {
          rc = parse_param_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "return") == 0 || strcmp(cmd, "returns") == 0) {
          rc = parse_return_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "summary") == 0 || strcmp(cmd, "brief") == 0) {
          if (out->summary)
            free(out->summary);
          out->summary = extract_rest(cmd_end, line_end);
        } else if (strcmp(cmd, "operationId") == 0 ||
                   strcmp(cmd, "operationid") == 0) {
          if (out->operation_id)
            free(out->operation_id);
          out->operation_id = extract_rest(cmd_end, line_end);
        } else if (strcmp(cmd, "description") == 0 ||
                   strcmp(cmd, "details") == 0) {
          if (out->description)
            free(out->description);
          out->description = extract_rest(cmd_end, line_end);
        } else if (strcmp(cmd, "tag") == 0 || strcmp(cmd, "tags") == 0) {
          rc = parse_tags_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "deprecated") == 0) {
          rc = parse_deprecated_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "externalDocs") == 0 ||
                   strcmp(cmd, "externaldocs") == 0) {
          rc = parse_external_docs_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "security") == 0) {
          rc = parse_security_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "server") == 0) {
          rc = parse_server_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "requestBody") == 0 ||
                   strcmp(cmd, "requestbody") == 0) {
          rc = parse_request_body_line(cmd_end, line_end, out);
        }

        free(cmd);
        if (rc != 0)
          goto cleanup;
      }
    } else {
      /* Continuation / Description lines not handled in this basic pass */
    }

    /* Advance to next line */
    p = line_end;
    while (*p && is_eol(*p))
      p++;
  }

cleanup:
  return rc;
}
