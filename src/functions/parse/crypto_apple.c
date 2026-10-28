/**
 * @file crypto_apple.c
 * @brief Apple CommonCrypto implementation of the Abstract Crypto Interface.
 *
 * Implements hashing and HMAC using Apple's CommonCrypto framework.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "functions/parse/crypto_types.h"

#include <errno.h>

#if defined(__APPLE__)
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonHMAC.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_crypto_fail_sha256;
extern C_CDD_EXPORT int g_crypto_fail_mdctx_new;
extern C_CDD_EXPORT int g_crypto_fail_digestinit;
extern C_CDD_EXPORT int g_crypto_fail_digestupdate;
extern C_CDD_EXPORT int g_crypto_fail_digestfinal;
extern C_CDD_EXPORT int g_crypto_fail_digestfinal_len;
extern C_CDD_EXPORT int g_crypto_fail_hmac;
extern C_CDD_EXPORT int g_crypto_fail_hmac_len;
#endif

/**
 * @brief Computes a 32-byte SHA-256 digest over a data buffer using Apple
 * CommonCrypto.
 *
 * @param[in] data Pointer to the input buffer to hash.
 * @param[in] data_len Length of the input buffer in bytes.
 * @param[out] out_digest Output buffer receiving the 32-byte SHA-256 digest.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t crypto_sha256(const void *data, size_t data_len,
                            unsigned char *out_digest) {
#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_sha256 || g_crypto_fail_digestinit ||
      g_crypto_fail_digestupdate || g_crypto_fail_digestfinal ||
      g_crypto_fail_digestfinal_len)
    return CDD_C_ERROR_IO;
  if (g_crypto_fail_mdctx_new)
    return CDD_C_ERROR_MEMORY;
#endif

  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  CC_SHA256(data, (CC_LONG)data_len, out_digest);
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes a 32-byte HMAC-SHA256 digest over a data buffer using Apple
 * CommonCrypto.
 *
 * @param[in] key Secret key bytes.
 * @param[in] key_len Length of `key` in bytes.
 * @param[in] data Message payload bytes to authenticate.
 * @param[in] data_len Length of `data` in bytes.
 * @param[out] out_mac Output buffer receiving the 32-byte HMAC-SHA256 tag.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t crypto_hmac_sha256(const void *key, size_t key_len,
                                 const void *data, size_t data_len,
                                 unsigned char *out_mac) {

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_hmac || g_crypto_fail_hmac_len)
    return CDD_C_ERROR_IO;
#endif

  if (!key && key_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!out_mac)
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
 * @brief Performs crypto sha256 operation (non-Apple stub).
 *
 * @param[in] data Pointer to the input buffer to hash.
 * @param[in] data_len Length of the input buffer in bytes.
 * @param[out] out_digest Output buffer receiving the 32-byte SHA-256 digest.
 * @return `CDD_C_ERROR_SYSTEM` when compiled without Apple CommonCrypto.
 */
cdd_c_error_t crypto_sha256(const void *data, size_t data_len,
                            unsigned char *out_digest) {
  if (!data && data_len > 0 && !out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return CDD_C_ERROR_SYSTEM;
}

/**
 * @brief Performs crypto hmac sha256 operation (non-Apple stub).
 *
 * @param[in] key Secret key bytes.
 * @param[in] key_len Length of `key` in bytes.
 * @param[in] data Message payload bytes to authenticate.
 * @param[in] data_len Length of `data` in bytes.
 * @param[out] out_mac Output buffer receiving the 32-byte HMAC-SHA256 tag.
 * @return `CDD_C_ERROR_SYSTEM` when compiled without Apple CommonCrypto.
 */
cdd_c_error_t crypto_hmac_sha256(const void *key, size_t key_len,
                                 const void *data, size_t data_len,
                                 unsigned char *out_mac) {
  if (!key && key_len > 0 && !data && data_len > 0 && !out_mac)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return CDD_C_ERROR_SYSTEM;
}

#endif
