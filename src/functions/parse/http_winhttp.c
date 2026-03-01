/**
 * @file http_winhttp.c
 * @brief Implementation of the WinHTTP backend.
 *
 * WinHTTP specific implementation that ensures binary-safe responses
 * by accurately tracking bytes read from `WinHttpReadData` and explicitly
 * null-terminating the buffer for text safety without truncating binary
 * content.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "c_cddConfig.h"
#include <winbase.h>
#include <windef.h>
#include <winhttp.h>
#else
/* Stub definitions */
typedef void *HINTERNET;
typedef int BOOL;
typedef unsigned long DWORD;
typedef int wchar_t;
typedef void *LPVOID;
#define FALSE 0
#define TRUE 1
#endif

#include "functions/parse/fs.h"
#include "functions/parse/http_winhttp.h"
#include "functions/parse/str.h"

#ifdef _WIN32
#pragma comment(lib, "winhttp.lib")
#endif

struct HttpTransportContext {
  HINTERNET hSession;
  DWORD security_flags;
};

/* ... (Helpers like method_to_wide, safe_close_handle omitted for brevity if
   unchanged logic is same, but providing full file content below for
   correctness) ... */

#ifdef _WIN32
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
  case HTTP_OPTIONS:
    return L"OPTIONS";
  case HTTP_TRACE:
    return L"TRACE";
  case HTTP_QUERY:
    return L"QUERY";
  case HTTP_CONNECT:
    return L"CONNECT";
  case HTTP_PATCH:
    return L"PATCH";
  default:
    return L"GET";
  }
}

static void safe_close_handle(HINTERNET *h) {
  if (h && *h) {
    WinHttpCloseHandle(*h);
    *h = NULL;
  }
}

static int headers_to_wide_block(const struct HttpHeaders *headers,
                                 wchar_t **out) {
  size_t i;
  size_t total_wide_chars = 0;
  wchar_t *buf;
  wchar_t *p;

  *out = NULL;
  if (!headers || headers->count == 0)
    return 0;

  for (i = 0; i < headers->count; ++i) {
    total_wide_chars += strlen(headers->headers[i].key);
    total_wide_chars += strlen(headers->headers[i].value);
    total_wide_chars += 4;
  }
  total_wide_chars += 1;

  buf = (wchar_t *)calloc(total_wide_chars, sizeof(wchar_t));
  if (!buf)
    return ENOMEM;

  p = buf;
  for (i = 0; i < headers->count; ++i) {
    size_t written = 0;
    if (ascii_to_wide(headers->headers[i].key, p, total_wide_chars - (p - buf),
                      &written) != 0) {
      free(buf);
      return EIO;
    }
    p += written;
    wcscpy_s(p, total_wide_chars - (p - buf), L": ");
    p += 2;
    if (ascii_to_wide(headers->headers[i].value, p,
                      total_wide_chars - (p - buf), &written) != 0) {
      free(buf);
      return EIO;
    }
    p += written;
    wcscpy_s(p, total_wide_chars - (p - buf), L"\r\n");
    p += 2;
  }
  *out = buf;
  return 0;
}
#endif /* _WIN32 */

int http_winhttp_global_init(void) { return 0; }
void http_winhttp_global_cleanup(void) {}

int http_winhttp_context_init(struct HttpTransportContext **const ctx) {
#ifdef _WIN32
  HINTERNET hSession;
  if (!ctx)
    return EINVAL;

  hSession = WinHttpOpen(L"c_cdd/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                         WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession)
    return EIO;

  *ctx = (struct HttpTransportContext *)malloc(
      sizeof(struct HttpTransportContext));
  if (!*ctx) {
    WinHttpCloseHandle(hSession);
    return ENOMEM;
  }
  (*ctx)->hSession = hSession;
  (*ctx)->security_flags = 0;
  return 0;
#else
  (void)ctx;
  return ENOTSUP;
#endif
}

void http_winhttp_context_free(struct HttpTransportContext *const ctx) {
#ifdef _WIN32
  if (ctx) {
    if (ctx->hSession)
      WinHttpCloseHandle(ctx->hSession);
    free(ctx);
  }
#else
  (void)ctx;
#endif
}

int http_winhttp_config_apply(struct HttpTransportContext *const ctx,
                              const struct HttpConfig *const config) {
#ifdef _WIN32
  DWORD dwTimeout;
  if (!ctx || !ctx->hSession || !config)
    return EINVAL;

  dwTimeout = (DWORD)config->timeout_ms;
  if (!WinHttpSetTimeouts(ctx->hSession, dwTimeout, dwTimeout, dwTimeout,
                          dwTimeout))
    return EIO;

  if (config->proxy_url) {
    WINHTTP_PROXY_INFO proxyInfo;
    wchar_t *wProxy = NULL;
    size_t wLen = 0;
    size_t bufSize = strlen(config->proxy_url) + 1;
    wProxy = (wchar_t *)calloc(bufSize, sizeof(wchar_t));
    if (!wProxy)
      return ENOMEM;
    if (ascii_to_wide(config->proxy_url, wProxy, bufSize, &wLen) != 0) {
      free(wProxy);
      return EINVAL;
    }
    proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
    proxyInfo.lpszProxy = wProxy;
    proxyInfo.lpszProxyBypass = NULL;
    if (!WinHttpSetOption(ctx->hSession, WINHTTP_OPTION_PROXY, &proxyInfo,
                          sizeof(proxyInfo))) {
      free(wProxy);
      return EIO;
    }
    free(wProxy);
  } else {
    WINHTTP_PROXY_INFO proxyInfo;
    proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
    proxyInfo.lpszProxy = NULL;
    proxyInfo.lpszProxyBypass = NULL;
    WinHttpSetOption(ctx->hSession, WINHTTP_OPTION_PROXY, &proxyInfo,
                     sizeof(proxyInfo));
  }

  ctx->security_flags = 0;
  if (!config->verify_peer) {
    ctx->security_flags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
    ctx->security_flags |= SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
  }
  if (!config->verify_host) {
    ctx->security_flags |= SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
  }
  return 0;
#else
  (void)ctx;
  (void)config;
  return ENOTSUP;
#endif
}

#define CLEANUP_AND_RET(err)                                                   \
  do {                                                                         \
    rc = (err);                                                                \
    goto cleanup;                                                              \
  } while (0)

int http_winhttp_send(struct HttpTransportContext *const ctx,
                      const struct HttpRequest *const req,
                      struct HttpResponse **const res) {
#ifdef _WIN32
  HINTERNET hConnect = NULL, hRequest = NULL;
  URL_COMPONENTS urlComp;
  wchar_t *wUrl = NULL;
  wchar_t *wHeaders = NULL;
  wchar_t *hostName = NULL;
  wchar_t *urlPath = NULL;
  int rc = 0;
  size_t wLen = 0;
  DWORD dwStatusCode = 0;
  DWORD dwSize = sizeof(dwStatusCode);

  /* Body reading state */
  char *readBuf = NULL;
  char *totalBody = NULL;
  size_t totalSize = 0;
  DWORD dwDownloaded = 0;

  if (!ctx || !ctx->hSession || !req || !res)
    return EINVAL;

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

  memset(&urlComp, 0, sizeof(urlComp));
  urlComp.dwStructSize = sizeof(urlComp);
  hostName = (wchar_t *)calloc(wLen + 1, sizeof(wchar_t));
  urlPath = (wchar_t *)calloc(wLen + 1, sizeof(wchar_t));
  if (!hostName || !urlPath) {
    free(wUrl);
    if (hostName)
      free(hostName);
    if (urlPath)
      free(urlPath);
    return ENOMEM;
  }
  urlComp.lpszHostName = hostName;
  urlComp.dwHostNameLength = (DWORD)wLen + 1;
  urlComp.lpszUrlPath = urlPath;
  urlComp.dwUrlPathLength = (DWORD)wLen + 1;

  if (!WinHttpCrackUrl(wUrl, (DWORD)wcslen(wUrl), 0, &urlComp)) {
    CLEANUP_AND_RET(EINVAL);
  }

  hConnect = WinHttpConnect(ctx->hSession, urlComp.lpszHostName,
                            (INTERNET_PORT)urlComp.nPort, 0);
  if (!hConnect)
    CLEANUP_AND_RET(EIO);

  hRequest = WinHttpOpenRequest(
      hConnect, method_to_wide(req->method), urlComp.lpszUrlPath, NULL,
      WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
      (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
  if (!hRequest)
    CLEANUP_AND_RET(EIO);

  if (ctx->security_flags != 0) {
    WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS,
                     &ctx->security_flags, sizeof(ctx->security_flags));
  }

  if (req->headers.count > 0) {
    if (headers_to_wide_block(&req->headers, &wHeaders) != 0)
      CLEANUP_AND_RET(ENOMEM);
    if (wHeaders) {
      WinHttpAddRequestHeaders(hRequest, wHeaders, (DWORD)-1L,
                               WINHTTP_ADDREQ_FLAG_ADD);
    }
  }

  if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                          req->body ? req->body : WINHTTP_NO_REQUEST_DATA,
                          (DWORD)req->body_len, (DWORD)req->body_len, 0)) {
    CLEANUP_AND_RET(EIO);
  }

  if (!WinHttpReceiveResponse(hRequest, NULL))
    CLEANUP_AND_RET(EIO);

  if (!WinHttpQueryHeaders(
          hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
          WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode, &dwSize,
          WINHTTP_NO_HEADER_INDEX)) {
    CLEANUP_AND_RET(EIO);
  }

  /* Read Body Cycle */
  readBuf = (char *)malloc(8192);
  if (!readBuf)
    CLEANUP_AND_RET(ENOMEM);

  do {
    dwSize = 0;
    if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
      CLEANUP_AND_RET(EIO);
    if (dwSize == 0)
      break;
    if (dwSize > 8192)
      dwSize = 8192;

    if (!WinHttpReadData(hRequest, (LPVOID)readBuf, dwSize, &dwDownloaded))
      CLEANUP_AND_RET(EIO);

    if (dwDownloaded > 0) {
      char *new_ptr = (char *)realloc(totalBody, totalSize + dwDownloaded + 1);
      if (!new_ptr)
        CLEANUP_AND_RET(ENOMEM);
      totalBody = new_ptr;
      memcpy(totalBody + totalSize, readBuf, dwDownloaded);
      totalSize += dwDownloaded;
      totalBody[totalSize] = '\0'; /* Null terminate for text safety */
    }
  } while (dwSize > 0);

  *res = (struct HttpResponse *)calloc(1, sizeof(struct HttpResponse));
  if (!*res)
    CLEANUP_AND_RET(ENOMEM);
  if (http_response_init(*res) != 0) {
    free(*res);
    *res = NULL;
    CLEANUP_AND_RET(ENOMEM);
  }

  (*res)->status_code = (int)dwStatusCode;
  (*res)->body = totalBody;
  (*res)->body_len = totalSize;
  totalBody = NULL;

cleanup:
  safe_close_handle(&hRequest);
  safe_close_handle(&hConnect);
  if (wUrl)
    free(wUrl);
  if (wHeaders)
    free(wHeaders);
  if (hostName)
    free(hostName);
  if (urlPath)
    free(urlPath);
  if (readBuf)
    free(readBuf);
  if (totalBody)
    free(totalBody);

  return rc;
#else
  (void)ctx;
  (void)req;
  (void)res;
  return ENOTSUP;
#endif
}
