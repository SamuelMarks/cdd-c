/**
 * @file test_transport_factory.h
 * @brief Unit tests for the Platform-Aware Transport Factory.
 *
 * Verifies that the factory correctly initializes global state,
 * binds the correct function pointers, and handles memory cleanup
 * regardless of the build platform.
 *
 * @author Samuel Marks
 */

#ifndef TEST_TRANSPORT_FACTORY_H
#define TEST_TRANSPORT_FACTORY_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/http_types.h"

/* Prototype declarations for the factory (no header file provided in spec,
 * assuming internal linkage or separate header ref) */
/* In a real scenario, this would be `transport_factory.h` or integrated into
 * the generated client code */
int transport_global_init(void);
void transport_global_cleanup(void);
int transport_factory_init_client(struct HttpClient *const client);
void transport_factory_cleanup_client(struct HttpClient *const client);

TEST test_transport_global_lifecycle(void) {
  /* Should succeed on either platform (WinHTTP no-op, Curl ref-counted) */
  ASSERT_EQ(0, transport_global_init());

  /* Double init safety (Curl handles refcounting, WinHTTP no-op) */
  ASSERT_EQ(0, transport_global_init());

  transport_global_cleanup();
  transport_global_cleanup();
  PASS();
}

TEST test_transport_client_creation(void) {
  struct HttpClient client;
  int rc;

  /* Zero out stack struct */
  memset(&client, 0, sizeof(client));

  /* Prerequisite: Global Init */
  transport_global_init();

  rc = transport_factory_init_client(&client);
  ASSERT_EQ(0, rc);

  /* Verify context allocation */
  ASSERT(client.transport != NULL);

  /* Verify vtable binding */
  ASSERT(client.send != NULL);

  /* Verify function pointer resolution check (optional weak check) */
  /* We can't strictly compare function addresses across modules reliably in
     standard C tests without exposing the internal symbols, but non-null is
     sufficient for interface contract. */

  /* Cleanup */
  transport_factory_cleanup_client(&client);

  /* Verify double-free safety */
  transport_factory_cleanup_client(&client);
  ASSERT_EQ(NULL, client.transport);

  transport_global_cleanup();
  PASS();
}

TEST test_transport_init_null_safety(void) {
  ASSERT_EQ(EINVAL, transport_factory_init_client(NULL));

  /* Should not crash */
  transport_factory_cleanup_client(NULL);

  PASS();
}

SUITE(transport_factory_suite) {
  RUN_TEST(test_transport_global_lifecycle);
  RUN_TEST(test_transport_client_creation);
  RUN_TEST(test_transport_init_null_safety);
}

#endif /* TEST_TRANSPORT_FACTORY_H */
