/**
 * @file test_crypto.h
 * @brief Unit tests for the Abstract Crypto Interface.
 *
 * Verifies SHA-256 and HMAC-SHA-256 implementations against known test vectors
 * (RFC 4231). Ensures cross-platform consistency between OpenSSL and WinCrypt
 * implementations depending on the build target.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CRYPTO_H
#define TEST_CRYPTO_H

#include <errno.h>
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/crypto_types.h"

/* Helpers */

/**
 * @brief Convert binary buffer to hex string for easy comparison.
 */
static void bin2hex(const unsigned char *bin, size_t len, char *out) {
  size_t i;
  for (i = 0; i < len; ++i) {
    sprintf(out + (i * 2), "%02x", bin[i]);
  }
}

/**
 * @brief Check a list if Platform is supported to skip tests gracefully.
 * Returns 1 if supported, 0 otherwise.
 */
static int is_crypto_supported(void) {
  unsigned char buf[CRYPTO_SHA256_SIZE];
  if (crypto_sha256("test", 4, buf) == ENOTSUP) {
    return 0;
  }
  return 1;
}

/* SHA256 Tests */

TEST test_sha256_empty_string(void) {
  unsigned char digest[CRYPTO_SHA256_SIZE];
  char hex[CRYPTO_SHA256_SIZE * 2 + 1];
  /* SHA256("") */
  const char *expected =
      "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

  if (!is_crypto_supported())
    SKIPm("Crypto backend not compiled");

  ASSERT_EQ(0, crypto_sha256("", 0, digest));
  bin2hex(digest, CRYPTO_SHA256_SIZE, hex);
  ASSERT_STR_EQ(expected, hex);
  PASS();
}

TEST test_sha256_known_string(void) {
  unsigned char digest[CRYPTO_SHA256_SIZE];
  char hex[CRYPTO_SHA256_SIZE * 2 + 1];
  const char *input = "The quick brown fox jumps over the lazy dog";
  /* Echo -n "..." | shasum -a 256 */
  const char *expected =
      "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592";

  if (!is_crypto_supported())
    SKIPm("Crypto backend not compiled");

  ASSERT_EQ(0, crypto_sha256(input, strlen(input), digest));
  bin2hex(digest, CRYPTO_SHA256_SIZE, hex);
  ASSERT_STR_EQ(expected, hex);
  PASS();
}

/* HMAC-SHA256 Tests (Vectors from RFC 4231) */

TEST test_hmac_rfc4231_case1(void) {
  unsigned char mac[CRYPTO_SHA256_SIZE];
  char hex[CRYPTO_SHA256_SIZE * 2 + 1];
  /* Key: 20 bytes of 0x0b */
  unsigned char key[20];
  const char *data = "Hi There";
  const char *expected =
      "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7";

  if (!is_crypto_supported())
    SKIPm("Crypto backend not compiled");

  memset(key, 0x0b, sizeof(key));

  ASSERT_EQ(0, crypto_hmac_sha256(key, sizeof(key), data, strlen(data), mac));
  bin2hex(mac, CRYPTO_SHA256_SIZE, hex);
  ASSERT_STR_EQ(expected, hex);
  PASS();
}

TEST test_hmac_rfc4231_case2(void) {
  unsigned char mac[CRYPTO_SHA256_SIZE];
  char hex[CRYPTO_SHA256_SIZE * 2 + 1];
  /* Key: "Jefe" */
  const char *key = "Jefe";
  const char *data = "what do ya want for nothing?";
  const char *expected =
      "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843";

  if (!is_crypto_supported())
    SKIPm("Crypto backend not compiled");

  ASSERT_EQ(0, crypto_hmac_sha256(key, strlen(key), data, strlen(data), mac));
  bin2hex(mac, CRYPTO_SHA256_SIZE, hex);
  ASSERT_STR_EQ(expected, hex);
  PASS();
}

TEST test_hmac_empty_keys_or_data(void) {
  unsigned char mac[CRYPTO_SHA256_SIZE];
  const char *key = "key";
  const char *data = "data";

  if (!is_crypto_supported())
    SKIPm("Crypto backend not compiled");

  /* Null Output */
  ASSERT_EQ(EINVAL,
            crypto_hmac_sha256(key, strlen(key), data, strlen(data), NULL));

  /* Null Key logic varies by backend, but if key_len > 0 and key is null,
   * EINVAL */
  ASSERT_EQ(EINVAL, crypto_hmac_sha256(NULL, 5, data, strlen(data), mac));

  /* Null Data logic with len > 0 */
  ASSERT_EQ(EINVAL, crypto_hmac_sha256(key, strlen(key), NULL, 5, mac));

  /* Valid empty data -> HMAC should run on empty buffer */
  ASSERT_EQ(0, crypto_hmac_sha256(key, strlen(key), "", 0, mac));

  /* Valid empty key -> HMAC should run using 0-length key (e.g. key padded with
   * 0 usually) */
  ASSERT_EQ(0, crypto_hmac_sha256("", 0, data, strlen(data), mac));

  PASS();
}

SUITE(crypto_suite) {
  RUN_TEST(test_sha256_empty_string);
  RUN_TEST(test_sha256_known_string);
  RUN_TEST(test_hmac_rfc4231_case1);
  RUN_TEST(test_hmac_rfc4231_case2);
  RUN_TEST(test_hmac_empty_keys_or_data);
}

#endif /* TEST_CRYPTO_H */
