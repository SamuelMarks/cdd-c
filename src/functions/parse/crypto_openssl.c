/**
 * @file crypto_openssl.c
 * @brief OpenSSL implementation of the Abstract Crypto Interface.
 *
 * Uses the OpenSSL EVP API (High-level envelope interface) to ensure
 * compatibility with OpenSSL 1.1 and 3.0+.
 *
 * Compile definitions:
 * - Requires linking against `libssl` and `libcrypto`.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <string.h>

#include "c_cdd/log.h"
#include "functions/parse/crypto_types.h"
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

#define HOOK_EVP_sha256() (g_crypto_fail_sha256 ? NULL : EVP_sha256())
#define HOOK_EVP_MD_CTX_new()                                                  \
  (g_crypto_fail_mdctx_new ? NULL : EVP_MD_CTX_new())
#define HOOK_EVP_DigestInit_ex(ctx, type, impl)                                \
  (g_crypto_fail_digestinit ? 0 : EVP_DigestInit_ex((ctx), (type), (impl)))
#define HOOK_EVP_DigestUpdate(ctx, d, cnt)                                     \
  (g_crypto_fail_digestupdate ? 0 : EVP_DigestUpdate((ctx), (d), (cnt)))
#define HOOK_EVP_DigestFinal_ex(ctx, md, s)                                    \
  (g_crypto_fail_digestfinal ? 0                                               \
                             : (g_crypto_fail_digestfinal_len                  \
                                    ? (*(s) = 0, 1)                            \
                                    : EVP_DigestFinal_ex((ctx), (md), (s))))
#define HOOK_HMAC(evp_md, key, key_len, d, n, md, md_len)                      \
  (g_crypto_fail_hmac                                                          \
       ? NULL                                                                  \
       : (g_crypto_fail_hmac_len                                               \
              ? (HMAC((evp_md), (key), (key_len), (d), (n), (md), (md_len)),   \
                 *(md_len) = 0, (md))                                          \
              : HMAC((evp_md), (key), (key_len), (d), (n), (md), (md_len))))

#else

/** @brief Hook or direct call for EVP_sha256 */
#define HOOK_EVP_sha256() EVP_sha256()
/** @brief Hook or direct call for EVP_MD_CTX_new */
#define HOOK_EVP_MD_CTX_new() EVP_MD_CTX_new()
/** @brief Hook or direct call for EVP_DigestInit_ex */
#define HOOK_EVP_DigestInit_ex EVP_DigestInit_ex
/** @brief Hook or direct call for EVP_DigestUpdate */
#define HOOK_EVP_DigestUpdate EVP_DigestUpdate
/** @brief Hook or direct call for EVP_DigestFinal_ex */
#define HOOK_EVP_DigestFinal_ex EVP_DigestFinal_ex
/** @brief Hook or direct call for HMAC */
#define HOOK_HMAC HMAC

#endif

/**
 * @brief Computes a 32-byte SHA-256 digest over a data buffer.
 */
cdd_c_error_t crypto_sha256(const void *data, size_t data_len,
                            unsigned char *out_digest) {
  EVP_MD_CTX *mdctx;
  const EVP_MD *md;
  unsigned int len = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if ((!data && data_len > 0) || !out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  md = HOOK_EVP_sha256();
  if (!md)
    return CDD_C_ERROR_IO;

  mdctx = HOOK_EVP_MD_CTX_new();
  if (!mdctx) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  if (HOOK_EVP_DigestInit_ex(mdctx, md, NULL) != 1) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  if (HOOK_EVP_DigestUpdate(mdctx, data, data_len) != 1) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  if (HOOK_EVP_DigestFinal_ex(mdctx, out_digest, &len) != 1) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  if (len != CRYPTO_SHA256_SIZE) {
    rc = CDD_C_ERROR_IO; /* Should not happen for SHA256 */
  }

cleanup:
  EVP_MD_CTX_free(mdctx);
  return rc;
}

/**
 * @brief Computes a 32-byte HMAC-SHA256 digest over a data buffer.
 */
cdd_c_error_t crypto_hmac_sha256(const void *key, size_t key_len,
                                 const void *data, size_t data_len,
                                 unsigned char *out_mac) {
  unsigned int len = 0;

  if ((!key && key_len > 0) || (!data && data_len > 0) || !out_mac) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if ((int)key_len < 0) {
    return CDD_C_ERROR_IO;
  }

  if (key_len == 0) {
    static const unsigned char zero_pad[16] = {0};
    key = zero_pad;
    key_len = 16;
  }

  /*
   * Using high-level HMAC function for simplicity and C89 compatibility.
   * HMAC() is deprecated in OpenSSL 3.0 in favor of EVP_MAC, but still widely
   * available and simpler for a single-shot wrapper without managing
   * comprehensive EVP_MAC_CTX state.
   */
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  /* OpenSSL 3.0+ EVP Pkey Logic or simple HMAC if not compiled out.
     If strict deprecation warnings are on, this needs EVP_MAC implementation.
     For simplicity in C89/Portable context, standard HMAC is used if avail. */
  if (!HOOK_HMAC(HOOK_EVP_sha256(), key, (int)key_len,
                 (const unsigned char *)data, data_len, out_mac, &len)) {
    return CDD_C_ERROR_IO;
  }
#else
  /* OpenSSL 1.1 */
  {
    unsigned char *result =
        HOOK_HMAC(HOOK_EVP_sha256(), key, (int)key_len,
                  (const unsigned char *)data, data_len, out_mac, &len);
    if (!result)
      return CDD_C_ERROR_IO;
  }
#endif

  if (len != CRYPTO_SHA256_SIZE)
    return CDD_C_ERROR_IO;

  return CDD_C_SUCCESS;
}
