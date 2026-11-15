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
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "c_cddConfig.h"
#ifndef WIN32_LEAN_AND_MEAN
/** @brief Exclude rarely-used stuff from Windows headers. */
#define WIN32_LEAN_AND_MEAN
#endif
/* windef.h must precede winbase.h to prevent DWORD redefinition errors */
#include "win_compat_sym.h"
#include <winsock2.h>
#include <wincrypt.h>
#endif

#include "c_cdd/memory.h"
#include "functions/parse/crypto_types.h"
/* clang-format on */

#ifdef CDD_CRYPTO_WINCRYPT_STANDALONE_TEST
/** @brief Standalone test symbol name for WinCrypt SHA-256. */
#define CDD_WINCRYPT_CRYPTO_SHA256 cdd_wincrypt_crypto_sha256
/** @brief Standalone test symbol name for WinCrypt HMAC-SHA-256. */
#define CDD_WINCRYPT_CRYPTO_HMAC_SHA256 cdd_wincrypt_crypto_hmac_sha256
/**
 * @brief Standalone test entrypoint `cdd_wincrypt_crypto_sha256`.
 * @param data Parameter `data`.
 * @param data_len Parameter `data_len`.
 * @param out_digest Parameter `out_digest`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_wincrypt_crypto_sha256(const void *data,
                                                    size_t data_len,
                                                    unsigned char *out_digest);
/**
 * @brief Standalone test entrypoint `cdd_wincrypt_crypto_hmac_sha256`.
 * @param key Parameter `key`.
 * @param key_len Parameter `key_len`.
 * @param data Parameter `data`.
 * @param data_len Parameter `data_len`.
 * @param out_mac Parameter `out_mac`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_wincrypt_crypto_hmac_sha256(
    const void *key, size_t key_len, const void *data, size_t data_len,
    unsigned char *out_mac);
#else
/** @brief Production symbol name for WinCrypt SHA-256. */
#define CDD_WINCRYPT_CRYPTO_SHA256 crypto_sha256
/** @brief Production symbol name for WinCrypt HMAC-SHA-256. */
#define CDD_WINCRYPT_CRYPTO_HMAC_SHA256 crypto_hmac_sha256
#endif

#ifdef _WIN32

/**
 * @brief Internal structure for importing plaintext keys into CAPI.
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
#endif

/**
 * @brief Helper to acquire a cryptographic provider context.
 * Uses MS_ENH_RSA_AES_PROV for SHA-256 support.
 *
 * @param[out] hProv Pointer receiving the acquired `HCRYPTPROV` handle.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
 */
static NO_DISCARD cdd_c_error_t acquire_context(HCRYPTPROV *hProv) {
  if (!CryptAcquireContext(hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES,
                           CRYPT_VERIFYCONTEXT)) {
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
 * @brief Computes a 32-byte SHA-256 digest over a data buffer.
 *
 * @param[in] data Pointer to the input buffer to hash.
 * @param[in] data_len Length of the input buffer in bytes.
 * @param[out] out_digest Output buffer receiving the 32-byte SHA-256 digest.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t CDD_WINCRYPT_CRYPTO_SHA256(const void *data, size_t data_len,
                                         unsigned char *out_digest) {
  HCRYPTPROV hProv = 0;
  HCRYPTHASH hHash = 0;
  DWORD cbHash = CRYPTO_SHA256_SIZE;
  cdd_c_error_t rc = CDD_C_SUCCESS;

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_sha256)
    return CDD_C_ERROR_IO;
#endif

  if (!out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = acquire_context(&hProv);
  if (rc != CDD_C_SUCCESS)
    return rc;

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
 * @brief Computes a 32-byte HMAC-SHA256 digest over a data buffer.
 *
 * @param[in] key Secret key bytes.
 * @param[in] key_len Length of `key` in bytes.
 * @param[in] data Message payload bytes to authenticate.
 * @param[in] data_len Length of `data` in bytes.
 * @param[out] out_mac Output buffer receiving the 32-byte HMAC-SHA256 tag.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t CDD_WINCRYPT_CRYPTO_HMAC_SHA256(const void *key, size_t key_len,
                                              const void *data, size_t data_len,
                                              unsigned char *out_mac) {
  HCRYPTPROV hProv = 0;
  HCRYPTHASH hHash = 0;
  HCRYPTKEY hKey = 0;
  struct PlainTextKeyBlob *pBlob = NULL;
  HMAC_INFO HmacInfo;
  DWORD cbHash = CRYPTO_SHA256_SIZE;
  DWORD blobSize;
  cdd_c_error_t rc = CDD_C_SUCCESS;

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_hmac || g_crypto_fail_hmac_len)
    return CDD_C_ERROR_IO;
#endif

  if (!out_mac)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!key && key_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (key_len == 0) {
    static const unsigned char zero_pad[16] = {0};
    key = zero_pad;
    key_len = 16;
  }

  rc = acquire_context(&hProv);
  if (rc != CDD_C_SUCCESS)
    return rc;

  blobSize = (DWORD)(sizeof(BLOBHEADER) + sizeof(DWORD) + key_len);
  pBlob = (struct PlainTextKeyBlob *)C_CDD_MALLOC(blobSize);
  if (!pBlob) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  pBlob->hdr.bType = PLAINTEXTKEYBLOB;
  pBlob->hdr.bVersion = CUR_BLOB_VERSION;
  pBlob->hdr.reserved = 0;
  pBlob->hdr.aiKeyAlg = CALG_RC2;
  pBlob->cbKeySize = (DWORD)key_len;
  if (key_len > 0) {
    memcpy(pBlob->rgbKeyData, key, key_len);
  }

  if (!CryptImportKey(hProv, (const BYTE *)pBlob, (DWORD)blobSize, 0,
                      CRYPT_IPSEC_HMAC_KEY, &hKey)) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  if (!CryptCreateHash(hProv, CALG_HMAC, hKey, 0, &hHash)) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  memset(&HmacInfo, 0, sizeof(HmacInfo));
  HmacInfo.HashAlgid = CALG_SHA_256;

  if (!CryptSetHashParam(hHash, HP_HMAC_INFO, (const BYTE *)&HmacInfo, 0)) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  if (data_len > 0) {
    if (!CryptHashData(hHash, (const BYTE *)data, (DWORD)data_len, 0)) {
      rc = CDD_C_ERROR_IO;
      goto cleanup;
    }
  }

  if (!CryptGetHashParam(hHash, HP_HASHVAL, out_mac, &cbHash, 0)) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

cleanup:
  if (pBlob)
    C_CDD_FREE(pBlob);
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
 * @brief Computes a 32-byte SHA-256 digest over a data buffer (non-Windows
 * stub).
 *
 * @param[in] data Pointer to the input buffer to hash.
 * @param[in] data_len Length of the input buffer in bytes.
 * @param[out] out_digest Output buffer receiving the 32-byte SHA-256 digest.
 * @return `CDD_C_ERROR_SYSTEM` when compiled without Windows CAPI.
 */
cdd_c_error_t CDD_WINCRYPT_CRYPTO_SHA256(const void *data, size_t data_len,
                                         unsigned char *out_digest) {
  if (!out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return CDD_C_ERROR_SYSTEM;
}

/**
 * @brief Computes a 32-byte HMAC-SHA256 digest over a data buffer (non-Windows
 * stub).
 *
 * @param[in] key Secret key bytes.
 * @param[in] key_len Length of `key` in bytes.
 * @param[in] data Message payload bytes to authenticate.
 * @param[in] data_len Length of `data` in bytes.
 * @param[out] out_mac Output buffer receiving the 32-byte HMAC-SHA256 tag.
 * @return `CDD_C_ERROR_SYSTEM` when compiled without Windows CAPI.
 */
cdd_c_error_t CDD_WINCRYPT_CRYPTO_HMAC_SHA256(const void *key, size_t key_len,
                                              const void *data, size_t data_len,
                                              unsigned char *out_mac) {
  if (!out_mac)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!key && key_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return CDD_C_ERROR_SYSTEM;
}

#endif /* _WIN32 */
