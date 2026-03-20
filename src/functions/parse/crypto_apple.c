/**
 * @file crypto_apple.c
 * @brief Apple CommonCrypto implementation of the Abstract Crypto Interface.
 *
 * Implements hashing and HMAC using Apple's CommonCrypto framework.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "functions/parse/crypto_types.h"

#include <errno.h>

#if defined(__APPLE__)
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonHMAC.h>
/* clang-format on */

/**
 * @brief Executes the crypto sha256 operation.
 */
int crypto_sha256(const void *data, size_t data_len,
                  unsigned char *out_digest) {
  if (!data || !out_digest)
    return EINVAL;
  CC_SHA256(data, (CC_LONG)data_len, out_digest);
  return 0;
}

/**
 * @brief Executes the crypto hmac sha256 operation.
 */
int crypto_hmac_sha256(const void *key, size_t key_len, const void *data,
                       size_t data_len, unsigned char *out_mac) {
  if (!key || !data || !out_mac)
    return EINVAL;
  CCHmac(kCCHmacAlgSHA256, key, key_len, data, data_len, out_mac);
  return 0;
}

#else

/* Non-Apple stub implementation */

/**
 * @brief Executes the crypto sha256 operation.
 */
int crypto_sha256(const void *data, size_t data_len,
                  unsigned char *out_digest) {
  (void)data;
  (void)data_len;
  (void)out_digest;
  return ENOSYS;
}

/**
 * @brief Executes the crypto hmac sha256 operation.
 */
int crypto_hmac_sha256(const void *key, size_t key_len, const void *data,
                       size_t data_len, unsigned char *out_mac) {
  (void)key;
  (void)key_len;
  (void)data;
  (void)data_len;
  (void)out_mac;
  return ENOSYS;
}

#endif
