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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <wincrypt.h>
#include <windows.h>

#endif

#include "functions/parse_crypto_types.h"

#ifdef _WIN32

/*
 * Internal structure for importing plaintext keys into CAPI.
 * See:
 * https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-importing-a-plaintext-key
 */
struct PlainTextKeyBlob {
  BLOBHEADER hdr;
  DWORD cbKeySize;
  BYTE rgbKeyData[1]; /* Variable Length: Actual size allocated dynamically */
};

/**
 * @brief Helper to acquire a cryptographic provider context.
 * Uses MS_ENH_RSA_AES_PROV for SHA-256 support.
 */
static int acquire_context(HCRYPTPROV *hProv) {
  if (!CryptAcquireContext(hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES,
                           CRYPT_VERIFYCONTEXT)) {
    /* If failed, try creating new keyset (rarely needed for VERIFYCONTEXT but
     * safe practice) */
    if (GetLastError() == NTE_BAD_KEYSET) {
      if (!CryptAcquireContext(hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES,
                               CRYPT_NEWKEYSET | CRYPT_VERIFYCONTEXT)) {
        return EIO;
      }
    } else {
      return EIO;
    }
  }
  return 0;
}

int crypto_sha256(const void *data, size_t data_len,
                  unsigned char *out_digest) {
  HCRYPTPROV hProv = 0;
  HCRYPTHASH hHash = 0;
  DWORD cbHash = CRYPTO_SHA256_SIZE;
  int rc = 0;

  if ((!data && data_len > 0) || !out_digest)
    return EINVAL;

  if (acquire_context(&hProv) != 0)
    return EIO;

  if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
    rc = EIO;
    goto cleanup;
  }

  if (data_len > 0) {
    if (!CryptHashData(hHash, (const BYTE *)data, (DWORD)data_len, 0)) {
      rc = EIO;
      goto cleanup;
    }
  }

  if (!CryptGetHashParam(hHash, HP_HASHVAL, out_digest, &cbHash, 0)) {
    rc = EIO;
    goto cleanup;
  }

cleanup:
  if (hHash)
    CryptDestroyHash(hHash);
  if (hProv)
    CryptReleaseContext(hProv, 0);
  return rc;
}

int crypto_hmac_sha256(const void *key, size_t key_len, const void *data,
                       size_t data_len, unsigned char *out_mac) {
  HCRYPTPROV hProv = 0;
  HCRYPTHASH hHash = 0;
  HCRYPTKEY hKey = 0;
  struct PlainTextKeyBlob *pBlob = NULL;
  HMAC_INFO HmacInfo;
  DWORD cbHash = CRYPTO_SHA256_SIZE;
  DWORD blobSize;
  int rc = 0;

  if ((!key && key_len > 0) || (!data && data_len > 0) || !out_mac)
    return EINVAL;

  if (acquire_context(&hProv) != 0)
    return EIO;

  /* 1. Import the Key */
  /* CAPI requires keys to be imported via blobs. We build a PLAINTEXTKEYBLOB */
  blobSize = sizeof(BLOBHEADER) + sizeof(DWORD) + key_len;
  pBlob = (struct PlainTextKeyBlob *)malloc(blobSize);
  if (!pBlob) {
    rc = ENOMEM;
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
    rc = EIO; /* GetLastError() mapping */
    goto cleanup;
  }

  /* 2. Create HMAC Hash Object */
  if (!CryptCreateHash(hProv, CALG_HMAC, hKey, 0, &hHash)) {
    rc = EIO;
    goto cleanup;
  }

  /* 3. Set HMAC Info (Alg ID) */
  memset(&HmacInfo, 0, sizeof(HmacInfo));
  HmacInfo.HashAlgid = CALG_SHA_256;

  if (!CryptSetHashParam(hHash, HP_HMAC_INFO, (const BYTE *)&HmacInfo, 0)) {
    rc = EIO;
    goto cleanup;
  }

  /* 4. Hash Data */
  if (data_len > 0) {
    if (!CryptHashData(hHash, (const BYTE *)data, (DWORD)data_len, 0)) {
      rc = EIO;
      goto cleanup;
    }
  }

  /* 5. Get Result */
  if (!CryptGetHashParam(hHash, HP_HASHVAL, out_mac, &cbHash, 0)) {
    rc = EIO;
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

int crypto_sha256(const void *data, size_t data_len,
                  unsigned char *out_digest) {
  (void)data;
  (void)data_len;
  (void)out_digest;
  return ENOTSUP;
}

int crypto_hmac_sha256(const void *key, size_t key_len, const void *data,
                       size_t data_len, unsigned char *out_mac) {
  (void)key;
  (void)key_len;
  (void)data;
  (void)data_len;
  (void)out_mac;
  return ENOTSUP;
}

#endif /* _WIN32 */
