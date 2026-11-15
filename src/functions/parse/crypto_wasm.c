/**
 * @file crypto_wasm.c
 * @brief WASM Crypto implementation (Native C Fallback + Node/JS Sync wrapper).
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "functions/parse/crypto_types.h"
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#include <stdint.h>
#endif
#include <string.h>

#ifdef CDD_BUILD_TESTS
#include "c_cdd_export.h"
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
/** @brief External test variable `g_crypto_wasm_fail_step`. */
extern C_CDD_EXPORT int g_crypto_wasm_fail_step;
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
/* clang-format on */

#ifdef CDD_CRYPTO_WASM_STANDALONE_TEST
/** @brief Standalone test symbol name for WASM SHA-256. */
#define CDD_WASM_CRYPTO_SHA256 cdd_wasm_crypto_sha256
/** @brief Standalone test symbol name for WASM HMAC-SHA-256. */
#define CDD_WASM_CRYPTO_HMAC_SHA256 cdd_wasm_crypto_hmac_sha256
/**
 * @brief Standalone test entrypoint `cdd_wasm_crypto_sha256`.
 * @param data Parameter `data`.
 * @param data_len Parameter `data_len`.
 * @param out_digest Parameter `out_digest`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_wasm_crypto_sha256(const void *data,
                                                size_t data_len,
                                                unsigned char *out_digest);
/**
 * @brief Standalone test entrypoint `cdd_wasm_crypto_hmac_sha256`.
 * @param key Parameter `key`.
 * @param key_len Parameter `key_len`.
 * @param data Parameter `data`.
 * @param data_len Parameter `data_len`.
 * @param out_mac Parameter `out_mac`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_wasm_crypto_hmac_sha256(const void *key,
                                                     size_t key_len,
                                                     const void *data,
                                                     size_t data_len,
                                                     unsigned char *out_mac);
#else
/** @brief Production symbol name for WASM SHA-256. */
#define CDD_WASM_CRYPTO_SHA256 crypto_sha256
/** @brief Production symbol name for WASM HMAC-SHA-256. */
#define CDD_WASM_CRYPTO_HMAC_SHA256 crypto_hmac_sha256
#endif

#if defined(__EMSCRIPTEN__) && !defined(CDD_CRYPTO_WASM_STANDALONE_TEST)
/* Synchronous SHA-256 for Node/Deno/Bun using EM_JS */
EM_JS(int, js_crypto_sha256, (const uint8_t *data, size_t len, uint8_t *out), {
  if (typeof process != 'undefined' && process.versions &&
      process.versions.node) {
    try {
      var crypto = require('crypto');
      var hash = crypto.createHash('sha256');
      hash.update(new Uint8Array(HEAPU8.buffer, data, len));
      var digest = hash.digest();
      HEAPU8.set(digest, out);
      return 1;
    } catch (e) {
      return 0;
    }
  }
  return 0;
})

/* Synchronous HMAC-SHA-256 for Node/Deno/Bun using EM_JS */
EM_JS(int, js_crypto_hmac_sha256,
      (const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len,
       uint8_t *out),
      {
        if (typeof process != 'undefined' && process.versions &&
            process.versions.node) {
          try {
            var crypto = require('crypto');
            var hmac = crypto.createHmac(
                'sha256', new Uint8Array(HEAPU8.buffer, key, key_len));
            hmac.update(new Uint8Array(HEAPU8.buffer, data, data_len));
            var digest = hmac.digest();
            HEAPU8.set(digest, out);
            return 1;
          } catch (e) {
            return 0;
          }
        }
        return 0;
      })
#endif

/**
 * @brief SHA-256 state context for native fallback.
 */
struct cdd_sha256_ctx {
  uint32_t state[8]; /**< State intermediate hash values */
  uint64_t bitlen;   /**< Total message length in bits */
  uint8_t data[64];  /**< 512-bit message chunk buffer */
  uint32_t datalen;  /**< Number of bytes currently in buffer */
};

/** @brief SHA-256 round constants K[0..63]. */
static const uint32_t cdd_k[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
    0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
    0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
    0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
    0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
    0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
    0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
    0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
    0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
    0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL};

/** @brief Bitwise right rotation macro. */
#define CDD_ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))
/** @brief SHA-256 Ch logical function. */
#define CDD_CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
/** @brief SHA-256 Maj logical function. */
#define CDD_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
/** @brief SHA-256 Sigma0 function. */
#define CDD_EP0(x)                                                             \
  (CDD_ROTRIGHT(x, 2) ^ CDD_ROTRIGHT(x, 13) ^ CDD_ROTRIGHT(x, 22))
/** @brief SHA-256 Sigma1 function. */
#define CDD_EP1(x)                                                             \
  (CDD_ROTRIGHT(x, 6) ^ CDD_ROTRIGHT(x, 11) ^ CDD_ROTRIGHT(x, 25))
/** @brief SHA-256 sigma0 function. */
#define CDD_SIG0(x) (CDD_ROTRIGHT(x, 7) ^ CDD_ROTRIGHT(x, 18) ^ ((x) >> 3))
/** @brief SHA-256 sigma1 function. */
#define CDD_SIG1(x) (CDD_ROTRIGHT(x, 17) ^ CDD_ROTRIGHT(x, 19) ^ ((x) >> 10))

/**
 * @brief Compresses a single 512-bit (64-byte) block into the SHA-256 state.
 *
 * @param[in,out] ctx SHA-256 state context.
 * @param[in] data 64-byte input message block.
 * @return `CDD_C_SUCCESS` on success, or an error code on fault injection.
 */
static NO_DISCARD cdd_c_error_t cdd_sha256_transform(struct cdd_sha256_ctx *ctx,
                                                     const uint8_t data[]) {
  uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

#ifdef CDD_BUILD_TESTS
  if (g_crypto_wasm_fail_step > 0 && --g_crypto_wasm_fail_step == 0)
    return CDD_C_ERROR_IO;
#endif

  for (i = 0, j = 0; i < 16; ++i, j += 4)
    m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j + 1] << 16) |
           ((uint32_t)data[j + 2] << 8) | ((uint32_t)data[j + 3]);
  for (; i < 64; ++i)
    m[i] = CDD_SIG1(m[i - 2]) + m[i - 7] + CDD_SIG0(m[i - 15]) + m[i - 16];

  a = ctx->state[0];
  b = ctx->state[1];
  c = ctx->state[2];
  d = ctx->state[3];
  e = ctx->state[4];
  f = ctx->state[5];
  g = ctx->state[6];
  h = ctx->state[7];

  for (i = 0; i < 64; ++i) {
    t1 = h + CDD_EP1(e) + CDD_CH(e, f, g) + cdd_k[i] + m[i];
    t2 = CDD_EP0(a) + CDD_MAJ(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }

  ctx->state[0] += a;
  ctx->state[1] += b;
  ctx->state[2] += c;
  ctx->state[3] += d;
  ctx->state[4] += e;
  ctx->state[5] += f;
  ctx->state[6] += g;
  ctx->state[7] += h;
  return CDD_C_SUCCESS;
}

/**
 * @brief Initializes a `cdd_sha256_ctx` state structure with standard IVs.
 *
 * @param[out] ctx SHA-256 context to initialize.
 * @return `CDD_C_SUCCESS` on success, or an error code on fault injection.
 */
static NO_DISCARD cdd_c_error_t cdd_sha256_init(struct cdd_sha256_ctx *ctx) {
#ifdef CDD_BUILD_TESTS
  if (g_crypto_wasm_fail_step > 0 && --g_crypto_wasm_fail_step == 0)
    return CDD_C_ERROR_IO;
#endif
  ctx->datalen = 0;
  ctx->bitlen = 0;
  ctx->state[0] = 0x6a09e667UL;
  ctx->state[1] = 0xbb67ae85UL;
  ctx->state[2] = 0x3c6ef372UL;
  ctx->state[3] = 0xa54ff53aUL;
  ctx->state[4] = 0x510e527fUL;
  ctx->state[5] = 0x9b05688cUL;
  ctx->state[6] = 0x1f83d9abUL;
  ctx->state[7] = 0x5be0cd19UL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Feeds a message byte slice into an active SHA-256 state context.
 *
 * @param[in,out] ctx Active SHA-256 context.
 * @param[in] data Input message bytes.
 * @param[in] len Number of bytes in `data`.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static NO_DISCARD cdd_c_error_t cdd_sha256_update(struct cdd_sha256_ctx *ctx,
                                                  const uint8_t data[],
                                                  size_t len) {
  size_t i;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_crypto_wasm_fail_step > 0 && --g_crypto_wasm_fail_step == 0)
    return CDD_C_ERROR_IO;
#endif
  for (i = 0; i < len; ++i) {
    ctx->data[ctx->datalen] = data[i];
    ctx->datalen++;
    if (ctx->datalen == 64) {
      rc = cdd_sha256_transform(ctx, ctx->data);
      if (rc != CDD_C_SUCCESS)
        return rc;
      ctx->bitlen += 512;
      ctx->datalen = 0;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Finalizes SHA-256 padding and writes the 32-byte digest into `hash`.
 *
 * @param[in,out] ctx Active SHA-256 context.
 * @param[out] hash 32-byte output buffer receiving the big-endian digest.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static NO_DISCARD cdd_c_error_t cdd_sha256_final(struct cdd_sha256_ctx *ctx,
                                                 uint8_t hash[]) {
  uint32_t i;
  cdd_c_error_t rc;

  i = ctx->datalen;
  if (ctx->datalen < 56) {
    ctx->data[i++] = 0x80;
    while (i < 56)
      ctx->data[i++] = 0x00;
  } else {
    ctx->data[i++] = 0x80;
    while (i < 64)
      ctx->data[i++] = 0x00;
    rc = cdd_sha256_transform(ctx, ctx->data);
    if (rc != CDD_C_SUCCESS)
      return rc;
    memset(ctx->data, 0, 56);
  }
  ctx->bitlen += (uint64_t)ctx->datalen * 8;
  ctx->data[63] = (uint8_t)(ctx->bitlen);
  ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
  ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
  ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
  ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
  ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
  ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
  ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
  rc = cdd_sha256_transform(ctx, ctx->data);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < 4; ++i) {
    hash[i] = (uint8_t)((ctx->state[0] >> (24 - i * 8)) & 0x000000ffUL);
    hash[i + 4] = (uint8_t)((ctx->state[1] >> (24 - i * 8)) & 0x000000ffUL);
    hash[i + 8] = (uint8_t)((ctx->state[2] >> (24 - i * 8)) & 0x000000ffUL);
    hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i * 8)) & 0x000000ffUL);
    hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i * 8)) & 0x000000ffUL);
    hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i * 8)) & 0x000000ffUL);
    hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i * 8)) & 0x000000ffUL);
    hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i * 8)) & 0x000000ffUL);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes a 32-byte SHA-256 digest in WebAssembly or pure-C fallback
 * mode.
 *
 * @param[in] data Input message buffer.
 * @param[in] data_len Length of input message in bytes.
 * @param[out] out_digest 32-byte output buffer receiving the SHA-256 digest.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t CDD_WASM_CRYPTO_SHA256(const void *data, size_t data_len,
                                     unsigned char *out_digest) {
  const uint8_t *d;
  struct cdd_sha256_ctx ctx;
  cdd_c_error_t rc;

  if (!out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  d = data ? (const uint8_t *)data : (const uint8_t *)"";

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_sha256)
    return CDD_C_ERROR_IO;
  if (g_crypto_fail_mdctx_new)
    return CDD_C_ERROR_MEMORY;
  if (g_crypto_fail_digestinit || g_crypto_fail_digestupdate ||
      g_crypto_fail_digestfinal || g_crypto_fail_digestfinal_len)
    return CDD_C_ERROR_IO;
#endif

#if defined(__EMSCRIPTEN__) && !defined(CDD_CRYPTO_WASM_STANDALONE_TEST)
  if (js_crypto_sha256(d, data_len, out_digest)) {
    return CDD_C_SUCCESS;
  }
#endif

  rc = cdd_sha256_init(&ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (data_len > 0) {
    rc = cdd_sha256_update(&ctx, d, data_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  rc = cdd_sha256_final(&ctx, out_digest);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes HMAC-SHA-256 in WebAssembly or pure-C fallback environment.
 *
 * @param[in] key Secret key buffer.
 * @param[in] key_len Length of secret key in bytes.
 * @param[in] data Input message buffer.
 * @param[in] data_len Length of input message in bytes.
 * @param[out] out_mac 32-byte output buffer receiving the HMAC-SHA-256 digest.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t CDD_WASM_CRYPTO_HMAC_SHA256(const void *key, size_t key_len,
                                          const void *data, size_t data_len,
                                          unsigned char *out_mac) {
  const uint8_t *k = key ? (const uint8_t *)key : (const uint8_t *)"";
  const uint8_t *d = data ? (const uint8_t *)data : (const uint8_t *)"";
  struct cdd_sha256_ctx ctx;
  uint8_t k_ipad[64];
  uint8_t k_opad[64];
  uint8_t tk[32];
  uint8_t out_digest[32];
  size_t i;
  cdd_c_error_t rc;

  if (!out_mac)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!key && key_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!data && data_len > 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_crypto_fail_hmac || g_crypto_fail_hmac_len)
    return CDD_C_ERROR_IO;
#endif

#if defined(__EMSCRIPTEN__) && !defined(CDD_CRYPTO_WASM_STANDALONE_TEST)
  if (js_crypto_hmac_sha256(k, key_len, d, data_len, out_mac)) {
    return CDD_C_SUCCESS;
  }
#endif

  if (key_len > 64) {
    struct cdd_sha256_ctx tctx;
    rc = cdd_sha256_init(&tctx);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_sha256_update(&tctx, k, key_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_sha256_final(&tctx, tk);
    if (rc != CDD_C_SUCCESS)
      return rc;
    k = tk;
    key_len = 32;
  }

  memset(k_ipad, 0, sizeof(k_ipad));
  memset(k_opad, 0, sizeof(k_opad));
  memcpy(k_ipad, k, key_len);
  memcpy(k_opad, k, key_len);

  for (i = 0; i < 64; i++) {
    k_ipad[i] ^= 0x36;
    k_opad[i] ^= 0x5c;
  }

  rc = cdd_sha256_init(&ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_sha256_update(&ctx, k_ipad, 64);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (data_len > 0) {
    rc = cdd_sha256_update(&ctx, d, data_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  rc = cdd_sha256_final(&ctx, out_digest);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_sha256_init(&ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_sha256_update(&ctx, k_opad, 64);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_sha256_update(&ctx, out_digest, 32);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_sha256_final(&ctx, out_mac);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}
