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
 * @param[in] data The input data to hash.
 * @param[in] data_len The length of the input data.
 * @param[out] out_digest The output buffer for the SHA256 digest (must be at
 * least 32 bytes).
 * @return 0 on success, EINVAL if invalid arguments are provided.
 */
int crypto_sha256(const void *data, size_t data_len,
                  unsigned char *out_digest) {
  if (!data || !out_digest)
    return EINVAL;
  if (data_len > 0 && !data)
    return EINVAL;
  CC_SHA256(data, (CC_LONG)data_len, out_digest);
  return 0;
}

/**
 * @brief Executes the crypto hmac sha256 operation.
 *
 * @param[in] key The key for HMAC.
 * @param[in] key_len The length of the key.
 * @param[in] data The input data to authenticate.
 * @param[in] data_len The length of the input data.
 * @param[out] out_mac The output buffer for the HMAC (must be at least 32
 * bytes).
 * @return 0 on success, EINVAL if invalid arguments are provided.
 */
int crypto_hmac_sha256(const void *key, size_t key_len, const void *data,
                       size_t data_len, unsigned char *out_mac) {
  if ((!key && key_len > 0) || (!data && data_len > 0) || !out_mac)
    return EINVAL;
  if (key_len == 0) {
    static const unsigned char zero_pad[16] = {0};
    key = zero_pad;
    key_len = 16;
  }
  CCHmac(kCCHmacAlgSHA256, key, key_len, data, data_len, out_mac);
  return 0;
}

#else

/* Non-Apple stub implementation */

/**
 * @brief Executes the crypto sha256 operation (stub).
 *
 * @param[in] data The input data to hash.
 * @param[in] data_len The length of the input data.
 * @param[out] out_digest The output buffer for the SHA256 digest.
 * @return ENOSYS since it's a stub.
 */
int crypto_sha256(const void *data, size_t data_len,
                  unsigned char *out_digest) {
  (void)data;
  (void)data_len;
  (void)out_digest;
  return ENOSYS;
}

/**
 * @brief Executes the crypto hmac sha256 operation (stub).
 *
 * @param[in] key The key for HMAC.
 * @param[in] key_len The length of the key.
 * @param[in] data The input data to authenticate.
 * @param[in] data_len The length of the input data.
 * @param[out] out_mac The output buffer for the HMAC.
 * @return ENOSYS since it's a stub.
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
