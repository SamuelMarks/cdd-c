/**
 * @file http_apple.c
 * @brief Apple CFNetwork implementation of the Abstract Network Interface.
 *
 * Implements the HTTP transport using Apple's CFNetwork framework.
 *
 * @author Samuel Marks
 */

#include "functions/parse/http_apple.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>

struct HttpTransportContext {
  struct HttpConfig config;
};

int http_apple_global_init(void) {
  return 0;
}

void http_apple_global_cleanup(void) {
}

int http_apple_context_init(struct HttpTransportContext **ctx) {
  if (!ctx) return EINVAL;

  *ctx = (struct HttpTransportContext *)malloc(sizeof(struct HttpTransportContext));
  if (!*ctx) return ENOMEM;

  http_config_init(&(*ctx)->config);

  return 0;
}

void http_apple_context_free(struct HttpTransportContext *ctx) {
  if (ctx) {
    http_config_free(&ctx->config);
    free(ctx);
  }
}

int http_apple_config_apply(struct HttpTransportContext *ctx,
                            const struct HttpConfig *config) {
  if (!ctx || !config) return EINVAL;
  /* Copy relevant config or store a reference. Here we just copy. */
  /* In a real implementation, we'd deep copy or map to Apple settings. */
  return 0;
}

int http_apple_send(struct HttpTransportContext *ctx,
                    const struct HttpRequest *req,
                    struct HttpResponse **res) {
  CFURLRef url;
  CFStringRef urlStr;
  CFStringRef method;
  CFHTTPMessageRef requestRef;
  CFReadStreamRef readStream;
  CFIndex bytesRead;
  UInt8 buf[4096];
  CFDataRef bodyData = NULL;
  size_t i;
  CFHTTPMessageRef responseRef = NULL;
  CFDictionaryRef headersDict = NULL;
  
  if (!ctx || !req || !res) return EINVAL;

  *res = (struct HttpResponse *)malloc(sizeof(struct HttpResponse));
  if (!*res) return ENOMEM;
  
  if (http_response_init(*res) != 0) {
    free(*res);
    *res = NULL;
    return ENOMEM;
  }

  urlStr = CFStringCreateWithCString(kCFAllocatorDefault, req->url, kCFStringEncodingUTF8);
  if (!urlStr) return EINVAL;

  url = CFURLCreateWithString(kCFAllocatorDefault, urlStr, NULL);
  CFRelease(urlStr);
  if (!url) return EINVAL;

  method = CFSTR("GET");
  if (req->method == HTTP_POST) method = CFSTR("POST");
  else if (req->method == HTTP_PUT) method = CFSTR("PUT");
  else if (req->method == HTTP_DELETE) method = CFSTR("DELETE");
  else if (req->method == HTTP_PATCH) method = CFSTR("PATCH");
  else if (req->method == HTTP_HEAD) method = CFSTR("HEAD");
  else if (req->method == HTTP_OPTIONS) method = CFSTR("OPTIONS");
  else if (req->method == HTTP_TRACE) method = CFSTR("TRACE");
  else if (req->method == HTTP_CONNECT) method = CFSTR("CONNECT");

  requestRef = CFHTTPMessageCreateRequest(kCFAllocatorDefault, method, url, kCFHTTPVersion1_1);
  CFRelease(url);
  if (!requestRef) return ENOMEM;

  for (i = 0; i < req->headers.count; ++i) {
    CFStringRef key = CFStringCreateWithCString(kCFAllocatorDefault, req->headers.headers[i].key, kCFStringEncodingUTF8);
    CFStringRef val = CFStringCreateWithCString(kCFAllocatorDefault, req->headers.headers[i].value, kCFStringEncodingUTF8);
    if (key && val) {
      CFHTTPMessageSetHeaderFieldValue(requestRef, key, val);
    }
    if (key) CFRelease(key);
    if (val) CFRelease(val);
  }

  if (req->body && req->body_len > 0) {
    CFDataRef body = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)req->body, (CFIndex)req->body_len);
    if (body) {
      CFHTTPMessageSetBody(requestRef, body);
      CFRelease(body);
    }
  }

  readStream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, requestRef);
  CFRelease(requestRef);
  if (!readStream) return ENOMEM;

  /* Since this is a synchronous call, we wait until it's done */
  if (!CFReadStreamOpen(readStream)) {
    CFRelease(readStream);
    return EIO;
  }

  bodyData = CFDataCreateMutable(kCFAllocatorDefault, 0);

  while (1) {
    bytesRead = CFReadStreamRead(readStream, buf, sizeof(buf));
    if (bytesRead < 0) {
      CFRelease(readStream);
      if (bodyData) CFRelease(bodyData);
      return EIO;
    } else if (bytesRead == 0) {
      break; /* EOF */
    }
    CFDataAppendBytes((CFMutableDataRef)bodyData, buf, bytesRead);
  }

  responseRef = (CFHTTPMessageRef)CFReadStreamCopyProperty(readStream, kCFStreamPropertyHTTPResponseHeader);
  if (responseRef) {
    (*res)->status_code = (int)CFHTTPMessageGetResponseStatusCode(responseRef);
    headersDict = CFHTTPMessageCopyAllHeaderFields(responseRef);
    
    if (headersDict) {
      /* Ideally iterate dictionary and map to (*res)->headers */
      CFRelease(headersDict);
    }
    CFRelease(responseRef);
  }

  if (bodyData) {
    CFIndex len = CFDataGetLength(bodyData);
    (*res)->body = malloc((size_t)len + 1);
    if ((*res)->body) {
      CFDataGetBytes(bodyData, CFRangeMake(0, len), (UInt8 *)(*res)->body);
      ((char *)(*res)->body)[len] = '\0';
      (*res)->body_len = (size_t)len;
    }
    CFRelease(bodyData);
  }

  CFReadStreamClose(readStream);
  CFRelease(readStream);

  return 0;
}

#else

/* Non-Apple stub implementation */

struct HttpTransportContext {
  int dummy;
};

int http_apple_global_init(void) {
  return 0;
}

void http_apple_global_cleanup(void) {
}

int http_apple_context_init(struct HttpTransportContext **ctx) {
  if (!ctx) return EINVAL;
  *ctx = (struct HttpTransportContext *)malloc(sizeof(struct HttpTransportContext));
  if (!*ctx) return ENOMEM;
  (*ctx)->dummy = 0;
  return 0;
}

void http_apple_context_free(struct HttpTransportContext *ctx) {
  if (ctx) {
    free(ctx);
  }
}

int http_apple_config_apply(struct HttpTransportContext *ctx,
                            const struct HttpConfig *config) {
  if (!ctx || !config) return EINVAL;
  return 0;
}

int http_apple_send(struct HttpTransportContext *ctx,
                    const struct HttpRequest *req,
                    struct HttpResponse **res) {
  if (!ctx || !req || !res) return EINVAL;
  return ENOSYS;
}

#endif
