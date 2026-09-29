/* clang-format off */
#include "c_cdd/memory.h"
/**
 * @file doc_tags.c
 * @brief Tag, server, license, and route documentation directive parsing.
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

cdd_c_error_t doc_add_tag(struct DocMetadata *out, const char *tag) {
  char **new_tags;
  cdd_c_error_t rc;

  if (!out || !tag) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  new_tags =
      (char **)C_CDD_REALLOC(out->tags, (out->n_tags + 1) * sizeof(char *));
  if (!new_tags) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  out->tags = new_tags;
  rc = c_cdd_strdup(tag, &out->tags[out->n_tags]);
  if (rc != CDD_C_SUCCESS)
    return CDD_C_ERROR_MEMORY;
  out->n_tags++;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_add_tag_meta(struct DocMetadata *out,
                               struct DocTagMeta *meta) {
  struct DocTagMeta *new_meta;
  if (!out)
    return CDD_C_SUCCESS;
  new_meta = (struct DocTagMeta *)C_CDD_REALLOC(
      out->tag_meta, (out->n_tag_meta + 1) * sizeof(struct DocTagMeta));
  if (!new_meta) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  out->tag_meta = new_meta;
  out->tag_meta[out->n_tag_meta] = *meta;
  out->n_tag_meta++;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_tags_line(const char *line, const char *end,
                                  struct DocMetadata *out) {
  char *rest = NULL;
  char *cursor;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  rc = doc_extract_rest(line, end, &rest);
  if (rc != CDD_C_SUCCESS || !rest)
    return rc;

  cursor = rest;
  while (*cursor) {
    char *comma = strchr(cursor, ',');
    char *tag;
    if (comma)
      *comma = '\0';

    rc = doc_trim_segment(cursor, &tag);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(rest);
      return rc;
    }
    if (*tag) {
      rc = doc_add_tag(out, tag);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(rest);
        return rc;
      }
    }

    if (comma)
      cursor = comma + 1;
    else
      break;
  }

  C_CDD_FREE(rest);
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_tag_meta_line(const char *line, const char *end,
                                      struct DocMetadata *out) {
  const char *cur = line;
  struct DocTagMeta meta;
  cdd_c_error_t rc;

  memset(&meta, 0, sizeof(meta));

  rc = doc_extract_word(cur, end, &cur, &meta.name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!meta.name)
    return CDD_C_SUCCESS;

  rc = doc_skip_ws(cur, &cur);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(meta.name);
    return rc;
  }

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

        if (strncmp(attr, "summary:", 8) == 0) {
          char *val = NULL;
          rc = doc_trim_segment((char *)(size_t)(attr + 8), &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            goto cleanup_meta;
          }
          if (*val) {
            if (meta.summary)
              C_CDD_FREE(meta.summary);
            rc = c_cdd_strdup(val, &meta.summary);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              goto cleanup_meta;
            }
          }
        } else if (strncmp(attr, "description:", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            goto cleanup_meta;
          }
          if (*val) {
            if (meta.description)
              C_CDD_FREE(meta.description);
            rc = c_cdd_strdup(val, &meta.description);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              goto cleanup_meta;
            }
          }
        } else if (strncmp(attr, "parent:", 7) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 7, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            goto cleanup_meta;
          }
          if (*val) {
            if (meta.parent)
              C_CDD_FREE(meta.parent);
            rc = c_cdd_strdup(val, &meta.parent);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              goto cleanup_meta;
            }
          }
        } else if (strncmp(attr, "kind:", 5) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 5, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            goto cleanup_meta;
          }
          if (*val) {
            if (meta.kind)
              C_CDD_FREE(meta.kind);
            rc = c_cdd_strdup(val, &meta.kind);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              goto cleanup_meta;
            }
          }
        } else if (strncmp(attr, "externalDocs:", 13) == 0 ||
                   strncmp(attr, "externalDocs=", 13) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 13, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            goto cleanup_meta;
          }
          if (*val) {
            if (meta.external_docs_url)
              C_CDD_FREE(meta.external_docs_url);
            rc = c_cdd_strdup(val, &meta.external_docs_url);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              goto cleanup_meta;
            }
          }
        } else if (strncmp(attr, "externalDocsDescription:", 24) == 0 ||
                   strncmp(attr, "externalDocsDescription=", 24) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 24, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            goto cleanup_meta;
          }
          if (*val) {
            if (meta.external_docs_description)
              C_CDD_FREE(meta.external_docs_description);
            rc = c_cdd_strdup(val, &meta.external_docs_description);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              goto cleanup_meta;
            }
          }
        }
        C_CDD_FREE(attr);
      }
      cur = close_bracket + 1;
      rc = doc_skip_ws(cur, &cur);
      if (rc != CDD_C_SUCCESS)
        goto cleanup_meta;
    } else {
      break;
    }
  }

  rc = doc_add_tag_meta(out, &meta);
  if (rc != CDD_C_SUCCESS) {
    goto cleanup_meta;
  }
  return CDD_C_SUCCESS;

cleanup_meta:
  C_CDD_FREE(meta.name);
  C_CDD_FREE(meta.summary);
  C_CDD_FREE(meta.description);
  C_CDD_FREE(meta.parent);
  C_CDD_FREE(meta.kind);
  C_CDD_FREE(meta.external_docs_url);
  C_CDD_FREE(meta.external_docs_description);
  return rc;
}

cdd_c_error_t doc_parse_deprecated_line(const char *line, const char *end,
                                        struct DocMetadata *out) {
  char *rest = NULL;
  int value = 1;
  cdd_c_error_t rc;

  out->deprecated_set = 1;
  rc = doc_extract_rest(line, end, &rest);
  if (rc != CDD_C_SUCCESS || !rest) {
    out->deprecated = 1;
    return CDD_C_SUCCESS;
  }
  if (doc_parse_bool_text(rest, &value) == CDD_C_SUCCESS)
    out->deprecated = value;
  else
    out->deprecated = 1;
  C_CDD_FREE(rest);
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_external_docs_line(const char *line, const char *end,
                                           struct DocMetadata *out) {
  const char *cur = line;
  char *url = NULL;
  char *desc = NULL;
  cdd_c_error_t rc;

  rc = doc_extract_word(cur, end, &cur, &url);
  if (rc != CDD_C_SUCCESS || !url)
    return rc;

  if (out->external_docs_url)
    C_CDD_FREE(out->external_docs_url);
  out->external_docs_url = url;

  rc = doc_extract_rest(cur, end, &desc);
  if (rc == CDD_C_SUCCESS && desc) {
    if (out->external_docs_description)
      C_CDD_FREE(out->external_docs_description);
    out->external_docs_description = desc;
  }

  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_contact_line(const char *line, const char *end,
                                     struct DocMetadata *out) {
  char *rest = NULL;
  char *name = NULL;
  char *url = NULL;
  char *email = NULL;
  char *cursor;
  char *open;
  cdd_c_error_t rc;

  rc = doc_extract_rest(line, end, &rest);
  if (rc != CDD_C_SUCCESS || !rest)
    return rc;

  cursor = rest;
  while ((open = strchr(cursor, '[')) != NULL) {
    char *close = strchr(open, ']');
    char *attr;
    if (!close)
      break;
    *close = '\0';
    rc = doc_trim_segment(open + 1, &attr);
    if (rc != CDD_C_SUCCESS)
      goto fail_contact;
    if (*attr) {
      if (strncmp(attr, "name:", 5) == 0 || strncmp(attr, "name=", 5) == 0) {
        char *val = NULL;
        rc = doc_trim_segment(attr + 5, &val);
        if (rc != CDD_C_SUCCESS)
          goto fail_contact;
        if (*val) {
          if (name)
            C_CDD_FREE(name);
          rc = c_cdd_strdup(val, &name);
          if (rc != CDD_C_SUCCESS)
            goto fail_contact;
        }
      } else if (strncmp(attr, "url:", 4) == 0 ||
                 strncmp(attr, "url=", 4) == 0) {
        char *val = NULL;
        rc = doc_trim_segment(attr + 4, &val);
        if (rc != CDD_C_SUCCESS)
          goto fail_contact;
        if (*val) {
          if (url)
            C_CDD_FREE(url);
          rc = c_cdd_strdup(val, &url);
          if (rc != CDD_C_SUCCESS)
            goto fail_contact;
        }
      } else if (strncmp(attr, "email:", 6) == 0 ||
                 strncmp(attr, "email=", 6) == 0) {
        char *val = NULL;
        rc = doc_trim_segment(attr + 6, &val);
        if (rc != CDD_C_SUCCESS)
          goto fail_contact;
        if (*val) {
          if (email)
            C_CDD_FREE(email);
          rc = c_cdd_strdup(val, &email);
          if (rc != CDD_C_SUCCESS)
            goto fail_contact;
        }
      }
    }
    memset(open, ' ', (size_t)(close - open + 1));
    cursor = close + 1;
  }

  if (!name) {
    char *trimmed = NULL;
    rc = doc_trim_segment(rest, &trimmed);
    if (rc != CDD_C_SUCCESS)
      goto fail_contact;
    if (*trimmed) {
      rc = c_cdd_strdup(trimmed, &name);
      if (rc != CDD_C_SUCCESS)
        goto fail_contact;
    }
  }

  C_CDD_FREE(rest);

  if (name) {
    if (out->contact_name)
      C_CDD_FREE(out->contact_name);
    out->contact_name = name;
  }
  if (url) {
    if (out->contact_url)
      C_CDD_FREE(out->contact_url);
    out->contact_url = url;
  }
  if (email) {
    if (out->contact_email)
      C_CDD_FREE(out->contact_email);
    out->contact_email = email;
  }

  return CDD_C_SUCCESS;

fail_contact:
  C_CDD_FREE(rest);
  C_CDD_FREE(name);
  C_CDD_FREE(url);
  C_CDD_FREE(email);
  return rc;
}

cdd_c_error_t doc_parse_license_line(const char *line, const char *end,
                                     struct DocMetadata *out) {
  char *rest = NULL;
  char *name = NULL;
  char *url = NULL;
  char *identifier = NULL;
  char *cursor;
  char *open;
  cdd_c_error_t rc;

  rc = doc_extract_rest(line, end, &rest);
  if (rc != CDD_C_SUCCESS || !rest)
    return rc;

  cursor = rest;
  while ((open = strchr(cursor, '[')) != NULL) {
    char *close = strchr(open, ']');
    char *attr;
    if (!close)
      break;
    *close = '\0';
    rc = doc_trim_segment(open + 1, &attr);
    if (rc != CDD_C_SUCCESS)
      goto fail_license;
    if (*attr) {
      if (strncmp(attr, "name:", 5) == 0 || strncmp(attr, "name=", 5) == 0) {
        char *val = NULL;
        rc = doc_trim_segment(attr + 5, &val);
        if (rc != CDD_C_SUCCESS)
          goto fail_license;
        if (*val) {
          if (name)
            C_CDD_FREE(name);
          rc = c_cdd_strdup(val, &name);
          if (rc != CDD_C_SUCCESS)
            goto fail_license;
        }
      } else if (strncmp(attr, "identifier:", 11) == 0 ||
                 strncmp(attr, "identifier=", 11) == 0) {
        char *val = NULL;
        rc = doc_trim_segment(attr + 11, &val);
        if (rc != CDD_C_SUCCESS)
          goto fail_license;
        if (*val) {
          if (identifier)
            C_CDD_FREE(identifier);
          rc = c_cdd_strdup(val, &identifier);
          if (rc != CDD_C_SUCCESS)
            goto fail_license;
        }
      } else if (strncmp(attr, "url:", 4) == 0 ||
                 strncmp(attr, "url=", 4) == 0) {
        char *val = NULL;
        rc = doc_trim_segment(attr + 4, &val);
        if (rc != CDD_C_SUCCESS)
          goto fail_license;
        if (*val) {
          if (url)
            C_CDD_FREE(url);
          rc = c_cdd_strdup(val, &url);
          if (rc != CDD_C_SUCCESS)
            goto fail_license;
        }
      }
    }
    memset(open, ' ', (size_t)(close - open + 1));
    cursor = close + 1;
  }

  if (!name) {
    char *trimmed = NULL;
    rc = doc_trim_segment(rest, &trimmed);
    if (rc != CDD_C_SUCCESS)
      goto fail_license;
    if (*trimmed) {
      rc = c_cdd_strdup(trimmed, &name);
      if (rc != CDD_C_SUCCESS)
        goto fail_license;
    }
  }

  C_CDD_FREE(rest);

  if (!name) {
    if (url)
      C_CDD_FREE(url);
    if (identifier)
      C_CDD_FREE(identifier);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (url && identifier) {
    C_CDD_FREE(name);
    C_CDD_FREE(url);
    C_CDD_FREE(identifier);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (out->license_name)
    C_CDD_FREE(out->license_name);
  out->license_name = name;
  if (url) {
    if (out->license_url)
      C_CDD_FREE(out->license_url);
    out->license_url = url;
  }
  if (identifier) {
    if (out->license_identifier)
      C_CDD_FREE(out->license_identifier);
    out->license_identifier = identifier;
  }

  return CDD_C_SUCCESS;

fail_license:
  C_CDD_FREE(rest);
  C_CDD_FREE(name);
  C_CDD_FREE(url);
  C_CDD_FREE(identifier);
  return rc;
}

cdd_c_error_t doc_parse_server_line(const char *line, const char *end,
                                    struct DocMetadata *out) {
  const char *cur = line;
  char *url = NULL;
  char *rest = NULL;
  char *name = NULL;
  char *desc = NULL;
  struct DocServer *new_servers;
  struct DocServer *srv;
  cdd_c_error_t rc;

  rc = doc_extract_word(cur, end, &cur, &url);
  if (rc != CDD_C_SUCCESS || !url)
    return rc;

  rc = doc_extract_rest(cur, end, &rest);
  if (rc == CDD_C_SUCCESS && rest) {
    size_t name_key_len = 0;
    size_t desc_key_len = 0;
    char *name_key = NULL;
    char *desc_key = NULL;

    rc = doc_find_key_token(rest, "name", &name_key_len, &name_key);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(url);
      C_CDD_FREE(rest);
      return rc;
    }
    rc = doc_find_key_token(rest, "description", &desc_key_len, &desc_key);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(url);
      C_CDD_FREE(rest);
      return rc;
    }

    if (name_key) {
      char *name_start = name_key + name_key_len;
      char *name_end = NULL;
      char saved;
      if (desc_key && desc_key > name_start) {
        name_end = desc_key;
      } else {
        name_end = name_start + strcspn(name_start, " 	");
      }
      saved = *name_end;
      *name_end = '\0';
      {
        char *trimmed = NULL;
        rc = doc_trim_segment(name_start, &trimmed);
        if (rc != CDD_C_SUCCESS) {
          *name_end = saved;
          C_CDD_FREE(url);
          C_CDD_FREE(rest);
          return rc;
        }
        if (*trimmed) {
          rc = c_cdd_strdup(trimmed, &name);
          if (rc != CDD_C_SUCCESS) {
            *name_end = saved;
            C_CDD_FREE(url);
            C_CDD_FREE(rest);
            return rc;
          }
        }
      }
      *name_end = saved;
    }
    if (desc_key) {
      char *desc_start = desc_key + desc_key_len;
      char *trimmed = NULL;
      rc = doc_trim_segment(desc_start, &trimmed);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(name);
        C_CDD_FREE(url);
        C_CDD_FREE(rest);
        return rc;
      }
      if (*trimmed) {
        rc = c_cdd_strdup(trimmed, &desc);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(name);
          C_CDD_FREE(url);
          C_CDD_FREE(rest);
          return rc;
        }
      }
    }
    if (!name_key && !desc_key) {
      char *trimmed = NULL;
      rc = doc_trim_segment(rest, &trimmed);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(url);
        C_CDD_FREE(rest);
        return rc;
      }
      rc = c_cdd_strdup(trimmed, &desc);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(url);
        C_CDD_FREE(rest);
        return rc;
      }
    }
  }

  new_servers = (struct DocServer *)C_CDD_REALLOC(
      out->servers, (out->n_servers + 1) * sizeof(struct DocServer));
  if (!new_servers) {
    C_CDD_FREE(url);
    C_CDD_FREE(name);
    C_CDD_FREE(desc);
    C_CDD_FREE(rest);
    return CDD_C_ERROR_MEMORY;
  }
  out->servers = new_servers;
  srv = &out->servers[out->n_servers];
  memset(srv, 0, sizeof(*srv));
  srv->url = url;
  srv->name = name;
  srv->description = desc;
  out->n_servers++;

  if (rest)
    C_CDD_FREE(rest);
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_server_var_line(const char *line, const char *end,
                                        struct DocMetadata *out) {
  const char *cur = line;
  char *name = NULL;
  char *description = NULL;
  char *default_value = NULL;
  char *enum_raw = NULL;
  cdd_c_error_t rc;

  if (out->n_servers == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = doc_extract_word(cur, end, &cur, &name);
  if (rc != CDD_C_SUCCESS || !name)
    return rc;

  rc = doc_skip_ws(cur, &cur);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(name);
    return rc;
  }

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

        if (strncmp(attr, "default:", 8) == 0 ||
            strncmp(attr, "default=", 8) == 0) {
          char *val = NULL;
          rc = doc_trim_segment((char *)(size_t)(attr + 8), &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            goto fail_server_var;
          }
          if (*val) {
            if (default_value)
              C_CDD_FREE(default_value);
            rc = c_cdd_strdup(val, &default_value);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              goto fail_server_var;
            }
          }
        } else if (strncmp(attr, "enum:", 5) == 0 ||
                   strncmp(attr, "enum=", 5) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 5, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            goto fail_server_var;
          }
          if (*val) {
            if (enum_raw)
              C_CDD_FREE(enum_raw);
            rc = c_cdd_strdup(val, &enum_raw);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              goto fail_server_var;
            }
          }
        } else if (strncmp(attr, "description:", 12) == 0 ||
                   strncmp(attr, "description=", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            goto fail_server_var;
          }
          if (*val) {
            if (description)
              C_CDD_FREE(description);
            rc = c_cdd_strdup(val, &description);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              goto fail_server_var;
            }
          }
        }
        C_CDD_FREE(attr);
      }
      cur = close_bracket + 1;
      rc = doc_skip_ws(cur, &cur);
      if (rc != CDD_C_SUCCESS)
        goto fail_server_var;
    } else {
      break;
    }
  }

  if (!description) {
    rc = doc_extract_rest(cur, end, &description);
    if (rc != CDD_C_SUCCESS)
      goto fail_server_var;
  }

  if (!default_value) {
    C_CDD_FREE(name);
    if (description)
      C_CDD_FREE(description);
    if (enum_raw)
      C_CDD_FREE(enum_raw);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  {
    struct DocServer *srv = &out->servers[out->n_servers - 1];
    struct DocServerVar *new_vars = (struct DocServerVar *)C_CDD_REALLOC(
        srv->variables, (srv->n_variables + 1) * sizeof(struct DocServerVar));
    struct DocServerVar *var;
    if (!new_vars) {
      C_CDD_FREE(name);
      C_CDD_FREE(default_value);
      if (description)
        C_CDD_FREE(description);
      if (enum_raw)
        C_CDD_FREE(enum_raw);
      return CDD_C_ERROR_MEMORY;
    }
    srv->variables = new_vars;
    var = &srv->variables[srv->n_variables];
    memset(var, 0, sizeof(*var));
    var->name = name;
    var->default_value = default_value;
    var->description = description;
    if (enum_raw) {
      if (doc_split_enum_values(enum_raw, &var->enum_values,
                                &var->n_enum_values) != CDD_C_SUCCESS) {
        C_CDD_FREE(enum_raw);
        C_CDD_FREE(var->name);
        C_CDD_FREE(var->default_value);
        if (var->description)
          C_CDD_FREE(var->description);
        return CDD_C_ERROR_MEMORY;
      }
      C_CDD_FREE(enum_raw);
    }
    srv->n_variables++;
  }

  return CDD_C_SUCCESS;

fail_server_var:
  C_CDD_FREE(name);
  C_CDD_FREE(default_value);
  C_CDD_FREE(description);
  C_CDD_FREE(enum_raw);
  return rc;
}

cdd_c_error_t doc_parse_encoding_line(const char *line, const char *end,
                                      struct DocMetadata *out, int kind) {
  cdd_c_error_t rc_opt;
  const char *cur = line;
  struct DocEncoding *new_arr;
  struct DocEncoding *entry;
  cdd_c_error_t rc;

  new_arr = (struct DocEncoding *)C_CDD_REALLOC(
      out->encodings, (out->n_encodings + 1) * sizeof(struct DocEncoding));
  if (!new_arr) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  out->encodings = new_arr;
  entry = &out->encodings[out->n_encodings];
  memset(entry, 0, sizeof(*entry));
  entry->kind = kind;

  rc = doc_skip_ws(cur, &cur);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (kind == 0) {
    /* Need to parse property name */
    const char *name_end = cur;
    while (name_end < end && !isspace((unsigned char)*name_end) &&
           *name_end != '[') {
      name_end++;
    }
    if (name_end > cur) {
      rc = doc_extract_rest(cur, name_end, &entry->name);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
    }
    cur = name_end;
    rc = doc_skip_ws(cur, &cur);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  while (cur < end && *cur == '[') {
    const char *close_bracket = strchr(cur, ']');
    if (close_bracket && close_bracket < end) {
      char *attr = NULL;
      rc = doc_extract_rest(cur + 1, close_bracket, &attr);
      if (rc == CDD_C_SUCCESS && attr) {
        if (strncmp(attr, "contentType:", 12) == 0 ||
            strncmp(attr, "contentType=", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (*val) {
            rc = c_cdd_strdup(val, &entry->content_type);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "style:", 6) == 0 ||
                   strncmp(attr, "style=", 6) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 6, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (*val) {
            enum DocParamStyle style = DOC_PARAM_STYLE_UNSET;
            if (doc_parse_style_text(val, &style) == CDD_C_SUCCESS) {
              entry->style = style;
              entry->style_set = 1;
            }
          }
        } else {
          rc_opt = doc_parse_optional_bool_attr(
              attr, "explode", &entry->explode_set, &entry->explode);
          if (rc_opt == CDD_C_ERROR_MEMORY) {
            C_CDD_FREE(attr);
            return rc_opt;
          }
          rc_opt = doc_parse_optional_bool_attr(attr, "allowReserved",
                                                &entry->allow_reserved_set,
                                                &entry->allow_reserved);
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

  out->n_encodings++;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_route_line(const char *line, const char *end,
                                   struct DocMetadata *out) {
  const char *cur = line;
  char *word1 = NULL;
  char *word2 = NULL;
  cdd_c_error_t rc;

  rc = doc_extract_word(cur, end, &cur, &word1);
  if (rc != CDD_C_SUCCESS || !word1)
    return rc;

  /* Check if word1 is a Verb or Path */
  /* Heuristic: Starts with / is path. Uppercase is verb. */
  if (word1[0] == '/') {
    /* No verb specified */
    if (out->route)
      C_CDD_FREE(out->route);
    out->route = word1;
  } else {
    /* Assume verb */
    if (out->verb)
      C_CDD_FREE(out->verb);
    out->verb = word1;

    /* Next word should be path */
    rc = doc_extract_word(cur, end, &cur, &word2);
    if (rc == CDD_C_SUCCESS && word2) {
      if (out->route)
        C_CDD_FREE(out->route);
      out->route = word2;
    }
  }
  return CDD_C_SUCCESS;
}
