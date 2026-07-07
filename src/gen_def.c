/* clang-format off */
#include "url_utils.h"
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef USE_WININET
#include "http_wininet.h"
#elif defined(USE_WINHTTP)
#include "http_winhttp.h"
#elif defined(__APPLE__)
#include "http_apple.h"
#else
#include "http_curl.h"
#endif

#include "gen_def.h"
/* clang-format on */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

/**
 * @brief Auto-generated code from OpenAPI specification
 */
enum cdd_c_error ApiError_cleanup(struct ApiError *err) {
  if (!err)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (err->type)
    free(err->type);
  if (err->title)
    free(err->title);
  if (err->detail)
    free(err->detail);
  if (err->instance)
    free(err->instance);
  if (err->raw_body)
    free(err->raw_body);
  free(err);
  return CDD_C_SUCCESS;
  return CDD_C_SUCCESS;
}

/**
 * @brief Auto-generated code from OpenAPI specification
 */
static enum cdd_c_error ApiError_from_json(const char *json,
                                           struct ApiError **out) {
  JSON_Value *root;
  JSON_Object *obj;
  if (!json || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = calloc(1, sizeof(struct ApiError));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  (*out)->raw_body = strdup(json);
  root = json_parse_string(json);
  if (!root)
    return CDD_C_SUCCESS; /* Not JSON, return strict success but object only has
                             raw_body */
  obj = json_value_get_object(root);
  if (obj) {
    if (json_object_has_value(obj, "type")) {
      (*out)->type = strdup(json_object_get_string(obj, "type"));
      if (!(*out)->type) {
        json_value_free(root);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (json_object_has_value(obj, "title")) {
      (*out)->title = strdup(json_object_get_string(obj, "title"));
      if (!(*out)->title) {
        json_value_free(root);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (json_object_has_value(obj, "detail")) {
      (*out)->detail = strdup(json_object_get_string(obj, "detail"));
      if (!(*out)->detail) {
        json_value_free(root);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (json_object_has_value(obj, "instance")) {
      (*out)->instance = strdup(json_object_get_string(obj, "instance"));
      if (!(*out)->instance) {
        json_value_free(root);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (json_object_has_value(obj, "status"))
      (*out)->status = (int)json_object_get_number(obj, "status");
  }
  json_value_free(root);
  return CDD_C_SUCCESS;
}

enum cdd_c_error init(struct HttpClient *client, const char *base_url) {
  int rc;
  if (!client)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = http_client_init(client);
  if (rc != 0)
    return rc;
  const char *default_url = "/";
  if (!base_url || base_url[0] == '\0') {
    base_url = default_url;
  }
  if (base_url) {
    client->base_url = malloc(strlen(base_url) + 1);
    if (!client->base_url)
      return CDD_C_ERROR_MEMORY;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    strcpy_s(client->base_url, strlen(base_url) + 1, base_url);
#else
    strcpy(client->base_url, base_url);
#endif
  }
#ifdef USE_WININET
  rc = http_wininet_context_init(&client->transport);
  client->send = http_wininet_send;
#elif defined(USE_WINHTTP)
  rc = http_winhttp_context_init(&client->transport);
  client->send = http_winhttp_send;
#elif defined(__APPLE__)
  rc = http_apple_context_init(&client->transport);
  client->send = http_apple_send;
#else /* Default to Libcurl */
  rc = http_curl_context_init(&client->transport);
  client->send = http_curl_send;
#endif
  return rc;
}

enum cdd_c_error cleanup(struct HttpClient *client) {
  if (!client)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef USE_WININET
  http_wininet_context_free(client->transport);
#elif defined(USE_WINHTTP)
  http_winhttp_context_free(client->transport);
#elif defined(__APPLE__)
  http_apple_context_free(client->transport);
#else
  http_curl_context_free(client->transport);
#endif
  http_client_free(client);
  return CDD_C_SUCCESS;
}

enum cdd_c_error test_op(struct HttpClient *ctx, struct ApiError **api_error) {
  struct HttpRequest req;
  struct HttpResponse *res = NULL;
  int rc = 0;
  int attempt = 0;
  char *url = NULL;
  if (api_error)
    *api_error = NULL;

  if (!ctx || !ctx->send)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = http_request_init(&req);
  if (rc != 0)
    return rc;

  if (asprintf(&url, "%s/test", ctx->base_url) == -1) {
    return CDD_C_ERROR_MEMORY;
  }
  req.url = url;
  req.method = HTTP_GET;

  do {
    if (attempt > 0) {
      /* Implement backoff delay here if needed */
    }
    rc = ctx->send(ctx->transport, &req, &res);
    attempt++;
  } while (rc != 0 && attempt <= ctx->config.retry_count);

  if (rc != 0)
    goto cleanup;
  if (!res) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  int handled = 0;
  switch (res->status_code) {
  case 200:
    handled = 1;
    break;
  default:
    break;
  }
  if (!handled) {
    rc = CDD_C_ERROR_IO;
    if (res->body && api_error) {
      enum cdd_c_error api_rc =
          ApiError_from_json((const char *)res->body, api_error);
      if (api_rc != CDD_C_SUCCESS) {
        C_CDD_LOG_DEBUG("Failed to parse ApiError: %d\n", api_rc);
      }
    }
  }

cleanup:
  http_request_free(&req);
  if (res) {
    http_response_free(res);
    free(res);
  }
  return rc;
}
