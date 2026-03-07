/**
 * @file doc.c
 * @brief Implementation of the documentation comment parser.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "docstrings/parse/doc.h"
#include "functions/parse/str.h"


/* --- Helpers --- */

/**
 * @brief Check if a character is a newline.
 */
static int is_eol(char c) { return c == '\n' || c == '\r'; }

/**
 * @brief Skip whitespace in a string buffer.
 */
static int skip_ws(const char *p, const char **_out_val) {
  while (*p && isspace((unsigned char)*p) && !is_eol(*p)) {
    p++;
  }
  {
    *_out_val = p;
    return 0;
  }
}

/**
 * @brief Extract the next word from the string.
 *
 * @param str Input string pointer.
 * @param end Pointer to end of string (or newline).
 * @param next_out [out] Pointer to where the scan stopped.
 * @return Allocated string containing the word, or NULL on failure.
 */
static int extract_word(const char *str, const char *end, const char **next_out,
                        char **_out_val) {
  const char *_ast_skip_ws_0;
  const char *p = (skip_ws(str, &_ast_skip_ws_0), _ast_skip_ws_0);
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
    {
      *_out_val = NULL;
      return 0;
    }
  }

  res = (char *)malloc(len + 1);
  if (!res) {
    *_out_val = NULL;
    return 0;
  }

  memcpy(res, word_start, len);
  res[len] = '\0';

  if (next_out)
    *next_out = p;
  {
    *_out_val = res;
    return 0;
  }
}

/**
 * @brief Extract the remainder of the line as text.
 * Trims leading/trailing whitespace.
 */
static int extract_rest(const char *str, const char *end, char **_out_val) {
  const char *_ast_skip_ws_1;
  const char *p = (skip_ws(str, &_ast_skip_ws_1), _ast_skip_ws_1);
  const char *e = end;
  size_t len;
  char *res;

  /* Trim trailing */
  while (e > p && isspace((unsigned char)*(e - 1))) {
    e--;
  }

  len = (size_t)(e - p);
  if (len == 0) {
    *_out_val = NULL;
    return 0;
  }

  res = (char *)malloc(len + 1);
  if (!res) {
    *_out_val = NULL;
    return 0;
  }

  memcpy(res, p, len);
  res[len] = '\0';
  {
    *_out_val = res;
    return 0;
  }
}

static int add_tag(struct DocMetadata *out, const char *tag) {
  char *_ast_strdup_0 = NULL;
  char **new_tags;
  if (!out || !tag || !*tag)
    return 0;
  new_tags = (char **)realloc(out->tags, (out->n_tags + 1) * sizeof(char *));
  if (!new_tags)
    return ENOMEM;
  out->tags = new_tags;
  out->tags[out->n_tags] = (c_cdd_strdup(tag, &_ast_strdup_0), _ast_strdup_0);
  if (!out->tags[out->n_tags])
    return ENOMEM;
  out->n_tags++;
  return 0;
}

static int add_tag_meta(struct DocMetadata *out, struct DocTagMeta *meta) {
  struct DocTagMeta *new_meta;
  if (!out || !meta || !meta->name || !*meta->name)
    return 0;
  new_meta = (struct DocTagMeta *)realloc(
      out->tag_meta, (out->n_tag_meta + 1) * sizeof(struct DocTagMeta));
  if (!new_meta)
    return ENOMEM;
  out->tag_meta = new_meta;
  out->tag_meta[out->n_tag_meta] = *meta;
  out->n_tag_meta++;
  return 0;
}

static int trim_segment(char *s, char **_out_val) {
  char *start;
  char *end;
  if (!s) {
    *_out_val = NULL;
    return 0;
  }
  start = s;
  while (*start && isspace((unsigned char)*start))
    start++;
  end = start + strlen(start);
  while (end > start && isspace((unsigned char)*(end - 1)))
    end--;
  *end = '\0';
  {
    *_out_val = start;
    return 0;
  }
}

static int parse_tags_line(const char *line, const char *end,
                           struct DocMetadata *out) {
  char *_ast_extract_rest_2;
  char *_ast_trim_segment_3;
  char *rest;
  char *cursor;
  int rc = 0;

  if (!out)
    return EINVAL;

  rest = (extract_rest(line, end, &_ast_extract_rest_2), _ast_extract_rest_2);
  if (!rest)
    return 0;

  cursor = rest;
  while (cursor && *cursor) {
    char *comma = strchr(cursor, ',');
    if (comma)
      *comma = '\0';
    {
      char *tag =
          (trim_segment(cursor, &_ast_trim_segment_3), _ast_trim_segment_3);
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
  if (c_cdd_stricmp(s, "true") == 0 || strcmp(s, "1") == 0 ||
      c_cdd_stricmp(s, "yes") == 0) {
    *out = 1;
    return 1;
  }
  if (c_cdd_stricmp(s, "false") == 0 || strcmp(s, "0") == 0 ||
      c_cdd_stricmp(s, "no") == 0) {
    *out = 0;
    return 1;
  }
  return 0;
}

static int parse_tag_meta_line(const char *line, const char *end,
                               struct DocMetadata *out) {
  char *_ast_extract_word_4;
  const char *_ast_skip_ws_5;
  char *_ast_trim_segment_6;
  char *_ast_trim_segment_7;
  char *_ast_trim_segment_8;
  char *_ast_trim_segment_9;
  char *_ast_trim_segment_10;
  char *_ast_trim_segment_11;
  const char *_ast_skip_ws_12;
  char *_ast_strdup_1 = NULL;
  char *_ast_strdup_2 = NULL;
  char *_ast_strdup_3 = NULL;
  char *_ast_strdup_4 = NULL;
  char *_ast_strdup_5 = NULL;
  char *_ast_strdup_6 = NULL;
  const char *cur = line;
  struct DocTagMeta meta;

  if (!out)
    return EINVAL;

  memset(&meta, 0, sizeof(meta));
  printf("LINE: %.*s\n", (int)(end - line), line);

  meta.name =
      (extract_word(cur, end, &cur, &_ast_extract_word_4), _ast_extract_word_4);
  if (!meta.name)
    return 0;

  cur = (skip_ws(cur, &_ast_skip_ws_5), _ast_skip_ws_5);
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

        if (strncmp(attr, "summary:", 8) == 0) {
          char *val = (trim_segment((char *)(attr + 8), &_ast_trim_segment_6),
                       _ast_trim_segment_6);
          if (val && *val)
            meta.summary = (c_cdd_strdup(val, &_ast_strdup_1), _ast_strdup_1);
        } else if (strncmp(attr, "description:", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_7),
                       _ast_trim_segment_7);
          if (val && *val)
            meta.description =
                (c_cdd_strdup(val, &_ast_strdup_2), _ast_strdup_2);
        } else if (strncmp(attr, "parent:", 7) == 0) {
          char *val = (trim_segment(attr + 7, &_ast_trim_segment_8),
                       _ast_trim_segment_8);
          if (val && *val)
            meta.parent = (c_cdd_strdup(val, &_ast_strdup_3), _ast_strdup_3);
        } else if (strncmp(attr, "kind:", 5) == 0) {
          char *val = (trim_segment(attr + 5, &_ast_trim_segment_9),
                       _ast_trim_segment_9);
          if (val && *val)
            meta.kind = (c_cdd_strdup(val, &_ast_strdup_4), _ast_strdup_4);
        } else if (strncmp(attr, "externalDocs:", 13) == 0 ||
                   strncmp(attr, "externalDocs=", 13) == 0) {
          char *val = (trim_segment(attr + 13, &_ast_trim_segment_10),
                       _ast_trim_segment_10);
          if (val && *val)
            meta.external_docs_url =
                (c_cdd_strdup(val, &_ast_strdup_5), _ast_strdup_5);
        } else if (strncmp(attr, "externalDocsDescription:", 24) == 0 ||
                   strncmp(attr, "externalDocsDescription=", 24) == 0) {
          char *val = (trim_segment(attr + 24, &_ast_trim_segment_11),
                       _ast_trim_segment_11);
          if (val && *val)
            meta.external_docs_description =
                (c_cdd_strdup(val, &_ast_strdup_6), _ast_strdup_6);
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = (skip_ws(cur, &_ast_skip_ws_12), _ast_skip_ws_12);
    } else {
      break;
    }
  }

  return add_tag_meta(out, &meta);
}

static int parse_style_text(const char *s, enum DocParamStyle *out) {
  if (!s || !out)
    return 0;
  if (c_cdd_stricmp(s, "form") == 0) {
    *out = DOC_PARAM_STYLE_FORM;
    return 1;
  }
  if (c_cdd_stricmp(s, "simple") == 0) {
    *out = DOC_PARAM_STYLE_SIMPLE;
    return 1;
  }
  if (c_cdd_stricmp(s, "matrix") == 0) {
    *out = DOC_PARAM_STYLE_MATRIX;
    return 1;
  }
  if (c_cdd_stricmp(s, "label") == 0) {
    *out = DOC_PARAM_STYLE_LABEL;
    return 1;
  }
  if (c_cdd_stricmp(s, "spaceDelimited") == 0) {
    *out = DOC_PARAM_STYLE_SPACE_DELIMITED;
    return 1;
  }
  if (c_cdd_stricmp(s, "pipeDelimited") == 0) {
    *out = DOC_PARAM_STYLE_PIPE_DELIMITED;
    return 1;
  }
  if (c_cdd_stricmp(s, "deepObject") == 0) {
    *out = DOC_PARAM_STYLE_DEEP_OBJECT;
    return 1;
  }
  if (c_cdd_stricmp(s, "cookie") == 0) {
    *out = DOC_PARAM_STYLE_COOKIE;
    return 1;
  }
  return 0;
}

static void parse_optional_bool_attr(const char *attr, const char *key,
                                     int *out_set, int *out_val) {
  char *_ast_trim_segment_13;
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
  if (strncmp(attr, key, key_len) == 0 &&
      (attr[key_len] == ':' || attr[key_len] == '=')) {
    int value = 0;
    char *val_trimmed =
        (trim_segment((char *)(attr + key_len + 1), &_ast_trim_segment_13),
         _ast_trim_segment_13);
    parsed = parse_bool_text(val_trimmed, &value);
    if (parsed) {
      *out_set = 1;
      *out_val = value;
    }
  }
}

static int parse_optional_example_attr(const char *attr, char **out_example) {
  char *_ast_trim_segment_14;
  char *_ast_strdup_7 = NULL;
  char *val;
  if (!attr || !out_example)
    return 0;
  if (strncmp(attr, "example:", 8) != 0 && strncmp(attr, "example=", 8) != 0)
    return 0;
  val = (trim_segment((char *)(attr + 8), &_ast_trim_segment_14),
         _ast_trim_segment_14);
  if (!val || !*val)
    return 1;
  if (*out_example)
    free(*out_example);
  *out_example = (c_cdd_strdup(val, &_ast_strdup_7), _ast_strdup_7);
  return *out_example ? 1 : ENOMEM;
}

static int parse_deprecated_line(const char *line, const char *end,
                                 struct DocMetadata *out) {
  char *_ast_extract_rest_15;
  char *rest;
  int value = 1;

  if (!out)
    return EINVAL;

  out->deprecated_set = 1;
  rest = (extract_rest(line, end, &_ast_extract_rest_15), _ast_extract_rest_15);
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
  char *_ast_extract_word_16;
  char *_ast_extract_rest_17;
  const char *cur = line;
  char *url;
  char *desc;

  if (!out)
    return EINVAL;

  url = (extract_word(cur, end, &cur, &_ast_extract_word_16),
         _ast_extract_word_16);
  if (!url)
    return 0;

  if (out->external_docs_url)
    free(out->external_docs_url);
  out->external_docs_url = url;

  desc = (extract_rest(cur, end, &_ast_extract_rest_17), _ast_extract_rest_17);
  if (out->external_docs_description)
    free(out->external_docs_description);
  out->external_docs_description = desc;

  return 0;
}

static int parse_contact_line(const char *line, const char *end,
                              struct DocMetadata *out) {
  char *_ast_extract_rest_18;
  char *_ast_trim_segment_19;
  char *_ast_trim_segment_20;
  char *_ast_trim_segment_21;
  char *_ast_trim_segment_22;
  char *_ast_trim_segment_23;
  char *_ast_strdup_8 = NULL;
  char *_ast_strdup_9 = NULL;
  char *_ast_strdup_10 = NULL;
  char *_ast_strdup_11 = NULL;
  char *rest;
  char *name = NULL;
  char *url = NULL;
  char *email = NULL;
  char *cursor;
  char *open;

  if (!out)
    return EINVAL;

  rest = (extract_rest(line, end, &_ast_extract_rest_18), _ast_extract_rest_18);
  if (!rest)
    return 0;

  cursor = rest;
  while ((open = strchr(cursor, '[')) != NULL) {
    char *close = strchr(open, ']');
    char *attr;
    if (!close)
      break;
    *close = '\0';
    attr =
        (trim_segment(open + 1, &_ast_trim_segment_19), _ast_trim_segment_19);
    if (attr && *attr) {
      if (strncmp(attr, "name:", 5) == 0 || strncmp(attr, "name=", 5) == 0) {
        char *val = (trim_segment(attr + 5, &_ast_trim_segment_20),
                     _ast_trim_segment_20);
        if (val && *val) {
          if (name)
            free(name);
          name = (c_cdd_strdup(val, &_ast_strdup_8), _ast_strdup_8);
        }
      } else if (strncmp(attr, "url:", 4) == 0 ||
                 strncmp(attr, "url=", 4) == 0) {
        char *val = (trim_segment(attr + 4, &_ast_trim_segment_21),
                     _ast_trim_segment_21);
        if (val && *val) {
          if (url)
            free(url);
          url = (c_cdd_strdup(val, &_ast_strdup_9), _ast_strdup_9);
        }
      } else if (strncmp(attr, "email:", 6) == 0 ||
                 strncmp(attr, "email=", 6) == 0) {
        char *val = (trim_segment(attr + 6, &_ast_trim_segment_22),
                     _ast_trim_segment_22);
        if (val && *val) {
          if (email)
            free(email);
          email = (c_cdd_strdup(val, &_ast_strdup_10), _ast_strdup_10);
        }
      }
    }
    memset(open, ' ', (size_t)(close - open + 1));
    cursor = close + 1;
  }

  if (!name) {
    char *trimmed =
        (trim_segment(rest, &_ast_trim_segment_23), _ast_trim_segment_23);
    if (trimmed && *trimmed)
      name = (c_cdd_strdup(trimmed, &_ast_strdup_11), _ast_strdup_11);
  }

  free(rest);

  if (name) {
    if (out->contact_name)
      free(out->contact_name);
    out->contact_name = name;
  }
  if (url) {
    if (out->contact_url)
      free(out->contact_url);
    out->contact_url = url;
  }
  if (email) {
    if (out->contact_email)
      free(out->contact_email);
    out->contact_email = email;
  }

  return 0;
}

static int parse_license_line(const char *line, const char *end,
                              struct DocMetadata *out) {
  char *_ast_extract_rest_24;
  char *_ast_trim_segment_25;
  char *_ast_trim_segment_26;
  char *_ast_trim_segment_27;
  char *_ast_trim_segment_28;
  char *_ast_trim_segment_29;
  char *_ast_strdup_12 = NULL;
  char *_ast_strdup_13 = NULL;
  char *_ast_strdup_14 = NULL;
  char *_ast_strdup_15 = NULL;
  char *rest;
  char *name = NULL;
  char *url = NULL;
  char *identifier = NULL;
  char *cursor;
  char *open;

  if (!out)
    return EINVAL;

  rest = (extract_rest(line, end, &_ast_extract_rest_24), _ast_extract_rest_24);
  if (!rest)
    return 0;

  cursor = rest;
  while ((open = strchr(cursor, '[')) != NULL) {
    char *close = strchr(open, ']');
    char *attr;
    if (!close)
      break;
    *close = '\0';
    attr =
        (trim_segment(open + 1, &_ast_trim_segment_25), _ast_trim_segment_25);
    if (attr && *attr) {
      if (strncmp(attr, "name:", 5) == 0 || strncmp(attr, "name=", 5) == 0) {
        char *val = (trim_segment(attr + 5, &_ast_trim_segment_26),
                     _ast_trim_segment_26);
        if (val && *val) {
          if (name)
            free(name);
          name = (c_cdd_strdup(val, &_ast_strdup_12), _ast_strdup_12);
        }
      } else if (strncmp(attr, "identifier:", 11) == 0 ||
                 strncmp(attr, "identifier=", 11) == 0) {
        char *val = (trim_segment(attr + 11, &_ast_trim_segment_27),
                     _ast_trim_segment_27);
        if (val && *val) {
          if (identifier)
            free(identifier);
          identifier = (c_cdd_strdup(val, &_ast_strdup_13), _ast_strdup_13);
        }
      } else if (strncmp(attr, "url:", 4) == 0 ||
                 strncmp(attr, "url=", 4) == 0) {
        char *val = (trim_segment(attr + 4, &_ast_trim_segment_28),
                     _ast_trim_segment_28);
        if (val && *val) {
          if (url)
            free(url);
          url = (c_cdd_strdup(val, &_ast_strdup_14), _ast_strdup_14);
        }
      }
    }
    memset(open, ' ', (size_t)(close - open + 1));
    cursor = close + 1;
  }

  if (!name) {
    char *trimmed =
        (trim_segment(rest, &_ast_trim_segment_29), _ast_trim_segment_29);
    if (trimmed && *trimmed)
      name = (c_cdd_strdup(trimmed, &_ast_strdup_15), _ast_strdup_15);
  }

  free(rest);

  if (!name) {
    if (url)
      free(url);
    if (identifier)
      free(identifier);
    return EINVAL;
  }

  if (url && identifier) {
    free(name);
    free(url);
    free(identifier);
    return EINVAL;
  }

  if (out->license_name)
    free(out->license_name);
  out->license_name = name;
  if (url) {
    if (out->license_url)
      free(out->license_url);
    out->license_url = url;
  }
  if (identifier) {
    if (out->license_identifier)
      free(out->license_identifier);
    out->license_identifier = identifier;
  }

  return 0;
}

static int parse_response_header_line(const char *line, const char *end,
                                      struct DocMetadata *out) {
  char *_ast_extract_word_30;
  char *_ast_extract_word_31;
  const char *_ast_skip_ws_32;
  char *_ast_trim_segment_33;
  char *_ast_trim_segment_34;
  char *_ast_trim_segment_35;
  const char *_ast_skip_ws_36;
  char *_ast_extract_rest_37;
  char *_ast_strdup_16 = NULL;
  char *_ast_strdup_17 = NULL;
  char *_ast_strdup_18 = NULL;
  char *_ast_strdup_19 = NULL;
  struct DocResponseHeader *new_headers;
  struct DocResponseHeader *h;
  const char *cur = line;

  if (!out)
    return EINVAL;

  new_headers = (struct DocResponseHeader *)realloc(
      out->response_headers,
      (out->n_response_headers + 1) * sizeof(struct DocResponseHeader));
  if (!new_headers)
    return ENOMEM;
  out->response_headers = new_headers;
  h = &out->response_headers[out->n_response_headers];
  memset(h, 0, sizeof(*h));

  h->code = (extract_word(cur, end, &cur, &_ast_extract_word_30),
             _ast_extract_word_30);
  if (!h->code)
    return 0;

  h->name = (extract_word(cur, end, &cur, &_ast_extract_word_31),
             _ast_extract_word_31);
  if (!h->name) {
    free(h->code);
    h->code = NULL;
    return 0;
  }

  cur = (skip_ws(cur, &_ast_skip_ws_32), _ast_skip_ws_32);
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

        if (strncmp(attr, "type:", 5) == 0) {
          if (h->type)
            free(h->type);
          h->type = (c_cdd_strdup(attr + 5, &_ast_strdup_16), _ast_strdup_16);
        } else if (strncmp(attr, "format:", 7) == 0 ||
                   strncmp(attr, "format=", 7) == 0) {
          char *val = (trim_segment(attr + 7, &_ast_trim_segment_33),
                       _ast_trim_segment_33);
          if (val && *val) {
            if (h->format)
              free(h->format);
            h->format = (c_cdd_strdup(val, &_ast_strdup_17), _ast_strdup_17);
          }
        } else if (strncmp(attr, "contentType:", 12) == 0 ||
                   strncmp(attr, "contentType=", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_34),
                       _ast_trim_segment_34);
          if (val && *val) {
            if (h->content_type)
              free(h->content_type);
            h->content_type =
                (c_cdd_strdup(val, &_ast_strdup_18), _ast_strdup_18);
          }
        } else if (strncmp(attr, "content:", 8) == 0 ||
                   strncmp(attr, "content=", 8) == 0) {
          char *val = (trim_segment((char *)(attr + 8), &_ast_trim_segment_35),
                       _ast_trim_segment_35);
          if (val && *val) {
            if (h->content_type)
              free(h->content_type);
            h->content_type =
                (c_cdd_strdup(val, &_ast_strdup_19), _ast_strdup_19);
          }
        } else if (parse_optional_example_attr(attr, &h->example) == ENOMEM) {
          free(attr);
          return ENOMEM;
        } else {
          parse_optional_bool_attr(attr, "required", &h->required_set,
                                   &h->required);
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = (skip_ws(cur, &_ast_skip_ws_36), _ast_skip_ws_36);
    } else {
      break;
    }
  }

  h->description =
      (extract_rest(cur, end, &_ast_extract_rest_37), _ast_extract_rest_37);
  out->n_response_headers++;
  return 0;
}

static int parse_link_line(const char *line, const char *end,
                           struct DocMetadata *out) {
  char *_ast_extract_word_38;
  char *_ast_extract_word_39;
  const char *_ast_skip_ws_40;
  char *_ast_trim_segment_41;
  char *_ast_trim_segment_42;
  char *_ast_trim_segment_43;
  char *_ast_trim_segment_44;
  char *_ast_trim_segment_45;
  char *_ast_trim_segment_46;
  char *_ast_trim_segment_47;
  char *_ast_trim_segment_48;
  char *_ast_trim_segment_49;
  const char *_ast_skip_ws_50;
  char *_ast_extract_rest_51;
  char *_ast_strdup_20 = NULL;
  char *_ast_strdup_21 = NULL;
  char *_ast_strdup_22 = NULL;
  char *_ast_strdup_23 = NULL;
  char *_ast_strdup_24 = NULL;
  char *_ast_strdup_25 = NULL;
  char *_ast_strdup_26 = NULL;
  char *_ast_strdup_27 = NULL;
  char *_ast_strdup_28 = NULL;
  struct DocLink *new_links;
  struct DocLink *link;
  const char *cur = line;

  if (!out)
    return EINVAL;

  new_links =
      (struct DocLink *)realloc(out->links, (out->n_links + 1) * sizeof(*link));
  if (!new_links)
    return ENOMEM;
  out->links = new_links;
  link = &out->links[out->n_links];
  memset(link, 0, sizeof(*link));

  link->code = (extract_word(cur, end, &cur, &_ast_extract_word_38),
                _ast_extract_word_38);
  if (!link->code)
    return 0;

  link->name = (extract_word(cur, end, &cur, &_ast_extract_word_39),
                _ast_extract_word_39);
  if (!link->name) {
    free(link->code);
    link->code = NULL;
    return 0;
  }

  cur = (skip_ws(cur, &_ast_skip_ws_40), _ast_skip_ws_40);
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

        if (strncmp(attr, "operationId:", 12) == 0 ||
            strncmp(attr, "operationId=", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_41),
                       _ast_trim_segment_41);
          if (val && *val) {
            if (link->operation_id)
              free(link->operation_id);
            link->operation_id =
                (c_cdd_strdup(val, &_ast_strdup_20), _ast_strdup_20);
          }
        } else if (strncmp(attr, "operationRef:", 13) == 0 ||
                   strncmp(attr, "operationRef=", 13) == 0) {
          char *val = (trim_segment(attr + 13, &_ast_trim_segment_42),
                       _ast_trim_segment_42);
          if (val && *val) {
            if (link->operation_ref)
              free(link->operation_ref);
            link->operation_ref =
                (c_cdd_strdup(val, &_ast_strdup_21), _ast_strdup_21);
          }
        } else if (strncmp(attr, "parameters:", 11) == 0 ||
                   strncmp(attr, "parameters=", 11) == 0) {
          char *val = (trim_segment(attr + 11, &_ast_trim_segment_43),
                       _ast_trim_segment_43);
          if (val && *val) {
            if (link->parameters_json)
              free(link->parameters_json);
            link->parameters_json =
                (c_cdd_strdup(val, &_ast_strdup_22), _ast_strdup_22);
          }
        } else if (strncmp(attr, "requestBody:", 12) == 0 ||
                   strncmp(attr, "requestBody=", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_44),
                       _ast_trim_segment_44);
          if (val && *val) {
            if (link->request_body_json)
              free(link->request_body_json);
            link->request_body_json =
                (c_cdd_strdup(val, &_ast_strdup_23), _ast_strdup_23);
          }
        } else if (strncmp(attr, "summary:", 8) == 0 ||
                   strncmp(attr, "summary=", 8) == 0) {
          char *val = (trim_segment((char *)(attr + 8), &_ast_trim_segment_45),
                       _ast_trim_segment_45);
          if (val && *val) {
            if (link->summary)
              free(link->summary);
            link->summary =
                (c_cdd_strdup(val, &_ast_strdup_24), _ast_strdup_24);
          }
        } else if (strncmp(attr, "serverUrl:", 10) == 0 ||
                   strncmp(attr, "serverUrl=", 10) == 0) {
          char *val = (trim_segment(attr + 10, &_ast_trim_segment_46),
                       _ast_trim_segment_46);
          if (val && *val) {
            if (link->server_url)
              free(link->server_url);
            link->server_url =
                (c_cdd_strdup(val, &_ast_strdup_25), _ast_strdup_25);
          }
        } else if (strncmp(attr, "serverName:", 11) == 0 ||
                   strncmp(attr, "serverName=", 11) == 0) {
          char *val = (trim_segment(attr + 11, &_ast_trim_segment_47),
                       _ast_trim_segment_47);
          if (val && *val) {
            if (link->server_name)
              free(link->server_name);
            link->server_name =
                (c_cdd_strdup(val, &_ast_strdup_26), _ast_strdup_26);
          }
        } else if (strncmp(attr, "serverDescription:", 18) == 0 ||
                   strncmp(attr, "serverDescription=", 18) == 0) {
          char *val = (trim_segment(attr + 18, &_ast_trim_segment_48),
                       _ast_trim_segment_48);
          if (val && *val) {
            if (link->server_description)
              free(link->server_description);
            link->server_description =
                (c_cdd_strdup(val, &_ast_strdup_27), _ast_strdup_27);
          }
        } else if (strncmp(attr, "description:", 12) == 0 ||
                   strncmp(attr, "description=", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_49),
                       _ast_trim_segment_49);
          if (val && *val) {
            if (link->description)
              free(link->description);
            link->description =
                (c_cdd_strdup(val, &_ast_strdup_28), _ast_strdup_28);
          }
        }

        free(attr);
      }
      cur = close_bracket + 1;
      cur = (skip_ws(cur, &_ast_skip_ws_50), _ast_skip_ws_50);
    } else {
      break;
    }
  }

  if (!link->description) {
    link->description =
        (extract_rest(cur, end, &_ast_extract_rest_51), _ast_extract_rest_51);
  }

  out->n_links++;
  return 0;
}

/* --- Core Logic --- */

int doc_metadata_init(struct DocMetadata *meta) {
  if (!meta)
    return EINVAL;
  memset(meta, 0, sizeof(*meta));
  return 0;
}

void doc_metadata_free(struct DocMetadata *meta) {
  size_t i;
  if (!meta)
    return;

  if (meta->route)
    free(meta->route);
  if (meta->verb)
    free(meta->verb);
  if (meta->operation_id)
    free(meta->operation_id);
  if (meta->json_schema_dialect)
    free(meta->json_schema_dialect);
  if (meta->summary)
    free(meta->summary);
  if (meta->description)
    free(meta->description);
  if (meta->info_title)
    free(meta->info_title);
  if (meta->info_version)
    free(meta->info_version);
  if (meta->info_summary)
    free(meta->info_summary);
  if (meta->info_description)
    free(meta->info_description);
  if (meta->terms_of_service)
    free(meta->terms_of_service);
  if (meta->contact_name)
    free(meta->contact_name);
  if (meta->contact_url)
    free(meta->contact_url);
  if (meta->contact_email)
    free(meta->contact_email);
  if (meta->license_name)
    free(meta->license_name);
  if (meta->license_identifier)
    free(meta->license_identifier);
  if (meta->license_url)
    free(meta->license_url);
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
      if (meta->params[i].format)
        free(meta->params[i].format);
      if (meta->params[i].content_type)
        free(meta->params[i].content_type);
      if (meta->params[i].example)
        free(meta->params[i].example);
    }
    free(meta->params);
  }

  if (meta->returns) {
    for (i = 0; i < meta->n_returns; ++i) {
      free(meta->returns[i].code);
      if (meta->returns[i].summary)
        free(meta->returns[i].summary);
      free(meta->returns[i].description);
      if (meta->returns[i].content_type)
        free(meta->returns[i].content_type);
      if (meta->returns[i].example)
        free(meta->returns[i].example);
    }
    free(meta->returns);
  }

  if (meta->response_headers) {
    for (i = 0; i < meta->n_response_headers; ++i) {
      free(meta->response_headers[i].code);
      free(meta->response_headers[i].name);
      free(meta->response_headers[i].type);
      if (meta->response_headers[i].format)
        free(meta->response_headers[i].format);
      if (meta->response_headers[i].content_type)
        free(meta->response_headers[i].content_type);
      free(meta->response_headers[i].description);
      if (meta->response_headers[i].example)
        free(meta->response_headers[i].example);
    }
    free(meta->response_headers);
  }

  if (meta->links) {
    for (i = 0; i < meta->n_links; ++i) {
      struct DocLink *link = &meta->links[i];
      free(link->code);
      free(link->name);
      if (link->operation_id)
        free(link->operation_id);
      if (link->operation_ref)
        free(link->operation_ref);
      if (link->summary)
        free(link->summary);
      if (link->description)
        free(link->description);
      if (link->parameters_json)
        free(link->parameters_json);
      if (link->request_body_json)
        free(link->request_body_json);
      if (link->server_url)
        free(link->server_url);
      if (link->server_name)
        free(link->server_name);
      if (link->server_description)
        free(link->server_description);
    }
    free(meta->links);
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

  if (meta->security_schemes) {
    for (i = 0; i < meta->n_security_schemes; ++i) {
      size_t f;
      struct DocSecurityScheme *sch = &meta->security_schemes[i];
      free(sch->name);
      if (sch->description)
        free(sch->description);
      if (sch->scheme)
        free(sch->scheme);
      if (sch->bearer_format)
        free(sch->bearer_format);
      if (sch->param_name)
        free(sch->param_name);
      if (sch->open_id_connect_url)
        free(sch->open_id_connect_url);
      if (sch->oauth2_metadata_url)
        free(sch->oauth2_metadata_url);
      if (sch->flows) {
        for (f = 0; f < sch->n_flows; ++f) {
          size_t s;
          struct DocOAuthFlow *flow = &sch->flows[f];
          if (flow->authorization_url)
            free(flow->authorization_url);
          if (flow->token_url)
            free(flow->token_url);
          if (flow->refresh_url)
            free(flow->refresh_url);
          if (flow->device_authorization_url)
            free(flow->device_authorization_url);
          if (flow->scopes) {
            for (s = 0; s < flow->n_scopes; ++s) {
              free(flow->scopes[s].name);
              if (flow->scopes[s].description)
                free(flow->scopes[s].description);
            }
            free(flow->scopes);
          }
        }
        free(sch->flows);
      }
    }
    free(meta->security_schemes);
  }

  if (meta->servers) {
    for (i = 0; i < meta->n_servers; ++i) {
      size_t v;
      free(meta->servers[i].url);
      free(meta->servers[i].name);
      free(meta->servers[i].description);
      if (meta->servers[i].variables) {
        for (v = 0; v < meta->servers[i].n_variables; ++v) {
          size_t e;
          struct DocServerVar *var = &meta->servers[i].variables[v];
          free(var->name);
          free(var->default_value);
          if (var->description)
            free(var->description);
          if (var->enum_values) {
            for (e = 0; e < var->n_enum_values; ++e) {
              free(var->enum_values[e]);
            }
            free(var->enum_values);
          }
        }
        free(meta->servers[i].variables);
      }
    }
    free(meta->servers);
  }

  if (meta->request_bodies) {
    for (i = 0; i < meta->n_request_bodies; ++i) {
      free(meta->request_bodies[i].content_type);
      free(meta->request_bodies[i].description);
      if (meta->request_bodies[i].example)
        free(meta->request_bodies[i].example);
    }
    free(meta->request_bodies);
  }

  if (meta->encodings) {
    for (i = 0; i < meta->n_encodings; ++i) {
      if (meta->encodings[i].name)
        free(meta->encodings[i].name);
      if (meta->encodings[i].content_type)
        free(meta->encodings[i].content_type);
    }
    free(meta->encodings);
  }

  if (meta->tag_meta) {
    for (i = 0; i < meta->n_tag_meta; ++i) {
      free(meta->tag_meta[i].name);
      free(meta->tag_meta[i].summary);
      free(meta->tag_meta[i].description);
      free(meta->tag_meta[i].parent);
      free(meta->tag_meta[i].kind);
      free(meta->tag_meta[i].external_docs_url);
      free(meta->tag_meta[i].external_docs_description);
    }
    free(meta->tag_meta);
  }

  if (meta->request_body_description)
    free(meta->request_body_description);
  if (meta->request_body_content_type)
    free(meta->request_body_content_type);

  memset(meta, 0, sizeof(*meta));
}

static int parse_param_line(const char *line, const char *end,
                            struct DocMetadata *out) {
  char *_ast_extract_word_52;
  const char *_ast_skip_ws_53;
  char *_ast_trim_segment_54;
  const char *_ast_skip_ws_55;
  char *_ast_extract_rest_56;
  char *_ast_strdup_29 = NULL;
  char *_ast_strdup_30 = NULL;
  char *_ast_strdup_31 = NULL;
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
  p->name = (extract_word(cur, end, &cur, &_ast_extract_word_52),
             _ast_extract_word_52);
  if (!p->name) {
    /* Malformed param line, ignore but don't crash */
    return 0;
  }

  /* 2. Check for Attributes [key:val] or [required] */
  cur = (skip_ws(cur, &_ast_skip_ws_53), _ast_skip_ws_53);
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
          p->in_loc = (c_cdd_strdup(attr + 3, &_ast_strdup_29), _ast_strdup_29);
        } else if (strcmp(attr, "required") == 0) {
          p->required = 1;
        } else if (strncmp(attr, "contentType:", 12) == 0) {
          if (p->content_type)
            free(p->content_type);
          p->content_type =
              (c_cdd_strdup(attr + 12, &_ast_strdup_30), _ast_strdup_30);
        } else if (strncmp(attr, "format:", 7) == 0 ||
                   strncmp(attr, "format=", 7) == 0) {
          char *val = (trim_segment(attr + 7, &_ast_trim_segment_54),
                       _ast_trim_segment_54);
          if (val && *val) {
            if (p->format)
              free(p->format);
            p->format = (c_cdd_strdup(val, &_ast_strdup_31), _ast_strdup_31);
          }
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
          if (strcmp(attr, "itemSchema") == 0 ||
              strcmp(attr, "itemSchema:true") == 0 ||
              strcmp(attr, "itemSchema=true") == 0) {
            p->item_schema = 1;
          }
          parse_optional_bool_attr(attr, "deprecated", &p->deprecated_set,
                                   &p->deprecated);
          if (parse_optional_example_attr(attr, &p->example) == ENOMEM) {
            free(attr);
            return ENOMEM;
          }
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = (skip_ws(cur, &_ast_skip_ws_55), _ast_skip_ws_55);
    } else {
      break; /* Unbalanced */
    }
  }

  /* 3. Description */
  p->description =
      (extract_rest(cur, end, &_ast_extract_rest_56), _ast_extract_rest_56);

  out->n_params++;
  return 0;
}

static int parse_return_line(const char *line, const char *end,
                             struct DocMetadata *out) {
  char *_ast_extract_word_57;
  const char *_ast_skip_ws_58;
  char *_ast_trim_segment_59;
  char *_ast_trim_segment_60;
  const char *_ast_skip_ws_61;
  char *_ast_extract_rest_62;
  char *_ast_strdup_32 = NULL;
  char *_ast_strdup_33 = NULL;
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
  r->code = (extract_word(cur, end, &cur, &_ast_extract_word_57),
             _ast_extract_word_57);
  if (!r->code) {
    return 0;
  }

  /* 2. Optional Attributes [key:val] */
  cur = (skip_ws(cur, &_ast_skip_ws_58), _ast_skip_ws_58);
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

        if (strncmp(attr, "contentType:", 12) == 0 ||
            strncmp(attr, "contentType=", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_59),
                       _ast_trim_segment_59);
          if (val && *val) {
            if (r->content_type)
              free(r->content_type);
            r->content_type =
                (c_cdd_strdup(val, &_ast_strdup_32), _ast_strdup_32);
          }
        } else if (strncmp(attr, "summary:", 8) == 0 ||
                   strncmp(attr, "summary=", 8) == 0) {
          char *val = (trim_segment((char *)(attr + 8), &_ast_trim_segment_60),
                       _ast_trim_segment_60);
          if (val && *val) {
            if (r->summary)
              free(r->summary);
            r->summary = (c_cdd_strdup(val, &_ast_strdup_33), _ast_strdup_33);
          }
        } else if (strcmp(attr, "itemSchema") == 0 ||
                   strcmp(attr, "itemSchema:true") == 0 ||
                   strcmp(attr, "itemSchema=true") == 0) {
          r->item_schema = 1;
        } else if (parse_optional_example_attr(attr, &r->example) == ENOMEM) {
          free(attr);
          return ENOMEM;
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = (skip_ws(cur, &_ast_skip_ws_61), _ast_skip_ws_61);
    } else {
      break; /* Unbalanced */
    }
  }

  /* 3. Description */
  r->description =
      (extract_rest(cur, end, &_ast_extract_rest_62), _ast_extract_rest_62);

  out->n_returns++;
  return 0;
}

static int split_scopes(const char *input, char ***out_scopes,
                        size_t *out_count) {
  char *_ast_trim_segment_63;
  char *_ast_strdup_34 = NULL;
  char *_ast_strdup_35 = NULL;
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

  buf = (c_cdd_strdup(input, &_ast_strdup_34), _ast_strdup_34);
  if (!buf)
    return ENOMEM;

#ifdef _WIN32
  token = strtok_s(buf, ", \t", &saveptr);
#else
  token = strtok_r(buf, ", \t", &saveptr);
#endif
  while (token) {
    char *trimmed =
        (trim_segment(token, &_ast_trim_segment_63), _ast_trim_segment_63);
    char **new_scopes;
    if (!trimmed || !*trimmed) {
#ifdef _WIN32
      token = strtok_s(NULL, ", \t", &saveptr);
#else
      token = strtok_r(NULL, ", \t", &saveptr);
#endif
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
    scopes[n] = (c_cdd_strdup(trimmed, &_ast_strdup_35), _ast_strdup_35);
    if (!scopes[n]) {
      size_t i;
      for (i = 0; i < n; ++i)
        free(scopes[i]);
      free(scopes);
      free(buf);
      return ENOMEM;
    }
    n++;
#ifdef _WIN32
    token = strtok_s(NULL, ", \t", &saveptr);
#else
    token = strtok_r(NULL, ", \t", &saveptr);
#endif
  }

  free(buf);
  *out_scopes = scopes;
  *out_count = n;
  return 0;
}

static int parse_security_line(const char *line, const char *end,
                               struct DocMetadata *out) {
  char *_ast_extract_word_64;
  char *_ast_extract_rest_65;
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

  scheme = (extract_word(cur, end, &cur, &_ast_extract_word_64),
            _ast_extract_word_64);
  if (!scheme)
    return 0;

  rest = (extract_rest(cur, end, &_ast_extract_rest_65), _ast_extract_rest_65);
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

static int parse_security_type_text(const char *text,
                                    enum DocSecurityType *_out_val) {
  if (!text) {
    *_out_val = DOC_SEC_UNSET;
    return 0;
  }
  if (strcmp(text, "apiKey") == 0) {
    *_out_val = DOC_SEC_APIKEY;
    return 0;
  }
  if (strcmp(text, "http") == 0) {
    *_out_val = DOC_SEC_HTTP;
    return 0;
  }
  if (strcmp(text, "mutualTLS") == 0) {
    *_out_val = DOC_SEC_MUTUALTLS;
    return 0;
  }
  if (strcmp(text, "oauth2") == 0) {
    *_out_val = DOC_SEC_OAUTH2;
    return 0;
  }
  if (strcmp(text, "openIdConnect") == 0) {
    *_out_val = DOC_SEC_OPENID;
    return 0;
  }
  {
    *_out_val = DOC_SEC_UNSET;
    return 0;
  }
}

static int parse_security_in_text(const char *text,
                                  enum DocSecurityIn *_out_val) {
  if (!text) {
    *_out_val = DOC_SEC_IN_UNSET;
    return 0;
  }
  if (strcmp(text, "query") == 0) {
    *_out_val = DOC_SEC_IN_QUERY;
    return 0;
  }
  if (strcmp(text, "header") == 0) {
    *_out_val = DOC_SEC_IN_HEADER;
    return 0;
  }
  if (strcmp(text, "cookie") == 0) {
    *_out_val = DOC_SEC_IN_COOKIE;
    return 0;
  }
  {
    *_out_val = DOC_SEC_IN_UNSET;
    return 0;
  }
}

static int parse_oauth_flow_type_text(const char *text,
                                      enum DocOAuthFlowType *_out_val) {
  if (!text) {
    *_out_val = DOC_OAUTH_FLOW_UNSET;
    return 0;
  }
  if (strcmp(text, "implicit") == 0) {
    *_out_val = DOC_OAUTH_FLOW_IMPLICIT;
    return 0;
  }
  if (strcmp(text, "password") == 0) {
    *_out_val = DOC_OAUTH_FLOW_PASSWORD;
    return 0;
  }
  if (strcmp(text, "clientCredentials") == 0) {
    *_out_val = DOC_OAUTH_FLOW_CLIENT_CREDENTIALS;
    return 0;
  }
  if (strcmp(text, "authorizationCode") == 0) {
    *_out_val = DOC_OAUTH_FLOW_AUTHORIZATION_CODE;
    return 0;
  }
  if (strcmp(text, "deviceAuthorization") == 0) {
    *_out_val = DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION;
    return 0;
  }
  {
    *_out_val = DOC_OAUTH_FLOW_UNSET;
    return 0;
  }
}

static int parse_oauth_scopes(const char *input, struct DocOAuthScope **out,
                              size_t *out_count) {
  char **names = NULL;
  size_t n = 0;
  size_t i;
  struct DocOAuthScope *scopes;
  int rc;

  if (!out || !out_count)
    return EINVAL;
  *out = NULL;
  *out_count = 0;

  rc = split_scopes(input, &names, &n);
  if (rc != 0)
    return rc;
  if (n == 0)
    return 0;

  scopes = (struct DocOAuthScope *)calloc(n, sizeof(struct DocOAuthScope));
  if (!scopes) {
    for (i = 0; i < n; ++i)
      free(names[i]);
    free(names);
    return ENOMEM;
  }

  for (i = 0; i < n; ++i) {
    scopes[i].name = names[i];
    scopes[i].description = NULL;
    names[i] = NULL;
  }
  free(names);
  *out = scopes;
  *out_count = n;
  return 0;
}

static int parse_security_scheme_line(const char *line, const char *end,
                                      struct DocMetadata *out) {
  char *_ast_extract_word_66;
  const char *_ast_skip_ws_67;
  char *_ast_trim_segment_68;
  enum DocSecurityType _ast_parse_security_type_text_69;
  char *_ast_trim_segment_70;
  char *_ast_trim_segment_71;
  char *_ast_trim_segment_72;
  char *_ast_trim_segment_73;
  char *_ast_trim_segment_74;
  enum DocSecurityIn _ast_parse_security_in_text_75;
  char *_ast_trim_segment_76;
  char *_ast_trim_segment_77;
  char *_ast_trim_segment_78;
  enum DocOAuthFlowType _ast_parse_oauth_flow_type_text_79;
  char *_ast_trim_segment_80;
  char *_ast_trim_segment_81;
  char *_ast_trim_segment_82;
  char *_ast_trim_segment_83;
  char *_ast_trim_segment_84;
  const char *_ast_skip_ws_85;
  char *_ast_strdup_36 = NULL;
  char *_ast_strdup_37 = NULL;
  char *_ast_strdup_38 = NULL;
  char *_ast_strdup_39 = NULL;
  char *_ast_strdup_40 = NULL;
  char *_ast_strdup_41 = NULL;
  char *_ast_strdup_42 = NULL;
  char *_ast_strdup_43 = NULL;
  char *_ast_strdup_44 = NULL;
  char *_ast_strdup_45 = NULL;
  struct DocSecurityScheme *new_schemes;
  struct DocSecurityScheme *scheme;
  const char *cur = line;
  struct DocOAuthFlow *current_flow = NULL;

  if (!out)
    return EINVAL;

  new_schemes = (struct DocSecurityScheme *)realloc(
      out->security_schemes,
      (out->n_security_schemes + 1) * sizeof(struct DocSecurityScheme));
  if (!new_schemes)
    return ENOMEM;
  out->security_schemes = new_schemes;
  scheme = &out->security_schemes[out->n_security_schemes];
  memset(scheme, 0, sizeof(*scheme));
  scheme->type = DOC_SEC_UNSET;
  scheme->in = DOC_SEC_IN_UNSET;

  scheme->name = (extract_word(cur, end, &cur, &_ast_extract_word_66),
                  _ast_extract_word_66);
  if (!scheme->name)
    return 0;

  cur = (skip_ws(cur, &_ast_skip_ws_67), _ast_skip_ws_67);
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

        if (strncmp(attr, "type:", 5) == 0 || strncmp(attr, "type=", 5) == 0) {
          char *val = (trim_segment(attr + 5, &_ast_trim_segment_68),
                       _ast_trim_segment_68);
          scheme->type =
              (parse_security_type_text(val, &_ast_parse_security_type_text_69),
               _ast_parse_security_type_text_69);
        } else if (strncmp(attr, "description:", 12) == 0 ||
                   strncmp(attr, "description=", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_70),
                       _ast_trim_segment_70);
          if (val && *val) {
            if (scheme->description)
              free(scheme->description);
            scheme->description =
                (c_cdd_strdup(val, &_ast_strdup_36), _ast_strdup_36);
          }
        } else if (strncmp(attr, "scheme:", 7) == 0 ||
                   strncmp(attr, "scheme=", 7) == 0) {
          char *val = (trim_segment(attr + 7, &_ast_trim_segment_71),
                       _ast_trim_segment_71);
          if (val && *val) {
            if (scheme->scheme)
              free(scheme->scheme);
            scheme->scheme =
                (c_cdd_strdup(val, &_ast_strdup_37), _ast_strdup_37);
          }
        } else if (strncmp(attr, "bearerFormat:", 13) == 0 ||
                   strncmp(attr, "bearerFormat=", 13) == 0) {
          char *val = (trim_segment(attr + 13, &_ast_trim_segment_72),
                       _ast_trim_segment_72);
          if (val && *val) {
            if (scheme->bearer_format)
              free(scheme->bearer_format);
            scheme->bearer_format =
                (c_cdd_strdup(val, &_ast_strdup_38), _ast_strdup_38);
          }
        } else if (strncmp(attr, "paramName:", 10) == 0 ||
                   strncmp(attr, "paramName=", 10) == 0) {
          char *val = (trim_segment(attr + 10, &_ast_trim_segment_73),
                       _ast_trim_segment_73);
          if (val && *val) {
            if (scheme->param_name)
              free(scheme->param_name);
            scheme->param_name =
                (c_cdd_strdup(val, &_ast_strdup_39), _ast_strdup_39);
          }
        } else if (strncmp(attr, "in:", 3) == 0 ||
                   strncmp(attr, "in=", 3) == 0) {
          char *val = (trim_segment(attr + 3, &_ast_trim_segment_74),
                       _ast_trim_segment_74);
          scheme->in =
              (parse_security_in_text(val, &_ast_parse_security_in_text_75),
               _ast_parse_security_in_text_75);
        } else if (strncmp(attr, "openIdConnectUrl:", 17) == 0 ||
                   strncmp(attr, "openIdConnectUrl=", 17) == 0) {
          char *val = (trim_segment(attr + 17, &_ast_trim_segment_76),
                       _ast_trim_segment_76);
          if (val && *val) {
            if (scheme->open_id_connect_url)
              free(scheme->open_id_connect_url);
            scheme->open_id_connect_url =
                (c_cdd_strdup(val, &_ast_strdup_40), _ast_strdup_40);
          }
        } else if (strncmp(attr, "oauth2MetadataUrl:", 18) == 0 ||
                   strncmp(attr, "oauth2MetadataUrl=", 18) == 0) {
          char *val = (trim_segment(attr + 18, &_ast_trim_segment_77),
                       _ast_trim_segment_77);
          if (val && *val) {
            if (scheme->oauth2_metadata_url)
              free(scheme->oauth2_metadata_url);
            scheme->oauth2_metadata_url =
                (c_cdd_strdup(val, &_ast_strdup_41), _ast_strdup_41);
          }
        } else if (strncmp(attr, "flow:", 5) == 0 ||
                   strncmp(attr, "flow=", 5) == 0) {
          char *val = (trim_segment(attr + 5, &_ast_trim_segment_78),
                       _ast_trim_segment_78);
          enum DocOAuthFlowType flow_type =
              (parse_oauth_flow_type_text(val,
                                          &_ast_parse_oauth_flow_type_text_79),
               _ast_parse_oauth_flow_type_text_79);
          if (flow_type != DOC_OAUTH_FLOW_UNSET) {
            struct DocOAuthFlow *new_flows = (struct DocOAuthFlow *)realloc(
                scheme->flows,
                (scheme->n_flows + 1) * sizeof(struct DocOAuthFlow));
            if (new_flows) {
              scheme->flows = new_flows;
              current_flow = &scheme->flows[scheme->n_flows];
              memset(current_flow, 0, sizeof(*current_flow));
              current_flow->type = flow_type;
              scheme->n_flows++;
              if (scheme->type == DOC_SEC_UNSET)
                scheme->type = DOC_SEC_OAUTH2;
            }
          }
        } else if (strncmp(attr, "authorizationUrl:", 17) == 0 ||
                   strncmp(attr, "authorizationUrl=", 17) == 0) {
          char *val = (trim_segment(attr + 17, &_ast_trim_segment_80),
                       _ast_trim_segment_80);
          if (current_flow && val && *val) {
            if (current_flow->authorization_url)
              free(current_flow->authorization_url);
            current_flow->authorization_url =
                (c_cdd_strdup(val, &_ast_strdup_42), _ast_strdup_42);
          }
        } else if (strncmp(attr, "tokenUrl:", 9) == 0 ||
                   strncmp(attr, "tokenUrl=", 9) == 0) {
          char *val = (trim_segment(attr + 9, &_ast_trim_segment_81),
                       _ast_trim_segment_81);
          if (current_flow && val && *val) {
            if (current_flow->token_url)
              free(current_flow->token_url);
            current_flow->token_url =
                (c_cdd_strdup(val, &_ast_strdup_43), _ast_strdup_43);
          }
        } else if (strncmp(attr, "refreshUrl:", 11) == 0 ||
                   strncmp(attr, "refreshUrl=", 11) == 0) {
          char *val = (trim_segment(attr + 11, &_ast_trim_segment_82),
                       _ast_trim_segment_82);
          if (current_flow && val && *val) {
            if (current_flow->refresh_url)
              free(current_flow->refresh_url);
            current_flow->refresh_url =
                (c_cdd_strdup(val, &_ast_strdup_44), _ast_strdup_44);
          }
        } else if (strncmp(attr, "deviceAuthorizationUrl:", 23) == 0 ||
                   strncmp(attr, "deviceAuthorizationUrl=", 23) == 0) {
          char *val = (trim_segment(attr + 23, &_ast_trim_segment_83),
                       _ast_trim_segment_83);
          if (current_flow && val && *val) {
            if (current_flow->device_authorization_url)
              free(current_flow->device_authorization_url);
            current_flow->device_authorization_url =
                (c_cdd_strdup(val, &_ast_strdup_45), _ast_strdup_45);
          }
        } else if (strncmp(attr, "scopes:", 7) == 0 ||
                   strncmp(attr, "scopes=", 7) == 0) {
          char *val = (trim_segment(attr + 7, &_ast_trim_segment_84),
                       _ast_trim_segment_84);
          if (current_flow && val) {
            struct DocOAuthScope *scopes = NULL;
            size_t n_scopes = 0;
            if (parse_oauth_scopes(val, &scopes, &n_scopes) == 0) {
              size_t i;
              for (i = 0; i < current_flow->n_scopes; ++i) {
                free(current_flow->scopes[i].name);
                free(current_flow->scopes[i].description);
              }
              free(current_flow->scopes);
              current_flow->scopes = scopes;
              current_flow->n_scopes = n_scopes;
            }
          }
        } else {
          parse_optional_bool_attr(attr, "deprecated", &scheme->deprecated_set,
                                   &scheme->deprecated);
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = (skip_ws(cur, &_ast_skip_ws_85), _ast_skip_ws_85);
    } else {
      break;
    }
  }

  out->n_security_schemes++;
  return 0;
}

static int find_key_token(char *s, const char *key, size_t *key_len,
                          char **_out_val) {
  char *p;
  size_t klen;
  if (!s || !key) {
    *_out_val = NULL;
    return 0;
  }
  klen = strlen(key);
  p = strstr(s, key);
  while (p) {
    if ((p == s || isspace((unsigned char)p[-1])) &&
        (p[klen] == '=' || p[klen] == ':')) {
      if (key_len)
        *key_len = klen + 1;
      {
        *_out_val = p;
        return 0;
      }
    }
    p = strstr(p + klen, key);
  }
  {
    *_out_val = NULL;
    return 0;
  }
}

static int parse_server_line(const char *line, const char *end,
                             struct DocMetadata *out) {
  char *_ast_extract_word_86;
  char *_ast_extract_rest_87;
  char *_ast_find_key_token_88;
  char *_ast_find_key_token_89;
  char *_ast_trim_segment_90;
  char *_ast_trim_segment_91;
  char *_ast_trim_segment_92;
  char *_ast_strdup_46 = NULL;
  char *_ast_strdup_47 = NULL;
  char *_ast_strdup_48 = NULL;
  const char *cur = line;
  char *url;
  char *rest;
  char *name = NULL;
  char *desc = NULL;
  struct DocServer *new_servers;
  struct DocServer *srv;

  if (!out)
    return EINVAL;

  url = (extract_word(cur, end, &cur, &_ast_extract_word_86),
         _ast_extract_word_86);
  if (!url)
    return 0;

  rest = (extract_rest(cur, end, &_ast_extract_rest_87), _ast_extract_rest_87);
  if (rest && *rest) {
    size_t name_key_len = 0;
    size_t desc_key_len = 0;
    char *name_key =
        (find_key_token(rest, "name", &name_key_len, &_ast_find_key_token_88),
         _ast_find_key_token_88);
    char *desc_key = (find_key_token(rest, "description", &desc_key_len,
                                     &_ast_find_key_token_89),
                      _ast_find_key_token_89);

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
        char *trimmed = (trim_segment(name_start, &_ast_trim_segment_90),
                         _ast_trim_segment_90);
        if (trimmed && *trimmed)
          name = (c_cdd_strdup(trimmed, &_ast_strdup_46), _ast_strdup_46);
      }
      *name_end = saved;
    }
    if (desc_key) {
      char *desc_start = desc_key + desc_key_len;
      char *trimmed = (trim_segment(desc_start, &_ast_trim_segment_91),
                       _ast_trim_segment_91);
      if (trimmed && *trimmed)
        desc = (c_cdd_strdup(trimmed, &_ast_strdup_47), _ast_strdup_47);
    }
    if (!name_key && !desc_key) {
      char *trimmed =
          (trim_segment(rest, &_ast_trim_segment_92), _ast_trim_segment_92);
      if (trimmed && *trimmed)
        desc = (c_cdd_strdup(trimmed, &_ast_strdup_48), _ast_strdup_48);
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

static int split_scopes(const char *input, char ***out_scopes,
                        size_t *out_count);

static int split_enum_values(const char *input, char ***out_vals,
                             size_t *out_count) {
  char *_ast_strdup_49 = NULL;
  char *buf;
  size_t i;
  int rc;

  if (!out_vals || !out_count)
    return EINVAL;
  *out_vals = NULL;
  *out_count = 0;
  if (!input || !*input)
    return 0;

  buf = (c_cdd_strdup(input, &_ast_strdup_49), _ast_strdup_49);
  if (!buf)
    return ENOMEM;
  for (i = 0; buf[i]; ++i) {
    if (buf[i] == '|')
      buf[i] = ',';
  }
  rc = split_scopes(buf, out_vals, out_count);
  free(buf);
  return rc;
}

static int parse_server_var_line(const char *line, const char *end,
                                 struct DocMetadata *out) {
  char *_ast_extract_word_93;
  const char *_ast_skip_ws_94;
  char *_ast_trim_segment_95;
  char *_ast_trim_segment_96;
  char *_ast_trim_segment_97;
  const char *_ast_skip_ws_98;
  char *_ast_extract_rest_99;
  char *_ast_strdup_50 = NULL;
  char *_ast_strdup_51 = NULL;
  char *_ast_strdup_52 = NULL;
  const char *cur = line;
  char *name = NULL;
  char *description = NULL;
  char *default_value = NULL;
  char *enum_raw = NULL;

  if (!out || out->n_servers == 0)
    return EINVAL;

  name = (extract_word(cur, end, &cur, &_ast_extract_word_93),
          _ast_extract_word_93);
  if (!name)
    return 0;

  cur = (skip_ws(cur, &_ast_skip_ws_94), _ast_skip_ws_94);
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

        if (strncmp(attr, "default:", 8) == 0 ||
            strncmp(attr, "default=", 8) == 0) {
          char *val = (trim_segment((char *)(attr + 8), &_ast_trim_segment_95),
                       _ast_trim_segment_95);
          if (val && *val) {
            if (default_value)
              free(default_value);
            default_value =
                (c_cdd_strdup(val, &_ast_strdup_50), _ast_strdup_50);
          }
        } else if (strncmp(attr, "enum:", 5) == 0 ||
                   strncmp(attr, "enum=", 5) == 0) {
          char *val = (trim_segment(attr + 5, &_ast_trim_segment_96),
                       _ast_trim_segment_96);
          if (val && *val) {
            if (enum_raw)
              free(enum_raw);
            enum_raw = (c_cdd_strdup(val, &_ast_strdup_51), _ast_strdup_51);
          }
        } else if (strncmp(attr, "description:", 12) == 0 ||
                   strncmp(attr, "description=", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_97),
                       _ast_trim_segment_97);
          if (val && *val) {
            if (description)
              free(description);
            description = (c_cdd_strdup(val, &_ast_strdup_52), _ast_strdup_52);
          }
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = (skip_ws(cur, &_ast_skip_ws_98), _ast_skip_ws_98);
    } else {
      break;
    }
  }

  if (!description) {
    char *rest =
        (extract_rest(cur, end, &_ast_extract_rest_99), _ast_extract_rest_99);
    if (rest && *rest) {
      description = rest;
    } else if (rest) {
      free(rest);
    }
  }

  if (!default_value) {
    free(name);
    if (description)
      free(description);
    if (enum_raw)
      free(enum_raw);
    return EINVAL;
  }

  {
    struct DocServer *srv = &out->servers[out->n_servers - 1];
    struct DocServerVar *new_vars = (struct DocServerVar *)realloc(
        srv->variables, (srv->n_variables + 1) * sizeof(struct DocServerVar));
    struct DocServerVar *var;
    if (!new_vars) {
      free(name);
      free(default_value);
      if (description)
        free(description);
      if (enum_raw)
        free(enum_raw);
      return ENOMEM;
    }
    srv->variables = new_vars;
    var = &srv->variables[srv->n_variables];
    memset(var, 0, sizeof(*var));
    var->name = name;
    var->default_value = default_value;
    var->description = description;
    if (enum_raw) {
      if (split_enum_values(enum_raw, &var->enum_values, &var->n_enum_values) !=
          0) {
        free(enum_raw);
        return ENOMEM;
      }
      free(enum_raw);
    }
    srv->n_variables++;
  }

  return 0;
}

static int parse_encoding_line(const char *line, const char *end,
                               struct DocMetadata *out, int kind) {
  const char *_ast_skip_ws_100;
  char *_ast_extract_rest_101;
  const char *_ast_skip_ws_102;
  char *_ast_extract_rest_103;
  char *_ast_trim_segment_104;
  char *_ast_trim_segment_105;
  const char *_ast_skip_ws_106;
  char *_ast_strdup_53 = NULL;
  const char *cur = line;
  struct DocEncoding *new_arr;
  struct DocEncoding *entry;

  if (!out)
    return EINVAL;

  new_arr = (struct DocEncoding *)realloc(
      out->encodings, (out->n_encodings + 1) * sizeof(struct DocEncoding));
  if (!new_arr)
    return ENOMEM;
  out->encodings = new_arr;
  entry = &out->encodings[out->n_encodings];
  memset(entry, 0, sizeof(*entry));
  entry->kind = kind;

  cur = (skip_ws(cur, &_ast_skip_ws_100), _ast_skip_ws_100);

  if (kind == 0) {
    /* Need to parse property name */
    const char *name_end = cur;
    while (name_end < end && !isspace((unsigned char)*name_end) &&
           *name_end != '[') {
      name_end++;
    }
    if (name_end > cur) {
      entry->name = (extract_rest(cur, name_end, &_ast_extract_rest_101),
                     _ast_extract_rest_101);
      if (!entry->name)
        return ENOMEM;
    }
    cur = name_end;
    cur = (skip_ws(cur, &_ast_skip_ws_102), _ast_skip_ws_102);
  }

  while (cur < end && *cur == '[') {
    const char *close_bracket = strchr(cur, ']');
    if (close_bracket && close_bracket < end) {
      char *attr =
          (extract_rest(cur + 1, close_bracket, &_ast_extract_rest_103),
           _ast_extract_rest_103);
      if (attr) {
        if (strncmp(attr, "contentType:", 12) == 0 ||
            strncmp(attr, "contentType=", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_104),
                       _ast_trim_segment_104);
          if (val && *val) {
            entry->content_type =
                (c_cdd_strdup(val, &_ast_strdup_53), _ast_strdup_53);
          }
        } else if (strncmp(attr, "style:", 6) == 0 ||
                   strncmp(attr, "style=", 6) == 0) {
          char *val = (trim_segment(attr + 6, &_ast_trim_segment_105),
                       _ast_trim_segment_105);
          if (val && *val) {
            enum DocParamStyle style = DOC_PARAM_STYLE_UNSET;
            if (parse_style_text(val, &style)) {
              entry->style = style;
              entry->style_set = 1;
            }
          }
        } else {
          parse_optional_bool_attr(attr, "explode", &entry->explode_set,
                                   &entry->explode);
          parse_optional_bool_attr(attr, "allowReserved",
                                   &entry->allow_reserved_set,
                                   &entry->allow_reserved);
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = (skip_ws(cur, &_ast_skip_ws_106), _ast_skip_ws_106);
    } else {
      break;
    }
  }

  out->n_encodings++;
  return 0;
}
static int parse_request_body_line(const char *line, const char *end,
                                   struct DocMetadata *out) {
  const char *_ast_skip_ws_107;
  char *_ast_trim_segment_108;
  char *_ast_trim_segment_109;
  const char *_ast_skip_ws_110;
  char *_ast_extract_rest_111;
  char *_ast_strdup_54 = NULL;
  char *_ast_strdup_55 = NULL;
  char *_ast_strdup_56 = NULL;
  char *_ast_strdup_57 = NULL;
  const char *cur = line;
  struct DocRequestBody *new_arr;
  struct DocRequestBody *entry;
  char *content_type = NULL;
  char *description = NULL;
  char *example = NULL;
  int required_set = 0;
  int required_val = 0;
  int item_schema = 0;

  if (!out)
    return EINVAL;

  cur = (skip_ws(cur, &_ast_skip_ws_107), _ast_skip_ws_107);
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

        parse_optional_bool_attr(attr, "required", &required_set,
                                 &required_val);
        if (strncmp(attr, "contentType:", 12) == 0 ||
            strncmp(attr, "contentType=", 12) == 0) {
          char *val = (trim_segment(attr + 12, &_ast_trim_segment_108),
                       _ast_trim_segment_108);
          if (val && *val) {
            if (content_type)
              free(content_type);
            content_type = (c_cdd_strdup(val, &_ast_strdup_54), _ast_strdup_54);
          }
        } else if (strncmp(attr, "content:", 8) == 0 ||
                   strncmp(attr, "content=", 8) == 0) {
          char *val = (trim_segment((char *)(attr + 8), &_ast_trim_segment_109),
                       _ast_trim_segment_109);
          if (val && *val) {
            if (content_type)
              free(content_type);
            content_type = (c_cdd_strdup(val, &_ast_strdup_55), _ast_strdup_55);
          }
        } else if (strcmp(attr, "itemSchema") == 0 ||
                   strcmp(attr, "itemSchema:true") == 0 ||
                   strcmp(attr, "itemSchema=true") == 0) {
          item_schema = 1;
        } else if (parse_optional_example_attr(attr, &example) == ENOMEM) {
          free(attr);
          if (content_type)
            free(content_type);
          if (example)
            free(example);
          return ENOMEM;
        }

        free(attr);
      }
      cur = close_bracket + 1;
      cur = (skip_ws(cur, &_ast_skip_ws_110), _ast_skip_ws_110);
    } else {
      break;
    }
  }

  description =
      (extract_rest(cur, end, &_ast_extract_rest_111), _ast_extract_rest_111);

  new_arr = (struct DocRequestBody *)realloc(out->request_bodies,
                                             (out->n_request_bodies + 1) *
                                                 sizeof(struct DocRequestBody));
  if (!new_arr) {
    if (content_type)
      free(content_type);
    if (description)
      free(description);
    if (example)
      free(example);
    return ENOMEM;
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
      free(out->request_body_content_type);
    out->request_body_content_type =
        (c_cdd_strdup(entry->content_type, &_ast_strdup_56), _ast_strdup_56);
    if (!out->request_body_content_type)
      return ENOMEM;
  }
  if (entry->description) {
    if (out->request_body_description)
      free(out->request_body_description);
    out->request_body_description =
        (c_cdd_strdup(entry->description, &_ast_strdup_57), _ast_strdup_57);
    if (!out->request_body_description)
      return ENOMEM;
  }
  return 0;
}

static int parse_route_line(const char *line, const char *end,
                            struct DocMetadata *out) {
  char *_ast_extract_word_112;
  char *_ast_extract_word_113;
  const char *cur = line;
  char *word1 = (extract_word(cur, end, &cur, &_ast_extract_word_112),
                 _ast_extract_word_112);
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
    word2 = (extract_word(cur, end, &cur, &_ast_extract_word_113),
             _ast_extract_word_113);
    if (word2) {
      if (out->route)
        free(out->route);
      out->route = word2;
    }
  }
  return 0;
}

int doc_parse_block(const char *comment, struct DocMetadata *out) {
  char *_ast_extract_rest_114;
  char *_ast_extract_rest_115;
  char *_ast_extract_rest_116;
  char *_ast_extract_rest_117;
  char *_ast_extract_rest_118;
  char *_ast_extract_rest_119;
  char *_ast_extract_rest_120;
  char *_ast_extract_rest_121;
  char *_ast_extract_rest_122;
  const char *p = comment;
  int rc = 0;

  if (!comment || !out)
    return EINVAL;

  /* Skip initial marker if present (e.g. block or line comments) via simple
   * scan loop
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
          if (rc == 0)
            out->is_webhook = 0;
        } else if (strcmp(cmd, "webhook") == 0) {
          rc = parse_route_line(cmd_end, line_end, out);
          if (rc == 0)
            out->is_webhook = 1;
        } else if (strcmp(cmd, "param") == 0) {
          rc = parse_param_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "return") == 0 || strcmp(cmd, "returns") == 0) {
          rc = parse_return_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "responseHeader") == 0 ||
                   strcmp(cmd, "responseheader") == 0) {
          rc = parse_response_header_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "link") == 0) {
          rc = parse_link_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "summary") == 0 || strcmp(cmd, "brief") == 0) {
          if (out->summary)
            free(out->summary);
          out->summary =
              (extract_rest(cmd_end, line_end, &_ast_extract_rest_114),
               _ast_extract_rest_114);
        } else if (strcmp(cmd, "operationId") == 0 ||
                   strcmp(cmd, "operationid") == 0) {
          if (out->operation_id)
            free(out->operation_id);
          out->operation_id =
              (extract_rest(cmd_end, line_end, &_ast_extract_rest_115),
               _ast_extract_rest_115);
        } else if (strcmp(cmd, "description") == 0 ||
                   strcmp(cmd, "details") == 0) {
          if (out->description)
            free(out->description);
          out->description =
              (extract_rest(cmd_end, line_end, &_ast_extract_rest_116),
               _ast_extract_rest_116);
        } else if (strcmp(cmd, "tag") == 0 || strcmp(cmd, "tags") == 0) {
          rc = parse_tags_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "tagMeta") == 0 || strcmp(cmd, "tagmeta") == 0) {
          rc = parse_tag_meta_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "deprecated") == 0) {
          rc = parse_deprecated_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "externalDocs") == 0 ||
                   strcmp(cmd, "externaldocs") == 0) {
          rc = parse_external_docs_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "security") == 0) {
          rc = parse_security_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "securityScheme") == 0 ||
                   strcmp(cmd, "securityscheme") == 0) {
          rc = parse_security_scheme_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "server") == 0) {
          rc = parse_server_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "serverVar") == 0 ||
                   strcmp(cmd, "servervar") == 0) {
          rc = parse_server_var_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "requestBody") == 0 ||
                   strcmp(cmd, "requestbody") == 0) {
          rc = parse_request_body_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "encoding") == 0) {
          rc = parse_encoding_line(cmd_end, line_end, out, 0);
        } else if (strcmp(cmd, "prefixEncoding") == 0 ||
                   strcmp(cmd, "prefixencoding") == 0) {
          rc = parse_encoding_line(cmd_end, line_end, out, 1);
        } else if (strcmp(cmd, "itemEncoding") == 0 ||
                   strcmp(cmd, "itemencoding") == 0) {
          rc = parse_encoding_line(cmd_end, line_end, out, 2);
        } else if (strcmp(cmd, "jsonSchemaDialect") == 0 ||
                   strcmp(cmd, "jsonschemadialect") == 0) {
          if (out->json_schema_dialect)
            free(out->json_schema_dialect);
          out->json_schema_dialect =
              (extract_rest(cmd_end, line_end, &_ast_extract_rest_117),
               _ast_extract_rest_117);
        } else if (strcmp(cmd, "infoTitle") == 0 ||
                   strcmp(cmd, "infotitle") == 0) {
          if (out->info_title)
            free(out->info_title);
          out->info_title =
              (extract_rest(cmd_end, line_end, &_ast_extract_rest_118),
               _ast_extract_rest_118);
        } else if (strcmp(cmd, "infoVersion") == 0 ||
                   strcmp(cmd, "infoversion") == 0) {
          if (out->info_version)
            free(out->info_version);
          out->info_version =
              (extract_rest(cmd_end, line_end, &_ast_extract_rest_119),
               _ast_extract_rest_119);
        } else if (strcmp(cmd, "infoSummary") == 0 ||
                   strcmp(cmd, "infosummary") == 0) {
          if (out->info_summary)
            free(out->info_summary);
          out->info_summary =
              (extract_rest(cmd_end, line_end, &_ast_extract_rest_120),
               _ast_extract_rest_120);
        } else if (strcmp(cmd, "infoDescription") == 0 ||
                   strcmp(cmd, "infodescription") == 0) {
          if (out->info_description)
            free(out->info_description);
          out->info_description =
              (extract_rest(cmd_end, line_end, &_ast_extract_rest_121),
               _ast_extract_rest_121);
        } else if (strcmp(cmd, "termsOfService") == 0 ||
                   strcmp(cmd, "termsofservice") == 0) {
          if (out->terms_of_service)
            free(out->terms_of_service);
          out->terms_of_service =
              (extract_rest(cmd_end, line_end, &_ast_extract_rest_122),
               _ast_extract_rest_122);
        } else if (strcmp(cmd, "contact") == 0) {
          rc = parse_contact_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "license") == 0) {
          rc = parse_license_line(cmd_end, line_end, out);
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
