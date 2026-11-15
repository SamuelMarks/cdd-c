/**
 * @file crypto_openssl.c
 * @brief OpenSSL implementation of the Abstract Crypto Interface.
 *
 * Uses the OpenSSL EVP API (High-level envelope interface) to ensure
 * compatibility with OpenSSL 1.1 and 3.0+ without deprecated API calls.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "c_cdd/log.h"
#include "functions/parse/crypto_types.h"

#if !defined(CDD_NO_OPENSSL)
#include <openssl/evp.h>
#if OPENSSL_VERSION_NUMBER < 0x30000000L
#include <openssl/hmac.h>
#endif
#endif
/* clang-format on */

#ifdef CDD_CRYPTO_OPENSSL_STANDALONE_TEST
/** @brief Standalone test symbol name for OpenSSL SHA-256. */
#define CDD_OPENSSL_CRYPTO_SHA256 cdd_openssl_crypto_sha256
/** @brief Standalone test symbol name for OpenSSL HMAC-SHA-256. */
#define CDD_OPENSSL_CRYPTO_HMAC_SHA256 cdd_openssl_crypto_hmac_sha256
/**
 * @brief Standalone test entrypoint `cdd_openssl_crypto_sha256`.
 * @param data Parameter `data`.
 * @param data_len Parameter `data_len`.
 * @param out_digest Parameter `out_digest`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_openssl_crypto_sha256(const void *data,
                                                   size_t data_len,
                                                   unsigned char *out_digest);
/**
 * @brief Standalone test entrypoint `cdd_openssl_crypto_hmac_sha256`.
 * @param key Parameter `key`.
 * @param key_len Parameter `key_len`.
 * @param data Parameter `data`.
 * @param data_len Parameter `data_len`.
 * @param out_mac Parameter `out_mac`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_openssl_crypto_hmac_sha256(const void *key,
                                                        size_t key_len,
                                                        const void *data,
                                                        size_t data_len,
                                                        unsigned char *out_mac);
#else
/** @brief Production symbol name for OpenSSL SHA-256. */
#define CDD_OPENSSL_CRYPTO_SHA256 crypto_sha256
/** @brief Production symbol name for OpenSSL HMAC-SHA-256. */
#define CDD_OPENSSL_CRYPTO_HMAC_SHA256 crypto_hmac_sha256
#endif

#if !defined(CDD_NO_OPENSSL)

#ifdef CDD_BUILD_TESTS
/** @brief External test variable `g_crypto_fail_sha256`. */
extern C_CDD_EXPORT int g_crypto_fail_sha256;
/** @brief External test variable `g_crypto_fail_mdctx_new`. */
extern C_CDD_EXPORT int g_crypto_fail_mdctx_new;
/** @brief External test variable `g_crypto_fail_digestinit`. */
extern C_CDD_EXPORT int g_crypto_fail_digestinit;
/** @brief External test variable `g_crypto_fail_digestupdate`. */
extern C_CDD_EXPORT int g_crypto_fail_digestupdate;
/** @brief External test variable `g_crypto_fail_digestfinal`. */
extern C_CDD_EXPORT int g_crypto_fail_digestfinal;
/** @brief External test variable `g_crypto_fail_digestfinal_len`. */
extern C_CDD_EXPORT int g_crypto_fail_digestfinal_len;
/** @brief External test variable `g_crypto_fail_hmac`. */
extern C_CDD_EXPORT int g_crypto_fail_hmac;
/** @brief External test variable `g_crypto_fail_hmac_len`. */
extern C_CDD_EXPORT int g_crypto_fail_hmac_len;

/** @brief Fault-injecting wrapper for `EVP_sha256`. */
#define HOOK_EVP_sha256() (g_crypto_fail_sha256 ? NULL : EVP_sha256())
/** @brief Fault-injecting wrapper for `EVP_MD_CTX_new`. */
#define HOOK_EVP_MD_CTX_new()                                                  \
  (g_crypto_fail_mdctx_new ? NULL : EVP_MD_CTX_new())
/** @brief Fault-injecting wrapper for `EVP_DigestInit_ex`. */
#define HOOK_EVP_DigestInit_ex(ctx, type, impl)                                \
  (g_crypto_fail_digestinit ? 0 : EVP_DigestInit_ex((ctx), (type), (impl)))
/** @brief Fault-injecting wrapper for `EVP_DigestUpdate`. */
#define HOOK_EVP_DigestUpdate(ctx, d, cnt)                                     \
  (g_crypto_fail_digestupdate ? 0 : EVP_DigestUpdate((ctx), (d), (cnt)))
/** @brief Fault-injecting wrapper for `EVP_DigestFinal_ex`. */
#define HOOK_EVP_DigestFinal_ex(ctx, md, s)                                    \
  (g_crypto_fail_digestfinal ? 0                                               \
                             : (g_crypto_fail_digestfinal_len                  \
                                    ? (*(s) = 0, 1)                            \
                                    : EVP_DigestFinal_ex((ctx), (md), (s))))
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
/** @brief Fault-injecting wrapper for `EVP_Q_mac` on OpenSSL 3.0+. */
#define HOOK_EVP_Q_mac(key, key_len, d, n, md, md_size, md_len)                \
  (g_crypto_fail_hmac                                                          \
       ? NULL                                                                  \
       : (g_crypto_fail_hmac_len                                               \
              ? (*(md_len) = 0, (md))                                          \
              : EVP_Q_mac(NULL, "HMAC", NULL, "SHA256", NULL, (key),           \
                          (key_len), (d), (n), (md), (md_size), (md_len))))
#else
/** @brief Fault-injecting wrapper for `HMAC` on OpenSSL 1.1. */
#define HOOK_HMAC(evp_md, key, key_len, d, n, md, md_len)                      \
  (g_crypto_fail_hmac                                                          \
       ? NULL                                                                  \
       : (g_crypto_fail_hmac_len                                               \
              ? (*(md_len) = 0, (md))                                          \
              : HMAC((evp_md), (key), (key_len), (d), (n), (md), (md_len))))
#endif

#else

/** @brief Direct call for `EVP_sha256`. */
#define HOOK_EVP_sha256() EVP_sha256()
/** @brief Direct call for `EVP_MD_CTX_new`. */
#define HOOK_EVP_MD_CTX_new() EVP_MD_CTX_new()
/** @brief Direct call for `EVP_DigestInit_ex`. */
#define HOOK_EVP_DigestInit_ex EVP_DigestInit_ex
/** @brief Direct call for `EVP_DigestUpdate`. */
#define HOOK_EVP_DigestUpdate EVP_DigestUpdate
/** @brief Direct call for `EVP_DigestFinal_ex`. */
#define HOOK_EVP_DigestFinal_ex EVP_DigestFinal_ex
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
/** @brief Direct call for `EVP_Q_mac` on OpenSSL 3.0+. */
#define HOOK_EVP_Q_mac(key, key_len, d, n, md, md_size, md_len)                \
  EVP_Q_mac(NULL, "HMAC", NULL, "SHA256", NULL, (key), (key_len), (d), (n),    \
            (md), (md_size), (md_len))
#else
/** @brief Direct call for `HMAC` on OpenSSL 1.1. */
#define HOOK_HMAC HMAC
#endif

#endif

/**
 * @brief Computes a 32-byte SHA-256 digest over a data buffer.
 *
 * @param[in] data Pointer to the input buffer to hash.
 * @param[in] data_len Length of the input buffer in bytes.
 * @param[out] out_digest Output buffer receiving the 32-byte SHA-256 digest.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t CDD_OPENSSL_CRYPTO_SHA256(const void *data, size_t data_len,
                                        unsigned char *out_digest) {
  EVP_MD_CTX *mdctx;
  const EVP_MD *md;
  const void *payload = data ? data : "";
  unsigned int len = 0;
  int ok;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  md = HOOK_EVP_sha256();
  if (!md)
    return CDD_C_ERROR_IO;

  mdctx = HOOK_EVP_MD_CTX_new();
  if (!mdctx) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  ok = HOOK_EVP_DigestInit_ex(mdctx, md, NULL);
  if (ok != 1) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  ok = HOOK_EVP_DigestUpdate(mdctx, payload, data_len);
  if (ok != 1) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  ok = HOOK_EVP_DigestFinal_ex(mdctx, out_digest, &len);
  if (ok != 1) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  if (len != CRYPTO_SHA256_SIZE) {
    rc = CDD_C_ERROR_IO;
  }

cleanup:
  EVP_MD_CTX_free(mdctx);
  return rc;
}

/**
 * @brief Computes a 32-byte HMAC-SHA256 message authentication code.
 *
 * @param[in] key Secret key bytes.
 * @param[in] key_len Length of `key` in bytes.
 * @param[in] data Message payload bytes to authenticate.
 * @param[in] data_len Length of `data` in bytes.
 * @param[out] out_mac Output buffer receiving the 32-byte HMAC-SHA256 tag.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t CDD_OPENSSL_CRYPTO_HMAC_SHA256(const void *key, size_t key_len,
                                             const void *data, size_t data_len,
                                             unsigned char *out_mac) {
  static const unsigned char zero_pad[16] = {0};
  const unsigned char *payload =
      data ? (const unsigned char *)data : (const unsigned char *)"";
  unsigned int len = 0;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  size_t mac_len = 0;
  unsigned char *mac_ptr;
#else
  unsigned char *result;
#endif

  if (!out_mac)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!key && key_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (key_len > (size_t)INT_MAX) {
    return CDD_C_ERROR_IO;
  }

  if (key_len == 0) {
    key = zero_pad;
    key_len = 16;
  }

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  mac_ptr = HOOK_EVP_Q_mac(key, key_len, payload, data_len, out_mac,
                           CRYPTO_SHA256_SIZE, &mac_len);
  if (!mac_ptr) {
    return CDD_C_ERROR_IO;
  }
  len = (unsigned int)mac_len;
#else
  result = HOOK_HMAC(HOOK_EVP_sha256(), key, (int)key_len, payload, data_len,
                     out_mac, &len);
  if (!result)
    return CDD_C_ERROR_IO;
#endif

  if (len != CRYPTO_SHA256_SIZE)
    return CDD_C_ERROR_IO;

  return CDD_C_SUCCESS;
}

#else /* defined(CDD_NO_OPENSSL) */

/**
 * @brief Computes a 32-byte SHA-256 digest (non-OpenSSL stub).
 *
 * @param[in] data Pointer to the input buffer to hash.
 * @param[in] data_len Length of the input buffer in bytes.
 * @param[out] out_digest Output buffer receiving the 32-byte SHA-256 digest.
 * @return `CDD_C_ERROR_SYSTEM` when compiled without OpenSSL.
 */
cdd_c_error_t CDD_OPENSSL_CRYPTO_SHA256(const void *data, size_t data_len,
                                        unsigned char *out_digest) {
  if (!out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return CDD_C_ERROR_SYSTEM;
}

/**
 * @brief Computes a 32-byte HMAC-SHA256 tag (non-OpenSSL stub).
 *
 * @param[in] key Secret key bytes.
 * @param[in] key_len Length of `key` in bytes.
 * @param[in] data Message payload bytes to authenticate.
 * @param[in] data_len Length of `data` in bytes.
 * @param[out] out_mac Output buffer receiving the 32-byte HMAC-SHA256 tag.
 * @return `CDD_C_ERROR_SYSTEM` when compiled without OpenSSL.
 */
cdd_c_error_t CDD_OPENSSL_CRYPTO_HMAC_SHA256(const void *key, size_t key_len,
                                             const void *data, size_t data_len,
                                             unsigned char *out_mac) {
  if ((!key && key_len > 0) || (!data && data_len > 0) || !out_mac)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return CDD_C_ERROR_SYSTEM;
}

#endif /* !defined(CDD_NO_OPENSSL) */
