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
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <string.h>

#include "functions/parse/crypto_types.h"
#include "c_cdd/log.h"
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

static const EVP_MD *get_hook_EVP_sha256(void) {
  if (g_crypto_fail_sha256)
    return NULL;
  return EVP_sha256();
}
#define HOOK_EVP_sha256 get_hook_EVP_sha256

static EVP_MD_CTX *get_hook_EVP_MD_CTX_new(void) {
  if (g_crypto_fail_mdctx_new)
    return NULL;
  return EVP_MD_CTX_new();
}
#define HOOK_EVP_MD_CTX_new get_hook_EVP_MD_CTX_new

static int calc_hook_EVP_DigestInit_ex(EVP_MD_CTX *ctx, const EVP_MD *type,
                                       ENGINE *impl) {
  if (g_crypto_fail_digestinit)
    return 0;
  return EVP_DigestInit_ex(ctx, type, impl);
}
#define HOOK_EVP_DigestInit_ex calc_hook_EVP_DigestInit_ex

static int calc_hook_EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *d,
                                      size_t cnt) {
  if (g_crypto_fail_digestupdate)
    return 0;
  return EVP_DigestUpdate(ctx, d, cnt);
}
#define HOOK_EVP_DigestUpdate calc_hook_EVP_DigestUpdate

static int calc_hook_EVP_DigestFinal_ex(EVP_MD_CTX *ctx, unsigned char *md,
                                        unsigned int *s) {
  if (g_crypto_fail_digestfinal)
    return 0;
  if (g_crypto_fail_digestfinal_len) {
    *s = 0;
    return 1;
  }
  return EVP_DigestFinal_ex(ctx, md, s);
}
#define HOOK_EVP_DigestFinal_ex calc_hook_EVP_DigestFinal_ex

static unsigned char *get_hook_HMAC(const EVP_MD *evp_md, const void *key,
                                    int key_len, const unsigned char *d,
                                    size_t n, unsigned char *md,
                                    unsigned int *md_len) {
  if (g_crypto_fail_hmac)
    return NULL;
  if (g_crypto_fail_hmac_len) {
    unsigned char *res = HMAC(evp_md, key, key_len, d, n, md, md_len);
    *md_len = 0;
    return res;
  }
  return HMAC(evp_md, key, key_len, d, n, md, md_len);
}
#define HOOK_HMAC get_hook_HMAC

#else

#define HOOK_EVP_sha256() EVP_sha256()
#define HOOK_EVP_MD_CTX_new() EVP_MD_CTX_new()
#define HOOK_EVP_DigestInit_ex EVP_DigestInit_ex
#define HOOK_EVP_DigestUpdate EVP_DigestUpdate
#define HOOK_EVP_DigestFinal_ex EVP_DigestFinal_ex
#define HOOK_HMAC HMAC

#endif

/**
 * @brief Executes the crypto sha256 operation.
 */
enum cdd_c_error crypto_sha256(const void *data, size_t data_len,
                               unsigned char *out_digest) {
  EVP_MD_CTX *mdctx;
  const EVP_MD *md;
  unsigned int len = 0;
  int rc = 0;

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
 * @brief Executes the crypto hmac sha256 operation.
 */
enum cdd_c_error crypto_hmac_sha256(const void *key, size_t key_len,
                                    const void *data, size_t data_len,
                                    unsigned char *out_mac) {
  unsigned int len = 0;

  if ((!key && key_len > 0) || (!data && data_len > 0) || !out_mac) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
