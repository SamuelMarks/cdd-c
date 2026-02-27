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

#include <errno.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <string.h>

#include "functions/parse_crypto_types.h"

int crypto_sha256(const void *data, size_t data_len,
                  unsigned char *out_digest) {
  EVP_MD_CTX *mdctx;
  const EVP_MD *md;
  unsigned int len = 0;
  int rc = 0;

  if (!data && data_len > 0)
    return EINVAL;
  if (!out_digest)
    return EINVAL;

  md = EVP_sha256();
  if (!md)
    return EIO;

  mdctx = EVP_MD_CTX_new();
  if (!mdctx)
    return ENOMEM;

  if (EVP_DigestInit_ex(mdctx, md, NULL) != 1) {
    rc = EIO;
    goto cleanup;
  }

  if (EVP_DigestUpdate(mdctx, data, data_len) != 1) {
    rc = EIO;
    goto cleanup;
  }

  if (EVP_DigestFinal_ex(mdctx, out_digest, &len) != 1) {
    rc = EIO;
    goto cleanup;
  }

  if (len != CRYPTO_SHA256_SIZE) {
    rc = EIO; /* Should not happen for SHA256 */
  }

cleanup:
  EVP_MD_CTX_free(mdctx);
  return rc;
}

int crypto_hmac_sha256(const void *key, size_t key_len, const void *data,
                       size_t data_len, unsigned char *out_mac) {
  unsigned int len = 0;

  if ((!key && key_len > 0) || (!data && data_len > 0) || !out_mac) {
    return EINVAL;
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
  if (!HMAC(EVP_sha256(), key, (int)key_len, (const unsigned char *)data,
            data_len, out_mac, &len)) {
    return EIO;
  }
#else
  /* OpenSSL 1.1 */
  result = HMAC(EVP_sha256(), key, (int)key_len, (const unsigned char *)data,
                data_len, out_mac, &len);
  if (!result)
    return EIO;
#endif

  if (len != CRYPTO_SHA256_SIZE)
    return EIO;

  return 0;
}
