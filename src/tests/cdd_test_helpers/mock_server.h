/**
 * @file mock_server.h
 * @brief Lightweight, cross-platform TCP server for integration testing.
 *
 * Provides a threaded socket server that accepts a single connection (per
 * cycle), reads the request payload, stores it for inspection, and sends a
 * canned response. Supports both Winsock2 (Windows) and BSD Sockets (POSIX).
 *
 * @author Samuel Marks
 */

#ifndef CDD_TEST_HELPERS_MOCK_SERVER_H
#define CDD_TEST_HELPERS_MOCK_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cdd_test_helpers_export.h"
#include <stddef.h>

/**
 * @brief Opaque handle for the server instance.
 */
typedef struct MockServer_ *MockServerPtr;

/**
 * @brief Captured transaction details.
 */
struct MockServerRequest {
  char *raw_header;  /**< The raw request headers received */
  size_t header_len; /**< Length of raw_header */
};

/**
 * @brief Initialize a mock server instance.
 *
 * Allocates memory and initializes synchronization primitives
 * (mutexes/conditions). Does not start the network thread yet.
 *
 * @return A handle to the server, or NULL on failure.
 */
extern CDD_TEST_HELPERS_EXPORT MockServerPtr mock_server_init(void);

/**
 * @brief Destroy a mock server instance.
 *
 * Stops the thread if running, closes sockets, and frees memory.
 *
 * @param[in] server The server handle.
 */
extern CDD_TEST_HELPERS_EXPORT void mock_server_destroy(MockServerPtr server);

/**
 * @brief Start the server thread listening on loopback.
 *
 * Binds to 127.0.0.1 on an ephemeral port.
 *
 * @param[in] server The server handle.
 * @return 0 on success, non-zero error code on failure.
 */
extern CDD_TEST_HELPERS_EXPORT int mock_server_start(MockServerPtr server);

/**
 * @brief Get the port number the server is listening on.
 *
 * @param[in] server The server handle.
 * @return The port number (host byte order), or 0 if not running.
 */
extern CDD_TEST_HELPERS_EXPORT int mock_server_get_port(MockServerPtr server);

/**
 * @brief Wait for a client request and capture the headers.
 *
 * Blocks until a client connects, sends data, and the server receives it.
 * The server automatically replies with "200 OK" upon receipt.
 *
 * @param[in] server The server handle.
 * @param[out] out_req Pointer to struct to receive request data.
 *                     Caller must free contents.
 * @return 0 on success, non-zero on timeout or error.
 */
extern CDD_TEST_HELPERS_EXPORT int
mock_server_wait_for_request(MockServerPtr server,
                             struct MockServerRequest *out_req);

/**
 * @brief Helper to free the request content captured by wait_for_request.
 *
 * @param[in] req The request structure to clean.
 */
extern CDD_TEST_HELPERS_EXPORT void
mock_server_request_cleanup(struct MockServerRequest *req);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TEST_HELPERS_MOCK_SERVER_H */
