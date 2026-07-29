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
 *
 * least 32 bytes).
 */
extern C_CDD_EXPORT int g_crypto_fail_sha256;
extern C_CDD_EXPORT int g_crypto_fail_mdctx_new;
extern C_CDD_EXPORT int g_crypto_fail_digestinit;
extern C_CDD_EXPORT int g_crypto_fail_digestupdate;
extern C_CDD_EXPORT int g_crypto_fail_digestfinal;
extern C_CDD_EXPORT int g_crypto_fail_digestfinal_len;
extern C_CDD_EXPORT int g_crypto_fail_hmac;
extern C_CDD_EXPORT int g_crypto_fail_hmac_len;

cdd_c_error_t crypto_sha256(const void *data, size_t data_len,
                            unsigned char *out_digest) {
  if (g_crypto_fail_sha256 || g_crypto_fail_digestinit ||
      g_crypto_fail_digestupdate || g_crypto_fail_digestfinal ||
      g_crypto_fail_digestfinal_len)
    return CDD_C_ERROR_IO;
  if (g_crypto_fail_mdctx_new)
    return CDD_C_ERROR_MEMORY;

  if ((!data && data_len > 0) || !out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  CC_SHA256(data, (CC_LONG)data_len, out_digest);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the crypto hmac sha256 operation.
 *
 * bytes).
 */
cdd_c_error_t crypto_hmac_sha256(const void *key, size_t key_len,
                                 const void *data, size_t data_len,
                                 unsigned char *out_mac) {

  if (g_crypto_fail_hmac || g_crypto_fail_hmac_len)
    return CDD_C_ERROR_IO;

  if ((!key && key_len > 0) || (!data && data_len > 0) || !out_mac)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (key_len == 0) {
    static const unsigned char zero_pad[16] = {0};
    key = zero_pad;
    key_len = 16;
  }
  CCHmac(kCCHmacAlgSHA256, key, key_len, data, data_len, out_mac);
  return CDD_C_SUCCESS;
}

#else

/* Non-Apple stub implementation */

/**
 * @brief Executes the crypto sha256 operation (stub).
 *
 */
cdd_c_error_t crypto_sha256(const void *data, size_t data_len,
                            unsigned char *out_digest) {
  (void)data;
  (void)data_len;
  (void)out_digest;
  return CDD_C_ERROR_SYSTEM;
}

/**
 * @brief Executes the crypto hmac sha256 operation (stub).
 *
 */
cdd_c_error_t crypto_hmac_sha256(const void *key, size_t key_len,
                                 const void *data, size_t data_len,
                                 unsigned char *out_mac) {
  (void)key;
  (void)key_len;
  (void)data;
  (void)data_len;
  (void)out_mac;
  return CDD_C_ERROR_SYSTEM;
}

#endif
