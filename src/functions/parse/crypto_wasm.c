/**
 * @file crypto_wasm.c
 * @brief WASM Crypto implementation (Native C Fallback + Node/JS Sync wrapper)
 */

/* clang-format off */
#include "functions/parse/crypto_types.h"
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#include <stdint.h>
#endif
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

/* Synchronous SHA-256 for Node/Deno/Bun using EM_JS */
EM_JS(int, js_crypto_sha256, (const uint8_t* data, size_t len, uint8_t* out), {
  if (typeof process !== 'undefined' && process.versions && process.versions.node) {
    try {
      var crypto = require('crypto');
      var hash = crypto.createHash('sha256');
      hash.update(new Uint8Array(HEAPU8.buffer, data, len));
      var digest = hash.digest();
      HEAPU8.set(digest, out);
      return 1;
    } catch(e) {
      return 0;
    }
  }
  return 0;
})

/* Synchronous HMAC-SHA-256 for Node/Deno/Bun using EM_JS */
EM_JS(int, js_crypto_hmac_sha256, (const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t* out), {
  if (typeof process !== 'undefined' && process.versions && process.versions.node) {
    try {
      var crypto = require('crypto');
      var hmac = crypto.createHmac('sha256', new Uint8Array(HEAPU8.buffer, key, key_len));
      hmac.update(new Uint8Array(HEAPU8.buffer, data, data_len));
      var digest = hmac.digest();
      HEAPU8.set(digest, out);
      return 1;
    } catch(e) {
      return 0;
    }
  }
  return 0;
})
#endif

/* Native C Fallback for Browser/WASI (from crypto_standalone.c) */
struct cdd_sha256_ctx {
  uint32_t state[8];
  uint64_t bitlen;
  uint8_t data[64];
  uint32_t datalen;
};

static const uint32_t cdd_k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define CDD_ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CDD_CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define CDD_MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define CDD_EP0(x) (CDD_ROTRIGHT(x,2) ^ CDD_ROTRIGHT(x,13) ^ CDD_ROTRIGHT(x,22))
#define CDD_EP1(x) (CDD_ROTRIGHT(x,6) ^ CDD_ROTRIGHT(x,11) ^ CDD_ROTRIGHT(x,25))
#define CDD_SIG0(x) (CDD_ROTRIGHT(x,7) ^ CDD_ROTRIGHT(x,18) ^ ((x) >> 3))
#define CDD_SIG1(x) (CDD_ROTRIGHT(x,17) ^ CDD_ROTRIGHT(x,19) ^ ((x) >> 10))

static enum cdd_c_error cdd_sha256_transform(struct cdd_sha256_ctx *ctx, const uint8_t data[]) {
  uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

  for (i = 0, j = 0; i < 16; ++i, j += 4)
    m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j + 1] << 16) | ((uint32_t)data[j + 2] << 8) | ((uint32_t)data[j + 3]);
  for ( ; i < 64; ++i)
    m[i] = CDD_SIG1(m[i - 2]) + m[i - 7] + CDD_SIG0(m[i - 15]) + m[i - 16];

  a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
  e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

  for (i = 0; i < 64; ++i) {
    t1 = h + CDD_EP1(e) + CDD_CH(e,f,g) + cdd_k[i] + m[i];
    t2 = CDD_EP0(a) + CDD_MAJ(a,b,c);
    h = g; g = f; f = e; e = d + t1;
    d = c; c = b; b = a; a = t1 + t2;
  }

  ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
  ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
  return CDD_C_SUCCESS;
}

static enum cdd_c_error cdd_sha256_init(struct cdd_sha256_ctx *ctx) {
  ctx->datalen = 0;
  ctx->bitlen = 0;
  ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
  ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
  ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
  ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
  return CDD_C_SUCCESS;
}

static enum cdd_c_error cdd_sha256_update(struct cdd_sha256_ctx *ctx, const uint8_t data[], size_t len) {
  size_t i;
  for (i = 0; i < len; ++i) {
    ctx->data[ctx->datalen] = data[i];
    ctx->datalen++;
    if (ctx->datalen == 64) {
      cdd_sha256_transform(ctx, ctx->data);
      ctx->bitlen += 512;
      ctx->datalen = 0;
    }
  }
  return CDD_C_SUCCESS;
}

static enum cdd_c_error cdd_sha256_final(struct cdd_sha256_ctx *ctx, uint8_t hash[]) {
  uint32_t i;
  i = ctx->datalen;
  if (ctx->datalen < 56) {
    ctx->data[i++] = 0x80;
    while (i < 56) ctx->data[i++] = 0x00;
  } else {
    ctx->data[i++] = 0x80;
    while (i < 64) ctx->data[i++] = 0x00;
    cdd_sha256_transform(ctx, ctx->data);
    memset(ctx->data, 0, 56);
  }
  ctx->bitlen += ctx->datalen * 8;
  ctx->data[63] = (uint8_t)(ctx->bitlen);
  ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
  ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
  ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
  ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
  ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
  ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
  ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
  cdd_sha256_transform(ctx, ctx->data);
  for (i = 0; i < 4; ++i) {
    hash[i]      = (uint8_t)((ctx->state[0] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 4]  = (uint8_t)((ctx->state[1] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 8]  = (uint8_t)((ctx->state[2] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i * 8)) & 0x000000ff);
    hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i * 8)) & 0x000000ff);
  }
  return CDD_C_SUCCESS;
}
/* clang-format on */

enum cdd_c_error crypto_sha256(const void *data, size_t data_len,
                               unsigned char *out_digest) {
  if ((!data && data_len > 0) || !out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef __EMSCRIPTEN__
  if (js_crypto_sha256((const uint8_t *)data, data_len, out_digest)) {
    return CDD_C_SUCCESS;
  }
#endif

  {
    struct cdd_sha256_ctx ctx;
    (void)cdd_sha256_init(&ctx);
    if (data && data_len > 0) {
      (void)cdd_sha256_update(&ctx, (const uint8_t *)data, data_len);
    }
    (void)cdd_sha256_final(&ctx, out_digest);
    return CDD_C_SUCCESS;
  }
}

enum cdd_c_error crypto_hmac_sha256(const void *key, size_t key_len,
                                    const void *data, size_t data_len,
                                    unsigned char *out_digest) {
  if (!key || (!data && data_len > 0) || !out_digest)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef __EMSCRIPTEN__
  if (js_crypto_hmac_sha256((const uint8_t *)key, key_len,
                            (const uint8_t *)data, data_len, out_digest)) {
    return CDD_C_SUCCESS;
  }
#endif

  {
    struct cdd_sha256_ctx ctx;
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t tk[32];
    size_t i;
    const uint8_t *k = (const uint8_t *)key;

    if (key_len > 64) {
      struct cdd_sha256_ctx tctx;
      cdd_sha256_init(&tctx);
      cdd_sha256_update(&tctx, k, key_len);
      cdd_sha256_final(&tctx, tk);
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

    (void)cdd_sha256_init(&ctx);
    (void)cdd_sha256_update(&ctx, k_ipad, 64);
    if (data && data_len > 0) {
      (void)cdd_sha256_update(&ctx, (const uint8_t *)data, data_len);
    }
    (void)cdd_sha256_final(&ctx, out_digest);

    (void)cdd_sha256_init(&ctx);
    (void)cdd_sha256_update(&ctx, k_opad, 64);
    (void)cdd_sha256_update(&ctx, out_digest, 32);
    (void)cdd_sha256_final(&ctx, out_digest);

    return CDD_C_SUCCESS;
  }
}
