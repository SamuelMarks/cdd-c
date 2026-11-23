/**
 * @file test_platform_crypto_and_cst_unit.c
 * @brief 100%-coverage unit and OOM tests for `crypto_wasm.c`,
 * `crypto_openssl.c`, `crypto_wincrypt.c`, and `cdd_cst_transform.c`.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "c_cdd/log.h"
#include "c_str_span.h"
#include "cdd_c_error.h"
#include "cdd_cst_transform.h"
#include "classes/emit/cdd_cst_emit.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_node.h"
#include "classes/parse/cdd_cst_parser.h"
#include "functions/parse/crypto_types.h"
#include <greatest.h>
/* clang-format on */

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

/**
 * @brief Standalone test entrypoint `cdd_legacy_transform_extern_c`.
 * @param tree Parameter `tree`.
 * @param config Parameter `config`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_legacy_transform_extern_c(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);
/**
 * @brief Standalone test entrypoint `cdd_legacy_transform_msvc`.
 * @param tree Parameter `tree`.
 * @param config Parameter `config`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_legacy_transform_msvc(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);
/**
 * @brief Standalone test entrypoint `cdd_legacy_transform_gnu`.
 * @param tree Parameter `tree`.
 * @param config Parameter `config`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_legacy_transform_gnu(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);

/**
 * @brief Resets all crypto fault-injection globals to 0.
 */
static void reset_crypto_faults(void) {
  g_crypto_fail_sha256 = 0;
  g_crypto_fail_mdctx_new = 0;
  g_crypto_fail_digestinit = 0;
  g_crypto_fail_digestupdate = 0;
  g_crypto_fail_digestfinal = 0;
  g_crypto_fail_digestfinal_len = 0;
  g_crypto_fail_hmac = 0;
  g_crypto_fail_hmac_len = 0;
  g_crypto_wasm_fail_step = 0;
}

/**
 * @brief Exhaustive NIST & fault-injection tests for `crypto_wasm.c`.
 * @return Greatest test result.
 */
TEST test_crypto_wasm_exhaustive(void) {
  /* NIST SHA-256("abc") =
   * ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad */
  static const unsigned char expected_abc[32] = {
      0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40,
      0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17,
      0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
  /* NIST SHA-256("") =
   * e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 */
  static const unsigned char expected_empty[32] = {
      0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4,
      0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b,
      0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};
  unsigned char out[32];
  unsigned char out_ref[32];
  unsigned char msg_128[128];
  unsigned char long_key[131];
  int step;

  reset_crypto_faults();
  memset(msg_128, 'a', sizeof(msg_128));
  memset(long_key, 0xaa, sizeof(long_key));

  /* Invalid arguments for SHA-256 */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_wasm_crypto_sha256(NULL, 1, out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wasm_crypto_sha256("abc", 3, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wasm_crypto_sha256(NULL, 0, NULL));

  /* Valid SHA-256: NULL/0, "abc" (<56), 55 bytes, 56 bytes (>=56), 63 bytes,
   * 128 bytes (>=64) */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_wasm_crypto_sha256(NULL, 0, out));
  ASSERT_MEM_EQ(expected_empty, out, 32);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_wasm_crypto_sha256("abc", 3, out));
  ASSERT_MEM_EQ(expected_abc, out, 32);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_wasm_crypto_sha256(msg_128, 55, out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_wasm_crypto_sha256(msg_128, 56, out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_wasm_crypto_sha256(msg_128, 63, out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_wasm_crypto_sha256(msg_128, 128, out));

  /* Global fault flags on SHA-256 */
  g_crypto_fail_sha256 = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_wasm_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_mdctx_new = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_wasm_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_digestinit = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_wasm_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_digestupdate = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_wasm_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_digestfinal = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_wasm_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_digestfinal_len = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_wasm_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  /* Countdown step failures across init (1), update entry (2), update >=64 (3),
   * final >=56 (4, 5), and <56 (1..3) */
  for (step = 1; step <= 5; step++) {
    g_crypto_wasm_fail_step = step;
    ASSERT_EQ(CDD_C_ERROR_IO, cdd_wasm_crypto_sha256(msg_128, 120, out));
  }
  g_crypto_wasm_fail_step = 6;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_wasm_crypto_sha256(msg_128, 120, out));
  for (step = 1; step <= 3; step++) {
    g_crypto_wasm_fail_step = step;
    ASSERT_EQ(CDD_C_ERROR_IO, cdd_wasm_crypto_sha256("abc", 3, out));
  }
  reset_crypto_faults();

  /* Invalid arguments for HMAC-SHA-256 */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wasm_crypto_hmac_sha256(NULL, 1, "abc", 3, out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wasm_crypto_hmac_sha256("k", 1, NULL, 1, out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wasm_crypto_hmac_sha256("k", 1, "abc", 3, NULL));

  /* Global fault flags on HMAC-SHA-256 */
  g_crypto_fail_hmac = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_wasm_crypto_hmac_sha256("k", 1, "abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_hmac_len = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_wasm_crypto_hmac_sha256("k", 1, "abc", 3, out));
  reset_crypto_faults();

  /* Valid HMAC-SHA-256: NULL/0 key & data, short key, and >64-byte key (RFC
   * 4231 Case 6) */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_wasm_crypto_hmac_sha256(NULL, 0, NULL, 0, out));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_wasm_crypto_hmac_sha256(
                "Jefe", 4, "what do ya want for nothing?", 28, out));
  ASSERT_EQ(CDD_C_SUCCESS,
            crypto_hmac_sha256("Jefe", 4, "what do ya want for nothing?", 28,
                               out_ref));
  ASSERT_MEM_EQ(out_ref, out, 32);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_wasm_crypto_hmac_sha256(
                long_key, sizeof(long_key),
                "Test Using Larger Than Block-Size Key - Hash Key First", 54,
                out));
  ASSERT_EQ(CDD_C_SUCCESS,
            crypto_hmac_sha256(
                long_key, sizeof(long_key),
                "Test Using Larger Than Block-Size Key - Hash Key First", 54,
                out_ref));
  ASSERT_MEM_EQ(out_ref, out, 32);

  /* Step-by-step error percolation across every init/update/final in HMAC */
  for (step = 1; step <= 16; step++) {
    g_crypto_wasm_fail_step = step;
    ASSERT_EQ(CDD_C_ERROR_IO,
              cdd_wasm_crypto_hmac_sha256(long_key, sizeof(long_key), msg_128,
                                          70, out));
  }
  g_crypto_wasm_fail_step = 17;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_wasm_crypto_hmac_sha256(
                               long_key, sizeof(long_key), msg_128, 70, out));
  reset_crypto_faults();
  PASS();
}

/**
 * @brief Exhaustive tests for `crypto_openssl.c` and `crypto_wincrypt.c`.
 * @return Greatest test result.
 */
TEST test_crypto_openssl_and_wincrypt_exhaustive(void) {
  unsigned char out[32];
#if !defined(CDD_TEST_NO_OPENSSL)
  unsigned char ref[32];
  unsigned char dummy = 'k';
#endif

  reset_crypto_faults();

  /* 1. crypto_openssl.c */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_openssl_crypto_sha256(NULL, 1, out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_openssl_crypto_sha256("abc", 3, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_openssl_crypto_hmac_sha256(NULL, 1, "abc", 3, out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_openssl_crypto_hmac_sha256("k", 1, NULL, 1, out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_openssl_crypto_hmac_sha256("k", 1, "abc", 3, NULL));

#if !defined(CDD_TEST_NO_OPENSSL)
  ASSERT_EQ(CDD_C_SUCCESS, cdd_openssl_crypto_sha256(NULL, 0, out));
  ASSERT_EQ(CDD_C_SUCCESS, crypto_sha256(NULL, 0, ref));
  ASSERT_MEM_EQ(ref, out, 32);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_openssl_crypto_sha256("abc", 3, out));
  ASSERT_EQ(CDD_C_SUCCESS, crypto_sha256("abc", 3, ref));
  ASSERT_MEM_EQ(ref, out, 32);

  g_crypto_fail_sha256 = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_openssl_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_mdctx_new = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_openssl_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_digestinit = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_openssl_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_digestupdate = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_openssl_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_digestfinal = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_openssl_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_digestfinal_len = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_openssl_crypto_sha256("abc", 3, out));
  reset_crypto_faults();

  /* HMAC-SHA256: >INT_MAX key_len, empty key, empty data, normal, and hooks */
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_openssl_crypto_hmac_sha256(
                                &dummy, ((size_t)INT_MAX) + 1u, "abc", 3, out));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_openssl_crypto_hmac_sha256(NULL, 0, NULL, 0, out));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_openssl_crypto_hmac_sha256(
                "Jefe", 4, "what do ya want for nothing?", 28, out));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      crypto_hmac_sha256("Jefe", 4, "what do ya want for nothing?", 28, ref));
  ASSERT_MEM_EQ(ref, out, 32);

  g_crypto_fail_hmac = 1;
  ASSERT_EQ(CDD_C_ERROR_IO,
            cdd_openssl_crypto_hmac_sha256("Jefe", 4, "abc", 3, out));
  reset_crypto_faults();

  g_crypto_fail_hmac_len = 1;
  ASSERT_EQ(CDD_C_ERROR_IO,
            cdd_openssl_crypto_hmac_sha256("Jefe", 4, "abc", 3, out));
  reset_crypto_faults();
#else
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, cdd_openssl_crypto_sha256("abc", 3, out));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, cdd_openssl_crypto_sha256(NULL, 0, out));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            cdd_openssl_crypto_hmac_sha256("k", 1, "abc", 3, out));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            cdd_openssl_crypto_hmac_sha256(NULL, 0, NULL, 0, out));
#endif

  /* 2. crypto_wincrypt.c standalone (non-Windows stub on POSIX, CAPI on Win32)
   */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wincrypt_crypto_sha256(NULL, 1, out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wincrypt_crypto_sha256("abc", 3, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wincrypt_crypto_hmac_sha256(NULL, 1, "abc", 3, out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wincrypt_crypto_hmac_sha256("k", 1, NULL, 1, out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_wincrypt_crypto_hmac_sha256("k", 1, "abc", 3, NULL));
#if !defined(_WIN32)
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, cdd_wincrypt_crypto_sha256("abc", 3, out));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, cdd_wincrypt_crypto_sha256(NULL, 0, out));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            cdd_wincrypt_crypto_hmac_sha256("k", 1, "abc", 3, out));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            cdd_wincrypt_crypto_hmac_sha256(NULL, 0, NULL, 0, out));
#else
  ASSERT_EQ(CDD_C_SUCCESS, cdd_wincrypt_crypto_sha256("abc", 3, out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_wincrypt_crypto_sha256(NULL, 0, out));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_wincrypt_crypto_hmac_sha256("Jefe", 4, "abc", 3, out));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_wincrypt_crypto_hmac_sha256(NULL, 0, NULL, 0, out));
#endif

  PASS();
}

/**
 * @brief Exhaustive unit and OOM tests for
 * `src/classes/parse/cdd_cst_transform.c`.
 * @return Greatest test result.
 */
TEST test_legacy_cst_transform_exhaustive(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_tree_t empty_tree;
  cdd_cst_node_t empty_root;
  cdd_transform_config_t cfg;
  char *emitted = NULL;
  int fail_step;

  memset(&cfg, 0, sizeof(cfg));
  memset(&empty_tree, 0, sizeof(empty_tree));
  memset(&empty_root, 0, sizeof(empty_root));

  /* 1. Invalid arguments and empty root */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_legacy_transform_extern_c(NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_legacy_transform_extern_c(&empty_tree, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_legacy_transform_msvc(NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_legacy_transform_msvc(&empty_tree, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_legacy_transform_gnu(NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_legacy_transform_gnu(&empty_tree, &cfg));

  empty_tree.root = &empty_root;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_extern_c(&empty_tree, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_legacy_transform_msvc(&empty_tree, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_legacy_transform_gnu(&empty_tree, &cfg));

  /* 2. extern "C": with #include + #define + declaration, without #include,
   *    with #ifndef OTHER, and idempotent when __cplusplus already present */
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_cst_parse(az_span_create_from_str(
                        "#ifndef HDR_H\n#define HDR_H\n#endif\n"
                        "#include <stdio.h>\n#define X 1\nint foo(void);\n"),
                    &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_extern_c(tree, &cfg));
  /* Second call hits found_cpp == 1 idempotence branch */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_extern_c(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &emitted));
  ASSERT(strstr(emitted, "extern \"C\"") != NULL);
  C_CDD_FREE(emitted);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  /* Without any #include (inserts before first child, and hits
   * FUNCTION_DEFINITION break) */
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_cst_parse(
          az_span_create_from_str(
              "#ifdef OTHER_MACRO\n#endif\nint bar(void) { return 0; }\n"),
          &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_extern_c(tree, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  /* Synthetic malformed conditional/directive children (0 children, NODE child,
   * NULL token, short token) */
  {
    cdd_cst_node_t *cond_empty = NULL;
    cdd_cst_node_t *cond_null_tok = NULL;
    cdd_cst_node_t *sub_node = NULL;
    cdd_token_t short_tok;

    memset(&short_tok, 0, sizeof(short_tok));
    short_tok.kind = CDD_TOKEN_PREPROC_IFDEF;
    short_tok.start = (const uint8_t *)"#if";
    short_tok.length = 3;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x;\n"), &tree));
    /* Add a direct token child on root + empty conditional + empty directive +
     * node-child directive + declaration */
    {
      cdd_cst_node_t *dir_e = NULL;
      cdd_cst_node_t *dir_n = NULL;
      cdd_cst_node_t *dir_sub = NULL;
      cdd_cst_node_t *decl_n = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &dir_e));
      dir_e->parent = tree->root;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, dir_e));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &dir_n));
      dir_n->parent = tree->root;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dir_sub));
      dir_sub->parent = dir_n;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(dir_n, dir_sub));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, dir_n));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_append_child_token(tree->root, &short_tok));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_DECLARATION, &decl_n));
      decl_n->parent = tree->root;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, decl_n));
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_PREPROC_CONDITIONAL, &cond_empty));
    cond_empty->parent = tree->root;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, cond_empty));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_PREPROC_CONDITIONAL, &cond_null_tok));
    cond_null_tok->parent = tree->root;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &sub_node));
    sub_node->parent = cond_null_tok;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_append_child_node(cond_null_tok, sub_node));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_append_child_node(tree->root, cond_null_tok));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_extern_c(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    /* NULL token inside conditional triggers CDD_C_ERROR_INVALID_ARGUMENT */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x;\n"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_PREPROC_CONDITIONAL, &cond_null_tok));
    cond_null_tok->parent = tree->root;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_append_child_token(cond_null_tok, &short_tok));
    cond_null_tok->children[0].val.token = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_append_child_node(tree->root, cond_null_tok));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_legacy_transform_extern_c(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  }

  /* OOM sweeps across cdd_legacy_transform_extern_c */
  for (fail_step = 1; fail_step <= 25; fail_step++) {
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "#include <stdio.h>\nint x;\n"),
                                           &tree));
    g_cdd_alloc_fail = fail_step;
    g_cdd_last_alloc_err = cdd_legacy_transform_extern_c(tree, &cfg);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "#include <stdio.h>\nint x;\n"),
                                           &tree));
    g_cdd_cst_realloc_fail = fail_step;
    g_cdd_last_alloc_err = cdd_legacy_transform_extern_c(tree, &cfg);
    g_cdd_cst_realloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "#include <stdio.h>\nint x;\n"),
                                           &tree));
    g_cdd_cst_alloc_node_fail = fail_step;
    g_cdd_last_alloc_err = cdd_legacy_transform_extern_c(tree, &cfg);
    g_cdd_cst_alloc_node_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "#include <stdio.h>\nint x;\n"),
                                           &tree));
    g_cdd_cst_alloc_token_fail = fail_step;
    g_cdd_last_alloc_err = cdd_legacy_transform_extern_c(tree, &cfg);
    g_cdd_cst_alloc_token_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  }

  /* Also hit insert_node_after / insert_node_before failure when root child has
   * parent == NULL */
  {
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "#include <stdio.h>\nint x;\n"),
                                           &tree));
    tree->root->children[0].val.node->parent = NULL;
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_legacy_transform_extern_c(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("int x;\nint y;\n"), &tree));
    tree->root->children[0].val.node->parent = NULL;
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_legacy_transform_extern_c(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("int x;\nint y;\n"), &tree));
    tree->root->children[1].val.node->parent = NULL;
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_legacy_transform_extern_c(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "#ifdef A\n#endif\nint x;\n"),
                                           &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_extern_c(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  }

  /* OOM on cdd_cst_find_nodes_by_type in extern_c (with #ifdef present) */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                             "#ifdef OTHER\n#endif\nint x;\n"),
                                         &tree));
  g_cdd_cst_realloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_legacy_transform_extern_c(tree, &cfg));
  g_cdd_cst_realloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  /* 3. cdd_legacy_transform_msvc: <unistd.h>, <sys/time.h>, <stdio.h>, #define,
   *    POSIX identifiers + same-length non-matching identifiers, malformed
   * directive nodes, and OOM sweeps */
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_cst_parse(
          az_span_create_from_str(
              "#include <unistd.h>\n#include <sys/time.h>\n"
              "#include <stdio.h>\n#define M 1\n"
              "int abcdefghij, abcdefghijk, abcdef, abcdefg, "
              "abcdefghijklmnop;\n"
              "void f(void) { ssize_t n = strcasecmp(a, b) + strncasecmp(a, b, "
              "1); "
              "char *p = strdup(a); if (__builtin_expect(n, 1)) return; }\n"),
          &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_msvc(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &emitted));
  ASSERT(strstr(emitted, "win_compat_sym.h") != NULL);
  ASSERT(strstr(emitted, "_stricmp") != NULL);
  ASSERT(strstr(emitted, "_strnicmp") != NULL);
  ASSERT(strstr(emitted, "_strdup") != NULL);
  ASSERT(strstr(emitted, "SSIZE_T") != NULL);
  ASSERT(strstr(emitted, "cdd_builtin_expect") != NULL);
  C_CDD_FREE(emitted);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  /* Synthetic directive nodes in MSVC transform (empty dir, NULL token,
   * parent==NULL replace fail) */
  {
    cdd_cst_node_t *dir_empty = NULL;
    cdd_cst_node_t *dir_null_tok = NULL;
    cdd_token_t dir_tok;

    memset(&dir_tok, 0, sizeof(dir_tok));
    dir_tok.kind = CDD_TOKEN_PREPROC_INCLUDE;
    dir_tok.start = (const uint8_t *)"#include";
    dir_tok.length = 8;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x;\n"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &dir_empty));
    dir_empty->parent = tree->root;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, dir_empty));
    {
      cdd_cst_node_t *dir_node_ch = NULL;
      cdd_cst_node_t *inner = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &dir_node_ch));
      dir_node_ch->parent = tree->root;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &inner));
      inner->parent = dir_node_ch;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(dir_node_ch, inner));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_append_child_node(tree->root, dir_node_ch));
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x;\n"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &dir_null_tok));
    dir_null_tok->parent = tree->root;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_append_child_token(dir_null_tok, &dir_tok));
    dir_null_tok->children[0].val.token = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_append_child_node(tree->root, dir_null_tok));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_legacy_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "#include <unistd.h>\nint x;\n"),
                                           &tree));
    tree->root->children[0].val.node->parent = NULL;
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_legacy_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(
            az_span_create_from_str("#include <sys/time.h>\nint x;\n"), &tree));
    tree->root->children[0].val.node->parent = NULL;
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_legacy_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  }

  for (fail_step = 1; fail_step <= 25; fail_step++) {
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "#include <unistd.h>\n#include <sys/time.h>\n"),
                            &tree));
    g_cdd_alloc_fail = fail_step;
    g_cdd_last_alloc_err = cdd_legacy_transform_msvc(tree, &cfg);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "#include <unistd.h>\n#include <sys/time.h>\n"),
                            &tree));
    g_cdd_cst_realloc_fail = fail_step;
    g_cdd_last_alloc_err = cdd_legacy_transform_msvc(tree, &cfg);
    g_cdd_cst_realloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  }

  /* 4. cdd_legacy_transform_gnu: attributes (noreturn, unused, packed with
   *    braces & without semicolon, unknown attr, malformed __attribute__),
   *    __extension__, __alignof__, statement exprs, >8 VLAs (pool growth!),
   *    non-VLA brackets, VLA without semicolon, empty struct {}, and OOM! */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(
                              "__attribute__((noreturn)) void die(void);\n"
                              "__attribute__((unused)) int u;\n"
                              "__attribute__((abcdefgh)) int c8;\n"
                              "__attribute__((abcdef)) int c6;\n"
                              "struct __attribute__((packed)) S { int a; };\n"
                              "struct Empty {};\n"
                              "int abcdefghijklm, abcdefghijk;\n"),
                          &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(
                az_span_create_from_str("int __attribute__ y = 1;\n"
                                        "int __attribute__(a, b, c, d, e);\n"
                                        "int __attribute__((a, b));\n"
                                        "int __attribute__((a) + 1);\n"),
                &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(
                              "x[n];\n"
                              "void g2(int n) { (a)[n]; a[n + 1]; }\n"),
                          &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(
                az_span_create_from_str(
                    "void g(int n) {\n"
                    "  __extension__ int e = __alignof__(int);\n"
                    "  int v = ({ int z = 1; z; });\n"
                    "  int a0[n]; int a1[n]; int a2[n]; int a3[n];\n"
                    "  int a4[n]; int a5[n]; int a6[n]; int a7[n]; int a8[n];\n"
                    "  int fixed[4]; a0[0] = v + e + fixed[0];\n"
                    "}\n"),
                &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  /* Packed without trailing semicolon and VLA without trailing semicolon */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(
                              "struct __attribute__((packed)) S { int a }\n"),
                          &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("int arr[n] = 0;"), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("int arr[n]"), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
  {
    cdd_token_t trail_lbrack[3];
    cdd_token_list_t trail_list;
    cdd_token_list_t *orig_list = tree->base_tokens;
    memset(trail_lbrack, 0, sizeof(trail_lbrack));
    trail_lbrack[0].kind = CDD_TOKEN_IDENTIFIER;
    trail_lbrack[1].kind = CDD_TOKEN_IDENTIFIER;
    trail_lbrack[2].kind = CDD_TOKEN_LBRACKET;
    trail_list.tokens = trail_lbrack;
    trail_list.size = 3;
    trail_list.capacity = 3;
    tree->base_tokens = &trail_list;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
    tree->base_tokens = orig_list;
  }
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  /* Edge tokens at start/end of stream: `[`, `(`, `}`, `{`, `__attribute__` */
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_cst_parse(az_span_create_from_str("[ int __attribute__"), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
  {
    cdd_token_t edge_toks[4];
    cdd_token_list_t edge_list;
    cdd_token_list_t *orig_list = tree->base_tokens;
    memset(edge_toks, 0, sizeof(edge_toks));
    edge_toks[0].kind = CDD_TOKEN_LPAREN;
    edge_toks[1].kind = CDD_TOKEN_RBRACE;
    edge_toks[2].kind = CDD_TOKEN_LBRACE;
    edge_toks[3].kind = CDD_TOKEN_LBRACKET;
    edge_list.tokens = edge_toks;
    edge_list.size = 1;
    edge_list.capacity = 4;
    tree->base_tokens = &edge_list;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
    edge_list.tokens = &edge_toks[1];
    ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
    edge_list.tokens = &edge_toks[2];
    ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
    edge_list.tokens = &edge_toks[3];
    ASSERT_EQ(CDD_C_SUCCESS, cdd_legacy_transform_gnu(tree, &cfg));
    tree->base_tokens = orig_list;
  }
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  /* OOM on VLA string_pool realloc and strdup */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("void f(int n){ int a[n]; }"),
                          &tree));
  g_cdd_cst_realloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_legacy_transform_gnu(tree, &cfg));
  g_cdd_cst_realloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("void f(int n){ int a[n]; }"),
                          &tree));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_legacy_transform_gnu(tree, &cfg));
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  PASS();
}

/**
 * @brief Suite covering platform crypto backends and legacy CST transforms.
 */
SUITE(platform_crypto_and_cst_suite) {
  RUN_TEST(test_crypto_wasm_exhaustive);
  RUN_TEST(test_crypto_openssl_and_wincrypt_exhaustive);
  RUN_TEST(test_legacy_cst_transform_exhaustive);
}
