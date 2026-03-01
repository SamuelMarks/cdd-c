/**
 * @file transport_factory.c
 * @brief Platform-aware Abstract Network Interface (ANI) Factory.
 *
 * Implements a unified initialization entry point that selects the appropriate
 * HTTP backend (WinHTTP for Windows, Libcurl for POSIX) at compile time.
 * This simplifies client usage by abstracting the `#ifdef` logic into a single
 * translation unit.
 *
 * @author Samuel Marks
 */

#include "functions/parse/http_types.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#include "functions/parse/http_winhttp.h"
#else
#include "functions/parse/http_curl.h"
#endif

#include <errno.h>
#include <stddef.h>

/**
 * @brief Initialize the global transport layer dependencies.
 *
 * Calls `http_winhttp_global_init` on Windows or `http_curl_global_init` on
 * POSIX. Should be called once at application startup.
 *
 * @return 0 on success, or an error code from the underlying backend.
 */
int transport_global_init(void) {
#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
  return http_winhttp_global_init();
#else
  return http_curl_global_init();
#endif
}

/**
 * @brief Cleanup global transport layer dependencies.
 *
 * Calls the platform-specific cleanup function.
 */
void transport_global_cleanup(void) {
#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
  http_winhttp_global_cleanup();
#else
  http_curl_global_cleanup();
#endif
}

/**
 * @brief Initialize a transport context and bind the `send` method.
 *
 * Allocates the backend-specific context structure and assigns the correct
 * function pointer to `client->send`.
 *
 * @param[out] client The high-level client structure to populate.
 *                    Must be pre-allocated.
 * @return 0 on success, error code (EINVAL/ENOMEM/EIO) on failure.
 */
int transport_factory_init_client(struct HttpClient *const client) {
  int rc;

  if (!client) {
    return EINVAL;
  }

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
  rc = http_winhttp_context_init(&client->transport);
  if (rc == 0) {
    client->send = http_winhttp_send;
  }
#else
  rc = http_curl_context_init(&client->transport);
  if (rc == 0) {
    client->send = http_curl_send;
  }
#endif

  return rc;
}

/**
 * @brief Free the transport context within a client.
 *
 * Calls the platform-specific context free function on `client->transport`.
 * NOTE: This function does *not* free the `client` pointer itself, nor
 * other members like `base_url` or `config`; it only handles the opaque
 * transport handle.
 *
 * @param[in] client The client containing the transport handle.
 */
void transport_factory_cleanup_client(struct HttpClient *const client) {
  if (!client || !client->transport) {
    return;
  }

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
  http_winhttp_context_free(client->transport);
#else
  http_curl_context_free(client->transport);
#endif

  client->transport = NULL;
}
