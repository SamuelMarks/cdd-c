/**
 * @file http_types.c
 * @brief Implementation of HTTP types lifecycle and Multipart Logic.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "functions/parse_http_types.h"
#include "functions/parse_str.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define sprintf_s_wrapper(buf, start, cap, ...)                                \
  sprintf_s(buf + start, cap - start, __VA_ARGS__)
#else
#define sprintf_s_wrapper(buf, start, cap, ...)                                \
  sprintf(buf + start, __VA_ARGS__)
#endif

int http_headers_init(struct HttpHeaders *const headers) {
  if (!headers)
    return EINVAL;
  headers->headers = NULL;
  headers->count = 0;
  headers->capacity = 0;
  return 0;
}

void http_headers_free(struct HttpHeaders *const headers) {
  size_t i;
  if (!headers)
    return;

  if (headers->headers) {
    for (i = 0; i < headers->count; ++i) {
      if (headers->headers[i].key)
        free(headers->headers[i].key);
      if (headers->headers[i].value)
        free(headers->headers[i].value);
    }
    free(headers->headers);
    headers->headers = NULL;
  }
  headers->count = 0;
  headers->capacity = 0;
}

int http_headers_add(struct HttpHeaders *const headers, const char *const key,
                     const char *const value) {
  if (!headers || !key || !value)
    return EINVAL;

  if (headers->count >= headers->capacity) {
    const size_t new_cap = (headers->capacity == 0) ? 8 : headers->capacity * 2;
    struct HttpHeader *new_arr = (struct HttpHeader *)realloc(
        headers->headers, new_cap * sizeof(struct HttpHeader));
    if (!new_arr)
      return ENOMEM;
    headers->headers = new_arr;
    headers->capacity = new_cap;
  }

  headers->headers[headers->count].key = c_cdd_strdup(key);
  if (!headers->headers[headers->count].key)
    return ENOMEM;

  headers->headers[headers->count].value = c_cdd_strdup(value);
  if (!headers->headers[headers->count].value) {
    free(headers->headers[headers->count].key);
    return ENOMEM;
  }

  headers->count++;
  return 0;
}

/* --- Multipart Implementation --- */

int http_parts_init(struct HttpParts *const parts) {
  if (!parts)
    return EINVAL;
  parts->parts = NULL;
  parts->count = 0;
  parts->capacity = 0;
  return 0;
}

void http_parts_free(struct HttpParts *const parts) {
  size_t i;
  if (!parts)
    return;
  if (parts->parts) {
    for (i = 0; i < parts->count; ++i) {
      if (parts->parts[i].name)
        free(parts->parts[i].name);
      if (parts->parts[i].filename)
        free(parts->parts[i].filename);
      if (parts->parts[i].content_type)
        free(parts->parts[i].content_type);
      http_headers_free(&parts->parts[i].headers);
      /* data ownership is typically external references for efficiency,
         but for safety in this generator we assume data pointers are managed
         by the source (e.g. read_to_file or literal).
         The generic part struct doesn't own data memory. */
    }
    free(parts->parts);
    parts->parts = NULL;
  }
  parts->count = 0;
  parts->capacity = 0;
}

int http_request_add_part(struct HttpRequest *const req, const char *const name,
                          const char *const filename,
                          const char *const content_type,
                          const void *const data, size_t data_len) {
  struct HttpParts *p;
  if (!req || !name || (!data && data_len > 0))
    return EINVAL;

  p = &req->parts;
  if (p->count >= p->capacity) {
    size_t new_cap = (p->capacity == 0) ? 4 : p->capacity * 2;
    struct HttpPart *new_arr =
        (struct HttpPart *)realloc(p->parts, new_cap * sizeof(struct HttpPart));
    if (!new_arr)
      return ENOMEM;
    p->parts = new_arr;
    p->capacity = new_cap;
  }

  /* Zero new slot */
  memset(&p->parts[p->count], 0, sizeof(struct HttpPart));
  if (http_headers_init(&p->parts[p->count].headers) != 0)
    return EINVAL;

  p->parts[p->count].name = c_cdd_strdup(name);
  if (!p->parts[p->count].name) {
    http_headers_free(&p->parts[p->count].headers);
    return ENOMEM;
  }

  if (filename) {
    p->parts[p->count].filename = c_cdd_strdup(filename);
    if (!p->parts[p->count].filename) {
      free(p->parts[p->count].name);
      http_headers_free(&p->parts[p->count].headers);
      return ENOMEM;
    }
  }

  if (content_type) {
    p->parts[p->count].content_type = c_cdd_strdup(content_type);
    if (!p->parts[p->count].content_type) {
      if (p->parts[p->count].filename)
        free(p->parts[p->count].filename);
      free(p->parts[p->count].name);
      http_headers_free(&p->parts[p->count].headers);
      return ENOMEM;
    }
  }

  /* Reference data, do not copy yet. Serialization will copy. */
  p->parts[p->count].data = (void *)data;
  p->parts[p->count].data_len = data_len;

  p->count++;
  return 0;
}

int http_request_add_part_header_last(struct HttpRequest *const req,
                                      const char *const key,
                                      const char *const value) {
  struct HttpPart *part;
  if (!req || !key || !value)
    return EINVAL;
  if (req->parts.count == 0)
    return EINVAL;
  part = &req->parts.parts[req->parts.count - 1];
  return http_headers_add(&part->headers, key, value);
}

int http_request_flatten_parts(struct HttpRequest *const req) {
  char boundary[64];
  size_t i;
  size_t estimated_size = 0;
  char *buffer = NULL;
  size_t pos = 0;

  if (!req || req->parts.count == 0)
    return 0; /* Nothing to flatten */

  /* 1. Generate Boundary */
  if (req->body) {
    /* Already have a body, parts would conflict. Fail or warn? */
    return EINVAL;
  }

  srand((unsigned int)time(NULL));
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(boundary, sizeof(boundary), "------------------------cddbound%08x",
            rand());
#else
  sprintf(boundary, "------------------------cddbound%08x", rand());
#endif

  /* 2. Calculate Size */
  for (i = 0; i < req->parts.count; ++i) {
    struct HttpPart *part = &req->parts.parts[i];
    size_t h;

    estimated_size += strlen(boundary) + 6; /* --bound\r\n */
    /* Content-Disposition: form-data; name="..." */
    estimated_size += 38 + strlen(part->name) + 4;
    if (part->filename) {
      /* ; filename="..." */
      estimated_size += 13 + strlen(part->filename) + 1;
    }
    estimated_size += 2; /* \r\n */

    if (part->content_type) {
      /* Content-Type: ...\r\n */
      estimated_size += 14 + strlen(part->content_type) + 2;
    } else if (part->filename) {
      /* Default to octet-stream for files */
      estimated_size +=
          14 + 24 + 2; /* Content-Type: application/octet-stream\r\n */
    }

    for (h = 0; h < part->headers.count; ++h) {
      const struct HttpHeader *hdr = &part->headers.headers[h];
      if (!hdr->key || !hdr->value)
        continue;
      estimated_size += strlen(hdr->key) + 2 + strlen(hdr->value) + 2;
    }

    estimated_size += 2; /* \r\n (end of headers) */
    estimated_size += part->data_len;
    estimated_size += 2; /* \r\n (end of part) */
  }
  estimated_size += strlen(boundary) + 8; /* --bound--\r\n\0 */

  /* 3. Build Buffer */
  buffer = (char *)malloc(estimated_size);
  if (!buffer)
    return ENOMEM;

  for (i = 0; i < req->parts.count; ++i) {
    struct HttpPart *part = &req->parts.parts[i];
    size_t h;
    int written;

    /* Boundary start */
    /* Note: Using sprintf directly is unsafe if we didn't pre-calc, assume size
     * is ok */
    /* Using safe wrapper logic */
    /* --boundary\r\n */
    written =
        sprintf_s_wrapper(buffer, pos, estimated_size, "--%s\r\n", boundary);
    pos += written;

    /* Header */
    written = sprintf_s_wrapper(buffer, pos, estimated_size,
                                "Content-Disposition: form-data; name=\"%s\"",
                                part->name);
    pos += written;

    if (part->filename) {
      written = sprintf_s_wrapper(buffer, pos, estimated_size,
                                  "; filename=\"%s\"", part->filename);
      pos += written;
    }
    written = sprintf_s_wrapper(buffer, pos, estimated_size, "\r\n");
    pos += written;

    /* Content-Type */
    if (part->content_type) {
      written = sprintf_s_wrapper(buffer, pos, estimated_size,
                                  "Content-Type: %s\r\n", part->content_type);
      pos += written;
    } else if (part->filename) {
      written = sprintf_s_wrapper(buffer, pos, estimated_size,
                                  "Content-Type: application/octet-stream\r\n");
      pos += written;
    }

    for (h = 0; h < part->headers.count; ++h) {
      const struct HttpHeader *hdr = &part->headers.headers[h];
      if (!hdr->key || !hdr->value)
        continue;
      written = sprintf_s_wrapper(buffer, pos, estimated_size, "%s: %s\r\n",
                                  hdr->key, hdr->value);
      pos += written;
    }

    /* End of headers */
    written = sprintf_s_wrapper(buffer, pos, estimated_size, "\r\n");
    pos += written;

    /* Data */
    if (part->data_len > 0 && part->data) {
      memcpy(buffer + pos, part->data, part->data_len);
      pos += part->data_len;
    }

    /* End of part */
    written = sprintf_s_wrapper(buffer, pos, estimated_size, "\r\n");
    pos += written;
  }

  /* Final Boundary */
  {
    int written =
        sprintf_s_wrapper(buffer, pos, estimated_size, "--%s--\r\n", boundary);
    pos += written;
  }

  /* 4. Update Request */
  req->body = buffer;
  req->body_len = pos;

  {
    char ct[128];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(ct, sizeof(ct), "multipart/form-data; boundary=%s", boundary);
#else
    sprintf(ct, "multipart/form-data; boundary=%s", boundary);
#endif
    /* If header exists, this adds a duplicate. Typically client code shouldn't
     * set CT if using parts. */
    http_headers_add(&req->headers, "Content-Type", ct);
  }

  /* 5. Clear parts logic to avoid double-process (though body takes precedence)
   */
  /* We don't free the parts struct itself as caller might own data, but we
   * effectively consumed it into body */

  return 0;
}

int http_config_init(struct HttpConfig *const config) {
  if (!config)
    return EINVAL;

  config->timeout_ms = 30000;
  config->verify_peer = 1;
  config->verify_host = 1;
  config->proxy_url = NULL;
  config->user_agent = c_cdd_strdup("c_cdd/0.1.0");

  /* Retry defaults */
  config->retry_count = 0;
  config->retry_policy = HTTP_RETRY_NONE;

  if (!config->user_agent)
    return ENOMEM;

  return 0;
}

void http_config_free(struct HttpConfig *const config) {
  if (!config)
    return;
  if (config->user_agent) {
    free(config->user_agent);
    config->user_agent = NULL;
  }
  if (config->proxy_url) {
    free(config->proxy_url);
    config->proxy_url = NULL;
  }
}

int http_client_init(struct HttpClient *const client) {
  if (!client)
    return EINVAL;

  memset(client, 0, sizeof(struct HttpClient));
  return http_config_init(&client->config);
}

void http_client_free(struct HttpClient *const client) {
  if (!client)
    return;

  http_config_free(&client->config);

  if (client->base_url) {
    free(client->base_url);
    client->base_url = NULL;
  }
}

int http_request_init(struct HttpRequest *const req) {
  if (!req)
    return EINVAL;
  req->url = NULL;
  req->method = HTTP_GET;
  req->body = NULL;
  req->body_len = 0;
  if (http_headers_init(&req->headers) != 0)
    return ENOMEM;
  if (http_parts_init(&req->parts) != 0) {
    http_headers_free(&req->headers);
    return ENOMEM;
  }
  return 0;
}

void http_request_free(struct HttpRequest *const req) {
  if (!req)
    return;
  if (req->url) {
    free(req->url);
    req->url = NULL;
  }
  if (req->body) {
    free(req->body);
    req->body = NULL;
  }
  http_headers_free(&req->headers);
  http_parts_free(&req->parts);
}

int http_request_set_auth_bearer(struct HttpRequest *const req,
                                 const char *const token) {
  char *val = NULL;
  int rc;
  size_t len;

  if (!req || !token)
    return EINVAL;

  len = strlen(token) + 8;
  val = (char *)malloc(len);
  if (!val)
    return ENOMEM;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(val, len, "Bearer %s", token);
#else
  sprintf(val, "Bearer %s", token);
#endif

  rc = http_headers_add(&req->headers, "Authorization", val);
  free(val);

  return rc;
}

int http_request_set_auth_basic(struct HttpRequest *const req,
                                const char *const token) {
  char *val = NULL;
  int rc;
  size_t len;

  if (!req || !token)
    return EINVAL;

  len = strlen(token) + 7;
  val = (char *)malloc(len);
  if (!val)
    return ENOMEM;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(val, len, "Basic %s", token);
#else
  sprintf(val, "Basic %s", token);
#endif

  rc = http_headers_add(&req->headers, "Authorization", val);
  free(val);

  return rc;
}

int http_response_init(struct HttpResponse *const res) {
  if (!res)
    return EINVAL;
  res->status_code = 0;
  res->body = NULL;
  res->body_len = 0;
  return http_headers_init(&res->headers);
}

void http_response_free(struct HttpResponse *const res) {
  if (!res)
    return;
  if (res->body) {
    free(res->body);
    res->body = NULL;
  }
  http_headers_free(&res->headers);
}

int http_response_save_to_file(const struct HttpResponse *const res,
                               const char *const path) {
  FILE *f = NULL;
  size_t written;

  if (!res || !path)
    return EINVAL;

  if (!res->body && res->body_len > 0)
    return EINVAL;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (fopen_s(&f, path, "wb") != 0)
    return EIO;
#else
  f = fopen(path, "wb");
  if (!f)
    return errno ? errno : EIO;
#endif

  if (res->body_len > 0 && res->body) {
    written = fwrite(res->body, 1, res->body_len, f);
    if (written != res->body_len) {
      fclose(f);
      return EIO;
    }
  }

  if (fclose(f) != 0)
    return EIO;

  return 0;
}
