/**
 * @file crypto_types.h
 * @brief Abstract Crypto Interface (ACI) Definitions.
 *
 * Provides a unified interface for cryptographic primitives required by
 * API Client generation (specifically Hashing and HMAC signing).
 * By implementing this interface, the generated client code remains agnostic
 * to the underlying SSL library (OpenSSL vs WinCrypt).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CRYPTO_TYPES_H
#define C_CDD_CRYPTO_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include <stddef.h>

/**
 * @brief Size of a SHA-256 Digest in bytes.
 */
#define CRYPTO_SHA256_SIZE 32

/**
 * @brief Compute the SHA-256 hash of a data buffer.
 *
 * @param[in] data Pointer to the input data.
 * @param[in] data_len Length of the input data in bytes.
 * @param[out] out_digest Output buffer. Must be at least `CRYPTO_SHA256_SIZE`
 * bytes.
 * @return 0 on success, error code on failure (EINVAL, ENOMEM, EIO).
 */
extern C_CDD_EXPORT int crypto_sha256(const void *data, size_t data_len,
                                      unsigned char *out_digest);

/**
 * @brief Compute the HMAC-SHA-256 signature of a data buffer.
 *
 * Performs Keyed-Hashing for Message Authentication (HMAC) using the SHA-256
 * digest algorithm.
 *
 * @param[in] key Pointer to the secret key.
 * @param[in] key_len Length of the secret key in bytes.
 * @param[in] data Pointer to the input data to sign.
 * @param[in] data_len Length of the input data in bytes.
 * @param[out] out_mac Output buffer. Must be at least `CRYPTO_SHA256_SIZE`
 * bytes.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int crypto_hmac_sha256(const void *key, size_t key_len,
                                           const void *data, size_t data_len,
                                           unsigned char *out_mac);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CRYPTO_TYPES_H */
