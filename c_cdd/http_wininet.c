/**
 * @file http_wininet.c
 * @brief Implementation of the WinInet backend.
 *
 * Checks for multipart data (which should be flattened by the caller or
 * wrapper) prior to transmission.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#else
/* Stub definitions for non-Windows linting/compilation checks */
typedef void *HINTERNET;
typedef unsigned long DWORD;
typedef unsigned short WORD; /* INTERNET_PORT is usually WORD or int */
typedef int wchar_t;
#define INTERNET_DEFAULT_HTTP_PORT 80
#define INTERNET_DEFAULT_HTTPS_PORT 443
#define INTERNET_SCHEME_HTTP 1
#define INTERNET_SCHEME_HTTPS 2
typedef struct {
  DWORD dwStructSize;
  wchar_t *lpszScheme;
  DWORD dwSchemeLength;
  int nScheme;
  wchar_t *lpszHostName;
  DWORD dwHostNameLength;
  int nPort;
  wchar_t *lpszUserName;
  DWORD dwUserNameLength;
  wchar_t *lpszPassword;
  DWORD dwPasswordLength;
  wchar_t *lpszUrlPath;
  DWORD dwUrlPathLength;
  wchar_t *lpszExtraInfo;
  DWORD dwExtraInfoLength;
} URL_COMPONENTSW;
typedef URL_COMPONENTSW URL_COMPONENTS;
#endif

#include "fs.h" /* For ascii_to_wide helpers */
#include "http_wininet.h"
#include "str_utils.h"

#define CHECK_EINVAL(x)                                                        \
  do {                                                                         \
    if (!(x))                                                                  \
      return EINVAL;                                                           \
  } while (0)

/**
 * @brief Opaque context definition.
 * Holds the root internet session and cached security configuration.
 */
struct HttpTransportContext {
  HINTERNET hInternet;  /**< Root handle from InternetOpen */
  DWORD security_flags; /**< Flags to apply to requests (e.g. ignore cert) */
};

/* --- Internal Helpers --- */

#ifdef _WIN32
/**
 * @brief Safely close a WinInet handle and NULL it.
 */
static void safe_close_handle(HINTERNET *h) {
  if (h && *h) {
    InternetCloseHandle(*h);
    *h = NULL;
  }
}

/**
 * @brief Convert HttpMethod enum to wide string verb.
 */
static const wchar_t *method_to_wide(enum HttpMethod method) {
  switch (method) {
  case HTTP_GET:
    return L"GET";
  case HTTP_POST:
    return L"POST";
  case HTTP_PUT:
    return L"PUT";
  case HTTP_DELETE:
    return L"DELETE";
  case HTTP_HEAD:
    return L"HEAD";
  case HTTP_PATCH:
    return L"PATCH";
  case HTTP_OPTIONS:
    return L"OPTIONS";
  case HTTP_TRACE:
    return L"TRACE";
  case HTTP_CONNECT:
    return L"CONNECT";
  default:
    return L"GET";
  }
}

/**
 * @brief Construct raw header string block from HttpHeaders.
 * Returns a malloc'd wide string (Key: Value\r\n...) or NULL.
 */
static int headers_to_wide_block(const struct HttpHeaders *headers,
                                 wchar_t **out) {
  size_t i;
  size_t total_wchars = 0;
  wchar_t *buf, *p;

  *out = NULL;
  if (!headers || headers->count == 0)
    return 0;

  /* 1. Calculate Size */
  for (i = 0; i < headers->count; ++i) {
    /* Len = key + ": " + value + "\r\n" */
    /* Assume roughly 1 wchar per char. Detailed conversion follows. */
    total_wchars += strlen(headers->headers[i].key);
    total_wchars += strlen(headers->headers[i].value);
    total_wchars += 4; /* ": " and "\r\n" */
  }
  total_wchars += 1; /* Null term */

  buf = (wchar_t *)calloc(total_wchars, sizeof(wchar_t));
  if (!buf)
    return ENOMEM;

  p = buf;
  for (i = 0; i < headers->count; ++i) {
    size_t written = 0;
    /* Key */
    if (ascii_to_wide(headers->headers[i].key, p, total_wchars - (p - buf),
                      &written) != 0) {
      free(buf);
      return EIO;
    }
    p += written;
    wcscpy(p, L": ");
    p += 2;

    /* Value */
    if (ascii_to_wide(headers->headers[i].value, p, total_wchars - (p - buf),
                      &written) != 0) {
      free(buf);
      return EIO;
    }
    p += written;
    wcscpy(p, L"\r\n");
    p += 2;
  }

  *out = buf;
  return 0;
}
#endif /* _WIN32 */

/* --- Public API Implementation --- */

int http_wininet_global_init(void) { return 0; }

void http_wininet_global_cleanup(void) {}

int http_wininet_context_init(struct HttpTransportContext **const ctx) {
#ifdef _WIN32
  HINTERNET hInternet;
  DWORD flags = 0;

  CHECK_EINVAL(ctx);

  /* INTERNET_OPEN_TYPE_PRECONFIG: Use ID/system settings */
  hInternet = InternetOpenW(L"c_cdd/WinInet", INTERNET_OPEN_TYPE_PRECONFIG,
                            NULL, NULL, flags);

  if (!hInternet) {
    return EIO;
  }

  *ctx = (struct HttpTransportContext *)malloc(
      sizeof(struct HttpTransportContext));
  if (!*ctx) {
    InternetCloseHandle(hInternet);
    return ENOMEM;
  }

  (*ctx)->hInternet = hInternet;
  (*ctx)->security_flags = 0;

  return 0;
#else
  (void)ctx;
  return ENOTSUP;
#endif
}

void http_wininet_context_free(struct HttpTransportContext *const ctx) {
#ifdef _WIN32
  if (ctx) {
    if (ctx->hInternet)
      InternetCloseHandle(ctx->hInternet);
    free(ctx);
  }
#else
  (void)ctx;
#endif
}

int http_wininet_config_apply(struct HttpTransportContext *const ctx,
                              const struct HttpConfig *const config) {
#ifdef _WIN32
  DWORD timeout;
  CHECK_EINVAL(ctx);
  CHECK_EINVAL(ctx->hInternet);
  CHECK_EINVAL(config);

  /* Timeouts: WinInet uses milliseconds */
  timeout = (DWORD)config->timeout_ms;

  /* Apply to logical handle (Connect, Send, Receive) */
  if (!InternetSetOption(ctx->hInternet, INTERNET_OPTION_CONNECT_TIMEOUT,
                         &timeout, sizeof(timeout)))
    return EIO;
  if (!InternetSetOption(ctx->hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout,
                         sizeof(timeout)))
    return EIO;
  if (!InternetSetOption(ctx->hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT,
                         &timeout, sizeof(timeout)))
    return EIO;

  /* Cache Security Flags for HttpOpenRequest */
  ctx->security_flags = 0;
  if (!config->verify_peer) {
    ctx->security_flags |= INTERNET_FLAG_IGNORE_CERT_CN_INVALID;
    ctx->security_flags |= INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
  }
  if (!config->verify_host) {
    ctx->security_flags |= INTERNET_FLAG_IGNORE_CERT_CN_INVALID;
  }

  return 0;
#else
  (void)ctx;
  (void)config;
  return ENOTSUP;
#endif
}

int http_wininet_send(struct HttpTransportContext *const ctx,
                      const struct HttpRequest *const req,
                      struct HttpResponse **const res) {
#ifdef _WIN32
  HINTERNET hConnect = NULL;
  HINTERNET hRequest = NULL;
  URL_COMPONENTSW urlComp;
  wchar_t *wUrl = NULL;
  wchar_t *wHost = NULL;
  wchar_t *wPath = NULL;
  wchar_t *wHeaders = NULL;
  size_t wLen;
  int rc = 0;
  DWORD dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
  DWORD dwStatusCode = 0;
  DWORD dwSize = sizeof(dwStatusCode);

  /* Body read logic */
  char *bodyBuf = NULL;
  char *readChunk = NULL;
  size_t bodySize = 0;
  DWORD bytesRead = 0;

  CHECK_EINVAL(ctx);
  CHECK_EINVAL(ctx->hInternet);
  CHECK_EINVAL(req);
  CHECK_EINVAL(res);

  if (req->parts.count > 0 && !req->body) {
    /* Request has parts but not flattened. Caller error strictly,
       as we cannot modify const struct here. */
    /* See http_curl.c implementation comment for architectural reasoning */
    return EINVAL;
  }

  /* 1. Convert URL to Wide */
  {
    size_t cap = strlen(req->url) + 1;
    wUrl = (wchar_t *)malloc(cap * sizeof(wchar_t));
    if (!wUrl)
      return ENOMEM;
    if (ascii_to_wide(req->url, wUrl, cap, &wLen) != 0) {
      free(wUrl);
      return EINVAL;
    }
  }

  /* 2. Crack URL */
  memset(&urlComp, 0, sizeof(urlComp));
  urlComp.dwStructSize = sizeof(urlComp);

  /* Allocate buffers for parsing */
  wHost = (wchar_t *)calloc(wLen + 1, sizeof(wchar_t));
  wPath = (wchar_t *)calloc(wLen + 1, sizeof(wchar_t));
  if (!wHost || !wPath) {
    rc = ENOMEM;
    goto cleanup;
  }

  urlComp.lpszHostName = wHost;
  urlComp.dwHostNameLength = (DWORD)wLen + 1;
  urlComp.lpszUrlPath = wPath;
  urlComp.dwUrlPathLength = (DWORD)wLen + 1;

  if (!InternetCrackUrlW(wUrl, (DWORD)wcslen(wUrl), 0, &urlComp)) {
    rc = EINVAL;
    goto cleanup;
  }

  /* 3. Connect */
  hConnect = InternetConnectW(ctx->hInternet, urlComp.lpszHostName,
                              (INTERNET_PORT)urlComp.nPort, NULL, NULL,
                              INTERNET_SERVICE_HTTP, 0, 0);
  if (!hConnect) {
    rc = EIO;
    goto cleanup;
  }

  /* 4. Open Request */
  if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) {
    dwFlags |= INTERNET_FLAG_SECURE;
    dwFlags |= ctx->security_flags; /* Apply ignore-cert flags here */
  }

  hRequest =
      HttpOpenRequestW(hConnect, method_to_wide(req->method),
                       urlComp.lpszUrlPath, NULL, NULL, NULL, dwFlags, 0);
  if (!hRequest) {
    rc = EIO;
    goto cleanup;
  }

  /* 5. Headers */
  if (req->headers.count > 0) {
    if (headers_to_wide_block(&req->headers, &wHeaders) != 0) {
      rc = ENOMEM;
      goto cleanup;
    }
    /* Add headers (replacing existing if needed or adding) */
    if (!HttpAddRequestHeadersW(hRequest, wHeaders, (DWORD)-1L,
                                HTTP_ADDREQ_FLAG_ADD |
                                    HTTP_ADDREQ_FLAG_REPLACE)) {
      rc = EIO;
      goto cleanup;
    }
  }

  /* 6. Send */
  /* body is binary safe void*, no string conversion */
  if (!HttpSendRequestW(hRequest, NULL, 0, req->body, (DWORD)req->body_len)) {
    rc = EIO;
    goto cleanup;
  }

  /* 7. Query Info (Status Code) */
  /* HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER requires output to define
   * DWORD */
  if (!HttpQueryInfoW(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                      &dwStatusCode, &dwSize, NULL)) {
    rc = EIO;
    goto cleanup;
  }

  /* 8. Read Response Body */
  readChunk = (char *)malloc(4096);
  if (!readChunk) {
    rc = ENOMEM;
    goto cleanup;
  }

  while (1) {
    if (!InternetReadFile(hRequest, readChunk, 4096, &bytesRead)) {
      rc = EIO;
      goto cleanup;
    }
    if (bytesRead == 0)
      break;

    /* Append */
    {
      char *new_buf = (char *)realloc(bodyBuf, bodySize + bytesRead + 1);
      if (!new_buf) {
        rc = ENOMEM;
        goto cleanup;
      }
      bodyBuf = new_buf;
      memcpy(bodyBuf + bodySize, readChunk, bytesRead);
      bodySize += bytesRead;
      bodyBuf[bodySize] = '\0'; /* Ensure null-term for text usage */
    }
  }

  /* 9. Construct Response */
  *res = (struct HttpResponse *)calloc(1, sizeof(struct HttpResponse));
  if (!*res) {
    rc = ENOMEM;
    goto cleanup;
  }
  http_response_init(*res);
  (*res)->status_code = (int)dwStatusCode;
  (*res)->body = bodyBuf;
  (*res)->body_len = bodySize;
  bodyBuf = NULL; /* Transferred ownership */

cleanup:
  safe_close_handle(&hRequest);
  safe_close_handle(&hConnect);
  if (bodyBuf)
    free(bodyBuf);
  if (readChunk)
    free(readChunk);
  if (wUrl)
    free(wUrl);
  if (wHost)
    free(wHost);
  if (wPath)
    free(wPath);
  if (wHeaders)
    free(wHeaders);

  return rc;
#else
  (void)ctx;
  (void)req;
  (void)res;
  return ENOTSUP;
#endif
}
