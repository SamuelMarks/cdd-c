/**
 * @file crypto_wincrypt.c
 * @brief Windows Cryptography API (CAPI) implementation of the ACI.
 *
 * Uses `wincrypt.h` primitives to perform SHA-256 and HMAC-SHA-256.
 * Note: HMAC in CAPI requires the "Microsoft Enhanced RSA and AES Cryptographic
 * Provider" and explicit key import structures (PLAINTEXTKEYBLOB).
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "c_cddConfig.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
/* windef.h must precede winbase.h to prevent DWORD redefinition errors */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201 4214)
#endif
#include "win_compat_sym.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>

#include <wincrypt.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

#include "functions/parse/crypto_types.h"
/* clang-format on */

#ifdef _WIN32

/**
 * @brief Internal structure for importing plaintext keys into CAPI.
 * See:
 * https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-importing-a-plaintext-key
 */
struct PlainTextKeyBlob {
  /** @brief The blob header */
  BLOBHEADER hdr;
  /** @brief The size of the key data */
  DWORD cbKeySize;
  /** @brief Variable Length: Actual size allocated dynamically */
  BYTE rgbKeyData[1];
};

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
 * @brief Helper to acquire a cryptographic provider context.
 * Uses MS_ENH_RSA_AES_PROV for SHA-256 support.
 */
static enum cdd_c_error acquire_context(HCRYPTPROV *hProv) {
  if (!CryptAcquireContext(hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES,
                           CRYPT_VERIFYCONTEXT)) {
    /* If failed, try creating new keyset (rarely needed for VERIFYCONTEXT but
     * safe practice) */
    if (GetLastError() == (DWORD)NTE_BAD_KEYSET) {
      if (!CryptAcquireContext(hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES,
                               CRYPT_NEWKEYSET | CRYPT_VERIFYCONTEXT)) {
        return CDD_C_ERROR_IO;
      }
    } else {
      return CDD_C_ERROR_IO;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the crypto sha256 operation.
 */
enum cdd_c_error crypto_sha256(const void *data, size_t data_len,
                               unsigned char *out_digest) {
  HCRYPTPROV hProv = 0;
  HCRYPTHASH hHash = 0;
  DWORD cbHash = CRYPTO_SHA256_SIZE;
  int rc = 0;

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_sha256)
    return CDD_C_ERROR_IO;
#endif

  if ((!data && data_len > 0) || !out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (acquire_context(&hProv) != 0)
    return CDD_C_ERROR_IO;

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_mdctx_new) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }
  if (g_crypto_fail_digestinit) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }
#endif

  if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_digestupdate) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }
#endif

  if (data_len > 0) {
    if (!CryptHashData(hHash, (const BYTE *)data, (DWORD)data_len, 0)) {
      rc = CDD_C_ERROR_IO;
      goto cleanup;
    }
  }

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_digestfinal || g_crypto_fail_digestfinal_len) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }
#endif

  if (!CryptGetHashParam(hHash, HP_HASHVAL, out_digest, &cbHash, 0)) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

cleanup:
  if (hHash)
    CryptDestroyHash(hHash);
  if (hProv)
    CryptReleaseContext(hProv, 0);
  return rc;
}

/**
 * @brief Executes the crypto hmac sha256 operation.
 */
enum cdd_c_error crypto_hmac_sha256(const void *key, size_t key_len,
                                    const void *data, size_t data_len,
                                    unsigned char *out_mac) {
  HCRYPTPROV hProv = 0;
  HCRYPTHASH hHash = 0;
  HCRYPTKEY hKey = 0;
  struct PlainTextKeyBlob *pBlob = NULL;
  HMAC_INFO HmacInfo;
  DWORD cbHash = CRYPTO_SHA256_SIZE;
  DWORD blobSize;
  int rc = 0;

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_hmac || g_crypto_fail_hmac_len)
    return CDD_C_ERROR_IO;
#endif

  if ((!key && key_len > 0) || (!data && data_len > 0) || !out_mac)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* CAPI CryptImportKey fails with 0-length keys, and also small keys might
     trigger NTE_BAD_LEN. HMAC pads keys with 0s to the block size (64 for
     SHA256), so we can just use 64 zero bytes. */
  if (key_len == 0) {
    static const unsigned char zero_pad[16] = {0};
    key = zero_pad;
    key_len = 16;
  }

  if (acquire_context(&hProv) != 0)
    return CDD_C_ERROR_IO;

  /* 1. Import the Key */
  /* CAPI requires keys to be imported via blobs. We build a PLAINTEXTKEYBLOB */
  blobSize = (DWORD)(sizeof(BLOBHEADER) + sizeof(DWORD) + key_len);
  pBlob = (struct PlainTextKeyBlob *)malloc(blobSize);
  if (!pBlob) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  pBlob->hdr.bType = PLAINTEXTKEYBLOB;
  pBlob->hdr.bVersion = CUR_BLOB_VERSION;
  pBlob->hdr.reserved = 0;
  /* CALG_RC2 is required for plaintext import usually, even though we use it
   * for HMAC */
  pBlob->hdr.aiKeyAlg = CALG_RC2;
  pBlob->cbKeySize = (DWORD)key_len;
  if (key_len > 0) {
    memcpy(pBlob->rgbKeyData, key, key_len);
  }

  /* CRYPT_IPSEC_HMAC_KEY flag allows importing generic keys for HMAC */
  if (!CryptImportKey(hProv, (const BYTE *)pBlob, (DWORD)blobSize, 0,
                      CRYPT_IPSEC_HMAC_KEY, &hKey)) {
    rc = CDD_C_ERROR_IO; /* GetLastError() mapping */
    goto cleanup;
  }

  /* 2. Create HMAC Hash Object */
  if (!CryptCreateHash(hProv, CALG_HMAC, hKey, 0, &hHash)) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  /* 3. Set HMAC Info (Alg ID) */
  memset(&HmacInfo, 0, sizeof(HmacInfo));
  HmacInfo.HashAlgid = CALG_SHA_256;

  if (!CryptSetHashParam(hHash, HP_HMAC_INFO, (const BYTE *)&HmacInfo, 0)) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  /* 4. Hash Data */
  if (data_len > 0) {
    if (!CryptHashData(hHash, (const BYTE *)data, (DWORD)data_len, 0)) {
      rc = CDD_C_ERROR_IO;
      goto cleanup;
    }
  }

  /* 5. Get Result */
  if (!CryptGetHashParam(hHash, HP_HASHVAL, out_mac, &cbHash, 0)) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

cleanup:
  if (pBlob)
    free(pBlob);
  if (hHash)
    CryptDestroyHash(hHash);
  if (hKey)
    CryptDestroyKey(hKey);
  if (hProv)
    CryptReleaseContext(hProv, 0);

  return rc;
}

#else /* Non-Windows Support Stub */

/**
 * @brief Executes the crypto sha256 operation.
 */
enum cdd_c_error crypto_sha256(const void *data, size_t data_len,
                               unsigned char *out_digest) {
  (void)data;
  (void)data_len;
  (void)out_digest;
  return CDD_C_ERROR_SYSTEM;
}

/**
 * @brief Executes the crypto hmac sha256 operation.
 */
enum cdd_c_error crypto_hmac_sha256(const void *key, size_t key_len,
                                    const void *data, size_t data_len,
                                    unsigned char *out_mac) {
  (void)key;
  (void)key_len;
  (void)data;
  (void)data_len;
  (void)out_mac;
  return CDD_C_ERROR_SYSTEM;
}

#endif /* _WIN32 */
